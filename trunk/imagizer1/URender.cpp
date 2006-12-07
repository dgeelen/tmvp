//---------------------------------------------------------------------------


#pragma hdrstop

#include "URender.h"
#include "math.h"
#include "median_cut.h"
#include <vector>

//---------------------------------------------------------------------------


void TPalStandard::CalcPal(RawRGBImage* src, TextPal* dst)
{
	dst->SetColor(  0, 0x000000ul);  // black
	dst->SetColor(  1, 0x0000AAul);  // dark blue
	dst->SetColor(  2, 0x00AA00ul);  // dark green
	dst->SetColor(  3, 0x00AAAAul);  // dark cyan
	dst->SetColor(  4, 0xAA0000ul);  // dark red
	dst->SetColor(  5, 0xAA00AAul);  // dark purple
	dst->SetColor(  6, 0xAA5500ul);  // brown
	dst->SetColor(  7, 0xAAAAAAul);  // light gray
	dst->SetColor(  8, 0x555555ul);  // dark gray
	dst->SetColor(  9, 0x5555FFul);  // bright blue
	dst->SetColor( 10, 0x55FF55ul);  // bright green
	dst->SetColor( 11, 0x00FFFFul);  // bright cyan
	dst->SetColor( 12, 0xFF5555ul);  // bright red
	dst->SetColor( 13, 0xFF00FFul);  // bright purple
	dst->SetColor( 14, 0xFFFF55ul);  // yellow
	dst->SetColor( 15, 0xFFFFFFul);  // white black
}

void TPalAnsiCygwin::CalcPal(RawRGBImage* src, TextPal* dst)
{
	dst->SetColor(  0, 0x000000ul);  // black
	dst->SetColor(  1, 0x800000ul);  // dark blue
	dst->SetColor(  2, 0x008000ul);  // dark green
	dst->SetColor(  3, 0x808000ul);  // dark cyan
	dst->SetColor(  4, 0x000080ul);  // dark red
	dst->SetColor(  5, 0x800080ul);  // dark purple
	dst->SetColor(  6, 0x008080ul);  // brown
	dst->SetColor(  7, 0xC0C0C0ul);  // light gray
	dst->SetColor(  8, 0x808080ul);  // dark gray
	dst->SetColor(  9, 0xFF0000ul);  // bright blue
	dst->SetColor( 10, 0x00FF00ul);  // bright green
	dst->SetColor( 11, 0xFFFF00ul);  // bright cyan
	dst->SetColor( 12, 0x0000FFul);  // bright red
	dst->SetColor( 13, 0xFF00FFul);  // bright purple
	dst->SetColor( 14, 0x00FFFFul);  // yellow
	dst->SetColor( 15, 0xFFFFFFul);  // white black
}

void TPalAnsiPutty::CalcPal(RawRGBImage* src, TextPal* dst)
{
	dst->SetColor(  0, 0x000000ul);  // black
	dst->SetColor(  1, 0xBB0000ul);  // dark blue
	dst->SetColor(  2, 0x00BB00ul);  // dark green
	dst->SetColor(  3, 0xBBBB00ul);  // dark cyan
	dst->SetColor(  4, 0x0000BBul);  // dark red
	dst->SetColor(  5, 0xBB00BBul);  // dark purple
	dst->SetColor(  6, 0x00BBBBul);  // brown
	dst->SetColor(  7, 0xBBBBBBul);  // light gray
	dst->SetColor(  8, 0x555555ul);  // dark gray
	dst->SetColor(  9, 0xFF5555ul);  // bright blue
	dst->SetColor( 10, 0x55FF55ul);  // bright green
	dst->SetColor( 11, 0xFFFF55ul);  // bright cyan
	dst->SetColor( 12, 0x5555FFul);  // bright red
	dst->SetColor( 13, 0xFF55FFul);  // bright purple
	dst->SetColor( 14, 0x55FFFFul);  // yellow
	dst->SetColor( 15, 0xFFFFFFul);  // white black
}

