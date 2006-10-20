//---------------------------------------------------------------------------

#pragma hdrstop

#include "mystream.h"

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
	if (!fhandle) return;
	uint32 readind = numread & BUFMASK;
	uint32 toread = BUFSIZE - readind;

	if (toread > BUFREAD) toread = BUFREAD;

	uint32 bread = fread(&buffer[readind], 1, toread, fhandle);

	numread += bread;
}

/* inlined in mystream.h
uint8 MyInStream::operator[](uint32 index)
{
	return buffer[index & BUFMASK];
}


bool MyInStream::check(uint32 index)
{
	if (index < numread) return true;
	DoRead();
	return (index < numread);
}
*/

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
	fwrite(buffer, 1, numwrite, fhandle);
	numwrite = 0;
}

/* inlined in mystream.h
uint8 MyOutStream::operator[](uint32 index)
{
	return buffer[index & BUFMASK];
}
*/

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

