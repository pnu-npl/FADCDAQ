#include "TFile.h"
#include "TTree.h"
#include "TString.h"

#include <bitset>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>

#define DEBUG 1

using namespace std;

bool Exist(set<int> List, int id)
{
	if (List.find(id) != List.end()) return true;
	else return false;
}

void jbnu_daq_decode2(const char* inFile = "10015.dat")
{
	cout <<Form("Start decoding for %s...\n", inFile);

	//Get data file size (the size should be < 2 GB)
	FILE *fp = fopen(inFile, "rb");
	fseek(fp, 0L, SEEK_END);
	int file_size = ftell(fp);
	fclose(fp);

	//Variables
	char header[32];
	char data[2048];
	int data_read = 0;

	//----------------------------------------------------------

	map<int, long long> map_ltntime; //ckim, local trigger number and its time
	set<int> set_ltn; //ckim, local trigger number

	map<int, unsigned int> map_ltn_chvec;
	map<int, int> map_ltn_rnumber;
	map<int, int> map_ltn_dlength;
	map<int, int> map_ltn_mid;
	map<int, int> map_ltn_trigtype;
	map<int, int> map_ltn_tcbtrignum;
	map<int, unsigned long> map_ltn_tcbtrigtime;
	map<int, int> map_ltn_localtrigpattern;
	map<int, unsigned long> map_ltn_localgatetime;
	map<int, vector<vector<int>>> map_ltn_waveform;

	TString inFileName = inFile;
	inFileName.ReplaceAll("dat", "root");
	TFile* F = new TFile(Form("out2_%s", inFileName.Data()), "recreate");
	TTree* T = new TTree("T", "Header");
	//TTree* U = new TTree("U", "Spectrum");
	//TTree* V = new TTree("V", "Waveform");

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
	int u_charge[32];
	int u_timing[32];
	int u_hitFlag[32];
	int v_local_trigger_number;
	int v_channel;
	int v_wave_length;
	int v_adc[32][500] = {0};
	T->Branch("data_length",           &t_data_length,           "data_length/I");
	T->Branch("run_number",            &t_run_number,            "run_number/I");
	T->Branch("mid",                   &t_mid,                   "mid/I");
	T->Branch("trigger_type",          &t_trigger_type,          "trigger_type/I");
	T->Branch("tcb_trigger_number",    &t_tcb_trigger_number,    "tcb_trigger_number/I");
	T->Branch("tcb_trigger_time",      &t_tcb_trigger_time,      "tcb_trigger_time/l");
	T->Branch("local_trigger_number",  &t_local_trigger_number,  "local_trigger_number/I");
	T->Branch("local_trigger_pattern", &t_local_trigger_pattern, "local_trigger_pattern/I");
	T->Branch("local_gate_time",       &t_local_gate_time,       "local_gate_time/l");
	T->Branch("wave_length",           &v_wave_length,           "wave_length/I");
	T->Branch("adc",                   v_adc,                    "adc[32][500]/I");
	/*
	U->Branch("local_trigger_number",  &u_local_trigger_number,  "local_trigger_number/I");
	U->Branch("charge",                u_charge,                 "charge[32]/I");
	U->Branch("timing",                u_timing,                 "timing[32]/I");
	U->Branch("hit_flag",              u_hitFlag,                "hit_flag[32]/I");
	V->Branch("local_trigger_number",  &v_local_trigger_number,  "local_trigger_number/I");
	V->Branch("channel",               &v_channel,               "channel/I");
	V->Branch("wave_length",           &v_wave_length,           "wave_length/I");
	V->Branch("adc",                   v_adc,                    "adc[wave_length]/I");
	*/

	//----------------------------------------------------------

	//Open data file and read event by event
	fp = fopen(inFile, "rb");

	while (data_read < file_size)
	{
		//Read header
		//++++++++++++++++++++++++++++++++++++++++++++

		fread(header, 1, 32, fp);

		int data_length = 0;
		for (int a=0; a<4; a++) data_length += ((int)(header[a] & 0xFF) << 8*a);

		int run_number = 0;
		for (int a=0; a<2; a++) run_number += ((int)(header[a+4] & 0xFF) << 8*a);

		int trigger_type = ((int)header[6] & 0xFF);

		int tcb_trigger_number = 0;
		for (int a=0; a<4; a++) tcb_trigger_number += ((int)(header[a+7] & 0xFF) << 8*a);

		long long tcb_trigger_fine_time = ((long long)header[11] & 0xFF);

		long long tcb_trigger_coarse_time = 0;
		for (int a=0; a<3; a++) tcb_trigger_coarse_time += ((long long)(header[a+12] & 0xFF) << 8*a);

		long long tcb_trigger_time = (tcb_trigger_fine_time * 8) + (tcb_trigger_coarse_time * 1000);

		int mid = ((int)header[15] & 0xFF);

		int channel = ((int)header[16] & 0xFF);

		int local_trigger_number = 0;
		for (int a=0; a<4; a++) local_trigger_number += ((int)(header[a+17] & 0xFF) << 8*a);

		int local_trigger_pattern = 0;
		for (int a=0; a<4; a++) local_trigger_pattern += ((int)(header[a+21] & 0xFF) << 8*a);

		long long local_gate_fine_time = ((long long)header[25] & 0xFF);

		long long local_gate_coarse_time = 0;
		for (int a=0; a<6; a++) local_gate_coarse_time += ((long long)(header[a+26] & 0xFF) << 8*a);

		long long local_gate_time = (local_gate_fine_time * 8) + (local_gate_coarse_time * 1000);

		int wave_length = (data_length - 32) / 2;

		if ( 0 ){
			cout <<endl;
			cout <<"mid: " <<mid <<endl;
			cout <<"channel: " <<channel <<endl;
			cout <<"run_number: " <<run_number <<endl;
			cout <<"data_length: " <<data_length <<endl;
			cout <<"trigger_type: " <<trigger_type <<endl;
			cout <<"tcb_trigger_number: " <<tcb_trigger_number <<endl;
			cout <<"tcb_trigger_fine_time: " <<tcb_trigger_fine_time <<endl;
			cout <<"tcb_trigger_coarse_time: " <<tcb_trigger_coarse_time <<endl;
			cout <<"tcb_trigger_time: " <<tcb_trigger_time <<endl;
			cout <<"local_trigger_number: " <<local_trigger_number <<endl;
			cout <<"local_trigger_pattern: " <<local_trigger_pattern <<endl;
			cout <<"local_gate_fine_time: " <<local_gate_fine_time <<endl;
			cout <<"local_gate_coarse_time: " <<local_gate_coarse_time <<endl;
			cout <<"local_gate_time: " <<local_gate_time <<endl;
			cout <<Form("%i %i %s\n", local_trigger_number, channel, channel==0?"":"!");
		}

		const int ltn = local_trigger_number; //ckim
		if ( Exist(set_ltn, ltn) == false )
		{
			/*
			set_ltn.insert(ltn);

			t_data_length           = data_length;
			t_run_number            = run_number;
			t_mid                   = mid;
			t_trigger_type          = trigger_type;
			t_tcb_trigger_number    = tcb_trigger_number;
			t_tcb_trigger_time      = tcb_trigger_time;
			t_local_trigger_number  = local_trigger_number;
			t_local_trigger_pattern = local_trigger_pattern;
			t_local_gate_time       = local_gate_time;
			T->Fill();
			*/

			if ( map_ltn_chvec.find(local_trigger_number)==map_ltn_chvec.end() )
			{
				map_ltn_dlength.insert(make_pair(local_trigger_number, data_length));
				map_ltn_rnumber.insert(make_pair(local_trigger_number, run_number));
				map_ltn_mid.insert(make_pair(local_trigger_number, mid));
				map_ltn_trigtype.insert(make_pair(local_trigger_number, trigger_type));
				map_ltn_tcbtrignum.insert(make_pair(local_trigger_number, tcb_trigger_number));
				map_ltn_tcbtrigtime.insert(make_pair(local_trigger_number, tcb_trigger_time));
				map_ltn_localtrigpattern.insert(make_pair(local_trigger_number, local_trigger_pattern));
				map_ltn_localgatetime.insert(make_pair(local_trigger_number, local_gate_time));
				map_ltn_chvec.insert(make_pair(local_trigger_number, (unsigned int)(0x00000000)));  
				map_ltn_waveform[local_trigger_number] = vector<vector<int>>(32, vector <int>(500,0));

				if ( DEBUG ){
					cout <<Form("Initial trigger entry: %u, %u\n", 
							local_trigger_number, 
							map_ltn_chvec.find(local_trigger_number)->second 
							);
				}

			}

			//map_ltntime.insert(std::pair<int, long long>(local_trigger_number, local_gate_time));
		}

		//Check local trigger time is the same for a local trigger number
		for (auto iter : map_ltntime)
		{
			if (iter.first==ltn && iter.second!=local_gate_time)
			{
				cout <<Form("WARNING! Unmatching trigger number/time!\n %i - %lli",	ltn, local_gate_time);
			}
		}

		//Read body, data_length - 32 bytes (header)
		//++++++++++++++++++++++++++++++++++++++++++++

		fread(data, 1, data_length - 32, fp);

		int charge[32] = {0};
		int timing[32] = {0};
		int hitFlag[32] = {0};
		int adc[wave_length];
		for (int a=0; a<wave_length; a++) adc[a] = 0;

		if (channel == 0) //Spectrum data
		{
			/*
			for (int ch=0; ch<32; ch++)
			{
				for (int a=0; a<3; a++) charge[ch] += ((int)(data[6 * ch + a + 0] & 0xFF) << 8*a);
				for (int a=0; a<2; a++) timing[ch] += ((int)(data[6 * ch + a + 3] & 0xFF) << 8*a);
				hitFlag[ch] = data[6 * ch + 5] & 0xFF;
			}//a

			u_local_trigger_number = local_trigger_number;
			for (int a=0; a<32; a++) u_charge[a] = charge[a];
			for (int a=0; a<32; a++) u_timing[a] = timing[a];
			for (int a=0; a<32; a++) u_hitFlag[a] = hitFlag[a];
			U->Fill();
			*/

			if ( DEBUG ){
				//cout <<endl <<u_local_trigger_number <<endl;
				//for (int a=0; a<32; a++) cout <<Form("%i, %i, %i, %i\n", a, u_hitFlag[a], u_timing[a], u_charge[a]);
			}
		}//Spectrum
		else //Waveform data
		{
			for (int a=0; a<wave_length; a++)
			{
				int t_adc1 = (data[2 * a + 0] & 0xFF);
				int t_adc2 = (data[2 * a + 1] & 0xFF) << 8;
				adc[a] = (short)(t_adc1 + t_adc2);

				map_ltn_waveform[local_trigger_number][channel-1][a] = adc[a];
			}//a

			/*
			v_local_trigger_number = local_trigger_number;
			v_channel = channel;
			v_wave_length = wave_length;
			for (int a=0; a<wave_length; a++) v_adc[a] = adc[a];
			V->Fill();
			*/

			unsigned int chvec_new = (unsigned int)(1 << (channel-1));
			unsigned int chvec_old = map_ltn_chvec.find(local_trigger_number)->second;
			map_ltn_chvec[local_trigger_number] = chvec_new + chvec_old;

			if ( DEBUG ){
				//cout <<endl <<v_local_trigger_number <<endl;
				//for (int a=0; a<v_wave_length; a++) cout <<Form("%i, %i\n", v_channel, v_adc[a]);

				cout <<Form("Add a channel: %u, %u, %u, %u\n", local_trigger_number, channel, chvec_new, chvec_old);
			}

			if ( map_ltn_chvec.find(local_trigger_number)->second==(unsigned int)(~(0x00000000)) )
			{
				if ( DEBUG ){
					cout <<Form("All channels added: %u, %u\n", local_trigger_number, map_ltn_chvec.find(local_trigger_number)->second);
				}

				t_data_length = map_ltn_dlength.find(local_trigger_number)->second;
				t_run_number = map_ltn_rnumber.find(local_trigger_number)->second;
				t_mid = map_ltn_mid.find(local_trigger_number)->second;
				t_trigger_type = map_ltn_trigtype.find(local_trigger_number)->second;
				t_tcb_trigger_number = map_ltn_tcbtrignum.find(local_trigger_number)->second;
				t_tcb_trigger_time = map_ltn_tcbtrigtime.find(local_trigger_number)->second;
				t_local_trigger_number = local_trigger_number;
				t_local_trigger_pattern = map_ltn_localtrigpattern.find(local_trigger_number)->second;
				t_local_gate_time = map_ltn_localgatetime.find(local_trigger_number)->second;

				v_wave_length = wave_length;
				for (int ich=0; ich<32; ich++){
					for (int ii=0; ii<wave_length; ii++){
						v_adc[ich][ii] = map_ltn_waveform[local_trigger_number][ich][ii];
					}
				}

				T->Fill();

				map_ltn_waveform[local_trigger_number].clear();
			}
		}//Waveform

		data_read = data_read + data_length;
	}//while

	fclose(fp);

	F->Write();
	F->Close();

	return;
}//Main
