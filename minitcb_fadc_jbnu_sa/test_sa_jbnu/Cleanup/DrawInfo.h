#ifndef _DRAWINFO_H
#define _DRAWINFO_H

namespace DrawInfo
{
TLegend* generalInfo(const char* infile)
{
	std::ifstream txtfile(infile);
	if(! txtfile.is_open())
	{
		cout <<"log file is not opend. Return false" << endl;
		return NULL;
	}	
	
	int input=0;
	int input_line=0;
	int storeLineNumIdx=0;
	int storeLineNumber[5]={
		/*FEE1 HV*/ 1,
		/*FEE2 HV*/ 2,
		/*Mult TH*/ 35,
		/*Trig info*/36,
		/*Zero supp*/37
	};

	int storeVal[5];
	while(txtfile >> input)
	{
		input_line++;
		if(input_line == storeLineNumber[storeLineNumIdx])
		{
			storeVal[storeLineNumIdx] = input;
			storeLineNumIdx++;
		}
	}

	TLegend* l1 = new TLegend(0.1, 0.1, 0.7, 0.7);
	l1->SetFillStyle(0);
	l1->SetTextFont(62);
	l1->SetBorderSize(0);
	l1->SetTextSize(0.06);
	l1->AddEntry("", Form("FEE1 HV: %d", storeVal[0]), "h");
	l1->AddEntry("", Form("FEE2 HV: %d", storeVal[1]), "h");
	l1->AddEntry("", Form("MultThresh: %d", storeVal[2]), "h");
	l1->AddEntry("", Form("Trigger Info: %d", storeVal[3]), "h");
	l1->AddEntry("", Form("ZeroSup(Off 0/ On 1): %d", storeVal[4]), "h");

	return l1;
}

//TLegend* channelThresh(const TString inputTxt)
TLegend* channelThresh(const char* infile)
{
	//std::ifstream txtfile( inputTxt.Data().c_str() );
	std::ifstream txtfile(infile);
	if(! txtfile.is_open())
	{
		cout <<"log file is not opend. Return false" << endl;
		return NULL;
	}	
	
	int input=0;
	int input_line=0;
	int storeLineNumIdx=0;
	int storeLineNumber[32]={
		3, 4, 5, 6, 7, 8, 9, 10,	//ch 1~8
		11, 12, 13, 14, 15, 16, 17, 18,	//ch 9~16
		19, 20, 21, 22, 23, 24, 25, 26,	//ch 17~24
		27, 28, 29, 30, 31, 32, 33, 34	//ch 25~32
	};

	int storeVal[32];
	while(txtfile >> input)
	{
		input_line++;
		if(input_line == storeLineNumber[storeLineNumIdx])
		{
			storeVal[storeLineNumIdx] = input;
			storeLineNumIdx++;
		}
	}

	TLegend* l1 = new TLegend(0.1, 0.1, 0.6, 0.6);
	l1->SetFillStyle(0);
	l1->SetTextFont(62);
	l1->SetBorderSize(0);
	l1->SetTextSize(0.05);
	l1->AddEntry("", "Threshold of each channels", "h");

	int i=0;
	const int chPerLine=4;
	for(i=0; i<32/chPerLine; i++)
	{
		l1->AddEntry("", 
			Form("Ch %d~%d: %d/%d/%d/%d/\n",
			chPerLine*i+1, //starting channel
			chPerLine*(i+1), //Finishing channel
			storeVal[chPerLine*i+0],	
			storeVal[chPerLine*i+1],	
			storeVal[chPerLine*i+2],	
			storeVal[chPerLine*i+3]), "h");
	}
	
	return l1;
}
}
#endif
