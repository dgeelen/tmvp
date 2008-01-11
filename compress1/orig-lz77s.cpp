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

const uint32 HASH_SIZE = (1 << 24);
const uint32 WIND_SIZE = (1 << 15);

const uint32 HASH_MASK = HASH_SIZE - 1;
const uint32 WIND_MASK = WIND_SIZE - 1;

/* hash should generate value from 0 to HASH_SIZE-1 from 4 bytes */
//#define lookuphash(a, b, c, d) (((a)<<0)^((b)<<5)^((c)<<11)^((d)<<16))
//#define lookuphash(a, b, c, d) (((a)<<0)^((b)<<8)^((c)<<8)^((d)<<8))
//#define lookuphash(a, b, c, d) (((a)<<0)|((b)<<8))

#define nexthash(new) lookupind = (((lookupind<<6)^(new))&HASH_MASK)

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

//	int32 hashhead[HASH_SIZE];
	int32* hashhead = new int32[HASH_SIZE];
	int32* hashprev = new int32[WIND_SIZE];

	for (int i = 0; i < HASH_SIZE; ++i)
		hashhead[i] = -(int32)WIND_SIZE;

	uint32 mind;
	uint32 mlen;

	uint32 nmlen = 0;

	uint32 ifpos = 0;

	uint32 lookupind = 0;

	nexthash(instr[0]);
	nexthash(instr[1]);
	nexthash(instr[2]);
	nexthash(instr[3]);

	while (instr.check(ifpos))
	{
		mlen = 0;
		// check for a match (of at least length 4)
		if (instr.check(ifpos+3)) {
			instr.check(ifpos+255);
			uint32 bound = instr.getmax() - ifpos;
			if (bound > 255) bound = 255;

			for (int32 lind = hashhead[lookupind]; ifpos < lind + WIND_SIZE; lind = hashprev[lind & WIND_MASK])
			{
				uint32 clen = 0;
				while ((clen < bound) && (instr[ifpos+clen] == instr[lind+clen]))
					clen++;
				if (clen > 3 && clen > mlen) {
					mlen = clen;
					mind = lind;
					if (mlen == 255) break;
				}
			}
		}

		// if no match increase nomatch count (and add lookup)
		if (mlen==0)
		{
			if (instr.check(ifpos+3)) {
				hashprev[ifpos & WIND_MASK] = hashhead[lookupind];
				hashhead[lookupind] = ifpos;
			}
			++nmlen;
			++ifpos;
			nexthash(instr[ifpos+3]);
		};
		// if match or nomatch limit exceeded, output nomatch (literal) codes
		if ((mlen!=0 && nmlen!=0) || nmlen == 127)
		{
			outstr.write(nmlen | (1<<7));
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
					hashprev[ifpos & WIND_MASK] = hashhead[lookupind];
					hashhead[lookupind] = ifpos;
				}
				++ifpos;
				--mlen;
				nexthash(instr[ifpos+3]);
			}
		}
	}

	if (nmlen!=0)
	{
		outstr.write(nmlen | (1<<7));
		for (uint i = ifpos-nmlen; i < ifpos; ++i)
			outstr.write(instr[i]);
//		nmlen = 0;
	}

	outstr.flush();

	delete[] hashhead;
	delete[] hashprev;

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}