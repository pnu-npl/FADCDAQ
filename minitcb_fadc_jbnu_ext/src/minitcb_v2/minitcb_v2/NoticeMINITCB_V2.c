#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include <libusb.h>

#include "NoticeMINITCB_V2.h"

// open MINITCB_V2
int MINITCB_V2open(char *ip)
{
  struct sockaddr_in serv_addr;
  int tcp_Handle;
  const int disable = 1;
        
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port        = htons(5000);
        
  if ( (tcp_Handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open MINITCB_V2\n");
    return -1;
  }

  setsockopt(tcp_Handle, IPPROTO_TCP,TCP_NODELAY,(char *) &disable, sizeof(disable)); 

  if (connect(tcp_Handle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("client: can't connect to server\n");
    printf("ip %s , port 5000 \n", ip);
    printf("error number is %d \n", connect(tcp_Handle, (struct sockaddr *) &serv_addr,sizeof(serv_addr)));

    return -2;
  } 
  
  return tcp_Handle;
}

// close MINITCB_V2
void MINITCB_V2close(int tcp_Handle)
{
  close(tcp_Handle);
}

// transmit characters to MINITCB_V2
int MINITCB_V2transmit(int tcp_Handle, char *buf, int len)
{
  int result;
  int bytes_more;
  int  bytes_xferd;
  char *idxPtr;

  bytes_more = len;
  idxPtr = buf;
  bytes_xferd = 0;
  while (1) {
    idxPtr = buf + bytes_xferd;
    result=write (tcp_Handle, (char *) idxPtr, bytes_more);

    if (result<0) {
      printf("Could not write the rest of the block successfully, returned: %d\n",bytes_more);
      return -1;
    }
    
    bytes_xferd += result;
    bytes_more -= result;
    
    if (bytes_more <= 0)
      break;
  }

  return 0;
}

// receive characters from MINITCB_V2
int MINITCB_V2receive(int tcp_Handle, char *buf, int len)
{
  int result;
  int accum;
  int space_left;
  int bytes_more;
  int buf_count;
  char *idxPtr;

  fd_set rfds;
  struct timeval tval;

  tval.tv_sec = MAX_TCP_READ;
  tval.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(tcp_Handle, &rfds);

  if (buf==NULL)
    return -1;

  idxPtr = buf;

  buf_count = 0;
  space_left = len;
  while (1) {
    accum = 0;
    while (1) {
      idxPtr = buf + (buf_count + accum);
      bytes_more = space_left;
      
      if ((result = read(tcp_Handle, (char *) idxPtr, (bytes_more>2048)?2048:bytes_more)) < 0) {
        printf("Unable to receive data from the server.\n");
        return -1;
      }
      
      accum += result;
      if ((accum + buf_count) >= len)
	break;

      if(result<bytes_more) {
        printf("wanted %d got %d \n",bytes_more,result);
        return accum+buf_count;
      }
    }
    
    buf_count += accum;
    space_left -= accum;

    if (space_left <= 0)
      break;
  }

  return buf_count;
}

// write to MINITCB_V2
void MINITCB_V2write(int tcp_Handle, unsigned long address, unsigned long data)
{
  char tcpBuf[3];

  tcpBuf[0] = 11;
  tcpBuf[1] = address & 0xFF;
  tcpBuf[2] = data & 0xFF;

  MINITCB_V2transmit(tcp_Handle, tcpBuf, 3);
  
  MINITCB_V2receive(tcp_Handle, tcpBuf, 1);
}

// read from MINITCB_V2
unsigned long MINITCB_V2read(int tcp_Handle, unsigned long address)
{
  char tcpBuf[2];
  unsigned long data;

  tcpBuf[0] = 12;
  tcpBuf[1] = address & 0xFF;

  MINITCB_V2transmit(tcp_Handle, tcpBuf, 2);
  
  MINITCB_V2receive(tcp_Handle, tcpBuf, 1);

  data = tcpBuf[0] & 0xFF;

  return data;
}

// write to module
void MINITCB_V2write_Module(int tcp_Handle, unsigned long mid, unsigned long address, unsigned long data)
{
  // set mid
  MINITCB_V2write(tcp_Handle, 0x17, mid);

  // set address
  MINITCB_V2write(tcp_Handle, 0x18, address & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x19, (address >> 8) & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x1A, (address >> 16) & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x1B, (address >> 24) & 0xFF);

  // set data
  MINITCB_V2write(tcp_Handle, 0x1C, data & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x1D, (data >> 8) & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x1E, (data >> 16) & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x1F, (data >> 24) & 0xFF);

  // send command
  MINITCB_V2write(tcp_Handle, 0x15, 0);
}

// read from module
unsigned long MINITCB_V2read_Module(int tcp_Handle, unsigned long mid, unsigned long address)
{
  unsigned long data;
  unsigned long value;

  // set mid
  MINITCB_V2write(tcp_Handle, 0x17, mid);

  // set address
  MINITCB_V2write(tcp_Handle, 0x18, address & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x19, (address >> 8) & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x1A, (address >> 16) & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x1B, (address >> 24) & 0xFF);

  // send command
  MINITCB_V2write(tcp_Handle, 0x16, 0);

  // get data
  data = MINITCB_V2read(tcp_Handle, 0x1C);
  value = MINITCB_V2read(tcp_Handle, 0x1D);
  data = data + (value << 8);
  value = MINITCB_V2read(tcp_Handle, 0x1E);
  data = data + (value << 16);
  value = MINITCB_V2read(tcp_Handle, 0x1F);
  data = data + (value << 24);

  return data;
}

// reset data acquisition
void MINITCB_V2reset(int tcp_Handle)
{
  MINITCB_V2write(tcp_Handle, 0x00, 4);
}

// reset timer
void MINITCB_V2resetTIMER(int tcp_Handle)
{
  MINITCB_V2write(tcp_Handle, 0x00, 1);
}

// start data acquisition
void MINITCB_V2start(int tcp_Handle)
{
  MINITCB_V2write(tcp_Handle, 0x00, 8);
}

// stop data acquisition
void MINITCB_V2stop(int tcp_Handle)
{
  MINITCB_V2write(tcp_Handle, 0x00, 0);
}

// read RUN status
unsigned long MINITCB_V2read_RUN(int tcp_Handle, unsigned long mid)
{
  unsigned long data;

  if (mid)
    data = MINITCB_V2read_Module(tcp_Handle, mid, 0x20000000);
  else
    data = MINITCB_V2read(tcp_Handle, 0x00);

  return data;
}

// write coincidence window
void MINITCB_V2write_CW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;;

  if(mid > 0) {
    addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);
    MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
  }
  else {
    MINITCB_V2write(tcp_Handle, 0x02, data & 0xFF);
    MINITCB_V2write(tcp_Handle, 0x03, (data >> 8) & 0xFF);
  }
}

