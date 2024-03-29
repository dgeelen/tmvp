//---------------------------------------------------------------------------


#pragma hdrstop

#include <assert.h>
#include "UImage.h"
#include "stdio.h"
#include <limits>
#ifdef WITH_PNG
# include <png.h>
#endif

using namespace std;

RawRGBImage::RawRGBImage()
{
	width = 0;
	height = 0;
	data = NULL;
}

RawRGBImage::RawRGBImage(uint32 awidth, uint32 aheight)
{
	RawRGBImage();
	SetSize(awidth, aheight);
}

RawRGBImage::~RawRGBImage()
{
	delete[] data;
}

void RawRGBImage::SetSize(uint32 awidth, uint32 aheight)
{
	if (width != awidth || height != aheight)
	{
		width = awidth;
		height = aheight;
		if (data != NULL) delete[] data;
		data = new RGBColor[width*height];
	}
}

RGBColor RawRGBImage::GetPixel(uint32 x, uint32 y)
{
	return data[x + y*width];
}

void RawRGBImage::SetPixel(uint32 x, uint32 y, RGBColor val)
{
	data[x + y*width] = val;
}

uint32 RawRGBImage::GetWidth()
{
	return width;
}

uint32 RawRGBImage::GetHeight()
{
	return height;
}

void RawRGBImage::LoadFromFile(string filename)
{
}

#ifdef WITH_PNG
void RawRGBImage::LoadFromPNG(string filename, uint32 awidth, uint32 aheight)
{
	FILE *fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		return ;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,	NULL, NULL);
	if (!png_ptr)
			return ;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			return ;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return ;
	}

	png_init_io(png_ptr, fp);

	png_bytep* row_pointers;

	if (awidth != 0 && aheight != 0)
	{
		SetSize(awidth, aheight);
		row_pointers = (png_bytep*)png_malloc(png_ptr, height*sizeof(png_bytep));
		for (uint i = 0; i < height; i++)
			row_pointers[i] = (png_bytep)&data[i*width];
		png_set_rows(png_ptr, info_ptr, row_pointers);
	}

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, NULL);

	if (awidth != 0 && aheight != 0)
	{
		png_free(png_ptr, row_pointers);
	}
	else
	{
		SetSize(info_ptr->width, info_ptr->height);

		row_pointers = png_get_rows(png_ptr, info_ptr);

		for (uint i = 0; i < height; ++i)
		{
			memcpy(&data[i*width], row_pointers[i], width * 3);
		}
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	fclose(fp);
}
#endif

#ifdef WITH_PNG
void RawRGBImage::SaveToPNG(std::string filename)
{
	FILE *fp = fopen(filename.c_str(), "wb");
	if (!fp) {
		return ;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,	NULL, NULL);
	if (!png_ptr)
			return ;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			return ;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return ;
	}

	png_init_io(png_ptr, fp);

//	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	png_set_IHDR(png_ptr, info_ptr, width, height,
			 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_bytep* row_pointers;

	row_pointers = (png_bytep*)png_malloc(png_ptr, height*sizeof(png_bytep));
	for (uint i = 0; i < height; i++)
		row_pointers[i] = (png_bytep)&data[i*width];
	png_set_rows(png_ptr, info_ptr, row_pointers);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_free(png_ptr, row_pointers);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
}
#endif

void RawRGBImage::LoadFromBMP(string filename)
{
//  TBitmap bitmap;
}

void RawRGBImage::LoadFromRAW(std::string filename, uint32 awidth, uint32 aheight, uint32 framenr)
{
	FILE *fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		return ;
	}

	SetSize(awidth, aheight);

	for (uint i = 0; i <= framenr; ++i)
		fread(data, sizeof(*data), awidth * aheight, fp);

	fclose(fp);
}

void RawRGBImage::LoadFromRAW(FILE* filehandle, uint32 awidth, uint32 aheight)
{
	if (!filehandle) {
		return ;
	}

	SetSize(awidth, aheight);

	fread(data, sizeof(*data), awidth * aheight, filehandle);
}

//---------------------------------------------------------------------------

void TextPal::SetColor(uint8 ind, RGBColor col)
{
	data[ind] = col;
}

uint32 TextPal::FindColorIndex(RGBColor color) {
  uint32 best_dist=std::numeric_limits<uint32>::max();
  uint32 index=0;
  for(uint32 i=0; i<16; ++i) {
    uint32 dist=MRGBDistInt( color, data[i]);
    if(dist<best_dist) {
      best_dist=dist;
      ++index;
    }
  }
  return index;
}

//RGBColor TextPal::GetColor(uint8 ind)
//{
//	return data[ind];
//}

//---------------------------------------------------------------------------

