//---------------------------------------------------------------------------

#ifndef MyStreamH
#define MyStreamH

#include <queue>
#include <string>
#include <stdio.h>
#include "UTypes.h"

/**
 * MyInStream - provides limited random access reading into a file
 *
 * USAGE:
 *   -pass the constructor a file handle opened for reading
 *   -use check() to check for eof
 *   -use operator[]() to read data
 *
 * NOTES:
 *   suppose y is the highest value for which check(y) has returned true so far,
 *   then operator[](x) is valid for all x where
 *   (0 <= x)  &&  (y-128k+32k < x <= y)
 *
 *   check(y) should only really return false when y is beyond eof (or possibly when
 *   you're skipping ahead in the file in steps larger than 32k, but just dont do that, k?)
 */
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
	uint32 getmax();
};

/**
 * MyOutStream - provides limited random access writing into a file
 *
 * ehmm.... not really actually, especially not atm
 * it just does a little bit of buffered output for now...
 *
 * USAGE:
 *   -pass the constructor a file handle opened for writing
 *   -use write() to write data
 *   -use operator[]() to inspect written data
 *   -use flush() when you're done
 *
 * NOTES:
 *   suppose y is the number of times write() was called
 *   then operator[](x) is valid for all x where
 *   (0 <= x)  &&  (y-128k <= x < y)
 *
 *   only use flush() when you dont need operator[] anymore,
 *   cause it will invalidate it's results (more precisely,
 *   it will 'reset' the 'y' above to 0)
 */
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



/* const stuff */
const uint32 BUFSIZE = 0x20000;  // = 128k
const uint32 BUFMASK = 0x1FFFF;
const uint32 BUFREAD = 0x08000;  // = 32k

/* inlined stuff */
inline uint8 MyInStream::operator[](uint32 index)
{
	return buffer[index & BUFMASK];
}

inline bool MyInStream::check(uint32 index)
{
	if (index < numread) return true;
	DoRead();
	return (index < numread);
}

inline uint32 MyInStream::getmax()
{
	return numread;
}

inline uint8 MyOutStream::operator[](uint32 index)
{
	return buffer[index & BUFMASK];
}
//---------------------------------------------------------------------------
#endif
