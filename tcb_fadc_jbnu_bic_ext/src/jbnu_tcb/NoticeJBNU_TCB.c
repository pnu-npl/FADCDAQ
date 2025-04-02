#include "NoticeJBNU_TCB.h"

enum EManipInterface {kInterfaceClaim, kInterfaceRelease};

struct dev_open {
   libusb_device_handle *devh;
   uint16_t vendor_id;
   uint16_t product_id;
   int serial_id;
   struct dev_open *next;
} *ldev_open = 0;

// internal functions *********************************************************************************
static int is_device_open(libusb_device_handle *devh);
static unsigned char get_serial_id(libusb_device_handle *devh);
static void add_device(struct dev_open **list, libusb_device_handle *tobeadded, int sid);
static int handle_interface_id(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type);
static void remove_device_id(struct dev_open **list, int sid);
libusb_device_handle* nkusb_get_device_handle(int sid);
int TCBWrite(int sid, int mid, int addr, int data);
int TCBRead(int sid, int mid, int addr);
void TCBstart_DRAM(int sid, int mid);
void TCBstop_DRAM(int sid, int mid);
int TCBread_DRAM_READY(int sid, int mid);
void TCBsetup_ADC(int sid, int mid, int addr, int data);
int TCBread_ADC_ALIGN(int sid, int mid, int ch);
void TCBwrite_DRAM_TEST(int sid, int mid, int data);
int TCBread_DRAM_ALIGN(int sid, int mid, int ch);
void TCBreset_REF_CLK(int sid, int mid);
void TCBreset_ADC(int sid, int mid);
void TCBwrite_ADC_IDLY(int sid, int mid, int ch, int data);
void TCBwrite_ADC_BITSLIP(int sid, int mid, int ch);
void TCBwrite_ADC_IMUX(int sid, int mid, int ch, int data);
void TCBwrite_DRAM_IDLY(int sid, int mid, int ch, int data);
void TCBwrite_DRAM_BITSLIP(int sid, int mid, int ch);

static int is_device_open(libusb_device_handle *devh)
{
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

static unsigned char get_serial_id(libusb_device_handle *devh)
{
  int ret;
  if (!devh) {
    return 0;
  }
  unsigned char data[1];
  ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, 0xD2, 0, 0, data, 1, 1000);

  if (ret < 0) {
    fprintf(stdout, "Warning: get_serial_id: Could not get serial id.\n");
    return 0;
  }

  return data[0];
}

static void add_device(struct dev_open **list, libusb_device_handle *tobeadded, int sid)
{
  struct dev_open *curr;

  curr = (struct dev_open *)malloc(sizeof(struct dev_open));
  curr->devh = tobeadded;
  curr->vendor_id = JBNU_TCB_VID;
  curr->product_id = JBNU_TCB_PID;
  curr->serial_id = sid;
  curr->next  = *list;
  *list = curr;
}

static int handle_interface_id(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type)
{
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
      fprintf(stdout, "Warning: remove_device: could not get device device descriptior."
                          " Ignoring.\n");
      continue;
    }
    if (desc.idVendor == JBNU_TCB_VID && desc.idProduct == JBNU_TCB_PID
    && (sid == 0xFF || sid == get_serial_id(curr->devh))) { 
      if (manip_type == kInterfaceClaim) {
        if ((ret = libusb_claim_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not claim interface (%d) on device (%u, %u, %u)\n",
                  interface, JBNU_TCB_VID, JBNU_TCB_PID, sid);
        }
      }
      else if (manip_type == kInterfaceRelease) {
        if ((ret =libusb_release_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not release interface (%d) on device (%u, %u, %u)\n",
                  interface, JBNU_TCB_VID, JBNU_TCB_PID, sid);
        }
      }
      else {
        fprintf(stderr, "Error: handle_interface_id: Unknown interface handle request: %d\n.",
                manip_type);
              
        ret = -1;
        return ret;
      }
    }

    curr = curr->next;
  }

  return ret;
}

