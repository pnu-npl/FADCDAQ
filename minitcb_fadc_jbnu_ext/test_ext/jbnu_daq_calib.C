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

// Landau
double F1Landau(double *x, double *p)
{
	double X = x[0];

	double Height = p[0];
	double MPV    = p[1];
	double Sigma  = p[2];

	return Height * TMath::Landau(X, MPV, Sigma);
}//F1Landau

// Gaussian
double F1Gaus(double *x, double *p)  //x for variable, p for parameter
{
    double X = x[0];

    double Height = p[0];
    double Center = p[1];
    double Width  = p[2];

    return Height * TMath::Gaus(X, Center, Width);
}//F1Gaus

// Landau + Gaussian + pol0 (pedestal)
double F1Conv(double *x, double *p)
{
    return F1Landau(x, p) + F1Gaus(x, &p[3]) + p[6];
}//F1Conv

// Main
void jbnu_daq_calib(int runNo = 0, int mid = 21, bool Show = false)
{
	// Read calibration data and make a set of calibration factors for 32 channels
	// DO NOT correct for the channel mapping here!

	const int nSmpMax = 1000;
	const float fitR_peak[] = {40, 60};
	const float fitR_ped[] = {30, 70}; // Potential peak position to delete them
	const float min_peak = 20; // max. pulse height ADC must exceeds this value

	// Open file
	//---------------------------------------------------------------

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
	const int nSmpIdx = v_wave_length;

	TH2F* H2_waveform[nCh];
	for (int a=0; a<nCh; a++)
	{
		H2_waveform[a] = new TH2F(Form("Waveform_ch%i", a+1), "", nEventsT,0,nEventsT, nSmpIdx,0,nSmpIdx);
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
		for (int b=0; b<nSmpIdx; b++)
		{
			H2_waveform[ch]->SetBinContent(ltn+1, b+1, v_adc[b]);
			H2_waveform[ch]->SetBinError  (ltn+1, b+1, 1); // Artificial error for later fit process
		}
	}//a

	// Analysis
	//---------------------------------------------------------------

	// Leave only the valid waveforms
	int nValidEvt[nCh] = {0};
	float pedestal[nCh] = {0.};
	TH1F* H1_temp = new TH1F();
	TH2F* H2_waveform_valid[nCh];
	TF1* F1_ped = new TF1("F1_ped", "pol0", 0, nSmpIdx);

	for (int a=0; a<nCh; a++)
	{
		H2_waveform_valid[a] = (TH2F*)H2_waveform[a]->Clone();
		H2_waveform_valid[a]->Reset();

		for (int b=0; b<nEventsT; b++)
		{
			H1_temp->Reset();
			H1_temp = (TH1F*)H2_waveform[a]->ProjectionY(Form("wave_%i_%i_ped", a, b), b+1, b+1);
			const float yMax = H1_temp->GetMaximum(); // Potential peak

			// Skip empty events
			if (H1_temp->GetEntries() == 0) continue;

			// Delete peak bins to measure the pedestal
			for (int c=0; c<nSmpIdx; c++)
			{
				if ( (c+1 >= fitR_ped[0]) && (c+1 <= fitR_ped[1]) )
				{
					H1_temp->SetBinContent(c+1, 0);
					H1_temp->SetBinError  (c+1, 0);
				}
			}//c

			F1_ped->SetParameter(0, 0);
			H1_temp->Fit(F1_ped->GetName(), "EQR0", "", 0, nSmpIdx);
			const float ped_val = F1_ped->GetParameter(0);
			const float ped_err = F1_ped->GetParError (0);
			pedestal[a] = ped_val;

			if ( (yMax > min_peak) && ( yMax > ped_val + ped_err * 5 ) ) // Valid pulse
			{
				nValidEvt[a]++;
				for (int c=0; c<nSmpIdx; c++)
				{
					const float val = H2_waveform[a]->GetBinContent(b+1, c+1);
					const float err = H2_waveform[a]->GetBinError  (b+1, c+1);
					H2_waveform_valid[a]->SetBinContent(b+1, c+1, val);
					H2_waveform_valid[a]->SetBinError  (b+1, c+1, err);
				}
				//cout <<Form("ch%i_evt%i: %6.3f +- %4.3f / %1.0f\n", a, b, ped_val, ped_err, yMax);
			}
		}//b

		H2_waveform_valid[a]->Scale(1./(float)nValidEvt[a]); // Nomalization by # of valid events
		//cout <<nValidEvt[a] <<endl;
	}//a

	// Get accumulated waveforms nomalized by # of events (triggers) and fit
	TH1F* H1_eDep = new TH1F("H1_edEp", "", nCh, 0, nCh);
	H1_eDep->SetTitle(Form("run%i response (nomalized);channel;eDep", runNo));
	H1_eDep->Sumw2();

	TH1F* H1_waveaccu[nCh];
	TF1*  F1_waveaccu[nCh];
	for (int a=0; a<nCh; a++)
	{
		H1_waveaccu[a] = (TH1F*)H2_waveform_valid[a]->ProjectionY();
		H1_waveaccu[a]->SetName(Form("%s_accu", H2_waveform_valid[a]->GetName()));

		H1_temp->Reset();
		H1_temp = (TH1F*)H1_waveaccu[a]->Clone(Form("%s_temp", H2_waveform[a]->GetName()));
		H1_temp->GetXaxis()->SetRangeUser(fitR_peak[0], fitR_peak[1]);

		F1_waveaccu[a] = new TF1(Form("F1_ch%i", a+1), F1Conv, 0, nSmpIdx, 7);
		F1_waveaccu[a]->SetParameter(0, H1_temp->GetMaximum()*0.5);
		F1_waveaccu[a]->SetParameter(1, H1_temp->GetMean());
		F1_waveaccu[a]->SetParameter(2, H1_temp->GetRMS());
		F1_waveaccu[a]->SetParameter(3, H1_temp->GetMaximum()*0.5);
		F1_waveaccu[a]->SetParameter(4, H1_temp->GetMean());
		F1_waveaccu[a]->SetParameter(5, H1_temp->GetRMS());
		F1_waveaccu[a]->SetParameter(6, 1);

		F1_waveaccu[a]->SetParLimits(0, 0, H1_temp->GetMaximum());
		F1_waveaccu[a]->SetParLimits(1, fitR_peak[0], fitR_peak[1]);
		F1_waveaccu[a]->SetParLimits(2, 0, H1_temp->GetRMS()*2);
		F1_waveaccu[a]->SetParLimits(3, 0, H1_temp->GetMaximum());
		F1_waveaccu[a]->SetParLimits(4, fitR_peak[0], fitR_peak[1]);
		F1_waveaccu[a]->SetParLimits(5, 0, H1_temp->GetRMS()*2);
		F1_waveaccu[a]->SetParLimits(6, 0, H1_temp->GetMaximum());

		H1_waveaccu[a]->Fit(F1_waveaccu[a]->GetName(), "EQR0", "kRed", 0, nSmpIdx);

		//+++++++++++++++++++++++++++++++++++++++

		const float eDep_tot = F1_waveaccu[a]->Integral(fitR_peak[0], fitR_peak[1]);
		const float eDep_err = F1_waveaccu[a]->IntegralError(fitR_peak[0], fitR_peak[1]);
		const float eDep_ped = F1_waveaccu[a]->GetParameter(6) * fabs(fitR_peak[1] - fitR_peak[0]);
		H1_eDep->SetBinContent(a+1, eDep_tot - eDep_ped);
		H1_eDep->SetBinError  (a+1, eDep_err);
	}//a

	TF1* F1_eDep = new TF1 ("F1_eDep", "pol0", 0, nCh);
	H1_eDep->Fit(F1_eDep->GetName(), "EQR0", "kRed", 0, nCh);
	H1_eDep->Scale(1./F1_eDep->GetParameter(0));

	TH1F* H1_eDep_dev = new TH1F("H1_eDep_dev", ";eDep;Entries", 30, 0, 3);
	H1_eDep_dev->Sumw2();
	for (int a=0; a<H1_eDep->GetNbinsX(); a++)
	{
		const float eDep_val = H1_eDep->GetBinContent(a+1);
		H1_eDep_dev->Fill(eDep_val);
	}

	// Printout calibration factors
	ofstream out;
	out.open(Form("jbnu_daq_gain_run%i.txt", runNo));
	cout <<Form("Printing out calibration factors obtained from run %i...\n", runNo);
	for (int a=0; a<nCh; a++) out <<Form("%2i %7.4f\n", a+1, 1./H1_eDep->GetBinContent(a+1));
	out.close();

	// Draw
	//---------------------------------------------------------------

	#if 0
	if (Show)
	{
		TCanvas* c1 = new TCanvas("c1", "waveforms",            400*8, 300*4); c1->Divide(8, 4);
		TCanvas* c2 = new TCanvas("c2", "waveforms_valid",      400*8, 300*4); c2->Divide(8, 4);
		TCanvas* c3 = new TCanvas("c3", "waveforms_proj",       400*8, 300*4); c3->Divide(8, 4);
		TCanvas* c4 = new TCanvas("c4", "waveforms_proj_valid", 400*8, 300*4); c4->Divide(8, 4);
		for (int a=0; a<nCh; a++)
		{
			c1->cd(a+1);
			H2_waveform[a]->SetStats(false);
			H2_waveform[a]->DrawCopy("colz");

			c2->cd(a+1);
			H2_waveform_valid[a]->SetStats(false);
			H2_waveform_valid[a]->DrawCopy("colz");

			c3->cd(a+1);
			H2_waveform[a]->ProjectionY()->DrawCopy("hist e");

			c4->cd(a+1);
			H2_waveform_valid[a]->ProjectionY()->DrawCopy("hist e");
		}
	}//Show
	#endif

	#if 1
	if (Show)
	{
		gStyle->SetOptFit();
		TCanvas* c5[2];

		for (int a=0; a<2; a++)
		{
			c5[a] =	new TCanvas(Form("c5_%i", a), "", 400*4, 300*4);
			c5[a]->Divide(4, 4);
		}
		for (int a=0; a<nCh; a++)
		{
			if (a<16) c5[0]->cd(a+1);
			else      c5[1]->cd(a+1-16);
			H1_waveaccu[a]->DrawCopy("hist e");
			F1_waveaccu[a]->SetLineColor(2);
			F1_waveaccu[a]->SetLineStyle(2);
			F1_waveaccu[a]->SetLineWidth(2);
			F1_waveaccu[a]->Draw("same");
		}
	}//Show
	#endif

	#if 1
	if (Show)
	{
		const int chIdx = 18;
		TCanvas* c6 = new TCanvas(Form("c6_ch%i", chIdx+1), "", 1600*1.2, 900*1.2);
		c6->Divide(2, 1);
		c6->cd(1)->SetMargin(0.135, 0.135, 0.135, 0.135);
		TH2F* H2_sample = (TH2F*)H2_waveform_valid[chIdx]->Clone();
		H2_sample->GetXaxis()->SetRangeUser(0, 1001);
		H2_sample->GetXaxis()->SetTitleSize(0.04);
		H2_sample->GetYaxis()->SetTitleSize(0.04);
		H2_sample->SetStats(false);
		H2_sample->SetTitle(Form("Channel %i;Trigger index;Sampling index", chIdx+1));
		H2_sample->DrawCopy("colz");
		c6->cd(2)->SetMargin(0.135, 0.135, 0.135, 0.135);
		TH1F* H1_sample = (TH1F*)H1_waveaccu[chIdx]->Clone();
		H1_sample->SetLineColor(1);
		H1_sample->SetLineWidth(2);
		H1_sample->SetStats(false);
		H1_sample->SetTitle(Form("Channel %i (accumulated);Sampling index;Entries", chIdx+1));
		H1_sample->DrawCopy("hist e");
		F1_waveaccu[chIdx]->Draw("same");
		//c6->Print(Form("%s.png", c6->GetName()));
		//c6->Print(Form("%s.eps", c6->GetName()));
		//c6->Print(Form("%s.pdf", c6->GetName()));

		gStyle->SetPaintTextFormat("4.3f");
		TCanvas* c7 = new TCanvas("c7_response", "", 1600*1.2, 900*1.2);
		c7->Divide(2, 1);
		c7->cd(1);
		gPad->SetLeftMargin(0.125);
		gPad->SetRightMargin(0.075);
		H1_eDep->SetTitle("Response;channel;");
		H1_eDep->GetXaxis()->SetTitleSize(0.045);
		H1_eDep->GetYaxis()->SetTitleSize(0.045);
		H1_eDep->GetYaxis()->SetRangeUser(0, H1_eDep->GetMaximum()*1.1);
		H1_eDep->SetStats(false);
		H1_eDep->SetLineColor(1);
		H1_eDep->SetLineWidth(2);
		H1_eDep->DrawCopy("hist e");// text45");
		TLine* L1 = new TLine(0, 1, 32, 1);
		L1->SetLineColor(2);
		L1->SetLineWidth(2);
		L1->SetLineStyle(2);
		L1->Draw();
		c7->cd(2);
		H1_eDep_dev->SetTitle("Response (deviation)");
		H1_eDep_dev->GetXaxis()->SetTitleSize(0.045);
		H1_eDep_dev->GetYaxis()->SetTitleSize(0.045);
		H1_eDep_dev->GetXaxis()->SetRangeUser(0, H1_eDep->GetMaximum()*1.2);
		H1_eDep_dev->SetStats(false);
		H1_eDep_dev->SetLineColor(1);
		H1_eDep_dev->SetLineWidth(2);
		H1_eDep_dev->DrawCopy("hist");
		TLine* L2 = new TLine(1, 0, 1, H1_eDep_dev->GetMaximum()*1.05);
		L2->SetLineColor(2);
		L2->SetLineWidth(2);
		L2->SetLineStyle(2);
		L2->Draw();
		TLegend* Leg = new TLegend(0.45, 0.70, 0.9, 0.85);
		Leg->SetLineWidth(0);
		Leg->SetFillColor(0);
		Leg->SetFillStyle(0);
		Leg->AddEntry((TObject*)0, Form("- Entries: %1.0f", H1_eDep_dev->GetEntries()), "");
		Leg->AddEntry((TObject*)0, Form("- Mean: %4.3f", H1_eDep_dev->GetMean()), "");
		Leg->AddEntry((TObject*)0, Form("- Std Dev: %4.3f", H1_eDep_dev->GetRMS()), "");
		Leg->Draw("same");
		//c7->Print(Form("%s.eps", c7->GetName()));
		//c7->Print(Form("%s.pdf", c7->GetName()));
		//c7->Print(Form("%s.png", c7->GetName()));
	}//Show
	#endif

	return;
}//Main
