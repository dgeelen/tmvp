//---------------------------------------------------------------------------

#ifndef URenderH
#define URenderH

#include "UImage.h"

class TCalcPallete {
 public:
	virtual void CalcPal(RawRGBImage* src, TextPal* dst) = 0;
};

class TPalStandard : public TCalcPallete {
 public:
	virtual void CalcPal(RawRGBImage* src, TextPal* dst);
};

class TPalMedianCut : public TCalcPallete {
 public:
	void CalcPal(RawRGBImage* src, TextPal* dst);
};

class TPalMedianCutSort : public TPalMedianCut {
	double threshold;
 public:
	TPalMedianCutSort(double t);
	void CalcPal(RawRGBImage* src, TextPal* dst);
};

class TRenderMethod {
 public:
	virtual void DoRender(RawRGBImage* src, TextImage* dst) = 0;
};

class TRenderBruteBlock : public TRenderMethod {
 public:
	void DoRender(RawRGBImage* src, TextImage* dst);
};

#include "URenderSimAnn.h"

//---------------------------------------------------------------------------
#endif