// read coincidence window
unsigned long MINITCB_V2read_CW(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  unsigned long temp;

  if(mid > 0) {
    addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);
    data = MINITCB_V2read_Module(tcp_Handle, mid, addr);
  }
  else {
    data = MINITCB_V2read(tcp_Handle, 0x02);
    temp = MINITCB_V2read(tcp_Handle, 0x03);
    data = data + (temp << 8);
  }

  return data;
}

// write segment setting 
void MINITCB_V2write_RL(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000002, data);
}

// read segment setting 
unsigned long MINITCB_V2read_RL(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000002);
}

// turn on/off DRAM
// 0 = off, 1 = on
void MINITCB_V2write_DRAMON(int tcp_Handle, unsigned long mid, unsigned long data)
{
  unsigned long status;

  // turn on DRAM
  if (data) {
    // check DRAM is on
    status = MINITCB_V2read_Module(tcp_Handle, mid, 0x20000003);

    // when DRAM is on now, turn it off
    if (status)
      MINITCB_V2write_Module(tcp_Handle, mid, 0x20000003, 0);

    // turn on DRAM
    MINITCB_V2write_Module(tcp_Handle, mid, 0x20000003, 1);

    // wait for DRAM ready
    status = 0;
    while (!status)
      status = MINITCB_V2read_Module(tcp_Handle, mid, 0x20000003);
  }
  // trun off DRAM
  else 
    MINITCB_V2write_Module(tcp_Handle, mid, 0x20000003, 0);
}

// read DRAM status
unsigned long MINITCB_V2read_DRAMON(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000003);
}

// write offset adjustment
void MINITCB_V2write_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
  sleep(1);
}

// read offset adjustment
unsigned long MINITCB_V2read_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// measure pedestal
void MINITCB_V2measure_PED(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000005 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, 0);
}

// read pedestal
unsigned long MINITCB_V2read_PED(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000006 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write input delay
void MINITCB_V2write_DLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  unsigned long value;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  value = ((data / 1000) << 10) | (data % 1000);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, value);
}

// read input delay
unsigned long MINITCB_V2read_DLY(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  unsigned long value;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  value = MINITCB_V2read_Module(tcp_Handle, mid, addr);

  data = (value >> 10)*1000 + (value&0X3FF);
  
  return data;
}

// write input delay
void MINITCB_V2write_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read input delay
unsigned long MINITCB_V2read_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  data = MINITCB_V2read_Module(tcp_Handle, mid, addr);
  
  return data;
}

