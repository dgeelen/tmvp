//---------------------------------------------------------------------------
#include "../common/UTypes.h"
#include <string>
#include <stdio.h>
#include <assert.h>

#pragma hdrstop
//---------------------------------------------------------------------------

using namespace std;

#pragma argsused
int main(int argc, char* argv[])
{
	string ifname = "-";
	string ofname = "-";

	uint factor = 256 / 2;

	// TODO: maybe read other cmd line params???
	if (argc > 1) ifname = argv[1];
	if (argc > 2) ofname = argv[2];
	if (argc > 3) factor = atoi(argv[3]);

	FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");

	uint8 buf[32*1024];

	uint32 read;
	while (!feof(ifhandle))
	{
		read = fread(buf, 1, 32*1024, ifhandle);
		for (uint i = 0; i < read; ++i)
			buf[i] = buf[i] * factor / 256;
		fwrite(buf, 1, read, ofhandle);
	}

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}

