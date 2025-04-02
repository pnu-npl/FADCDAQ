#ifndef MINITCB_V2_H
#define MINITCB_V2_H

#define MAX_TCP_CONNECT         5       /* time in secs to get a connection */
#define MAX_TCP_READ            3       /* time in secs to wait for the DSO
                                           to respond to a read request */
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0

#ifdef __cplusplus
extern  "C" {
#endif

extern int MINITCB_V2open(char *ip);
extern void MINITCB_V2close(int tcp_Handle);
extern int MINITCB_V2transmit(int tcp_Handle, char *buf, int len);
extern int MINITCB_V2receive(int tcp_Handle, char *buf, int len);
extern void MINITCB_V2write(int tcp_Handle, unsigned long address, unsigned long data);
extern unsigned long MINITCB_V2read(int tcp_Handle, unsigned long address);
extern void MINITCB_V2write_Module(int tcp_Handle, unsigned long mid, unsigned long address, unsigned long data);
extern unsigned long MINITCB_V2read_Module(int tcp_Handle, unsigned long mid, unsigned long address);
extern void MINITCB_V2reset(int tcp_Handle);
extern void MINITCB_V2resetTIMER(int tcp_Handle);
extern void MINITCB_V2start(int tcp_Handle);
extern void MINITCB_V2stop(int tcp_Handle);
extern unsigned long MINITCB_V2read_RUN(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_CW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_CW(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_RL(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_RL(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_DRAMON(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_DRAMON(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2measure_PED(int tcp_Handle, unsigned long mid, unsigned long ch);
extern unsigned long MINITCB_V2read_PED(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_DLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_DLY(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_THR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_THR(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_POL(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_POL(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_PSW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_PSW(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_PCT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_PCT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_PCI(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_PCI(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_PWT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_PWT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_DT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_DT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_TM(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_TM(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_TLT(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_TLT(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_STLT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_STLT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2send_ADCRST(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2send_ADCCAL(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_ADCDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern void MINITCB_V2write_ADCALIGN(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_ADCSTAT(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_BITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern void MINITCB_V2write_FMUX(int tcp_Handle, unsigned long mid, unsigned long ch);
extern unsigned long MINITCB_V2read_FMUX(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2arm_FADC(int tcp_Handle, unsigned long mid);
extern unsigned long MINITCB_V2read_FREADY(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_ZSFD(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_ZSFD(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_DSR(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_DSR(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2read_FADCBUF(int tcp_Handle, unsigned long mid, unsigned long *data);
extern void MINITCB_V2_ADCALIGN(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2_ADCALIGN_64(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_PSS(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_PSS(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_RT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_RT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_SR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_SR(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_ST(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_ST(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_PT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCB_V2read_PT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_RUNNO(int tcp_Handle, unsigned long data);
extern unsigned long MINITCB_V2read_RUNNO(int tcp_Handle);
extern void MINITCB_V2send_TRIG(int tcp_Handle);
extern unsigned long MINITCB_V2read_LNSTAT(int tcp_Handle);
extern unsigned long MINITCB_V2read_MIDS(int tcp_Handle, unsigned long ch);
extern void MINITCB_V2write_PTRIG(int tcp_Handle, unsigned long data);
extern unsigned long MINITCB_V2read_PTRIG(int tcp_Handle);
extern void MINITCB_V2write_TRIGENABLE(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_TRIGENABLE(int tcp_Handle, unsigned long mid);
extern void MINITCB_V2write_MTHR(int tcp_Handle, unsigned long data);
extern unsigned long MINITCB_V2read_MTHR(int tcp_Handle);
extern void MINITCB_V2write_PSCALE(int tcp_Handle, unsigned long data);
extern unsigned long MINITCB_V2read_PSCALE(int tcp_Handle);
extern void MINITCB_V2write_DRAMDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern void MINITCB_V2write_DRAMBITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2write_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCB_V2read_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_V2_ADCALIGN_DRAM(int tcp_Handle, unsigned long mid);

extern void TCBwrite_ACQ_TIME(int tcp_Handle, int data);
extern int TCBread_ACQ_TIME(int tcp_Handle);
extern void TCBwrite_HV(int tcp_Handle, unsigned long mid, int ch, float data);
extern float TCBread_HV(int tcp_Handle, unsigned long mid, int ch);
extern void TCBwrite_COIN_WIDTH(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_COIN_WIDTH(int tcp_Handle, unsigned long mid);
extern void TCBwrite_MULTI_THR(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_MULTI_THR(int tcp_Handle, unsigned long mid);
extern void TCBwrite_PEDESTAL_TRIGGER_INTERVAL(int tcp_Handle, int data);
extern int TCBread_PEDESTAL_TRIGGER_INTERVAL(int tcp_Handle);
extern void TCBwrite_DISC_THR(int tcp_Handle, unsigned long mid, int ch, int data);
extern int TCBread_DISC_THR(int tcp_Handle, unsigned long mid, int ch);
extern void TCBsend_TRIG(int tcp_Handle);
extern void TCBwrite_GATE_WIDTH(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_GATE_WIDTH(int tcp_Handle, unsigned long mid);
extern void TCBwrite_TRIGGER_ENABLE(int tcp_Handle, int data);
extern int TCBread_TRIGGER_ENABLE(int tcp_Handle);
extern void TCBwrite_DELAY(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_DELAY(int tcp_Handle, unsigned long mid);
extern void TCBmeasure_PED(int tcp_Handle, unsigned long mid);
extern int TCBread_PED(int tcp_Handle, unsigned long mid, int ch);
extern void TCBwrite_PEAK_SUM_WIDTH(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_PEAK_SUM_WIDTH(int tcp_Handle, unsigned long mid);
extern void TCBwrite_ZERO_SUP(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_ZERO_SUP(int tcp_Handle, unsigned long mid);
extern void TCBwrite_PRESCALE(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_PRESCALE(int tcp_Handle, unsigned long mid);
extern void TCBread_TEMP(int tcp_Handle, unsigned long mid, float *temp);
extern void TCBwrite_ECHO(int tcp_Handle, unsigned long mid, int data);
extern int TCBread_ECHO(int tcp_Handle, unsigned long mid);
extern void TCBread_LINK_STATUS(int tcp_Handle, int *data);
extern void TCBread_MID(int tcp_Handle, int *data);
extern void TCBinit_ADC(int tcp_Handle, unsigned long mid);
extern void TCBinit_DRAM(int tcp_Handle, unsigned long mid);

#ifdef __cplusplus
}
#endif

#endif
