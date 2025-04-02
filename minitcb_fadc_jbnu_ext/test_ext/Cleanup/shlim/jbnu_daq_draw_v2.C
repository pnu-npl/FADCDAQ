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

map<int, int>   GetChMap(void);
map<int, float> GetCfMap(void);
TH1F* GetPed(const char* inFile, int cut, int sigma);





//============================================================================
void jbnu_daq_draw_v2(int runNo = 2080, const char* inPath = "data_hodoscope")
{
	gStyle->SetPalette(55);
	const char* inFile = Form("%s/jbnu_daq_%i_3.root", inPath, runNo);
	TString drawOpt = "1234";

	const int nCh = 32;
	map<int, int>   chMap = GetChMap(); //Channel mapping
	map<int, float> cfMap = GetCfMap(); //Calibration factors

	// Get pedestal map by using input data itself
    //---------------------------------------------------------------

	//TMP
	TH1F* H1_ped = new TH1F("H1_ped", ";ch;pedestal", 200, -100, 100);
	//for (int a=0; a<nCh; a++) H1_ped->GetBinContent(a+1, 0);
	//for (int a=0; a<nCh; a++) H1_ped->GetBinError(a+1, 50);
	//TH1F* H1_ped = GetPed(inFile, 50, 5); //Arguments: naive cut and sigma

    // Open file
    //---------------------------------------------------------------

    TFile* F = TFile::Open(inFile);
    TTree* T = (TTree*)F->Get("T"); //Header
    TTree* V = (TTree*)F->Get("V"); //Waveform
    int t_data_length;
    int t_run_number;
    int t_mid;
    int t_trigger_type;
    int t_tcb_trigger_number;
    int t_local_trigger_number;
    int t_local_trigger_pattern;
	int t_channel;

    int v_local_trigger_number;
    int v_channel;
    int v_wave_length;
    int v_adc[2500]; //~2100 for 32 us setting
    ULong64_t t_tcb_trigger_time;
    ULong64_t t_local_gate_time;
    T->SetBranchAddress("data_length",           &t_data_length);
    T->SetBranchAddress("run_number",            &t_run_number);
    T->SetBranchAddress("mid",                   &t_mid);
    T->SetBranchAddress("trigger_type",          &t_trigger_type);
    T->SetBranchAddress("tcb_trigger_number",    &t_tcb_trigger_number);
    T->SetBranchAddress("tcb_trigger_time",      &t_tcb_trigger_time);
    T->SetBranchAddress("local_trigger_number",  &t_local_trigger_number);
    T->SetBranchAddress("local_trigger_pattern", &t_local_trigger_pattern);
    T->SetBranchAddress("local_gate_time",       &t_local_gate_time);
    T->SetBranchAddress("channel",               &t_channel);

    V->SetBranchAddress("local_trigger_number",  &v_local_trigger_number);
    V->SetBranchAddress("channel",               &v_channel);
    V->SetBranchAddress("wave_length",           &v_wave_length);
    V->SetBranchAddress("adc",                   v_adc);

    const int nEventsT = T->GetEntriesFast();
    const int nEventsV = V->GetEntriesFast();

	//Get total # of triggers
    //---------------------------------------------------------------

	T->GetEntry(0);            const int trig_s = t_tcb_trigger_number;
	T->GetEntry(nEventsT - 1); const int trig_e = t_tcb_trigger_number;
	const int nTrig = trig_e - trig_s;
	
	cout << "nTrig: " << nTrig << endl;

	// Draw: Trigger vs. Trigger time
    //---------------------------------------------------------------

	if (drawOpt.Contains("1"))
	{
		TH2F *H2_trigtime[33];
		for (int ich=0; ich<33; ich++){
			H2_trigtime[ich] = new TH2F(Form("H2_trigvstime_ch%d",ich), "", (nTrig<100000)?nTrig:100000,trig_s,trig_e, 200,0,2*1.E10);
			H2_trigtime[ich]->SetTitle(Form("run %i, ch %d;iTrig;trigTime", runNo,ich));
			H2_trigtime[ich]->Sumw2();
		}

		for (int a=0; a<nEventsT; a++)
		{
			T->GetEntry(a);
			/*
			if ( t_tcb_trigger_number!=t_local_trigger_number ){
				cout << "WARNNING!! tcb trigger number and local trigger number are not matched" << endl;
				cout << "tcb trigger number: " << t_tcb_trigger_number << ", local trigger number: " << t_local_trigger_number << endl;
			}
			*/
			H2_trigtime[t_channel]->Fill(t_tcb_trigger_number, t_tcb_trigger_time);
		}

		TCanvas* c1 = new TCanvas(Form("%i_c1_trigtime", runNo), "", 250*6*1.2, 250*6*1.0); 
		c1->SetName(Form("run%i_trigtime", runNo));
		c1->Divide(6,6);
		for (int ii=0; ii<33; ii++){
			c1->cd(ii+1);
			gPad->SetMargin(0.1,0.02,0.1,0.02);
			H2_trigtime[ii]->SetStats(false);
			H2_trigtime[ii]->DrawCopy("colz");
			cout << "CH: " << ii << ", # OF EVT: " << H2_trigtime[ii]->Integral() << endl;
		}
	}//1, Trigger vs. Trigger time

	return;

	// Waveform analysis
    //---------------------------------------------------------------

	TH2F* H2_trighit = new TH2F("H2_trigvshit", "", nTrig,trig_s,trig_e, nCh,0.5,nCh+0.5); H2_trighit->Sumw2();
	TH2F* H2_trigmax = new TH2F("H2_trigvsmax", "", nTrig,trig_s,trig_e, nCh,0.5,nCh+0.5); H2_trigmax->Sumw2();
	TH2F* H2_trigene = new TH2F("H2_trigvse",   "", nTrig,trig_s,trig_e, nCh,0.5,nCh+0.5); H2_trigene->Sumw2();
	H2_trighit->SetTitle(Form("run%i, valid hits;iTrig;ch", runNo));
	H2_trigmax->SetTitle(Form("run%i, pulse max.;iTrig;ch", runNo));
	H2_trigene->SetTitle(Form("run%i, energy;iTrig;ch", runNo));

	V->GetEntry(0); const int wLen = v_wave_length;
	TH1F* H1_temp = new TH1F("H1_wavetemp", "", wLen, 0, wLen); H1_temp->Sumw2();

	for (int a=0; a<nEventsV; a++)
	{
		V->GetEntry(a);

		const int ltn  = v_local_trigger_number; //In general, starts from 0
		const int ch   = chMap[v_channel]; //v_channel; //Starts from 1
		const int wlen = v_wave_length;

		bool hitExist = false;
		for (int b=0; b<wlen; b++)
		{
			const int adc    = v_adc[b];
			const int pedcut = H1_ped->GetBinContent(ch) + H1_ped->GetBinError(ch);
			if (adc > pedcut) { hitExist = true; break; }
		}//b
		if (hitExist == false) continue;

		H1_temp->Reset();
		for (int b=0; b<wlen; b++)
		{
			const int adc = v_adc[b];
			H1_temp->SetBinContent(b+1, adc);
		}

		H2_trighit->SetBinContent(ltn+1, ch, 1);
		H2_trigmax->SetBinContent(ltn+1, ch, H1_temp->GetMaximum());

		//Get energy: (sum of ADCs +/- 20 around the peak) x calibration factor
		const int maxBin = H1_temp->GetMaximumBin();
		const int maxPos = H1_temp->GetBinCenter(maxBin);
		const int xBin1 = H1_temp->GetXaxis()->FindBin(maxPos - 15);
		const int xBin2 = H1_temp->GetXaxis()->FindBin(maxPos + 15);
		float adcSum = 0;
		for (int a=xBin1; a<=xBin2; a++) adcSum += H1_temp->GetBinContent(a);
		H2_trigene->SetBinContent(ltn+1, ch, adcSum * cfMap[ch]);
	}//a

	if (drawOpt.Contains("2"))
	{
		TCanvas* c2 = new TCanvas(Form("%i_c2_trigvs", runNo), "", 800*2*1.2, 600*2*1.2);
		c2->SetName(Form("run%i_trigvs", runNo));
		c2->Divide(2, 2);
		c2->cd(1); H2_trighit->DrawCopy("colz");
		c2->cd(2); H2_trigmax->DrawCopy("colz");
		c2->cd(3); H2_trigene->DrawCopy("colz");
	}

	if (drawOpt.Contains("3"))
	{
		TH1F* H1_trighit = (TH1F*)H2_trighit->ProjectionY();
		TH1F* H1_trigmax = (TH1F*)H2_trigmax->ProjectionY(); H1_trigmax->Divide(H1_trighit);
		TH1F* H1_trigene = (TH1F*)H2_trigene->ProjectionY(); H1_trigene->Divide(H1_trighit);

		TCanvas* c3 = new TCanvas(Form("%i_c3_trigvs_1D", runNo), "", 800*2*1.2, 600*2*1.2);
		c3->SetName(Form("run%i_trigvs", runNo));
		c3->Divide(2, 2);
		c3->cd(1); H1_trighit->DrawCopy("hist e");
		c3->cd(2); H1_trigmax->DrawCopy("hist e");
		c3->cd(3); H1_trigene->DrawCopy("hist e");
	}

	return;
}//Main





