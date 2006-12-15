//---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

#include <string>
#include <stdio.h>
#include <sys/stat.h>
//#include <conio.h>
//#include <assert.h>
#include "../common/UTypes.h"

using namespace std;

const int BUFSIZE = 32*1024;

string size_suffix[] =
{
	"",
	"K",
	"M",
	"G",
	"T",
	"?"
};

string size_string(float size)
{
	int suf;
	for (suf = 0; size > 999; ++suf)
		size /= 1024;

	if (suf > 5) suf = 5;

	char buf[11];

	int decs = 2;

	if (size > 99) --decs;
	if (size > 9)  --decs;

	if (suf == 0) decs = 0;

	snprintf(buf, 10, "%.*f", decs, size);

	string res(buf);
	res += size_suffix[suf];
	return res;
};

#pragma argsused
int main(int argc, char* argv[])
{
	string ifname = "-";
	string ofname = "-";
	int32 fsize = -1;

	if (argc > 1) ifname = argv[1];
	if (argc > 2) ofname = argv[2];
	if (argc > 3) fsize = atoi(argv[3]);

	FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");

	if (fsize == -1) {
		struct stat statbuf;
		fstat(fileno(ifhandle), &statbuf);
		fsize = statbuf.st_size;
	}

	int32 done = 0;
	int lsize = 0,nsize;
	uint32 read;
	uint8 buf[BUFSIZE];

	string out;
	//fprintf(stderr, "\n");

	do {
		read = fread (buf, 1, BUFSIZE,  ifhandle);
		fwrite(buf,  1, read, ofhandle);
		done += read;

		out.clear();

		out.push_back('[');

		int i = 0;
		for (; i < 50 && fsize/50*i < done; ++i)
			out.push_back('#');

		out.push_back('|');

		for (;i < 50; ++i)
			out.push_back('.');

		out.push_back(']');

		char sbuf[11];
		snprintf(sbuf, 10, "%i", (int)(100*((float)done/fsize)));

		out.push_back(' ');
		out += sbuf;
		out += "%%";
//		out.push_back('%');
//		out.push_back('%');

		out.push_back(' ');
		out += (size_string(done) + "/" + size_string(fsize) );



//		out.push_back('\t');
		nsize = out.size();
		while (out.size() < lsize)
			out.push_back(' ');

		fprintf(stderr, "\r");

		lsize = nsize;


		fprintf(stderr, out.c_str());

	} while (read != 0);

//	getch();

	fprintf(stderr, "\n");




	return 0;
}
//---------------------------------------------------------------------------

