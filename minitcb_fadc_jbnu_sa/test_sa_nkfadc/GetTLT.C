#include <TString.h>
#include <iostream>
using namespace std;

bool Judge(int chA, bool cond_AB, int chB, bool cond_BC, int chC, bool cond_CD, int chD)
{
	bool Flag = false;

	if      ( cond_AB &&  cond_BC &&  cond_CD) { if ((chA && chB) && (chC && chD)) Flag = true; }
	else if ( cond_AB && !cond_BC &&  cond_CD) { if ((chA && chB) || (chC && chD)) Flag = true; }
	else if ( cond_AB &&  cond_BC && !cond_CD) { if ((chA && chB) && (chC || chD)) Flag = true; }
	else if ( cond_AB && !cond_BC && !cond_CD) { if ((chA && chB) || (chC || chD)) Flag = true; }
	else if (!cond_AB &&  cond_BC &&  cond_CD) { if ((chA || chB) && (chC && chD)) Flag = true; }
	else if (!cond_AB && !cond_BC &&  cond_CD) { if ((chA || chB) || (chC && chD)) Flag = true; }
	else if (!cond_AB &&  cond_BC && !cond_CD) { if ((chA || chB) && (chC || chD)) Flag = true; }
	else { cout <<"No pre-defined judgment: return false statement by default...\n"; return false; }

	//If a combination does not exists add it by yourself: true for &&, false for ||
	//else if (cond_AB && cond_BC && cond_CD) { if ((chA XX chB) XX (chC XX chD)) Flag = true; }

	return Flag;
}//Judge

unsigned long GetTLT(
		int ch1st = 1,
		int ch2nd = 2,
		int ch3rd = 3,
		int ch4th = 4,
		bool cond_12 = true, //true for AND, false for OR
		bool cond_23 = false,
		bool cond_34 = true,
		bool Show = true
		)
{
	/*
	Bits: 15-0, Channels: 4-1
	---------------------------------------------

	Ch4 Ch3 Ch2 Ch1 Data (bit)

	Y Y Y Y  bit15 -> 0 or 1 (0 for false, 1 for true)
	Y Y Y N  bit14
	Y Y N Y  bit13
	Y Y N N  bit12

	Y N Y Y  bit11
	Y N Y N  bit10
	Y N N Y  bit9
	Y N N N  bit8

	N Y Y Y  bit7
	N Y Y N  bit6
	N Y N Y  bit5
	N Y N N  bit4

	N N Y Y  bit3
	N N Y N  bit2
	N N N Y  bit1
	N N N N  bit0

	---------------------------------------------
	- Any (self): 1111 1111 1111 1110 = 0xFFFE
	- 1&2 | 3&4:  1111 1000 1000 1000 = 0xF888
	- 1&3 | 2&4:  1110 1100 1010 0000 = 0xECA0
	- 1|2 & 3|4:  1110 1110 1110 0000 = 0xEEE0
	*/

	//Map in orthodox order: bit 0-15, ch 1-4 (whoever made original map just go to hell)
	bool Map[16][4] =
	{
		{0, 0, 0, 0}, {1, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, //Bit 0-3 (ch 1-4)
		{0, 0, 1, 0}, {1, 0, 1, 0}, {0, 1, 1, 0}, {1, 1, 1, 0}, //Bit 4-7
		{0, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 0, 1}, {1, 1, 0, 1}, //Bi1 8-11
		{0, 0, 1, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 1}, //bit 12-15
	};

	if (Show) printf("%d%s%d %s %d%s%d = ",ch1st,cond_12?"&":"|",ch2nd,cond_23?"&":"|",ch3rd,cond_34?"&":"|",ch4th);
	bool Val[16] = {0};
	for (int i=15; i>-1; i--) //Bit
	{
		const int chA = ch1st - 1;
		const int chB = ch2nd - 1;
		const int chC = ch3rd - 1;
		const int chD = ch4th - 1;
		Val[i] = Judge(Map[i][chA], cond_12, Map[i][chB], cond_23, Map[i][chC], cond_34, Map[i][chD]);
		if (Show) printf("%d%s%s", Val[i], (i%4==0)?" ":"", (i==0)?"= ":"");
	}//bit, ch

	unsigned int TLT_d1 = Val[15]<<3 | Val[14]<<2 | Val[13]<<1 | Val[12];
	unsigned int TLT_d2 = Val[11]<<3 | Val[10]<<2 | Val[ 9]<<1 | Val[ 8];
	unsigned int TLT_d3 = Val[ 7]<<3 | Val[ 6]<<2 | Val[ 5]<<1 | Val[ 4];
	unsigned int TLT_d4 = Val[ 3]<<3 | Val[ 2]<<2 | Val[ 1]<<1 | Val[ 0];
	unsigned long TLT = printf("0x%X%X%X%X\n", TLT_d1, TLT_d2, TLT_d3, TLT_d4);

	return TLT;
}//Main
