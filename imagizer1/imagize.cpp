//---------------------------------------------------------------------------


#include "UImage.h"
#include <string>
#include <stdio.h>
#include <assert.h>

#include "UColor.h"
#include "URender.h"


#pragma hdrstop
//---------------------------------------------------------------------------

using namespace std;

#pragma argsused
int main(int argc, char* argv[])
{
	string ifname = "-";
	string ofname = "-";

	uint palthreshold = 250;

	if (argc > 1) ifname = argv[1];
	if (argc > 2) ofname = argv[2];
	if (argc > 3) palthreshold = atoi(argv[3]);

	FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
	FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");

	RawRGBImage r_sourceimage;
	TextFont r_font;
	TextImage r_textimage;

	TCalcPallete *r_palcalc = NULL;
	TextPal r_pal;

	TRenderMethod *r_renderer = NULL;

	r_renderer = new TRenderBruteBlock;
	//r_renderer = new TRenderSemiBruteBlock
	r_palcalc = new TPalMedianCutSort(palthreshold);

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