static void remove_device_id(struct dev_open **list, int sid)
{
  if (!*list) return;

  struct dev_open *curr = *list;
  struct dev_open *prev = 0;
  struct libusb_device_descriptor desc;
  libusb_device *dev;

  while (curr) {
    dev =libusb_get_device(curr->devh);
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
      fprintf(stdout, "Warning, remove_device: could not get device device descriptior." " Ignoring.\n");
      continue;
    }
    if (desc.idVendor == JBNU_TCB_VID && desc.idProduct == JBNU_TCB_PID
    && (sid == 0xFF || sid == get_serial_id(curr->devh))) { 
      if (*list == curr) { 
        *list = curr->next;
        libusb_close(curr->devh);
        free(curr); 
        curr = *list;
      }
      else {
        prev->next = curr->next;
        libusb_close(curr->devh);
        free(curr); 
        curr = prev->next;
      }
    }
    else {
      prev = curr;
      curr = curr->next;
    }    
  }
}

libusb_device_handle* nkusb_get_device_handle(int sid) 
{
  struct dev_open *curr = ldev_open;
  while (curr) {
    if (curr->vendor_id == JBNU_TCB_VID && curr->product_id == JBNU_TCB_PID) {
      if (sid == 0xFF)
        return curr->devh;
      else if (curr->serial_id == sid)
        return curr->devh;
    }

    curr = curr->next;
  }

  return 0;
}

int TCBWrite(int sid, int mid, int addr, int data)
{
  int transferred = 0;  
  const unsigned int timeout = 1000;
  int length = 12;
  unsigned char *buffer;
  int stat = 0;
  
  if (!(buffer = (unsigned char *)malloc(length))) {
    fprintf(stderr, "TCBWrite: Could not allocate memory (size = %d\n)", length);
    return -1;
  }
  
  buffer[0] = data & 0xFF;
  buffer[1] = (data >> 8)  & 0xFF;
  buffer[2] = (data >> 16)  & 0xFF;
  buffer[3] = (data >> 24)  & 0xFF;
  
  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8)  & 0xFF;
  buffer[6] = (addr >> 16)  & 0xFF;
  buffer[7] = (addr >> 24)  & 0x7F;

  buffer[8] = mid & 0xFF;
  buffer[9] = (mid >> 8) & 0xFF;
  buffer[10] = (mid >> 16) & 0xFF;
  buffer[11] = (mid >> 24) & 0xFF;
  
  libusb_device_handle *devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "TCBWrite: Could not get device handle for the device.\n");
    return 0;
  }
  
  if ((stat = libusb_bulk_transfer(devh, 0x06, buffer, length, &transferred, timeout)) < 0) {
    fprintf(stderr, "TCBWrite: Could not make write request; error = %d\n", stat);
    free(buffer);
    return 0;
  }
  
  free(buffer);

  usleep(1000);

  return stat;
}

int TCBRead(int sid, int mid, int addr)
{
  const unsigned int timeout = 1000; // Wait forever
  int length = 12;
  int transferred;
  unsigned char *buffer;
  int stat = 0;
  int tmp;
  int data;

  if (!(buffer = (unsigned char *)malloc(length))) {
    fprintf(stderr, "TCBRead: Could not allocate memory (size = %d\n)", length);
    return -1;
  }
  
  buffer[0] = 1;
  buffer[1] = 0;
  buffer[2] = 0;
  buffer[3] = 0;
  
  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8)  & 0xFF;
  buffer[6] = (addr >> 16)  & 0xFF;
  buffer[7] = (addr >> 24)  & 0x7F;
  buffer[7] = buffer[7] | 0x80;

  buffer[8] = mid & 0xFF;
  buffer[9] = (mid >> 8) & 0xFF;
  buffer[10] = (mid >> 16) & 0xFF;
  buffer[11] = (mid >> 24) & 0xFF;

  libusb_device_handle *devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "TCBRead: Could not get device handle for the device.\n");
    return 0;
  }

  if ((stat = libusb_bulk_transfer(devh, 0x06, buffer, length, &transferred, timeout)) < 0) {
    fprintf(stderr, "TCBRead: Could not make write request; error = %d\n", stat);
    free(buffer);
    return 0;
  }

  if ((stat = libusb_bulk_transfer(devh, 0x82, buffer, 4, &transferred, timeout)) < 0) {
      fprintf(stderr, "TCBRead: Could not make read request; error = %d\n", stat);
      return 0;
  }

  data = buffer[0] & 0xFF;
  tmp = buffer[1] & 0xFF;
  tmp = tmp << 8;
  data = data + tmp;
  tmp = buffer[2] & 0xFF;
  tmp = tmp << 16;
  data = data + tmp;
  tmp = buffer[3] & 0xFF;
  tmp = tmp << 24;
  data = data + tmp;

  free(buffer);
  
  return data;
}

