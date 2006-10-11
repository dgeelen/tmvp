//---------------------------------------------------------------------------

#ifndef MyStreamH
#define MyStreamH

#include <queue>
#include <string>
#include <stdio>
#include "UTypes.h"

class MyInStream {
 private:
	std::deque<uint8> buffer;
	uint32 offset;
	FILE* fhandle;
	uint32 lastread;

	void DoRead();
 public:
	MyInStream(std::string filename);
	~MyInStream();

	uint8& operator[](uint32 index);
	bool empty();
};

class MyOutStream {
 private:
	std::deque<uint8> buffer;
	uint32 offset;
 public:
	MyOutStream(std::string filename);
	~MyOutStream();

	uint8& operator[](uint32 index);
};
//---------------------------------------------------------------------------
#endif
