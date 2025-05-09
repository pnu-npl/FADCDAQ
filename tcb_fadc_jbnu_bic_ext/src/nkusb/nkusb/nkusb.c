/*
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nkusb.h"
#include "nkusbconst.h"

#define PVMEX_GET_ADDRESS             (0xD2)

enum EManipInterface {kInterfaceClaim, kInterfaceRelease};

/*struct dev_open {
   libusb_device_handle *devh;
   struct dev_open *next;
} *ldev_open = 0;
*/

struct dev_open {
   libusb_device_handle *devh;
   uint16_t vendor_id;
   uint16_t product_id;
   int serial_id;
   struct dev_open *next;
} *ldev_open = 0;

///////////////////////////////////////////////////////////////////////////////
static void add_device(struct dev_open **list, libusb_device_handle *tobeadded,
                       uint16_t vendor_id, uint16_t product_id, int sid)
{
// Add device to the open device list

   struct dev_open *curr;
   curr = (struct dev_open *)malloc(sizeof(struct dev_open));
   curr->devh = tobeadded;
   curr->vendor_id = vendor_id;
   curr->product_id = product_id;
   curr->serial_id = sid;
   curr->next  = *list;
   *list = curr;
}

///////////////////////////////////////////////////////////////////////////////
static unsigned char get_serial_id(libusb_device_handle *devh)
{
// Get serial id of device handle devh. devh may or may not be on open device list 'ldev_open'.
// Returns 0 if error.
   int ret;
   if (!devh) {
      return 0;
   }
   unsigned char data[1];
   ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, PVMEX_GET_ADDRESS, 0, 0, data, 1, 1000);

   if (ret < 0) {
      fprintf(stdout, "Warning: get_serial_id: Could not get serial id.\n");

fprintf(stdout, "Error code = %d\n", ret);

      return 0;
   }
   return data[0];
}

///////////////////////////////////////////////////////////////////////////////
static void remove_device_handle(struct dev_open **list, libusb_device_handle *devh)
{
// Close the device and remove from the open device list

   if (!*list) return;

   struct dev_open *curr = *list;
   struct dev_open *prev = 0;

   while (curr) {
      if (curr->devh == devh) {
         if ((*list) == curr) { //update head
            *list = curr->next;
         }
         else {
            prev->next = curr->next;
         }
         libusb_close(curr->devh);
         free(curr);
         return;
      }
      else {
         prev = curr;
         curr = curr->next;
      }
   }
   return;
}

///////////////////////////////////////////////////////////////////////////////
static void remove_device_id(struct dev_open **list, uint16_t vendor_id, uint16_t product_id,
                          int sid)
{
// Close and remove device with specified vendor id, product id and serial id
// from open device list. If sid == NK_SID_ANY, all devices with given vendor id
// and product id are removed.

   if (!*list) return;

   struct dev_open *curr = *list;
   struct dev_open *prev = 0;
   struct libusb_device_descriptor desc;
   libusb_device *dev;

   while (curr) {
      dev =libusb_get_device(curr->devh);
      if (libusb_get_device_descriptor(dev, &desc) < 0) {
         // Ignore with message
         fprintf(stdout, "Warning, remove_device: could not get device device descriptior."
                          " Ignoring.\n");
          continue;
      }
      if (desc.idVendor == vendor_id && desc.idProduct == product_id
       && (sid == NK_SID_ANY || sid == get_serial_id(curr->devh))) { 
          // Match.
          if (*list == curr) { 
             // Update head, remove current element and move cursor forward.
             *list = curr->next;
             libusb_close(curr->devh);
             free(curr); 
             curr = *list;
          }
          else {
             // Update link, remove current element and move cursor forward.
             prev->next = curr->next;
             libusb_close(curr->devh);
             free(curr); 
             curr = prev->next;
          }
      }
      else {
         // No match. Move cursor forward.
         prev = curr;
         curr = curr->next;
      }    
   }
}

///////////////////////////////////////////////////////////////////////////////
static int handle_interface_id(struct dev_open **list, uint16_t vendor_id, uint16_t product_id,
                          int sid, int interface, enum EManipInterface manip_type)
{
// Manipulate interface on the device with specified vendor id, product id and serial id
// from open device list. Claim interface if manip_type == kInterfaceClaim, release interface
// if manip_type == kInterfaceRelease. If sid == NK_SID_ANY, all devices with given vendor id
// and product id are handled.

   int ret = 0;
   if (!*list) {
      ret = -1;
      return ret;
   }

   struct dev_open *curr = *list;
   struct libusb_device_descriptor desc;
   libusb_device *dev;

   while (curr) {
      dev =libusb_get_device(curr->devh);
      if (libusb_get_device_descriptor(dev, &desc) < 0) {
         // Ignore with message
         fprintf(stdout, "Warning: remove_device: could not get device device descriptior."
                          " Ignoring.\n");
          continue;
      }
      if (desc.idVendor == vendor_id && desc.idProduct == product_id
       && (sid == NK_SID_ANY || sid == get_serial_id(curr->devh))) { 
          // Match.
          if (manip_type == kInterfaceClaim) {
             if ((ret = libusb_claim_interface(curr->devh, interface)) < 0) {
                fprintf(stdout, "Warning: handle_interface_id: Could not claim interface (%d) on device (%u, %u, %u)\n",
                         interface, vendor_id, product_id, sid);
             }
          }
          else if (manip_type == kInterfaceRelease) {
             if ((ret =libusb_release_interface(curr->devh, interface)) < 0) {
                fprintf(stdout, "Warning: handle_interface_id: Could not release interface (%d) on device (%u, %u, %u)\n",
                         interface, vendor_id, product_id, sid);
             }
          }
          else {
             fprintf(stderr, "Error: handle_interface_id: Unknown interface handle request: %d\n.",
                              manip_type);
              
             ret = -1;
             return ret;
          }
      }
      // Move forward
      curr = curr->next;
   }
   return ret;
}

