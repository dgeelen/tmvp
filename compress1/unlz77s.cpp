//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused

#include "mystream.h"
#include <string>
#include <stdio.h>
#include <assert.h>
/* unLZ77 (Simple) */

/*
 * 1xxx xxxx											-> copy x literals from encoded stream
 * 0xxx xxxx xxxx xxxx yyyy yyyy	-> copy y bytes from decoded stream starting at offset x
 */

using namespace std;

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

	uint32 ifpos = 0;
	uint32 ofpos = 0;
	uint8 icode;
	while (instr.check(ifpos))
	{
		icode = instr[ifpos++];

		if (icode & (1<<7)) {
			// 1xxx xxxx											-> copy x literals from encoded stream
			uint8 nmlen = icode & ~(1<<7);
			if (!instr.check(ifpos+nmlen-1)) // has side-effects
				assert(false);
			ofpos += nmlen;
			for (;nmlen > 0; --nmlen)
				outstr.write(instr[ifpos++]);
		} else {
			// 0xxx xxxx xxxx xxxx yyyy yyyy	-> copy y bytes from decoded stream starting at offset x
			if (!instr.check(ifpos+2-1)) // has side-effects
				assert(false);
			uint32 offset = (icode << 8) | instr[ifpos++];
			uint8 mlen = instr[ifpos++];
			for (uint i = ofpos-offset; i < ofpos-offset+mlen; ++i)
				outstr.write(outstr[i]);
			ofpos += mlen;
		}
	}

	outstr.flush();

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}

