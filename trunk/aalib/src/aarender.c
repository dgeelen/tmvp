#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "aalib.h"
#include "aaint.h"
#include "aamktabl.h"
#include "config.h"
aa_renderparams aa_defrenderparams =
{0, 0, 1.0, AA_FLOYD_S, 0, 0};
#define VAL (13)                /*maximum distance good for fill tables */
#define pos(i1,i2,i3,i4) (((int)(i1)<<12)+((int)(i2)<<8)+((int)(i3)<<4)+(int)(i4))
__AA_CONST char * __AA_CONST aa_dithernames[] =
{
	"no dithering",
	"error-distribution",
	"floyd-steelberg dithering",
	NULL
};

int bmap[256] = {
 32, 96, 39, 33, 32, 45, 94, 33, 34, 34, 34, 57, 34, 34, 34, 57,
 46, 58, 40, 33, 47, 47, 43, 33, 47, 63, 55, 57, 74, 55, 55, 80,
 44, 59,105,108, 47,114,102, 84, 47, 55, 89, 80, 74, 55, 80, 80,
 44,101, 49, 49,112,112,102,102, 74,112, 80, 80, 74, 86, 80, 80,
 32, 92, 92, 92,124,124, 92, 57,124, 34, 42, 57, 74, 42, 42, 57,
 95, 61,116, 83,124,124, 67, 53, 74,100, 42, 57, 74, 74, 42, 77,
 95,115, 49, 76, 97,118,107, 69, 74,100, 86, 64, 74, 74, 64, 64,
 97,112, 49, 76,112,112,112, 69, 74,112, 48, 48, 74, 71, 48,113,
 95, 92, 92, 92,124, 92, 92, 53,106, 52, 52, 78, 74, 52, 52,119,
 95,110, 71, 98,106,113, 71, 53,106, 52, 52, 78, 74, 52,119,119,
 97, 97, 76, 76,106,121, 71,104,106, 66, 87, 66, 74,104, 71,121,
 97,119, 76, 76, 77,119,119, 52, 78, 78, 52, 52, 83, 53, 71,113,
 95, 92, 92, 92,113,113, 48, 71,106, 48, 52,112, 69, 52,112,112,
113,113, 71, 71,106,113, 48, 71,106, 48, 48,112, 69, 69,112,112,
 77,113, 71, 76, 77, 64, 64, 74, 77, 64, 86,100, 69, 69,107,118,
 77,119,119, 76, 77, 77, 42, 74, 78, 57, 42,100, 83, 53, 67,124
};

int imap[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  1,  1,  1,
  0,  0,  0,  0,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  0,  1,  1,  0,  1,  1,
  0,  0,  0,  0,  0,  0,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,
  1,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
};
/*
int r_pal[16] = { // putty pallete
0x000000ul,
0xBB0000ul,
0x00BB00ul,
0xBBBB00ul,
0x0000BBul,
0xBB00BBul,
0x00BBBBul,
0xBBBBBBul,
0x555555ul,
0xFF5555ul,
0x55FF55ul,
0xFFFF55ul,
0x5555FFul,
0xFF55FFul,
0x00FFFFul,
0x55FFFFul
};
//*/
//*
int r_pal[16] = {
0x000000ul,
0x800000ul,
0x008000ul,
0x808000ul,
0x000080ul,
0x800080ul,
0x008080ul,
0xC0C0C0ul,
0x808080ul,
0xFF0000ul,
0x00FF00ul,
0xFFFF00ul,
0x0000FFul,
0xFF00FFul,
0x00FFFFul,
0xFFFFFFul
};
//*/

#define DO_CONTRAST(i,c) (i<c?0:(i>256-c)?255:(i-c)*255/(255-2*c))
aa_renderparams *aa_getrenderparams(void)
{
	aa_renderparams *p = calloc(1, sizeof(*p));
	if (p == NULL)
		return NULL;
	*p = aa_defrenderparams;
	return (p);
}
#define MYLONG_MAX 0xffffffffU     /*this is enought for me. */
#define myrand() (state = ((state * 1103515245) + 12345) & MYLONG_MAX)

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned int uint;

//struct RGBColor {
//	union {
//		uint8 a[3];
//		struct { uint8 r,g,b; } c;
//	};
//};


// assumes LE
struct RGBColor32 {
	union {
		uint8 a[4];
		struct { uint8 b,g,r,a; } c;
		uint32 i;
	};
};

//typedef struct RGBColor RGBColor;
typedef struct RGBColor32 RGBColor32;

