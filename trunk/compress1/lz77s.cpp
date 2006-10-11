//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused

#include "MyStream.h"
/* LZ77 (Simple) */

/*
 * 1xxx xxxx											-> copy x literals from encoded stream
 * 0xxx xxxx xxxx xxxx yyyy yyyy	-> copy y bytes from decoded stream starting at offset x
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
int main(int argc, char* argv[])
{
	std::deque<uint8> blah;


	return 0;
}
//---------------------------------------------------------------------------
