//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "UColor.h"
#include "URender.h"
#include "UImage.h"
#include "ULZ77.h"
#include "UHuffman.h"
#include <stdio>
#include <string>
#include <dos>
#include "assert.h"

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
	if (Form1->CheckBoxSaveScreen->Checked || Form1->CheckBoxSavePng->Checked)
		textimage.SaveToRawRGBImage(&image);

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
		FILE *fp=fopen(fname.c_str(), "wb");
		if (fseek(fp, loadnum*(8000+48), SEEK_SET) == 0) {
			fwrite(textimage.data, 8000, 1, fp);
			fwrite(textimage.pal, 48, 1, fp);
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