///////////////////////////////////////////////////////////////////////////////
static int is_device_open(libusb_device_handle *devh)
{
// See if the device handle "devh" is on the open device list

   struct dev_open *curr = ldev_open;
   libusb_device *dev, *dev_curr;
   int bus, bus_curr, addr, addr_curr;
   while (curr) {

      dev_curr = libusb_get_device(curr->devh);
      bus_curr = libusb_get_bus_number(dev_curr);
      addr_curr = libusb_get_device_address(dev_curr);

      dev = libusb_get_device(devh);
      bus = libusb_get_bus_number(dev);
      addr = libusb_get_device_address(dev);

      if (bus == bus_curr && addr == addr_curr) return 1;
      curr = curr->next;
   }
   return 0;
}

/*
///////////////////////////////////////////////////////////////////////////////
libusb_device_handle* nkusb_get_device_handle(uint16_t vendor_id, uint16_t product_id,
                  int sid) {
// Get open device handle with given vendor id, product id and serial id

   libusb_device *dev = 0;
   int r;

   struct dev_open *curr = ldev_open;
   while (curr) {
      dev =libusb_get_device(curr->devh);
      struct libusb_device_descriptor desc;
      r = libusb_get_device_descriptor(dev, &desc);
      if (r < 0) {
         // Ignore with message
         fprintf(stdout, "Warning, nkusb_get_device_handle: could not get device device descriptior."
                  " Ignoring.\n");
          continue;
      }
      if (desc.idVendor == vendor_id && desc.idProduct == product_id && sid == get_serial_id(curr->devh))
         return curr->devh;
      curr = curr->next;
   }
   return 0;
}*/

