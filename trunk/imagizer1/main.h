//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TImage *Image1;
	TButton *ButtonRender;
	TComboBox *ComboBoxRenderMethod;
	TComboBox *ComboBoxPalleteMethod;
	TComboBox *ComboBoxCharset;
	TMemo *Memo1;
	TButton *Button4;
	TButton *Button5;
	TButton *Button6;
	TGroupBox *GroupBoxInPut;
	TRadioButton *RadioButtonLoadRaw;
	TRadioButton *RadioButtonLoadPNG;
	TEdit *EditLoadRaw;
	TEdit *EditLoadPng;
	TGroupBox *GroupBoxOutPut;
	TCheckBox *CheckBoxSaveScreen;
	TCheckBox *CheckBoxSaveText;
	TCheckBox *CheckBoxSavePng;
	TEdit *EditSaveText;
	TEdit *EditSavePng;
	TLabeledEdit *EditFrameNum;
	TLabeledEdit *EditFrameCount;
	TCheckBox *CheckBoxUpdateFrameNum;
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall Button6Click(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall ButtonRenderClick(TObject *Sender);
	void __fastcall ComboBoxRenderMethodChange(TObject *Sender);
	void __fastcall ComboBoxPalleteMethodChange(TObject *Sender);
	void __fastcall ComboBoxCharsetChange(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
