#ifndef MEDIANCUT_H
#define MEDIANCUT_H

#include "UTypes.h"
#include "UColor.h"
#include "UColor.h"
#include "UImage.h"
#include <vector>
using namespace std;

class box {
  private:
  public:
    box();
    ~box();
    vector<RGBColor>::iterator start;
    vector<RGBColor>::iterator end;
    RGBColor minp, maxp;
    uint32 longest_axis_size;
    uint32 longest_axis;
    void shrink();
    int size();
    box split();
};

void MedianCut(RawRGBImage* img,  TextPal* pal);
#endif
