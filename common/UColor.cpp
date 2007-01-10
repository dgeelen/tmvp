//---------------------------------------------------------------------------


#pragma hdrstop

#include "UColor.h"
#include <stdio.h>
#include "stdlib.h"
#include <math.h>

RGBColor AvgRGBColor::avg() {
  if(avrg.count != count) { //speedup
    avrg.color = RGBColor(a[0]/count, a[1]/count, a[2]/count);
    avrg.count = count;
    }
  return avrg.color;
}

AvgRGBColor AvgRGBColor::operator-=(RGBColor& color) {
  if(count>0) {
    for(uint32 i=0; i<3; ++i) {
      a[i]-=color.a[i];
    }
    --count;
  }
  return *this;
}

AvgRGBColor AvgRGBColor::operator+=(RGBColor& color) {
  for(uint32 i=0; i<3; ++i) {
    a[i]+=color.a[i];
  }
  ++count;
  return *this;
}

AvgRGBColor AvgRGBColor::operator+=(AvgRGBColor& color) {
  for(uint32 i=0; i<3; ++i) {
    a[i]+=color.avg().a[i];
  }
  count+=color.size();
  return *this;
}

uint32 AvgRGBColor::size() {
  return count;
}

AvgRGBColor AvgRGBColor::operator-(RGBColor& color) {
  AvgRGBColor q=*this;
  if(q.count>0){
    q-=color;
    --q.count;
  }
  return q;
}

AvgRGBColor AvgRGBColor::operator+(AvgRGBColor& color) {
  AvgRGBColor q=*this;
  q+=color;
  return q;
}

AvgRGBColor AvgRGBColor::operator+(RGBColor& color) {
  AvgRGBColor q=*this;
  q+=color;
  return q;
}

AvgRGBColor::AvgRGBColor(RGBColor color) {
  for(uint32 i=0; i<3; ++i) {
    a[i]=color.a[i];
  }
  count=1;
  avrg.count=0;
}

RGBColor RGBColor::operator+(RGBColor& color) {
//  fprintf(stderr, "RGBColor::operator+(): this={%i, %i, %i} color={%i, %i, %i}\n", this->a[0],this->a[1],this->a[2], color.a[0], color.a[1], color.a[2]);
  AvgRGBColor q(color);
  q+=(*this);
  return q.avg();
}

RGBColor::RGBColor()
{
	set_i(0);
}

RGBColor::RGBColor(uint8 rval, uint8 gval, uint8 bval)
{
	c.r = rval;
	c.g = gval;
	c.b = bval;
}

RGBColor::RGBColor(uint8 aval[3])
{
	a[0] = aval[0];
	a[1] = aval[1];
	a[2] = aval[2];
}

RGBColor::RGBColor(RGBAColor c)
{
	set_i(c.i);
}

RGBColor::RGBColor(uint32 val)
{
	set_i(val);
}

RGBAColor::RGBAColor()
{
	i = 0;
}

RGBAColor::RGBAColor(RGBColor col)
{
	i = col.get_i();
}

RGBColor RGBAvg2(RGBColor c1, RGBColor c2, uint32 weight, uint32 total)
{
	RGBColor result;
	for (int i = 0; i < 3; ++i)
		result.a[i] = (c1.a[i]*weight + c2.a[i]*(total-weight)) / total;
	return result;
}

uint32 RGBDistInt(RGBColor c1, RGBColor c2)
{
	return  (c1.c.r - c2.c.r)*(c1.c.r - c2.c.r)
				+ (c1.c.g - c2.c.g)*(c1.c.g - c2.c.g)
				+ (c1.c.b - c2.c.b)*(c1.c.b - c2.c.b);
}

double RGBDistDouble(RGBColor c1, RGBColor c2)
{
	double result = 0;
	for (int i = 0; i < 3; ++i)
		result = result + (c1.a[i] - c2.a[i])*(c1.a[i] - c2.a[i]);
	return sqrt(result);
}

uint32 RGBDistLUVInt(RGBColor c1, RGBColor c2)
{
	uint32 r,g,b;
	uint32 rmean;

	rmean = ( (int)c1.c.r + (int)c2.c.r ) / 2;
	r = (int)c1.c.r - (int)c1.c.r;
	g = (int)c1.c.g - (int)c2.c.g;
	b = (int)c1.c.b - (int)c2.c.b;
	return (((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8);
}


//---------------------------------------------------------------------------

#pragma package(smart_init)
