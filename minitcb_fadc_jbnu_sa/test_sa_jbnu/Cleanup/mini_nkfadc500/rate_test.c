#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <math.h>

#include "NoticeNKFADC500.h"
#include "usb3com.h"
#include "nkusbconst.h"

void adc_setting(int sid);
void adc_waveform(int sid, int disc);

int main(int argc, char *argv[])
{
  int disc;
  int sid;

  // check # of argument
  if (argc < 2) {
    printf("./rate_test.exe 0 for not writing to disc\n");
    printf("./rate_test.exe 1 for writing to disc\n");
    return -1;
  }

  disc = atoi(argv[1]);
  sid = 1;

  // open USB3
  USB3Init(0);

  // open KFADC500
  NKFADC500open(sid, 0);

  // setting ADC
  adc_setting(sid);

  adc_waveform(sid, disc);

  // close TCB 
  NKFADC500close(sid);

  // exit Libusb
  USB3Exit(0);

  return 0;
}

void adc_setting(int sid)
{
  unsigned long rl = 256;	       	// recording length: 1=128ns, 2=256ns, 4=512ns, 
  unsigned long tlt = 0xFFFE;	       	// trigger lookup table value
  unsigned long ptrig_interval = 0;    	// pedestal trigger interval in ms, 0 for disable
  unsigned long sr = 1;                 // sampling rate /1/2/4/8
  unsigned long cw = 1000;	 	// coincidence width (8 ~ 32760)
  unsigned long offset = 2000;          // ADC offset value (0~4095)
  unsigned long dly = 400;         	// ADC waveform delay from trigger point (0 ~ 31992)
  unsigned long thr = 200;		// discrimination threshold 
  unsigned long pol = 0;		// input pulse polarity, 0 = negative, 1 = positive
  unsigned long psw = 2;		// peak sum width in ns (2 ~ 16382 ns)
  unsigned long amode = 0;		// ADC mode, 0 = raw, 1 = filtered
  unsigned long pct = 1;		// pulse count threshold (1 ~ 15)
  unsigned long pci = 1000;		// pulse count interval (32 ~ 8160 ns)
  unsigned long pwt = 100;		// pulse width threshold (2 ~ 1022 ns)
  unsigned long dt = 0;			// trigger deadtime (0 ~ 8355840 ns)
  unsigned long ten = 1; 		 // when 1 enables pulse count trigger 
  unsigned long tew = 0; 		 // when 1 enables pulse width trigger
  unsigned long tep = 0; 	         // when 1 enables peak sum trigger
  unsigned long tep2 = 0;                // when 1 enables peak sum OR trigger
  unsigned long tmode = (tep2<<3) | (tep<<2) | (tew<<1) | ten;


  // set NKFADC500
  NKFADC500write_DSR(sid, sr);
  NKFADC500resetTIMER(sid);  
  NKFADC500reset(sid);  
  NKFADC500_ADCALIGN_500(sid);
  NKFADC500write_PTRIG(sid, ptrig_interval);
  NKFADC500write_CW(sid, 1, cw); 
  NKFADC500write_CW(sid, 2, cw); 
  NKFADC500write_CW(sid, 3, cw); 
  NKFADC500write_CW(sid, 4, cw); 
  NKFADC500write_RL(sid, rl); 
  NKFADC500write_DRAMON(sid, 1);
  NKFADC500write_DACOFF(sid, 1, offset);
  NKFADC500write_DACOFF(sid, 2, offset);
  NKFADC500write_DACOFF(sid, 3, offset);
  NKFADC500write_DACOFF(sid, 4, offset);
  NKFADC500measure_PED(sid, 1);
  NKFADC500measure_PED(sid, 2);
  NKFADC500measure_PED(sid, 3);
  NKFADC500measure_PED(sid, 4);
  printf("ch1 pedestal = %ld\n", NKFADC500read_PED(sid, 1));
  printf("ch2 pedestal = %ld\n", NKFADC500read_PED(sid, 2));
  printf("ch3 pedestal = %ld\n", NKFADC500read_PED(sid, 3));
  printf("ch4 pedestal = %ld\n", NKFADC500read_PED(sid, 4));
  NKFADC500write_DLY(sid, 1, cw + dly);
  NKFADC500write_DLY(sid, 2, cw + dly);
  NKFADC500write_DLY(sid, 3, cw + dly);
  NKFADC500write_DLY(sid, 4, cw + dly);
  NKFADC500write_THR(sid, 1, thr);
  NKFADC500write_THR(sid, 2, thr);
  NKFADC500write_THR(sid, 3, thr);
  NKFADC500write_THR(sid, 4, thr);
  NKFADC500write_POL(sid, 1, pol);
  NKFADC500write_POL(sid, 2, pol);
  NKFADC500write_POL(sid, 3, pol);
  NKFADC500write_POL(sid, 4, pol);
  NKFADC500write_PSW(sid, 1, psw);
  NKFADC500write_PSW(sid, 2, psw);
  NKFADC500write_PSW(sid, 3, psw);
  NKFADC500write_PSW(sid, 4, psw);
  NKFADC500write_AMODE(sid, 1, amode);
  NKFADC500write_AMODE(sid, 2, amode);
  NKFADC500write_AMODE(sid, 3, amode);
  NKFADC500write_AMODE(sid, 4, amode);
  NKFADC500write_PCT(sid, 1, pct);
  NKFADC500write_PCT(sid, 2, pct);
  NKFADC500write_PCT(sid, 3, pct);
  NKFADC500write_PCT(sid, 4, pct);
  NKFADC500write_PCI(sid, 1, pci);
  NKFADC500write_PCI(sid, 2, pci);
  NKFADC500write_PCI(sid, 3, pci);
  NKFADC500write_PCI(sid, 4, pci);
  NKFADC500write_PWT(sid, 1, pwt);
  NKFADC500write_PWT(sid, 2, pwt);
  NKFADC500write_PWT(sid, 3, pwt);
  NKFADC500write_PWT(sid, 4, pwt);
  NKFADC500write_DT(sid, 1, dt);
  NKFADC500write_DT(sid, 2, dt);
  NKFADC500write_DT(sid, 3, dt);
  NKFADC500write_DT(sid, 4, dt);
  NKFADC500write_TM(sid, 1, tmode);
  NKFADC500write_TM(sid, 2, tmode);
  NKFADC500write_TM(sid, 3, tmode);
  NKFADC500write_TM(sid, 4, tmode);
  NKFADC500write_TLT(sid, tlt);
}

