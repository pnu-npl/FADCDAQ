#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1.h"
#include "TH2.h"
#include "/home/npl/jbnu_daq/notice/test/DrawInfo.h"

#include <iostream>
#include <fstream>
#include <set>
using namespace std;

bool Exist(set<int> List, int id)
{
    if (List.find(id) != List.end()) return true;
    else return false;
}

void jbnu_daq_draw(const char* inFile = "out_10015.root", const char* logFile= "setlog.txt", bool PRINT = false)
{
	const int nCh = 32;
	const int nSmpMax = 500;
	bool validOnly = false;

	// Open file
	//---------------------------------------------------------------

    TFile* F = TFile::Open(inFile);
    TTree* T = (TTree*)F->Get("T"); //Header
    TTree* U = (TTree*)F->Get("U"); //Spectrum
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
    int u_local_trigger_number;
    int u_charge[nCh];
    int u_timing[nCh];
    int u_hitFlag[nCh];
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
    U->SetBranchAddress("local_trigger_number",  &u_local_trigger_number);
    U->SetBranchAddress("charge",                u_charge);
    U->SetBranchAddress("timing",                u_timing);
    U->SetBranchAddress("hit_flag",              u_hitFlag);
    V->SetBranchAddress("local_trigger_number",  &v_local_trigger_number);
    V->SetBranchAddress("channel",               &v_channel);
    V->SetBranchAddress("wave_length",           &v_wave_length);
    V->SetBranchAddress("adc",                   v_adc);

	const int nEventsT = T->GetEntriesFast();
	const int nEventsU = U->GetEntriesFast();
	const int nEventsV = V->GetEntriesFast();

	// Containers
	//---------------------------------------------------------------

	TH1F* H1_nMult = new TH1F("H1_nMult", "", nEventsU,0,nEventsU); H1_nMult->Sumw2();
	TH1F* H1_waveL = new TH1F("H1_waveL", "", nEventsU,0,nEventsU); H1_waveL->Sumw2();

	TH2F* H2_valid  = new TH2F("H2_valid",  "", nEventsU,0,nEventsU, nCh,0.5,nCh+0.5); H2_valid->Sumw2();
	TH2F* H2_charge = new TH2F("H2_charge", "", nEventsU,0,nEventsU, nCh,0.5,nCh+0.5); H2_charge->Sumw2();
	TH2F* H2_adc    = new TH2F("H2_adc",    "", nEventsU,0,nEventsU, nSmpMax,0.5,nSmpMax+0.5); H2_adc->Sumw2();
	TH2F* H2_waveform[nCh];
	for (int a=0; a<nCh; a++) H2_waveform[a] = (TH2F*)H2_adc->Clone(Form("Waveform_ch%i", a));

	// Loop over trees
	//---------------------------------------------------------------

	/*
	//Tree T (header)
	for (int a=0; a<nEventsT; a++)
	{
		T->GetEntry(a);
	}//a
	*/

	//Tree U (Spectrum)
	for (int a=0; a<nEventsU; a++)
	{
		U->GetEntry(a);

		const int ltn = u_local_trigger_number; //Statrs from 0
		for (int b=0; b<nCh; b++)
		{
			if (validOnly && u_hitFlag[b] != true) continue;
			if (validOnly && u_charge[b] == 0) continue; //Hit exists (hitFlag true) but peak sum of ADC is 0?

			H1_nMult->Fill(ltn);
			H2_charge->SetBinContent(ltn+1, b+1, u_charge[b]);
			H2_valid->SetBinContent(ltn+1, b+1, true);
		}//b

		//cout <<endl <<u_local_trigger_number <<endl;
		//for (int b=0; b<nCh; b++) cout <<Form("%i, %i, %i, %i\n", b, u_hitFlag[b], u_timing[b], u_charge[b]);
	}//a

	//Tree V (Waveform)
	for (int a=0; a<nEventsV; a++)
	{
		V->GetEntry(a);

		const int ch  = v_channel - 1; //v_channel starts from 1
		const int ltn = v_local_trigger_number;
		if (validOnly && H2_valid->GetBinContent(ltn+1, ch+1)==false) continue;

		H1_waveL->SetBinContent(ltn+1, v_wave_length);
		for (int b=0; b<v_wave_length; b++)
		{
			H2_waveform[ch]->SetBinContent(ltn+1, b+1, v_adc[b]);
		}//b

		//cout <<endl <<v_local_trigger_number <<endl;
		//for (int b=0; b<v_wave_length; b++) cout <<Form("%i, %i\n", v_channel, v_adc[b]);
	}//a

	// Draw
	//---------------------------------------------------------------

	//Spectrum, etc
	TString runNo = inFile;
	runNo.ReplaceAll("out_", "");
	runNo.ReplaceAll(".root", "");

	TCanvas* c1 = new TCanvas("c1", "Sanity check", 1600, 900); c1->Divide(3, 2);
	c1->cd(1);
	H1_nMult->SetStats(false);
	H1_nMult->SetTitle("Trigger multiplicity;trig");
	H1_nMult->SetLineColor(1);
	H1_nMult->SetMinimum(0);
	H1_nMult->DrawCopy("hist");
	c1->cd(2);
	H1_waveL->SetStats(false);
	H1_waveL->SetTitle("Wavelength (# of sampling index);trig");
	H1_waveL->SetLineColor(1);
	H1_waveL->DrawCopy("hist");
	c1->cd(4);
	H2_charge->SetStats(false);
	H2_charge->SetTitle("ADC peak sum;trig;ch");
	H2_charge->DrawCopy("colz");
	c1->cd(5);
	TH1F* H1_chargeAllTrig = (TH1F*)H2_charge->ProjectionY();
	TH1F* H1_validAllTrig = (TH1F*)H2_valid->ProjectionY();
	H1_chargeAllTrig->Divide(H1_validAllTrig);
	H1_chargeAllTrig->SetStats(false);
	H1_chargeAllTrig->SetTitle("ADC peak sum for all triggers (normalized);ch");
	H1_chargeAllTrig->SetLineColor(1);
	H1_chargeAllTrig->DrawCopy("hist");

	//Write out information about setups
	c1->cd(3);
	TLegend* l1 = (TLegend*)DrawInfo::generalInfo(logFile);
	l1->AddEntry("", runNo, "h");
	l1 ->Draw();

	c1->cd(6);
	TLegend* l2 = (TLegend*)DrawInfo::channelThresh(logFile);
	l2->Draw();

	#if 1
	//Waveform
	gStyle->SetTitleFontSize(0.07);
	TCanvas* c2[2];
	for (int a=0; a<2; a++)
	{
		c2[a] = new TCanvas(Form("c2_%i", a), "", 1600, 900);
		c2[a]->SetTitle(Form("Waveform, ch%i - ch%i", a==0?1:17, a==0?16:32));
		c2[a]->Divide(4, 4);
	}
	for (int a=0; a<32; a++)
	{
		const int iCVS = a<16 ? 0:1;
		const int iPAD = a<16 ? a:(a-16);
		c2[iCVS]->cd(iPAD+1);

		const int nTrig = H2_waveform[a]->GetNbinsX();
		const int nWave = H2_waveform[a]->GetNbinsY();
		int yMin = 0;
		int yMax = 0;
		for (int b=0; b<nTrig; b++)
		for (int c=0; c<nWave; c++)
		{
			const int BC = H2_waveform[a]->GetBinContent(b+1, c+1);
			if (BC < yMin) yMin = BC - 10;
			if (BC > yMax) yMax = BC + 10;
		}

		TH1F* H1_frame = (TH1F*)H2_waveform[a]->ProjectionY();
		H1_frame->Reset();
		H1_frame->SetName(Form("Frame_ch%i", a));
		H1_frame->SetStats(false);
		H1_frame->GetXaxis()->SetLabelSize(0.05);
		H1_frame->GetYaxis()->SetLabelSize(0.05);
		H1_frame->GetYaxis()->SetRangeUser(yMin>0?(yMin*0.9):(yMin*1.1), yMax*1.1);
		const int nTrigValid = H1_validAllTrig->GetBinContent(a+1);
		H1_frame->SetTitle(Form("ch %i, stacked %i waveforms", a+1, validOnly ? nTrigValid:nTrig));
		H1_frame->DrawCopy();

		for (int b=0; b<nTrig; b++)
		{
			if (validOnly && H2_valid->GetBinContent(b+1, a+1)!=true) continue;
			TH1F* H1_wavetemp = (TH1F*)H2_waveform[a]->ProjectionY(Form("ch%i_trig%i", a+1, b), b+1, b+1);
			H1_wavetemp->SetLineColor(1);
			H1_wavetemp->DrawCopy("hist same");
			H1_wavetemp->Delete();
		}//b, nTrig
	}//a, ch

	if (PRINT)
	{
		TString runNo = inFile;
		runNo.ReplaceAll("out_", "");
		runNo.ReplaceAll(".root", "");
		c1->Print(Form("%s_%s.png", c1->GetName(), runNo.Data()));
		for (int a=0; a<2; a++) c2[a]->Print(Form("%s_%s.png", c2[a]->GetName(), runNo.Data()));
	}
	#endif

	return;
}//Main