// write discriminator threshold
void MINITCB_V2write_THR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read discriminator threshold
unsigned long MINITCB_V2read_THR(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write input pulse polarity
void MINITCB_V2write_POL(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read input pulse polarity
unsigned long MINITCB_V2read_POL(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write pulse sum trigger width
void MINITCB_V2write_PSW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read pulse sum trigger width
unsigned long MINITCB_V2read_PSW(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write ADC mode
void MINITCB_V2write_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000B + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read ADC mode
unsigned long MINITCB_V2read_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000B + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write pulse count threshold ; TFADC500
void MINITCB_V2write_PCT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read pulse count threshold ; TFADC500
unsigned long MINITCB_V2read_PCT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write pulse count interval
void MINITCB_V2write_PCI(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read pulse count interval
unsigned long MINITCB_V2read_PCI(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write pulse width threshold
void MINITCB_V2write_PWT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read pulse width threshold
unsigned long MINITCB_V2read_PWT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write deadtime
void MINITCB_V2write_DT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;;

  if(mid > 0) {
    addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);
    MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
  }
  else {
    MINITCB_V2write(tcp_Handle, 0x01, data & 0xFF);
    MINITCB_V2write(tcp_Handle, 0x0B, (data >> 8) & 0xFF);
    MINITCB_V2write(tcp_Handle, 0x0D, (data >> 16) & 0xFF);
  }
}

// read deadtime
unsigned long MINITCB_V2read_DT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  unsigned long temp;

  if(mid > 0) {
    addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);
    data = MINITCB_V2read_Module(tcp_Handle, mid, addr);
  }
  else {
    data = MINITCB_V2read(tcp_Handle, 0x01);
    temp = MINITCB_V2read(tcp_Handle, 0x0B);
    data = data + (temp << 8);
    temp = MINITCB_V2read(tcp_Handle, 0x0D);
    data = data + (temp << 16);
  }

  return data;
}

// write trigger mode
void MINITCB_V2write_TM(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read trigger mode
unsigned long MINITCB_V2read_TM(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write trigger lookup table
void MINITCB_V2write_TLT(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000015, data);
}

// read trigger lookup table
unsigned long MINITCB_V2read_TLT(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000015);
}

// write trigger lookup table for M64ADC only
void MINITCB_V2write_STLT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000015 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read trigger lookup table for M64ADC only
unsigned long MINITCB_V2read_STLT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000015 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write zero suppression
void MINITCB_V2write_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000016 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read zero suppression
unsigned long MINITCB_V2read_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000016 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// send ADC reset signal
void MINITCB_V2send_ADCRST(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000017, 0);
}

// send ADC calibration signal
void MINITCB_V2send_ADCCAL(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000018, 0);
}

// write ADC calibration delay
void MINITCB_V2write_ADCDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000019 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// write ADC align delay
void MINITCB_V2write_ADCALIGN(int tcp_Handle, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x2000001A;
  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read ADC status; TFADC500
unsigned long MINITCB_V2read_ADCSTAT(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000001A);
}

// write BitSlip : TFADC64
void MINITCB_V2write_BITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000001B + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// write flash ADC buffer mux ; TFADC64
void MINITCB_V2write_FMUX(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000001C;

  MINITCB_V2write_Module(tcp_Handle, mid, addr, ch -1);
}

// read flash ADC buffer mux ; TFADC64
unsigned long MINITCB_V2read_FMUX(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000001C);
}

// arm flash ADC buffer ; TFADC64
void MINITCB_V2arm_FADC(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000001D, 0);
}

// read flash ADC buffer ready ; TFADC64
unsigned long MINITCB_V2read_FREADY(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000001D);
}

// write zerosuppression flag duration (in ns 8 ~ 65532)
void MINITCB_V2write_ZSFD(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000001E, data);
}

// read zerosuppression flag duration 
unsigned long MINITCB_V2read_ZSFD(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000001E);
}

// write ADC down-sampling rate
void MINITCB_V2write_DSR(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000001F, data);
}

// read ADC down-sampling rate
unsigned long MINITCB_V2read_DSR(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000001F);
}

// read flash ADC buffer : TFADC64
void MINITCB_V2read_FADCBUF(int tcp_Handle, unsigned long mid, unsigned long *data)
{
  unsigned long addr = 0x20008000;
  unsigned long i;

  for (i = 0; i < 2048; i++) 
    data[i] = MINITCB_V2read_Module(tcp_Handle, mid, addr + i);
}

// align ADC for NKFADC500
void MINITCB_V2_ADCALIGN(int tcp_Handle, unsigned long mid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long gdly;
  int flag;

  MINITCB_V2send_ADCRST(tcp_Handle, mid); 
  usleep(500000);
  MINITCB_V2send_ADCCAL(tcp_Handle, mid); 
  MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 1);

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; dly++) {
      MINITCB_V2write_ADCDLY(tcp_Handle, mid, ch, dly); 
      value = (MINITCB_V2read_ADCSTAT(tcp_Handle, mid) >> (ch -1)) & 0x1; 

      // count bad delay
      if (!value) {
        flag = 1;
        count = count + 1;
        sum = sum + dly;
      }
      else {
        if (flag)
          dly = 32;
      }
    }

    // get bad delay center
    center = sum / count;

    // set good delay
    if (center < 11)
      gdly = center + 11;
    else
      gdly = center - 11;

    // set delay
    MINITCB_V2write_ADCDLY(tcp_Handle, mid, ch, gdly); 
    printf("ch%ld calibration delay = %ld\n", ch, gdly);
  }

  MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0); 
  MINITCB_V2send_ADCCAL(tcp_Handle, mid); 
}

