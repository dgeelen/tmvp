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
#include <math.h>
#include "imagizer.h"
#include "png.h"
#define sqr(x) ((x)*(x))

RGBColor::RGBColor() {
  r=0;
  g=0;
  b=0;
  }
unsigned long int RGBColor::distance(RGBColor &target) {
  return sqr(target.r - this->r)+sqr(target.g - this->g)+sqr(target.b - this->b);
  }
RGBColor::~RGBColor() { }
struct fgbg_verh {
  float fg;
  float bg;
  };

struct letter {
  unsigned char *rij;
  };

letter *font = NULL;
fgbg_verh * quad_verh = NULL;
unsigned char *bitcount = NULL;

void uninitialize_imagizer() {
  delete quad_verh;
  delete bitcount;
  for(int i =0; i<256;i++) {
    delete font[i].rij;
    }
  delete font;
  }
void initialize_imagizer(unsigned char * biosfont) {
  quad_verh = new fgbg_verh[4];
  bitcount = new unsigned char [256];
  font = new letter[256];
  for(int i=0; i<256; i++) {
    bitcount[i]=(i&1)+((i>>1)&1)+((i>>2)&1)+((i>>3)&1)+((i>>4)&1)+((i>>5)&1)+((i>>6)&1)+((i>>7)&1);
    }

  //for( unsigned long int y = 0 ; y < 16; y++) { // FIXME: This is only neccesary in the current font input format
    for( unsigned long int x = 0 ; x < 256; x++) {
      font[x].rij = new unsigned char[8];
      for(unsigned char omg =0; omg < 8 ; omg++) {
        font[x].rij[omg]=biosfont[(x<<3)+omg];
        }
      }
    //}
  quad_verh[0].fg= float(bitcount[font[0].rij[0]] + bitcount[font[0].rij[1]] + bitcount[font[0].rij[2]] + bitcount[font[0].rij[3]])/16.0;
  quad_verh[1].fg= float(bitcount[font[1].rij[0]] + bitcount[font[1].rij[1]] + bitcount[font[1].rij[2]] + bitcount[font[1].rij[3]])/16.0;
  quad_verh[2].fg= float(bitcount[font[2].rij[0]] + bitcount[font[2].rij[1]] + bitcount[font[2].rij[2]] + bitcount[font[2].rij[3]])/16.0;
  quad_verh[3].fg= float(bitcount[font[3].rij[0]] + bitcount[font[3].rij[1]] + bitcount[font[3].rij[2]] + bitcount[font[3].rij[3]])/16.0; //*/

/*  quad_verh[0].fg=0;
  quad_verh[1].fg=4.0/16.0;
  quad_verh[2].fg=8.0/16.0;
  quad_verh[3].fg=12.0/16.0; */
  quad_verh[0].bg= 1.0 - quad_verh[0].fg;
  quad_verh[1].bg= 1.0 - quad_verh[1].fg;
  quad_verh[2].bg= 1.0 - quad_verh[2].fg;
  quad_verh[3].bg= 1.0 - quad_verh[3].fg;
  }

