//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "UColor.h"
#include "URender.h"
#include "UImage.h"
#include "ULZ77.h"
#include "UHuffman.h"
#include <stdio.h>
#include <string>
#include <dos>
#include "assert.h"
#include "mystream.cpp"

using namespace std;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}

uint8 canvas[80*50*8*8*3];
BITMAPINFO canvasbi;

string GetCurTimeStr()
{
	struct time curtime;
	gettime(&curtime);
	char buf[100];
	snprintf(buf, 100, "%02i:%02i:%02i" , curtime.ti_min, curtime.ti_sec, curtime.ti_hund);
	return string(buf);
}

void DrawRGBImageToScreen(RawRGBImage* image)
{
	for (uint x=0; x < image->GetWidth(); ++x)
		for (uint y=0; y < image->GetHeight(); ++y) {
			canvas[(x+(y*80*8))*3+0] = image->GetPixel(x,image->GetHeight()-y-1).c.b;
			canvas[(x+(y*80*8))*3+1] = image->GetPixel(x,image->GetHeight()-y-1).c.g;
			canvas[(x+(y*80*8))*3+2] = image->GetPixel(x,image->GetHeight()-y-1).c.r;
		}

	canvasbi.bmiHeader.biSize = sizeof(canvasbi.bmiHeader);
	canvasbi.bmiHeader.biWidth = 80*8;
	canvasbi.bmiHeader.biHeight = 50*8;
	canvasbi.bmiHeader.biPlanes = 1;
	canvasbi.bmiHeader.biBitCount = 24;
	canvasbi.bmiHeader.biCompression = BI_RGB;
	canvasbi.bmiHeader.biSizeImage = 0;
	canvasbi.bmiHeader.biClrUsed = 0;
	canvasbi.bmiHeader.biClrImportant = 0;

	// specification sais set bmiColors to NULL, so that is what we do
	*((uint32*)(canvasbi.bmiColors)) = NULL;

	Form1->FormPaint(NULL);
}

void __fastcall TForm1::FormPaint(TObject *Sender)
{
	if (this->Active) {
		HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
		SetDIBitsToDevice(hdc, this->Left + this->Image1->Left + 4, this->Top + this->Image1->Top + 23, 80*8, 50*8, 0, 0, 0, 50*8, canvas, &canvasbi, DIB_RGB_COLORS);
	}
}

string CalcFName(string name, int32 fnum)
{
	char buf[100];
	snprintf(buf, 100, name.c_str(), fnum);

	return string(buf);
}

void DoInput(RawRGBImage &image, int32 frame)
{
	if (Form1->RadioButtonLoadRaw->Checked) {
		int loadnum = 0;
		string fname(Form1->EditLoadRaw->Text.c_str());
		if (fname[fname.size()-1] == '%') {
			fname.pop_back();
			loadnum = frame;
		}
		fname = CalcFName(fname, frame);
		image.LoadFromRAW(fname, 160, 100, loadnum);
	} else if (Form1->RadioButtonLoadPNG->Checked) {
		string fname(Form1->EditLoadPng->Text.c_str());
		fname = CalcFName(fname, frame);
		image.LoadFromPNG(fname);
	}
}

