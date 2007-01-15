//---------------------------------------------------------------------------
#include "../common/UTypes.h"
#include <string>
#include <stdio.h>
#include <assert.h>

#pragma hdrstop
//---------------------------------------------------------------------------

using namespace std;

#pragma argsused
#define BUFSIZE (32*1024)
int main(int argc, char* argv[]) {
  string ifname = "-";
  string ofname = "-";
  if (argc > 1) ifname = argv[1];
  if (argc > 2) ofname = argv[2];
  FILE* ifhandle = (ifname == "-") ? stdin  : fopen(ifname.c_str(), "rb");
  FILE* ofhandle = (ofname == "-") ? stdout : fopen(ofname.c_str(), "wb");
  uint8 buf[BUFSIZE];
  uint8 t[256][3*8];
  for(uint32 i = 0 ; i < 256 ; ++i) {
    for(uint32 j = 0 ; j < 8 ; ++j) {
      if(i&(0x80>>j)) {
        for(uint32 k = 0; k < 3; ++k) {
          t[i][j*3+k]=255;
          }
      }
      else {
        for(uint32 k = 0; k < 3; ++k) {
          t[i][j*3+k]=0;
          }
      }
    }
  }
  uint32 read;
  while (!feof(ifhandle)) {
    read = fread(buf, 1, BUFSIZE, ifhandle);
    for (uint i = 0; i < read; ++i) {
      fwrite(t[buf[i]], 1, 3*8, ofhandle);
      }
  }

  if (ifname != "-") fclose(ifhandle);
  if (ofname != "-") fclose(ofhandle);
  return EXIT_SUCCESS;
}
