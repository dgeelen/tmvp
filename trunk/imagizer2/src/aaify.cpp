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

#include "aalib.h"
#include <iostream>

aa_context *aacontext = NULL;
aa_palette palette;
aa_renderparams aarendrparms;
unsigned char *framebuffer = NULL;

int do_uninit_aalib() {
  aa_close(aacontext);
  }

int do_init_aalib(char * argv[], int argc ) {
  aa_defparams.width=80;
  aa_defparams.height=50;
  aarendrparms.bright=atoi(argv[2]) ; //-128;
  aarendrparms.contrast=atoi(argv[3]);//64;
  aarendrparms.gamma=1.0;
  aarendrparms.inversion=0;
  aarendrparms.randomval=0;
  aarendrparms.dither=AA_FLOYD_S;
  if (!aa_parseoptions (&aa_defparams, NULL, &argc, argv) ) { // || argc != 1) {  /* Parse command line options and output the help text.  */
      printf ("%s", aa_help);
      return EXIT_FAILURE;
    }
  aa_defparams.width=80;
  aa_defparams.height=50;
  aa_defparams.supported=AA_NORMAL_MASK | AA_DIM_MASK | AA_BOLD_MASK; // AA_EXTENDED / AA_EIGHT

  aacontext = aa_init(&mem_d,&aa_defparams,NULL); //aa_init(&mem_d,aahwp,NULL);
  if(aacontext == NULL) {
    fprintf(stderr,"Cannot initialize AA-lib. Sorry\n");
    return EXIT_FAILURE;
    }
  framebuffer = aa_image(aacontext);
  if(framebuffer==NULL) {
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
  }

int do_aa_conversion(unsigned char *imgdata, unsigned char *b800h, unsigned long int *w, unsigned long int *h) {
  if(framebuffer!=NULL) {
    int aawidth=aa_imgwidth(aacontext);
    int aaheight=aa_imgheight(aacontext);

    //Copy Image into AA_BUF
    for(int y=0; y<(*h); y++) {
      for(int x=0; x< (*w); x++) {
        framebuffer[x + aawidth*y]= imgdata[x*3+(*w)*3*y]; // this is ok because AALib uses a 1byte greyscale frame buffer
                                                           // and images should thus be greyscaled. Also this is used in mode Da Fox
                                                           // which has been fixed to use 3Bpp
        }
      }
    int txtwidth=aa_scrwidth (aacontext);//w>>1;  // should be(come) 80
    int txtheight=aa_scrheight (aacontext); //h>>1; // should be(come) 50
    (*w)=txtwidth;
    (*h)=txtheight;
    aa_render (aacontext,& aarendrparms, 0, 0, aa_scrwidth (aacontext), aa_scrheight (aacontext));

    unsigned char * aa_img=aa_text(aacontext);
    unsigned char * aa_att=aa_attrs(aacontext);

    /*** Convert AA_BUF to AA_IMAGE (aka: VIDMEM_$B800) ***/
    unsigned char c,t;
    for(int y=0; y<txtheight; y++) {
      for(int x=0; x< txtwidth; x++) {
        c=aa_att[x + txtwidth*y];
        t=aa_img[x + txtwidth*y];
        switch(c){
          case AA_NORMAL:
            c= (unsigned char)0x07;
            break;
          case AA_DIM:
            c= (unsigned char)0x08;
            break;
          case AA_BOLD:
            c= (unsigned char)0x0F;
            break;
          case AA_BOLDFONT:
            c= (unsigned char)0x0F;
            break;
          case AA_REVERSE:
            c= (unsigned char)0x70;
            break;
          case AA_SPECIAL:
            c= (unsigned char)0x17;
            break;
          default:
            c= (unsigned char)0x47;
            break;
          }
        /*fputc(t, op );
        fputc(c, op ); */
        b800h[2*x+txtwidth*y*2    ]=t;
        b800h[2*x+txtwidth*y*2 + 1]=c;
        }
      }
    }
  return EXIT_SUCCESS;
  }