///////////////////////////////////////////////////////////////////////////////
libusb_device_handle* nkusb_get_device_handle(uint16_t vendor_id, uint16_t product_id,
                  int sid) {
// Get open device handle with given vendor id, product id and serial id.
// Return first found device handle if sid == NK_SID_ANY.

   struct dev_open *curr = ldev_open;
   while (curr) {
      if (curr->vendor_id == vendor_id && curr->product_id == product_id) {
         if (sid == NK_SID_ANY)
            return curr->devh;
         else if (curr->serial_id == sid) 
            return curr->devh;
      }
      curr = curr->next;
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_is_device_open(uint16_t vendor_id, uint16_t product_id, int sid)
{
// See if the device with given vendor id, product id and serial id is 
// on the open device list. sid of value NK_SID_ANY means any serial id.
// If the device's serial id is not NK_SID_ANY, its interface must have been claimed.
// Otherwise it will be regarded as not open.

   struct dev_open *curr = ldev_open;
   struct libusb_device_descriptor desc;
   libusb_device *dev;
   int r;
   while (curr) {
      dev = libusb_get_device(curr->devh);
      r = libusb_get_device_descriptor(dev, &desc);
      if (r < 0) {
         // Ignore with message
         fprintf(stdout, "Warning, nkusb_is_device_open: could not get device device descriptior."
                 " Ignoring.\n");
         continue;
      }
      if (desc.idVendor == vendor_id && desc.idProduct == product_id
          && (sid == NK_SID_ANY || get_serial_id(curr->devh) == sid)) return 1;
      curr = curr->next;
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_init(libusb_context **ctx)
{
// Initialize libusb3

   int ret;
   if ((ret = libusb_init(ctx)) < 0) {
      fprintf(stderr, "failed to initialise libusb\n");
      exit(1);
   }
   return ret;
}

///////////////////////////////////////////////////////////////////////////////
void nkusb_print_open_devices(void)
{
// Print out open devices

   struct dev_open* curr = ldev_open;
   libusb_device *dev;
   while (curr) {
      dev = libusb_get_device(curr->devh);
      fprintf(stdout, "bus = %d, address = %d\n",
         libusb_get_bus_number(dev), libusb_get_device_address(dev));
      curr = curr->next;
   }
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_open_device(uint16_t vendor_id, uint16_t product_id,
             int sid, libusb_context *ctx)
{
// Open the device with vender id, product id and serial id. If sid == NK_SID_ANY, open all
// devices with gievn vendor id and product id. Returns 0 if any device is open, <0 otherwise.

   struct libusb_device **devs;
   struct libusb_device *dev;
   struct libusb_device_handle *devh;
   size_t i = 0;
   int nopen_devices = 0; //number of open devices

   int r;
   int status = 1;

   if (libusb_get_device_list(ctx, &devs) < 0) {
      fprintf(stderr, "Error: nkusb_open_device: Could not get device list\n");
      return -1;
   }
   fprintf(stdout, "Info: nkusb_open_device: opening device Vendor ID = 0x%X, Product ID = 0x%X, Serial ID = %u\n",
                   vendor_id, product_id, sid);

   while ((dev = devs[i++])) {
      struct libusb_device_descriptor desc;
      r = libusb_get_device_descriptor(dev, &desc);
      if (r < 0) {
         // Ignore with message
         fprintf(stdout, "Warning, nkusb_open_device: could not get device device descriptior."
                 " Ignoring.\n");
         continue;
      }

      if (desc.idVendor == vendor_id && desc.idProduct == product_id)  {


fprintf(stdout, "vme controller found!\n");
         

         r = libusb_open(dev, &devh);
         if (r < 0) {
            fprintf(stdout, "Warning, nkusb_open_device: could not open device."
                 " Ignoring.\n");
            continue;
         }

         // do not open twice
         if (is_device_open(devh)) {
         //if (nkusb_is_device_open(vendor_id, product_id, sid)) {
            fprintf(stdout, "Info, nkusb_open_device: device already open."
                 " Ignoring.\n");
            libusb_close(devh);
            continue;
         }

         // See if sid matches
         // Assume interface 0
         int interface = 0;
         if (libusb_claim_interface(devh, interface) < 0) {
            fprintf(stdout, "Warning, nkusb_open_device: could not claim interface 0 on the device."
                 " Ignoring.\n");
            libusb_close(devh);
            continue;
         }

         int sid_tmp = get_serial_id(devh);
         if (sid == NK_SID_ANY || sid == sid_tmp) {
            add_device(&ldev_open, devh, vendor_id, product_id, sid_tmp);
//         if (sid == NK_SID_ANY) {
//            add_device(&ldev_open, devh, vendor_id, product_id, NK_SID_ANY);
            nopen_devices++;
  
            // Print out the speed of just open device 
            int speed = libusb_get_device_speed(dev);
			status = 1;
            switch (speed) {
            case 4:
               fprintf(stdout, "Info: nkusb_open_device: super speed device opened");
            break;
            case 3:
               fprintf(stdout, "Info: nkusb_open_device: high speed device opened");
            break;
            case 2:
               fprintf(stdout, "Info: nkusb_open_device: full speed device opened");
            break;
            case 1:
               fprintf(stdout, "Info: nkusb_open_device: low speed device opened");
            break;
            case 0:
               fprintf(stdout, "Info: nkusb_open_device: unknown speed device opened");
            break;
            }
            fprintf(stdout, " (bus = %d, address = %d, serial id = %u).\n",
                    libusb_get_bus_number(dev), libusb_get_device_address(dev), sid_tmp);
//            fprintf(stdout, " (bus = %d, address = %d, serial id = %u).\n",
//                   libusb_get_bus_number(dev), libusb_get_device_address(dev), NK_SID_ANY);
            // Release interface
            libusb_release_interface(devh, interface);
            break;
         }
         else {
			status = 0;
            libusb_release_interface(devh, interface);
            libusb_close(devh);
            //devh = 0;
         }
      }
   }

   libusb_free_device_list(devs, 1);
   if (nopen_devices) return status;
   else return -1;
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_claim_interface(uint16_t vendor_id, uint16_t product_id, int sid,
                       int interface)
{
// Claim interface on the device with specified vendor id, product id and serial id
// on open device list. If sid == NK_SID_ANY, all devices with given vendor id
// and product id are handled.

   return handle_interface_id(&ldev_open, vendor_id, product_id, sid, interface, kInterfaceClaim);
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_release_interface(uint16_t vendor_id, uint16_t product_id, int sid,
                         int interface)
{
// Release interface on the device with specified vendor id, product id and serial id
// on open device list. If sid == NK_SID_ANY, all devices with given vendor id
// and product id are handled.

   return handle_interface_id(&ldev_open, vendor_id, product_id, sid, interface, kInterfaceRelease);
}

///////////////////////////////////////////////////////////////////////////////
void nkusb_close_device(uint16_t vendor_id, uint16_t product_id, int sid)
{
// Remove the device from open device list and close.
// If sid == NK_SID_ANY, remove all devices with specified vendor_id and product_id

   // remove (and close) from open device list
   remove_device_id(&ldev_open, vendor_id, product_id, sid);
   return;
}

///////////////////////////////////////////////////////////////////////////////
void nkusb_exit(libusb_context *ctx)
{
   libusb_exit(ctx); 
}
