/****************************************************************************/
#ifndef YACQA_H
#define YACQA_H
#include "../common/UTypes.h"
#include "../common/UColor.h"
#include "../common/UImage.h"

struct cinfo {
  uint32 n;
  RGBColor c;
};

void yacqa(RawRGBImage* img,  TextPal* pal);
#endif
