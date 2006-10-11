//---------------------------------------------------------------------------


#pragma hdrstop

#include "UColor.h"
#include <math>

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

//---------------------------------------------------------------------------

#pragma package(smart_init)
