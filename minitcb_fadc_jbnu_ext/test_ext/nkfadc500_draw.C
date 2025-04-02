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

void nkfadc500_draw(const int RunNo, bool Print = false, const char* inPath = "data", const char* Suffix = "")
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

	T->GetEntry(T->GetEntriesFast()-1);
	const int nTrig = fTrigNum[0] + 1;
	#if 1
	TH2F* H1_trigtime = new TH2F("TrigTime", ";iTrig;trigTime", nTrig,0,nTrig, 200,0,2*1.E10);
	T->Project(H1_trigtime->GetName(), "trigtime:trignum", "mid==1 && ch==1");
	TCanvas* c0 = new TCanvas("c0", "", 800*1.3, 600*1.3); c0->cd(1);
	H1_trigtime->SetStats(false); H1_trigtime->DrawCopy("colz");
	#endif

	TH2F* H2[nMID][nCh]; //Sampling index vs. trigger #, bin content is ADC
	for (int a=0; a<nMID; a++)
	for (int b=0; b<nCh;  b++)
	{
		H2[a][b] = new TH2F(Form("H2_%i%i", a, b), "", nTrig,0,nTrig, nADC+1,0,nADC+1);
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
			//if (a != tLtnum) continue;
			if (fModuleID[b]<1 || fModuleID[b]>2) continue;
			if (fChannel[b]<1 || fChannel[b]>5) continue;

			for (int c=0; c<nADC; c++)
			{
				const int tADC = fADC[tCh-1][c];
				H2[tMid-1][tCh-1]->SetBinContent(tLtnum+1, c+1, tADC);
			}//c, # of sampling per event

			if (b==0 && a>nEvtMax[tMid-1]) nEvtMax[tMid-1] = a;
		}//b, ch
	}//a, events

	//-------------------------------------------
	
	//Get maximum and minimum of each channel
	int zMax[nMID][nCh] = {0};
	for (int a=0; a<nMID; a++)
	for (int b=0; b<nCh;  b++)
	for (int c=0; c<H2[a][b]->GetNbinsX(); c++)
	for (int d=0; d<H2[a][b]->GetNbinsY(); d++)
	{
		const float BC = H2[a][b]->GetBinContent(c+1, d+1);
		if (BC > zMax[a][b]) zMax[a][b] = BC + 100;
	}

	//-------------------------------------------

	const int REX = 400;
	const int REY = 300;
	gStyle->SetOptStat(0);

	TCanvas* c1 = new TCanvas("c1", "", 1600*0.8, 900*0.8);//REX*nCh, REY*nMID);
	c1->Divide(nCh, nMID);
	for (int a=0; a<nMID; a++)
	for (int b=0; b<nCh;  b++)
	{
		c1->cd(4*a + b + 1);
		H2[a][b]->GetXaxis()->SetRangeUser(0, a==0?nEvtMax[a]:(nEvtMax[a]-nEvtMax[a-1]));
		H2[a][b]->GetYaxis()->SetRangeUser(0, nADC);
		H2[a][b]->SetMaximum(zMax[a][b]);
		H2[a][b]->DrawCopy("colz");
	}
	if (Print) c1->Print(Form("c1_fadc_%i%s.png", RunNo, Suffix));

	#if 0
	TCanvas* c2 = new TCanvas("c2", "", 1600*0.8, 900*0.8);//REX*nCh, REY*nMID);
	c2->Divide(nCh, nMID);
	for (int a=0; a<nMID; a++)
	for (int b=0; b<nCh;  b++)
	{
		c2->cd(4*a + b + 1);
		int LineColor = 1;
		TH1F* H1Temp = new TH1F();
		const int nTrig = H2[a][b]->GetNbinsX();
		for (int c=0; c<nTrig; c++)
		{
			H1Temp->Reset();
			H1Temp = (TH1F*)H2[a][b]->ProjectionY("", c+1, c+1);
			if (H1Temp->GetEntries() == 0) continue;

			if (c%500 == 0) LineColor++;
			H1Temp->SetLineColor(LineColor);
			H1Temp->SetMaximum(zMax[a][b]);
			H1Temp->GetYaxis()->SetRangeUser(-200, 4100);
			H1Temp->DrawCopy(c==0?"hist e":"hist e same");
		}
		H1Temp->Delete();
	}
	if (Print) c2->Print(Form("c2_fadc_%i%s.png", RunNo, Suffix));
	#endif

	return;
}//Main
