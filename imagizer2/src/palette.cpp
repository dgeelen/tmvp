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

unsigned long int random_sort( unsigned char *pal,unsigned char *prevpal ) {
  #define sqr(x) ((x)*(x))
  unsigned long max_dist=1024*16; //ULONG_MAX; // temperature
  unsigned long int dist=0;
  unsigned long int olddist=0;
  unsigned long int best_dist=ULONG_MAX;
  for(int i = 0 ; i<16; i++){
    dist += sqr((((unsigned long int)(prevpal[3*i+0]))-((unsigned long int)(pal[3*i+0]))))+
            sqr((((unsigned long int)(prevpal[3*i+1]))-((unsigned long int)(pal[3*i+1]))))+
            sqr((((unsigned long int)(prevpal[3*i+2]))-((unsigned long int)(pal[3*i+2]))));
    }
  best_dist = dist;
  FILE *rnd = fopen("/dev/urandom","r");
  if(rnd==NULL) {
    fprintf(stderr, "Error opening /dev/urandom!\n");
    return -1;
    }
  unsigned char r=0;
  #define randomize (r = rand())
  while(max_dist>0) {
    //try a swap
    unsigned char i=r&0xf; randomize;
    unsigned char j=r&0xf; randomize;
    unsigned char tmp_r, tmp_g, tmp_b;
    if(i!=j) {
      olddist=dist;
      dist -= sqr((((unsigned long int)(prevpal[3*i+0]))-((unsigned long int)(pal[3*i+0]))))+
              sqr((((unsigned long int)(prevpal[3*i+1]))-((unsigned long int)(pal[3*i+1]))))+
              sqr((((unsigned long int)(prevpal[3*i+2]))-((unsigned long int)(pal[3*i+2]))))+
              sqr((((unsigned long int)(prevpal[3*j+0]))-((unsigned long int)(pal[3*j+0]))))+
              sqr((((unsigned long int)(prevpal[3*j+1]))-((unsigned long int)(pal[3*j+1]))))+
              sqr((((unsigned long int)(prevpal[3*j+2]))-((unsigned long int)(pal[3*j+2]))));
      tmp_r = pal[3*i+0];
      tmp_g = pal[3*i+1];
      tmp_b = pal[3*i+2];
      pal[3*i+0]=pal[3*j+0];
      pal[3*i+1]=pal[3*j+1];
      pal[3*i+2]=pal[3*j+2];
      pal[3*j+0]=tmp_r;
      pal[3*j+1]=tmp_g;
      pal[3*j+2]=tmp_b;
      //we did a swap, now check the new temperature
      dist += sqr((((unsigned long int)(prevpal[3*i+0]))-((unsigned long int)(pal[3*i+0]))))+
              sqr((((unsigned long int)(prevpal[3*i+1]))-((unsigned long int)(pal[3*i+1]))))+
              sqr((((unsigned long int)(prevpal[3*i+2]))-((unsigned long int)(pal[3*i+2]))))+
              sqr((((unsigned long int)(prevpal[3*j+0]))-((unsigned long int)(pal[3*j+0]))))+
              sqr((((unsigned long int)(prevpal[3*j+1]))-((unsigned long int)(pal[3*j+1]))))+
              sqr((((unsigned long int)(prevpal[3*j+2]))-((unsigned long int)(pal[3*j+2]))));
      if(dist<best_dist + (max_dist>>1) ) {
        best_dist = dist;
        }
      else { //this is not better, let's undo the damage :p
        tmp_r = pal[3*i+0];
        tmp_g = pal[3*i+1];
        tmp_b = pal[3*i+2];
        pal[3*i+0]=pal[3*j+0];
        pal[3*i+1]=pal[3*j+1];
        pal[3*i+2]=pal[3*j+2];
        pal[3*j+0]=tmp_r;
        pal[3*j+1]=tmp_g;
        pal[3*j+2]=tmp_b;
        dist=olddist;
        }
//      max_dist>best_dist?max_dist-=best_dist:(max_dist>10?max_dist-=10:max_dist--); //lower the temperature a bit
      max_dist--;
      //fprintf(stderr, "temperature=%u, best_dist=%u\n",max_dist, best_dist);
      }
    }
  //fprintf(stderr, "best_dist=%u\n",best_dist);
  fclose(rnd);
  return best_dist;
  }

