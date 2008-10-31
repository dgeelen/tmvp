//---------------------------------------------------------------------------

#ifndef UImageH
#define UImageH

#ifdef WITH_PNG
//#include <vcl.h>	// this is needed for BCB here.... TODO: find correct define
#endif
#include <string>
#include <stdio.h>
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
	void LoadFromPNG(std::string filename, uint32 awidth = 0, uint32 aheight = 0);
	void LoadFromRAW(std::string filename, uint32 awidth, uint32 aheight, uint32 framenr = 0);
	void LoadFromRAW(FILE* filehandle, uint32 awidth, uint32 aheight);
	void LoadFromBMP(std::string filename);

	void SaveToPNG(std::string filename);
};

class TextFont {
	uint8 ratio[256][5];
	void CalcRatios();
 public:
	uint8 lorval;
	uint8 hirval;
	bool data[256][8][8];

	uint8 bmap[256]; // TODO: create get* func
	bool imap[256];

	void LoadFromRGBImage(RawRGBImage* img);
	void LoadFromRAWFile(std::string filename);

  void SaveToRGBImage(RawRGBImage* img);
	void SaveToRAWFile(std::string filename);

	void DisableMap();

	uint8 GetRatio(uint8 chr, uint8 quad);
};

class TextPal {
	RGBColor data[16];
 public:
	void SetColor(uint8 ind, RGBColor col);
	RGBColor GetColor(uint8 ind) { return data[ind]; };
  uint32 FindColorIndex(RGBColor color);
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
