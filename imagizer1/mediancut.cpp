#include "mediancut.h"
#include "UColor.h"
#include "UImage.h"
#include "UTypes.h"
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

class ccmp {
    int axis;
  public:
    ccmp(int a) { axis = a; };
    bool operator()(const RGBColor &a,const RGBColor &b) {
      return a.a[axis] < b.a[axis];
    }
};

bool operator<(const box &a, const box &b) {
  //fprintf(stderr, "Comparing axis %08u, %08u\n",a.longest_axis_size , b.longest_axis_size);
  return a.longest_axis_size < b.longest_axis_size;
};

box::box() {
  longest_axis_size=0;
  longest_axis=0;
}

box::~box() {
}

void box::setcutmethod(cutmethod method) {
  CutMethod = method;
}

box box::split() {
  box newbox;
  newbox.setcutmethod( CutMethod ); //FIXME: Make CutMethod a property of Class MedianCut instead of Class Box
  vector<RGBColor>::iterator median=start + (end-start)/2;
  ccmp comp(longest_axis);
  if(CutMethod==MEDIAN_CUT) {
    nth_element(start, median, end, comp);
  }
  else if(CutMethod==SMALLESTBOX_CUT) { //instead of nth element look for the 2 elements wich lie furthest apart (along AXIS)
    sort(start, end, comp);
    uint32 furthest=0;
    vector<RGBColor>::iterator m1=end;
    vector<RGBColor>::iterator m2=(--m1)--;
    uint32 x=end-start;
    uint32 split=0;
    uint32 dist=0;
    while(m1!=start) {
      dist=MRGBDistInt((*m1), (*m2));
      if( dist >= furthest) {
        median = m1;
        furthest = dist;
        split=x;
      }
      m2=m1--;
      --x;
    }
  }
  else if(CutMethod==AVERAGE_CUT) {
    sort(start, end, comp);
    uint32 cr=0;
    uint32 cg=0;
    uint32 cb=0;
    for(vector<RGBColor>::iterator i=start; i!=end; ++i) {
      cr+=i->a[0];
      cg+=i->a[1];
      cb+=i->a[2];
    }
    RGBColor avg(cr/(end-start), cg/(end-start), cb/(end-start));
    uint32 best_dist=ULONG_MAX;
    uint32 split=0;
    sort(start, end, comp);
    for(vector<RGBColor>::iterator i=start; i!=end; ++i) {
      uint32 dist=MRGBDistInt((*i), avg);
      if(dist < best_dist) {
        median = i;
        best_dist=dist;
      }
      ++split;
    }
  }
  else if(CutMethod==CENTER_CUT) { //center cut (center of box)
    sort(start, end, comp);
    vector<RGBColor>::iterator q=end;
    --q;
    AvgRGBColor tcol(((*start)+(*q)));
    uint32 best_dist=ULONG_MAX;
    for(vector<RGBColor>::iterator i=start; i!=end; ++i) {
      uint32 dist=MRGBDistInt((*i), tcol.avg());
      if(dist < best_dist) {
        best_dist=dist;
        median=i;
      }
    }
  }
  else {
    fprintf(stderr,"box::split(): Unknow split method!\n");
  }
  newbox.start=start;
  newbox.end=median++;
  start=median;
  return newbox;
}

void box::shrink(){
//  fprintf(stderr, "SHRINK()\n");
  minp = *start;
  maxp = *start;
//  fprintf(stderr, "calculating new boundingbox\n");
  for(vector<RGBColor>::iterator i=start+1; i!=end; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (i->a[j] > maxp.a[j]) maxp.a[j] = i->a[j];
      if (i->a[j] < minp.a[j]) minp.a[j] = i->a[j];
    }
  }
//  fprintf(stderr, "new boundingbox done\n");
  longest_axis_size = 0;
  for(uint32 i=0; i<3; i++) {
    if(maxp.a[i]-minp.a[i] >= longest_axis_size) {
      longest_axis_size = maxp.a[i]-minp.a[i];
      longest_axis = i;
    }
  }
//  fprintf(stderr, "Longest axis=%i, size=%u\n",longest_axis, longest_axis_size);
}

