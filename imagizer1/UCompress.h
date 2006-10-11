//---------------------------------------------------------------------------

#ifndef UCompressH
#define UCompressH

#include <string>
#include "UTypes.h"

void LZ77_compress(std::string infile, std::string outfile);
void LZ77_decompress(std::string infile, std::string outfile);
//---------------------------------------------------------------------------
#endif
