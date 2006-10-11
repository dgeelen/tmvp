/***************************************************************************
 *   Copyright (C) 2006 by Da Fox   *
 *   dafox@shogoki   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
#include <caca.h>
#include <config.h>
#include <common.h>
#include <cucul.h>
#include <cucul_internals.h>

cucul_canvas_t *cv = NULL;
cucul_dither_t *dither = NULL;
bool init_caca(char *argv[], unsigned long int TEXTWIDTH, unsigned long int TEXTHEIGHT, unsigned long int PIXWIDTH, unsigned long int PIXHEIGHT,int hax) {
  cv = cucul_create_canvas(TEXTWIDTH, TEXTHEIGHT);
  if (cv  == NULL) {
    fprintf(stderr,"cucul_canvas_t unhappy!\n");
    return false;
    }
  dither = cucul_create_dither(24, PIXWIDTH, PIXHEIGHT, 3 * PIXWIDTH, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
  if (dither  == NULL) {
    fprintf(stderr,"cucul_create_dither unhappy!\n");
    return false;
    }
  cucul_set_dither_brightness(dither, strtof(argv[2],NULL));
  cucul_set_dither_gamma(dither, strtof(argv[3],NULL));
  cucul_set_dither_contrast(dither, strtof(argv[4],NULL));
  cucul_set_dither_mode(dither, argv[5]); // none, ordered2,ordered4, ordered8,random, fstein
  switch(hax) {
    case 1:
      cucul_set_dither_charset(dither, "shades"); // ascii, shades, blocks
      break;
    case 2:
      /* FIXME: Someday LibCaCa may actually support this */
      cucul_set_dither_color(dither, "gray"); // "mono", "gray", "8", "16", "fullgray", "full8", "full16"
      break;
    }
  return true;
  }

void pixtotext(unsigned long int* pix, unsigned short* text,int hax) {
  cucul_dither_bitmap(cv, 0, 0, cucul_get_canvas_width(cv), cucul_get_canvas_height(cv), dither, pix);
  for (int x=0; x<4000; ++x) {
    if(hax==1) {
    if (cv->chars[x]==0xb7)   cv->chars[x]=176;
    if (cv->chars[x]==0x2591) cv->chars[x]=177;
    if (cv->chars[x]==0x2592) cv->chars[x]=178;
    }
    //text[x] = (cv->chars[x] & 0xFF) | ((cv->attr[x] << 8) & 0x0F00) | ((cv->attr[x] << 4) & 0xF000);
    text[x] = (cv->chars[x] & 0xFF) | ((cv->attr[x] << 8) & 0x0F00) | ((cv->attr[x] >> 4) & 0xF000);
    }
  }

void uninit_caca() {
  cucul_free_dither(dither);
  cucul_free_canvas(cv);
  }