// ADC waveform test
void adc_waveform(int sid, int disc)
{
  char filename[256];               
  unsigned long bcount;
  FILE *fp;
  int i;
  char data[1048576];
  struct timeval startt, stopt;
  double elapst, rate;

  // reset DAQ
  NKFADC500reset(sid); 

  // start DAQ
  NKFADC500start(sid);

  // set data filename
  if (disc)
    sprintf(filename, "rate_test.dat");

//  startt = time(NULL);
  gettimeofday(&startt, 0x0);

  bcount = 0;
  while (bcount < 4096000) {
    bcount = NKFADC500read_BCOUNT(sid);
//    printf("bcount = %ld\n", bcount);
  }    

//  stopt = time(NULL);
  gettimeofday(&stopt, 0x0);

//  elapst = difftime(stopt, startt);
  elapst = (double)((stopt.tv_usec - startt.tv_usec) + (stopt.tv_sec - startt.tv_sec) * 1000000);
  printf("time = %lf\n", elapst);

//  rate = 4000.0 / elapst;
  rate = 4.0 * 1024.0 * 1024.0 * 1024.0 / elapst;
  printf("data input rate = %lf\n", rate);

  // open data file
  if (disc)
    fp = fopen(filename, "wb");

//  startt = time(NULL);
  gettimeofday(&stopt, 0x0);

  for (i = 0; i < 4000; i++) {
    // read data from ADC's DRAM
    NKFADC500read_DATA(sid, 1024, data);
      
    // write to file
    if (disc)
      fwrite(data, 1, 1048576, fp);
  }

//  stopt = time(NULL);
  gettimeofday(&stopt, 0x0);

//  elapst = difftime(stopt, startt);
  elapst = (double)((stopt.tv_usec - startt.tv_usec) + (stopt.tv_sec - startt.tv_sec) * 1000000);
  printf("time = %lf\n", elapst);

//  rate = 4000.0 / elapst;
  rate = 4.0 * 1024.0 * 1024.0 * 1024.0 / elapst;
  printf("data transfer rate = %lf\n", rate);

  if (disc)
    fclose(fp);

  // stop DAQ
  NKFADC500stop(sid);

  // reset DAQ
  NKFADC500reset(sid);  
}


