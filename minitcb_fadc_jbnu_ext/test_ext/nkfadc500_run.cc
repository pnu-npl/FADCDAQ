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

bool EXTSTOP = false;
void sigint_handler(int sig)
{
	cout<< "SIGINT (FADC): Close\n";
	EXTSTOP = true;
	return;
}

int main(int argc, char** argv)
{
	std::signal(SIGINT, sigint_handler);

	//Local variables
	const int runNo  = std::atoi( argv[1] ); //Run ID
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

	bool bCountNonzero = false;
	unsigned long bCount[nMID] = {0};
	while (EXTSTOP == false) //Take data continually if nEvent = 0
	{
		bCountNonzero = false;
		for (int a=0; a<nMID; a++) //Check buffer count
		{
			bCount[a] = nkfadc->NKFADC500read_BCOUNT(mID[a]);
			if (bCount[a] > 0) bCountNonzero = true;
		}
		if (bCountNonzero == false) continue;

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
	}//While

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
