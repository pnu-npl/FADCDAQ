#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_DAQ.h"

long long data_size_calculator(int gate_width,int prescale, int event){
	return ((gate_width * 256 * 32 + 256 * prescale) * event) / 1024;
}

int evt_calculator(int gate_width,int prescale, long long data_size){
	return data_size * 1024 / (gate_width * 256 * 32 + 256 * prescale);
}

int main(int argc, char *argv[])
{
	int sid;
	int run_number;
	char filename[100];
	char filename_apix1[100];
	char filename_apix2[100];
	FILE *fp;
	FILE *fp_apix1;
	FILE *fp_apix2;
	int run;
	int data_size;
	int data_size_apix1;
	int data_size_apix2;
	int data_written;
	int data_written_apix1;
	int data_written_apix2;
	char *data;

	int flag_data_size;
    int status;
	int nevts_flag;

	int flag =0;
	sid = atoi(argv[1]);
	run_number = atoi(argv[2]);
	nevts_flag = atoi(argv[3]);
	char gateW_file[100];
	sprintf(gateW_file,"gatewidth%d.txt",sid);
	FILE *fp_GW = fopen(gateW_file,"rt");
	int gate_width;
	int prescale;
	fscanf(fp_GW,"%d %d",&gate_width,&prescale);
	flag_data_size = data_size_calculator(gate_width,prescale,nevts_flag);
	// init usb
	fclose(fp_GW);
	char rm_GW[100];
	sprintf(rm_GW,"rm gatewidth%d.txt",sid);
	system(rm_GW);
	USB3Init();

	// open DAQ
	status = DAQopen(sid);
	char mkdir_name[200];
	sprintf(mkdir_name,"mkdir -p /home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/",run_number,run_number,sid);
	system(mkdir_name);

	sprintf(filename, "/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/bic_daq_%d_%d.dat",run_number,run_number,sid, sid, run_number);
	sprintf(filename_apix1, "/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/apix_1_%d_%d.dat",run_number,run_number,sid, sid, run_number);
	sprintf(filename_apix2, "/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/apix_2_%d_%d.dat",run_number,run_number,sid, sid, run_number);
	fp = fopen(filename, "wb"); 
	fp_apix1 = fopen(filename_apix1, "wb"); 
	fp_apix2 = fopen(filename_apix2, "wb"); 
	data_written = 0;
	data_written_apix1 = 0;
	data_written_apix2 = 0;
    if (status > 0){
		// open file
		
		// assign data array
		data = (char *)malloc(65536);
		
		run = 1;
		while (run) {
			// check data size
			data_size = DAQread_DATASIZE(sid);
		
			// read data
			if (data_size) {
				//printf("data_size = %d\n", data_size);    
				//if (data_size > 16) data_size = 16;
				//printf("BIC data dbg\n");
				DAQread_DATA(sid, data_size, data);
				fwrite(data, 1, data_size * 1024, fp);
				data_written = data_written + data_size;
				int nevts_now = evt_calculator(gate_width,prescale, data_written);
				printf("BIC_DAQ[%d] %d events are taken\n", sid, nevts_now);
			}
		
			// check apix1 data size
			data_size_apix1 = DAQread_APIX_DATASIZE(sid, 1);
		
			// read apix1 data
			if (data_size_apix1) {
				//printf("apix1 data dbg\n");
				DAQread_APIX_DATA(sid, 1, data_size_apix1, data);
				fwrite(data, 1, data_size_apix1 * 1024, fp_apix1);
				data_written_apix1 = data_written_apix1 + data_size_apix1;
				printf("APIX1[%d] %d kbytes are taken\n", sid, data_written_apix1);
			}
		
			// check apix2 data size
			data_size_apix2 = DAQread_APIX_DATASIZE(sid, 2);
		
			// read apix2 data
			if (data_size_apix2) {
				//printf("apix2 data dbg\n");
				DAQread_APIX_DATA(sid, 2, data_size_apix2, data);
				fwrite(data, 1, data_size_apix2 * 1024, fp_apix2);
				data_written_apix2 = data_written_apix2 + data_size_apix2;
				printf("APIX2[%d] %d kbytes are taken\n", sid, data_written_apix2);
			}
		
			run = DAQread_RUN(sid);
			if (access("KILLME",F_OK)==0)
				break;
			if (data_written > flag_data_size && flag ==0){
				char flag_name[100];
				sprintf(flag_name,"touch KILLTCB%d",sid);
				system(flag_name);
				flag = 1;
			}
		}
		
		printf("DAQ is stopped and read remaining data\n");
		
		// read remaining data  
		data_size = 16;
		while (data_size) {
			// check data size
			data_size = DAQread_DATASIZE(sid);
		
			// read data
			if (data_size) {
				//printf("data_size = %d\n", data_size);    
				//if (data_size > 16) data_size = 16;
				DAQread_DATA(sid, data_size, data);
				fwrite(data, 1, data_size * 1024, fp);
				data_written = data_written + data_size;
				printf("BIC_DAQ[%d] %d kbytes are taken\n", sid, data_written);
			}
		}
		
		data_size_apix1 = 16;
		while (data_size_apix1) {
			// check apix1 data size
			data_size_apix1 = DAQread_APIX_DATASIZE(sid, 1);
		
			// read apix1 data
			if (data_size_apix1) {
				DAQread_APIX_DATA(sid, 1, data_size_apix1, data);
				fwrite(data, 1, data_size_apix1 * 1024, fp_apix1);
				data_written_apix1 = data_written_apix1 + data_size_apix1;
				printf("APIX1[%d] %d kbytes are taken\n", sid, data_written_apix1);
			}
		}
		
		data_size_apix2 = 16;
		while (data_size_apix2) {
			// check apix2 data size
			data_size_apix2 = DAQread_APIX_DATASIZE(sid, 2);
		
			// read apix2 data
			if (data_size_apix2) {
				DAQread_APIX_DATA(sid, 2, data_size_apix2, data);
				fwrite(data, 1, data_size_apix2 * 1024, fp_apix2);
				data_written_apix2 = data_written_apix2 + data_size_apix2;
				printf("APIX2[%d] %d kbytes are taken\n", sid, data_written_apix2);
			}
		}    
    }
	// release data array
	free(data);

	// close file
	fclose(fp);
	fclose(fp_apix1);
	fclose(fp_apix2);

	// close DAQ
	DAQclose(sid);

	// close usb
	USB3Exit();

	return 0;
}