void TPalMedianCut::CalcPal(RawRGBImage* src, TextPal* dst)
{
	APoint* points = new APoint[src->GetHeight() * src->GetWidth()];
	memcpy(points, src->data, src->GetHeight() * src->GetWidth() * 3);
	std::list<APoint> tpal = medianCut(points, src->GetHeight() * src->GetWidth(), 16);

	uint32 ti = 0;
	for (std::list<APoint>::iterator iter = tpal.begin() ; iter != tpal.end(); iter++) {
		dst->SetColor(ti,(*iter).x);
		++ti;
	}

	delete[] points;
}


TPalMedianCutSort::TPalMedianCutSort(double t)
{
	threshold = t;
}

void TPalMedianCutSort::CalcPal(RawRGBImage* src, TextPal* dst)
{
	TextPal newpal;
	TextPal medpal;

	TPalMedianCut::CalcPal(src, &medpal);

//	for (int i = 0; i < 16; ++i) {
//		newpal.SetColor(i,dst->GetColor(i));
//	}

	bool *old_done = new bool[16];
	bool *med_done = new bool[16];
	for(int i=0; i < 16; i++) {
		old_done[i]=false;
		med_done[i]=false;
	}

	double total_dist = 0;

	for(int u = 0 ; u < 16 ; u++) {  // for every entry in the palette
		uint32 best_dist = ULONG_MAX;

		long int best_old = -1;
		long int best_med = -1;

		for(int i = 0 ; i < 16 ; i++)  // for every entry in the palette not assigned yet
		if(!old_done[i]) {
			for(int j = 0 ; j < 16 ; j++) // try all (remaining) entries to find the lowest diff
			if(!med_done[j]) {
				uint32 dist = MRGBDistInt(dst->GetColor(i), medpal.GetColor(j));
				if(dist < best_dist) {
					best_dist = dist;
					best_old = i;
					best_med = j;
				}
			}
		}
		if (best_old == -1 || best_med == -1)
			best_old=0;
		old_done[best_old]=true;
		med_done[best_med]=true;
		newpal.SetColor(best_old, medpal.GetColor(best_med));
		total_dist += sqrt(best_dist);
	}

	if (total_dist > threshold)
		for (uint32 i = 0; i < 16; ++i)
			dst->SetColor(i, newpal.GetColor(i));
}

TPalMedianCutRandomSort::TPalMedianCutRandomSort(uint32 t)
{
  threshold = t;
}

TPalMedianCutSmartSort::TPalMedianCutSmartSort(uint32 t)
{
  threshold = t;
}