//---- start of pallete stuff
//TODO: move to own header?
typedef struct Block {
	int start, end;
	RGBColor32 max,min;
	int ldist;
	int laxis;
} Block;

RGBColor32* palbuf = NULL; // scratchpad for pal calcs
int palbufsize = 0;

Block blocks[16];
int nblocks;

inline void CalcBlock(Block* block)
{
	int i,j;
	i = block->start;
	block->max.i = block->min.i = palbuf[i].i;
	for (++i; i < block->end; ++i) {
		for (j = 0; j < 3; ++j) {
			if (block->max.a[j] < palbuf[i].a[j]) block->max.a[j] = palbuf[i].a[j];
			if (block->min.a[j] > palbuf[i].a[j]) block->min.a[j] = palbuf[i].a[j];
		}
	}
	block->ldist = -1;
	for (j = 0; j < 3; ++j) {
		if ((block->max.a[j]-block->min.a[j]) > block->ldist) {
			block->ldist = block->max.a[j]-block->min.a[j];
			block->laxis = j;
		}
	}
}

#define swap(a, b) { \
	int iswap; \
	iswap = (a); \
	(a) = (b); \
	(b) = iswap; \
	}

inline void SplitBlock(int blocknum)
{
	Block* block = &blocks[blocknum];
	Block* nblock = &blocks[nblocks++];


//	fprintf(stderr, "<splitvalue %i\n", splitvalue);
	int p = block->start;
	int r = block->end;
	int i = (p + r) / 2;

	// target: [p-i) <= [i-r)

	// variant p <= i <= r
	//         [*-p) <= [r-*)
	// end at p==r
	while (p != r)
	{
		int split = p + (rand()%(r-p));
		int splitval = palbuf[split].a[block->laxis];

		int s = p;
		int t = r;

		--t;
		swap(palbuf[split].i, palbuf[t].i);

		while (s < t) {
			swap(palbuf[s].i, palbuf[t-1].i);

			while (s != t && palbuf[s].a[block->laxis] <= splitval)
				++s;
			while (s != t && palbuf[t-1].a[block->laxis] >= splitval)
				--t;
		};

		swap(palbuf[s].i, palbuf[r-1].i);
		// [p - s) <= [s - s+1) <= [s+1 - r)

		if (i == s || i == s+1 ) {
			p = r = i;
		} else if (i < s) {
			// [p-i) - [i-s) <= [s-r)
			r = s;
		} else if (i > s)  {
			// [p-s) <= [s-i) - [i-r)
			p = s + 1;
		};
	}

	nblock->start = p;
	nblock->end = block->end;
	block->end = p;

	CalcBlock(block);
	CalcBlock(nblock);
}

inline void CalcPal(int* pal)
{
	nblocks = 1;
	int i,j,k;

	blocks[0].start = 0;
	blocks[0].end = palbufsize;
	CalcBlock(&blocks[0]);

//	fprintf(stderr, "starting calc with %i points\n",palbufsize);

	while (nblocks < 16) {
		// pick biggest block
		j = 0;
		for (i = 1; i < nblocks; ++i)
			if (blocks[i].ldist > blocks[j].ldist)
				j = i;

//		fprintf(stderr, "going to split %i blocks\n",nblocks);
		// split biggest block....
		SplitBlock(j);

	}

//	fprintf(stderr, "calcing colors of %i blocks\n",nblocks);

	// calc block colors
	for (i = 0; i < nblocks; ++i) {
		int tval[3];
		for (j = 0; j < 3; ++j)
			tval[j] = 0;

//		fprintf(stderr, "calcing colors of %i block %i-%i\n", i, blocks[i].start, blocks[i].end);

		for (k = blocks[i].start; k < blocks[i].end; ++k) {
			for (j = 0; j < 3; ++ j) {
				tval[j] += palbuf[k].a[j];
			}
		}

//		fprintf(stderr, "calcing colors tval: %i-%i-%i\n", tval[0], tval[1], tval[2]);

		if (blocks[i].end != blocks[i].start) {
		 	for (j = 0; j < 3; ++j) {
				((RGBColor32*)pal)[i].a[j] = tval[j] / (blocks[i].end-blocks[i].start);
//				fprintf(stderr, "pass: %x-%x\n", ((RGBColor32*)pal)[i].a[j], tval[j] / (blocks[i].end-blocks[i].start));
			}
		} else {
			((RGBColor32*)pal)[i].i = 0;
		}

//		fprintf(stderr, "pentry: %6x-%6x\n",pal[i], ((RGBColor32*)pal)[i].i);

	}
}