// ******************************************************************************************************

// initialize libusb library
void USB3Init(void)
{
  if (libusb_init(0) < 0) {
    fprintf(stderr, "Failed to initialise LIBUSB\n");
    exit(1);
  }
}

// de-initialize libusb library
void USB3Exit(void)
{
  libusb_exit(0); 
}

// open JBNU_TCB
int TCBopen(int sid)
{
  struct libusb_device **devs;
  struct libusb_device *dev;
  struct libusb_device_handle *devh;
  size_t i = 0;
  int nopen_devices = 0; //number of open devices
  int r;
  int interface = 0;
  int sid_tmp;
  int speed;

  if (libusb_get_device_list(0, &devs) < 0) 
    fprintf(stderr, "Error: open_device: Could not get device list\n");

  fprintf(stdout, "Info: open_device: opening device Vendor ID = 0x%X, Product ID = 0x%X, Serial ID = %u\n",
                   JBNU_TCB_VID, JBNU_TCB_PID, sid);

  while ((dev = devs[i++])) {
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      fprintf(stdout, "Warning, open_device: could not get device device descriptior." " Ignoring.\n");
      continue;
    }

    if (desc.idVendor == JBNU_TCB_VID && desc.idProduct == JBNU_TCB_PID)  {
      r = libusb_open(dev, &devh);
      if (r < 0) {
        fprintf(stdout, "Warning, open_device: could not open device." " Ignoring.\n");
        continue;
      }

      if (is_device_open(devh)) {
        fprintf(stdout, "Info, open_device: device already open." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      if (libusb_claim_interface(devh, interface) < 0) {
        fprintf(stdout, "Warning, open_device: could not claim interface 0 on the device." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      sid_tmp = get_serial_id(devh);

      if (sid == 0xFF || sid == sid_tmp) {
        add_device(&ldev_open, devh, sid_tmp);
        nopen_devices++;
  
        // Print out the speed of just open device 
        speed = libusb_get_device_speed(dev);
        switch (speed) {
          case 4:
            fprintf(stdout, "Info: open_device: super speed device opened");
            break;
          case 3:
            fprintf(stdout, "Info: open_device: high speed device opened");
            break;
          case 2:
            fprintf(stdout, "Info: open_device: full speed device opened");
            break;
          case 1:
            fprintf(stdout, "Info: open_device: low speed device opened");
            break;
          case 0:
            fprintf(stdout, "Info: open_device: unknown speed device opened");
            break;
        }
        
        fprintf(stdout, " (bus = %d, address = %d, serial id = %u).\n",
                    libusb_get_bus_number(dev), libusb_get_device_address(dev), sid_tmp);
        libusb_release_interface(devh, interface);
        break;
      }
      else {
        libusb_release_interface(devh, interface);
        libusb_close(devh);
      }
    }
  }

  libusb_free_device_list(devs, 1);

  // claim interface
  handle_interface_id(&ldev_open, sid, 0, kInterfaceClaim);

  if (!nopen_devices)
    return -1;

  devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "Could not get device handle for the device.\n");
    return -1;
  }

  return 0;
}

// close TCB
void TCBclose(int sid)
{
  handle_interface_id(&ldev_open, sid, 0, kInterfaceRelease);
  remove_device_id(&ldev_open, sid);
}

// reset timer 
void TCBreset_TIMER(int sid)
{
  TCBWrite(sid, 0, 0x20000000, 0x01);
}

// reset 
void TCBreset(int sid)
{
  TCBWrite(sid, 0, 0x20000000, 0x04);
}

// start DAQ
void TCBstart_DAQ(int sid)
{
  TCBWrite(sid, 0, 0x20000000, 0x08);
}

// read DAQ status
int TCBread_RUN(int sid)
{
  return TCBRead(sid, 0, 0x20000000);
}

// write run number
void TCBwrite_RUN_NUMBER(int sid, int data)
{
  TCBWrite(sid, 0, 0x20000001, data);
}

// read run number
int TCBread_RUN_NUMBER(int sid)
{
  return TCBRead(sid, 0, 0x20000001);
}

// start DRAM
void TCBstart_DRAM(int sid, int mid)
{
  TCBWrite(sid, mid, 0x20000001, 1);
}

// stop DRAM
void TCBstop_DRAM(int sid, int mid)
{
  TCBWrite(sid, mid, 0x20000001, 0);
}

// read DRAM ready
int TCBread_DRAM_READY(int sid, int mid)
{
  return TCBRead(sid, mid, 0x20000001);
}

// write acquisition time
void TCBwrite_ACQ_TIME(int sid, int data)
{
  TCBWrite(sid, 0, 0x20000002, data);
}

// read acquisition time
int TCBread_ACQ_TIME(int sid)
{
  return TCBRead(sid, 0, 0x20000002);
}

// write High voltage
void TCBwrite_HV(int sid, int mid, int ch, float data)
{
  float fval;
  int value;
  int addr;
  int target_val;
  int present_val;
  int diff_val;
  
  addr = (ch - 1) * 0x10000 + 0x20000002;
  fval = 4.63 * (data - 4.5);
  target_val = (int)(fval);
  if (target_val > 254)
    target_val = 254;
  else if (target_val < 0)
    target_val = 0;
    
  present_val = TCBRead(sid, mid, addr);
  diff_val = target_val - present_val;
  value = present_val;

  if (diff_val > 0) {
    while (value < target_val) {
      value = value + 20;
      if (value > target_val)
        value = target_val;
      TCBWrite(sid, mid, addr, value);
      usleep(100000);
    }
  }
  else {
    while (value > target_val) {
      value = value - 20;
      if (value < target_val)
        value = target_val;
      TCBWrite(sid, mid, addr, value);
      usleep(100000);
    }
  }
}

// read high voltage
float TCBread_HV(int sid, int mid, int ch)
{
  int data;
  float value;
  int addr;

  addr = (ch - 1) * 0x10000 + 0x20000002;
  data = TCBRead(sid, mid, addr);
  value = data;
  value = value / 4.63 + 4.5;

  return value;
}

// write coincidence width
void TCBwrite_COIN_WIDTH(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x20000003, data);
}