//==========================
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

//============================
map<int, float> GetCfMap(void)
{
    std::map<int, float> cfMap;

    cfMap.insert( std::pair<int, float> ( 1, 1.01281) );
    cfMap.insert( std::pair<int, float> ( 2, 0.999058) );
    cfMap.insert( std::pair<int, float> ( 3, 0.949068) );
    cfMap.insert( std::pair<int, float> ( 4, 1.05528) );
    cfMap.insert( std::pair<int, float> ( 5, 0.957217) );
    cfMap.insert( std::pair<int, float> ( 6, 1.02508) );
    cfMap.insert( std::pair<int, float> ( 7, 1.05675) );
    cfMap.insert( std::pair<int, float> ( 8, 1.10437) );
    cfMap.insert( std::pair<int, float> ( 9, 0.974009) );
    cfMap.insert( std::pair<int, float> (10, 0.953719) );
    cfMap.insert( std::pair<int, float> (11, 0.987462) );
    cfMap.insert( std::pair<int, float> (12, 1.22136) );
    cfMap.insert( std::pair<int, float> (13, 0.981164) );
    cfMap.insert( std::pair<int, float> (14, 1.02641) );
    cfMap.insert( std::pair<int, float> (15, 0.977444) );
    cfMap.insert( std::pair<int, float> (16, 1.10649) );

    cfMap.insert( std::pair<int, float> (17, 1.03359) );
    cfMap.insert( std::pair<int, float> (18, 0.972918) );
    cfMap.insert( std::pair<int, float> (19, 1.02953) );
    cfMap.insert( std::pair<int, float> (20, 1.03279) );
    cfMap.insert( std::pair<int, float> (21, 0.982285) );
    cfMap.insert( std::pair<int, float> (22, 0.979264) );
    cfMap.insert( std::pair<int, float> (23, 0.95666) );
    cfMap.insert( std::pair<int, float> (24, 0.987208) );
    cfMap.insert( std::pair<int, float> (25, 0.939534) );
    cfMap.insert( std::pair<int, float> (26, 0.936297) );
    cfMap.insert( std::pair<int, float> (27, 0.929513) );
    cfMap.insert( std::pair<int, float> (28, 1.01255) );
    cfMap.insert( std::pair<int, float> (29, 0.969629) );
    cfMap.insert( std::pair<int, float> (30, 0.935796) );
    cfMap.insert( std::pair<int, float> (31, 0.95885) );
    cfMap.insert( std::pair<int, float> (32, 1.0634) );

    return cfMap;
}//map

