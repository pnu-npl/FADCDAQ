#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TDirectory.h"

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <fstream>
#include <numeric>
#include <vector>

using namespace std;

const int nCh  = 32;
int wave_length = 496;
int nEvents;
FILE* fp; 

TCanvas* cPulse = new TCanvas("cPulse","cPulse",1000,500);
TLegend* LegInfo[nCh];
TPad* Pad_Pulse[nCh];
TH1* Frame_Pulse[nCh];
TH1F* H1_Pulse[nCh];

void ReadADC();
void DrawNextEv();

void Draw_waveform32ch( int RunNo = 20058, char* inDir = "data_20241119" )
{

   	TString FName = Form("%s/jbnu_daq_1_%5d.dat", inDir, RunNo); 

   	fp = fopen( FName, "rb");
	if( fp == NULL ) {
		cerr << FName << " Not Found!" << endl;
		return;
	}

	fseek(fp, 0L, SEEK_END);
	long long file_size = ftell(fp);
	fclose(fp);
	nEvents = file_size / 32 / 1024;
	cout << "Total Events: " << nEvents << endl;
    //nEvents = 100;

	fp = fopen( FName, "rb");
    gStyle->SetOptStat(0);
    //cPulse -> Divide(8,4,0,0);
    cPulse -> Divide(8,4);
    DrawNextEv();
    return;
}



void DrawNextEv()
{
    const int ADCMax = 200;

    ReadADC();
    for(int thisPad=0; thisPad<nCh; thisPad++){

        cPulse -> cd( thisPad+1 );
        Pad_Pulse[thisPad] = new TPad( Form("Pad_Pulse%d", thisPad), "", 0, 0, 1, 1); 
        //Pad_Pulse[thisPad] -> SetMargin(.00, .00, .00, .00);
        Pad_Pulse[thisPad] -> SetMargin(1.00, 1.00, 1.00, 1.00);
        Pad_Pulse[thisPad] -> Draw();
        Pad_Pulse[thisPad] -> cd();

        Frame_Pulse[thisPad] = Pad_Pulse[thisPad] -> DrawFrame( 0, -40, wave_length, ADCMax);
        //Frame_Pulse[thisPad] = Pad_Pulse[thisPad] -> DrawFrame( 220, -40, 240, ADCMax);
        Frame_Pulse[thisPad] -> Draw();
        //LegInfo[thisPad] -> Draw();

        Pad_Pulse[thisPad] -> cd();
        //H1_Pulse[thisPad] -> SetLineColor( Color );
        H1_Pulse[thisPad] -> Draw( "SAME HIST X" );
        //LegInfo[thisPad] -> AddEntry( H1_Pulse[CHIdx], Form( "%s-CH#%d", LR, CHIdx+1 ), "l" );
        //LegInfo[thisPad] -> AddEntry( H1_Pulse[thisPad], Form( "CH#%d", thisPad+1 ), "l" );
    }
    cPulse -> Draw();
}


void ReadADC( )
{
    char header[32];
    char data[2048];
    int itmp;
    int data_length;
    int channel;
    short adc;

    std::vector<short> tempADC;

    for(int iCh=0; iCh<nCh; iCh++){

        tempADC.resize(0);
        fread(header, 1, 32, fp);

        if(iCh == 0){

            data_length = header[0] & 0xFF;
            itmp = header[1] & 0xFF;
            itmp = itmp << 8;
            data_length = data_length + itmp;
            itmp = header[2] & 0xFF;
            itmp = itmp << 16;
            data_length = data_length + itmp;
            itmp = header[3] & 0xFF;
            itmp = itmp << 24;
            data_length = data_length + itmp;

            wave_length = (data_length - 32) / 2;

            cout << data_length << endl
                << wave_length << endl;
        }

        delete H1_Pulse[iCh];
        H1_Pulse[iCh] = new TH1F( Form("Pulse_ch%i", iCh+1), "", wave_length, 0, wave_length);

        fread(data, 1, data_length - 32, fp);

        for (int i = 0; i < wave_length; i++) {
            adc = data[2 * i] & 0xFF;
            itmp = data[2 * i + 1] & 0xFF;
            itmp = itmp << 8;
            adc = adc + itmp;

            // fill waveform
            //wave[channel - 1]->Fill(i * 16, adc);
            tempADC.push_back( adc );
        }

        float sum = std::accumulate(tempADC.begin(), tempADC.begin()+100, 0);
        float Pedestal = sum / 100;

        auto min_element_iter = std::min_element(tempADC.begin(), tempADC.end());
        short min_value = *min_element_iter;
        int min_index = std::distance(tempADC.begin(), min_element_iter);

        float PeakAmp = abs( (float)min_value - Pedestal );
        //cout << PeakAmp << endl;


        for( int c=0; c<wave_length; c++){
            H1_Pulse[iCh] -> Fill( c+1, (float)tempADC[c]-Pedestal );
        }


    }

}
