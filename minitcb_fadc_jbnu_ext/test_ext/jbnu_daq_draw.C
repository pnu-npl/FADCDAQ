#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1.h"
#include "TH2.h"

#include <iostream>
#include <fstream>
#include <map>
using namespace std;

void jbnu_daq_pedestal(const char* inFile, float* pedVal, float* pedErr, bool Show = false);
void jbnu_daq_waveform(TH2F* H2[32], int runNo, int evt = -1, bool PRINT = false);
map<int, int> jbnu_daq_mapper(void);

void jbnu_daq_draw(int runNo, int evtCheck = -1, bool PRINT = false)
{
	const int nCh = 32;
	const int nSmpMax = 1000;

#if 1
	// Get pedestal
	float pedVal[nCh] = {0};
	float pedErr[nCh] = {0};
	//const char* inFilePed = "./data/jbnu_daq_1016_3.root";
	//jbnu_daq_pedestal(inFilePed, pedVal, pedErr, 0);
	//for (int a=0; a<nCh; a++) cout <<Form("ch%02i ped: %7.4f +/- %6.4f\n", a+1, pedVal[a], pedErr[a]*3);
	for (int a=0; a<nCh; a++) pedErr[a] = 50;
#endif

	map<int, int> chMap = jbnu_daq_mapper();

	// Open file
	//---------------------------------------------------------------

	const char* inFile = Form("./data/jbnu_daq_%i_3.root", runNo);
	TFile* F = TFile::Open(inFile);
	TTree* T = (TTree*)F->Get("T"); //Header
	TTree* V = (TTree*)F->Get("V"); //Waveform
	int t_data_length;
	int t_run_number;
	int t_mid;
	int t_trigger_type;
	int t_tcb_trigger_number;
	ULong64_t t_tcb_trigger_time;
	int t_local_trigger_number;
	int t_local_trigger_pattern;
	ULong64_t t_local_gate_time;
	int v_local_trigger_number;
	int v_channel;
	int v_wave_length;
	int v_adc[nSmpMax];
	T->SetBranchAddress("data_length",           &t_data_length);
	T->SetBranchAddress("run_number",            &t_run_number);
	T->SetBranchAddress("mid",                   &t_mid);
	T->SetBranchAddress("trigger_type",          &t_trigger_type);
	T->SetBranchAddress("tcb_trigger_number",    &t_tcb_trigger_number);
	T->SetBranchAddress("tcb_trigger_time",      &t_tcb_trigger_time);
	T->SetBranchAddress("local_trigger_number",  &t_local_trigger_number);
	T->SetBranchAddress("local_trigger_pattern", &t_local_trigger_pattern);
	T->SetBranchAddress("local_gate_time",       &t_local_gate_time);
	V->SetBranchAddress("local_trigger_number",  &v_local_trigger_number);
	V->SetBranchAddress("channel",               &v_channel);
	V->SetBranchAddress("wave_length",           &v_wave_length);
	V->SetBranchAddress("adc",                   v_adc);

	const int nEventsT = T->GetEntriesFast();
	const int nEventsV = V->GetEntriesFast();

	// Containers
	//---------------------------------------------------------------

#if 1
	T->GetEntry(nEventsT-1);
	const int nTrig = t_tcb_trigger_number + 1;
	TH2F* H1_trigtime = new TH2F("TrigTime", ";iTrig;trigTime", nTrig,0,nTrig, 200,0,2*1.E10);
	T->Project(H1_trigtime->GetName(), "tcb_trigger_time:tcb_trigger_number", "");
	TCanvas* c0 = new TCanvas("c0", "", 800*1.3, 600*1.3); c0->cd(1);
	H1_trigtime->SetStats(false); H1_trigtime->DrawCopy("colz");
#endif

	V->GetEntry(0);
	const float nWave = v_wave_length;
	TH2F* H2_waveform[nCh];
	for (int a=0; a<nCh; a++)
	{
		H2_waveform[a] = new TH2F(Form("Waveform_ch%i", a+1), "", nEventsT,0,nEventsT, nWave,0,nWave);
		H2_waveform[a]->Sumw2();
	}

	// Loop over trees
	//---------------------------------------------------------------

	//Tree V (Waveform)
	int nWaveAccept[32] = {0};
	for (int a=0; a<nEventsV; a++)
	{
		V->GetEntry(a);

		//const int ch = v_channel - 1; //v_channel starts from 1
		const int ch = chMap[v_channel] - 1;
		const int ltn = v_local_trigger_number;

		bool isHit = false;
		for (int b=0; b<v_wave_length; b++)
		{
			if (v_adc[b] > (pedVal[ch] + pedErr[ch]))
			{
				isHit = true;
				break;
			}
		}
		if (isHit == false) continue;

		for (int b=0; b<v_wave_length; b++) H2_waveform[ch]->SetBinContent(ltn+1, b+1, v_adc[b]);
	}//a

	jbnu_daq_waveform(H2_waveform, runNo, evtCheck);

	return;
}//Main

