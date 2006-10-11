//---------------------------------------------------------------------------

#ifndef ULZ77H
#define ULZ77H

#include <string>
#include "UTypes.h"

void LZ77_compress(std::string infile, std::string outfile);
void LZ77_compress2(std::string infile, std::string outfile);
void LZ77_decompress(std::string infile, std::string outfile);
//---------------------------------------------------------------------------
#endif