// align ADC for M64ADC
void MINITCB_V2_ADCALIGN_64(int tcp_Handle, unsigned long mid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long bitslip;
  unsigned long gdly;
  unsigned long gbitslip;
  int flag;

  MINITCB_V2send_ADCRST(tcp_Handle, mid); 
  usleep(500000);
  MINITCB_V2send_ADCCAL(tcp_Handle, mid); 

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;
    gbitslip = 0;

    // ADC initialization codes
    MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0x030002); 
    usleep(100);
    MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0x010010); 
    usleep(100);
    MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0xC78001); 
    usleep(100);
    MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0xDE01C0); 
    usleep(100);

    // set deskew pattern
    MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0x450001); 

    // set bitslip = 0
    MINITCB_V2write_BITSLIP(tcp_Handle, mid, ch, 0); 

    for (dly = 0; dly < 32; dly++) {
      MINITCB_V2write_ADCDLY(tcp_Handle, mid, ch, dly); 
      value = (MINITCB_V2read_ADCSTAT(tcp_Handle, mid) >> (ch -1)) & 0x1; 
      
      // count bad delay
      if (!value) {
        flag = 1;
        count = count + 1;
        sum = sum + dly;
      }
      else {
        if (flag)
          dly = 32;
      }
    }

    // get bad delay center
    center = sum / count;

    // set good delay
    if (center < 9)
      gdly = center + 9;
    else
      gdly = center - 9;

    // sets delay
    MINITCB_V2write_ADCDLY(tcp_Handle, mid, ch, gdly); 

    // set sync pattern
    MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0x450002); 
    usleep(100);

    for (bitslip = 0; bitslip < 12; bitslip++) {
      MINITCB_V2write_BITSLIP(tcp_Handle, mid, ch, bitslip); 

      value = (MINITCB_V2read_ADCSTAT(tcp_Handle, mid) >> ((ch -1) + 4)) & 0x1; 
      if (value) {
        gbitslip = bitslip;
        bitslip = 12;
      }
    }

    // set good bitslip
    MINITCB_V2write_BITSLIP(tcp_Handle, mid, ch, gbitslip); 

    printf("ch%ld calibration delay = %ld, bitslip = %ld\n", ch, gdly, gbitslip);
  }

  // set normal ADC mode
  MINITCB_V2write_ADCALIGN(tcp_Handle, mid, 0x450000); 
  usleep(100);
  MINITCB_V2send_ADCCAL(tcp_Handle, mid); 
}

