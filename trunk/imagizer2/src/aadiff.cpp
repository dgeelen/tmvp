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
#include "aadiff.h"

//unsigned long int totals;

unsigned int diffcount(unsigned char *a, unsigned char *b, unsigned int s) {
  unsigned int i = s;
  while(i<4000 && (a[i<<1]!=b[i<<1] || a[1+(i<<1)]!=b[1+(i<<1)] ) ) { //(short int)(a[i<<1])!=(short int)(b[i<<1])) {
    i++;
    }
  return i-s;
  }
unsigned int simicount(unsigned char *a, unsigned char *b, unsigned int s) {
  unsigned int i = s;
  while(i<4000 && a[i<<1]==b[i<<1] && a[1+(i<<1)]==b[1+(i<<1)] ){ //(short int)(a[i<<1])==(short int)(b[i<<1])) {
    i++;
    }
  return i-s;
  }

void diff(FILE *f, unsigned char *a, unsigned char *b, unsigned char *output, unsigned long int * outputlen) {
  unsigned int i = 0,dc=0,sc=0,copystart=0;
  unsigned int s=0; /* If you're feeling adventures try to optimize s out of the loop */

  while(i<4000) {
    copystart=s;
    dc=diffcount(a,b,s); //copies
    dc=min(dc,254);
    s+=dc;
    sc=simicount(a,b,s); //skips
    sc=min(sc,254);
/*      while(sc==1 && dc+sc<=254) { // god dammit it needs diff search as well
      s+=sc;
      dc+=sc;
      sc=simicount(a,b,s); //skips
      }//*/
    while(sc==1 && dc<254) {
      s+=sc;
      dc+=sc;
      sc=diffcount(a,b,s);
      sc=min(sc,254-dc);
      s+=sc;
      dc+=sc;
      sc=simicount(a,b,s); //skips
      sc=min(sc,254);
      } //*/
    output[(*outputlen)]=dc;(*outputlen)++;
    output[(*outputlen)]=sc;(*outputlen)++;
    for(int x=(copystart<<1); x<((copystart+dc)<<1); x++) {
      int cntr=0;
      output[(*outputlen)]=a[x];(*outputlen)++;
      }
    i+=sc+dc;s=i;
    }
  output[(*outputlen)]=0xff;(*outputlen)++;
  output[(*outputlen)]=0xff;(*outputlen)++;
  //totals+=(*outputlen);
  }
