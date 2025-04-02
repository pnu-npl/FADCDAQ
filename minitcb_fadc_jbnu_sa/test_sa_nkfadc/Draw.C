#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"

#include <iostream>
#include <fstream>
#include <numeric>
#include <vector>
using namespace std;

int GetDataLength(const char* inFile)
{
    //Reading 1st event, 1st header (32 bytes fixed), 1st channel's very first four bits will be enough
    ifstream in;
    in.open(inFile, std::ios::binary);
    if (!in.is_open()) { cout <<"GetDataLength - cannot open the file! Stop.\n"; return 1; }

    char data[32];
    in.read(data, 32);
    unsigned long dataLength = 0;
    for (int i=0; i<4; i++) dataLength += ((ULong_t)(data[4*i] & 0xFF) << 8*i);

    in.close();
    return (int)dataLength;
}//GetDataLength

void Draw(const int RunNo, const char* inPath = "./data", const char* Suffix = "", bool Print = false)
{
	const int nMID = 2;
	const int nCh  = 4;

	const int DLen = GetDataLength(Form("%s/FADCData_%i_1.dat", inPath, RunNo));
	const int nADC = (DLen-32)/2;
	const int nTDC = nADC/4;

	//-------------------------------------------

	TFile* F = TFile::Open(Form("%s/FADCData_%i.root", inPath, RunNo));
	TTree* T = (TTree*)F->Get("T");

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

    T->SetBranchAddress("mid",      &fModuleID);
    T->SetBranchAddress("ch",       &fChannel);
    T->SetBranchAddress("trigtype", &fTrigType);
    T->SetBranchAddress("ltrigpat", &fLocTPat);
    T->SetBranchAddress("ped",      &fPed);
    T->SetBranchAddress("dlength",  &fDataLen);
    T->SetBranchAddress("dtime",    &fDatTime);
    T->SetBranchAddress("trignum",  &fTrigNum);
    T->SetBranchAddress("trigtime", &fTrigTime);
    T->SetBranchAddress("ltrignum", &fLocTNum);
    T->SetBranchAddress("adc",      &fADC);
    T->SetBranchAddress("tdc",      &fTDC);

	//-------------------------------------------

	TH2F* H2[nMID][nCh]; //Sampling index vs. trigger #, bin content is ADC
	for (int a=0; a<nMID; a++)
	for (int b=0; b<nCh;  b++)
	{
		H2[a][b] = new TH2F(Form("H2_%i%i", a, b), "", 1.E3,0,1.E3, nADC+1,0,nADC+1);
		H2[a][b]->SetTitle(Form("mID=%i, ch=%i;iTrig;iSample", a+1, b+1));
		H2[a][b]->Sumw2();
	}//a

	//-------------------------------------------

	int nEvtMax[nMID] = {0};
	const int nEvents = T->GetEntriesFast();
	for (int a=0; a<nEvents; a++)
	{
		T->GetEntry(a);
		for (int b=0; b<nCh; b++)
		{
			const int tMid   = fModuleID[b];
			const int tCh    = fChannel[b];
			const int tLtnum = fLocTNum[b];

			for (int c=0; c<nADC; c++)
			{
				const int tADC = fADC[tCh-1][c];
				H2[tMid-1][tCh-1]->SetBinContent(tLtnum+1, c+1, tADC);
			}//c, # of sampling per event

			if (b==0 && a>nEvtMax[tMid-1]) nEvtMax[tMid-1] = a;
		}//b, ch
	}//a, events

	//-------------------------------------------

	const int REX = 400;
	const int REY = 300;

	gStyle->SetOptStat(0);
	TCanvas* c1 = new TCanvas("c1", "", REX*nCh, REY*nMID);
	c1->Divide(nCh, nMID);
	for (int a=0; a<nMID; a++)
	for (int b=0; b<nCh;  b++)
	{
		c1->cd(4*a + b + 1);
		H2[a][b]->GetXaxis()->SetRangeUser(0, a==0?nEvtMax[a]:(nEvtMax[a]-nEvtMax[a-1]));
		H2[a][b]->GetYaxis()->SetRangeUser(0, nADC);
		H2[a][b]->DrawCopy("colz");
	}
	if (Print) c1->Print(Form("Run%i%s.png", RunNo, Suffix));

	return;
}//Main
