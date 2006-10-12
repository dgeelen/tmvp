object Form1: TForm1
  Left = 325
  Top = 140
  Width = 813
  Height = 659
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 13
  object Image1: TImage
    Left = 8
    Top = 120
    Width = 640
    Height = 400
  end
  object CheckBoxUpdateFrameNum: TCheckBox
    Left = 368
    Top = 64
    Width = 97
    Height = 17
    TabOrder = 12
  end
  object ButtonRender: TButton
    Left = 288
    Top = 56
    Width = 75
    Height = 25
    Caption = 'Render'
    TabOrder = 0
    OnClick = ButtonRenderClick
  end
  object ComboBoxRenderMethod: TComboBox
    Left = 656
    Top = 120
    Width = 145
    Height = 21
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 1
    Text = 'Brute Force - Blocks'
    OnChange = ComboBoxRenderMethodChange
    Items.Strings = (
      'Brute Force - Blocks'
      'Brute Force - Any'
      'Simulated Annealing'
      'Smart Blocks 1')
  end
  object ComboBoxPalleteMethod: TComboBox
    Left = 656
    Top = 160
    Width = 145
    Height = 21
    ItemHeight = 13
    ItemIndex = 1
    TabOrder = 2
    Text = 'Median Cut'
    OnChange = ComboBoxPalleteMethodChange
    Items.Strings = (
      'Standard Dos Colors'
      'Median Cut'
      'Median Cut + sort'
      'Median Cut + sort (750)'
      'Median Cut + sort (1000)'
      'Median Cut + sort (1250)'
      'Median Cut + sort (1500)')
  end
  object ComboBoxCharset: TComboBox
    Left = 656
    Top = 200
    Width = 145
    Height = 21
    ItemHeight = 13
    ItemIndex = 1
    TabOrder = 3
    Text = 'Blocks'
    OnChange = ComboBoxCharsetChange
    Items.Strings = (
      'ASCII'
      'Blocks')
  end
  object Memo1: TMemo
    Left = 8
    Top = 528
    Width = 641
    Height = 97
    Lines.Strings = (
      'Memo1')
    TabOrder = 4
  end
  object Button4: TButton
    Left = 656
    Top = 472
    Width = 75
    Height = 25
    Caption = 'Button4'
    TabOrder = 5
    OnClick = Button4Click
  end
  object Button5: TButton
    Left = 656
    Top = 400
    Width = 75
    Height = 25
    Caption = 'Button5'
    TabOrder = 6
    OnClick = Button5Click
  end
  object Button6: TButton
    Left = 664
    Top = 328
    Width = 75
    Height = 25
    Caption = 'Button6'
    TabOrder = 7
    OnClick = Button6Click
  end
  object GroupBoxInPut: TGroupBox
    Left = 8
    Top = 8
    Width = 249
    Height = 105
    Caption = 'Input'
    TabOrder = 8
    object RadioButtonLoadRaw: TRadioButton
      Left = 8
      Top = 24
      Width = 113
      Height = 17
      Caption = 'Raw RGB'
      Checked = True
      TabOrder = 0
      TabStop = True
    end
    object RadioButtonLoadPNG: TRadioButton
      Left = 8
      Top = 48
      Width = 113
      Height = 17
      Caption = 'PNG File'
      TabOrder = 1
    end
    object EditLoadRaw: TEdit
      Left = 89
      Top = 21
      Width = 152
      Height = 21
      TabOrder = 2
      Text = 'c:\mov.raw%'
    end
    object EditLoadPng: TEdit
      Left = 89
      Top = 46
      Width = 152
      Height = 21
      TabOrder = 3
      Text = 'c:\render\pngin\%08i.png'
    end
  end
  object GroupBoxOutPut: TGroupBox
    Left = 392
    Top = 8
    Width = 257
    Height = 105
    Caption = 'OutPut'
    TabOrder = 9
    object CheckBoxSaveScreen: TCheckBox
      Left = 8
      Top = 16
      Width = 97
      Height = 17
      Caption = 'Screen'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object CheckBoxSaveText: TCheckBox
      Left = 8
      Top = 40
      Width = 97
      Height = 17
      Caption = 'Semi-Raw Text'
      TabOrder = 1
    end
    object CheckBoxSavePng: TCheckBox
      Left = 8
      Top = 64
      Width = 97
      Height = 17
      Caption = 'PNG File'
      TabOrder = 2
    end
    object EditSaveText: TEdit
      Left = 103
      Top = 37
      Width = 146
      Height = 21
      TabOrder = 3
      Text = 'c:\render\output.aap%'
    end
    object EditSavePng: TEdit
      Left = 104
      Top = 64
      Width = 145
      Height = 21
      TabOrder = 4
      Text = 'c:\render\pngout\%08i.png'
    end
  end
  object EditFrameNum: TLabeledEdit
    Left = 264
    Top = 96
    Width = 121
    Height = 21
    EditLabel.Width = 69
    EditLabel.Height = 13
    EditLabel.Caption = 'Frame Number'
    LabelPosition = lpAbove
    LabelSpacing = 3
    TabOrder = 10
    Text = '0'
  end
  object EditFrameCount: TLabeledEdit
    Left = 264
    Top = 32
    Width = 121
    Height = 21
    EditLabel.Width = 86
    EditLabel.Height = 13
    EditLabel.Caption = 'Number of Frames'
    LabelPosition = lpAbove
    LabelSpacing = 3
    TabOrder = 11
    Text = '1'
  end
end