void texttopng(unsigned char *textdata,unsigned char * palette) {
  unsigned char * img = new unsigned char[80*8*50*8*3];
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char tx;
  unsigned char ty;
  unsigned char cc;
  unsigned char cr;
  unsigned char fg;
  unsigned char bg;
  unsigned char fc;

  /*/wth
  for(int i=0; i<16; i++) {
    fprintf(stderr, "pal_%i = %i, %i, %i\n",i,palette[i*3+0],palette[i*3+1],palette[i*3+2]);
    } /**/

  for(unsigned long int y=0; y<400;y++) {
    for(unsigned long int x=0; x<80;x++) {
      tx=x;
      ty=y/8;
      cc=textdata[2*tx+160*ty];
      cr=font[cc].rij[y%8];
      fg=  textdata[2*tx+160*ty+1]&0x0f;
      bg=((textdata[2*tx+160*ty+1])>>4)&0x0f;
      for(char a=7; a>=0; a--) {
        if((cr>>a)&1) {
          img[x*8*3+640*y*3+3*(7-a)+0]=palette[fg*3+0];
          img[x*8*3+640*y*3+3*(7-a)+1]=palette[fg*3+1];
          img[x*8*3+640*y*3+3*(7-a)+2]=palette[fg*3+2];
          }
        else {
          img[x*8*3+640*y*3+3*(7-a)+0]=palette[bg*3+0];
          img[x*8*3+640*y*3+3*(7-a)+1]=palette[bg*3+1];
          img[x*8*3+640*y*3+3*(7-a)+2]=palette[bg*3+2];
          }
        }
      }
    } //*/
/*  FILE *rawfile = fopen("debug.raw","wb");
  fwrite(img,1,640*400*3,rawfile);
  fclose(rawfile);
  for(unsigned long int y=0; y<128;y++) {
    for(unsigned long int x=0; x<16;x++) {
      tx=x;
      ty=y/8;
      cc=tx+16*ty;
      cr=y%8;
      fc=font[cc].rij[cr];
      for(char a=7; a>=0; a--) {
        if((fc>>a)&1) {
          img[x*3*8+128*y*3+3*(7-a)+0]=255;
          img[x*3*8+128*y*3+3*(7-a)+1]=255;
          img[x*3*8+128*y*3+3*(7-a)+2]=255;
          }
        else {
          img[x*3*8+128*y*3+3*(7-a)+0]=0;
          img[x*3*8+128*y*3+3*(7-a)+1]=0;
          img[x*3*8+128*y*3+3*(7-a)+2]=0;
          }
        }
      }
    }
  FILE *fonfile = fopen("font.raw","wb");
  fwrite(img,1,128*128*3,rawfile);
  fclose(fonfile);
  //return; */
  FILE *pngfile = fopen("debug.png","wb");
  if(pngfile) {
    png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
    if (png_ptr) {
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr) {
        png_init_io(png_ptr, pngfile);
        png_set_IHDR(png_ptr, info_ptr, 640, 400, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_bytep *row_pointers = new png_bytep[400];
        for(unsigned long int i = 0 ; i < 400 ; i++) {
          row_pointers[i]=(png_bytep)&img[i*640*3];
          }
        png_set_rows(png_ptr, info_ptr, row_pointers);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
        png_write_end(png_ptr, info_ptr);
        for(unsigned long int i = 0 ; i < 400 ; i++) {
          row_pointers[i]=NULL;
          }
        delete row_pointers;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        }
      else {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        }
      }
    }
  fclose(pngfile);
  delete img;
  }

