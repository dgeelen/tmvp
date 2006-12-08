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

class TPalGray : public TCalcPallete {
 public:
	virtual void CalcPal(RawRGBImage* src, TextPal* dst);
};

class TPalAnsiCygwin : public TCalcPallete {
 public:
	virtual void CalcPal(RawRGBImage* src, TextPal* dst);
};

class TPalAnsiPutty : public TCalcPallete {
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

class TPalMedianCutRandomSort : public TPalMedianCut {
  uint32 threshold;
 public:
  TPalMedianCutRandomSort(uint32 t);
  void CalcPal(RawRGBImage* src, TextPal* dst);
};

class TPalMedianCutSmartSort : public TPalMedianCut {
  uint32 threshold;
 public:
  TPalMedianCutSmartSort(uint32 t);
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

class TRenderSemiBruteBlock : public TRenderMethod {
 public:
  TRenderSemiBruteBlock(uint32 t);
	void DoRender(RawRGBImage* src, TextImage* dst);
 private:
  uint32 threshold;
};

#include "URenderSimAnn.h"

//---------------------------------------------------------------------------
#endif
