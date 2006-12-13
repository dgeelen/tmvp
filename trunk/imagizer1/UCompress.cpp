//---------------------------------------------------------------------------


#pragma hdrstop

#include "UCompress.h"
#include <vector>
#include <stdio>

using namespace std;

const WINDOW_SIZE = (1 << 16)-1;
const COPYLEN_SIZE = (1 << 7)-1;
const SKIPLEN_SIZE = (1 << 7)-1;

/*
16 bits output
12 = offset/literal len
4 = copy len (0000 means literal copy)
*/
void getbestmatch(vector<uint8>& buf, uint32 sind, uint32& index, uint16& len)
{
	len = 0;
	for(int i=0; i < sind; ++i)
		for(int j=0; j < COPYLEN_SIZE && i+j<buf.size(); ++j)
			if (buf[sind+j]==buf[i+j]) {
				if (j >= len) {
					len = j+1;
					index = i;
				};
			} else j = COPYLEN_SIZE;
}

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
			for (int i = cind-nmlen; i < cind; ++i)
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
		for (int i = cind-nmlen; i < cind; ++i)
		{
			ocode = buffer[i];
			fwrite(&ocode, 1, 1, fout);
		}
		nmlen = 0;
	}

	fclose(fin);
	fclose(fout);
};

void LZ77_decompress(string infile, string outfile)
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

	while (!feof(fin))
	{
		fread(&icode, 1, 1, fin);
		if (!feof(fin)) {
			if (icode & (1<<7)) {
				mlen = icode & ~(1<<7);
				for (int i = 0; i < mlen; ++i) {
					fread(&icode, 1, 1, fin);
					buffer.push_back(icode);
				}
			} else {
				mind = icode << 8;
				fread(&icode, 1, 1, fin);
				mind |= icode;
				fread(&icode, 1, 1, fin);
				mlen = icode;
				cind = buffer.size();
				for (int i = 0; i < mlen; ++i) {
					buffer.push_back(buffer[cind-mind+i]);
				}
			}
		}
	}

	for (int i = 0; i < buffer.size(); ++i) {
		ocode = buffer[i];
		fwrite(&ocode, 1, 1, fout);
	}

	fclose(fin);
	fclose(fout);
};

//---------------------------------------------------------------------------

#pragma package(smart_init)
