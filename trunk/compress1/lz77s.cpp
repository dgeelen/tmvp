//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused

#include "mystream.h"
#include <string>
#include <iostream>
/* LZ77 (Simple) */

/*
 * 1xxx xxxx											-> copy x literals from encoded stream
 * 0xxx xxxx xxxx xxxx yyyy yyyy	-> copy y bytes from decoded stream starting at offset x
 */

using namespace std;

typedef deque<uint32>* pdeque;

int main(int argc, char* argv[])
{
	string ifname = "-";
	string ofname = "-";

	// TODO: read cmd line params
	ifname = "c:\\export.aap";
	ofname = "c:\\export.lz7";

	FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");

	MyInStream instr(ifhandle);
	MyOutStream outstr(ofhandle);
	pdeque *lookup = new pdeque[0xFFFFFF];

	for (int i = 0; i < 0xFFFFFF; ++i)
		lookup[i] = NULL;

  uint16 ocode;

	uint32 mind;
	uint16 mlen;

	uint32 nmlen = 0;

	uint32 ifpos = 0;
	while (instr.check(ifpos))
	{
		mlen = 0;
		if (instr.check(ifpos+3)) {
			uint32 lookupind = (instr[ifpos] << 16) | (instr[ifpos+1] << 8) | (instr[ifpos+2] << 0);
			if (lookup[lookupind] != NULL) {
				deque<uint32> *clook = lookup[lookupind];
				while ((clook->size() > 0) && ((*clook)[0] < ifpos-0x7FFF))
					clook->pop_front();
				for (uint i = 0; i < clook->size(); ++i)
				{
					uint32 lind = (*clook)[i];
					uint32 clen = 3;
					while (instr.check(ifpos+clen) && (clen < 255) && (instr[ifpos+clen] == instr[lind+clen]))
						clen++;
					if (clen > 3 && clen > mlen) {
						mlen = clen;
						mind = lind;
					}
				}
			}
		}

		if (mlen==0)
		{
			if (instr.check(ifpos+2)) {
				uint32 lookupind = (instr[ifpos] << 16) | (instr[ifpos+1] << 8) | (instr[ifpos+2] << 0);
				if (lookup[lookupind] == NULL) lookup[lookupind] = new deque<uint32>;
				lookup[lookupind]->push_back(ifpos);
			}
			++nmlen;
			++ifpos;
		};
		if ((mlen!=0 && nmlen!=0) || nmlen == 255)
		{
			outstr.write(nmlen + (1<<7));
			for (uint i = ifpos-nmlen; i < ifpos; ++i)
				outstr.write(instr[i]);
			nmlen = 0;
		}
		if (mlen!=0) {
			mind = ifpos - mind;
			outstr.write(mind >> 8);
			outstr.write(mind & 0xFF);
			outstr.write(mlen);
			while (instr.check(ifpos) && (mlen > 0)) {
				if (instr.check(ifpos+2)) {
					uint32 lookupind = (instr[ifpos] << 16) | (instr[ifpos+1] << 8) | (instr[ifpos+2] << 0);
					if (lookup[lookupind] == NULL) lookup[lookupind] = new deque<uint32>;
					lookup[lookupind]->push_back(ifpos);
				}
				++ifpos;
				--mlen;
			}
		}
	}

	outstr.flush();
	char buf[100]="woofddfdfdft";
	fwrite(buf, 40, 1, ofhandle);
	fwrite(buf, 40, 1, ofhandle);


//	delete[] lookup;

	for (int i = 0; i < 0xFFFFFF; ++i)
		if (lookup[i] != NULL) delete lookup[i];


	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}
/*
 void getbestmatch(vector<uint8>& buf, uint32 sind, uint32& index, uint16& len)
{
	len = 0;
	for(uint i=0; i < sind; ++i)
		for(uint j=0; j < COPYLEN_SIZE && i+j<buf.size(); ++j)
			if (buf[sind+j]==buf[i+j]) {
				if (j >= len) {
					len = j+1;
					index = i;
				};
			} else j = COPYLEN_SIZE;
}
*/
/*
void LZ77_compress(string infile, string outfile)
{
	FILE *fin = fopen(infile.c_str(), "rb");
	FILE *fout = fopen(outfile.c_str(), "wb");
	vector<uint8> buffer;
	uint8  icode;
	uint8  ocode;

	uint32 cind = 0;
	uint32 mind;
	uint16 mlen;

	uint32 nmlen = 0;

	if (!fin || !fout) return;

	while (buffer.size() > cind || !feof(fin))
	{
		while (buffer.size() - cind < COPYLEN_SIZE && !feof(fin)) {
			fread(&icode, 1, 1, fin);
			if (!feof(fin)) buffer.push_back(icode);
		}
		while (cind > WINDOW_SIZE) {
			buffer.erase(buffer.begin());
			--cind;
		}

		getbestmatch(buffer, cind, mind, mlen);
		if (mlen<4) mlen = 0;
		if (mlen==0)
		{
			nmlen++;
			++cind;
		};
		if ((mlen!=0 && nmlen!=0) || nmlen == SKIPLEN_SIZE)
		{
			ocode = nmlen + (1<<7);
			fwrite(&ocode, 1, 1, fout);
			for (uint i = cind-nmlen; i < cind; ++i)
			{
				ocode = buffer[i];
				fwrite(&ocode, 1, 1, fout);
			}
			nmlen = 0;
		}
		if (mlen!=0) {
			mind = cind - mind;
			ocode = mind >> 8;
			fwrite(&ocode, 1, 1, fout);
			ocode = mind & 0xFF;
			fwrite(&ocode, 1, 1, fout);
			ocode = mlen;
			fwrite(&ocode, 1, 1, fout);
			cind += mlen;
		}
	}

	if (nmlen!=0)
	{
		ocode = nmlen + (1<<7);
		fwrite(&ocode, 1, 1, fout);
		for (uint i = cind-nmlen; i < cind; ++i)
		{
			ocode = buffer[i];
			fwrite(&ocode, 1, 1, fout);
		}
		nmlen = 0;
	}

	fclose(fin);
	fclose(fout);
};

*/

//---------------------------------------------------------------------------