unsigned long int skip_count = 0;
void imagize(unsigned char * img, unsigned char * palette, unsigned char *b800h, unsigned char *prevb800h, unsigned long int img_width,unsigned long int img_height) {
#define exhaustive_search
//#define greedy
#ifdef greedy
  #define sqr(x) ((x)*(x))
  #define pal_r(x) (palette[3*x+0])
  #define pal_g(x) (palette[3*x+1])
  #define pal_b(x) (palette[3*x+2])
  #define img_r(x,y) (img[0+x*3+3*img_width*y])
  #define img_g(x,y) (img[1+x*3+3*img_width*y])
  #define img_b(x,y) (img[2+x*3+3*img_width*y])
  #define distance(x,y) sqr(int(img_r((x),(y)))-int(pal_r(fg))) + sqr(int(img_g((x),(y)))-int(pal_g(fg))) + sqr(int(img_b((x),(y)))-int(pal_b(fg)));

  #define quad_r(quad,_fg,_bg) (((unsigned long int)((quad_verh[quad].fg)*pal_r(_fg)))+((unsigned long int)((quad_verh[quad].bg)*pal_r(_bg))))
  #define quad_g(quad,_fg,_bg) (((unsigned long int)((quad_verh[quad].fg)*pal_g(_fg)))+((unsigned long int)((quad_verh[quad].bg)*pal_g(_bg))))
  #define quad_b(quad,_fg,_bg) (((unsigned long int)((quad_verh[quad].fg)*pal_b(_fg)))+((unsigned long int)((quad_verh[quad].bg)*pal_b(_bg))))

  #define quad_dr (sqr(int(img_r((x+(region%2)),(y+(region>1?1:0)))) - int(quad_r(quad,best_fg,bg))))
  #define quad_dg (sqr(int(img_g((x+(region%2)),(y+(region>1?1:0)))) - int(quad_g(quad,best_fg,bg))))
  #define quad_db (sqr(int(img_b((x+(region%2)),(y+(region>1?1:0)))) - int(quad_b(quad,best_fg,bg))))

  for( unsigned long int y = 0 ; y < img_height; y+=2) {
    for( unsigned long int x = 0 ; x < img_width; x+=2) { // for every quad region
      unsigned long int best_dist=ULONG_MAX;
      unsigned long int dist=ULONG_MAX;
      unsigned char best_fg=0;
      for(unsigned char fg=0; fg<16; fg++) {
        dist=distance(x,y)+distance(x+1,y)+distance(x,y+1)+distance(x+1,y+1);
        if(dist<best_dist) {
          best_dist=dist;
          best_fg=fg;
          }
        }
      //best_fg=0;
      unsigned long int chardist=0;
      unsigned long int best_chardist=ULONG_MAX;
      unsigned char best_char=0;
      unsigned char best_bg=0;
      for(unsigned char bg=0; bg<16; bg++) { //for all combinations of bg with best_fg
        unsigned long int region_dist[4];
        unsigned char best_quad[4];
        for(unsigned char region=0; region < 4; region++) { //try for all regions
          unsigned long int quaddist=0;
          unsigned long int best_quaddist=ULONG_MAX;
          for(unsigned char quad=0; quad < 4; quad++) { //all quads
            quaddist=quad_dr+quad_dg+quad_db;
            if(quaddist<best_quaddist) {
              best_quaddist=quaddist;
              best_quad[region]=quad;
              }
            }
          region_dist[region]=best_quaddist;
          }
        chardist=region_dist[0]+region_dist[1]+region_dist[2]+region_dist[3];
        if(chardist<best_chardist) {
          best_chardist=chardist;
          best_bg=bg;
          best_char = best_quad[0] + (best_quad[1]<<2)+ (best_quad[2]<<4)+ (best_quad[3]<<6);/*best_quad[0]  * 1      // 4^0
                    + best_quad[1]  * 4      // 4^1
                    + best_quad[2]  * 4*4    // 4^2
                    + best_quad[3]  * 4*4*4; // 4^3 */
          }
        }
//      unsigned char best_char=3+4*3+4*4*3+4*4*4*3;
//      unsigned char best_bg=5;
      b800h[2*(x>>1) + 0 + 160*(y>>1)]=best_char;//3+4*3+4*4*3+4*4*4*3;//best_char;
      b800h[2*(x>>1) + 1 + 160*(y>>1)]=(best_fg&0x0f)|((best_bg<<4)&0xf0);
      }
    }
#endif

#ifdef exhaustive_search // this is now as-fast as greedy currently is
  #define pal_r(x) (palette[3*(x)])
  #define pal_g(x) (palette[3*(x)+1])
  #define pal_b(x) (palette[3*(x)+2])
  #define avg_r r_verh[(fg|(bg<<4))+(quad<<8)]
  #define avg_g g_verh[(fg|(bg<<4))+(quad<<8)]
  #define avg_b b_verh[(fg|(bg<<4))+(quad<<8)]
  #define img_r (img[0+x*3+3*(region&1)+3*img_width*(y+(region>>1))])
  #define img_g (img[1+x*3+3*(region&1)+3*img_width*(y+(region>>1))])
  #define img_b (img[2+x*3+3*(region&1)+3*img_width*(y+(region>>1))])
  #define sqr(x) ((x)*(x))

/*/DEBUG PALETTE
  for(int i=0; i<16; i++) {
    palette[3*i+0] = i << 3;
    palette[3*i+1] = i << 3;
    palette[3*i+2] = i << 3;
    } /**/

  uint16_t * b800h_l = (uint16_t *)b800h;
  unsigned char * r_verh = new unsigned char[1024];
  unsigned char * g_verh = new unsigned char[1024];
  unsigned char * b_verh = new unsigned char[1024];
  for(int i=0; i<1024; i++){
    r_verh[i]=(unsigned char)((float(pal_r(i&0x0f))*quad_verh[i>>8].fg)+(float(pal_r((i>>4)&0x0f))*quad_verh[i>>8].bg));
    g_verh[i]=(unsigned char)((float(pal_g(i&0x0f))*quad_verh[i>>8].fg)+(float(pal_g((i>>4)&0x0f))*quad_verh[i>>8].bg));
    b_verh[i]=(unsigned char)((float(pal_b(i&0x0f))*quad_verh[i>>8].fg)+(float(pal_b((i>>4)&0x0f))*quad_verh[i>>8].bg));
    }
  /*wth
  for(int i=0; i<16; i++) {
    fprintf(stderr, "pal_%i = %i, %i, %i\n",i,pal_r(i),pal_g(i),pal_b(i));
    } */
  for( unsigned long int y = 0 ; y < img_height; y+=2) {
    for( unsigned long int x = 0 ; x < img_width; x+=2) { // for every quad region
      unsigned char best_bg=0;
      unsigned char best_fg=0;
      unsigned char best_char;
      unsigned long int char_dist=0;
      unsigned long int best_char_dist=ULONG_MAX;
      unsigned char best_quad[4];
      unsigned long int best_quad_dist=ULONG_MAX;
      unsigned long int quad_dist=ULONG_MAX;
      for( unsigned char bg=0;bg<16;bg++) {
        for( unsigned char fg=0;fg<16;fg++) { // and for all fore- and background colors
          unsigned long int lr=pal_r(fg)-pal_r(bg);
          unsigned long int lg=pal_g(fg)-pal_g(bg);
          unsigned long int lb=pal_b(fg)-pal_b(bg);
          char_dist=(sqr(lr) + sqr(lg) + sqr(lb))>>6; //magic shift?
          //char_dist=(unsigned long int)(sqrtl(sqr(lr) + sqr(lg) + sqr(lb))) >>6;
          for( unsigned char region=0; region < 4 ; region ++) { // try all regions
            best_quad_dist=ULONG_MAX;
            for( unsigned char quad=0; quad < 4; quad++) { // with all 4 quad blocks
              lr=img_r-avg_r;
              lg=img_g-avg_g;
              lb=img_b-avg_b;
              quad_dist=        sqr(lr) +      sqr(lg) +      sqr(lb);
              //quad_dist=sqrtl(sqr(lr)+sqr(lg)+sqr(lb)); //precision+=minimal, time+=too much
              if(quad_dist<best_quad_dist) {
                 best_quad_dist=quad_dist;
                 best_quad[region]=quad;
                 }
              }
            char_dist+=best_quad_dist;
            }
          if(char_dist<best_char_dist) {
            // select the char representing our chosen quads
            best_char = best_quad[0] + (best_quad[1]<<2) + (best_quad[2]<<4) + (best_quad[3]<<6);
            best_char_dist=char_dist;
            best_bg=bg;
            best_fg=fg;
            }
          }
        }
      //b800h[x + 0 + 80*y]=best_char;
      //b800h[x + 1 + 80*y]=(best_fg&0x0f)|((best_bg<<4)&0xf0);

      /*best_char, best_fg and best_bg represent the current 'best'
        let's look at what it would look like to use the previous values
      */
      unsigned char old_char = prevb800h[ x + y*80 ];
      unsigned char old_best_fg = prevb800h[ x + 1 + y*80 ]&0x0f;
      unsigned char old_best_bg = (0x0f&prevb800h[x + 1 + y*80])>>4;
      unsigned char *old_quad = new unsigned char [4];
      old_quad[0]=old_char&0x03;
      old_quad[1]=old_char&(0x03<<2);
      old_quad[2]=old_char&(0x03<<4);
      old_quad[3]=old_char&(0x03<<6);
      unsigned long int old_dist = 0;
      for(int region = 0 ; region<4;region++){
        unsigned long int tr = ((unsigned long int)(img_r)) - ((unsigned long int)(r_verh[(old_best_fg|(old_best_bg<<4))+(old_quad[region]<<8)]));
        unsigned long int tg = ((unsigned long int)(img_g)) - ((unsigned long int)(g_verh[(old_best_fg|(old_best_bg<<4))+(old_quad[region]<<8)]));
        unsigned long int tb = ((unsigned long int)(img_b)) - ((unsigned long int)(b_verh[(old_best_fg|(old_best_bg<<4))+(old_quad[region]<<8)]));
        old_dist+= sqr(tr) + sqr(tg) + sqr(tb);
        }
      if(old_dist < 1024*16) {
        best_char = old_char;
        best_fg = old_best_fg;
        best_bg = old_best_bg;
        skip_count++;

        best_char=0;
        best_fg=15;
        best_bg=15;
        }
      /**small compression optimalisation (may be slightly larger for lz77)**/
      if(best_char==0) {
        best_fg=best_bg;
        }
      if(best_fg==best_bg) {
        best_char=((best_fg&0x0f)|((best_bg<<4)&0xf0));
        } /**/
      b800h_l[(x + 80 * y)>>1] = (uint16_t)((((best_fg&0x0f)|((best_bg<<4)&0xf0))<<8)|(best_char));
      }
    }

/*  for(int i=0; i<16; i++) {
    b800h_l[i]=(i|(i<<4))<<8;
    } /**/
  delete r_verh;
  delete g_verh;
  delete b_verh;
#endif
  fprintf(stderr,"skip_count=%u\n",skip_count);
  }