void ryasa(unsigned char *pal,unsigned char *prevpal, unsigned char *bestpal, int pos, unsigned long int *best_dist) {
  #define sqr(x) ((x)*(x))
  if(pos<15) {
    for(int i=pos; i<16; i++) { //swap [pos] with [i], i!=pos, recurse
      //do the swap
      unsigned char tmp_r = pal[3*i+0];
      unsigned char tmp_g = pal[3*i+1];
      unsigned char tmp_b = pal[3*i+2];
      pal[3*i+0]=pal[3*pos+0];
      pal[3*i+1]=pal[3*pos+1];
      pal[3*i+2]=pal[3*pos+2];
      pal[3*pos+0]=tmp_r;
      pal[3*pos+1]=tmp_g;
      pal[3*pos+2]=tmp_b;
      //entries swapped, recurse
      ryasa(pal, prevpal, bestpal, pos + 1, best_dist);
      //undo swap
      pal[3*pos+0]=pal[3*i+0];
      pal[3*pos+1]=pal[3*i+1];
      pal[3*pos+2]=pal[3*i+2];
      pal[3*i+0]=tmp_r;
      pal[3*i+1]=tmp_g;
      pal[3*i+2]=tmp_b;
      }
    }
  else { /*/ pos >= 16                      //blerg assume it verks
    for(int i=0; i<16; i++) {
      fprintf(stderr,"%x",pal[i*3]);
      }
    fprintf(stderr,"\n"); /**/
    //now we should have a permutation, try it out!
    unsigned long int dist = 0;
    for(int i=0; i<16; i++) {
      dist +=
      sqr((((unsigned long int)(prevpal[3*i+0]))-((unsigned long int)(pal[3*i+0]))))+
      sqr((((unsigned long int)(prevpal[3*i+1]))-((unsigned long int)(pal[3*i+1]))))+
      sqr((((unsigned long int)(prevpal[3*i+2]))-((unsigned long int)(pal[3*i+2]))));
      }
    if(dist < *best_dist) {  // copy this permutation to bestpal
      *best_dist = dist;
      fprintf(stderr,"best_dist = %u, permutation=", *best_dist);
      for(int i=0; i<16; i++) {
        fprintf(stderr, "%x", pal[3*i]);
        bestpal[3*i+0]=pal[3*i+0];
        bestpal[3*i+1]=pal[3*i+1];
        bestpal[3*i+2]=pal[3*i+2];
        }
      fprintf(stderr,"\n");
      }
    }
  }

void find_opt_pal(unsigned char * img, unsigned char *palette,unsigned char *prevpalette, unsigned long int width, unsigned long int height, unsigned long int max_pal_dist) {
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
  /* Sort the palette according to distance from (0, 0, 0,) to reduce flikker */
  //median_cut_palette.sort();
  unsigned long int i=0;
  for (iter = median_cut_palette.begin() ; iter != median_cut_palette.end(); iter++) {
    palette[3*i+0]=(int)iter->x[0];
    palette[3*i+1]=(int)iter->x[1];
    palette[3*i+2]=(int)iter->x[2];
    i++;
    } /**/

  // Anneal sort
  unsigned long int dist=random_sort( palette,prevpalette );
  if(  dist< max_pal_dist) {
    //fprintf(stderr,"dist=%08u->old   ",dist);
    for(int i = 0 ; i < 48; i++) {
      palette[i] = prevpalette[i];
      }
    }
  else {
    //fprintf(stderr,"dist=%08u->new   ",dist);
    }
  delete points;


  /* YASA! (Yet Another Sorting Algo!) * /// Takes for fucking ever :(
  for(int i=0; i<16; i++) { //Fill palette with test-pattern
    palette[3*i+0]=i;
    palette[3*i+1]=i;
    palette[3*i+2]=i;
    }
  unsigned char *bestpal=new unsigned char[48];
  unsigned long int best_dist=ULONG_MAX;
  ryasa(palette,prevpalette,bestpal, 0, &best_dist );
  for(int i=0; i<16; i++) {
    palette[3*i+0]=bestpal[3*i+0];
    palette[3*i+1]=bestpal[3*i+1];
    palette[3*i+2]=bestpal[3*i+2];
    }

  /* Different sorting algo */ /* Yes this is a bit greedy * /
  char *unspalette = new char [3*16];
  unsigned long int i=0;
  for (iter = median_cut_palette.begin() ; iter != median_cut_palette.end(); iter++) {
    unspalette[3*i+0]=(int)iter->x[0];
    unspalette[3*i+1]=(int)iter->x[1];
    unspalette[3*i+2]=(int)iter->x[2];
    i++;
    }

  //provide a fixed palette
  for(int i=0; i<16; i++) {
  //  unspalette[3*i+0]=i<<3;
  //  unspalette[3*i+1]=i<<3;
  //  unspalette[3*i+2]=i<<3;
  //  prevpalette[46-3*i+0]=i<<3;
  //  prevpalette[46-3*i+1]=i<<3;
  //  prevpalette[46-3*i+2]=i<<3;
    prevpalette[3*i+0]=i<<3;
    prevpalette[3*i+1]=i<<3;
    prevpalette[3*i+2]=i<<3;
    } /** /

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

  /*/get_default_palette( prevpalette );
  for(int i=0; i<16; i++) {
    palette[i*3+0]=unspalette[i*3+0];
    palette[i*3+1]=unspalette[i*3+1];
    palette[i*3+2]=unspalette[i*3+2];
    } */

  /*/DEBUG PALETTE
  if(pal==NULL ) {
    pal = fopen("palette.raw","wb");
    }
  fprintf(stderr,"writing palette\n");
  for(int a=0; a<32; a++) {
    for(int i=0; i<16; i++) {
      for(int b=0; b<32; b++) {
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
