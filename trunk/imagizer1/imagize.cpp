//---------------------------------------------------------------------------


#include <string>
#include <stdio.h>
#include <assert.h>

#include "../common/UImage.h"
#include "../common/UColor.h"
#include "URender.h"
#include "../common/UTypes.h"
#include "../common/UCmdLineParser.h"

//---------------------------------------------------------------------------

using namespace std;

#pragma argsused

OPT_START(CmdLnParser);
//OPT_STRING(ofname      , 'o',       "output", 1,   "-", "Write to file"                   );
OPT_INT   (palthreshold, 'p', "palthreshold", 1,    64, "Set the pallete change threshold");
OPT_BOOL  (getVersion  , 'v',      "version", 0, false, "Display version info"            );
OPT_STRARR(files       , 'f',             "", 3,   "-",  "input, output files"            );
//OPT_STRING(ifname      , 'i',        "input", 1,   "-", "Read from file"                  );
//vector<string> files(0);

OPT_END(CmdLnParser);

int main(int argc, char* argv[])
{
	CmdLnParser.parse(--argc, ++argv);
//	fprintf(stderr, "ifname: %s\nofname: %s\ngetVersion: %s\npaltheshold=%i\n",ifname.c_str() ,ofname.c_str() ,getVersion?"true":"false",palthreshold);
	//return 0;

	if (getVersion) {
		fprintf(stderr, "version: imagizer a.b.c");

	}

	string ifname = files[0];
	string ofname = files[1];
	string ffname = files[2];

	uint palthreshold = 750;// 10000;
	uint charthreshold = 64;

	if (argc > 3) palthreshold = atoi(argv[3]);
	if (argc > 4) charthreshold = atoi(argv[4]);
	if (argc > 5) ffname = argv[5];

	FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");

	RawRGBImage r_sourceimage;
	TextFont r_font;
	if (ffname != "-") {
		r_font.lorval = 128+32;
		r_font.hirval = 255;
		r_font.LoadFromRAWFile(ffname);
	} else
		r_font.DisableMap();
	TextImage r_textimage;

	TCalcPallete *r_palcalc = NULL;
	TextPal r_pal;

	TRenderMethod *r_renderer = NULL;

	//r_renderer = new TRenderBruteBlock;
	r_renderer = new TRenderSemiBruteBlock(charthreshold);
	r_palcalc = new TPalMedianCutSort(palthreshold);
	//r_palcalc = new TPalMedianCutSmartSort(palthreshold);
	//r_palcalc = new TPalMedianCutRandomSort(palthreshold);

	r_textimage.font = &r_font;
	r_textimage.pal = &r_pal;

	while (!feof(ifhandle))
	{
		r_sourceimage.LoadFromRAW(ifhandle, 160, 100);

		r_palcalc->CalcPal(&r_sourceimage, &r_pal);

		r_renderer->DoRender(&r_sourceimage, &r_textimage);

		fwrite(r_textimage.data, 1, 8000, ofhandle);
		uint8 oval;
		for (int i=0; i<16; ++i) {
			oval = r_textimage.pal->GetColor(i).c.r >> 2;
			fwrite(&oval, 1, 1, ofhandle);
			oval = r_textimage.pal->GetColor(i).c.g >> 2;
			fwrite(&oval, 1, 1, ofhandle);
			oval = r_textimage.pal->GetColor(i).c.b >> 2;
			fwrite(&oval, 1, 1, ofhandle);
		}
	}

	delete r_renderer;
	delete r_palcalc;

	if (ifname != "-") fclose(ifhandle);
	if (ofname != "-") fclose(ofhandle);
	return 0;
}
//---------------------------------------------------------------------------

