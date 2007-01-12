#ifndef MEDIANCUT_H
#define MEDIANCUT_H

#include "../common/UTypes.h"
#include "../common/UColor.h"
#include "../common/UColor.h"
#include "../common/UImage.h"
#include <vector>
using namespace std;

enum cutmethod {
  MEDIAN_CUT,
  AVERAGE_CUT,
  SMALLESTBOX_CUT,
  CENTER_CUT
};

class box {
  private:
    cutmethod CutMethod;
  public:
    box();
    ~box();
    vector<RGBColor>::iterator start;
    vector<RGBColor>::iterator end;
    RGBColor minp, maxp;
    uint32 longest_axis_size;
    uint32 longest_axis;
    void shrink();
    uint32 size() const;
    box split();
    void setcutmethod(cutmethod method);
    bool operator<(const box &b) const;
};

void MedianCut(RawRGBImage* img,  TextPal* pal, cutmethod CutMethod);
#endif