void MedianCut(RawRGBImage* img,  TextPal* pal, cutmethod CutMethod) {
static int debug=0;
  //fprintf(stderr, "Starting MedianCut\n");
  vector<RGBColor> colorspace;
  //fprintf(stderr, "Starting reserving space\n");
  colorspace.reserve(img->GetWidth() * img->GetHeight());
  //fprintf(stderr, "filling colorspace\n");
  for(uint32 i=0; i < img->GetWidth() * img->GetHeight() ; ++i) {
    colorspace.push_back(img->data[i]);
  }
  //fprintf(stderr, "Filled colorspace\n");
  priority_queue<box> boxes;
  box boundingbox;
  boundingbox.setcutmethod( CutMethod );
  boundingbox.start=colorspace.begin();
  boundingbox.end=colorspace.end();
  //fprintf(stderr, "shrink\n");
  boundingbox.shrink();
  //fprintf(stderr, "done\n");
  boxes.push(boundingbox);
  RawRGBImage mypng;
  mypng.SetSize(50, 4*16);
  int py = 0;

    #define tehline(boxy) \
    for (int i = 0; i < 50; ++i) \
      mypng.SetPixel(i,py, RGBAvg2(boxy.minp, boxy.maxp, i, 50)); \
    py++;

  while(boxes.size()<16){
    for (int i = 0; i < 50; ++i) \
      mypng.SetPixel(i,py, RGBColor(255,0,255)); \
    py++;
    //fprintf(stderr, "loop\n");
    box largestbox = boxes.top();
    boxes.pop();
    if(debug==0)fprintf(stderr, "extracted box with minRGB(%03u, %03u,%03u), maxRGB(%03u, %03u,%03u) nelements=%03u\n", largestbox.minp.a[0], largestbox.minp.a[1], largestbox.minp.a[2], largestbox.maxp.a[0], largestbox.maxp.a[1], largestbox.maxp.a[2], largestbox.end - largestbox.start);
    tehline(largestbox);
    //fprintf(stderr, "splitting BOX WITH SIZE=%08u\n", largestbox.longest_axis_size);
    box newbox = largestbox.split();
    //fprintf(stderr, "shirnk a\n");
    largestbox.shrink();
    //fprintf(stderr, "shirnk b\n");
    newbox.shrink();
    //fprintf(stderr, "push_back new\n");
    if(debug==0)fprintf(stderr, "  inserts box with minRGB(%03u, %03u,%03u), maxRGB(%03u, %03u,%03u) nelements=%03u\n", newbox.minp.a[0], newbox.minp.a[1], newbox.minp.a[2], newbox.maxp.a[0], newbox.maxp.a[1], newbox.maxp.a[2],newbox.end - newbox.start);
    boxes.push(newbox);
    tehline(newbox);
    //fprintf(stderr, "push_back new\n");
    if(debug==0)fprintf(stderr, "  inserts box with minRGB(%03u, %03u,%03u), maxRGB(%03u, %03u,%03u) nelements=%03u\n", largestbox.minp.a[0], largestbox.minp.a[1], largestbox.minp.a[2], largestbox.maxp.a[0], largestbox.maxp.a[1], largestbox.maxp.a[2], largestbox.end - largestbox.start);
    boxes.push(largestbox);
    tehline(largestbox);
    //fprintf(stderr, "backtotop\n");
  }

  mypng.SaveToPNG("/tmp/debug.png");
  //fprintf(stderr, "calculating colors\n");
  uint32 x=0;
  while(!boxes.empty()){
    //fprintf(stderr, "gettop\n");
    box b = boxes.top();
    //fprintf(stderr, "getpop\n");
    boxes.pop();
    //fprintf(stderr, "avg %i\n",x);
    uint32 cr=0;
    uint32 cg=0;
    uint32 cb=0;
    for(vector<RGBColor>::iterator i=b.start; i!=b.end; ++i) {
      ////fprintf(stderr,"iterating\n");
      cr+=(i->a[0]);
      cg+=(i->a[1]);
      cb+=(i->a[2]);
    }
    cr/=(b.end-b.start);
    cg/=(b.end-b.start);
    cb/=(b.end-b.start);
    //fprintf(stderr, "setpal\n");
    pal->SetColor(x++, RGBColor(cr,cg,cb));
  }
  fprintf(stderr, "==============================================done cutting\n");
 // debug=1;
}