//=================================================================================
void jbnu_daq_pedestal(const char* inFile, float* pedVal, float* pedErr, bool Show)
{
	const int nCh = 32;
	const int nSmpMax = 500;
	TFile* F = TFile::Open(inFile);
	TTree* V = (TTree*)F->Get("V"); //Waveform
	int v_adc[nSmpMax];
	int v_channel;
	int v_wave_length;
	V->SetBranchAddress("adc", v_adc);
	V->SetBranchAddress("channel", &v_channel);
	V->SetBranchAddress("wave_length", &v_wave_length);

	TH1F* H1_ped[nCh];
	for (int a=0; a<nCh; a++)
	{
		H1_ped[a] = new TH1F(Form("H1_ped_ch%i", a+1), "", 1000,-500,500);
		H1_ped[a]->SetTitle(Form("Pedestal, ch%i", a+1));
		H1_ped[a]->Sumw2();
	}

	const int nEventsV = V->GetEntriesFast();
	for (int a=0; a<nEventsV; a++)
	{
		V->GetEntry(a);
		const int ch = v_channel - 1; //v_channel starts from 1
		for (int b=0; b<v_wave_length; b++) H1_ped[ch]->Fill(v_adc[b]);
	}//a

	for (int a=0; a<nCh; a++)
	{
		pedVal[a] = H1_ped[a]->GetMean();
		pedErr[a] = H1_ped[a]->GetRMS();
	}

	if (Show)
	{
		TCanvas* c1_ped[2];
		for (int a=0; a<2; a++)
		{
			c1_ped[a] = new TCanvas(Form("c1_ped_%i", a), "", 1600, 900);
			c1_ped[a]->SetTitle(Form("ped_%i", a));
			c1_ped[a]->Divide(4, 4);
		}
		for (int a=0; a<nCh; a++)
		{
			const int iCVS = a<16 ? 0:1;
			const int iPad = a<16 ? a:(a-16);
			c1_ped[iCVS]->cd(iPad+1);
			H1_ped[a]->GetXaxis()->SetRangeUser(pedVal[a]-pedErr[a]*10, pedVal[a]+pedErr[a]*10);
			H1_ped[a]->DrawCopy("hist e");
		}//b
	}//Show

	V->ResetBranchAddresses();
	F->Close();
	return;
}//Ped

//==================================================================
void jbnu_daq_waveform(TH2F* H2[32], int runNo, int evt, bool PRINT)
{
	//Waveform
	gStyle->SetTitleFontSize(0.07);
	TCanvas* c2[2];
	for (int a=0; a<2; a++)
	{
		c2[a] = new TCanvas(Form("c2_%i", a), "", 1600*0.8, 900*0.8);
		c2[a]->SetTitle(Form("Waveform, ch%i - ch%i", a==0?1:17, a==0?16:32));
		c2[a]->Divide(4, 4);
	}
	for (int a=0; a<32; a++)
	{
		const int iCVS = a<16 ? 0:1;
		const int iPAD = a<16 ? a:(a-16);
		c2[iCVS]->cd(iPAD+1);

		const int nTrig = H2[a]->GetNbinsX();
		const int nWave = H2[a]->GetNbinsY();
		int yMin = 0;
		int yMax = 0;
		for (int b=0; b<nTrig; b++)
		for (int c=0; c<nWave; c++)
		{
			const int BC = H2[a]->GetBinContent(b+1, c+1);
			if (BC < yMin) yMin = BC - 10;
			if (BC > yMax) yMax = BC + 10;
		}

		int nHit = 0;
		TH1F* H1_projX = (TH1F*)H2[a]->ProjectionX();
		for (int b=0; b<nTrig; b++)
		{
			if (H1_projX->GetBinContent(b+1) != 0)
			{
				//cout <<"ch" <<a+1 <<" evt" <<b <<endl;
				nHit++;
			}
		}
		//H1_projX->Delete();

		TH1F* H1_frame = (TH1F*)H2[a]->ProjectionY();
		H1_frame->Reset();
		H1_frame->SetName(Form("Frame_ch%i", a));
		H1_frame->SetStats(false);
		H1_frame->GetXaxis()->SetLabelSize(0.05);
		H1_frame->GetYaxis()->SetLabelSize(0.05);
		H1_frame->GetYaxis()->SetRangeUser(yMin>0?(yMin*0.9):(yMin*1.1), yMax*1.1);
		H1_frame->SetTitle(Form("ch %i, stacked %i/%i", a+1, nHit, nTrig));
		H1_frame->DrawCopy();

		int LineColor = 1;
		TH1F* H1Temp = new TH1F();
		for (int b=0; b<nTrig; b++)
		{
			if (evt!=-1 && evt!=b) continue;
			if (b>0 && b%500 == 0) LineColor++;
			if (H1_projX->GetBinContent(b+1) == 0) continue;

			H1Temp->Reset();
			H1Temp = (TH1F*)H2[a]->ProjectionY(Form("ch%i_trig%i", a+1, b), b+1, b+1);
			H1Temp->SetLineColor(LineColor);
			H1Temp->DrawCopy("hist same");
		}//b, nTrig
		H1Temp->Delete();
	}//a, ch

	if (PRINT) { for (int a=0; a<2; a++) c2[a]->Print(Form("jbnu_%i_%i.png", runNo, a)); }

	return;
}//Waveform

//=================================
map<int, int> jbnu_daq_mapper(void)
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

	return chMap;
}//map
