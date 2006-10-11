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
#include <string>
#include <map>
using std::string;
using std::map;

#define CODE_MAX 0xEFFF

map<string, short int> dict;
unsigned long int ffc;
string lzw_s="";
unsigned long int time_to_reset;

  
void finalize_lzw(FILE *op){ // outputs 2 additional characters
  short int v=dict[lzw_s];
  fputc(char(v&0xff),op);
  fputc(char((v&0xff00)>>8),op);
  }

bool init_lzw() {
  dict.clear();
  for(unsigned long int i=0 ; i<256 ; i++) {
    string s= (char*)(&i);
    dict[s]=i;
    }
  ffc=256;
  lzw_s="";
  return true;
  }

int main(int argc, char *argv[]) {
  init_lzw();
  if(argc>2) {
  FILE *ip = fopen(argv[1], "rb");
  FILE *op = fopen(argv[2], "wb");
  unsigned char c;
  unsigned long int br;
  int hax=0;
  while(!(feof(ip))) {
    br=fread(&c,1,1,ip);//input[i++];
    if(c==0xff){hax++;}else{hax=0;}
    if(hax==2){time_to_reset=1234500000;hax=0;}
    if(br==0 && feof(ip)){
      break;
      }
    if( dict.count(lzw_s+char(c)) > 0 ) { // in_dictionary(string s)
      lzw_s=lzw_s+char(c);
      }
    else {
      if(ffc<CODE_MAX) {
        dict[lzw_s+char(c)]=ffc++; //add_to_dictionary(s+c);
        }
      short int v=dict[lzw_s];
      fputc(char(v&0xff),op);
      fputc(char((v&0xff00)>>8),op);
      lzw_s=c;
      }
    if((time_to_reset++) == 1234500000 /* \inf */ ) {
      fprintf(stderr,"resetting lzw\n");
      finalize_lzw(op);
      short int v=0xFFFE;
      fputc(char(v&0xff),op);
      fputc(char((v&0xff00)>>8),op);
      init_lzw();
      time_to_reset=0;
      }
    }
  finalize_lzw(op);
  fclose(ip);
  fclose(op);
  }
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