//==================================================
TH1F* GetPed(const char* inFile, int cut, int sigma)
{
	map<int, int> chMap = GetChMap();

	TFile* F = TFile::Open(inFile);
	TTree* V = (TTree*)F->Get("V");
	int local_trigger_number;
    int channel;
    int wave_length;
    int adc[2500];
    V->SetBranchAddress("local_trigger_number",  &local_trigger_number);
    V->SetBranchAddress("channel",               &channel);
    V->SetBranchAddress("wave_length",           &wave_length);
    V->SetBranchAddress("adc",                   adc);
	const int nEvents = V->GetEntriesFast();

	//Get max. # of triggers
	V->GetEntry(nEvents-1);
	const int nTrig = local_trigger_number;
	TH2F* H2 = new TH2F("H2_ped", ";iTrig;ch", nTrig,0,nTrig, 32,1,33);

	//Loop over trees
	for (int a=0; a<nEvents; a++)
	{
		V->GetEntry(a);

		const int ltn  = local_trigger_number;
		const int ch   = chMap[channel]; //channel;
		const int wlen = wave_length;

		//Check if this event has a valid hit: continue if true
		bool hitExist = false;
		for (int b=0; b<wlen; b++)
		{
			if (adc[b] > cut) { hitExist = true; break; }
		}
		if (hitExist) continue;

		//Fill pedestal values
		for (int b=0; b<wlen; b++) H2->SetBinContent(ltn+1, ch, adc[b]);
	}//a
	//TCanvas* c1 = new TCanvas("c1_ped_temp", "", 800*1.2, 600*1.2); c1->cd(1); H2->DrawCopy("colz");

	//Fill pedestal values
	TH1F* H1_ped = new TH1F("H1_ped", ";ch;pedestal", 32, 0.5, 32.5);

	TH1F* H1_temp = new TH1F("H1_temp", ";adc", cut*2, -cut, cut); H1_temp->Sumw2();
	for (int y=0; y<H2->GetNbinsY(); y++)
	{
		H1_temp->Reset();

		for (int x=0; x<H2->GetNbinsX(); x++)
		{
			const float val = H2->GetBinContent(x+1, y+1);
			H1_temp->Fill(val);
		}

		H1_ped->SetBinContent(y+1, H1_temp->GetMean());
		H1_ped->SetBinError(y+1, H1_temp->GetRMS()*sigma);
	}//ch

	//Cleanup
	H1_temp->Delete();
	H2->Delete();
	V->ResetBranchAddresses();

	return H1_ped;
}//GetPed
