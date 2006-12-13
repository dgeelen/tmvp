//---------------------------------------------------------------------------


#pragma hdrstop

#include "URenderSimAnn.h"
#include <math>
#include <algorithm>

using namespace std;

TRenderSimulatedAnnealing::TRenderSimulatedAnnealing()
{
	cimage.SetSize(160,100);

	double dithering_level = 0.09*log((double)160*100) - 0.04*log((double)16) + 0.001;

	double variance = dithering_level*dithering_level;
	double sum = 0.0;
	for(int i=0; i<3; i++) {
		for(int j=0; j<3; j++) {
			for(int k=0; k<3; k++) {
				sum += filter3[i][j][k] =
					exp(-sqrt((double)((i-1)*(i-1) + (j-1)*(j-1)))/(variance));
			}
		}
	}

	sum /= 3;
	for(int i=0; i<3; i++) {
		for(int j=0; j<3; j++) {
			for(int k=0; k<3; k++) {
				filter3[i][j][k] /= sum;
			}
		}
	}

	sum = 0.0;
	for(int i=0; i<5; i++) {
		for(int j=0; j<5; j++) {
			for(int k=0; k<3; k++) {
				sum += filter5[i][j][k] =
					exp(-sqrt((double)((i-2)*(i-2) + (j-2)*(j-2)))/(variance));
			}
		}
	}

	sum /= 3;
	for(int i=0; i<5; i++) {
		for(int j=0; j<5; j++) {
			for(int k=0; k<3; k++) {
				filter5[i][j][k] /= sum;
			}
		}
	}
}

double TRenderSimulatedAnnealing::CalcPixDist(uint32 x, uint32 y)
{
	double c1[3], c2[3];

	for (int k=0; k < 3; ++k)
		c1[k] = c2[k] = 0;

	for (int i=0; i<3; i++) {
		int rx = x + i - 1;
		if ((rx >= 0) && (rx < 160)) {
			for (int j=0; j<3; j++) {
				int ry = y + j - 1;
				if ((ry >= 0) && (ry < 100)) {
					RGBColor scol = csrc->GetPixel(x,y);
					RGBColor dcol = cimage.GetPixel(x,y);
					for (int k=0; k<3; k++) {
						c1[k] = c1[k] + filter3[i][j][k] * scol.a[k];
						c2[k] = c2[k] + filter3[i][j][k] * dcol.a[k];
					}
				}
			}
		}
	}

	return sqrt(MSquare(c1[0] - c2[0]) + MSquare(c1[1] - c2[1]) + MSquare(c1[2] - c2[2]));
}

double TRenderSimulatedAnnealing::CalcRegionDist(uint32 x, uint32 y)
{
	double result = 0;

	for (int i=0; i<4; i++) {
		int rx = (x*2) + i - 1;
		if ((rx >= 0) && (rx < 160)) {
			for (int j=0; j<4; j++) {
				int ry = (y*2) + j - 1;
				if ((ry >= 0) && (ry < 100)) {
					result = result + CalcPixDist(rx, ry);
				}
			}
		}
	}

	return result;
}

void TRenderSimulatedAnnealing::CalcGlobalCurDist()
{
	curdist = 0;
	for (uint x = 0; x < 160; ++x)
		for (uint y = 0; y < 100; ++y)
			curdist += CalcPixDist(x,y);
}

double TRenderSimulatedAnnealing::DoMoveDist(uint32 x, uint32 y, uint8 chr, uint8 fg, uint8 bg)
{
//	double olddist = CalcPixDist(x*2+0,y*2+0)
//									+CalcPixDist(x*2+1,y*2+0)
//									+CalcPixDist(x*2+0,y*2+1)
//									+CalcPixDist(x*2+1,y*2+1);
	double olddist = CalcRegionDist(x,y);

	uint8 q1 = (chr >> 0) & 0x03;
	uint8 q2 = (chr >> 2) & 0x03;
	uint8 q3 = (chr >> 4) & 0x03;
	uint8 q4 = (chr >> 6) & 0x03;

	cimage.SetPixel(x*2+0,y*2+0, lookup[fg][bg][q1]);
	cimage.SetPixel(x*2+1,y*2+0, lookup[fg][bg][q2]);
	cimage.SetPixel(x*2+0,y*2+1, lookup[fg][bg][q3]);
	cimage.SetPixel(x*2+1,y*2+1, lookup[fg][bg][q4]);

	cdst->SetChar(x, y, chr, fg, bg);

//	double newdist = CalcPixDist(x*2+0,y*2+0)
//									+CalcPixDist(x*2+1,y*2+0)
//									+CalcPixDist(x*2+0,y*2+1)
//									+CalcPixDist(x*2+1,y*2+1);
	double newdist = CalcRegionDist(x,y);

	return newdist-olddist;
}