void DoOutPut(TextImage &textimage, int32 frame)
{
	static RawRGBImage image;
/*
	static RawRGBImage image2;

	image2.SetSize(320,320);
	for (int i=0; i < 16; ++i) {
		for (int j=0; j < 16; ++j) {
			for (int k=0; k < 16; ++k) {
				image2.SetPixel((frame-60)*16+j, i*16+k, textimage.pal->GetColor(i));
			}
		}
	}

	image2.SaveToPNG("c:\\debug.png");
	*/
	if (Form1->CheckBoxSaveScreen->Checked || Form1->CheckBoxSavePng->Checked)
		textimage.SaveToRawRGBImage(&image);

	if (Form1->CheckBoxSpecial->Checked)
		Form1->DoSpecialOutput(textimage, image);

	if (Form1->CheckBoxSaveScreen->Checked)
		DrawRGBImageToScreen(&image);

	if (Form1->CheckBoxSavePng->Checked) {
		string fname(Form1->EditSavePng->Text.c_str());
		fname = CalcFName(fname, frame);
		image.SaveToPNG(fname);
	}

	if (Form1->CheckBoxSaveText->Checked) {
		int loadnum = 0;
		string fname(Form1->EditSaveText->Text.c_str());
		if (fname[fname.size()-1] == '%') {
			fname.pop_back();
			loadnum = frame;
		}
		fname = CalcFName(fname, frame);
		FILE *fp;
		fp = fopen(fname.c_str(), "ab");
		fclose(fp);
		fp = fopen(fname.c_str(), "r+b");
		if (fseek(fp, loadnum*(8000+48), SEEK_SET) == 0) {
			fwrite(textimage.data, 1, 8000, fp);
			uint8 oval;
			for (int i=0; i<16; ++i) {
				oval = textimage.pal->GetColor(i).c.r >> 2;
				fwrite(&oval, 1, 1, fp);
				oval = textimage.pal->GetColor(i).c.g >> 2;
				fwrite(&oval, 1, 1, fp);
				oval = textimage.pal->GetColor(i).c.b >> 2;
				fwrite(&oval, 1, 1, fp);
			}
		}
		fclose(fp);
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm1::Button4Click(TObject *Sender)
{
	Memo1->Lines->Add((GetCurTimeStr() + " - starting render of frame").c_str());

	RawRGBImage sourceimage;
	DoInput(sourceimage, 0);

	RawRGBImage fontimage;
//	fontimage.LoadFromPNG("C:\\render\\charmap\\charmap.png");
	fontimage.LoadFromPNG("C:\\render\\charmap\\charmap-block.png");

//	DrawRGBImageToCanvas(&fontimage, Image1->Canvas);

	TextFont font;
	font.LoadFromRGBImage(&fontimage);

//	TPalStandard palcalc;
	TPalMedianCut palcalc;
	TextPal pal;
	palcalc.CalcPal(&sourceimage, &pal);

	TextImage textimage;
	textimage.font = &font;
	textimage.pal = &pal;

	RawRGBImage image;

	Memo1->Lines->Add((GetCurTimeStr() + " - loaded input").c_str());

	TRenderBruteBlock renderer;
	renderer.DoRender(&sourceimage, &textimage);

	Memo1->Lines->Add((GetCurTimeStr() + " - rendered image").c_str());

	DoOutPut(textimage, 0);

//	DrawRGBImageToCanvas(&sourceimage, Image1->Canvas);
//	image.LoadFromRAW("c:\\0001.dll", 179, w);
//	image.SaveToPNG("c:\\debug.png");
//	LZ77_compress2("c:\\export.aap", "c:\\export.int");
//	LZ77_compress("c:\\some.nor", "c:\\some.lz7");
//	LZ77_compress("c:\\export.aap", "c:\\test.lz7");
//	LZ77_decompress("c:\\test.lz7", "c:\\test.un7");
//	LZ77_compress("c:\\test.txt", "c:\\testtxt.lz7");
//	LZ77_decompress("c:\\testtxt.lz7", "c:\\untest.txt");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
{
	HuffCoder coder(10);

	for (int i = 0; i < 1; ++i)
		coder.AddCount(0);
	for (int i = 0; i < 1; ++i)
		coder.AddCount(1);
	for (int i = 0; i < 3; ++i)
		coder.AddCount(2);
	for (int i = 0; i < 5; ++i)
		coder.AddCount(3);
	for (int i = 0; i < 6; ++i)
		coder.AddCount(4);
	for (int i = 0; i < 11; ++i)
		coder.AddCount(5);
	for (int i = 0; i < 13; ++i)
		coder.AddCount(6);

	coder.CalcCodes(4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
	HuffCoder hcoder(0x0200);
	HuffCoder hcoder2(0x10000);

	FILE *fin = fopen("c:\\export.int", "rb");
//	FILE *fin = fopen("c:\\export.aap", "rb");
	uint16 icode;

	while (!feof(fin))
	{
		fread(&icode, 2, 1, fin);
		if (!feof(fin)) {
			hcoder.AddCount(icode);
			if (icode & 0x0100) {
				fread(&icode, 2, 1, fin);
				hcoder2.AddCount(icode);
			};
		};
	};
	fclose(fin);

	hcoder.CalcCodes(16);

	fin = fopen("c:\\export.int", "rb");
	FILE *fout = fopen("c:\\export.huf2", "wb");

	uint8 bitstodo = 0;
	uint32 cout32;
	uint16 cout16;

	for (int i = 1; i <= 16; ++i) {
		for (int n = 0; n <= 0x200; ++n) {
			uint8 len;
			int code = hcoder.GetCode(n, len);
			if (len == i) {
				cout16 = n;
				fwrite(&cout16, 2, 1, fout);
			}
		}
		cout16 = 0xFFFF;
		fwrite(&cout16, 2, 1, fout);
	};

	while (!feof(fin))
	{
		fread(&icode, 2, 1, fin);
		if (!feof(fin)) {
			uint8 len;
			uint32 code = hcoder.GetCode(icode, len);

			cout32 |= code << bitstodo;
			bitstodo += len;

			if (bitstodo >= 16) {
				cout16 = (cout32 & 0xFFFF);
				fwrite(&cout16, 2, 1, fout);
				bitstodo -= 16;
				cout32 >>= 16;
			}

			if (icode & 0x0100) {
				fread(&icode, 2, 1, fin);
				cout32 |= icode << (bitstodo);
				cout16 = (cout32 & 0xFFFF);
				fwrite(&cout16, 2, 1, fout);
				cout32 >>= 16;
			};
		};
	};

	cout16 = (cout32 >> 16);
	fwrite(&cout16, 2, 1, fout);

	fclose(fin);
	fclose(fout);

	Memo1->Clear();
	int offset = 0;
	for (int i = 1; i <= 16; ++i) {
		for (int n = 0; n <= 0x200; ++n) {
			uint8 len;
			int code = hcoder.GetCode(n, len);
			if (len == i) {
				string s;
				char buf[100];

				for (int b = 15; b >= 0; --b)
					s.push_back( (offset >> b)&0x01 ? '1' : '0');

				s.insert(i,"-");

				offset += 0x8000 >> (i-1);

				Memo1->Lines->Add(s.c_str());
			}
		}
	};

	hcoder2.CalcCodes(16);

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

RawRGBImage r_sourceimage;
TextFont r_font;
TextImage r_textimage;

TCalcPallete *r_palcalc = NULL;
TextPal r_pal;

TRenderMethod *r_renderer = NULL;

void __fastcall TForm1::ButtonRenderClick(TObject *Sender)
{
	Memo1->Lines->Add((GetCurTimeStr() + " - starting render of frame").c_str());
	int framenum = StrToIntDef(Form1->EditFrameNum->Text, 0);
	int framecnt = StrToIntDef(Form1->EditFrameCount->Text, 1);

	for (int frame = framenum; frame < framenum+framecnt; ++ frame)
	{
		DoInput(r_sourceimage, frame);

		r_palcalc->CalcPal(&r_sourceimage, &r_pal);

		r_textimage.font = &r_font;
		r_textimage.pal = &r_pal;

		Memo1->Lines->Add((GetCurTimeStr() + " - loaded input").c_str());

		r_renderer->DoRender(&r_sourceimage, &r_textimage);

		Memo1->Lines->Add((GetCurTimeStr() + " - rendered image").c_str());

		DoOutPut(r_textimage, frame);

		Memo1->Lines->Add((GetCurTimeStr() + " - did output").c_str());
	}

	if (CheckBoxUpdateFrameNum->Checked)
		Form1->EditFrameNum->Text = IntToStr(framenum+framecnt);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ComboBoxRenderMethodChange(TObject *Sender)
{
	if (r_renderer != NULL)
		delete r_renderer;
	r_renderer = NULL;
	switch (ComboBoxRenderMethod->ItemIndex) {
		case 0: r_renderer = new TRenderBruteBlock; break;
		case 1: r_renderer = new TRenderSemiBruteBlock; break;
		case 2: r_renderer = new TRenderSimulatedAnnealing; break;
	}
	assert(r_renderer != NULL);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ComboBoxPalleteMethodChange(TObject *Sender)
{
	if (r_palcalc != NULL)
		delete r_palcalc;
	r_palcalc = NULL;
	switch (ComboBoxPalleteMethod->ItemIndex) {
		case 0: r_palcalc = new TPalStandard; break;
		case 1: r_palcalc = new TPalMedianCut; break;
		case 2: r_palcalc = new TPalMedianCutSort(0); break;
		case 3: r_palcalc = new TPalMedianCutSort(250); break;
		case 4: r_palcalc = new TPalMedianCutSort(500); break;
		case 5: r_palcalc = new TPalMedianCutSort(750); break;
		case 6: r_palcalc = new TPalMedianCutSort(1000); break;
	}
	assert(r_palcalc != NULL);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ComboBoxCharsetChange(TObject *Sender)
{
	string fname;
	switch (ComboBoxCharset->ItemIndex) {
		case 0: fname = "C:\\render\\charmap\\charmap.png"; break;
		case 1: fname = "C:\\render\\charmap\\charmap-block.png"; break;
	}

	assert(fname != "");
	
	RawRGBImage fontimage;
	fontimage.LoadFromPNG(fname);

	r_font.LoadFromRGBImage(&fontimage);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
	ComboBoxRenderMethodChange(Sender);
	ComboBoxPalleteMethodChange(Sender);
	ComboBoxCharsetChange(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormDestroy(TObject *Sender)
{
	if (r_renderer != NULL)
		delete r_renderer;
	if (r_palcalc != NULL)
		delete r_palcalc;
}
//---------------------------------------------------------------------------

/* hash should generate value from 0 to 0xFFFF from 4 bytes */
#define lookuphash(a, b, c, d) (((a)<<0)^((b)<<2)^((c)<<6)^((d)<<8))
//#define lookuphash(a, b, c, d) (((a)<<0)^((b)<<8)^((c)<<8)^((d)<<8))
//#define lookuphash(a, b, c, d) (((a)<<0)|((b)<<8))

FILE* ofhandle;
MyInStream* instr;
MyOutStream* outstr;
MyOutStream* tststr;
deque<uint32>* lookup;

uint16 ocode;

uint32 mind;
uint32 mlen;

uint32 nmlen;

uint32 ifpos;
uint32 ofpos;

void __fastcall TForm1::ButtonSpecialResetClick(TObject *Sender)
{
	string ofname =  string(EditSpecial->Text.c_str());

	FILE* ofhandle = fopen(ofname.c_str(), "wb");

	instr = new MyInStream(NULL);
	outstr = new MyOutStream(ofhandle);
	tststr = new MyOutStream(NULL);
	lookup = new deque<uint32>[0x10000];

	nmlen = 0;
	ifpos = 0;
	ofpos = 0;
}

void DrawBorder(RawRGBImage &image, uint32 x, uint32 y, RGBColor col)
{
	for (uint i = 0; i<8; ++i) {
		image.SetPixel(x*8+i,y*8  ,col);
		image.SetPixel(x*8+i,y*8+8,col);
		image.SetPixel(x*8  ,y*8+i,col);
		image.SetPixel(x*8+8,y*8+i,col);
	}
}

void __fastcall TForm1::DoSpecialOutput(TextImage &textimage, RawRGBImage &rgbimage)
{
	static RawRGBImage limage;

	limage.SetSize(80*8, 50*8);
	memcpy(rgbimage.data, limage.data, 80*8* 50*8* 3);
	textimage.SaveToRawRGBImage(&limage);

	instr->DoRead((uint8*)textimage.data, 8000);
	instr->DoRead((uint8*)textimage.pal, 48);

	if (!instr->check(ofpos+8000+48+512))
		return;

	while (instr->check(ifpos+512))
	{
		mlen = 0;
		uint32 lookupind;
		// check for a match (of at least length 4)
		if (instr->check(ifpos+3)) {
			lookupind = lookuphash((*instr)[ifpos], (*instr)[ifpos+1], (*instr)[ifpos+2], (*instr)[ifpos+3]);
			deque<uint32> *clook = &lookup[lookupind];
			while ((clook->size() > 0) && (clook->front()+0x7FFF < ifpos))
				clook->pop_front();
			for (uint i = 0; i < clook->size(); ++i)
			{
				uint32 lind = (*clook)[i];
				uint32 clen = 0;
				while (instr->check(ifpos+clen) && (clen < 255) && ((*instr)[ifpos+clen] == (*instr)[lind+clen]))
					clen++;
				if (clen > 3 && clen > mlen) {
					mlen = clen;
					mind = lind;
				}
			}
		}

		// if no match increase nomatch count (and add lookup)
		if (mlen==0)
		{
			if (instr->check(ifpos+3))
				lookup[lookupind].push_back(ifpos);
			++nmlen;
			++ifpos;
		};
		// if match or nomatch limit exceeded, output nomatch (literal) codes
		if ((mlen!=0 && nmlen!=0) || nmlen == 127)
		{
			outstr->write(nmlen + (1<<7));
			for (uint i = ifpos-nmlen; i < ifpos; ++i) {
				outstr->write((*instr)[i]);
				tststr->write(0);
			}
			nmlen = 0;
		}
		// if match output codes for copy (and add lookups)
		if (mlen!=0) {
			mind = ifpos - mind;
			outstr->write(mind >> 8);
			outstr->write(mind & 0xFF);
			outstr->write(mlen);
			while (instr->check(ifpos) && (mlen > 0)) {
				tststr->write(1);
				if (instr->check(ifpos+3)) {
					lookupind = lookuphash((*instr)[ifpos], (*instr)[ifpos+1], (*instr)[ifpos+2], (*instr)[ifpos+3]);
					lookup[lookupind].push_back(ifpos);
				}
				++ifpos;
				--mlen;
			}
		}
	}

	for (uint i = 0; i < 4000; ++i) {
		uint x = i % 80;
		uint y = i / 80;

		bool b1 = (*tststr)[ofpos++];
		bool b2 = (*tststr)[ofpos++];
		RGBColor col;
		if (b1 && b2) col = 0xFF0000;
		if (b1 ^  b2) col = 0xFF00FF;
		if (!b1 && !b2) col = 0x0000FF;
		DrawBorder(rgbimage, x,y, col);
	}

	ofpos += 48;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ButtonSpecialEndClick(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

