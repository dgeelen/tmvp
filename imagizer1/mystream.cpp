//---------------------------------------------------------------------------

#pragma hdrstop

#include "mystream.h"

const uint32 BUFSIZE = 0x20000;  // = 128k
const uint32 BUFMASK = 0x1FFFF;
const uint32 BUFREAD = 0x08000;  // = 32k

MyInStream::MyInStream(FILE* file)
{
	fhandle = file;
	numread = 0;
	buffer = new uint8[BUFSIZE];
}

MyInStream::~MyInStream()
{
	delete [] buffer;
}

void MyInStream::DoRead()
{
//
}

void MyInStream::DoRead(uint8* buf, uint32 num)
{
	for (uint i=0; i < num; ++i)
		buffer[(numread++)&BUFMASK] = buf[i];
}

uint8 MyInStream::operator[](uint32 index)
{
	return buffer[index & BUFMASK];
}

bool MyInStream::check(uint32 index)
{
//	if (!fhandle) return false;
	if (index < numread) return true;
	DoRead();
	return (index < numread);
}


MyOutStream::MyOutStream(FILE* file)
{
	fhandle = file;
	numwrite = 0;
	buffer = new uint8[BUFSIZE];
}

MyOutStream::~MyOutStream()
{
	delete [] buffer;
}

void MyOutStream::DoWrite()
{
	if (fhandle)
		fwrite(buffer, 1, numwrite, fhandle);
	numwrite = 0;
}

uint8 MyOutStream::operator[](uint32 index)
{
	return buffer[index & BUFMASK];
}

void MyOutStream::write(uint8 val)
{
	buffer[numwrite++] = val;
	if (numwrite == BUFSIZE) DoWrite();
}

void MyOutStream::flush()
{
	DoWrite();
}

//---------------------------------------------------------------------------
#pragma package(smart_init)

