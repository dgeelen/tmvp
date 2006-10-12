//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused

#include "mystream.h"
#include <string>
#include <stdio.h>
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

	// TODO: maybe read other cmd line params???
	if (argc > 1) ifname = argv[1];
	if (argc > 2) ofname = argv[2];

	FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");

	MyInStream instr(ifhandle);
	MyOutStream outstr(ofhandle);
	pdeque *lookup = new pdeque[0xFFFFFF];

	for (int i = 0; i < 0xFFFFFF; ++i)
		lookup[i] = NULL;

	uint16 ocode;

	uint32 mind;
	uint32 mlen;

	uint32 nmlen = 0;

	uint32 ifpos = 0;
	while (instr.check(ifpos))
	{
		mlen = 0;
		if (instr.check(ifpos+3)) {
			uint32 lookupind = (instr[ifpos] << 0) | (instr[ifpos+1] << 8);// | (instr[ifpos+2] << 16);
			if (lookup[lookupind] != NULL) {
				deque<uint32> *clook = lookup[lookupind];
				while ((clook->size() > 0) && ((*clook)[0]+0x7FFF < ifpos))
					clook->pop_front();
				for (uint i = 0; i < clook->size(); ++i)
				{
					uint32 lind = (*clook)[i];
					uint32 clen = 2;
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
				uint32 lookupind = (instr[ifpos] << 0) | (instr[ifpos+1] << 8);// | (instr[ifpos+2] << 16);
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
					uint32 lookupind = (instr[ifpos] << 0) | (instr[ifpos+1] << 8);// | (instr[ifpos+2] << 16);
					if (lookup[lookupind] == NULL) lookup[lookupind] = new deque<uint32>;
					lookup[lookupind]->push_back(ifpos);
				}
				++ifpos;
				--mlen;
			}
		}
	}

	outstr.flush();

	for (int i = 0; i < 0xFFFFFF; ++i)
		if (lookup[i] != NULL) delete lookup[i];

	delete[] lookup;

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}

