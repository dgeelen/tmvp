//---------------------------------------------------------------------------
#include "../common/UTypes.h"
#include <string>
#include <stdio.h>
#include <assert.h>

#pragma hdrstop
//---------------------------------------------------------------------------

using namespace std;

#pragma argsused
#define BUFSIZE (32*1024*3) // make this a multiple of 3
int main(int argc, char* argv[]) {
  string ifname = "-";
  string ofname = "-";
  if (argc > 1) ifname = argv[1];
  if (argc > 2) ofname = argv[2];
  FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
  FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");
  uint8 buf[BUFSIZE];
  uint32 read;
  while (!feof(ifhandle)) {
    read = fread(buf, 1, BUFSIZE, ifhandle);
    uint32 j=0;
    uint32 k=0;
    for (uint i = 0; i < read; i+=3) {
      k|=((buf[i]&0x80)>>(j++));
      if(j==8) {
        fwrite(&k, 1, 1, ofhandle);
        j=0;
        k=0;
        }
      }
  }

  if (ifname != "-") fclose(ifhandle);
  if (ofname != "-") fclose(ofhandle);
  return EXIT_SUCCESS;
}
