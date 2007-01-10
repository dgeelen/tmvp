//---------------------------------------------------------------------------

#ifndef UColorH
#define UColorH

#include "UTypes.h"

struct RGBAColor;
class AvgRGBColor;

struct RGBColor {
	union {
		uint8 a[3];
		struct { uint8 r,g,b; } c;
	};
	uint32 get_i() { return (c.r << 16) | (c.g << 8) | (c.b << 0); };
	void set_i(uint32 val) { c.r = (val >> 16)&0xFF; c.g = (val >> 8)&0xFF; c.b = (val >> 0)&0xFF;};
//	__property uint32 i = { read = get_i, write = set_i };

	uint32 get_bgr() { return (c.b << 16) | (c.g << 8) | (c.r << 0); };

	RGBColor();
  RGBColor operator+(RGBColor& color);
	RGBColor(uint8 rval, uint8 gval, uint8 bval);
	RGBColor(uint8 aval[3]);
	// cast from RGBAColor
	RGBColor(RGBAColor col);
	// cast from int (uses set_i)
	RGBColor(uint32 val);
};

class AvgRGBColor {
  private:
  union {
    uint32 a[3];
    struct { uint32 r,g,b; } c;
  };
  uint32 count;
  struct avrg {
    RGBColor color;
    uint32 count;
  } avrg;
  public:
  AvgRGBColor(RGBColor color);
  uint32 size();
  RGBColor avg();
  AvgRGBColor operator+(RGBColor& color);
  AvgRGBColor operator+(AvgRGBColor& color);
  AvgRGBColor operator+=(RGBColor& color);
  AvgRGBColor operator+=(AvgRGBColor& color);
  AvgRGBColor operator-(RGBColor& color);
  AvgRGBColor operator-(AvgRGBColor& color);
  AvgRGBColor operator-=(RGBColor& color);
  AvgRGBColor operator-=(AvgRGBColor& color);
};

struct RGBAColor {
	union {
		uint8 a[3];
		uint32 i;
		struct { uint8 r,g,b,a; } c;
	};

	RGBAColor();
	// cast from RGBColor
	RGBAColor(RGBColor c);
};

RGBColor RGBAvg2(RGBColor c1, RGBColor c2, uint32 weight, uint32 total);
double RGBDistDouble(RGBColor c1, RGBColor c2);
uint32 RGBDistInt(RGBColor c1, RGBColor c2);
uint32 RGBDistLUVInt(RGBColor c1, RGBColor c2);

#define MSquare(x) ((x)*(x))
#define MRGBDistInt(c1, c2) (MSquare(c1.c.r-c2.c.r) + MSquare(c1.c.g-c2.c.g) + MSquare(c1.c.b-c2.c.b))
#define MRGBDistDouble(c1, c2) (sqrt(MSquare(c1.c.r-c2.c.r) + MSquare(c1.c.g-c2.c.g) + MSquare(c1.c.b-c2.c.b)))
#define MRGBDistLUVInt(c1, c2) (((1024+c1.c.r+c2.c.r)*MSquare(c1.c.r-c2.c.r))>>9 + \
																4*MSquare(c1.c.g-c2.c.g) + \
																((1534-(c1.c.r+c2.c.r))*MSquare(c1.c.b-c2.c.b))>>9)
//---------------------------------------------------------------------------
#endif
