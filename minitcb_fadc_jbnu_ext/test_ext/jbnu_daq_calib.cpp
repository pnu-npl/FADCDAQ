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

    return chMap;
}//map

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
double F1Gaus(double *x, double *p) //x for variable, p for parameter
{
    double X = x[0];

    double Height = p[0];
    double Center = p[1];
    double Width  = p[2];

    return Height * TMath::Gaus(X, Center, Width);
}//F1Gaus

// Exponential
double F1Exp(double *x, double *p)
{
	double X = x[0];

	double Height = p[0];
	double Coeff  = p[1];

	return Height * TMath::Exp(Coeff * X);
}

// Gaus + Exp
double F1Conv(double *x, double *p)
{
    return F1Gaus(x, p) + F1Exp(x, &p[3]);
}

//=========================================================================
void jbnu_daq_calib(const int RunNo = 60002, const char* inPath = "./data")
{
	// Constants
	const int nCh = 32;
	const int maxADC = 200; // Actually 16,384 (2^14)
	const int maxPed = 60;
	std::map<int, int> chMap = GetChMap();

	// Link 
	const char* inFile = Form("%s/jbnu_daq_31_%i.root", inPath, RunNo);
	TFile *infile = new TFile(inFile, "read");
	TTree *V = (TTree*)infile->Get("V");

	int v_ch = 0;
	int v_wlen = 0;
	int v_adc[500] = {0};
	V->SetBranchAddress("adc", v_adc);
	V->SetBranchAddress("channel", &v_ch);
	V->SetBranchAddress("wave_length", &v_wlen);
	V->GetEntry(0);
	const int std_wlen = v_wlen;
	const int nEvents = V->GetEntries();

	// Container
	TH1F* H1_adc[nCh]; // Inclusive, for all events in the scope
	for (int a=0; a<nCh; a++)
	{
		H1_adc[a] = new TH1F(Form("H1_ch%i", a+1), "", maxADC, 0, maxADC);
		H1_adc[a]->SetTitle(Form("Channel %i;ADC;Entries", a+1));
		H1_adc[a]->Rebin(2);
		H1_adc[a]->Sumw2();
	}

	// Loop over entries
	for (int a=0; a<nEvents; a++)
	{
		V->GetEntry(a);

		if (v_wlen != std_wlen)
		{
			cout <<Form("iTrig %i: wave length does not match!\n", a);
			continue;
		}

		const int u_ch = v_ch-1; // DAQ channel starts from 1
		for (int b=0; b<v_wlen; b++)
		{
			const float u_adc = v_adc[b];
			H1_adc[u_ch]->Fill(u_adc);
		}
	}//a, nEvents

	// Fit
	TH1F* H1_ped[nCh];
	TF1*  F1_ped[nCh];
	TH1F* H1_peak[nCh];
	TF1*  F1_peak[nCh];
	for (int a=0; a<nCh; a++)
	{
		H1_ped[a]  = (TH1F*)H1_adc[a]->Clone();
		H1_peak[a] = (TH1F*)H1_adc[a]->Clone();

		for (int b=0; b<maxADC; b++)
		{
			const float BC = H1_ped[a]->GetBinCenter(b+1);
			if (BC > maxPed) // Leave pedestal only
			{
				H1_ped[a]->SetBinContent(b+1, 0);
				H1_ped[a]->SetBinError  (b+1, 0);
			}
			else
			{
				H1_peak[a]->SetBinContent(b+1, 0);
				H1_peak[a]->SetBinError  (b+1, 0);
			}
		}//b

		F1_ped[a] = new TF1(Form("F1_ped_ch%i", a+1), F1Conv, 0, maxPed, 5);
		F1_ped[a]->SetLineColor(2);
		F1_ped[a]->SetLineStyle(2);
		F1_ped[a]->SetParameter(0, H1_ped[a]->GetMaximum());
		F1_ped[a]->SetParameter(1, 0);
		F1_ped[a]->SetParameter(2, 10);
		F1_ped[a]->SetParameter(3, H1_ped[a]->GetMaximum()*0.5);
		F1_ped[a]->SetParameter(4, -0.05);
		H1_ped[a]->Fit(F1_ped[a]->GetName(), "EQR0", "", 0, maxPed);

		F1_peak[a] = new TF1(Form("F1_peak_ch%i", a+1), F1Landau, 129, 138, 3);
		F1_peak[a]->SetLineColor(4);
		F1_peak[a]->SetLineStyle(2);
		F1_peak[a]->SetParameter(0, H1_peak[a]->GetMaximum());
		F1_peak[a]->SetParameter(1, 130);
		F1_peak[a]->SetParameter(2, 10);
		H1_peak[a]->Fit(F1_peak[a]->GetName(), "EQR0", "", 129, 138);
	}//a

	// Get ch by ch response and normalize it
	TH1F* H1_response = new TH1F("H1_response", ";ch;peak", nCh, 0, nCh); H1_response->Sumw2();
	for (int a=0; a<nCh; a++)
	{
		const float val = F1_peak[a]->GetParameter(1) - F1_ped[a]->GetParameter(1);
		const float err = sqrt(pow(F1_peak[a]->GetParError(1),2)+ pow(F1_ped[a]->GetParError(1),2));
		H1_response->SetBinContent(a+1, val);
		H1_response->SetBinError  (a+1, err);
	}
	TF1* F1_pol0 = new TF1("F1_pol0", "pol0", 0, nCh);
	H1_response->Fit(F1_pol0->GetName(), "EQR0", "", 0, nCh);
	H1_response->Scale(1./F1_pol0->GetParameter(0));

	// Get gain factors and their distribution
	TH1F* H1_gain = (TH1F*)H1_response->Clone("H1_gain");
	H1_gain->SetTitle(";ch;gain");
	for (int a=0; a<nCh; a++)
	{
		H1_gain->SetBinContent(a+1, 1);
		H1_gain->SetBinError  (a+1, 0);
	}
	H1_gain->Divide(H1_response);
	TH1F* H1_gain_dist = new TH1F("H1_gain_dist", ";gain;Entries", 30, 0.85, 1.15);
	for (int a=0; a<nCh; a++) H1_gain_dist->Fill(H1_gain->GetBinContent(a+1));

	// Printout gain factors
    ofstream out;
    out.open(Form("jbnu_daq_gain_run%i.txt", RunNo));
    cout <<Form("Printing out calibration factors obtained from run %i...\n", RunNo);
    for (int a=0; a<nCh; a++) out <<Form("%2i %7.4f\n", a+1, H1_gain->GetBinContent(a+1));
    out.close();

	// Draw
	//=====================================================

	#if 0
	TCanvas* c1[2];
	for (int a=0; a<2; a++)
	{
		c1[a] = new TCanvas(Form("c1_%i", a), Form("fit_%i", a), 400*4*1.5, 300*4*1.5);
		c1[a]->Divide(4, 4);
	}
	for (int a=0; a<nCh; a++)
	{
		const int iCVS = (a<nCh/2)?0:1;
		const int iPAD = (a<nCh/2)?(a+1):(a-nCh/2+1);
		c1[iCVS]->cd(iPAD)->SetLogy();

		H1_adc[a]->SetStats(false);
		H1_adc[a]->SetLineColor(1);
		H1_adc[a]->SetMinimum(1);
		H1_adc[a]->DrawCopy("hist e");

		F1_ped[a]->Draw("same");
		F1_peak[a]->Draw("same");
	}
	#endif

	#if 1
	const int chIdx1 = 7;
	const int chIdx2 = 28;
	TCanvas* c2 = new TCanvas("c2_sample", "", 400*3*2, 300*3);
	c2->Divide(2, 1);
	c2->cd(1);
	gPad->SetLogy();
	gPad->SetMargin(0.115, 0.085, 0.1, 0.1);
	TH1F* H1_smp1 = (TH1F*)H1_adc[chIdx1-1]->Clone();
	H1_smp1->GetXaxis()->SetTitleSize(0.0425);
	H1_smp1->GetYaxis()->SetTitleSize(0.0425);
	H1_smp1->SetLineColor(1);
	H1_smp1->SetLineWidth(2);
	H1_smp1->SetStats(false);
	H1_smp1->SetTitle(Form("Channel %i;ADC;Entries", chIdx1));
	H1_smp1->DrawCopy("hist e");
	F1_ped[chIdx1-1]->Draw("same");
	F1_peak[chIdx1-1]->Draw("same");
	c2->cd(2);
	gPad->SetLogy();
	gPad->SetMargin(0.115, 0.085, 0.1, 0.1);
	TH1F* H1_smp2 = (TH1F*)H1_adc[chIdx2-1]->Clone();
	H1_smp2->GetXaxis()->SetTitleSize(0.0425);
	H1_smp2->GetYaxis()->SetTitleSize(0.0425);
	H1_smp2->SetLineColor(1);
	H1_smp2->SetLineWidth(2);
	H1_smp2->SetStats(false);
	H1_smp2->SetTitle(Form("Channel %i;ADC;Entries", chIdx2));
	H1_smp2->DrawCopy("hist e");
	F1_ped[chIdx2-1]->Draw("same");
	F1_peak[chIdx2-1]->Draw("same");
	//c2->Print(Form("%s.png", c2->GetName()));
	//c2->Print(Form("%s.eps", c2->GetName()));
	//c2->Print(Form("%s.pdf", c2->GetName()));

	TCanvas* c3 = new TCanvas("c3_gain", "", 400*3*2, 300*3);
	c3->Divide(2, 1);
	c3->cd(1)->SetMargin(0.115, 0.085, 0.1, 0.1);
	H1_gain->GetXaxis()->SetTitleSize(0.0425);
	H1_gain->GetYaxis()->SetTitleSize(0.0425);
	H1_gain->GetYaxis()->SetRangeUser(0.85, 1.1);
	H1_gain->SetLineColor(1);
	H1_gain->SetLineWidth(2);
	H1_gain->SetStats(false);
	H1_gain->SetTitle("Gain factors;Channel;Gain");
	H1_gain->DrawCopy("hist e");
	TLine* L1_gain = new TLine(0, 1, nCh, 1);
	L1_gain->SetLineColor(2);
	L1_gain->SetLineStyle(2);
	L1_gain->SetLineWidth(2);
	L1_gain->Draw("same");
	c3->cd(2)->SetMargin(0.115, 0.085, 0.1, 0.1);
	H1_gain_dist->GetXaxis()->SetRangeUser(0.85, 1.1);
	H1_gain_dist->GetXaxis()->SetTitleSize(0.0425);
	H1_gain_dist->GetYaxis()->SetTitleSize(0.0425);
	H1_gain_dist->SetStats(false);
	H1_gain_dist->SetLineColor(1);
	H1_gain_dist->SetLineWidth(2);
	H1_gain_dist->SetTitle("Gain factors distribution;Gain;Entries");
	H1_gain_dist->DrawCopy("hist e");
	TLine* L1_gain_dist = new TLine(1, 0, 1, H1_gain_dist->GetMaximum()*1.3);
	L1_gain_dist->SetLineColor(2);
	L1_gain_dist->SetLineStyle(2);
	L1_gain_dist->SetLineWidth(2);
	L1_gain_dist->Draw("same");
	//c3->Print(Form("%s.png", c3->GetName()));
	//c3->Print(Form("%s.eps", c3->GetName()));
	//c3->Print(Form("%s.pdf", c3->GetName()));
	#endif


	return;
}//Main