// write peak sum scale - peak sum out = peak area/peak sum scale ; TFADC64
void MINITCB_V2write_PSS(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000010 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read peak sum sacle ; TFADC64
unsigned long MINITCB_V2read_PSS(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000010 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write rise time, rise time = set value * 2 * sampling period ; AMOREADC
void MINITCB_V2write_RT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000011 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read rise time ; AMOREADC
unsigned long MINITCB_V2read_RT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000011 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write sampling rate, sampling rate = 1 MHz / setting value; AMOREADC
void MINITCB_V2write_SR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000012 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read sampling rate ; AMOREADC
unsigned long MINITCB_V2read_SR(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000012 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write gain, value is 1/2/4, gain = 2^setting; AMOREADC
void MINITCB_V2write_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000013 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read gain ; AMOREADC
unsigned long MINITCB_V2read_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000013 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write signal window length in # of ADC samples ; AMOREADC
void MINITCB_V2write_ST(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000020 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read signal window length in # of ADC samples ; AMOREADC
unsigned long MINITCB_V2read_ST(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000020 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write pedestal window length in # of ADC samples ; AMOREADC
void MINITCB_V2write_PT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000021 + (((ch - 1) & 0xFF) << 16);

  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read pedestal window length in # of ADC samples ; AMOREADC
unsigned long MINITCB_V2read_PT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000021 + (((ch - 1) & 0xFF) << 16);

  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write run number
void MINITCB_V2write_RUNNO(int tcp_Handle, unsigned long data)
{
  MINITCB_V2write(tcp_Handle, 0x04, data & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x05, (data >> 8) & 0xFF);
}

// read run number
unsigned long MINITCB_V2read_RUNNO(int tcp_Handle)
{
  unsigned long data;
  unsigned long temp;

  data = MINITCB_V2read(tcp_Handle, 0x04);
  temp = MINITCB_V2read(tcp_Handle, 0x05);
  data = data + (temp << 8);

  return data;
}

// send trigger
void MINITCB_V2send_TRIG(int tcp_Handle)
{
  MINITCB_V2write(tcp_Handle, 0x06, 0);
}

// read link status
unsigned long MINITCB_V2read_LNSTAT(int tcp_Handle)
{
  return MINITCB_V2read(tcp_Handle, 0x10);
}

// read mids
unsigned long MINITCB_V2read_MIDS(int tcp_Handle, unsigned long ch)
{
  unsigned long addr;

  addr = 0x10 + ch;

  return  MINITCB_V2read(tcp_Handle, addr);
}

// write pedestal trigger interval in ms
void MINITCB_V2write_PTRIG(int tcp_Handle, unsigned long data)
{
  MINITCB_V2write(tcp_Handle, 0x07, (data >> 16) & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x08, data & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x09, (data >> 8) & 0xFF);
}

// read pedestal trigger interval in ms
unsigned long MINITCB_V2read_PTRIG(int tcp_Handle)
{
  unsigned long data;
  unsigned long temp;

  data = MINITCB_V2read(tcp_Handle, 0x08);
  temp = MINITCB_V2read(tcp_Handle, 0x09);
  data = data + (temp << 8);
  temp = MINITCB_V2read(tcp_Handle, 0x07);
  data = data + (temp << 16);

  return data;
}

// write trigger enable, D0 = self, D1 = pedestal, D2 = software, D3 = external
void MINITCB_V2write_TRIGENABLE(int tcp_Handle, unsigned long mid, unsigned long data)
{
  if (mid)
    MINITCB_V2write_Module(tcp_Handle, mid, 0x20000030, data);
  else
    MINITCB_V2write(tcp_Handle, 0x0A, data & 0xFF);
}

// read trigger enable
unsigned long MINITCB_V2read_TRIGENABLE(int tcp_Handle, unsigned long mid)
{
  unsigned long data;

  if (mid)
    data = MINITCB_V2read_Module(tcp_Handle, mid, 0x20000030);
  else data = MINITCB_V2read(tcp_Handle, 0x0A);

  return data;
}

// write multiplicity trigger threshold
void MINITCB_V2write_MTHR(int tcp_Handle, unsigned long data)
{
  MINITCB_V2write(tcp_Handle, 0x0C, data & 0xFF);
}

// read multiplicity trigger threshold
unsigned long MINITCB_V2read_MTHR(int tcp_Handle)
{
  return MINITCB_V2read(tcp_Handle, 0x0C);
}

// write trigger prescale
void MINITCB_V2write_PSCALE(int tcp_Handle, unsigned long data)
{
  MINITCB_V2write(tcp_Handle, 0x0E, data & 0xFF);
  MINITCB_V2write(tcp_Handle, 0x0F, (data >> 8) & 0xFF);
}

// read trigger prescale
unsigned long MINITCB_V2read_PSCALE(int tcp_Handle)
{
  unsigned long data;
  unsigned long temp;

  data = MINITCB_V2read(tcp_Handle, 0x0E);
  temp = MINITCB_V2read(tcp_Handle, 0x0F);
  data = data + (temp << 8);

  return data;
}

// write DRAM input delay
void MINITCB_V2write_DRAMDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000022 + ((ch & 0xFF) << 16);
  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// write DRAM input bitslip
void MINITCB_V2write_DRAMBITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000023 + ((ch & 0xFF) << 16);
  MINITCB_V2write_Module(tcp_Handle, mid, addr, 0);
}

// write DRAM test mode
void MINITCB_V2write_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000024, data);
}

// read DRAM test mode
unsigned long MINITCB_V2read_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000024 + ((ch & 0xFF) << 16);
  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// Align DRAM input
void MINITCB_V2_ADCALIGN_DRAM(int tcp_Handle, unsigned long mid)
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  int aflag;
  unsigned long gdly;
  int bitslip;

  // turn on DRAM    
  MINITCB_V2write_DRAMON(tcp_Handle, mid, 1);

  // enter DRAM test mode
  MINITCB_V2write_DRAMTEST(tcp_Handle, mid, 1);

  // send reset to iodelay  
  MINITCB_V2send_ADCCAL(tcp_Handle, mid);

  // fill DRAM test pattern
  MINITCB_V2write_DRAMTEST(tcp_Handle, mid, 2);

  for (ch = 0; ch < 8; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    // search delay
    for (dly = 0; dly < 32; dly++) {
      // set delay
      MINITCB_V2write_DRAMDLY(tcp_Handle, mid, ch, dly);

      // read DRAM test pattern
      MINITCB_V2write_DRAMTEST(tcp_Handle, mid, 3);
      value = MINITCB_V2read_DRAMTEST(tcp_Handle, mid, ch);

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
        if (count > 4)
          flag = 1; 
     }
      else {
        if (flag)
          dly = 32;
        else {
          count = 0;
          sum = 0;
        }
      }
    }

    // get bad delay center
    if (count)
      gdly = sum / count;
    else
      gdly = 9;

    // set delay
    MINITCB_V2write_DRAMDLY(tcp_Handle, mid, ch, gdly); 
  
    // get bitslip
    for (bitslip = 0; bitslip < 4; bitslip++) {
      // read DRAM test pattern
      MINITCB_V2write_DRAMTEST(tcp_Handle, mid, 3);
      value = MINITCB_V2read_DRAMTEST(tcp_Handle, mid, ch);

      if (value == 0xFFAA5500) {
        aflag = 1;
        bitslip = 4;
      }
      else {
        aflag = 0;
        MINITCB_V2write_DRAMBITSLIP(tcp_Handle, mid, ch);
      }
    }

    if (aflag)
      printf("DRAM(%ld) is aligned, delay = %ld\n", ch, gdly);
    else
      printf("Fail to align DRAM(%ld)!\n", ch);
  }

  // exit DRAM test mode
  MINITCB_V2write_DRAMTEST(tcp_Handle, mid, 0);
}

