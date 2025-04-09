#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
using namespace std;

int GetDataLength(const char* inFilePrx, const int nMID)
{
	//Get data length, check possible curruption
	unsigned int DLen[100] = {0};
	for (int a=0; a<nMID; a++)
	{
		ifstream in;
		in.open(Form("%s_%i.dat", inFilePrx, a+1), std::ios::binary);
		if (!in.is_open())
		{
			cout <<Form("\nGetDataLength::cannot open %s_%i.dat, Stop.\n", inFilePrx, a+1);
			return -999;
		}

		int errLine = 0;
		while (in.peek() != EOF)
		{
			char data[32];
			in.read(data, 32);
			unsigned long dataLength = 0;
			for (int i=0; i<4; i++) dataLength += ((ULong_t)(data[4*i] & 0xFF) << 8*i);

			DLen[a] = dataLength;
			if (DLen[a]<120 || DLen[a]>32800) //ns
			{
				errLine++;
				if (errLine > 5) { cout <<"Manual DLen input is required!\n"; return -999; }
				cout <<Form("Data curruption is suspected: MID%i, Line%i, %i\n", a, errLine, DLen[a]);
				string lineskip;
				std::getline(in, lineskip);
			}
			else break;
		}

		in.close();
	}//a, nMID

	bool DLenConsistent = true;
	for (int a=1; a<nMID; a++) if (DLen[0] != DLen[a]) DLenConsistent = false;
	if (DLenConsistent) return DLen[0];
	else return -999;
}//GetDataLength

