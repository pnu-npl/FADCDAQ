#include <stdio.h>

void plot_spectrum_1ch(void)
{
  char filename[100];
  FILE *fp;
  int file_size;
  TH1F **wave;
  TH1F **spect;
  char plot_name[10];
  char plot_title[10];
  int first_waveform;
  int data_read;
  char header[32];
  char data[2048];
  int data_length;
  int run_number;
  int trigger_type;
  int tcb_trigger_number;
  long long tcb_trigger_fine_time;
  long long tcb_trigger_coarse_time;
  long long tcb_trigger_time;
  int mid;
  int channel;
  int local_trigger_number;
  int local_trigger_pattern;
  long long local_gate_fine_time;
  long long local_gate_coarse_time;
  long long local_gate_time;
  int itmp;
  long long ltmp;
  int ch;
  int charge[32];
  int timing[32];
  int hit_flag[32];
  int wave_length;
  short adc;
  int i;
  TH1F **plot;
  
  // filename here
  sprintf(filename, "jbnu_daq_1.dat");
  
  // get data file size, size should be less than 2 Gbytes
  fp = fopen(filename, "rb");
  fseek(fp, 0L, SEEK_END);
  file_size = ftell(fp);
  fclose(fp);
  
  // define some histograms
  TCanvas *c1 = new TCanvas("c2", "Spectrum", 800, 400);
  wave = new TH1F *[32];
  spect = new TH1F *[32];
  for (ch = 0; ch < 32; ch++) {
    sprintf(plot_name, "spect%d", ch);
    sprintf(plot_title, "ch %d", ch + 1);
    spect[ch] = new TH1F(plot_name, plot_title, 1024, 0, 4096 * 16); // adjsut range by peak sum width
    spect[ch]->Reset();
  }
  
  // open data file  
  fp = fopen(filename, "rb");
  
  data_read = 0;
  first_waveform = 1;
  
  while (data_read < file_size) {
    // read header
    fread(header, 1, 32, fp);
    
    // get header information
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

    channel = header[16] & 0xFF;

    // read body, data_length - 32(header) bytes
    fread(data, 1, data_length - 32, fp);

    // for fast(spectrum) data
    if (channel == 0) {
      // get fast data
      for (ch = 1; ch <= 1; ch++) {
        charge[ch - 1] = data[6 * (ch - 1)] & 0xFF;
        itmp = data[6 * (ch - 1) + 1] & 0xFF;
        itmp = itmp << 8;
        charge[ch - 1] = charge[ch - 1] + itmp;
        itmp = data[6 * (ch - 1) + 2] & 0xFF;
        itmp = itmp << 16;
        charge[ch - 1] = charge[ch - 1] + itmp;
        
        // fill charge to spectrum
        spect[ch - 1]->Fill(charge[ch - 1]);
      }
    }
    
    data_read = data_read + data_length;
    printf("length = %d, %d / %d\n", data_length, data_read, file_size);
  }
  
  // plot spectrum
  for (ch = 1; ch <= 1; ch++) {
    spect[ch - 1]->Draw();      
  }  
  c1->Modified();
  c1->Update();
  
  fclose(fp);
}