//---- end of pallete stuff

#define MSquare(x) ((x)*(x))
#define MRGBDistInt(c1, c2) (MSquare(c1.c.r-c2.c.r) + MSquare(c1.c.g-c2.c.g) + MSquare(c1.c.b-c2.c.b))

#define palcol(p, i) ((p).c.r = (((i) >> 16) &0xFF), (p).c.g = (((i) >> 8) &0xFF), (p).c.b = (((i) >> 0) &0xFF))

void aa_renderpalette(aa_context * c, __AA_CONST aa_palette palette, __AA_CONST aa_renderparams * p, int x1, int y1, int x2, int y2)
{
	uint8 bg, fg, quad, region;
	uint x,y;
	static int state = 1;

	//int rx = myrand();

	static RGBColor32 lookup[16][16][4];
	if (--state == 0){
		state = 64;

		// do pallette here....
		if (palbufsize != aa_imgheight(c) * aa_imgwidth(c))
		{
			if (palbuf != NULL) free(palbuf);
			palbuf = malloc(4 * aa_imgheight(c) * aa_imgwidth(c));
			palbufsize = aa_imgheight(c) * aa_imgwidth(c);
		}

		for(y = 0 ; y < aa_imgheight(c); ++y) {
			for(x = 0 ; x < aa_imgwidth(c); ++x) { // for every quad region
				palbuf[x + y*aa_imgwidth(c)].i = palette[c->imagebuffer[(x  )+(y  )*aa_imgwidth(c)]];
			}
		}
		CalcPal(r_pal);

		for(bg=0;bg<16;bg++) {
			RGBColor32 bgcol;
			bgcol.i = r_pal[bg];
			for(fg=0;fg<16;fg++) {
				RGBColor32 fgcol;
				fgcol.i = r_pal[fg];
				for(quad=0; quad < 4; quad++) {
					// this is faster, TODO: macro-ize this in a nice way
					lookup[fg][bg][quad].c.r = (fgcol.c.r*quad*16 + bgcol.c.r*(64-(quad*16))) / 64;
					lookup[fg][bg][quad].c.g = (fgcol.c.g*quad*16 + bgcol.c.g*(64-(quad*16))) / 64;
					lookup[fg][bg][quad].c.b = (fgcol.c.b*quad*16 + bgcol.c.b*(64-(quad*16))) / 64;
				}
			}
		}
	}

	for(y = 0 ; y < aa_imgheight(c); y+=2) {
		for(x = 0 ; x < aa_imgwidth(c); x+=2) { // for every quad region
			RGBColor32 tcol[4];
			tcol[0].i = palette[c->imagebuffer[(x  )+(y  )*aa_imgwidth(c)]];
			tcol[1].i = palette[c->imagebuffer[(x+1)+(y  )*aa_imgwidth(c)]];
			tcol[2].i = palette[c->imagebuffer[(x  )+(y+1)*aa_imgwidth(c)]];
			tcol[3].i = palette[c->imagebuffer[(x+1)+(y+1)*aa_imgwidth(c)]];

			uint8 pcol[4];

			uint8 chosen[16];

			int i,j;

			for (j = 0; j < 16; ++j)
				chosen[j] = 0;

			for (i = 0; i < 4; ++i)
			{
				uint32 mindist = 0xFFFFFFFF;
				for (j = 0; j < 16; ++j)
				if (!chosen[j])
				{
					RGBColor32 cmpcol;
					cmpcol.i = r_pal[j];
					uint32 dist = MRGBDistInt(cmpcol, tcol[i]);
					if (dist < mindist) {
						mindist = dist;
						pcol[i] = j;
					}
				}
				chosen[pcol[i]] = 1;
			}

			unsigned char best_bg;
			unsigned char best_fg;
			unsigned char best_char;

			unsigned long int char_dist;
			unsigned long int best_char_dist=0xFFFFFF;
			unsigned char best_quad[4];
			unsigned long int best_quad_dist;
			unsigned long int quad_dist;

//			best_fg = pcol[0];
//			best_bg = pcol[0];
//			best_char = 0;

			for(bg=0;bg<4;bg++) {
//				RGBColor bgcol = dst->pal->GetColor(pcol[bg]);
				for(fg=0;fg<4;fg++) // and for all fore- and background colors
				if (bg != fg) { // fg == bg case is redundant cause char 0 is only fg
//					RGBColor fgcol = dst->pal->GetColor(pcol[fg]);
					char_dist = 0;
//					char_dist = MRGBDistInt(fgcol, bgcol) >> 6;
					RGBColor32* curlookup = &lookup[pcol[fg]][pcol[bg]][0];
					for(region=0; region < 4 ; region ++) { // try all regions
						RGBColor32 ttcol;
						ttcol.i = tcol[region].i;
						best_quad_dist=0xFFFFFF;
						for(quad=0; quad < 4; quad++) { // with all 4 quad blocks

							quad_dist = MRGBDistInt(ttcol, curlookup[quad]);
							if(quad_dist<best_quad_dist) {
								best_quad_dist=quad_dist;
								best_quad[region]=quad;
							}
						}
						char_dist += best_quad_dist;
//						char_dist += sqrtl(best_quad_dist);
					}

					if(char_dist<best_char_dist) {
					// select the char representing our chosen quads
						best_char = (best_quad[0] << 0)    // 4^0
											| (best_quad[1] << 2)    // 4^1
											| (best_quad[2] << 4)    // 4^2
											| (best_quad[3] << 6);   // 4^3
						best_char_dist=char_dist;
						best_bg=pcol[bg];
						best_fg=pcol[fg];
					}
				}
			}
			if (best_char == 0) best_fg = best_bg;
			if (best_bg == best_fg) best_char = (best_bg << 4) | best_bg;
			//dst->SetChar(x>>1, y>>1, best_char, best_fg, best_bg);

			int pos = (x>>1)+(y>>1)*aa_scrwidth(c);
			if (imap[best_char])
				c->attrbuffer[pos] = (best_fg << 4) | best_bg;
			else
				c->attrbuffer[pos] = (best_bg << 4) | best_fg;
			c->textbuffer[pos] = bmap[best_char];
		}
	}

	//fprintf(stderr, "render: (%i,%i) -> (%i,%i)\n", aa_imgwidth(c), aa_imgheight(c), aa_scrwidth(c), aa_scrheight(c));

	return;
}