//================================================================================================

// internal functions *********************************************************************************
void TCBstart_DRAM(int tcp_Handle, unsigned long mid);
void TCBstop_DRAM(int tcp_Handle, unsigned long mid);
int TCBread_DRAM_READY(int tcp_Handle, unsigned long mid);
void TCBsetup_ADC(int tcp_Handle, unsigned long mid, int addr, int data);
int TCBread_ADC_ALIGN(int tcp_Handle, unsigned long mid, int ch);
void TCBwrite_DRAM_TEST(int tcp_Handle, unsigned long mid, int data);
int TCBread_DRAM_ALIGN(int tcp_Handle, unsigned long mid, int ch);
void TCBreset_REF_CLK(int tcp_Handle, unsigned long mid);
void TCBreset_ADC(int tcp_Handle, unsigned long mid);
void TCBwrite_ADC_IDLY(int tcp_Handle, unsigned long mid, int ch, int data);
void TCBwrite_ADC_BITSLIP(int tcp_Handle, unsigned long mid, int ch);
void TCBwrite_ADC_IMUX(int tcp_Handle, unsigned long mid, int ch, int data);
void TCBwrite_DRAM_IDLY(int tcp_Handle, unsigned long mid, int ch, int data);
void TCBwrite_DRAM_BITSLIP(int tcp_Handle, unsigned long mid, int ch);


// start DRAM
void TCBstart_DRAM(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000001, 1);
}

// stop DRAM
void TCBstop_DRAM(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000001, 0);
}

// read DRAM ready
int TCBread_DRAM_READY(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000001);
}

// write acquisition time
void TCBwrite_ACQ_TIME(int tcp_Handle, int data)
{
  MINITCB_V2write_Module(tcp_Handle, 0, 0x20000002, data);
}

// read acquisition time
int TCBread_ACQ_TIME(int tcp_Handle)
{
  return MINITCB_V2read_Module(tcp_Handle, 0, 0x20000002);
}

// write High voltage
void TCBwrite_HV(int tcp_Handle, unsigned long mid, int ch, float data)
{
  float fval;
  int value;
  int addr;
  
  addr = (ch - 1) * 0x10000 + 0x20000002;
  fval = 4.63 * (data - 4.5);
  value = (int)(fval);
  if (value > 254)
    value = 254;
  else if (value < 0)
    value = 0;

  MINITCB_V2write_Module(tcp_Handle, mid, addr, value);
}

// read high voltage
float TCBread_HV(int tcp_Handle, unsigned long mid, int ch)
{
  int data;
  float value;
  int addr;

  addr = (ch - 1) * 0x10000 + 0x20000002;
  data = MINITCB_V2read_Module(tcp_Handle, mid, addr);
  value = data;
  value = value / 4.63 + 4.5;

  return value;
}

// write coincidence width
void TCBwrite_COIN_WIDTH(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000003, data);
}

// read coincidence width
int TCBread_COIN_WIDTH(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000003);
}

// write multiplicity threshold
void TCBwrite_MULTI_THR(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000004, data);
}

// read multiplicity threshold
int TCBread_MULTI_THR(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000004);
}

// write pedestal trigger interval, 0 ~ 65535 ms, when 0 disabled
void TCBwrite_PEDESTAL_TRIGGER_INTERVAL(int tcp_Handle, int data)
{
  MINITCB_V2write_Module(tcp_Handle, 0, 0x20000005, data);
}

// read pedestal trigger interval
int TCBread_PEDESTAL_TRIGGER_INTERVAL(int tcp_Handle)
{
  return MINITCB_V2read_Module(tcp_Handle, 0, 0x20000005);
}

// write discriminator threshold
void TCBwrite_DISC_THR(int tcp_Handle, unsigned long mid, int ch, int data)
{
  int addr;
  
  addr = (ch - 1) * 0x10000 + 0x20000005;
  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

// read discriminator threshold
int TCBread_DISC_THR(int tcp_Handle, unsigned long mid, int ch)
{
  int addr;
  
  addr = (ch - 1) * 0x10000 + 0x20000005;
  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// send software trigger
void TCBsend_TRIG(int tcp_Handle)
{
  MINITCB_V2write_Module(tcp_Handle, 0, 0x20000006, 0);
}

// write gate width
void TCBwrite_GATE_WIDTH(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000006, data);
}

// read gate width
int TCBread_GATE_WIDTH(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000006);
}

// write trigger enable, self = 1, pedestal = 2, software = 4, external = 8
void TCBwrite_TRIGGER_ENABLE(int tcp_Handle, int data)
{
  MINITCB_V2write_Module(tcp_Handle, 0, 0x20000007, data);
}