bool P(double temp, double delta)
{
	if (delta < 0)
		return true;
	return false;
}

void TRenderSimulatedAnnealing::DoRender(RawRGBImage* src, TextImage* dst)
{
	csrc = src;
	cdst = dst;

	for( unsigned char bg=0;bg<16;bg++) {
		RGBColor bgcol = dst->pal->GetColor(bg);
		for( unsigned char fg=0;fg<16;fg++) {
			RGBColor fgcol = dst->pal->GetColor(fg);
			for( unsigned char quad=0; quad < 4; quad++) {
				lookup[fg][bg][quad] = RGBAvg2(fgcol, bgcol, quad*16,64);
			}
		}
	}

	for (uint x = 0; x < 80; ++x) {
		for (uint y = 0; y < 50; ++y) {
			uint8 chr = dst->data[x+y*80] & 0xFF;
			uint8 fg = (dst->data[x+y*80] >>  8) & 0x0F;
			uint8 bg = (dst->data[x+y*80] >> 12) & 0x0F;

			uint8 q1 = (chr >> 0) & 0x03;
			uint8 q2 = (chr >> 2) & 0x03;
			uint8 q3 = (chr >> 4) & 0x03;
			uint8 q4 = (chr >> 6) & 0x03;

			cimage.SetPixel(x*2+0,y*2+0, lookup[fg][bg][q1]);
			cimage.SetPixel(x*2+1,y*2+0, lookup[fg][bg][q2]);
			cimage.SetPixel(x*2+0,y*2+1, lookup[fg][bg][q3]);
			cimage.SetPixel(x*2+1,y*2+1, lookup[fg][bg][q4]);
		}
	}

	CalcGlobalCurDist();

	double temp=2;

	uint32 xpos = 0;
	uint32 k = 0;
//	uint32 kmax = 80*50*256*256;
//	uint32 kmax = 1000000;
	uint32 kmax = 10000;
	uint32 a = 0;
	uint32 amax = 100;
	while (k < kmax) {
		xpos = (xpos+1) % (80*50);
		uint32 x = xpos % 80;
		uint32 y = xpos / 80;
//		uint32 x = random(80);
//		uint32 y = random(50);

		uint8 ochr= dst->data[x+y*80] & 0xFF;
		int8 ofg = (dst->data[x+y*80] >>  8) & 0x0F;
		int8 obg = (dst->data[x+y*80] >> 12) & 0x0F;

		uint nchr;
		uint nfg;
		uint nbg;
		uint8 rnd = random(100);
		if (rnd > 50) {
			int8 q[4];
			q[0] = (ochr >> 0) & 0x03;
			q[1] = (ochr >> 2) & 0x03;
			q[2] = (ochr >> 4) & 0x03;
			q[3] = (ochr >> 6) & 0x03;

			q[random(4)] = random(4);

			nchr = (q[0] << 0) | (q[1] << 2) | (q[2] << 4) | (q[3] << 6);
			nfg = ofg;
			nbg = obg;
		} else if (rnd > 30) {
			nchr = ochr;
			nfg = ofg;
			nbg = random(16);
		} else if (rnd > 10) {
			nchr = ochr;
			nfg = random(16);
			nbg = obg;
		} else {
			nchr = ochr;
			nfg = random(16);
			nbg = random(16);
		}


//		uint8 nchr = (nq1 << 0) | (nq2 << 2) | (nq3 << 4) | (nq4 << 6);
//		uint8 nfg = min(max(ofg + 1 - random(3), 0), 3);
//		uint8 nbg = min(max(obg + 1 - random(3), 0), 3);

//		nchr =  random(256);
//		nfg = random(16);
//		nbg = random(16);

		double movedist = DoMoveDist(x, y, nchr, nfg, nbg);

		++k;
		if (P(temp, movedist)) {
			++a;
			if (a == amax) {
				a=0; k=0; temp = temp*0.75;
			}
			curdist += movedist;
		}	else {
			DoMoveDist(x,y,ochr,ofg,obg);
		}
	}
//	cimage.SaveToPNG("c:\\debug.png");

}
//---------------------------------------------------------------------------

#pragma package(smart_init)