// read coincidence width
int TCBread_COIN_WIDTH(int sid, int mid)
{
  return TCBRead(sid, mid, 0x20000003);
}

// write multiplicity threshold
void TCBwrite_MULTI_THR(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x20000004, data);
}

// read multiplicity threshold
int TCBread_MULTI_THR(int sid, int mid)
{
  return TCBRead(sid, mid, 0x20000004);
}

// write pedestal trigger interval, 0 ~ 65535 ms, when 0 disabled
void TCBwrite_PEDESTAL_TRIGGER_INTERVAL(int sid, int data)
{
  TCBWrite(sid, 0, 0x20000005, data);
}

// read pedestal trigger interval
int TCBread_PEDESTAL_TRIGGER_INTERVAL(int sid)
{
  return TCBRead(sid, 0, 0x20000005);
}

// write discriminator threshold
void TCBwrite_DISC_THR(int sid, int mid, int ch, int data)
{
  int addr;
  
  addr = (ch - 1) * 0x10000 + 0x20000005;
  TCBWrite(sid, mid, addr, data);
}

// read discriminator threshold
int TCBread_DISC_THR(int sid, int mid, int ch)
{
  int addr;
  
  addr = (ch - 1) * 0x10000 + 0x20000005;
  return TCBRead(sid, mid, addr);
}