// read trigger enable
int TCBread_TRIGGER_ENABLE(int tcp_Handle)
{
  return MINITCB_V2read_Module(tcp_Handle, 0, 0x20000007);
}

// write input delay
void TCBwrite_DELAY(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000007, data);
}

// read input deay
int TCBread_DELAY(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000007);
}

// measure pedestal
void TCBmeasure_PED(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000008, 0);
}

// read pedestal
int TCBread_PED(int tcp_Handle, unsigned long mid, int ch)
{
  int addr;
  
  addr = (ch - 1) * 0x10000 + 0x20000008;
  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write peak sum width
void TCBwrite_PEAK_SUM_WIDTH(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000009, data);
}

// read peak sum width
int TCBread_PEAK_SUM_WIDTH(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x20000009);
}

// write zero suppression
void TCBwrite_ZERO_SUP(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000000A, data);
}

// read zero suppression
int TCBread_ZERO_SUP(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000000A);
}

// write waveform prescale
void TCBwrite_PRESCALE(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000000B, data);
}

// read waveform prescale
int TCBread_PRESCALE(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000000B);
}

// read temperature
void TCBread_TEMP(int tcp_Handle, unsigned long mid, float *temp)
{
  int data;
  float fval;

  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000000C, 0);

  data = MINITCB_V2read_Module(tcp_Handle, mid, 0x2000000C);
  fval = data;
  fval = fval * 0.04476 - 33.3;
  temp[0] = fval;

  data = MINITCB_V2read_Module(tcp_Handle, mid, 0x2001000C);
  fval = data;
  fval = fval * 0.04476 - 33.3;
  temp[1] = fval;
}

// write echo register
void TCBwrite_ECHO(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000000D, data);
}

// read echo register
int TCBread_ECHO(int tcp_Handle, unsigned long mid)
{
  return MINITCB_V2read_Module(tcp_Handle, mid, 0x2000000D);
}

void TCBsetup_ADC(int tcp_Handle, unsigned long mid, int addr, int data)
{
  int value;
  value = ((addr & 0xFFFF) << 16) | (data & 0xFFFF);
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000000E, value);
}

int TCBread_ADC_ALIGN(int tcp_Handle, unsigned long mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x2000000E;
  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// write DRAM test mode
void TCBwrite_DRAM_TEST(int tcp_Handle, unsigned long mid, int data)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x2000000F, data);
}

// read DRAM alignment pattern
int TCBread_DRAM_ALIGN(int tcp_Handle, unsigned long mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x2000000F;
  return MINITCB_V2read_Module(tcp_Handle, mid, addr);
}

// read DAQ link status
void TCBread_LINK_STATUS(int tcp_Handle, int *data)
{
  data[0] = MINITCB_V2read_Module(tcp_Handle, 0, 0x20000010);
  data[1] = MINITCB_V2read_Module(tcp_Handle, 0, 0x20000011);
}

// read DAQ mid
void TCBread_MID(int tcp_Handle, int *data)
{
  int i;

  for (i = 0; i < 40; i ++) 
    data[i] = MINITCB_V2read_Module(tcp_Handle, 0, 0x20000012 + i);
}

// reset delay reference clock
void TCBreset_REF_CLK(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000020, 0);
}

// reset ADC
void TCBreset_ADC(int tcp_Handle, unsigned long mid)
{
  MINITCB_V2write_Module(tcp_Handle, mid, 0x20000021, 0);
}

void TCBwrite_ADC_IDLY(int tcp_Handle, unsigned long mid, int ch, int data)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000022;
  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

void TCBwrite_ADC_BITSLIP(int tcp_Handle, unsigned long mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000023;
  MINITCB_V2write_Module(tcp_Handle, mid, addr, 0);
}

void TCBwrite_ADC_IMUX(int tcp_Handle, unsigned long mid, int ch, int data)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000024;
  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

void TCBwrite_DRAM_IDLY(int tcp_Handle, unsigned long mid, int ch, int data)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000025;
  MINITCB_V2write_Module(tcp_Handle, mid, addr, data);
}

void TCBwrite_DRAM_BITSLIP(int tcp_Handle, unsigned long mid, int ch)
{
  int addr;
  
  addr = ch * 0x10000 + 0x20000026;
  MINITCB_V2write_Module(tcp_Handle, mid, addr, 0);
}