void TPalMedianCutSmartSort::CalcPal(RawRGBImage* src, TextPal* dst) {
  /*TODO: Pair matching important for best pair or just for best match per index in new palette ?
   *
   * /

  TextPal newpal = (*dst);
  TextPal oldpal = (*dst);
  TPalMedianCut::CalcPal(src, &newpal);

  uint32 best_dist=ULONG_MAX;
  uint32 dist=0;
  uint32 index=0;
  uint32 match=0;
  bool avail[16];
  for(uint32 i=0; i < 16; i++) {
    avail[i]=true;
  }
  for (uint32 i = 0; i < 16; ++i) { // for every color in the new palette
    dist=0;
    best_dist=ULONG_MAX;
    for (uint32 i = 0; j < 16; ++j) { // calculate 'best' match with a color from the old palette
      if(avail[i]){
        dist=MRGBDistInt(newpal.GetColor(i), oldpal.GetColor(j));
        if(dist<best_dist){
          dist=best_dist;
          match=j;
        }
      }
    }
    //match is the best color match from oldpal[] for newpal[i], and best_dist the distance of the match
    if(best_dist<threshold) {
      (*dst).SetColor(i,oldpal.GetColor(match));
    }
    else {
      (*dst).SetColor(i,newpal.GetColor(match));
    }
    avail[match]=false;
  }

  /**/
  TextPal newpal = (*dst);
  TextPal oldpal = (*dst);
  TPalMedianCut::CalcPal(src, &newpal);
  struct match {
    uint32 index;
    uint32 dist;
    };
  struct match m[16];
  struct match r[16];
  bool avail_new[16];
  bool avail_old[16];
  for(uint32 i=0; i < 16; i++) {
    avail_new[i]=true;
    avail_old[i]=true;
  }
  uint32 best_dist=ULONG_MAX;
  uint32 dist=0;
  uint32 index=0;
  for (uint32 l = 0; l < 16; ++l) { // for every color
    for (uint32 i = 0; i < 16; ++i) { // calculate 'best' match
      if(avail_new[i]) {
        best_dist=ULONG_MAX;
        dist=0;
        for (uint32 j = 0; j < 16; ++j) { // amidst remaining colors
          if(avail_old[j]) {
            dist=MRGBDistInt(newpal.GetColor(i), oldpal.GetColor(j));
            if(dist<best_dist) {
              dist=best_dist;
              m[i].dist = best_dist;
              m[i].index = j;
            }
          }
        }
      }
    }
    //m is array with for every color in NewPal the index for the best matching color in Oldpal
    dist=0;
    best_dist=ULONG_MAX;
    index=0;
    for(uint32 i = 0; i < 16; i++) {
      if(avail_new[i]){
        if(m[i].dist < best_dist) {
          best_dist=m[i].dist;
          index=i;
        }
      }
      m[i].dist=ULONG_MAX;
    }
    //index is the best match for color [index] in newpal with oldpal m[index].index
    avail_new[index]=false;
    avail_old[index]=false;
    r[index].dist=m[index].dist;
    r[index].index=m[index].index;
  }
  TextPal result;
  for(uint32 i=0; i<16; i++) {
    if(r[i].dist < threshold) {
      result.SetColor(i, oldpal.GetColor(r[i].index));
    }
    else {
      result.SetColor(i, newpal.GetColor(r[i].index));
    }
  }
  (*dst)=result;
  /**/
}

void TPalMedianCutRandomSort::CalcPal(RawRGBImage* src, TextPal* dst)
{
  TextPal newpal = (*dst);
  TextPal oldpal = (*dst);
  TPalMedianCut::CalcPal(src, &newpal);
  uint32 loopcount=16*1024;
  uint32 olddist=0;
  uint32 dist=0;
  uint32 best_dist=0;
  for (uint32 i = 0; i < 16; ++i)
    dist+=MRGBDistInt(oldpal.GetColor(i), newpal.GetColor(i));
  best_dist=dist;
  uint32 i, j;
  RGBColor tmpcolor;
  while(loopcount>0) {
    // Select 2 random indices
    i=rand()&0x0f;
    j=rand()&0x0f;
    if(i!=j) {
      olddist=dist;
      dist -= MRGBDistInt(oldpal.GetColor(i), newpal.GetColor(i)) + MRGBDistInt(oldpal.GetColor(j), newpal.GetColor(j));
      tmpcolor=newpal.GetColor(i);
      newpal.SetColor(i, newpal.GetColor(j));
      newpal.SetColor(j, tmpcolor);
      //we did a swap, now check the new distance
      dist += MRGBDistInt(oldpal.GetColor(i), newpal.GetColor(i)) + MRGBDistInt(oldpal.GetColor(j), newpal.GetColor(j));
      if(dist<best_dist) {
        best_dist = dist;
        }
      else { //this is not better, let's undo the damage :p
        tmpcolor = newpal.GetColor(i);
        newpal.SetColor(i, newpal.GetColor(j));
        newpal.SetColor(j, tmpcolor);
        dist=olddist;
        }
    }
    loopcount--;
  }

  if ( dist < threshold )
    (*dst)=oldpal;
  else
    (*dst)=newpal;
}