// send software trigger
void TCBsend_TRIG(int sid)
{
  TCBWrite(sid, 0, 0x20000006, 0);
}

// write gate width
void TCBwrite_GATE_WIDTH(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x20000006, data);
}

// read gate width
int TCBread_GATE_WIDTH(int sid, int mid)
{
  return TCBRead(sid, mid, 0x20000006);
}

// write trigger enable, self = 1, pedestal = 2, software = 4, external = 8
void TCBwrite_TRIGGER_ENABLE(int sid, int data)
{
  TCBWrite(sid, 0, 0x20000007, data);
}

// read trigger enable
int TCBread_TRIGGER_ENABLE(int sid)
{
  return TCBRead(sid, 0, 0x20000007);
}

// write input delay
void TCBwrite_DELAY(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x20000007, data);
}

// read input deay
int TCBread_DELAY(int sid, int mid)
{
  return TCBRead(sid, mid, 0x20000007);
}

// measure pedestal
void TCBmeasure_PED(int sid, int mid)
{
  TCBWrite(sid, mid, 0x20000008, 0);
}

// read pedestal
int TCBread_PED(int sid, int mid, int ch)
{
  int addr;
  
  addr = (ch - 1) * 0x10000 + 0x20000008;
  return TCBRead(sid, mid, addr);
}

// write peak sum width
void TCBwrite_PEAK_SUM_WIDTH(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x20000009, data);
}

// read peak sum width
int TCBread_PEAK_SUM_WIDTH(int sid, int mid)
{
  return TCBRead(sid, mid, 0x20000009);
}

// write zero suppression
void TCBwrite_ZERO_SUP(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x2000000A, data);
}

// read zero suppression
int TCBread_ZERO_SUP(int sid, int mid)
{
  return TCBRead(sid, mid, 0x2000000A);
}

// write waveform prescale
void TCBwrite_PRESCALE(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x2000000B, data);
}

// read waveform prescale
int TCBread_PRESCALE(int sid, int mid)
{
  return TCBRead(sid, mid, 0x2000000B);
}

// read temperature
void TCBread_TEMP(int sid, int mid, float *temp)
{
  int data;
  float fval;

  TCBWrite(sid, mid, 0x2000000C, 0);

  data = TCBRead(sid, mid, 0x2000000C);
  fval = data;
  fval = fval * 0.04 - 20.0;
  temp[0] = fval;

  data = TCBRead(sid, mid, 0x2001000C);
  fval = data;
  fval = fval * 0.04 - 20.0;
  temp[1] = fval;
}

// write echo register
void TCBwrite_ECHO(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x2000000D, data);
}

// read echo register
int TCBread_ECHO(int sid, int mid)
{
  return TCBRead(sid, mid, 0x2000000D);
}

void TCBsetup_ADC(int sid, int mid, int addr, int data)
{
  int value;
  value = ((addr & 0xFFFF) << 16) | (data & 0xFFFF);
  TCBWrite(sid, mid, 0x2000000E, value);
}

int TCBread_ADC_ALIGN(int sid, int mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x2000000E;
  return TCBRead(sid, mid, addr);
}

// write DRAM test mode
void TCBwrite_DRAM_TEST(int sid, int mid, int data)
{
  TCBWrite(sid, mid, 0x2000000F, data);
}

// read DRAM alignment pattern
int TCBread_DRAM_ALIGN(int sid, int mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x2000000F;
  return TCBRead(sid, mid, addr);
}

// read DAQ link status
void TCBread_LINK_STATUS(int sid, int *data)
{
  data[0] = TCBRead(sid, 0, 0x20000010);
  data[1] = TCBRead(sid, 0, 0x20000011);
}

// read DAQ mid
void TCBread_MID(int sid, int *data)
{
  int i;

  for (i = 0; i < 40; i ++) 
    data[i] = TCBRead(sid, 0, 0x20000012 + i);
}

