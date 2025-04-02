#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JBNU_TCB_VID (0x0547)
#define JBNU_TCB_PID (0x1501)

extern void USB3Init(void);
extern void USB3Exit(void);
extern int TCBopen(int sid);
extern void TCBclose(int sid);
extern void TCBreset_TIMER(int sid);
extern void TCBreset(int sid);
extern void TCBstart_DAQ(int sid);
extern int TCBread_RUN(int sid);
extern void TCBwrite_RUN_NUMBER(int sid, int data);
extern int TCBread_RUN_NUMBER(int sid);
extern void TCBwrite_ACQ_TIME(int sid, int data);
extern int TCBread_ACQ_TIME(int sid);
extern void TCBwrite_HV(int sid, int mid, int ch, float data);
extern float TCBread_HV(int sid, int mid, int ch);
extern void TCBwrite_COIN_WIDTH(int sid, int mid, int data);
extern int TCBread_COIN_WIDTH(int sid, int mid);
extern void TCBwrite_MULTI_THR(int sid, int mid, int data);
extern int TCBread_MULTI_THR(int sid, int mid);
extern void TCBwrite_PEDESTAL_TRIGGER_INTERVAL(int sid, int data);
extern int TCBread_PEDESTAL_TRIGGER_INTERVAL(int sid);
extern void TCBwrite_DISC_THR(int sid, int mid, int ch, int data);
extern int TCBread_DISC_THR(int sid, int mid, int ch);
extern void TCBsend_TRIG(int sid);
extern void TCBwrite_GATE_WIDTH(int sid, int mid, int data);
extern int TCBread_GATE_WIDTH(int sid, int mid);
extern void TCBwrite_TRIGGER_ENABLE(int sid, int data);
extern int TCBread_TRIGGER_ENABLE(int sid);
extern void TCBwrite_DELAY(int sid, int mid, int data);
extern int TCBread_DELAY(int sid, int mid);
extern void TCBmeasure_PED(int sid, int mid);
extern int TCBread_PED(int sid, int mid, int ch);
extern void TCBwrite_PEAK_SUM_WIDTH(int sid, int mid, int data);
extern int TCBread_PEAK_SUM_WIDTH(int sid, int mid);
extern void TCBwrite_ZERO_SUP(int sid, int mid, int data);
extern int TCBread_ZERO_SUP(int sid, int mid);
extern void TCBwrite_PRESCALE(int sid, int mid, int data);
extern int TCBread_PRESCALE(int sid, int mid);
extern void TCBread_TEMP(int sid, int mid, float *temp);
extern void TCBwrite_ECHO(int sid, int mid, int data);
extern int TCBread_ECHO(int sid, int mid);
extern void TCBread_LINK_STATUS(int sid, int *data);
extern void TCBread_MID(int sid, int *data);
extern void TCBinit_ADC(int sid, int mid);
extern void TCBinit_DRAM(int sid, int mid);

#ifdef __cplusplus
}
#endif




