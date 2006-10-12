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

/* hash should generate value from 0 to 0xFFFF from 4 bytes */
#define lookuphash(a, b, c, d) (((a)<<0)^((b)<<2)^((c)<<6)^((d)<<8))
//#define lookuphash(a, b, c, d) (((a)<<0)^((b)<<8)^((c)<<8)^((d)<<8))
//#define lookuphash(a, b, c, d) (((a)<<0)|((b)<<8))

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
	deque<uint32> *lookup = new deque<uint32>[0x10000];

	uint16 ocode;

	uint32 mind;
	uint32 mlen;

	uint32 nmlen = 0;

	uint32 ifpos = 0;
	while (instr.check(ifpos))
	{
		mlen = 0;
		uint32 lookupind;
		// check for a match (of at least length 4)
		if (instr.check(ifpos+3)) {
			lookupind = lookuphash(instr[ifpos], instr[ifpos+1], instr[ifpos+2], instr[ifpos+3]);
			deque<uint32> *clook = &lookup[lookupind];
			while ((clook->size() > 0) && (clook->front()+0x7FFF < ifpos))
				clook->pop_front();
			for (uint i = 0; i < clook->size(); ++i)
			{
				uint32 lind = (*clook)[i];
				uint32 clen = 0;
				while (instr.check(ifpos+clen) && (clen < 255) && (instr[ifpos+clen] == instr[lind+clen]))
					clen++;
				if (clen > 3 && clen > mlen) {
					mlen = clen;
					mind = lind;
				}
			}
		}

		// if no match increate nomatch count (and add lookup)
		if (mlen==0)
		{
			if (instr.check(ifpos+3))
				lookup[lookupind].push_back(ifpos);
			++nmlen;
			++ifpos;
		};
		// if match or nomatch limit exceeded, output nomatch (literal) codes
		if ((mlen!=0 && nmlen!=0) || nmlen == 127)
		{
			outstr.write(nmlen + (1<<7));
			for (uint i = ifpos-nmlen; i < ifpos; ++i)
				outstr.write(instr[i]);
			nmlen = 0;
		}
		// if match output codes for copy (and add lookups)
		if (mlen!=0) {
			mind = ifpos - mind;
			outstr.write(mind >> 8);
			outstr.write(mind & 0xFF);
			outstr.write(mlen);
			while (instr.check(ifpos) && (mlen > 0)) {
				if (instr.check(ifpos+3)) {
					lookupind = lookuphash(instr[ifpos], instr[ifpos+1], instr[ifpos+2], instr[ifpos+3]);
					lookup[lookupind].push_back(ifpos);
				}
				++ifpos;
				--mlen;
			}
		}
	}

	if (nmlen!=0)
	{
		outstr.write(nmlen + (1<<7));
		for (uint i = ifpos-nmlen; i < ifpos; ++i)
			outstr.write(instr[i]);
		nmlen = 0;
	}

	outstr.flush();

	delete[] lookup;

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}

