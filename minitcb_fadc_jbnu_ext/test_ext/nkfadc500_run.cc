#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "TROOT.h"
#include "TString.h"
#include "NoticeNKFADC500ROOT.h"
#include "usb3comroot.h"
using namespace std;

#define DRAM_SIZE       (10)                  // available PC DRAM size in Mbyte
#define CHUNK_SIZE      (DRAM_SIZE*1024)      // array size in kilobyte
#define DATA_ARRAY_SIZE (DRAM_SIZE*1024*1024) // array size in byte

bool bSTOP = false;
void sigint_handler(int sig)
{
	cout<< "SIGINT (FADC): Close\n";
	bSTOP = true;
	return;
}

int main(int argc, char** argv)
{
	std::signal(SIGINT, sigint_handler);

	//Local variables
	const int runNo  = std::atoi( argv[1] ); //Run ID
	const int nEvent = std::atoi( argv[2] ); //# of events to take
	const int tLimit = 0; //Set timeout in seconds: this will slow down the process!
	if (tLimit > 0) cout <<Form("\nWARNING: %2.1f hour timeout is set!\n", (float)tLimit/3600);

	const int nCh  = 4; //# of channels per FADC module
	const int nMID = 2; //# of FADC modules
	const int mID[nMID] = {1, 2}; //FADC modules ID

	//-------------------------------------------

	//Output file
	string fName[nMID];
	ofstream ofStr[nMID];
	for (int a=0; a<nMID; a++)
	{
		fName[a] = Form("FADCData_%i_%i.dat", runNo, mID[a]);
		ofStr[a].open(fName[a].c_str(), std::ios::binary);
	}

	//Raw data array
	char* dArray = (char*)malloc(nCh * DATA_ARRAY_SIZE);

	//Define VME class
	usb3comroot *usb = new usb3comroot;
	usb->USB3Init(0);

	//Define KFADC500 class
	NKNKFADC500* nkfadc = new NKNKFADC500;
	for (int a=0; a<nMID; a++)
	{
		nkfadc->NKFADC500open(mID[a], 0);
		nkfadc->NKFADC500start(mID[a]);
		//nkfadc->NKFADC500flush_DATA(mID[a]); //Causes segfault... wtf
	}

	//-------------------------------------------

	chrono::steady_clock::time_point tStart, tStop;
	if (tLimit > 0) tStart = chrono::steady_clock::now();

	int tEvent = 1;
	bool bCountNonzero = false;
	unsigned long bCount[nMID] = {0};
	while (bSTOP==false && nEvent!=tEvent) //Take data continually if nEvent = 0
	{
		bCountNonzero = false;
		for (int a=0; a<nMID; a++) //Check buffer count
		{
			bCount[a] = nkfadc->NKFADC500read_BCOUNT(mID[a]);
			if (bCount[a] > 0) bCountNonzero = true;
		}
		if (bCountNonzero == false) continue; //usleep(10);

		for (int a=0; a<nMID; a++) //Loop over FADC modules
		{
			if (bCount[a] > 0)
			{
				if (bCount[a] > CHUNK_SIZE)
				{
					cout <<Form("WARNING! BCOUNT of mID_%i > CHUNK: truncate it...\n", a);
					bCount[a] = CHUNK_SIZE;
				}
				nkfadc->NKFADC500read_DATA(mID[a], bCount[a], dArray);
				ofStr[a].write(dArray, bCount[a] * 1024);
			}
		}

		if (tEvent>0 && tEvent%100==0)
		{
			cout <<"Writing... ";
			for (int a=0; a<nMID; a++) cout <<Form("mID_%i: %3li ", mID[a], bCount[a]);
			cout <<endl;
		}
		if (tLimit > 0)
		{
			tStop = chrono::steady_clock::now();
			int tElapse = chrono::duration_cast<chrono::seconds>(tStop - tStart).count();
			if (tElapse > tLimit) { cout <<"Timed up: stop.\n";	bSTOP = true; }
		}
		tEvent++;
	}//While

	/*
	//Added Mar. 7, 2025; Read remaining data
	printf("NKFADC500: DAQ is stopped and read remaining data\n");
	for (int a=0; a<nMID; a++) //Loop over FADC modules
	{
		nkfadc->NKFADC500read_DATA(mID[a], bCount[a], dArray);
		ofStr[a].write(dArray, bCount[a] * 1024);
	}
	*/

	free(dArray); //Free data array
	for (int a=0; a<nMID; a++)
	{
		nkfadc->NKFADC500stop(mID[a]);
		nkfadc->NKFADC500close(mID[a]); //Close KFADC500
		ofStr[a].close(); //Close data file
	}
	usb->USB3Exit(0);

	return 0;
}//Main
