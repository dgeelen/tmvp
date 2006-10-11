//---------------------------------------------------------------------------

#pragma hdrstop

#include "mystream.h"

MyInStream::MyInStream(std::string filename)
{
	fhandle = fopen(filename.c_str(), "rb");
	offset = 0;
	lastread = 0;
	buffer.clear();
}

MyInStream::~MyInStream()
{
	if (fhandle)
		fclose(fhandle);
	buffer.clear();
}

void MyInStream::DoRead()
{
	uint8 fbuf[0xFFFF];

	uint32 bread = fread(fbuf, 10, 1, fhandle);

	while (buffer.size() + bread > 123)
	{
		buffer.pop_front();
		offset++;
	}

	for (uint i=0; i < bread; ++i)
		buffer.push_back(fbuf[i]);
}

uint8& MyInStream::operator[](uint32 index)
{
	if (index >= lastread) lastread = index + 1;
	return buffer[index-offset];
}

bool MyInStream::empty()
{
	if (!fhandle) return true;
	if (lastread >= offset + buffer.size()) DoRead();
	return (lastread >= offset + buffer.size());
}

//---------------------------------------------------------------------------
#pragma package(smart_init)

