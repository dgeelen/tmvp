//---------------------------------------------------------------------------

#ifndef UImageH
#define UImageH

#include <vcl.h>
#include <string>
#include "UColor.h"

class RawRGBImage {
 public:
	RGBColor* data;
	uint32 width, height;

	// constructor
	RawRGBImage();
	RawRGBImage(uint32 awidth, uint32 aheight);
	// destructor
	~RawRGBImage();

	void SetSize(uint32 awidth, uint32 aheight);

	RGBColor GetPixel(uint32 x, uint32 y);
	void SetPixel(uint32 x, uint32 y, RGBColor val);
	uint32 GetWidth();
	uint32 GetHeight();

	void LoadFromFile(std::string filename);
	void LoadFromPNG(std::string filename, uint32 awidth = -1, uint32 aheight = -1);
	void LoadFromRAW(std::string filename, uint32 awidth, uint32 aheight, uint32 framenr = 0);
	void LoadFromBMP(std::string filename);

	void SaveToPNG(std::string filename);
};

class TextFont {
	uint8 ratio[256][5];
	void CalcRatios();
 public:
	bool data[256][8][8];


	void LoadFromRGBImage(RawRGBImage* img);
	void LoadFromRAWFile(std::string filename);

	void SaveToRAWFile(std::string filename);

	uint8 GetRatio(uint8 chr, uint8 quad);
};

class TextPal {
	RGBColor data[16];
 public:
	void SetColor(uint8 ind, RGBColor col);
	RGBColor GetColor(uint8 ind);
};

class TextImage {

 public:
	uint16 data[4000];

	TextFont* font;
	TextPal* pal;

	void SetChar(uint16 x, uint16 y, uint8 chr, uint8 fg, uint8 bg);

	void SaveToRawRGBImage(RawRGBImage* img);
};

//---------------------------------------------------------------------------
#endif
