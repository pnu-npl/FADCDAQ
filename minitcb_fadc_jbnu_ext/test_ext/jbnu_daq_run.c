#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "NoticeJBNU_DAQ_S.h"

int EXTSTOP = 0;
void sigint_handler(int sig)
{
	printf("SIGINT (JBNU): Close\n");
	EXTSTOP = 1;
	return;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sigint_handler);

	int sid;
	int link_status[2] = {0};
	int daq_mid[40];
	int num_of_daq;
	int daq;
	int daq_exist;
	int mid[40];
	//float temp[2];
	//int ch;
	int run_number;
	int acq_time;
	char filename[100];
	FILE *fp;
	//int run;
	int data_size;
	int data_written;
	char *data;

	if (argc > 3) {
		sid = atoi(argv[1]);
		run_number = atoi(argv[2]);
		acq_time = atoi(argv[3]);
	}
	else {
		printf("Enter DAQ sid : ");
		scanf("%d", &sid);
		printf("Enter run number(0~65535) : ");
		scanf("%d", &run_number);
		printf("Enter acquisition time(0~1,000,000s, 0 for infinite) : ");
		scanf("%d", &acq_time);
	}

	// open file
	//sprintf(filename, "jbnu_daq_%d_%05d.dat", sid, run_number);
	sprintf(filename, "jbnu_daq_%d_%0d.dat", run_number, sid);
	fp = fopen(filename, "wb"); 
	data_written = 0;

	// assign data array
	data = (char *)malloc(65536);

	// init USB
	USB3Init();

	// open TCB
	DAQopen(sid);

	// reset TCB
	//TCBreset(sid);

	// get link status and ADC mid
	//TCBread_LINK_STATUS(sid, link_status);
	//TCBread_MID(sid, daq_mid);
	//printf("link status = %X %X\n", link_status[1], link_status[0]);

	num_of_daq = 0;
	for (daq = 0; daq < 32; daq++) {
		daq_exist = link_status[0] & (1 << daq);
		if (daq_exist) {
			mid[num_of_daq] = daq_mid[daq]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq);
			num_of_daq = num_of_daq + 1;
		}
	}
	for (daq = 0; daq < 8; daq++) {
		daq_exist = link_status[1] & (1 << daq);
		if (daq_exist) {
			mid[num_of_daq] = daq_mid[daq + 32]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 32);
			num_of_daq = num_of_daq + 1;
		}
	}

	/*
	// read temperature and pedestal
	for (daq = 0; daq < num_of_daq; daq++) {
	TCBread_TEMP(sid, mid[daq], temp);
	printf("DAQ[%d] temperature ch1 = %f\n", mid[daq], temp[0]);
	printf("DAQ[%d] temperature ch2 = %f\n", mid[daq], temp[1]);
	for (ch = 1; ch <= 32; ch++)
	printf("DAQ[%d] ch%d pedestal = %d\n", mid[daq], ch, TCBread_PED(sid, mid[daq], ch));
	}
	 */

	// set run number
	//TCBwrite_RUN_NUMBER(sid, run_number);
	//printf("run_number = %d\n", TCBread_RUN_NUMBER(sid));

	// set acquisition time
	//TCBwrite_ACQ_TIME(sid, acq_time);
	//printf("acquisition time = %d s\n", TCBread_ACQ_TIME(sid));

	// reset timer if necessary
	// TCBreset_TIMER(sid);  

	//---------------------------------------------------------

	// start DAQ
	//TCBstart_DAQ(sid);
	//printf("running status = %d\n", TCBread_RUN(sid));

	int printCount = 0;
	while (EXTSTOP==0)
	{
		// check data size
		data_size = DAQread_DATASIZE(sid);

		// read data
		if (data_size) {
			//printf("data_size = %d\n", data_size);    
			if (data_size > 64) data_size = 64;
			DAQread_DATA(sid, data_size, data);
			fwrite(data, 1, data_size * 1024, fp);
			data_written = data_written + data_size;
			if (printCount%500 == 0) printf("JBNU_DAQ: %d kbytes are taken\n", data_written);
		}

		int run = DAQread_RUN(sid);
		printCount++;
	}

	printf("JBNU_DAQ: DAQ is stopped and read remaining data\n");

	// read remaining data  
	data_size = 16;
	while (data_size) {
		// check data size
		data_size = DAQread_DATASIZE(sid);

		// read data
		if (data_size) {
			//printf("data_size = %d\n", data_size);    
			if (data_size > 64) data_size = 64;
			DAQread_DATA(sid, data_size, data);
			fwrite(data, 1, data_size * 1024, fp);
			data_written = data_written + data_size;
			printf("JBNU_DAQ: %d kbytes are taken\n", data_written);
		}
	}    

	// release data array
	free(data);

	// close file
	fclose(fp);

	// close DAQ
	DAQclose(sid);

	// close usb
	USB3Exit();

	return 0;
}