void TRenderBruteBlock::DoRender(RawRGBImage* src, TextImage* dst)
{
	RGBColor lookup[16][16][4];
	for( unsigned char bg=0;bg<16;bg++) {
		RGBColor bgcol = dst->pal->GetColor(bg);
		for( unsigned char fg=0;fg<16;fg++) {
			RGBColor fgcol = dst->pal->GetColor(fg);
			for( unsigned char quad=0; quad < 4; quad++) {
				// this is faster, TODO: macro-ize this in a nice way
				lookup[fg][bg][quad].c.r = (fgcol.c.r*quad*16 + bgcol.c.r*(64-(quad*16))) / 64;
				lookup[fg][bg][quad].c.g = (fgcol.c.g*quad*16 + bgcol.c.g*(64-(quad*16))) / 64;
				lookup[fg][bg][quad].c.b = (fgcol.c.b*quad*16 + bgcol.c.b*(64-(quad*16))) / 64;
			}
		}
	}

	for(uint32 y = 0 ; y < src->GetHeight(); y+=2) {
		for(uint32 x = 0 ; x < src->GetWidth(); x+=2) { // for every quad region
			RGBColor tcol[4];
			tcol[0] = src->GetPixel(x + 0,y + 0);
			tcol[1] = src->GetPixel(x + 1,y + 0);
			tcol[2] = src->GetPixel(x + 0,y + 1);
			tcol[3] = src->GetPixel(x + 1,y + 1);
			unsigned char best_bg;
			unsigned char best_fg;
			unsigned char best_char;

			unsigned long int char_dist;
			unsigned long int best_char_dist=ULONG_MAX;
			unsigned char best_quad[4];
			unsigned long int best_quad_dist;
			unsigned long int quad_dist;

			for( unsigned char bg=0;bg<16;bg++) {
				RGBColor bgcol = dst->pal->GetColor(bg);
				for( unsigned char fg=0;fg<16;fg++) // and for all fore- and background colors
				if (bg != fg) { // fg == bg case is redundant cause char 0 is only fg
					RGBColor fgcol = dst->pal->GetColor(fg);
//					char_dist = 0;
					char_dist = MRGBDistInt(fgcol, bgcol) >> 6;
					RGBColor* curlookup = &lookup[fg][bg][0];
					for( unsigned char region=0; region < 4 ; region ++) { // try all regions
						RGBColor ttcol = tcol[region];
						best_quad_dist=ULONG_MAX;
						for( unsigned char quad=0; quad < 4; quad++) { // with all 4 quad blocks

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
						best_bg=bg;
						best_fg=fg;
					}
				}
			}
			if (best_char == 0) best_fg = best_bg;
			if (best_bg == best_fg) best_char = (best_bg << 4) | best_bg;
			dst->SetChar(x>>1, y>>1, best_char, best_fg, best_bg);
		}
	}
}

TRenderSemiBruteBlock::TRenderSemiBruteBlock(uint32 t){
  threshold = t;
}

void TRenderSemiBruteBlock::DoRender(RawRGBImage* src, TextImage* dst)
{
	int keeptest = 0;

	// TODO: save this lookup, and only recreate it when our pallete changes
	RGBColor lookup[16][16][4];
	for( unsigned char bg=0;bg<16;bg++) {
		RGBColor bgcol = dst->pal->GetColor(bg);
		for( unsigned char fg=0;fg<16;fg++) {
			RGBColor fgcol = dst->pal->GetColor(fg);
			for( unsigned char quad=0; quad < 4; quad++) {
				// this is faster, TODO: macro-ize this in a nice way
				lookup[fg][bg][quad].c.r = (fgcol.c.r*quad*16 + bgcol.c.r*(64-(quad*16))) / 64;
				lookup[fg][bg][quad].c.g = (fgcol.c.g*quad*16 + bgcol.c.g*(64-(quad*16))) / 64;
				lookup[fg][bg][quad].c.b = (fgcol.c.b*quad*16 + bgcol.c.b*(64-(quad*16))) / 64;
			}
		}
	}

	for(uint32 y = 0 ; y < src->GetHeight(); y+=2) {
		for(uint32 x = 0 ; x < src->GetWidth(); x+=2) { // for every quad region
			RGBColor tcol[4];
			tcol[0] = src->GetPixel(x + 0,y + 0);
			tcol[1] = src->GetPixel(x + 1,y + 0);
			tcol[2] = src->GetPixel(x + 0,y + 1);
			tcol[3] = src->GetPixel(x + 1,y + 1);

			uint8 pcol[4];

			bool chosen[16];
			for (int j = 0; j < 16; ++j)
				chosen[j] = false;

			for (int i = 0; i < 4; ++i)
			{
				uint32 mindist = 0xFFFFFFFF;
				for (int j = 0; j < 16; ++j)
				if (!chosen[j])
				{
					uint32 dist = MRGBDistInt(dst->pal->GetColor(j), tcol[i]);
					if (dist < mindist) {
						mindist = dist;
						pcol[i] = j;
					}
				}
				chosen[pcol[i]] = true;
			}

			unsigned char best_bg;
			unsigned char best_fg;
			unsigned char best_char;

			unsigned long int char_dist;
			unsigned long int best_char_dist=ULONG_MAX;
			unsigned char best_quad[4];
			unsigned long int best_quad_dist;
			unsigned long int quad_dist;

//			best_fg = pcol[0];
//			best_bg = pcol[0];
//			best_char = 0;

			for( unsigned char bg=0;bg<4;bg++) {
//				RGBColor bgcol = dst->pal->GetColor(pcol[bg]);
				for( unsigned char fg=0;fg<4;fg++) // and for all fore- and background colors
				if (bg != fg) { // fg == bg case is redundant cause char 0 is only fg
//					RGBColor fgcol = dst->pal->GetColor(pcol[fg]);
					char_dist = 0;
//					char_dist = MRGBDistInt(fgcol, bgcol) >> 6;
					RGBColor* curlookup = &lookup[pcol[fg]][pcol[bg]][0];
					for( unsigned char region=0; region < 4 ; region ++) { // try all regions
						RGBColor ttcol = tcol[region];
						best_quad_dist=ULONG_MAX;
						for( unsigned char quad=0; quad < 4; quad++) { // with all 4 quad blocks

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

      #define BLAAT 148
			if(BLAAT>0) {
				static uint16 oldbuffer[8000];
				uint8 old_char  = oldbuffer[x +     y*80];
				uint8 old_color = oldbuffer[x + 1 + y*80];
				uint8 lfg = old_color & 0xF;
				uint8 lbg = old_color >> 4;
				uint8 old_quad[4];
				old_quad[0]=old_char&0x03;
				old_quad[1]=(old_char>>2)&0x03;
				old_quad[2]=(old_char>>4)&0x03;
				old_quad[3]=(old_char>>6)&0x03;
				bool keepme=true;
				char_dist=0;

				for(uint8 region=0; region < 4 ; region ++) { // try all regions
					RGBColor ttcol = tcol[region];
					keepme = keepme && (MRGBDistInt(ttcol, lookup[lfg][lbg][old_quad[region]]) < BLAAT);
				}
				if(keepme){
					++keeptest;
					best_char = old_char;
					best_fg = old_color&0x0f;//old_best_fg;
					best_bg = (old_color>>4)&0x0f;//old_best_bg;
				} else {
					oldbuffer[x +     y*80] = best_char;
					oldbuffer[x + 1 + y*80] = best_fg | (best_bg <<4);
				}
			}

			//if (best_char == 0) best_fg = best_bg;
			//if (best_bg == best_fg) best_char = (best_bg << 4) | best_bg;
			dst->SetChar(x>>1, y>>1, best_char, best_fg, best_bg);
		}
	}

	keeptest = 0;
}

#pragma package(smart_init)


