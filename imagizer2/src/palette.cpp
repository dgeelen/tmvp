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
#include <map>
#include "palette.h"
#include "octree.h"
#include "mediancut.h"

FILE *pal = NULL;

void test_octree() {
  octree t(8,16);
  octreenode *n=new octreenode;
  n->red=25;
  n->green=55;
  n->blue=127;
  n->isLeaf=false;
  n->references=0;
  for(int i=0; i<8; i++) {
    n->child[i]=NULL;
    }
  /*t.insert_color(128,200,10);
  t.insert_color(54,0,54);
  t.insert_color(128,200,10);*/
  }

void palette6bit(unsigned char *palette) {
  for(unsigned long int i=0 ; i<48; i++) {
    palette[i]=(palette[i]>>2);
    }
  }

void get_default_palette(unsigned char *palette) {
  /* /                  black, dblue, dgreen, dcyan, dred, dpurple, dyellow, lgrey, drey, lblue, lgreen, lcyan, lred, lpink, lyellow, white */
  const unsigned char r[16] = { 0x00,  0x00,   0x00,  0x00, 0xa8,    0xa8,    0xa8,  0xa8, 0x54,  0x54,   0x54,  0x54, 0xfc,  0xfc,    0xfc,  0xfc };
  const unsigned char g[16] = { 0x00,  0x00,   0xa8,  0xa8, 0x00,    0x00,    0x54,  0xa8, 0x54,  0x54,   0xfc,  0xfc, 0x54,  0x54,    0xfc,  0xfc };
  const unsigned char b[16] = { 0x00,  0xa8,   0x00,  0xa8, 0x00,    0xa8,    0x00,  0xa8, 0x54,  0xfc,   0x54,  0xfc, 0x54,  0xfc,    0x54,  0xfc };
  for(int i=0; i<16; i++) {
    palette[3*i+0]=r[i];
    palette[3*i+1]=g[i];
    palette[3*i+2]=b[i];
    }
  }

void find_opt_pal(unsigned char * img, unsigned char *palette,unsigned char *prevpalette, unsigned long int width, unsigned long int height) {
/*  for(unsigned long int i=0; i<width*height;i++){ //6bit precision
    img[i]=img[i]&0xfc;
    } */
  Point *points = new Point[width * height]; //neccessary because medianCut sorts this array
  memcpy(points,img,width*height*3);
  std::list<Point> median_cut_palette = medianCut(points, width*height, 16);
  std::list<Point>::iterator iter;
/*  for (iter = median_cut_palette.begin() ; iter != median_cut_palette.end(); iter++) { //palette precision is only 6bits :)
    iter->x[0]=(iter->x[0])&0xfc;
    iter->x[1]=(iter->x[1])&0xfc;
    iter->x[2]=(iter->x[2])&0xfc;
    } /*
  /* Sort the palette according to distance from (0, 0, 0,) to reduce flikker * /
  median_cut_palette.sort();
  unsigned long int i=0;
  for (iter = median_cut_palette.begin() ; iter != median_cut_palette.end(); iter++) {
    palette[3*i+0]=(int)iter->x[0];
    palette[3*i+1]=(int)iter->x[1];
    palette[3*i+2]=(int)iter->x[2];
    i++;
    } /**/


  /* Different sorting algo */ /* Yes this is a bit greedy */
  char *unspalette = new char [3*16];
  unsigned long int i=0;
  for (iter = median_cut_palette.begin() ; iter != median_cut_palette.end(); iter++) {
    unspalette[3*i+0]=(int)iter->x[0];
    unspalette[3*i+1]=(int)iter->x[1];
    unspalette[3*i+2]=(int)iter->x[2];
    i++;
    }

  #define pal_r(p,x) ((unsigned long int)(p[3*(x)+0]))
  #define pal_g(p,x) ((unsigned long int)(p[3*(x)+1]))
  #define pal_b(p,x) ((unsigned long int)(p[3*(x)+2]))
  #define sqr(x) ((x)*(x))
  #define r_dist(x,y) (sqr((pal_r(prevpalette,x) - pal_r(unspalette,y))))
  #define g_dist(x,y) (sqr((pal_g(prevpalette,x) - pal_g(unspalette,y))))
  #define b_dist(x,y) (sqr((pal_b(prevpalette,x) - pal_b(unspalette,y))))
  #define rgb_dist(x,y) (r_dist(x,y) + g_dist(x,y) + b_dist(x,y))
  bool *pdone = new bool[16];
  bool *idone = new bool[16];
  for(int i=0; i < 16; i++) {
    pdone[i]=false;
    idone[i]=false;
    }
  for(int u = 0 ; u < 16 ; u++) {  // for every entry in the palette
    unsigned long int dist = ULONG_MAX;
    unsigned long int best_p_dist = ULONG_MAX;
    unsigned long int best_i_dist = ULONG_MAX;
    unsigned long int best_ip= 0;
    unsigned long int best_p = 0;
    unsigned long int best_i = 0;
    for(int i = 0 ; i < 16 ; i++) {  // for every entry in the palette not assigned yet
      best_p_dist=ULONG_MAX;
      best_p = 0;
      if(!idone[i]) {
        for(int p = 0 ; p < 16 ; p++) { // try all (remaining) entries to find the lowest diff
          if(!pdone[p]) {
            dist = rgb_dist(i,p);
            if(dist < best_p_dist) {
              best_p_dist = dist;
              best_p = p;
              }
            }
          }
        // bestp = the match with shortest rgb_dist for this i, not already matched
        if(best_p_dist < best_i_dist) {
          best_i_dist = best_p_dist;
          best_i = i;
          best_ip = best_p;
          }
        }
      }
    idone[best_i]=true;
    pdone[best_ip]=true;
    palette[best_i*3+0]=unspalette[best_ip*3+0];
    palette[best_i*3+1]=unspalette[best_ip*3+1];
    palette[best_i*3+2]=unspalette[best_ip*3+2];
    }

  delete unspalette;
  delete pdone;
  delete idone;
  /**/

  get_default_palette( prevpalette );

  //DEBUG PALETTE
  if(pal==NULL ) {
    pal = fopen("palette.raw","wb");
    }
  fprintf(stderr,"writing palette\n");
  for(int a=0; a<64; a++) {
    for(int i=0; i<16; i++) {
      for(int b=0; b<64; b++) {
        fputc(palette[3*i+0],pal);
        fputc(palette[3*i+1],pal);
        fputc(palette[3*i+2],pal);
        }
      }
    }
  fprintf(stderr,"done\n");
//  fclose(pal);
  /*  octree t(8,16);

  for(int i=0; i<len; i+=3) {
    t.insert_color((unsigned char)img[i],(unsigned char)img[i+1],(unsigned char)img[i+2]);
    }

  t.fill_palette( palette);
  /*for(int y = 0 ; y< height; y++) {
    for(int x=0 ; x< width; x++ ) {
      insert_color((unsigned char)img[3*x+3*width*height]);
      }
    } */
  //get_default_palette(palette);
  }
