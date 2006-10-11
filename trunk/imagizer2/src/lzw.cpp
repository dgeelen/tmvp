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

#include "lzw.h"
#include <iostream>
#include <string>
#include <map>
using std::string;
using std::map;

#define CODE_MAX 0xEFFF

map<string, short int> dict;
unsigned long int ffc;
string lzw_s="";
unsigned long int time_to_reset;

bool init_lzw() {
  dict.clear();
  for(unsigned long int i=0 ; i<256 ; i++) {
    string s;
    s.push_back(i);
    dict[s]=i;
    }
  ffc=256;
  lzw_s="";
  return true;
  }

void lzw(unsigned char *input, FILE *op,unsigned long int *outputlen) {
  char c;
  unsigned long int i=0,o=0;
  while(i<*outputlen) {
    c=input[i++];
    if( dict.count(lzw_s+c) > 0 ) { // in_dictionary(string s)
      lzw_s=lzw_s+c;
      }
    else {
      if(ffc<CODE_MAX) {
        dict[lzw_s+c]=ffc++; //add_to_dictionary(s+c);
        }
      short int v=dict[lzw_s];
      fputc(char(v&0xff),op);
      fputc(char((v&0xff00)>>8),op);
      o+=2;
      lzw_s=c;
      }
    }
  if((time_to_reset++) == 768 ) {
    fprintf(stderr,"resetting lzw\n");
    finalize_lzw(op);
    short int v=0xFFFE;
    fputc(char(v&0xff),op);
    fputc(char((v&0xff00)>>8),op);
    init_lzw();
    time_to_reset=0;
    }
  *outputlen=o;
  }
void finalize_lzw(FILE *op){ // outputs 2 additional characters
  if(lzw_s=="") {
    fprintf(stderr,"Omitting empty string from output\n");
    return;
    }
  short int v=dict[lzw_s];
  fputc(char(v&0xff),op);
  fputc(char((v&0xff00)>>8),op);
  }
  /*w = NIL;
  while ( read a character k )
      {
        if wk exists in the dictionary
        w = wk;
        else
          add wk to the dictionary;
          output the code for w;
          w = k;
      } */