void TCBinit_ADC(int tcp_Handle, unsigned long mid)
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
  int align_count;
  
  align_count = 12;

  // reset input delay ref clock
  TCBreset_REF_CLK(tcp_Handle, mid); 

  // reset ADC
  TCBreset_ADC(tcp_Handle, mid);

  // set ADC registers
  TCBsetup_ADC(tcp_Handle, mid, 0x00, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x01, 0x0010);
  TCBsetup_ADC(tcp_Handle, mid, 0xD1, 0x0240);
  TCBsetup_ADC(tcp_Handle, mid, 0x02, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x0F, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x14, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x1C, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x24, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x28, 0x8100);
  TCBsetup_ADC(tcp_Handle, mid, 0x29, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x2A, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x2B, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x38, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x42, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x45, 0x0000);
  TCBsetup_ADC(tcp_Handle, mid, 0x46, 0x8801);

  for (retry = 0; retry < 1; retry++) {
    all_aligned = 0;

    for (ch = 0; ch < 4; ch++) {
      count = 0;
      sum = 0;
      flag = 0;
      aligned = 0;

      // set test pattern
      TCBsetup_ADC(tcp_Handle, mid, 0x26, 0x3330);
      TCBsetup_ADC(tcp_Handle, mid, 0x25, 0x0013);
      
      // search delay
      for (dly = 0; dly < 32; dly++) {
        // set delay
        TCBwrite_ADC_IDLY(tcp_Handle, mid, ch, dly);

        // read bit_alignment status
        value = TCBread_ADC_ALIGN(tcp_Handle, mid, ch) & 0x1;

        if(value) {
          count = count + 1;
          sum = sum + dly;
          if(count > align_count) 
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
      if (count > align_count) {
        gdly = sum / count;
        aligned = 1;
      }
      else
        gdly = 0;
    
      // set good delay
      TCBwrite_ADC_IDLY(tcp_Handle, mid, ch, gdly);

      // set word sync test pattern
      TCBsetup_ADC(tcp_Handle, mid, 0x26, 0x0000);
      TCBsetup_ADC(tcp_Handle, mid, 0x25, 0x0012);

      // get bitslip
      TCBwrite_ADC_IMUX(tcp_Handle, mid, ch, 0);
      for (bitslip = 0; bitslip < 8; bitslip++) {
        if (bitslip == 4)
          TCBwrite_ADC_IMUX(tcp_Handle, mid, ch, 1);
            
        value = TCBread_ADC_ALIGN(tcp_Handle, mid, ch) & 0x2;

        if (value) {
          aligned = aligned + 1;
          bitslip = 8;
        }
        else 
          TCBwrite_ADC_BITSLIP(tcp_Handle, mid, ch);
      }
      
      all_aligned = all_aligned + aligned;
    }

    if (all_aligned == 8) 
      retry = 2;
  }
        
  // set ADC in normal output mode
  TCBsetup_ADC(tcp_Handle, mid, 0x25, 0x0000);

  if (all_aligned == 8)
    printf("mid = %ld : ADC is aligned.\n", mid);
  else
    printf("mid = %ld : Fail to align ADC!\n", mid);
}

void TCBinit_DRAM(int tcp_Handle, unsigned long mid)
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
  
  ready = TCBread_DRAM_READY(tcp_Handle, mid);
  
  if (ready)
    TCBstop_DRAM(tcp_Handle, mid);

  TCBstart_DRAM(tcp_Handle, mid);
  
  ready = 0;
  while (!ready)
    ready = TCBread_DRAM_READY(tcp_Handle, mid);
  
  // turn on DRAM    
  TCBstart_DRAM(tcp_Handle, mid);
  
  // enter DRAM test mode
  TCBwrite_DRAM_TEST(tcp_Handle, mid, 1); 

  for (retry = 0; retry < 2; retry++) {
    all_aligned = 0;

    // send reset to iodelay  
    TCBreset_REF_CLK(tcp_Handle, mid); 

    // fill DRAM test pattern
    TCBwrite_DRAM_TEST(tcp_Handle, mid, 2); 

    for (ch = 0; ch < 2; ch++) {
      count = 0;
      sum = 0;
      flag = 0;
      aligned = 0;

      // search delay
      for (dly = 0; dly < 32; dly++) {
        // set delay
        TCBwrite_DRAM_IDLY(tcp_Handle, mid, ch, dly);

        // read DRAM test pattern
        TCBwrite_DRAM_TEST(tcp_Handle, mid, 3); 
        value = TCBread_DRAM_ALIGN(tcp_Handle, mid, ch); 

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
      TCBwrite_DRAM_IDLY(tcp_Handle, mid, ch, gdly);
  
      // get bitslip
      for (bitslip = 0; bitslip < 4; bitslip++) {
        // read DRAM test pattern
        TCBwrite_DRAM_TEST(tcp_Handle, mid, 3); 
        value = TCBread_DRAM_ALIGN(tcp_Handle, mid, ch); 

        if (value == 0xFFAA5500) {
          aligned = aligned + 1;
          bitslip = 4;
        }
        else 
          TCBwrite_DRAM_BITSLIP(tcp_Handle, mid, ch);
      }
      
      all_aligned = all_aligned + aligned;
    }
    
    if (all_aligned == 4) 
      retry = 2;
  }

  if (all_aligned == 4)
    printf("mid = %ld : DRAM is aligned.\n", mid);
  else
    printf("mid = %ld : Fail to align DRAM!\n", mid);
   
  // exit DRAM test mode
  TCBwrite_DRAM_TEST(tcp_Handle, mid, 0); 
}



