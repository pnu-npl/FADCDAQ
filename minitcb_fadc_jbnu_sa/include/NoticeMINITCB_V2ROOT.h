#ifndef MINITCB_V2ROOT_H
#define MINITCB_V2ROOT_H

#include "TObject.h"
//#include "nkusbconst.h"

struct libusb_context;
//struct libusb_device;
//struct libusb_device_handle;

class NKMINITCB_V2 : public TObject {
public:
    
  NKMINITCB_V2();
  virtual ~NKMINITCB_V2();

  int MINITCB_V2open(char *ip);
  void MINITCB_V2close(int tcp_Handle);
  void MINITCB_V2reset(int tcp_Handle);
  void MINITCB_V2resetTIMER(int tcp_Handle);
  void MINITCB_V2start(int tcp_Handle);
  void MINITCB_V2stop(int tcp_Handle);
  unsigned long MINITCB_V2read_RUN(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_CW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_CW(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_RL(int tcp_Handle, unsigned long mid, unsigned long data);
  unsigned long MINITCB_V2read_RL(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_DRAMON(int tcp_Handle, unsigned long mid, unsigned long data);
  unsigned long MINITCB_V2read_DRAMON(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2measure_PED(int tcp_Handle, unsigned long mid, unsigned long ch);
  unsigned long MINITCB_V2read_PED(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_DLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_DLY(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_THR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_THR(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_POL(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_POL(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_PSW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_PSW(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_PCT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_PCT(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_PCI(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_PCI(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_PWT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_PWT(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_DT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_DT(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_TM(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_TM(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_TLT(int tcp_Handle, unsigned long mid, unsigned long data);
  unsigned long MINITCB_V2read_TLT(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2send_ADCRST(int tcp_Handle, unsigned long mid);
  void MINITCB_V2send_ADCCAL(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_ADCDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  void MINITCB_V2write_ADCALIGN(int tcp_Handle, unsigned long mid, unsigned long data);
  unsigned long MINITCB_V2read_ADCSTAT(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_BITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  void MINITCB_V2write_FMUX(int tcp_Handle, unsigned long mid, unsigned long ch);
  unsigned long MINITCB_V2read_FMUX(int tcp_Handle, unsigned long mid);
  void MINITCB_V2arm_FADC(int tcp_Handle, unsigned long mid);
  unsigned long MINITCB_V2read_FREADY(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_ZSFD(int tcp_Handle, unsigned long mid, unsigned long data);
  unsigned long MINITCB_V2read_ZSFD(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_DSR(int tcp_Handle, unsigned long mid, unsigned long data);
  unsigned long MINITCB_V2read_DSR(int tcp_Handle, unsigned long mid);
  void MINITCB_V2read_FADCBUF(int tcp_Handle, unsigned long mid, unsigned long *data);
  void MINITCB_V2_ADCALIGN(int tcp_Handle, unsigned long mid);
  void MINITCB_V2_ADCALIGN_64(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_PSS(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_PSS(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_RT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_RT(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_SR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_SR(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_ST(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_ST(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_PT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
  unsigned long MINITCB_V2read_PT(int tcp_Handle, unsigned long mid, unsigned long ch);
  void MINITCB_V2write_RUNNO(int tcp_Handle, unsigned long data);
  unsigned long MINITCB_V2read_RUNNO(int tcp_Handle);
  void MINITCB_V2send_TRIG(int tcp_Handle);
  unsigned long MINITCB_V2read_LNSTAT(int tcp_Handle);
  unsigned long MINITCB_V2read_MIDS(int tcp_Handle, unsigned long ch);
  void MINITCB_V2write_PTRIG(int tcp_Handle, unsigned long data);
  unsigned long MINITCB_V2read_PTRIG(int tcp_Handle);
  void MINITCB_V2write_TRIGENABLE(int tcp_Handle, unsigned long mid, unsigned long data);
  unsigned long MINITCB_V2read_TRIGENABLE(int tcp_Handle, unsigned long mid);
  void MINITCB_V2write_MTHR(int tcp_Handle, unsigned long data);
  unsigned long MINITCB_V2read_MTHR(int tcp_Handle);
  void MINITCB_V2write_PSCALE(int tcp_Handle, unsigned long data);
  unsigned long MINITCB_V2read_PSCALE(int tcp_Handle);
  void MINITCB_V2_ADCALIGN_DRAM(int tcp_Handle, unsigned long mid);

  ClassDef(NKMINITCB_V2, 0) // NKMINITCB_V2 wrapper class for root
};

#endif
