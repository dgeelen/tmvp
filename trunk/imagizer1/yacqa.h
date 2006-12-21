/****************************************************************************/
#ifndef YACQA_H
#define YACQA_H
#include "UTypes.h"
#include "UColor.h"
#include "UImage.h"

struct cinfo {
  uint32 n;
  RGBColor c;
};

void yacqa(RawRGBImage* img,  TextPal* pal);
#endif
