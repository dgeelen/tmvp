//---------------------------------------------------------------------------



#include "UHuffman.h"
#include <vcl.h> // WTF! needed for NULL???!?!

#include <queue>
#include <algorithm>

#pragma hdrstop

using namespace std;

struct HuffNodeComp : public binary_function<HuffNode*,HuffNode*,bool>
{
	bool operator()(const HuffNode*& n1, const HuffNode*& n2) const {
//		if (n1->weight == n2->weight) return n1->height > n2->height;
		return n1->weight > n2->weight;
	}
};

HuffCoder::HuffCoder(uint32 ncode)
{
	numcodes = ncode;
	node = new HuffNode[ncode];
	for (uint i = 0; i < ncode; ++i)
	{
		node[i].weight = 0;
		node[i].code = i;
		node[i].codelen = 0;
	}
}

HuffCoder::~HuffCoder()
{
	delete[] node;
}

void HuffCoder::AddCount(uint32 code)
{
	node[code].weight++;
}

struct packnode
{
	uint32 weight;
	HuffNode* node;
	uint32 lx, rx;

	bool operator<(const packnode &r) const { return weight < r.weight; };
};

void LimitStuff(vector<vector<packnode>* > &the_q, uint32 i, uint32 j)
{
	packnode pn = (*(the_q[i]))[j];

	if (pn.node == NULL) {
		LimitStuff(the_q, i-1, pn.lx);
		LimitStuff(the_q, i-1, pn.rx);
	} else {
		pn.node->codelen++;
	}
}

/*
 * Package-merge algorithm, calculates codelengths for each input symbol
 * where codelengths are limited by 'limit'
 */
void HuffCoder::CalcCodes(uint32 limit)
{
	vector<vector<packnode>* > q_all;

	vector<packnode>* q_one = new vector<packnode>;
	vector<int> l;

	/* Set q[1] <- p */
	/* Set l <- {0,0,...,0} */
	for (int i = 0; i < numcodes; ++i) {
		packnode pn;
		pn.weight = node[i].weight;
		pn.node = &node[i];
		node[i].codelen = 0;
		if (pn.weight > 0) {
			q_one->push_back(pn);
			l.push_back(0);
		}
	}
	sort(q_one->begin(), q_one->end());
	q_all.push_back(q_one);

	/* for i <- 1 to L - 1 do                  */
	/*   set q[i+1] <- merge(p, package(q[i])) */
	vector<packnode>* q_last = q_one;
	for (int i = 1; i < limit; ++i) {
		vector<packnode>* q_new = new vector<packnode>(q_one->begin(), q_one->end());
		for (int j = 1; j <= q_last->size() / 2; ++j) {
			packnode pn;
			pn.weight = (*q_last)[2*j-2].weight + (*q_last)[2*j-1].weight;
			pn.lx = 2*j-2;
			pn.rx = 2*j-1;
			pn.node = NULL;
			q_new->push_back(pn);
		}
		sort(q_new->begin(), q_new->end());
		q_all.push_back(q_new);
		q_last = q_new;
	};

	(*(q_all[0]))[0];

	/* for j <- 1 to 2n - 2 do */
	for (int j = 0; j < (2*q_one->size())-2; ++j)
	{
		/* expand q[L, j] */
		/* for each leaf in q[L,j], set l(k) <- l(k) + 1 */
		LimitStuff(q_all, limit - 1, j);
	}

	/* free some memory */
	while(q_all.size() > 0)
	{
		q_one = q_all.back();
		delete q_one;
		q_all.pop_back();
	}

	/* calculate the actual codes */
	int offset = 0;
	for (int i = 1; i <= limit; ++i) {
		for (int n = 0; n < numcodes; ++n) {
			if (node[n].codelen == i) {
				uint32 acode = offset >> (limit-i);
				uint32 bcode = 0;
				for (int j = 0; j < i; ++j) {
					uint32 b = acode & 0x01;
					acode >>= 1;
					bcode <<= 1;
					bcode |= b;
				};
				node[n].code = bcode;
				offset += 1 << (limit-i);
			};
		};
	};

}

uint32 HuffCoder::GetCode(uint32 code, uint8 &len)
{
	len = node[code].codelen;
	return node[code].code;
}

//---------------------------------------------------------------------------

#pragma package(smart_init)

