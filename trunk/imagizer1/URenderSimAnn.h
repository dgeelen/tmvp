//---------------------------------------------------------------------------

#ifndef URenderSimAnnH
#define URenderSimAnnH

#include "URender.h"

class TRenderSimulatedAnnealing : public TRenderMethod {
	RawRGBImage cimage;
	RawRGBImage* csrc;
	TextImage* cdst;
	double curdist;
	RGBColor lookup[16][16][4];

	double filter3[3][3][3];
	double filter5[5][5][3];

	double CalcPixDist(uint32 x, uint32 y);
	double CalcRegionDist(uint32 x, uint32 y);
	void CalcGlobalCurDist();

	double DoMoveDist(uint32 x, uint32 y, uint8 chr, uint8 fg, uint8 bg);
 public:
	//constructor
	TRenderSimulatedAnnealing();

	void DoRender(RawRGBImage* src, TextImage* dst);
};

//---------------------------------------------------------------------------
#endif
