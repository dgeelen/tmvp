
#include "yacqa.h"
#include <vector>
using namespace std;


void insert(vector<cinfo> &v, cinfo c, bool ForceMerge) { //PRE: v can hold at least one more item
  //fprintf(stderr,"PRE insert: %i\n", v.size());
  uint32 bd=ULONG_MAX;
  vector<cinfo>::iterator insertionpoint;
  for(vector<cinfo>::iterator i=v.begin(); i!=v.end(); ++i) {
    uint32 d=MRGBDistInt((i->c), c.c);
    if(d<bd) {
      bd=d;
      insertionpoint=i;
    }
  }
  if((bd > 32) && (!ForceMerge)) { //bad match, insert at back as new
    c.n=1;
    v.push_back(c);
  }
  else { // good match
    uint32 r = (insertionpoint->c.a[0])*(insertionpoint->n);
    uint32 g = (insertionpoint->c.a[1])*(insertionpoint->n);
    uint32 b = (insertionpoint->c.a[2])*(insertionpoint->n);
    r+=c.c.a[0];
    g+=c.c.a[1];
    b+=c.c.a[2];
    insertionpoint->n++;
    r/=insertionpoint->n;
    g/=insertionpoint->n;
    b/=insertionpoint->n;
    insertionpoint->c.a[0]=r;
    insertionpoint->c.a[1]=g;
    insertionpoint->c.a[2]=b;
  }
  //fprintf(stderr,"POST insert: %i\n", v.size());
}

void reduce(vector<cinfo> &v) { //finds 2 most matching colors and merges them
//PRE: |vector| > 1
  vector<cinfo>::iterator mi;
  vector<cinfo>::iterator mj;
  uint32 sd=ULONG_MAX;
  for(vector<cinfo>::iterator i=v.begin(); i!=v.end(); ++i) {
    for(vector<cinfo>::iterator j=v.begin(); j!=v.end(); ++j) {
      uint32 d=MRGBDistInt((i->c), (j->c));
      if(d<sd) {
        sd=d;
        mi=i;
        mj=j;
      }
    }
  }
 //mi, mj are best matching pair
 uint32 n=(mi->n);
 uint32 r=(mi->c.a[0])*n;
 uint32 g=(mi->c.a[1])*n;
 uint32 b=(mi->c.a[2])*n;
 uint32 m=(mj->n);
 r+=(mj->c.a[0])*m;
 g+=(mj->c.a[1])*m;
 b+=(mj->c.a[2])*m;
 r/=m+n;
 g/=m+n;
 b/=m+n;
 mj->c.a[0]=r;
 mj->c.a[1]=g;
 mj->c.a[2]=b;
 mj->n=m+n;
 v.erase(mi);
}

void seed(vector<cinfo>& colorspace, RawRGBImage* img){
  for(uint32 y=0; y<img->GetHeight(); y+=img->GetHeight()/8) {
    for(uint32 x=0; x<img->GetWidth(); x+=img->GetWidth()/8) {
      AvgRGBColor q(RGBColor(0ul));
      RGBColor black=RGBColor(0ul);
      for(uint32 v=0; v<img->GetHeight()/8; ++v) {
        for(uint32 u=0; u<img->GetWidth()/8; ++u) {
          q+=img->data[(x+u)+(y+v)*img->GetWidth()];
        }
      }
      q-=black;
      cinfo c;
      c.c=q.avg();
      insert(colorspace, c, false);
    }
  }
/* Insert DOS or extra colors
  cinfo c;
  c.n=1;
  for(uint32 i=0; i<16; i++) {
    c.c=RGBColor();
  }
  //*/
}

void yacqa(RawRGBImage* img,  TextPal* pal) {
  #define precolors 256+16
  //16*16*16 /*4096*/
  vector<cinfo> colorspace;
  colorspace.reserve(precolors);
  seed(colorspace, img);
  uint32 numpixels = img->GetHeight() * img->GetWidth();
  uint32 i=0;
  vector<cinfo>::iterator p=colorspace.begin();
//  fprintf(stderr,"Inserting %u pixels\n", numpixels);
  while(i<numpixels) {
    cinfo c;
    c.c=img->data[i];
    if(colorspace.size() < precolors) {
      insert(colorspace, c, false);
    }
    else {
      insert(colorspace, c, true);
    }
  ++i;
  }
//  fprintf(stderr, "reducing %u colors\n", colorspace.size());
  while(colorspace.size()>16) {
    reduce(colorspace);
  }
  uint32 x=0;
  fprintf(stderr,"===================== There are %i colors\n", colorspace.size());

//  RawRGBImage mypng;
//  mypng.SetSize(50, 4*16);
//  int py = 0;
//  #define tehline(xxx) \
//  for (int iiiii = 0; iiiii < 50; ++iiiii) \
    //mypng.SetPixel(iiiii,py, RGBAvg2((xxx->c),(xxx->c), iiiii, 50)); \
  //py++;

  for(vector<cinfo>::iterator i=colorspace.begin(); i!=colorspace.end(); ++i) {
  fprintf(stderr,"Color %u = (%i, %i, %i)\n", x, i->c.a[0], i->c.a[1], i->c.a[2]);
  //tehline(i);
  pal->SetColor(x++, i->c);
  }
//  mypng.SaveToPNG("/tmp/debug.png");
}