// reset delay reference clock
void TCBreset_REF_CLK(int sid, int mid)
{
  TCBWrite(sid, mid, 0x20000020, 0);
}

// reset ADC
void TCBreset_ADC(int sid, int mid)
{
  TCBWrite(sid, mid, 0x20000021, 0);
}

void TCBwrite_ADC_IDLY(int sid, int mid, int ch, int data)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000022;
  TCBWrite(sid, mid, addr, data);
}

void TCBwrite_ADC_BITSLIP(int sid, int mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000023;
  TCBWrite(sid, mid, addr, 0);
}

void TCBwrite_ADC_IMUX(int sid, int mid, int ch, int data)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000024;
  TCBWrite(sid, mid, addr, data);
}

void TCBwrite_DRAM_IDLY(int sid, int mid, int ch, int data)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000025;
  TCBWrite(sid, mid, addr, data);
}

void TCBwrite_DRAM_BITSLIP(int sid, int mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000026;
  TCBWrite(sid, mid, addr, 0);
}

void TCBinit_ADC(int sid, int mid)
{
  int retry;
  int all_aligned;
  int ch;
  int dly;
  int value;
  int flag;
  int count;
  int sum;
  int gdly;
  int bitslip;
  int aligned;

  // reset input delay ref clock
  TCBreset_REF_CLK(sid, mid); 

  // reset ADC
  TCBreset_ADC(sid, mid);

  // set ADC registers
  TCBsetup_ADC(sid, mid, 0x00, 0x0000);
  TCBsetup_ADC(sid, mid, 0x01, 0x0010);
  TCBsetup_ADC(sid, mid, 0xD1, 0x0240);
  TCBsetup_ADC(sid, mid, 0x02, 0x0000);
  TCBsetup_ADC(sid, mid, 0x0F, 0x0000);
  TCBsetup_ADC(sid, mid, 0x14, 0x0000);
  TCBsetup_ADC(sid, mid, 0x1C, 0x0000);
  TCBsetup_ADC(sid, mid, 0x24, 0x0000);
  TCBsetup_ADC(sid, mid, 0x28, 0x8100);
  TCBsetup_ADC(sid, mid, 0x29, 0x0000);
  TCBsetup_ADC(sid, mid, 0x2A, 0x0000);
  TCBsetup_ADC(sid, mid, 0x2B, 0x0000);
  TCBsetup_ADC(sid, mid, 0x38, 0x0000);
  TCBsetup_ADC(sid, mid, 0x42, 0x0000);
  TCBsetup_ADC(sid, mid, 0x45, 0x0000);
  TCBsetup_ADC(sid, mid, 0x46, 0x8801);

//  for (retry = 0; retry < 2; retry++) {
  for (retry = 0; retry < 1; retry++) {
    all_aligned = 0;

    for (ch = 0; ch < 4; ch++) {
      count = 0;
      sum = 0;
      flag = 0;
      aligned = 0;

      // set test pattern
      TCBsetup_ADC(sid, mid, 0x26, 0x3330);
      TCBsetup_ADC(sid, mid, 0x25, 0x0013);
      
      // search delay
      for (dly = 0; dly < 32; dly++) {
        // set delay
        TCBwrite_ADC_IDLY(sid, mid, ch, dly);

        // read bit_alignment status
        value = TCBread_ADC_ALIGN(sid, mid, ch) & 0x1;

        if(value) {
          count = count + 1;
          sum = sum + dly;
          if(count > 12) 
            flag = 1;
        }
        else{
          if (flag) 
            dly = 32;
          else {
            count = 0;
            sum = 0;
          }
        }
      }
        
      // get good center
      if (count > 12) {
        gdly = sum / count;
        aligned = 1;
      }
      else
        gdly = 0;
    
      // set good delay
      TCBwrite_ADC_IDLY(sid, mid, ch, gdly);

      // read word sync test pattern
      TCBsetup_ADC(sid, mid, 0x26, 0x0000);
      TCBsetup_ADC(sid, mid, 0x25, 0x0012);

      // get bitslip
      TCBwrite_ADC_IMUX(sid, mid, ch, 0);
      for (bitslip = 0; bitslip < 8; bitslip++) {
        if (bitslip == 4)
          TCBwrite_ADC_IMUX(sid, mid, ch, 1);
            
        value = TCBread_ADC_ALIGN(sid, mid, ch) & 0x2;

        if (value) {
          aligned = aligned + 1;
          bitslip = 8;
        }
        else 
          TCBwrite_ADC_BITSLIP(sid, mid, ch);
      }
      
      all_aligned = all_aligned + aligned;
    }

    if (all_aligned == 8) 
      retry = 2;
  }
        
  // set ADC in normal output mode
  TCBsetup_ADC(sid, mid, 0x25, 0x0000);

  if (all_aligned == 8)
    printf("mid = %d : ADC is aligned.\n", mid);
  else
    printf("mid = %d : Fail to align ADC!\n", mid);
}