/*
void aa_renderpalette(aa_context * c, __AA_CONST aa_palette palette, __AA_CONST aa_renderparams * p, int x1, int y1, int x2, int y2)
{
	static int state;
	int x, y;
	int val;
	int wi = c->imgwidth;
	int pos;
	int i;
	int pos1;
	int i1, i2, i3, i4, esum;
	int *errors[2];
	int cur = 0;
	int mval;
	int gamma = p->gamma != 1.0;

	int randomval = p->randomval;
	int dither = p->dither;
	aa_palette table;
	if (x2 < 0 || y2 < 0 || x1 > aa_scrwidth(c) || y1 > aa_scrheight(c))
		return;
	if (x2 >= aa_scrwidth(c))
		x2 = aa_scrwidth(c);
	if (y2 >= aa_scrheight(c))
		y2 = aa_scrheight(c);
	if (x1 < 0)
		x1 = 0;
	if (y1 < 0)
		y1 = 0;
	if (c->table == NULL)
		aa_mktable(c);
	if (dither == AA_FLOYD_S) {
		errors[0] = calloc(1, (x2 + 5) * sizeof(int));
		if (errors[0] == NULL)
			dither = AA_ERRORDISTRIB;
		errors[0] += 3;
		errors[1] = calloc(1, (x2 + 5) * sizeof(int));
		if (errors[1] == NULL)
			free(errors[0]), dither = AA_ERRORDISTRIB;
		errors[1] += 3;
		cur = 0;
	}
	for (i = 0; i < 256; i++) {
		y = palette[i] + p->bright;
		if (y > 255)
			y = 255;
		if (y < 0)
			y = 0;
		if (p->contrast)
			y = DO_CONTRAST(y, p->contrast);
		if (gamma) {
			y = pow(y / 255.0, p->gamma) * 255 + 0.5;
		}
		if (p->inversion)
			y = 255 - y;
		if (y > 255)
			y = 255;
		else if (y < 0)
			y = 0;
		table[i] = y;
	}
	gamma = 0;
	if (randomval)
		gamma = randomval / 2;
	mval = (c->parameters[c->filltable[255]].p[4]);
	for (y = y1; y < y2; y++) {
		pos = 2 * y * wi;
		pos1 = y * aa_scrwidth(c);
		esum = 0;
		for (x = x1; x < x2; x++) {
			i1 = table[((((int) c->imagebuffer[pos])))];
			i2 = table[((((int) c->imagebuffer[pos + 1])))];
			i3 = table[((((int) c->imagebuffer[pos + wi])))];
			i4 = table[((((int) c->imagebuffer[pos + 1 + wi])))];
			if (gamma) {
				i = myrand();
				i1 += (i) % randomval - gamma;
				i2 += (i >> 8) % randomval - gamma;
				i3 += (i >> 16) % randomval - gamma;
				i4 += (i >> 24) % randomval - gamma;
				if ((i1 | i2 | i3 | i4) & (~255)) {
					if (i1 < 0)
						i1 = 0;
					else if (i1 > 255)
						i1 = 255;
					if (i2 < 0)
						i2 = 0;
					else if (i2 > 255)
						i2 = 255;
					if (i3 < 0)
						i3 = 0;
					else if (i3 > 255)
						i3 = 255;
					if (i4 < 0)
						i4 = 0;
					else if (i4 > 255)
						i4 = 255;
				}
			}
			switch (dither) {
			case AA_ERRORDISTRIB:
				esum = (esum + 2) >> 2;
				i1 += esum;
				i2 += esum;
				i3 += esum;
				i4 += esum;
				break;
			case AA_FLOYD_S:
				if (i1 | i2 | i3 | i4) {
					errors[cur][x - 2] += esum >> 4;
					errors[cur][x - 1] += (5 * esum) >> 4;
					errors[cur][x] = (3 * esum) >> 4;
					esum = (7 * esum) >> 4;
					esum += errors[cur ^ 1][x];
					i1 += (esum + 1) >> 2;
					i2 += (esum) >> 2;
					i3 += (esum + 3) >> 2;
					i4 += (esum + 2) >> 2;
				}
				break;
			}
			if (dither) {
				esum = i1 + i2 + i3 + i4;
				val = (esum) >> 2;
				if ((abs(i1 - val) < VAL &&
					 abs(i2 - val) < VAL &&
					 abs(i3 - val) < VAL &&
					 abs(i4 - val) < VAL)) {
					if (esum >= 4 * 256)
						val = 255, esum = 4 * 256 - 1;
					if (val < 0)
						val = 0;
					val = c->filltable[val];
				} else {
					if ((i1 | i2 | i3 | i4) & (~255)) {
						if (i1 < 0)
							i1 = 0;
						else if (i1 > 255)
							i1 = 255;
						if (i2 < 0)
							i2 = 0;
						else if (i2 > 255)
							i2 = 255;
						if (i3 < 0)
							i3 = 0;
						else if (i3 > 255)
							i3 = 255;
						if (i4 < 0)
							i4 = 0;
						else if (i4 > 255)
							i4 = 255;
					}
					esum = i1 + i2 + i3 + i4;
					i1 >>= 4;
					i2 >>= 4;
					i3 >>= 4;
					i4 >>= 4;
					val = c->table[pos(i2, i1, i4, i3)];
				}
				esum = (esum - (c->parameters[val].p[4]) * 1020 / mval);
			} else {
				val = (i1 + i2 + i3 + i4) >> 2;
				if ((abs(i1 - val) < VAL &&
					 abs(i2 - val) < VAL &&
					 abs(i3 - val) < VAL &&
					 abs(i4 - val) < VAL)) {
					val = c->filltable[val];
				} else {
					i1 >>= 4;
					i2 >>= 4;
					i3 >>= 4;
					i4 >>= 4;
					val = c->table[pos(i2, i1, i4, i3)];
				}
			}
			c->attrbuffer[pos1] = val >> 8;
			c->textbuffer[pos1] = val & 0xff;
			pos += 2;
			pos1++;
		}
		if (dither == AA_FLOYD_S) {
			if (x2 - 1 > x1)
				errors[cur][x2 - 2] += (esum) >> 4;
			if (x2 > x1)
				errors[cur][x2 - 1] += (5 * esum) >> 4;
			cur ^= 1;
			errors[cur][x1] = 0;
			errors[cur ^ 1][-1] = 0;
		}
	}
	if (dither == AA_FLOYD_S) {
		free(errors[0] - 3);
		free(errors[1] - 3);
	}
}
*/
void aa_render(aa_context * c, __AA_CONST aa_renderparams * p, int x1, int y1, int x2, int y2)
{
	int i;
	static aa_palette table;
	if (table[255] != 255)
		for (i = 0; i < 256; i++) {
			table[i] = i;
		}
	aa_renderpalette(c, table, p, x1, y1, x2, y2);
}
