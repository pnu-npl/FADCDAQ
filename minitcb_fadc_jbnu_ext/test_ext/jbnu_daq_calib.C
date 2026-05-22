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

void jbnu_daq_calib(int runNo = 0, int mid = 21, bool Show = false)
{
	// Read calibration data and make a set of calibration factors for 32 channels
	// DO NOT correct for the channel mapping here!

	const float fitRange[2] = {40, 60};

	// Open file
	//---------------------------------------------------------------

	const int nSmpMax = 1000;
	const char* inFile = Form("data/jbnu_daq_%i_%i.root", runNo, mid);
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

	// Containers
	//---------------------------------------------------------------

	const int nCh = 32;
	const int nEventsT = T->GetEntriesFast();
	const int nEventsV = V->GetEntriesFast();

	T->GetEntry(nEventsT-1);
	const int nTrig = t_local_trigger_number;
	V->GetEntry(0);
	const int nWave = v_wave_length;

	TH2F* H2_waveform[nCh];
	for (int a=0; a<nCh; a++)
	{
		H2_waveform[a] = new TH2F(Form("Waveform_ch%i", a+1), "", nEventsT,0,nEventsT, nWave,0,nWave);
		H2_waveform[a]->SetTitle(Form("run%i_ch%i", runNo, a+1));
		H2_waveform[a]->Sumw2();
	}

	// Loop over trees
	//---------------------------------------------------------------

	for (int a=0; a<nEventsV; a++)
	{
		V->GetEntry(a);

		const int ch = v_channel - 1; // v_channel starts from 1
		const int ltn = v_local_trigger_number;
		for (int b=0; b<nWave; b++)
		{
			H2_waveform[ch]->SetBinContent(ltn+1, b+1, v_adc[b]);
			H2_waveform[ch]->SetBinError  (ltn+1, b+1, 1); // Artificial error for later fit process
		}
	}//a

	// Analysis
	//---------------------------------------------------------------

	// Get accumulated waveforms nomalized by # of triggers, then extract MPV by landau fit
	TH1F* H1_temp = new TH1F();
	TH1F* H1_waveaccu[nCh];
	TF1*  F1_waveaccu[nCh];
	for (int a=0; a<nCh; a++)
	{
		H1_waveaccu[a] = (TH1F*)H2_waveform[a]->ProjectionY();
		H1_waveaccu[a]->SetName(Form("%s_accu", H2_waveform[a]->GetName()));
		H1_waveaccu[a]->Scale(1./nTrig);

		H1_temp->Reset();
		H1_temp = (TH1F*)H1_waveaccu[a]->Clone(Form("%s_temp", H2_waveform[a]->GetName()));
		H1_temp->GetXaxis()->SetRangeUser(fitRange[0], fitRange[1]);

		F1_waveaccu[a] = new TF1(Form("F1_ch%i", a+1), "landau", fitRange[0], fitRange[1]);
		F1_waveaccu[a]->SetParameter(0, H1_temp->GetMaximum());
		F1_waveaccu[a]->SetParameter(1, H1_temp->GetMean());
		F1_waveaccu[a]->SetParameter(2, H1_temp->GetRMS());
		H1_waveaccu[a]->Fit(F1_waveaccu[a]->GetName(), "EQR0", "kRed", fitRange[0], fitRange[1]);
	}

	// Normalize MPV values by pol0 fit
	TH1F* H1_mpv = new TH1F("H1_mpv", "", nCh, 0, nCh);
	H1_mpv->SetTitle(Form("run%i response (nomalized by pol0 fit);channel;mpv", runNo));
	H1_mpv->Sumw2();
	TF1* F1_mpv = new TF1 ("F1_mpv", "pol0", 0, nCh);
	for (int a=0; a<nCh; a++)
	{
		H1_mpv->SetBinContent(a+1, F1_waveaccu[a]->GetParameter(1));
		H1_mpv->SetBinError  (a+1, F1_waveaccu[a]->GetParError (1));
	}
	H1_mpv->Fit(F1_mpv->GetName(), "EQR0", "kRed", 0, nCh);
	H1_mpv->Scale(1./F1_mpv->GetParameter(0));

	TH1F* H1_mpv_dev = new TH1F("H1_mpv_dev", ";mpv;Entries", 30, 0.98, 1.04);
	H1_mpv_dev->Sumw2();
	for (int a=0; a<H1_mpv->GetNbinsX(); a++)
	{
		const float mpv_val = H1_mpv->GetBinContent(a+1);
		H1_mpv_dev->Fill(mpv_val);
	}

	// Printout calibration factors
	ofstream out;
	out.open(Form("jbnu_daq_gain_run%i.txt", runNo));
	cout <<Form("Printing out calibration factors obtained from run %i...\n", runNo);
	for (int a=0; a<nCh; a++) out <<Form("%2i %7.4f\n", a+1, 1./H1_mpv->GetBinContent(a+1));
	out.close();

	// Draw
	//---------------------------------------------------------------

	//if (Show)
	{
		/*
		TCanvas* c1[2];
		for (int a=0; a<2; a++)
		{
			c1[a] = new TCanvas(Form("c1_%i", a), "", 1600, 900);
			c1[a]->Divide(4, 4);
			for (int b=0; b<16; b++)
			{
				c1[a]->cd(b+1);
				H2_waveform[16*a + b]->DrawCopy("colz");
			}
		}//a

		TCanvas* c2[2];
		for (int a=0; a<2; a++)
		{
			c2[a] = new TCanvas(Form("c2_%i", a), "", 1600, 900);
			c2[a]->Divide(4, 4);
			for (int b=0; b<16; b++)
			{
				c2[a]->cd(b+1);
				const int ch = 16 * a + b;

				H1_waveaccu[ch]->DrawCopy("hist e");
				F1_waveaccu[ch]->SetLineColor(2);
				F1_waveaccu[ch]->SetLineWidth(2);
				F1_waveaccu[ch]->SetLineStyle(2);
				F1_waveaccu[ch]->Draw("same");

				const float mpv = F1_waveaccu[16*a+b]->GetParameter(1);
				const float err = F1_waveaccu[16*a+b]->GetParError (1);
				//cout <<Form("%2i: %7.4f +/- %6.4f\n", ch+1, mpv, err);
			}
			c2[a]->Print(Form("%s.png", c2[a]->GetName()));
		}//a
		*/

		/*
		TCanvas* c3 = new TCanvas("c3_ch15", "", 1600*1.2, 900*1.2);
		c3->Divide(2, 1);
		c3->cd(1)->SetMargin(0.135, 0.135, 0.135, 0.135);
		H2_waveform[14]->GetXaxis()->SetRangeUser(0, 1001);
		H2_waveform[14]->GetXaxis()->SetTitleSize(0.04);
		H2_waveform[14]->GetYaxis()->SetTitleSize(0.04);
		H2_waveform[14]->SetStats(false);
		H2_waveform[14]->SetTitle("Channel 15;Trigger index;Sampling index");
		H2_waveform[14]->DrawCopy("colz");
		c3->cd(2)->SetMargin(0.135, 0.135, 0.135, 0.135);
		TH1F* H1_waveform14 = (TH1F*)H2_waveform[14]->ProjectionY();
		H1_waveform14->SetLineColor(1);
		H1_waveform14->SetLineWidth(2);
		H1_waveform14->SetStats(false);
		H1_waveform14->SetTitle("Channel 15 (accumulated);Sampling index;Entries");
		H1_waveform14->DrawCopy("hist e");
		TF1* F1_waveform14 = new TF1("F1_wavefrom14", "landau", 42, 55);
		F1_waveform14->SetParameters(12000, 50, 2);
		F1_waveform14->SetLineColor(2);
		F1_waveform14->SetLineStyle(2);
		F1_waveform14->SetLineWidth(2);
		H1_waveform14->Fit(F1_waveform14->GetName(), "ER0", "kRed", 42, 55);
		F1_waveform14->Draw("same");
		c3->Print(Form("%s.png", c3->GetName()));
		c3->Print(Form("%s.eps", c3->GetName()));
		c3->Print(Form("%s.pdf", c3->GetName()));
		*/

		gStyle->SetPaintTextFormat("4.3f");
		TCanvas* c4 = new TCanvas("c4_response", "", 1600*1.2, 900*1.2);
		c4->Divide(2, 1);
		c4->cd(1);
		gPad->SetLeftMargin(0.125);
		gPad->SetRightMargin(0.075);
		H1_mpv->SetTitle("Response");
		H1_mpv->GetYaxis()->SetRangeUser(0.98, 1.04);
		H1_mpv->GetXaxis()->SetTitleSize(0.045);
		H1_mpv->GetYaxis()->SetTitleSize(0.045);
		H1_mpv->SetStats(false);
		H1_mpv->SetLineColor(1);
		H1_mpv->SetLineWidth(2);
		H1_mpv->DrawCopy("hist e");// text45");
		TLine* L1 = new TLine(0, 1, 32, 1);
		L1->SetLineColor(2);
		L1->SetLineWidth(2);
		L1->SetLineStyle(2);
		L1->Draw();
		c4->cd(2);
		H1_mpv_dev->SetTitle("Response (deviation)");
		H1_mpv_dev->GetXaxis()->SetTitleSize(0.045);
		H1_mpv_dev->GetYaxis()->SetTitleSize(0.045);
		H1_mpv_dev->SetStats(false);
		H1_mpv_dev->SetLineColor(1);
		H1_mpv_dev->SetLineWidth(2);
		H1_mpv_dev->DrawCopy("hist");
		TLine* L2 = new TLine(1, 0, 1, H1_mpv_dev->GetMaximum()*1.05);
		L2->SetLineColor(2);
		L2->SetLineWidth(2);
		L2->SetLineStyle(2);
		L2->Draw();
		TLegend* Leg = new TLegend(0.45, 0.70, 0.9, 0.85);
		Leg->SetLineWidth(0);
		Leg->SetFillColor(0);
		Leg->SetFillStyle(0);
		Leg->AddEntry((TObject*)0, Form("- Entries: %1.0f", H1_mpv_dev->GetEntries()), "");
		Leg->AddEntry((TObject*)0, Form("- Mean: %4.3f", H1_mpv_dev->GetMean()), "");
		Leg->AddEntry((TObject*)0, Form("- Std Dev: %4.3f", H1_mpv_dev->GetRMS()), "");
		Leg->Draw("same");
		//c4->Print(Form("%s.eps", c4->GetName()));
		//c4->Print(Form("%s.pdf", c4->GetName()));
		//c4->Print(Form("%s.png", c4->GetName()));
	}//Show

	return;
}//Main
