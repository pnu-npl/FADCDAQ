#include <unistd.h>
#include <stdio.h>
#include "NoticeJBNU_DAQ_S.h"

int main(int argc, char *argv[])
{
	// Settings (ckim, updated 241114)
	//-----------------------------------------------------

	const float hv1 = 47; //High voltage for FEE1, 0 ~ 48 V
	const float hv2 = 46; //High voltage for FEE2, 0 ~ 48 V

	const int disc_thr[32] = //Discriminator threshold, 1 ~ 4095
	{
		//FEE1 (1-16) and FEE2 (17-32)
		150, 150, 150, 150, 150, 150, 150, 150, //150-8
		150, 150, 150, 150, 150, 150, 150, 150, //9-1506
		150, 150, 150, 150, 150, 150, 150, 150, //1507-24
		150, 150, 150, 150, 150, 150, 150, 150  //25-32
	};

	const int multi_thr_tcb  = 1; //TCB Multiplicity threshold, 1 ~ 1,280
	const int multi_thr_daq  = 2; //DAQ Multiplicity threshold, 1 ~ 32
	const int trig_enable    = 1; //Trigger mode, 1=self, 2=pedestal, 4=software, 8=external (15 for enable all)
	const int ptrig_interval = 1000; //Pedestal trigger interval in ms, 0 ~ 65535 ms, 0 for disable

	const int gate_width     = 8;    //Gate width, 1/2/4/8/16 us
	const int coin_width     = 100;  //Coincidence width, 16 ~ 16,000 ns
	const int input_delay    = 4000; //Input delay, 0 ~ 16,000 ns
	const int peak_sum_width = 500;  //Peak sum width, 16 ~ 16,000 ns

	const int zero_sup = 0; //Zero suppression for waveform, 0 = not suppress, 1 = suppress
	const int prescale = 1; //ADC waveform prescale, 1 ~ 65535, 0 for disable waveform save

	// Write some settings into txt file
	// Added by jinhyun
	FILE* setlog;
	setlog = fopen("setlog.txt", "w");
	fprintf(setlog, "%.0f\n", hv1);				// line1
	fprintf(setlog, "%.0f\n", hv2);				// line2
	
	int ich=0;
	for(ich=0; ich<32; ich++)
	{
		fprintf(setlog, "%d\n", disc_thr[ich]);		//line3~34
	}

	fprintf(setlog, "%d\n", multi_thr_daq);			//line35
	fprintf(setlog, "%d\n", trig_enable);			//line36
	fprintf(setlog, "%d\n", zero_sup);			//line37
	fclose(setlog);

	//-----------------------------------------------------

	int sid;
	int link_status[2];
	int daq_mid[40];
	int num_of_daq;
	int daq;
	int daq_exist;
	int mid[40];

	if (argc > 1) 
		sid = atoi(argv[1]);
	else 
		sid = 1;

	// init USB
	USB3Init();

	// open TCB
	DAQopen(sid);

	// reset TCB
	TCBreset(sid);

	// get link status and ADC mid
	TCBread_LINK_STATUS(sid, link_status);
	TCBread_MID(sid, daq_mid);
	printf("link status = %X %X\n", link_status[1], link_status[0]);
	num_of_daq = 0;
	for (daq = 0; daq < 32; daq++)
	{
		daq_exist = link_status[0] & (1 << daq);
		if (daq_exist)
		{
			mid[num_of_daq] = daq_mid[daq]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 1);
			num_of_daq = num_of_daq + 1;
		}
	}
	for (daq = 0; daq < 8; daq++)
	{
		daq_exist = link_status[1] & (1 << daq);
		if (daq_exist)
		{
			mid[num_of_daq] = daq_mid[daq + 32]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 33);
			num_of_daq = num_of_daq + 1;
		}
	}

	//-----------------------------------------------------
	
	// write setting
	TCBwrite_COIN_WIDTH(sid, 0, coin_width);
	TCBwrite_MULTI_THR(sid, 0, multi_thr_tcb);
	TCBwrite_PEDESTAL_TRIGGER_INTERVAL(sid, ptrig_interval);
	TCBwrite_TRIGGER_ENABLE(sid, trig_enable);
	for (daq = 0; daq < num_of_daq; daq++)
	{
		TCBinit_ADC(sid, mid[daq]);
		TCBinit_DRAM(sid, mid[daq]);
		TCBwrite_HV(sid, mid[daq], 1, hv1); //ckim
		TCBwrite_HV(sid, mid[daq], 2, hv2); //ckim
		TCBwrite_COIN_WIDTH(sid, mid[daq], coin_width);
		TCBwrite_MULTI_THR(sid, mid[daq], multi_thr_daq);
		for (int ch=0; ch<32; ch++) TCBwrite_DISC_THR(sid, mid[daq], ch+1, disc_thr[ch]); 
		TCBwrite_GATE_WIDTH(sid, mid[daq], gate_width);
		TCBwrite_DELAY(sid, mid[daq], input_delay);
		TCBwrite_PEAK_SUM_WIDTH(sid, mid[daq], peak_sum_width);
		TCBwrite_ZERO_SUP(sid, mid[daq], zero_sup);
		TCBwrite_PRESCALE(sid, mid[daq], prescale);
		TCBmeasure_PED(sid, mid[daq]);
	}

	// read back settings
	printf("TCB coincidence width = %d ns\n", TCBread_COIN_WIDTH(sid, 0));
	printf("TCB multiplicity threshold = %d\n", TCBread_MULTI_THR(sid, 0));
	if (trig_enable==2) printf("TCB pedestal trigger interval = %d ms\n", TCBread_PEDESTAL_TRIGGER_INTERVAL(sid));
	printf("TCB trigger enable(mode) = %d\n", TCBread_TRIGGER_ENABLE(sid));
	for (daq = 0; daq < num_of_daq; daq++)
	{
		float temp[2]; TCBread_TEMP(sid, mid[daq], temp);
		printf("DAQ[%d] temperature ch1 = %f\n", mid[daq], temp[0]);
		printf("DAQ[%d] temperature ch2 = %f\n", mid[daq], temp[1]);

		printf("DAQ[%d] high voltage ch1 = %f V\n", mid[daq], TCBread_HV(sid, mid[daq], 1));
		printf("DAQ[%d] high voltage ch2 = %f V\n", mid[daq], TCBread_HV(sid, mid[daq], 2));

		printf("DAQ[%d] discriminator thresholds for ch01 - ch08 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_DISC_THR(sid, mid[daq], ch+1));
		printf("\n");
		printf("DAQ[%d] discriminator thresholds for ch09 - ch16 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_DISC_THR(sid, mid[daq], ch+9));
		printf("\n");
		printf("DAQ[%d] discriminator thresholds for ch17 - ch24 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_DISC_THR(sid, mid[daq], ch+17));
		printf("\n");
		printf("DAQ[%d] discriminator thresholds for ch25 - ch32 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_DISC_THR(sid, mid[daq], ch+25));
		printf("\n");

		printf("DAQ[%d] multiplicity threshold = %d\n", mid[daq], TCBread_MULTI_THR(sid, mid[daq]));
		printf("DAQ[%d] coincidence width      = %d ns\n", mid[daq], TCBread_COIN_WIDTH(sid, mid[daq]));
		printf("DAQ[%d] gate width             = %d us\n", mid[daq], TCBread_GATE_WIDTH(sid, mid[daq]));
		printf("DAQ[%d] input delay            = %d ns\n", mid[daq], TCBread_DELAY(sid, mid[daq]));
		printf("DAQ[%d] peak sum width         = %d ns\n", mid[daq], TCBread_PEAK_SUM_WIDTH(sid, mid[daq]));
		printf("DAQ[%d] zero suppression       = %d\n", mid[daq], TCBread_ZERO_SUP(sid, mid[daq]));
		printf("DAQ[%d] waveform prescale      = %d\n", mid[daq], TCBread_PRESCALE(sid, mid[daq]));

		printf("DAQ[%d] pedestal for ch01 - ch08 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_PED(sid, mid[daq], ch+1));
		printf("\n");
		printf("DAQ[%d] pedestal for ch09 - ch16 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_PED(sid, mid[daq], ch+9));
		printf("\n");
		printf("DAQ[%d] pedestal for ch17 - ch24 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_PED(sid, mid[daq], ch+17));
		printf("\n");
		printf("DAQ[%d] pedestal for ch25 - ch32 = ", mid[daq]);
		for (int ch=0; ch<8; ch++) printf("%d ", TCBread_PED(sid, mid[daq], ch+25));
		printf("\n");
	}

	//-----------------------------------------------------

	// only if necessary, reset timer
	TCBreset_TIMER(sid);

	// reset DAQ
	TCBreset(sid);

	// close TCB
	DAQclose(sid);

	// close USB
	USB3Exit();

	return 0;
}


