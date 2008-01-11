#include "mystream.h"
#include <string>
#include <stdio.h>
/* LZ77 (Simple) */

/*
 * 1xxx xxxx                        -> copy x literals from encoded stream
 * 0xxx xxxx xxxx xxxx yyyy yyyy    -> copy y bytes from decoded stream starting at offset x
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

// calculates next hash based on the old one + the new octet
// the old hash in lookupind will be replaced by the new one
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

	MyInStream instr(ifhandle);              // input stream with buffering and limited random access
	MyOutStream outstr(ofhandle);            // output stream with buffering

	uint32 ifpos = 0;                        // encoding position in input stream
	uint32 lookupind = 0;                    // hash of the four octets starting at he encoding position

	int32* hashhead = new int32[HASH_SIZE];  // lookup table from hash to most recent occurrence of the four octets
	int32* hashprev = new int32[WIND_SIZE];  // lookup table from occurrence of four octets to previous occurrence of them

	uint32 mlen;                             // length of the match that was found (0 if none)
	uint32 mind;                             // index of the match that was found

	uint32 nmlen = 0;                        // amount of octets for which no match has been found, but are not yet written out

	// initialize hash lookup table
	for (int i = 0; i < HASH_SIZE; ++i)
		hashhead[i] = -(int32)WIND_SIZE;

	// calculate hash from first four octets
	nexthash(instr[0]);
	nexthash(instr[1]);
	nexthash(instr[2]);
	nexthash(instr[3]);

	while (instr.check(ifpos)) // while there are octets to encode
	{
		mlen = 0;
		// check for a match (of at least length 4)
		if (instr.check(ifpos+3)) { // if there are at least 3 more octets available
			instr.check(ifpos+255); // make sure octets needed for match finding are available

			// calculate maximum number of octets to check against
			uint32 bound = instr.getmax() - ifpos;
			if (bound > 255) bound = 255;

			// do magic with hashhead/hashprev to find longest match
			for (int32 lind = hashhead[lookupind]; ifpos < lind + WIND_SIZE; lind = hashprev[lind & WIND_MASK]) {
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

		// if no match increase nomatch count
		if (mlen==0) {
			if (instr.check(ifpos+3)) {
				// advance lookup hash
				hashprev[ifpos & WIND_MASK] = hashhead[lookupind];
				hashhead[lookupind] = ifpos;
			}
			++nmlen;
			++ifpos;
			nexthash(instr[ifpos+3]);
		};

		// if match found or nomatch limit exceeded, output nomatch (literal) codes
		if ((mlen!=0 && nmlen!=0) || nmlen == 127) {
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
					// advance lookup hash
					hashprev[ifpos & WIND_MASK] = hashhead[lookupind];
					hashhead[lookupind] = ifpos;
				}
				++ifpos;
				--mlen;
				nexthash(instr[ifpos+3]);
			}
		}
	}

	// finish up, write out last literals if there are any left
	if (nmlen!=0) {
		outstr.write(nmlen | (1<<7));
		for (uint i = ifpos-nmlen; i < ifpos; ++i)
			outstr.write(instr[i]);
	}

	outstr.flush();

	delete[] hashhead;
	delete[] hashprev;

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}