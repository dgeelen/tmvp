//---------------------------------------------------------------------------

#ifndef UHuffmanH
#define UHuffmanH

#include "../common/UTypes.h"

struct HuffNode {
	uint32 weight;
	uint32 code;
	uint8 codelen;
};

class HuffCoder {
 private:
	uint32 numcodes;
	HuffNode* node;
 public:
	HuffCoder(uint32 ncode);
	~HuffCoder();

	void AddCount(uint32 code);

	void CalcCodes(uint32 limit);

	uint32 GetCode(uint32 code, uint8 &len);
};;

//---------------------------------------------------------------------------
#endif
