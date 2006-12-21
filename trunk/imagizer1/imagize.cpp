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
OPT_STRING(ifname       , 'i',           "--0", 1,   "-", "Read from file"                  );
OPT_STRING(ofname       , 'o',           "--1", 1,   "-", "Write to file"                   );
OPT_STRING(ffname       , 'f',           "--2", 1,   "-", "Font file"                       );
OPT_INT   (palthreshold , 'p',  "palthreshold", 1,   750, "Set the pallete change threshold");
OPT_INT   (charthreshold, 'c', "charthreshold", 1,    64, "Set the pallete change threshold");
OPT_BOOL  (getVersion   , 'v',       "version", 0, false, "Display version info"            );
//vector<string> files(0);

OPT_END(CmdLnParser);

int main(int argc, char* argv[])
{
	CmdLnParser.parse(argc - 1, argv + 1);

	if (getVersion) {
		fprintf(stderr, "version: imagizer a.b\n");
	}

//	fprintf(stderr, "ifname: %s\nofname: %s\nffname: %s\ngetVersion: %s\npalthreshold=%i\ncharthreshold=%i\n",ifname.c_str() ,ofname.c_str(), ffname.c_str(), getVersion?"true":"false",palthreshold,charthreshold);

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
  //r_palcalc = new TPalMedianCut();
	//r_palcalc = new TPalMedianCutSort(palthreshold);
	r_palcalc = new TPalMedianCutSmartSort(palthreshold);
	//r_palcalc = new TPalMedianCutRandomSort(palthreshold);

	r_textimage.font = &r_font;
	r_textimage.pal = &r_pal;

  /*3-frame-avg-render:
   * First output a black frame (as 'previous frame')
   *
   *
   */
	while (!feof(ifhandle))
	{
		r_sourceimage.LoadFromRAW(ifhandle, 160, 100);

		r_palcalc->CalcPal(&r_sourceimage, &r_pal);

		r_renderer->DoRender(&r_sourceimage, &r_textimage);

    for(uint32 i=0; i<16; i++) {
      r_textimage.data[i] = i<<12;
    }

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

  fprintf(stderr,"IMAGIZER DONE\n");
	return 0;
}
//---------------------------------------------------------------------------

