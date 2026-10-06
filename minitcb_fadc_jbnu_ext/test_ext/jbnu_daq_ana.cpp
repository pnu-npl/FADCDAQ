#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TStyle.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"

#include <iostream>
#include <fstream>
#include <map>
using namespace std;

map<int, int> GetChMap(void)
{
    std::map<int, int> chMap;

    chMap.insert( std::pair<int, int> ( 1,  1) );
    chMap.insert( std::pair<int, int> ( 2,  2) );
    chMap.insert( std::pair<int, int> ( 3,  3) );
    chMap.insert( std::pair<int, int> ( 4,  4) );
    chMap.insert( std::pair<int, int> ( 5,  8) );
    chMap.insert( std::pair<int, int> ( 6,  7) );
    chMap.insert( std::pair<int, int> ( 7,  6) );
    chMap.insert( std::pair<int, int> ( 8,  5) );
    chMap.insert( std::pair<int, int> ( 9, 12) );
    chMap.insert( std::pair<int, int> (10, 11) );
    chMap.insert( std::pair<int, int> (11, 10) );
    chMap.insert( std::pair<int, int> (12,  9) );
    chMap.insert( std::pair<int, int> (13, 13) );
    chMap.insert( std::pair<int, int> (14, 14) );
    chMap.insert( std::pair<int, int> (15, 15) );
    chMap.insert( std::pair<int, int> (16, 16) );

    chMap.insert( std::pair<int, int> ( 1+16,  1+16) );
    chMap.insert( std::pair<int, int> ( 2+16,  2+16) );
    chMap.insert( std::pair<int, int> ( 3+16,  3+16) );
    chMap.insert( std::pair<int, int> ( 4+16,  4+16) );
    chMap.insert( std::pair<int, int> ( 5+16,  8+16) );
    chMap.insert( std::pair<int, int> ( 6+16,  7+16) );
    chMap.insert( std::pair<int, int> ( 7+16,  6+16) );
    chMap.insert( std::pair<int, int> ( 8+16,  5+16) );
    chMap.insert( std::pair<int, int> ( 9+16, 12+16) );
    chMap.insert( std::pair<int, int> (10+16, 11+16) );
    chMap.insert( std::pair<int, int> (11+16, 10+16) );
    chMap.insert( std::pair<int, int> (12+16,  9+16) );
    chMap.insert( std::pair<int, int> (13+16, 13+16) );
    chMap.insert( std::pair<int, int> (14+16, 14+16) );
    chMap.insert( std::pair<int, int> (15+16, 15+16) );
    chMap.insert( std::pair<int, int> (16+16, 16+16) );
	//for (int a=0; a<32; a++) cout <<Form("%2i -> %2i\n", a+1, chMap[a+1]);

    return chMap;
}//Channel map

map<int, float> GetChGain(const char* inFile)
{
	map<int, float> chGain;
	int ch;
	float gain;

	ifstream in;
	in.open(inFile);
	if (!in.is_open())
	{
		cout <<Form("Caanot fine the file %s: stop.\n", inFile);
		return chGain;
	}
	while (in.is_open())
	{
		if (!in.good()) break;

		in >>ch >>gain;
		chGain.insert(std::pair<int, float> (ch, gain));
		//cout <<Form("ch%i: %4.3f\n", ch, chGain[ch]);
	}
	in.close();

	return chGain;
}//Gain map

