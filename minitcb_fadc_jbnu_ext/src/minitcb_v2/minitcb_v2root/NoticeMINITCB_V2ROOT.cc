#include "NoticeMINITCB_V2ROOT.h"
#include "NoticeMINITCB_V2.h"

ClassImp(NKMINITCB_V2)

NKMINITCB_V2::NKMINITCB_V2()
{
}

NKMINITCB_V2::~NKMINITCB_V2()
{
}

int NKMINITCB_V2::MINITCB_V2open(char *ip)
{
    return ::MINITCB_V2open(ip);
}

void NKMINITCB_V2::MINITCB_V2close(int tcp_Handle)
{
    ::MINITCB_V2close(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2reset(int tcp_Handle)
{
    ::MINITCB_V2reset(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2resetTIMER(int tcp_Handle)
{
    ::MINITCB_V2resetTIMER(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2start(int tcp_Handle)
{
    ::MINITCB_V2start(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2stop(int tcp_Handle)
{
    ::MINITCB_V2stop(tcp_Handle);
}

unsigned long NKMINITCB_V2::MINITCB_V2read_RUN(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_RUN(tcp_Handle, mid);
}

void NKMINITCB_V2::MINITCB_V2write_CW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_CW(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_CW(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_CW(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_RL(int tcp_Handle, unsigned long mid, unsigned long data)
{
    ::MINITCB_V2write_RL(tcp_Handle, mid, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_RL(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_RL(tcp_Handle, mid);
}

void NKMINITCB_V2::MINITCB_V2write_DRAMON(int tcp_Handle, unsigned long mid, unsigned long data)
{
    ::MINITCB_V2write_DRAMON(tcp_Handle, mid, data);
}

unsigned long NKMINITCB_V2::MINITCB_V2read_DRAMON(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_DRAMON(tcp_Handle, mid);
}

void NKMINITCB_V2::MINITCB_V2write_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_DACOFF(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_DACOFF(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2measure_PED(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2measure_PED(tcp_Handle, mid, ch);
}

unsigned long NKMINITCB_V2::MINITCB_V2read_PED(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_PED(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_DLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_DLY(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_DLY(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_DLY(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_AMOREDLY(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_AMOREDLY(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_THR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_THR(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_THR(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_THR(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_POL(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_POL(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_POL(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_POL(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_PSW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_PSW(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_PSW(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_PSW(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_AMODE(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_AMODE(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_PCT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_PCT(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_PCT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_PCT(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_PCI(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_PCI(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_PCI(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_PCI(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_PWT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_PWT(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_PWT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_PWT(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_DT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_DT(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_DT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_DT(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_TM(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data) 
{
    ::MINITCB_V2write_TM(tcp_Handle, mid, ch, data);
}
unsigned long NKMINITCB_V2::MINITCB_V2read_TM(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_TM(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2write_TLT(int tcp_Handle, unsigned long mid, unsigned long data)
{
    ::MINITCB_V2write_TLT(tcp_Handle, mid, data);
} 
unsigned long NKMINITCB_V2::MINITCB_V2read_TLT(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_TLT(tcp_Handle, mid);
}

void NKMINITCB_V2::MINITCB_V2write_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_ZEROSUP(tcp_Handle, mid, ch, data);
} 
unsigned long NKMINITCB_V2::MINITCB_V2read_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_ZEROSUP(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::MINITCB_V2send_ADCRST(int tcp_Handle, unsigned long mid)
{
    ::MINITCB_V2send_ADCRST(tcp_Handle, mid);
}
 
void NKMINITCB_V2::MINITCB_V2send_ADCCAL(int tcp_Handle, unsigned long mid)
{
    ::MINITCB_V2send_ADCCAL(tcp_Handle, mid);
} 

void NKMINITCB_V2::MINITCB_V2write_ADCDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_ADCDLY(tcp_Handle, mid, ch, data);
} 

void NKMINITCB_V2::MINITCB_V2write_ADCALIGN(int tcp_Handle, unsigned long mid, unsigned long data)
{
    ::MINITCB_V2write_ADCALIGN(tcp_Handle, mid, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_ADCSTAT(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_ADCSTAT(tcp_Handle, mid);
} 

void NKMINITCB_V2::MINITCB_V2write_BITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_BITSLIP(tcp_Handle, mid, ch, data);
} 

void NKMINITCB_V2::MINITCB_V2write_FMUX(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    ::MINITCB_V2write_FMUX(tcp_Handle, mid, ch);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_FMUX(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_FMUX(tcp_Handle, mid);
} 

void NKMINITCB_V2::MINITCB_V2arm_FADC(int tcp_Handle, unsigned long mid)
{
    ::MINITCB_V2arm_FADC(tcp_Handle, mid);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_FREADY(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_FREADY(tcp_Handle, mid);
} 

void NKMINITCB_V2::MINITCB_V2write_ZSFD(int tcp_Handle, unsigned long mid, unsigned long data)
{
    ::MINITCB_V2write_ZSFD(tcp_Handle, mid, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_ZSFD(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_ZSFD(tcp_Handle, mid);
} 

void NKMINITCB_V2::MINITCB_V2write_DSR(int tcp_Handle, unsigned long mid, unsigned long data)
{
    ::MINITCB_V2write_DSR(tcp_Handle, mid, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_DSR(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_DSR(tcp_Handle, mid);
} 

void NKMINITCB_V2::MINITCB_V2read_FADCBUF(int tcp_Handle, unsigned long mid, unsigned long *data)
{
    ::MINITCB_V2read_FADCBUF(tcp_Handle, mid, data);
} 

void NKMINITCB_V2::MINITCB_V2_ADCALIGN(int tcp_Handle, unsigned long mid)
{
	::MINITCB_V2_ADCALIGN(tcp_Handle, mid);
}

void NKMINITCB_V2::MINITCB_V2_ADCALIGN_64(int tcp_Handle, unsigned long mid)
{
	::MINITCB_V2_ADCALIGN_64(tcp_Handle, mid);
}

void NKMINITCB_V2::MINITCB_V2write_PSS(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_PSS(tcp_Handle, mid, ch, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_PSS(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_PSS(tcp_Handle, mid, ch);
} 


void NKMINITCB_V2::MINITCB_V2write_RT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_RT(tcp_Handle, mid, ch, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_RT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_RT(tcp_Handle, mid, ch);
} 


void NKMINITCB_V2::MINITCB_V2write_SR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_SR(tcp_Handle, mid, ch, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_SR(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_SR(tcp_Handle, mid, ch);
} 


void NKMINITCB_V2::MINITCB_V2write_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_DACGAIN(tcp_Handle, mid, ch, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_DACGAIN(tcp_Handle, mid, ch);
} 

void NKMINITCB_V2::MINITCB_V2write_ST(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_ST(tcp_Handle, mid, ch, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_ST(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_ST(tcp_Handle, mid, ch);
} 


void NKMINITCB_V2::MINITCB_V2write_PT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
    ::MINITCB_V2write_PT(tcp_Handle, mid, ch, data);
} 

unsigned long NKMINITCB_V2::MINITCB_V2read_PT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
    return ::MINITCB_V2read_PT(tcp_Handle, mid, ch);
} 


void NKMINITCB_V2::MINITCB_V2write_RUNNO(int tcp_Handle, unsigned long data)
{
    ::MINITCB_V2write_RUNNO(tcp_Handle, data);
}

unsigned long  NKMINITCB_V2::MINITCB_V2read_RUNNO(int tcp_Handle)
{
    return ::MINITCB_V2read_RUNNO(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2send_TRIG(int tcp_Handle)
{
    ::MINITCB_V2send_TRIG(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2write_PTRIG(int tcp_Handle, unsigned long data)
{
    ::MINITCB_V2write_PTRIG(tcp_Handle, data);
}

unsigned long  NKMINITCB_V2::MINITCB_V2read_PTRIG(int tcp_Handle)
{
    return ::MINITCB_V2read_PTRIG(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2write_TRIGENABLE(int tcp_Handle, unsigned long mid, unsigned long data)
{
    ::MINITCB_V2write_TRIGENABLE(tcp_Handle, mid, data);
}

unsigned long  NKMINITCB_V2::MINITCB_V2read_TRIGENABLE(int tcp_Handle, unsigned long mid)
{
    return ::MINITCB_V2read_TRIGENABLE(tcp_Handle, mid);
}

void NKMINITCB_V2::MINITCB_V2write_MTHR(int tcp_Handle, unsigned long data)
{
    ::MINITCB_V2write_MTHR(tcp_Handle, data);
}

unsigned long  NKMINITCB_V2::MINITCB_V2read_MTHR(int tcp_Handle)
{
    return ::MINITCB_V2read_MTHR(tcp_Handle);
}

void NKMINITCB_V2::MINITCB_V2write_PSCALE(int tcp_Handle, unsigned long data)
{
    ::MINITCB_V2write_PSCALE(tcp_Handle, data);
}

unsigned long  NKMINITCB_V2::MINITCB_V2read_PSCALE(int tcp_Handle)
{
    return ::MINITCB_V2read_PSCALE(tcp_Handle);
}

unsigned long NKMINITCB_V2::MINITCB_V2read_LNSTAT(int tcp_Handle)
{
    return ::MINITCB_V2read_LNSTAT(tcp_Handle);
}

unsigned long NKMINITCB_V2::MINITCB_V2read_MIDS(int tcp_Handle, unsigned long ch)
{
    return ::MINITCB_V2read_MIDS(tcp_Handle, ch);
}

void NKMINITCB_V2::MINITCB_V2_ADCALIGN_DRAM(int tcp_Handle, unsigned long mid)
{
    ::MINITCB_V2_ADCALIGN_DRAM(tcp_Handle, mid);
}

//ckim, Mar. 1st
void NKMINITCB_V2::TCBwrite_HV(int tcp_Handle, unsigned long mid, int ch, float data)
{
    ::TCBwrite_HV(tcp_Handle, mid, ch, data);
}

float NKMINITCB_V2::TCBread_HV(int tcp_Handle, unsigned long mid, int ch)
{
    return::TCBread_HV(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::TCBwrite_COIN_WIDTH(int tcp_Handle, unsigned long mid, int data)
{
    ::TCBwrite_COIN_WIDTH(tcp_Handle, mid, data);
}

int NKMINITCB_V2::TCBread_COIN_WIDTH(int tcp_Handle, unsigned long mid)
{
    return::TCBread_COIN_WIDTH(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBwrite_MULTI_THR(int tcp_Handle, unsigned long mid, int data)
{
    ::TCBwrite_MULTI_THR(tcp_Handle, mid, data);
}

int NKMINITCB_V2::TCBread_MULTI_THR(int tcp_Handle, unsigned long mid)
{
    return::TCBread_MULTI_THR(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBwrite_DISC_THR(int tcp_Handle, unsigned long mid, int ch, int data)
{
    ::TCBwrite_DISC_THR(tcp_Handle, mid, ch, data);
}

int NKMINITCB_V2::TCBread_DISC_THR(int tcp_Handle, unsigned long mid, int ch)
{
    return::TCBread_DISC_THR(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::TCBwrite_GATE_WIDTH(int tcp_Handle, unsigned long mid, int data)
{
    ::TCBwrite_GATE_WIDTH(tcp_Handle, mid, data);
}

int NKMINITCB_V2::TCBread_GATE_WIDTH(int tcp_Handle, unsigned long mid)
{
    return::TCBread_GATE_WIDTH(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBwrite_DELAY(int tcp_Handle, unsigned long mid, int data)
{
    ::TCBwrite_DELAY(tcp_Handle, mid, data);
}

int NKMINITCB_V2::TCBread_DELAY(int tcp_Handle, unsigned long mid)
{
    return::TCBread_DELAY(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBmeasure_PED(int tcp_Handle, unsigned long mid)
{
    ::TCBmeasure_PED(tcp_Handle, mid);
}

int NKMINITCB_V2::TCBread_PED(int tcp_Handle, unsigned long mid, int ch)
{
    return::TCBread_PED(tcp_Handle, mid, ch);
}

void NKMINITCB_V2::TCBwrite_PEAK_SUM_WIDTH(int tcp_Handle, unsigned long mid, int data)
{
    ::TCBwrite_PEAK_SUM_WIDTH(tcp_Handle, mid, data);
}

int NKMINITCB_V2::TCBread_PEAK_SUM_WIDTH(int tcp_Handle, unsigned long mid)
{
    return::TCBread_PEAK_SUM_WIDTH(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBwrite_ZERO_SUP(int tcp_Handle, unsigned long mid, int data)
{
    ::TCBwrite_ZERO_SUP(tcp_Handle, mid, data);
}

int NKMINITCB_V2::TCBread_ZERO_SUP(int tcp_Handle, unsigned long mid)
{
    return::TCBread_ZERO_SUP(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBwrite_PRESCALE(int tcp_Handle, unsigned long mid, int data)
{
    ::TCBwrite_PRESCALE(tcp_Handle, mid, data);
}

int NKMINITCB_V2::TCBread_PRESCALE(int tcp_Handle, unsigned long mid)
{
    return::TCBread_PRESCALE(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBread_TEMP(int tcp_Handle, unsigned long mid, float *temp)
{
    ::TCBread_TEMP(tcp_Handle, mid, temp);
}

void NKMINITCB_V2::TCBinit_ADC(int tcp_Handle, unsigned long mid)
{
    ::TCBinit_ADC(tcp_Handle, mid);
}

void NKMINITCB_V2::TCBinit_DRAM(int tcp_Handle, unsigned long mid)
{
    ::TCBinit_DRAM(tcp_Handle, mid);
}


//int NKMINITCB_V2::TCBread_PED_JBNU(int tcp_Handle, int mid, int ch)
//{
//    return::TCBread_PED_JBNU(tcp_Handle, mid, ch);
//}