static inline int sq(int x) { return ((x) * (x)); }
unsigned char RGBto4bc(unsigned int c) {
  /***
  *  Possible colors:
  *   0x00 - Black
  *   0x01 - Dark blue
  *   0x02 - Dark green
  *   0x03 - Dark cyan (dark light blue-ish)
  *   0x04 - Dark red
  *   0x05 - Dark purple
  *   0x06 - Dark yellow (brown)
  *   0x07 - Light gr[ea]y
  *   0x08 - Dark gr[ea]y
  *   0x09 - Light blue
  *   0x0A - Bright green
  *   0x0B - Bright cyan (very bright blue-ish)
  *   0x0C - Bright red
  *   0x0D - Bright purple/pink
  *   0x0E - Bright yellow
  *   0x0F - Pure White
  */
/*int rgba[3]= { (c&0xff0000)>>16, (c&0x00ff00)>>8, (c&0x0000ff) };
  //fprintf(stderr, "COLOR: %x, %x, %x\n",rgba[0],rgba[1],rgba[2]);
  char outbg=7;
  int dist=0, distmin=INT_MAX;
/** Ripped from LIBCACA ** /
  for(int i = 0; i < 16; i++) {
    dist = sq(rgba[0] - rgb_palette[i * 3]) + sq(rgba[1] - rgb_palette[i * 3 + 1]) + sq(rgba[2] - rgb_palette[i * 3 + 2]);
    dist *= rgb_weight[i];
    if(dist < distmin) {
      fprintf(stderr,"wtf: dist=%i, distmin=%i, i=%i\n",dist,distmin,i);
      outbg = i;
      distmin = dist;
      }
    }
  return outbg;

  /* /                  black, dblue, dgreen, dcyan, dred, dpurple, dyellow, lgrey, drey, lblue, lgreen, lcyan, lred, lpink, lyellow, white */
  const char r[16] = { 0x00,  0x00,   0x00,  0x00, 0xa8,    0xa8,    0xa8,  0xa8, 0x54,  0x54,   0x54,  0x54, 0xfc,  0xfc,    0xfc,  0xfc };
  const char g[16] = { 0x00,  0x00,   0xa8,  0xa8, 0x00,    0x00,    0x54,  0xa8, 0x54,  0x54,   0xfc,  0xfc, 0x54,  0x54,    0xfc,  0xfc };
  const char b[16] = { 0x00,  0xa8,   0x00,  0xa8, 0x00,    0xa8,    0x00,  0xa8, 0x54,  0xfc,   0x54,  0xfc, 0x54,  0xfc,    0x54,  0xfc };
/*  const char rc= (c&0xff0000)>>16;
  const char gc= (c&0x00ff00)>> 8;
  const char bc= (c&0x0000ff)    ; */
  const int rc= (c&0xff0000)>>16;
  const int gc= (c&0x00ff00)>> 8;
  const int bc= (c&0x0000ff)    ;
  //fprintf(stderr,"inputcolor: r=%i g=%i b=%i\n",int(rc),int(gc),int(bc));
  long int D=0;
  long int BD=INT_MAX;
  char result;
  for(int i=0 ; i<16 ; i++) {
    //D= ((rc-r[i])*(rc-r[i]))+((gc-g[i])*(gc-g[i]))+((bc-b[i])*(bc-b[i]));
    D= sq(rc-r[i]) + sq(gc-g[i]) + sq(bc-b[i]);
//    fprintf(stderr, "---\nd=%i\n",D);
  //  D = sq(rc - rgb_palette[i * 3]) + sq(gc - rgb_palette[i * 3 + 1]) + sq(bc - rgb_palette[i * 3 + 2]);
//    fprintf(stderr, "Pal[%i]: %i , %i , %i\n",i,rgb_palette[i*3],rgb_palette[i*3+1],rgb_palette[i*3+2]);
    //fprintf(stderr, "d=%i best=%i\n",D, BD);
    if(D<BD) {
      BD=D;
      result=i;
      //fprintf(stderr,"Considering %i\n",i);
      }
    }
  //fprintf(stderr,"Decided on %i\n",result);
  return result;

//  char result=((rc&0xc0)>>2)|((gc&0xc0)>>4)|((bc&0xc0)>>6);
  /* /l00ps unrolled 4 t3h extra speed
  switch(result){
    case 0x00: //black
      result = 0x00;
      break;
    case 0x01: //dblue
      result = 0x01;
      break;
    case 0x02: //dcyan
      result = 0x03;
      break;
    case 0x03: //lbue
      result = 0x09;
      break;
    case 0x04: //dgreen
      result = 0x02;
      break;
    case 0x05: //dcyan
      result = 0x03;
      break;
    case 0x06: //lblue
      result = 0x09;
      break;
    case 0x07: //lblue
      result = 0x09;
      break;
    case 0x08: //dred
      result = 0x04;
      break;
    case 0x09: //dpurple
      result = 0x05;
      break;
    case 0x0A: //dpurple
      result = 0x05;
      break;
    case 0x0B: //black8*************************************
      result = 0x00;
      break;
    case 0x0C: //black
      result = 0x00;
      break;
    case 0x0D: //black
      result = 0x00;
      break;
    case 0x0E: //black
      result = 0x00;
      break;
    case 0x0F: //black
      result = 0x00;
      break;
    case 0x10: //black
      result = 0x00;
      break;
    case 0x11: //black
      result = 0x00;
      break;
    case 0x12: //black
      result = 0x00;
      break;
    case 0x13: //black
      result = 0x00;
      break;
    case 0x14: //black
      result = 0x00;
      break;
    case 0x15: //black
      result = 0x00;
      break;
    case 0x16: //black
      result = 0x00;
      break;
    case 0x17: //black
      result = 0x00;
      break;
    case 0x18: //black
      result = 0x00;
      break;
    case 0x19: //black
      result = 0x00;
      break;
    case 0x1A: //black
      result = 0x00;
      break;
    case 0x1B: //black
      result = 0x00;
      break;
    case 0x1C: //black
      result = 0x00;
      break;
    case 0x1D: //black
      result = 0x00;
      break;
    case 0x1E: //black
      result = 0x00;
      break;
    case 0x1F: //black
      result = 0x00;
      break;
    case 0x20: //black
      result = 0x00;
      break;
    case 0x21: //black
      result = 0x00;
      break;
    case 0x22: //black
      result = 0x00;
      break;
    case 0x23: //black
      result = 0x00;
      break;
    case 0x24: //black
      result = 0x00;
      break;
    case 0x25: //black
      result = 0x00;
      break;
    case 0x26: //black
      result = 0x00;
      break;
    case 0x27: //black
      result = 0x00;
      break;
    case 0x28: //black
      result = 0x00;
      break;
    case 0x29: //black
      result = 0x00;
      break;
    case 0x2A: //black
      result = 0x00;
      break;
    case 0x2B: //black
      result = 0x00;
      break;
    case 0x2C: //black
      result = 0x00;
      break;
    case 0x2D: //black
      result = 0x00;
      break;
    case 0x2E: //black
      result = 0x00;
      break;
    case 0x2F: //black
      result = 0x00;
      break;
    case 0x30: //black
      result = 0x00;
      break;
    case 0x31: //black
      result = 0x00;
      break;
    case 0x32: //black
      result = 0x00;
      break;
    case 0x33: //black
      result = 0x00;
      break;
    case 0x34: //black
      result = 0x00;
      break;
    case 0x35: //black
      result = 0x00;
      break;
    case 0x36: //black
      result = 0x00;
      break;
    case 0x37: //black
      result = 0x00;
      break;
    case 0x38: //black
      result = 0x00;
      break;
    case 0x39: //black
      result = 0x00;
      break;
    case 0x3A: //black
      result = 0x00;
      break;
    case 0x3B: //black
      result = 0x00;
      break;
    case 0x3C: //black
      result = 0x00;
      break;
    case 0x3D: //black
      result = 0x00;
      break;
    case 0x3E: //black
      result = 0x00;
      break;
    case 0x3F: //black
      result = 0x00;
      break;
    default:
      result = 0x07;
      break;
    }
  return result;
  /* /                  black, dblue, dgreen, dcyan, dred, dpurple, dyellow, lgrey, drey, lblue, lgreen, lcyan, lred, lpink, lyellow, white
  const char r[16] = { 0x00,  0x00,   0x00,  0x00, 0xa8,    0xa8,    0xa8,  0xa8, 0x54,  0x54,   0x54,  0x54, 0xfc,  0xfc,    0xfc,  0xfc };
  const char g[16] = { 0x00,  0x00,   0xa8,  0xa8, 0x00,    0x00,    0x54,  0xa8, 0x54,  0x54,   0xfc,  0xfc, 0x54,  0x54,    0xfc,  0xfc };
  const char b[16] = { 0x00,  0xa8,   0x00,  0xa8, 0x00,    0xa8,    0x00,  0xa8, 0x54,  0xfc,   0x54,  0xfc, 0x54,  0xfc,    0x54,  0xfc };
  const char rc= (c&0xff0000)>>16;
  const char gc= (c&0x00ff00)>> 8;
  const char bc= (c&0x0000ff)    ;
  int drift=0;
  int bestdrift=1024;
  char bestmatch=0;
  for(int i = 0 ; i<16; i++) {
    drift = abs(r[i]-rc)+abs(g[i]-gc)+abs(b[i]-gc);
    if(drift < bestdrift) {
      bestdrift=drift;
      bestmatch=i;
      }
    } */
  }
