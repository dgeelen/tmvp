/***************************************************************************
 *   Copyright (C) 2006 by Da Fox                                          *
 *   dafox@shogoki                                                         *
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <sys/time.h>
#include "mpngtocaas.h"
#include "readpng.h"
#include "aaify.h"
#include "aadiff.h"
#include "colorize.h"
#include "lzw.h"
#include "palette.h"
#include "imagizer.h"

#define AUDIO_BYTES_PER_FRAME 400
#define RawOutput
//#define FullLZW
//#define lz77s


using namespace std;
enum mode { dafox,  /* `Original' AA: Dark Grey/Light Grey/White text on Black background */
            thomas, /* Simulated 80x100 pixels, color */
            simon,  /* AA-Text, with color */
            opt_pal_opt_charset, /* Simon's algo */
            other   /* Your name could be here */
            };
#include <iostream>
int main(int argc, char *argv[]) {
  if(argc>2) {
    enum mode mode=opt_pal_opt_charset;
    /*if (argc>6) {  // select a mode
      switch(*(argv[6])) {
        case 'd':
          mode=dafox;
          fprintf(stderr, "Selected mode 'Da Fox'\n");
          break;
        case 't':
          mode=thomas;
          fprintf(stderr, "Selected mode 'Thomas'\n");
          break;
        case 's':
          mode=simon;
          fprintf(stderr, "Selected mode 'Simon'\n");
          break;
        case 'i':
          mode=opt_pal_opt_charset;
          fprintf(stderr, "Selected mode 'opt_pal_opt_charset'\n");
          break;
        case 'o':
          mode=other;
          fprintf(stderr, "Selected mode 'Other'\n");
          break;
        default:
          fprintf(stderr, "Unrecognized mode, defaulting to 'Da Fox'\n");
          break;
        }
      } */
    char * filename = new char [14];
    unsigned long int uncompressedbytes=0;
    unsigned long int rlecompressedbytes=0;
    unsigned long int compressedbytes=0;
    memcpy(filename, "00000001.png",13);
    bool done=false;
    FILE *op = fopen(argv[1], "wb");

    //Output FileMagic
    char *FileMagic = new char [16];
    FileMagic[00]='T';
    FileMagic[01]='M';
    FileMagic[02]='V';
    FileMagic[03]='P';
    FileMagic[04]= 1;
    FileMagic[05]= 0;
    FileMagic[06]= 0;
    FileMagic[07]= 0;
    FileMagic[ 8]= 1;
    FileMagic[ 9]= 0;
    FileMagic[10]= 0;
    FileMagic[11]= 0;
    FileMagic[12]= 0;
    FileMagic[13]= 0;
    FileMagic[14]= 0;
    FileMagic[15]= 0;
    fwrite(FileMagic, 1, 16, op);
    delete FileMagic;
    FILE *raw_wav_file = fopen(argv[2], "rb");
    //FILE *debug = fopen("debug", "wb");
    unsigned long int palettesize=48;
    unsigned long int frames=0;
    if(op!=NULL) {
      // Load custom font
      FILE *ffont = fopen("blocks.fon","rb");
      if(ffont==NULL) {
        fprintf(stderr, "Failed to load font, sorry!\nmake sure a file called 'blocks.fon' is in the current dir\n");
        fclose(op);
        return EXIT_FAILURE;
        }
      unsigned char *font=new unsigned char[2048];
      compressedbytes=fread(font,1,2048,ffont); // variable reuse ...
      if(compressedbytes!=2048) {
        fprintf(stderr, "error while reading font file (only %i bytes read). make sure it is 2048 bytes long\n",compressedbytes);
        fclose(ffont);
        fclose(op);
        return EXIT_FAILURE;
        }
      fwrite(font,1,2048,op);
      fclose(ffont);
      //extern unsigned long int totals;  //Total bytes
      unsigned long int max_pal_dist = 0;
      if(argc > 3) {
        max_pal_dist = atol(argv[3]);
        }
      unsigned long int max_char_dist = 0;
      if(argc > 4) {
        max_char_dist = atol(argv[4]);
        }
      unsigned char * b800h   = new unsigned char [8000];
      unsigned char * prev    = new unsigned char [8000];
      unsigned char * output  = new unsigned char [2*8000];
      unsigned char * imgdata = new unsigned char [160*100*4];
      unsigned char * palette = new unsigned char [palettesize];
      unsigned char * prevpalette = new unsigned char [palettesize];
      unsigned char * wavdata = new unsigned char [AUDIO_BYTES_PER_FRAME];
      memset(b800h, 0, 8000);
      memset(prev, 0, 8000);
      memset(wavdata, 0, AUDIO_BYTES_PER_FRAME);
      int hax=0;
      switch(mode){
        case thomas:
          hax = 1;
          break;
        case dafox:
          hax = 2;
          break;
        case opt_pal_opt_charset:
          initialize_imagizer(font);
          break;
        }
/*      if(do_init_aalib( argv, argc )==EXIT_FAILURE || !init_caca( argv, 80, 50, 160, 100, hax)) {
        return EXIT_FAILURE;
        } */
      if(!init_lzw()){
        return EXIT_FAILURE;
        }
      get_default_palette(  prevpalette );
      while(!done) {
        FILE *ip = fopen(filename, "rb");

        if(ip!=NULL) {
          //fprintf(stderr, "Processing %s\n",filename);
          /* Get the image data from the PNG. RGB, 24bbp */
          unsigned long int w=0, h=0;
          if(readpngintobuffer( ip, imgdata, &w ,&h) == EXIT_SUCCESS) {
            /* Transfrom the pngdata using libAA \/ libCaCa */
            //fwrite(imgdata,  1,  w*h*3,  op); // for t3h Simon
            if(mode==simon) {
              find_opt_pal(imgdata, palette, prevpalette,w, h,max_pal_dist);
              //********************************************
              //return EXIT_FAILURE;

              pixtotext( (unsigned long int *)imgdata, (unsigned short *)b800h,0);
              }
            else if (mode==thomas) {
              find_opt_pal(imgdata, palette, prevpalette, w, h,max_pal_dist);
              pixtotext( (unsigned long int *)imgdata, (unsigned short *)b800h,1);
              }
            else if (mode==dafox) {
             get_default_palette(palette);
             do_aa_conversion( imgdata, b800h, &w,&h );
             /* FIXME: When LibCaCa supports greyscale conversions LibAA is nolonger needed */
             //pixtotext( (unsigned long int *)imgdata, (unsigned short *)b800h,0);
              }
            else if (mode==opt_pal_opt_charset) {
              find_opt_pal(imgdata, palette, prevpalette, w, h, max_pal_dist);
              //get_default_palette(palette);
              timeval tv_pre;
              timeval tv_post;
              gettimeofday(&tv_pre, NULL);
              imagize(imgdata, palette,b800h,prev,w, h, max_char_dist);
              gettimeofday(&tv_post, NULL);
              fprintf(stderr,"Rendering %s cost %0.4f seconds\n",filename,float((1000000*tv_post.tv_sec+tv_post.tv_usec)-(1000000*tv_pre.tv_sec+tv_pre.tv_usec))/1000000.0);

              /* outputpng //* /
              gettimeofday(&tv_pre, NULL);
              texttopng(b800h,palette);
              gettimeofday(&tv_post, NULL);
              fprintf(stderr,"output to png cost %0.4f seconds\n",float((1000000*tv_post.tv_sec+tv_post.tv_usec)-(1000000*tv_pre.tv_sec+tv_pre.tv_usec))/1000000.0); //*/

              }
            unsigned long int outputlen;
            outputlen=0;

#ifdef RawOutput
            memcpy(prevpalette, palette, palettesize);
            palette6bit(palette);
            fwrite(b800h,  1,  8000,  op);
            fwrite(palette, 1, 48, op);
            if(raw_wav_file!=NULL) {
              fread(wavdata, 1,  AUDIO_BYTES_PER_FRAME, raw_wav_file);
              }
            fwrite(wavdata, 1, AUDIO_BYTES_PER_FRAME, op);
#endif

#ifdef lz77s
            uncompressedbytes+=8000; lz77s(output, op, &outputlen); compressedbytes+=outputlen;
            memcpy(prevpalette, palette, palettesize);
            palette6bit(palette);
            palettesize=48;
            uncompressedbytes+=palettesize; lz77s(palette, op, &outputlen); compressedbytes+=palettesize;
            if(raw_wav_file!=NULL) {
              fread(wavdata, 1,  AUDIO_BYTES_PER_FRAME, raw_wav_file);
              }
            outputlen=AUDIO_BYTES_PER_FRAME;
            uncompressedbytes+=outputlen; lz77s(wavdata, op, &outputlen); compressedbytes+=outputlen;
#endif

#ifdef FullLZW
            diff(op,b800h,prev,output,&outputlen); uncompressedbytes+=8000; rlecompressedbytes+=outputlen;
//            fwrite(b800h,  1,  8000,  debug);
//            fwrite(output,  1,  outputlen,  debug);
            lzw(output,op,&outputlen);
            compressedbytes+=outputlen;
            palettesize=48;
            memcpy(prevpalette, palette, palettesize);

            palette6bit(palette);
            uncompressedbytes+=palettesize;
            lzw(palette,op,&palettesize); // Palette always follows frame
            compressedbytes+=palettesize;
//            fwrite(output,  1,  outputlen,  op);

            /** NOTE: Early abort ** /
            finalize_lzw(op);
            return EXIT_FAILURE; /**/
#endif

            unsigned char *tmp = b800h;
            b800h=prev;
            prev=tmp;
            frames++;
            }
          else {
            fprintf(stderr, "Error: Image unsuitable!\nImages must be 160x140x24bpp\n" );
            delete imgdata;
            delete b800h;
            delete output;
            uninit_caca();
            do_uninit_aalib();
            uninitialize_imagizer();
            done=true;
            }
          /* Now we update filename to the next png file */
          char * numpart = new char[9];
          unsigned long int numval = strtoul(filename,NULL,10);
          snprintf(numpart,9, "%08lu",++numval);
          memcpy(filename,numpart,8);
          delete numpart;
          fclose(ip);
          }
        else {
          delete imgdata;
          delete b800h;
          delete output;
          delete palette;
          delete prevpalette;
          delete wavdata;
//          uninit_caca();
//          do_uninit_aalib();
          uninitialize_imagizer();
          done=true;
          }
        }
      finalize_lzw( op ); compressedbytes+=2;
      fprintf(stderr, "Frames written : %lu = %lu bytes\nAfter RLE : %lu = %.2f%% of original\nAfter LZW : %i bytes = %.2f%% of original\n",\
              frames, uncompressedbytes,rlecompressedbytes, float(rlecompressedbytes)/(float(uncompressedbytes)/100.0), compressedbytes, float(compressedbytes)/(float(uncompressedbytes)/100.0));
      fclose(op);
      if(raw_wav_file!=NULL) {
         fclose(raw_wav_file);
         }
      delete filename;
      }
    else {
      fprintf(stderr, "Error opening output file %s!\n",argv[1]);
      }
    }
  else {
    fprintf(stderr,"Usage: mpngtocaas output_file raw_wav_file [max_pal_dist] [max_char_dist]\n");
    }
  return EXIT_SUCCESS;
  }


