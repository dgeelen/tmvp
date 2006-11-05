//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused

#include <string>
#include <stdio.h>
#include <assert.h>
#include "UTypes.h"

using namespace std;

int main(int argc, char* argv[])
{
	string if1name = "-";
	string if2name = "-";
	string ofname = "-";

	// TODO: maybe read other cmd line params???
	if (argc > 1) if1name = argv[1];
	if (argc > 2) if2name = argv[2];
	if (argc > 3) ofname  = argv[3];

	if (if1name == "-" || if2name == "-")
		return 0;

	FILE* if1handle = fopen(if1name.c_str(), "rb");
	FILE* if2handle = fopen(if2name.c_str(), "rb");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");

	uint8 buffer[10000];

	uint32 read1, read2;
	do {
		read1 = fread(buffer, 1, 8000 + 48, if1handle);
		fwrite(buffer, 1, 8000 + 48, ofhandle);
		read2 = fread(buffer, 1, 400, if2handle);
		fwrite(buffer, 1, 400, ofhandle);
	} while (read1 != 0 || read2 != 0);

	fclose(if1handle);
	fclose(if2handle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}
//---------------------------------------------------------------------------
 