void TextFont::CalcRatios()
{
	for (int chr = 0; chr < 256; ++chr)
	{
		ratio[chr][4] = 0;	//all quads
		for (int sx = 0; sx < 8; ++sx)
			for (int sy = 0; sy < 8; ++sy)
				ratio[chr][4] += (data[chr][sx][sy] ? 1 : 0);

		ratio[chr][0] = 0;	//ne quad
		for (int sx = 0; sx < 4; ++sx)
			for (int sy = 0; sy < 4; ++sy)
				ratio[chr][0] += (data[chr][sx][sy] ? 4 : 0);

		ratio[chr][1] = 0;	//nw quad
		for (int sx = 4; sx < 8; ++sx)
			for (int sy = 0; sy < 4; ++sy)
				ratio[chr][1] += (data[chr][sx][sy] ? 4 : 0);

		ratio[chr][2] = 0;	//se quad
		for (int sx = 0; sx < 4; ++sx)
			for (int sy = 4; sy < 8; ++sy)
				ratio[chr][2] += (data[chr][sx][sy] ? 4 : 0);

		ratio[chr][3] = 0;	//sw quad
		for (int sx = 4; sx < 8; ++sx)
			for (int sy = 4; sy < 8; ++sy)
				ratio[chr][3] += (data[chr][sx][sy] ? 4 : 0);
	}

	for (uint chr = 0; chr < 256; ++chr) {
		uint8 nchr;
		bool ninv;
		uint8 q[4];
		q[0] = 16 * ((chr >> 0) & 3);
		q[1] = 16 * ((chr >> 2) & 3);
		q[2] = 16 * ((chr >> 4) & 3);
		q[3] = 16 * ((chr >> 6) & 3);

		uint32 mindist = 0xFFFFFFFF;

		for (int i = lorval; i <= hirval; ++i)
//		if (((i&127) >= 32) && (i != 127))
		{
			uint32 dist = 0;
			for (int j = 0; j < 4; ++j)
				dist += (GetRatio(i, j) - q[j]) * (GetRatio(i, j) - q[j]);
			if (dist < mindist) {
				mindist = dist;
				nchr = i;
				ninv = false;
			}

			dist = 0;
			for (int j = 0; j < 4; ++j)
				dist += ((64 - GetRatio(i, j)) - q[j]) * ((64 - GetRatio(i, j)) - q[j]);
			if (dist < mindist) {
				mindist = dist;
				nchr = i;
				ninv = true;
			}
		}

		bmap[chr] = nchr;
		imap[chr] = ninv;
	}
}

void TextFont::DisableMap()
{
	for (uint chr = 0; chr < 256; ++chr) {
		bmap[chr] = chr;
		imap[chr] = false;
	}
};

void TextFont::LoadFromRGBImage(RawRGBImage* img)
{
	assert(img->GetWidth() >= 128);
	assert(img->GetHeight() >= 128);

	int chr = 0;
	for (int cy = 0; cy < 128; cy += 8)
	{
		for (int cx = 0; cx < 128; cx += 8)
		{
			for (int sx = 0; sx < 8; ++sx)
			{
				for (int sy = 0; sy < 8; ++sy)
				{
					data[chr][sx][sy] = (img->GetPixel(cx+sx, cy+sy).get_i() != 0);
				}
			}
			++chr;
		}
	}
	CalcRatios();
}

void TextFont::LoadFromRAWFile(std::string filename)
{
	FILE *fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		return ;
	}

	uint8 tbuf[2048];

	fread(tbuf, 1, 2048, fp);
	fclose(fp);

	for (int i = 0; i < 2048; ++i)
	{
		uint8 chr = i / 8;
		uint8 line = i % 8;

		for (int j = 0; j < 8; ++j)
		{
			data[chr][j][line] = (tbuf[i] & 128) ? 1 : 0;
			tbuf[i] <<= 1;
		}
	}

	CalcRatios();
}

void TextFont::SaveToRAWFile(std::string filename)
{
	FILE *fp = fopen(filename.c_str(), "wb");
	if (!fp) {
		return ;
	}

	uint8 tbuf[2048];

	for (int i = 0; i < 2048; ++i)
	{
		uint8 chr = i / 8;
		uint8 line = i % 8;

		tbuf[i] = 0;
		for (int j = 0; j < 8; ++j)
		{
			tbuf[i] <<= 1;
			tbuf[i] |= data[chr][j][line] ? 1 : 0;
		}
	}

	fwrite(tbuf, 1, 2048, fp);

	fclose(fp);
}

uint8 TextFont::GetRatio(uint8 chr, uint8 quad)
{
	return ratio[chr][quad];
}

void TextImage::SetChar(uint16 x, uint16 y, uint8 chr, uint8 fg, uint8 bg)
{
	assert(x < 80);
	assert(y < 50);
	assert(fg < 16);
	assert(bg < 16);

	if (font->imap[chr])
		data[x + y*80] = font->bmap[chr] | (bg << 8) | (fg << 12);
	else
		data[x + y*80] = font->bmap[chr] | (fg << 8) | (bg << 12);
//	data[x + y*80] = chr | (fg << 8) | (bg << 12);
}

void TextImage::SaveToRawRGBImage(RawRGBImage* img)
{
	img->SetSize(80*8, 50*8);
	for (int x = 0; x < 80; ++x) {
		for (int y = 0; y < 50; ++y) {
			uint8 chr = data[x + y*80] & 0xFF;
			RGBColor fg = pal->GetColor((data[x + y*80] >>  8) & 0x0F);
			RGBColor bg = pal->GetColor((data[x + y*80] >> 12) & 0x0F);
			for (int sx = 0; sx < 8; ++sx)
				for (int sy = 0; sy < 8; ++sy)
					if (font->data[chr][sx][sy]) {
						img->SetPixel(x*8 + sx, y*8 + sy, fg);
					} else {
						img->SetPixel(x*8 + sx, y*8 + sy, bg);
					}
		}
	}
}
#pragma package(smart_init)