void TCBinit_DRAM(int sid, int mid)
{
  int ready;
  int retry;
  int all_aligned;
  int ch;
  int dly;
  int value;
  int flag;
  int count;
  int sum;
  int aflag;
  int gdly;
  int bitslip;
  int aligned;
  
  ready = TCBread_DRAM_READY(sid, mid);
  
  if (ready)
    TCBstop_DRAM(sid, mid);

  TCBstart_DRAM(sid, mid);
  
  ready = 0;
  while (!ready)
    ready = TCBread_DRAM_READY(sid, mid);
  
  // turn on DRAM    
  TCBstart_DRAM(sid, mid);
  
  // enter DRAM test mode
  TCBwrite_DRAM_TEST(sid, mid, 1); 

  for (retry = 0; retry < 2; retry++) {
    all_aligned = 0;

    // send reset to iodelay  
    TCBreset_REF_CLK(sid, mid); 

    // fill DRAM test pattern
    TCBwrite_DRAM_TEST(sid, mid, 2); 

    for (ch = 0; ch < 2; ch++) {
      count = 0;
      sum = 0;
      flag = 0;
      aligned = 0;

      // search delay
      for (dly = 0; dly < 32; dly++) {
        // set delay
        TCBwrite_DRAM_IDLY(sid, mid, ch, dly);

        // read DRAM test pattern
        TCBwrite_DRAM_TEST(sid, mid, 3); 
        value = TCBread_DRAM_ALIGN(sid, mid, ch); 

        aflag = 0;
        if (value == 0xFFAA5500)
          aflag = 1;
        else if (value == 0xAA5500FF)
          aflag = 1;
        else if (value == 0x5500FFAA)
          aflag = 1;
        else if (value == 0x00FFAA55)
          aflag = 1;
    
        if (aflag) {
          count = count + 1;
          sum = sum + dly;
          if (count > 8)
            flag = 1; 
        }
        else {
          if (flag) {
            dly = 32;
            aligned = 1;
          }
          else {
            count = 0;
            sum = 0;
          }
        }
      }

      // get good delay center
      if (count)
        gdly = sum / count;
      else
        gdly = 9;

      // set delay
      TCBwrite_DRAM_IDLY(sid, mid, ch, gdly);
  
      // get bitslip
      for (bitslip = 0; bitslip < 4; bitslip++) {
        // read DRAM test pattern
        TCBwrite_DRAM_TEST(sid, mid, 3); 
        value = TCBread_DRAM_ALIGN(sid, mid, ch); 

        if (value == 0xFFAA5500) {
          aligned = aligned + 1;
          bitslip = 4;
        }
        else 
          TCBwrite_DRAM_BITSLIP(sid, mid, ch);
      }
      
      all_aligned = all_aligned + aligned;
    }
    
    if (all_aligned == 4) 
      retry = 2;
  }

  if (all_aligned == 4)
    printf("mid = %d : DRAM is aligned.\n", mid);
  else
    printf("mid = %d : Fail to align DRAM!\n", mid);
   
  // exit DRAM test mode
  TCBwrite_DRAM_TEST(sid, mid, 0); 
}