//int nkfadc500_decode_v2(int runNo, int DLenExt = -999)
int main(int argc, char** argv)
{
	//Setup
	//-------------------------------------------

	const int runNo   = std::atoi(argv[1]);
	const int DLenExt = (argc == 3)? std::atoi(argv[2]) : -999;

	const bool FlipADC = true; //If true, flip the ADC to '4095 - ADC'
	const int nMID = 2;
	const int nCh  = 4;

	const char* FPath = "data";
	const char* FName = Form("FADCData_%i", runNo); //Filname prefix

	//DO NOT FUCKING TOUCH THIS PART OR FACE THE CONSEQUENCE
	//Data length: header (32) + body (vary), per ch, 1024 for 1 us
	int DLen = GetDataLength(Form("%s/%s", FPath, FName), nMID);
	if (DLen==-999 && DLenExt==-999)
	{
		cout <<"Weird DLen is detected and no manual value provided: stop.\n";
		return DLen;
	}

	const int TLen = DLen * nCh; //Total length
	const int nADC = (DLen - 32)/2; //# of ADC samples per event
	const int nTDC = nADC/4;

	//ROOT Tree
	//-------------------------------------------

	//TFile* F = new TFile(Form("%s/%s_%i.root", FPath, FName, runNo), "recreate");
	TFile* F = new TFile(Form("%s.root", FName), "recreate");
	TTree* T[nMID];
	for (int a=0; a<nMID; a++) T[a] = new TTree(Form("T%i", a+1), Form("MID%i", a+1));

	UInt_t  fModuleID[nCh]; //Module ID
	UInt_t  fChannel[nCh];  //Channel ID
	UInt_t  fTrigType[nCh]; //Trigger type
	UInt_t  fLocTPat[nCh];  //Local trigger pattern
	UInt_t  fPed[nCh];      //Pedestal
	ULong_t fDataLen[nCh];  //Data length
	ULong_t fDatTime[nCh];  //Data starting time (ns)
	ULong_t fTrigNum[nCh];  //Trigger #
	ULong_t fTrigTime[nCh]; //Trigger time (ns)
	ULong_t fLocTNum[nCh];  //Local trigger #
	UInt_t  fADC[nCh][nADC]; //ADC
	UInt_t  fTDC[nCh][nTDC]; //TDC
	Bool_t  fCurrupt[nCh];   //Currupted?

	for (int a=0; a<nMID; a++)
	{
		T[a]->Branch("mid",      fModuleID, Form("mid[%i]/i", nCh));
		T[a]->Branch("ch",       fChannel,  Form("ch[%i]/i", nCh));
		T[a]->Branch("trigtype", fTrigType, Form("trigtype[%i]/i", nCh));
		T[a]->Branch("ltrigpat", fLocTPat,  Form("ltrigpat[%i]/i", nCh));
		T[a]->Branch("ped",      fPed,      Form("ped[%i]/i", nCh));
		T[a]->Branch("dlength",  fDataLen,  Form("dlength[%i]/l", nCh));
		T[a]->Branch("dtime",    fDatTime,  Form("dtime[%i]/l", nCh));
		T[a]->Branch("trignum",  fTrigNum,  Form("trignum[%i]/l", nCh));
		T[a]->Branch("trigtime", fTrigTime, Form("trigtime[%i]/l", nCh));
		T[a]->Branch("ltrignum", fLocTNum,  Form("ltrignum[%i]/l", nCh));
		T[a]->Branch("adc",      fADC,      Form("adc[%i][%i]/i", nCh, nADC));
		T[a]->Branch("tdc",      fTDC,      Form("tdc[%i][%i]/i", nCh, nTDC));
		T[a]->Branch("currupt",  fCurrupt,  Form("currupt[%i]/O", nCh));
	}

	#if 1
	//Read binary and Convert it to ROOT
	//-------------------------------------------

	ifstream in;
	char data[TLen];
	char dataChop[nCh][DLen];
	for (int i=0; i<nMID; i++) //FADC modules
	{
		const char* openFile = Form("%s/%s_%i.dat", FPath, FName, i+1);
		in.open(openFile, std::ios::binary);
		if (!in.is_open()) { cout <<Form("Cannot open the file %s: stop.\n", openFile); return 1; }

		unsigned long nLine = 0;
		while (in.peek() != EOF)
		{
			//Reset all ROOT variabels
			for (int j=0; j<nCh; j++)
			{
				fModuleID[j] = -999;
				fChannel[j]  = -999;
				fTrigType[j] = -999;
				fLocTPat[j]  = -999;
				fPed[j]      = -999;
				fDataLen[j]  = -999;
				fDatTime[j]  = -999;
				fTrigNum[j]  = -999;
				fTrigTime[j] = -999;
				fLocTNum[j]  = -999;
				fCurrupt[j]  = 0;
				for (int k=0; k<nADC; k++) fADC[j][k] = -999;
				for (int k=0; k<nADC; k++) fTDC[j][k] = -999;
			}//j, reset

			//Start reading
			in.read(data, TLen);
			if (!in.good())	{ cout <<"Data file is currupted! Stop.\n"; break; }
			if (nLine>0 && nLine%10000==0) cout <<Form("Processed (MID%i): %lu\n", i+1, nLine);

			for (int j=0; j<nCh; j++)
			{
				/*
				+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				Header
				Byte: (item #) Contents

				  0-3:  (1) Data length: total data length in bytes including header & body,
				  4-5:  (2) Run number: run number received from TCB
				    6:  (3) Trigger type: trigger type received from TCB
			     7-10:  (4) Trigger number: trigger number received from TCB
				   11:  (5) Trigger fine time: trigger time received from TCB, 0~124, 8 ns per unit
			    12-14:  (6) Trigger coarse time: trigger time received from TCB, 0~16,777,215 (us), 1 us per unit
			       15:  (7) ADC module ID
				   16:  (8) ADC channel number
			    17-20:  (9) Local trigger number
			    21-22: (10) Local trigger pattern
			    23-24: (11) ADC pedestal
			       25: (12) Data starting fine time, unit is 8 ns
			    26-31: (13) Data starting coarse time. unit is us

				* 8 bits per bytes: e.g., 0-3 means [7:0], [15:8], [23:16], and [31:24]
				+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				*/

				for (int k=0; k<DLen; k++) dataChop[j][k] = data[j + 4*k];

				ULong_t tDataLen = 0;
				for (int a=0; a<4; a++) tDataLen += ((ULong_t)(dataChop[j][a] & 0xFF) << 8*a);
				if (tDataLen != (ULong_t)DLen)
				{
					//cout <<Form("WARNING: irregular data length found: %i\n", tDataLen);
					fCurrupt[j] = true;
				}

				UInt_t tTrigType = dataChop[j][6] & 0x0F;

				ULong_t tTrigNum = 0;
				for (int a=0; a<4; a++) tTrigNum += ((ULong_t)(dataChop[j][7+a] & 0xFF) << 8*a);

				ULong_t tTrigTime = ( (ULong_t)(dataChop[j][11] & 0xFF)        ) * 8    + //Fine time: 8 ns/unit
					                ( (ULong_t)(dataChop[j][12] & 0xFF)        ) * 1000 + //Coarse time: 1 us/unit
									( (ULong_t)(dataChop[j][13] & 0xFF) << 8   ) * 1000 +
									( (ULong_t)(dataChop[j][14] & 0xFF) << 8*2 ) * 1000;

				UInt_t tModuleID = dataChop[j][15] & 0xFF;
				UInt_t tChannel  = dataChop[j][16] & 0xFF;
				if (tChannel<1 || tChannel>4)
				{
					//cout <<Form("WARNING: irregular ch ID found: %i\n", tChannel);
					fCurrupt[j] = true;
				}

				ULong_t tLocTNum = 0;
				for (int a=0; a<4; a++) tLocTNum += ( (ULong_t)(dataChop[j][17+a] & 0xFF) << 8*a );

				UInt_t tLocTPat = 0;
				for (int a=0; a<2; a++) tLocTPat += ( (dataChop[j][21+a] & 0xFF) << 8*a );

				UInt_t tPed = 0;
				for (int a=0; a<2; a++) tPed += ( (dataChop[j][23+a] & 0xFF) << 8*a );

				//unsigned int: 16 bit, unsigned long: 32 bit, unsigned long long = 64 bit
				unsigned long long tDatTime = ( (unsigned long long)(dataChop[j][25] & 0xFF)        ) * 8    +
					                          ( (unsigned long long)(dataChop[j][26] & 0xFF)        ) * 1000 +
											  ( (unsigned long long)(dataChop[j][27] & 0xFF) << 8   ) * 1000 +
											  ( (unsigned long long)(dataChop[j][28] & 0xFF) << 8*2 ) * 1000 +
											  ( (unsigned long long)(dataChop[j][29] & 0xFF) << 8*3 ) * 1000 +
											  ( (unsigned long long)(dataChop[j][30] & 0xFF) << 8*4 ) * 1000 +
											  ( (unsigned long long)(dataChop[j][31] & 0xFF) << 8*5 ) * 1000;
				/*
				cout <<Form("%i %i %i %i | %4li %4i | %i %5li %5li %i | %li %lli \n",
						i+1,tModuleID,j+1,tChannel, tDataLen,tPed,
						tTrigType,tTrigNum,tLocTNum,tLocTPat, tTrigTime,tDatTime);
						*/

				fDataLen[j]  = tDataLen;
				fTrigType[j] = tTrigType;
				fTrigNum[j]  = tTrigNum;
				fTrigTime[j] = tTrigTime;
				fModuleID[j] = tModuleID;
				fChannel[j]  = tChannel;
				fLocTNum[j]  = tLocTNum;
				fLocTPat[j]  = tLocTPat;
				fPed[j]      = tPed;
				fDatTime[j]  = tDatTime;

				/*
				++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				Body
				Byte: [Bits], Contents

				0: [7~0], ADC data[0][ 7:0]
				1: [3~0], ADC data[0][11:8]
				1: [7~4], TDC data[0][ 3:0]
				2: [7~0], ADC data[1][ 7:0]
				3: [3~0], ADC data[1][11:8]
				3: [7~4], TDC data[0][ 7:4]
				4: [7~0], ADC data[2][ 7:0]
				5: [3~0], ADC data[2][11:8]
				5: [7~4], TDC data[0][11:8]
				6: [7~0], ADC data[3][ 7:0]
				7: [3~0], ADC data[3][11:8]
				7: [7~4], 0 (empty)
				---------------------------
				8: [7~0], ADC data[0][ 7:0]
				...

				* 4 ADC samples and 1 TDC sample per 8 bytes, then repeat for 'N = (data_length - 32)/2'
				* 8 ps per TDC channel
				++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				*/

				for (int a=0; a<nADC; a++)
				{
					const int iSmp = 32 + 2*a;
					UInt_t tADC = (dataChop[j][iSmp] & 0xFF) + ((dataChop[j][iSmp + 1] & 0xF) << 8);

					if (FlipADC == true) fADC[j][a] = (4095 - tADC);
					else fADC[j][a] = tADC;
				}

				for (int a=0; a<nTDC; a++)
				{
					const int iSmp = 32 + 8*a;
					UInt_t tTDC = ((dataChop[j][iSmp + 1] & 0xF0) >> 4) + 
						          ((dataChop[j][iSmp + 3] & 0xF0)) +
						          ((dataChop[j][iSmp + 5] & 0xF0) << 4);

					fTDC[j][a] = tTDC;
				}
			}//j, ch

			T[i]->Fill();
			nLine++;
		}//while loop

		in.close();
	}//i, mID

	F->Write();
	F->Close();
	#endif

	return 0;
}//Main

