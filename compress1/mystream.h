//---------------------------------------------------------------------------

#ifndef MyStreamH
#define MyStreamH

#include <queue>
#include <string>
#include <stdio>
#include "UTypes.h"

class MyInStream {
 private:
	uint8* buffer;
	FILE* fhandle;
	uint32 numread;

	void DoRead();
 public:
	MyInStream(FILE* file);
	~MyInStream();

	uint8 operator[](uint32 index);
	bool check(uint32 index);
};

class MyOutStream {
 private:
	uint8* buffer;
	FILE* fhandle;
	uint32 numwrite;

	void DoWrite();
 public:
	MyOutStream(FILE* file);
	~MyOutStream();

	uint8 operator[](uint32 index);
	void write(uint8 val);
	void flush();
};
//---------------------------------------------------------------------------
#endif