//================
void jbnu_daq_ana(
		const char* inData = "./data/jbnu_daq_31_60005.root",
		const char* inGain = "jbnu_daq_gain_run60002.txt"
		)
{
    // Constants
    const int nCh = 32;
    const int tMin = 35;
    const int tMax = 65;
	const int cutPed = 80;
    map<int, int> chMap = GetChMap();
	map<int, float> chGain = GetChGain(inGain);

    // Link 
    const char* inFile = inData;
    TFile *infile = new TFile(inFile, "read");
    TTree *V = (TTree*)infile->Get("V");

	int v_trig = 0;
    int v_ch = 0;
    int v_wlen = 0;
    int v_adc[500] = {0};
    V->SetBranchAddress("local_trigger_number", &v_trig);
    V->SetBranchAddress("channel", &v_ch);
    V->SetBranchAddress("wave_length", &v_wlen);
    V->SetBranchAddress("adc", v_adc);

	V->GetEntry(V->GetEntries()-1);
	const int nTrig = v_trig;
    const int std_wlen = v_wlen;

	// Containers
	//-------------------------------------------

	TH2F* H2[nCh]; // Trig vs. pulse (x: trig, y: sampling idx, z: adc)
	TH1F* H1[nCh]; // Pulse (x: sampling index, y: adc)
	for (int a=0; a<nCh; a++)
	{
		H2[a] = new TH2F(Form("H2_ch%i", a+1), "", nTrig,0,nTrig, std_wlen,0,std_wlen);
		H2[a]->SetTitle(Form("ch%i;iTrig;iSample", a+1));
		H2[a]->Sumw2();

		H1[a] = new TH1F(Form("H1_ch%i", a+1), "", std_wlen,0,std_wlen);
		H1[a]->SetTitle(Form("ch%i;iSample", a+1));
		H1[a]->Sumw2();
	}//a

	TH2F* H2_hitmap = new TH2F("H2_hitmap", "", nCh*0.5,1,nCh*0.5+1, nCh*0.5,1,nCh*0.5+1);
	H2_hitmap->SetTitle("Hitmap (single hit events only);ch (x);ch (y)");
	H2_hitmap->Sumw2();

	//-------------------------------------------

	// Loop over events, sort again in trigger base
	const int nEvents = V->GetEntries();
	for (int a=0; a<nEvents; a++)
	{
		V->GetEntry(a);
		if (std_wlen != v_wlen) { cout <<Form("iTrig %i: wlen doesn't match!\n", a); continue; }

		for (int b=0; b<v_wlen; b++)
		{
			// Apply channel mapping and gain (* ch# and trig# start from 1)
			const int u_ch = chMap[v_ch];
			const float u_adc = v_adc[b] * chGain[v_ch];

			// Fill trigger vs. sampling index
			H2[u_ch-1]->SetBinContent(v_trig+1, b+1, u_adc);
		}//b, sampling index
	}//a, event

	vector<int> hitX;
	vector<int> hitY;
	for (int a=0; a<nTrig; a++)
	{
		if (a!=0 && a%10000 == 0) cout <<Form("Processing... %i\n", a);

		hitX.clear();
		hitY.clear();
		for (int b=0; b<nCh; b++)
		{
			H1[b]->Reset();
			H1[b] = (TH1F*)H2[b]->ProjectionY("", a+1, a+1);
			H1[b]->GetXaxis()->SetRangeUser(tMin, tMax);

			if (H1[b]->GetMaximum() > cutPed)
			{
				if (b < nCh*0.5) hitX.push_back(b+1);
				else             hitY.push_back(b+1);
			}
		}//b

		if (hitX.size()==1 && hitY.size()==1) H2_hitmap->Fill(hitX[0], hitY[0] - nCh*0.5);
	}//a, trig

	TCanvas* c4 = new TCanvas("c4_hitmap", "", 400*2*3, 400*3);
	c4->Divide(2, 1);
	c4->cd(1)->SetMargin(0.118, 0.118, 0.118, 0.118);
	H2_hitmap->GetXaxis()->SetTitleOffset(1.2);
	H2_hitmap->GetZaxis()->SetLabelSize(0.0325);
	H2_hitmap->SetStats(false);
	H2_hitmap->SetTitle("Hitmap (single hit events only);Channels (x);Channels (y)");
	H2_hitmap->DrawCopy("colz");
	c4->cd(2)->SetMargin(0.118, 0.118, 0.118, 0.118);
	TH1F* H1_hitX = (TH1F*)H2_hitmap->ProjectionX();
	TH1F* H1_hitY = (TH1F*)H2_hitmap->ProjectionY();
	H1_hitX->GetXaxis()->SetTitleOffset(1.2);
	H1_hitX->SetTitle("1D projections;Channels;Entries");
	H1_hitX->SetStats(false);
	H1_hitX->SetLineColor(1);
	H1_hitY->SetLineColor(2);
	float yMax = H1_hitX->GetMaximum();
	if (H1_hitY->GetMaximum() > yMax) yMax = H1_hitY->GetMaximum()*1.1;
	H1_hitX->SetMaximum(yMax);
	H1_hitX->DrawCopy("hist e");
	H1_hitY->DrawCopy("hist e same");
	TLegend* Leg = new TLegend(0.2, 0.7, 0.5, 0.8);
	Leg->SetMargin(0.3);
	Leg->SetLineColor(0);
	Leg->SetLineWidth(0);
	Leg->AddEntry(H1_hitX, "Hit (x)", "l");
	Leg->AddEntry(H1_hitY, "Hit (y)", "l");
	Leg->Draw("same");
	c4->Print(Form("%s.png", c4->GetName()));
	c4->Print(Form("%s.pdf", c4->GetName()));
	c4->Print(Form("%s.eps", c4->GetName()));

	#if 0
	// Peak position check
	int yMax = 0;
	TH1F* H1Temp = new TH1F();
	for (int a=0; a<nCh; a++)
	{
		H1Temp->Reset();
		H1Temp = (TH1F*)H2[a]->ProjectionY();
		if (H1Temp->GetMaximum() > yMax) yMax = H1Temp->GetMaximum()*1.1;
	}
	const int COLOR[] = {1, 2, 210, 4, 95, 6, 7, 9};
	TCanvas* c1 = new TCanvas("c1_peak", "", 400*2*3, 300*2*3);
	c1->Divide(2, 2);
	for (int a=0; a<nCh; a++)
	{
		int iPAD = 0;
		int iCOL = 0;
		const char* iTITLE = "";
		if      (a< 8) { iPAD = 1; iCOL = COLOR[a];    iTITLE = "ch01 - ch08"; }
		else if (a<16) { iPAD = 2; iCOL = COLOR[a-8];  iTITLE = "ch09 - ch16"; }
		else if (a<24) { iPAD = 3; iCOL = COLOR[a-16]; iTITLE = "ch17 - ch24"; }
		else if (a<32) { iPAD = 4; iCOL = COLOR[a-24]; iTITLE = "ch25 - ch32"; }

		c1->cd(iPAD)->SetGrid();
		H1Temp->Reset();
		H1Temp = (TH1F*)H2[a]->ProjectionY()->Clone();
		H1Temp->GetYaxis()->SetRangeUser(-50, yMax);
		H1Temp->SetLineColor(iCOL);
		H1Temp->SetStats(false);
		H1Temp->SetTitle(iTITLE);
		H1Temp->DrawCopy((a%8==0)?"hist e":"hist e same");
	}//a, nCh
	//c1->Print(Form("%s.png", c1->GetName()));
	H1Temp->Delete();

	// 2D (event vs. sampling index) 
	TCanvas* c2[2];
	for (int a=0; a<2; a++)
	{
		c2[a] = new TCanvas(Form("c2_2D_%i", a), "", 400*4*1.5, 300*4*1.5);
		c2[a]->SetTitle(c2[a]->GetName());
		c2[a]->Divide(4, 4);
	}

	for (int a=0; a<nCh; a++)
	{
		int iCVS = (a<nCh*0.5)?0:1;
		int iPAD = (a<nCh*0.5)?(a+1):(a+1-nCh*0.5);
		c2[iCVS]->cd(iPAD);

		H2[a]->SetMinimum(0);
		H2[a]->SetStats(false);
		H2[a]->DrawCopy("colz");
	}//a, nCh

	/*
	// Hit by hit pulse amplitude check (to judge cutPed)
	TH1F* H1Temp = new TH1F();
	const int preScale = 20;
	TCanvas* c3[2];
	for (int a=0; a<2; a++)
	{
		c3[a] = new TCanvas(Form("c3_pulse_%i", a), "", 400*4*1.5, 300*4*1.5);
		c3[a]->SetTitle(c3[a]->GetName());
		c3[a]->Divide(4, 4);
	}

	for (int a=0; a<nCh; a++)
	{
		int iCVS = (a<nCh*0.5)?0:1;
		int iPAD = (a<nCh*0.5)?(a+1):(a+1-nCh*0.5);

		c3[iCVS]->cd(iPAD);
		const char* fTitle = Form("ch %i, prescale: %i;iSample;ADC (calibrated)", a+1, preScale);
		gPad->DrawFrame(0, 9, v_wlen, 1.E3, fTitle);
		gPad->SetLogy();

		cout <<Form("Processing ch %i...\n", a+1);
		for (int b=0; b<nEvents; b++)
		{
			if (b%preScale != 0) continue;

			H1Temp->Reset();
			H1Temp = (TH1F*)H2[a]->ProjectionY(Form("ch%i_evt%i", a, b), b+1, b+1);
			H1Temp->SetLineColor(1);
			H1Temp->DrawCopy("hist e same");
		}//b
	}//a, nCh
	//for (int a=0; a<2; a++) c3[a]->Print(Form("%s.png", c3[a]->GetName()));
	H1Temp->Delete();
	*/
	#endif

	//-------------------------------------------

	return;
}//Main
