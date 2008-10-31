#include <cstdio>
#include <string>

#include "../common/UImage.h"
#include "../imagizer1/URender.h"

using namespace std;

int main(int argc, char** argv)
{
	string ifname = "-";
	string ofname = "-";
	string fntname = "font.fnt";

	// TODO: maybe read other cmd line params???
	if (argc > 1) ifname = argv[1];
	if (argc > 2) ofname = argv[2];
	if (argc > 3) fntname = argv[3];

	TextFont fnt;
	fnt.lorval = 128+32;
	fnt.hirval = 255;
	fnt.LoadFromRAWFile(fntname);

	RawRGBImage input;
	RawRGBImage output;
	input.LoadFromPNG(ifname);

	TextPal pal;
	TextImage text;
	text.font = &fnt;
	text.pal = &pal;

	TPalMedianCutSort palcalc(100);
	//TPalAnsiCygwin palcalc;
	TRenderBruteBlock renderer;
	//TRenderSemiBruteBlock renderer(64);

	palcalc.CalcPal(&input, &pal);
	renderer.DoRender(&input, &text);

	text.SaveToRawRGBImage(&output);
	output.SaveToPNG(ofname);

	return 0;
}
