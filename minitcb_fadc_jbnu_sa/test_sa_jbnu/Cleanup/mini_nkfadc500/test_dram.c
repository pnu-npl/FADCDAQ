#include <unistd.h>
#include <stdio.h>
#include "usb3com.h"
#include "NoticeNKFADC500.h"

void alignDRAM(int sid);

int main(void)
{
  int sid = 1;  	                
  int com;
  unsigned long dly;
  unsigned long bcount;
  char data[16384];
  FILE *fp;

  USB3Init(0);
  NKFADC500open(sid, 0);


  NKFADC500_ADCALIGN_500(sid);
  

  // set NKFADC500
  NKFADC500write_RL(sid, 32); 
  NKFADC500write_DRAMON(sid, 1);
  NKFADC500write_TLT(sid, 0x0000);
  NKFADC500write_DRAMTEST(sid, 1);
  NKFADC500send_ADCCAL(sid);

  printf("rl = %ld\n", NKFADC500read_RL(sid));
  printf("dramon = %ld\n", NKFADC500read_DRAMON(sid));
  printf("tlt = %lX\n", NKFADC500read_TLT(sid));
  NKFADC500reset(sid); 

  com = 99;

  while(com) {
    printf("\n\n");
    printf("1. set delay  2. bitslip  3. read data  4. align DRAM  0. quit\n");
    printf("5. start  6. reset  7. send trigger\n");
    printf("enter command : ");
    scanf("%d", &com);

    if (com == 1) {
      printf("\n");
      printf("enter delay(0~31) : ");
      scanf("%ld", &dly);
      NKFADC500write_DRAMDLY(sid, dly);
    }
    else if (com == 2) {
      NKFADC500write_BITSLIP(sid);
    }
    else if (com == 3) {
      NKFADC500start(sid);
      bcount = 0;
      while (bcount < 16) {
        NKFADC500send_TRIG(sid);
        bcount = NKFADC500read_BCOUNT(sid);
      }
      
      NKFADC500read_DATA(sid, 16, data);
   
      fp = fopen("dram.dat", "wb");
      fwrite(data, 1, 16384, fp);
      fclose(fp);
      
      NKFADC500stop(sid);

      bcount = NKFADC500read_BCOUNT(sid);
      if (bcount)   
        NKFADC500read_DATA(sid, bcount, data);
    }
    else if (com == 4)
      alignDRAM(sid);
    else if (com == 5)
      NKFADC500start(sid);
    else if (com == 6)
      NKFADC500reset(sid);
    else if (com == 7)
      NKFADC500send_TRIG(sid);
  }

  NKFADC500close(sid);
  USB3Exit(0);

  return 0;	
}

void alignDRAM(int sid)
{
  unsigned long dly;
  unsigned long bcount;
  char data[16384];
  int i;
  int j;
  int k;
  char c[4];
  int flag;
  int count;
  int sum;
  int center;
  int aflag;
  unsigned long gdly;

  NKFADC500write_RL(sid, 32); 
  NKFADC500write_DRAMON(sid, 1);
  NKFADC500write_TLT(sid, 0x0000);
  NKFADC500write_DRAMTEST(sid, 1);
  NKFADC500send_ADCCAL(sid);
  NKFADC500reset(sid); 

  count = 0;
  sum = 0;
  flag = 0;

  for (dly = 0; dly < 32; dly++) {
    NKFADC500write_DRAMDLY(sid, dly);

    NKFADC500start(sid);
    bcount = 0;

    while (bcount < 16) {
      NKFADC500send_TRIG(sid);
      bcount = NKFADC500read_BCOUNT(sid);
    }
      
    NKFADC500read_DATA(sid, 16, data);
      
    NKFADC500stop(sid);

    for (j = 0; j < 4; j++)
      c[j] = 0;

    for (i = 0; i < 500; i++) {
      for (j = 0; j < 4; j++) {
        for (k = 0; k < 8; k++) 
          c[j] = c[j] | (data[i * 32 + j * 8 + k + 0x100] & 0xFF);
      }
    }

    bcount = NKFADC500read_BCOUNT(sid);
    if (bcount)   
      NKFADC500read_DATA(sid, bcount, data);

    aflag = 0;
    if ((c[0] == 0x00) && (c[1] == 0x55) && (c[2] == 0xAA) && (c[3] == 0xFF))
      aflag = 1;
    else if ((c[0] == 0x55) && (c[1] == 0xAA) && (c[2] == 0xFF) && (c[3] == 0x00))
      aflag = 1;
    else if ((c[0] == 0xAA) && (c[1] == 0xFF) && (c[2] == 0x00) && (c[3] == 0x55))
      aflag = 1;
    else if ((c[0] == 0xFF) && (c[1] == 0x00) && (c[2] == 0x55) && (c[3] == 0xAA))
      aflag = 1;
    
    if (!aflag) {
      flag = 1; 
      count = count + 1;
      sum = sum + dly;
    }
    else {
      if (aflag)
        dly = 32;
    }
  }

  // get bad delay center
  center = sum / count;

  // set good delay
  if (center < 10)
    gdly = center + 10;
  else
    gdly = center - 10;

  // set delay
  NKFADC500write_DRAMDLY(sid, gdly); 
  printf("DRAM calibration delay = %ld\n", gdly);
  
  // get bitslip
  for (i = 0; i < 4; i++) {
    NKFADC500start(sid);
    bcount = 0;

    while (bcount < 16) {
      NKFADC500send_TRIG(sid);
      bcount = NKFADC500read_BCOUNT(sid);
    }
      
    NKFADC500read_DATA(sid, 16, data);
      
    NKFADC500stop(sid);

    c[0] = data[0] & 0xFF;
    c[1] = data[4] & 0xFF;
    c[2] = data[8] & 0xFF;
    c[3] = data[12] & 0xFF;

    bcount = NKFADC500read_BCOUNT(sid);
    if (bcount)   
      NKFADC500read_DATA(sid, bcount, data);

    if ((c[0] == 0x00) && (c[1] == 0x01) && (c[2] == 0x02) && (c[3] == 0x03)) {
      aflag = 1;
      i = 4;
    }
    else {
      aflag = 0;
      NKFADC500write_BITSLIP(sid);
    }
  }

  if (aflag)
    printf("DRAM is aligned!\n");
  else
    printf("Fail to align DRAM!\n");

  NKFADC500write_TLT(sid, 0x0000);
//  NKFADC500write_DRAMTEST(sid, 0);
}