/** FROM MODE_THOMAS **/
              // diy conversion, crap++
/*              char r=0, g=0, b=0;
              int fgc=0,bgc=0,p1,p2,p3,p4;
              for(int y=0; y<h; y+=2) {
                for(int x=0; x<w; x+=2 ) {
                  /*r=imgdata[3*x + 3*w*y];
                  g=imgdata[3*x+1+3*w*y];
                  b=imgdata[3*x+2+3*w*y];
                  //fgc=(r<<16)|(g<<8)|b;
                  p1=(r<<16)|(g<<8)|b;
                  //fputc(r,op);fputc(0,op);fputc(0,op);
                  //fputc(0,op);fputc(g,op);fputc(0,op);
                  //fputc(0,op);fputc(0,op);fputc(b,op);
                  r=imgdata[3*x+3+3*w*y];
                  g=imgdata[3*x+4+3*w*y];
                  b=imgdata[3*x+5+3*w*y];
                  p2=(r<<16)|(g<<8)|b;
                  //fgc=(((r<<16)|(g<<8)|b)+fgc)>>1;
                  r=imgdata[3*x + 3*w*(y+1)];
                  g=imgdata[3*x+1+3*w*(y+1)];
                  b=imgdata[3*x+2+3*w*(y+1)];
                  p3=(r<<16)|(g<<8)|b;
                  //bgc=(r<<16)|(g<<8)|b;
                  r=imgdata[3*x+3+3*w*(y+1)];
                  g=imgdata[3*x+4+3*w*(y+1)];
                  b=imgdata[3*x+5+3*w*(y+1)];
                  p4=(r<<16)|(g<<8)|b;
                  //bgc=(((r<<16)|(g<<8)|b)+fgc)>>1;
                  fgc=((p1 ^ p2) >> 1) + (p1 & p2);
                  bgc=((p3 ^ p4) >> 1) + (p3 & p4); */
/*                  p1= ((imgdata[3*x+0+3*w*y]<<16)|(imgdata[3*x+1+3*w*y]<<8)|imgdata[3*x+2+3*w*y]);
                  p2= ((imgdata[3*x+3+3*w*y]<<16)|(imgdata[3*x+4+3*w*y]<<8)|imgdata[3*x+5+3*w*y]);
                  p3= ((imgdata[3*x+0+3*w*(y+1)]<<16)|(imgdata[3*x+1+3*w*(y+1)]<<8)|imgdata[3*x+2+3*w*(y+1)]);
                  p4= ((imgdata[3*x+3+3*w*(y+1)]<<16)|(imgdata[3*x+4+3*w*(y+1)]<<8)|imgdata[3*x+5+3*w*(y+1)]);
                  fgc=((p1 ^ p2) >> 1) + (p1 & p2); /* Possibly not correct, only for greyscale * /
                  bgc=((p3 ^ p4) >> 1) + (p3 & p4);
                  b800h[x+((w*y)>>1)  ]=223;
                  b800h[x+((w*y)>>1)+1]=(RGBto4bc(fgc) | (RGBto4bc(bgc)<<4));//RGBto4bc(fgc)|(RGBto4bc(bgc)<<4);
                  }
                }*/
