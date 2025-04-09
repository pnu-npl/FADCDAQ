#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <fstream>
#include <getopt.h>
#include <iostream>

#include "TROOT.h"
#include "NoticeMINITCB_V2ROOT.h"
using namespace std;

int GetIdProduct(void)
{
	const char* NKFADC500 = "lsusb -d 0547:1502";
	const char* JBNU_DAQ  = "lsusb -d 0547:2209";
	const char* BIC_DAQ   = "lsusb -d 0547:2503";

	if      ( system(NKFADC500) == 0 ) return 1502;
	else if ( system(JBNU_DAQ)  == 0 ) return 2209;
	else if ( system(BIC_DAQ)   == 0 ) return 2503;
	else cout <<"GetIdProduct: unknown type of DAQ!\n";

	return -999;
}//GetIdProduct

int main(int argc, char** argv)
{
	// FADC module ID starts from " 1 ":  let minitcb slots be the same
	// Prefixes: 't' for minitcb setting, 'f' for FADC setting, and 'm' for temporary

	// TCB setting
	// --------------------------------------------------

	unsigned long tCW     = 32;     // TCB coincidence width (8 ~ 32760)
	unsigned long tMTHR   = 1;      // Multiplicity trigger threshold @ MINITCB_V2
	unsigned long tPSCALE = 1;      // Trigger input prescaler @ MINITCB_V2
	unsigned long tPTRIG  = 1;      // Pedestal trigger interval in ms, 0 for disable

	unsigned long tTrigOn_self = 0; // D0: when 1 enables self trigger
	unsigned long tTrigOn_ped  = 0; // D1: when 1 enables pedestal trigger
	unsigned long tTrigOn_soft = 0; // D2: when 1 enables software trigger
	unsigned long tTrigOn_ext  = 1; // D3: when 1 enables external trigger
	unsigned long tTrigOn = (tTrigOn_ext<<3) | (tTrigOn_soft<<2) | (tTrigOn_ped<<1) | tTrigOn_self;

	// NKFADC500 setting: every timing setting unit is ns
	// --------------------------------------------------

	// When 1 enable local trigger as a self trigger
	unsigned long fTrigOn_local = 0;
	if (tTrigOn_self==false && fTrigOn_local==true)
	{
		cout <<"WARNING! self trigger (tcb) is OFF but local trigger (FADC) is ON\n";
		return 1;
	}
	const int nMID = 2; //# of available FADC modules
	const int nCh  = 4; //# of channels per FADC module

	// Recording lengt (ckim, updated 241114)h 
	// ns scale: 1=128 (ns), 2=256, and 4=512
	// us scale: 8=1 (us), 16=2, 32=4, 64=8, 128=16, and 256=32
	unsigned long fRL = 16;

	// Trigger lookup table value, any: FFFE, 1&3&2&4: 8000, 1&2|3&4: F888, 1&3|2&4: ECA0
	unsigned long fTLT[nMID] = {0xFFFE, 0xFFFE};

	unsigned long fAMODE   = 1;   // ADC mode: 0 = raw, 1 = filtered
	unsigned long fCW      = 32;  // Coincidence width: 8 ~ 32760
	unsigned long fDACOFF  = 3000; // ADC offset value: 0 ~ 4095
	unsigned long fPOL     = 0;	  // Input pulse polarity: 0 = negative, 1 = positive
	unsigned long fPCI     = 32;  // Pulse count interval: 32 ~ 8160 ns
	unsigned long fPCT     = 1;	  // Pulse count threshold: 1 ~ 15
	unsigned long fPWT     = 100; // Pulse width threshold: 2 ~ 1022 ns
	unsigned long fPSW     = 2;	  // Peak sum width: 2 ~ 16382 ns
	unsigned long fDT      = 20;  // Trigger deadtime: 0 ~ 8355840 ns
	unsigned long fZEROSUP = 0;	  // Zero-suppression: 0 = not use, 1 = use (* ckim: NOT working)

	// ADC waveform delay from trigger point: 0 ~ 31992
	unsigned long fDLY[nMID][nCh] = //{{0.}};
	{
		// By default +80 will be added as intrinsic delay of device (e.g. 64 = 64 + 80)
		// Give the number can be devided by 2 (e.g., 48 for ~50, 72 for ~75)
		{400, 400, 400, 400},
		{400, 400, 400, 400}
	};

	// Discrimination threshold
	// 1 ~ 4095 for pulse height threshold: 4,095 vs. 2 V (dynamic range) -> 100 ~ 50 mV 
	// 1 ~ 67108863 for peak sum threshold
	unsigned long fTHR[nMID][nCh] =
	{
		{100, 100, 100, 100},
		{100, 100, 100, 100}
	};

	// Set trigger mode
	unsigned long fTM_PC   = 1; // When 1 enables pulse count trigger 
	unsigned long fTM_PW   = 0; // When 1 enables pulse width trigger
	unsigned long fTM_PS   = 0; // When 1 enables peak sum trigger
	unsigned long fTM_PSOR = 0; // When 1 enables peak sum OR trigger
	unsigned long fTM = (fTM_PSOR<<3) | (fTM_PS<<2) | (fTM_PW<<1) | fTM_PC;

	// FADC Options parsing (written by JWLee @ KU)
	// --------------------------------------------------

	//Local variables
	string mIPAddr = "192.168.0.2";
	int mRunNumber = 0;
	int mTrigOpt = 0;

	while (1)
	{
		static struct option long_options[] =
		{
			{"RunNumber",     required_argument, 0, 'r'},
			{"IPAddr",        required_argument, 0, 'i'},
			{"Recording_len", required_argument, 0, 'l'},
			{"trig_option",   required_argument, 0, 't'},
			{"threshold",     required_argument, 0, 's'},
			{"zerosup",       required_argument, 0, 'z'},
			{"Offset",        required_argument, 0, 'o'},
			{"delay",         required_argument, 0, 'd'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		int c = getopt_long(argc, argv, "r:i:l:t:s:z:o:d", long_options, &option_index);
		if ( c == -1 ) { break;}

		switch ( c )
		{
			case 0:
				if ( long_options[option_index].flag != 0 ) { break; }
				printf("option %s", long_options[option_index].name);
				if ( optarg ) { printf( "with arg %s", optarg); }
				printf("\n");
				break;

			case 'r': 
				mRunNumber = std::atoi( optarg );
				break;

			case 'i':
				mIPAddr = optarg;
				break;

			case 'l':
				fRL = std::atoi( optarg );
				break;

			case 't':
				mTrigOpt = std::atoi( optarg );
				if      ( mTrigOpt == 0 ) { for (int i=0; i<nMID; i++) fTLT[i] = 0x0000; } // External
				else if ( mTrigOpt == 1 ) { for (int i=0; i<nMID; i++) fTLT[i] = 0xFFFE; } // Self, any
				else if ( mTrigOpt == 2 ) { for (int i=0; i<nMID; i++) fTLT[i] = 0xF888; } // 1&2 || 3&4
				else if ( mTrigOpt == 3 ) { for (int i=0; i<nMID; i++) fTLT[i] = 0xECA0; } // 1&3 || 2&4
				else if ( mTrigOpt == 4 ) { for (int i=0; i<nMID; i++) fTLT[i] = 0xEEE0; } // 1||2 & 3||4
				else { for (int i=0; i<nMID; i++) fTLT[i] = 0x0000; }
				break;

			case 's':
				for (int i=0; i<nMID; i++)
					for (int j=0; j<nCh;  j++) fTHR[i][j] = std::atoi( optarg );
				break;

			case 'z':
				fZEROSUP = std::atoi( optarg );
				break;

			case 'o':
				fDACOFF = std::atoi( optarg ); 
				break;

			case 'd':
				for (int i=0; i<nMID; i++)
					for (int j=0; j<nCh;  j++) fDLY[i][j] = std::atoi( optarg );

			default:
				abort();
		}
	}//Options

	// JBNU daq Settings
	//-----------------------------------------------------

	const float hv1 = 47; //High voltage for FEE1, 0 ~ 48 V
	const float hv2 = 47; //High voltage for FEE2, 0 ~ 48 V

	const int disc_thr[32] = //Discriminator threshold, 1 ~ 4095
	{
		//FEE1 (1-16) and FEE2 (17-32)
		100, 100, 100, 100, 100, 100, 100, 100, //100-8
		100, 100, 100, 100, 100, 100, 100, 100, //9-1006
		100, 100, 100, 100, 100, 100, 100, 100, //1007-24
		100, 100, 100, 100, 100, 100, 100, 100  //25-32
	};

	const int multi_thr_daq  = 2; //DAQ Multiplicity threshold, 1 ~ 32
	//const int multi_thr_tcb  = 1; //TCB Multiplicity threshold, 1 ~ 1,280
	//const int trig_enable    = 8; //Trigger mode, 1=self, 2=pedestal, 4=software, 8=external (15 for enable all)
	//const int ptrig_interval = 1000; //Pedestal trigger interval in ms, 0 ~ 65535 ms, 0 for disable

	const int gate_width     = 1;    //Gate width, 1/2/4/8/16 us
	const int coin_width     = 100;  //Coincidence width, 16 ~ 16,000 ns
	const int input_delay    = ((float)gate_width/2)*1000; //Input delay, 0 ~ 16,000 ns
	const int peak_sum_width = 500;  //Peak sum width, 16 ~ 16,000 ns

	const int zero_sup = 0; //Zero suppression for waveform, 0 = not suppress, 1 = suppress
	const int prescale = 1; //ADC waveform prescale, 1 ~ 65535, 0 for disable waveform save

	// TCB init
	// --------------------------------------------------

	// Define USB3TCB class
	NKMINITCB_V2 *tcb = new NKMINITCB_V2;

	// Open and Reset TCB
	int tcp_Handle = tcb->MINITCB_V2open((char*)mIPAddr.c_str()); // MINITCB_V2 USB3 tcp_Handle
	tcb->MINITCB_V2reset(tcp_Handle);

	// Get link status
	unsigned long data = tcb->MINITCB_V2read_LNSTAT(tcp_Handle);
	if (data == 0)
	{ 
		tcb->MINITCB_V2reset(tcp_Handle); 
		tcb->MINITCB_V2close(tcp_Handle); 
		cout <<"Found no module" <<endl;
		return 1;
	}

	// Temporary variables
	const int mTCBSlots = 4; //Mar. 1st: 1, 2 for FADC; 3 for JBNU
	bool mLinked[mTCBSlots] = {false};  // Link status of each minitcb slot
	unsigned long mMIDValue[mTCBSlots]; // FADC module ID

	//Search for linked channel, get MID if found, and then set it up: first come, first served
	for (int i=0; i<mTCBSlots; i++)
	{
		mLinked[i] = (data >> i) & 0x01;
		if (mLinked[i] == false)
		{
			mMIDValue[i] = -1;
			continue;
		}

		mMIDValue[i] = tcb->MINITCB_V2read_MIDS(tcp_Handle, i+1); //mID and its ch starts from 1
		const int idProduct = GetIdProduct(); //Apr. 9, 2025
		if (idProduct == -999) continue;

		unsigned long fMID = mMIDValue[i];
		tcb->MINITCB_V2reset(tcp_Handle);

		//minitcb
		printf("MINITCB_V2\n");
		printf("- %-25s: %ld\n", "Coincidence width", tcb->MINITCB_V2read_CW(tcp_Handle,0,1));
		printf("- %-25s: %ld\n", "Multiplicity threshold", tcb->MINITCB_V2read_MTHR(tcp_Handle));
		printf("- %-25s: %ld\n", "Prescale on trig input", tcb->MINITCB_V2read_PSCALE(tcp_Handle));
		printf("- %-25s: %ld\n", "Ped. trig interval (ms)", tcb->MINITCB_V2read_PTRIG(tcp_Handle));
		printf("- %-25s: %ld\n", "Trigger mode (TCB)", tcb->MINITCB_V2read_TRIGENABLE(tcp_Handle, 0));

		//FADC
		//-----------------------------------

		if (idProduct == 1502) //NKFADC500
		{
			printf("\nNKFADC500 (mID: %ld) found @ minitcb %d \n", mMIDValue[i], i+1);
			tcb->MINITCB_V2write_CW        (tcp_Handle, 0, 1, tCW);
			tcb->MINITCB_V2write_MTHR      (tcp_Handle, tMTHR);
			tcb->MINITCB_V2write_PSCALE    (tcp_Handle, tPSCALE);
			tcb->MINITCB_V2write_PTRIG     (tcp_Handle, tPTRIG);
			tcb->MINITCB_V2write_TRIGENABLE(tcp_Handle, 0, tTrigOn);

			tcb->MINITCB_V2write_DRAMON    (tcp_Handle, fMID, 1);
			tcb->MINITCB_V2write_RL        (tcp_Handle, fMID, fRL);
			tcb->MINITCB_V2write_TLT       (tcp_Handle, fMID, fTLT[fMID-1]);
			tcb->MINITCB_V2write_TRIGENABLE(tcp_Handle, fMID, fTrigOn_local);

			for (int j=0; j<nCh; j++)
			{
				tcb->MINITCB_V2measure_PED  (tcp_Handle, fMID, j+1);
				tcb->MINITCB_V2write_AMODE  (tcp_Handle, fMID, j+1, fAMODE);
				tcb->MINITCB_V2write_CW     (tcp_Handle, fMID, j+1, fCW);
				tcb->MINITCB_V2write_DACOFF (tcp_Handle, fMID, j+1, fDACOFF);
				tcb->MINITCB_V2write_DLY    (tcp_Handle, fMID, j+1, fDLY[fMID-1][j]);
				tcb->MINITCB_V2write_DT     (tcp_Handle, fMID, j+1, fDT);
				tcb->MINITCB_V2write_PCI    (tcp_Handle, fMID, j+1, fPCI);
				tcb->MINITCB_V2write_PCT    (tcp_Handle, fMID, j+1, fPCT);
				tcb->MINITCB_V2write_POL    (tcp_Handle, fMID, j+1, fPOL);
				tcb->MINITCB_V2write_PSW    (tcp_Handle, fMID, j+1, fPSW);
				tcb->MINITCB_V2write_PWT    (tcp_Handle, fMID, j+1, fPWT);
				tcb->MINITCB_V2write_THR    (tcp_Handle, fMID, j+1, fTHR[fMID-1][j]);
				tcb->MINITCB_V2write_TM     (tcp_Handle, fMID, j+1, fTM);
				tcb->MINITCB_V2write_ZEROSUP(tcp_Handle, fMID, j+1, fZEROSUP);
			}//j, FADC channels
			tcb->MINITCB_V2write_POL(tcp_Handle, fMID, 5, fPOL);

			tcb->MINITCB_V2_ADCALIGN     (tcp_Handle, fMID);
			tcb->MINITCB_V2_ADCALIGN_DRAM(tcp_Handle, fMID);

			//+++++++++++++++++++++++++++++++++++++++

			//FADC common
			printf("FADC (mID: %ld)\n", fMID);
			printf("- %-25s: %ld\n", "DRAM_ON", tcb->MINITCB_V2read_DRAMON(tcp_Handle, fMID));
			printf("- %-25s: %ld\n", "Recording length", tcb->MINITCB_V2read_RL(tcp_Handle, fMID));
			printf("- %-25s: %lX\n", "Trigger LUT value", tcb->MINITCB_V2read_TLT(tcp_Handle, fMID));
			printf("- %-25s: %ld\n", "Local trigger on", tcb->MINITCB_V2read_TRIGENABLE(tcp_Handle, fMID));

			//FADC ch by ch
			printf("- %-25s:", "ADC mode");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_AMODE(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "ADC offset");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_DACOFF(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Pedestal");
			for (int j=0; j<4; j++) printf("%5ld", tcb->MINITCB_V2read_PED(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Coincidence width");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_CW(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Wavefrom delay");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_DLY(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Input pulse polarity");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_POL(tcp_Handle, fMID, j+1));
			printf("%5ld (TDC)", tcb->MINITCB_V2read_POL(tcp_Handle, fMID, 5)); //TDC
			printf("\n- %-25s:", "Pulse count interval (ns)");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_PCI(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Pulse count threshold");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_PCT(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Pulse width threshold");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_PWT(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Peak sum width");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_PSW(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Trigger deadtime (ns)");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_DT(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Trigger mode (FADC)");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_TM(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Discrimination threshold");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_THR(tcp_Handle, fMID, j+1));
			printf("\n- %-25s:", "Zero suppression");
			for (int j=0; j<4; j++)	printf("%5ld", tcb->MINITCB_V2read_ZEROSUP(tcp_Handle, fMID, j+1));
			printf("\n");
		}//1, 2 (i < 2)

		//+++++++++++++++++++++++++++++++++++++++

		if (idProduct == 2209) //JBNU_DAQ
		{
			const int mid = 3;
			printf("\nJBNU_DAQ (mID: %d) found @ minitcb %d \n", mid, i+1);

			// write setting
			tcb->TCBinit_ADC(tcp_Handle, mid);
			tcb->TCBinit_DRAM(tcp_Handle, mid);
			tcb->TCBwrite_HV(tcp_Handle, mid, 1, hv1); //ckim
			tcb->TCBwrite_HV(tcp_Handle, mid, 2, hv2); //ckim
			tcb->TCBwrite_COIN_WIDTH(tcp_Handle, mid, coin_width);
			tcb->TCBwrite_MULTI_THR(tcp_Handle, mid, multi_thr_daq);
			for (int ch=0; ch<32; ch++) tcb->TCBwrite_DISC_THR(tcp_Handle, mid, ch+1, disc_thr[ch]);
			tcb->TCBwrite_GATE_WIDTH(tcp_Handle, mid, gate_width);
			tcb->TCBwrite_DELAY(tcp_Handle, mid, input_delay);
			tcb->TCBwrite_PEAK_SUM_WIDTH(tcp_Handle, mid, peak_sum_width);
			tcb->TCBwrite_ZERO_SUP(tcp_Handle, mid, zero_sup);
			tcb->TCBwrite_PRESCALE(tcp_Handle, mid, prescale);
			tcb->TCBmeasure_PED(tcp_Handle, mid);

			float temp[2]; tcb->TCBread_TEMP(tcp_Handle, mid, temp);
			printf("DAQ[%d] temperature ch1 = %f\n", mid, temp[0]);
			printf("DAQ[%d] temperature ch2 = %f\n", mid, temp[1]);
			printf("DAQ[%d] high voltage ch1 = %f V\n", mid, tcb->TCBread_HV(tcp_Handle, mid, 1));
			printf("DAQ[%d] high voltage ch2 = %f V\n", mid, tcb->TCBread_HV(tcp_Handle, mid, 2));

			printf("DAQ[%d] discriminator thresholds for ch01 - ch08 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_DISC_THR(tcp_Handle, mid, ch+1));
			printf("\n");
			printf("DAQ[%d] discriminator thresholds for ch09 - ch16 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_DISC_THR(tcp_Handle, mid, ch+9));
			printf("\n");
			printf("DAQ[%d] discriminator thresholds for ch17 - ch24 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_DISC_THR(tcp_Handle, mid, ch+17));
			printf("\n");
			printf("DAQ[%d] discriminator thresholds for ch25 - ch32 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_DISC_THR(tcp_Handle, mid, ch+25));
			printf("\n");

			printf("DAQ[%d] multiplicity threshold = %d\n", mid, tcb->TCBread_MULTI_THR(tcp_Handle, mid));
			printf("DAQ[%d] coincidence width = %d ns\n", mid, tcb->TCBread_COIN_WIDTH(tcp_Handle, mid));
			printf("DAQ[%d] gate width        = %d us\n", mid, tcb->TCBread_GATE_WIDTH(tcp_Handle, mid));
			printf("DAQ[%d] input delay       = %d ns\n", mid, tcb->TCBread_DELAY(tcp_Handle, mid));
			printf("DAQ[%d] peak sum width    = %d ns\n", mid, tcb->TCBread_PEAK_SUM_WIDTH(tcp_Handle, mid));
			printf("DAQ[%d] zero suppression       = %d\n", mid, tcb->TCBread_ZERO_SUP(tcp_Handle, mid));
			printf("DAQ[%d] waveform prescale      = %d\n", mid, tcb->TCBread_PRESCALE(tcp_Handle, mid));

			printf("DAQ[%d] pedestal for ch01 - ch08 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_PED(tcp_Handle, mid, ch+1));
			printf("\n");
			printf("DAQ[%d] pedestal for ch09 - ch16 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_PED(tcp_Handle, mid, ch+9));
			printf("\n");
			printf("DAQ[%d] pedestal for ch17 - ch24 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_PED(tcp_Handle, mid, ch+17));
			printf("\n");
			printf("DAQ[%d] pedestal for ch25 - ch32 = ", mid);
			for (int ch=0; ch<8; ch++) printf("%d ", tcb->TCBread_PED(tcp_Handle, mid, ch+25));
			printf("\n");
		}

	}//i, loop over minitcb slots

	// Close MINITCB_V2
	tcb->MINITCB_V2write_RUNNO(tcp_Handle, mRunNumber);
	tcb->MINITCB_V2close(tcp_Handle);

	cout <<"\nTCB/FADC initialized!\n\n";
	return 0;
}//Main
