object PCadToKiCadPCBForm: TPCadToKiCadPCBForm
  Left = 0
  Top = 0
  Caption = 
    'PCadToKiCad - PCad ASCII to KiCad conversion utility'
  ClientHeight = 212
  ClientWidth = 931
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 120
  TextHeight = 17
  object InputFile: TLabel
    Left = 10
    Top = 156
    Width = 53
    Height = 17
    Caption = 'FileName'
    Color = clBtnFace
    ParentColor = False
  end
  object Label1: TLabel
    Left = 10
    Top = 131
    Width = 100
    Height = 17
    Caption = 'Actual input file :'
    Color = clBtnFace
    ParentColor = False
  end
  object Button1: TButton
    Left = 10
    Top = 10
    Width = 901
    Height = 33
    Caption = 'Load and process PCad ASCII PCB BOARD OR PCB LIBRARY File '
    TabOrder = 0
    OnClick = ButtonPCBClick
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 193
    Width = 931
    Height = 19
    Panels = <>
    SimplePanel = True
  end
  object Button2: TButton
    Left = 10
    Top = 73
    Width = 901
    Height = 33
    Caption = 'Load and process PCad ASCII SCH SCHEMATICS OR SCH LIBRARY File '
    TabOrder = 2
    OnClick = ButtonSCHClick
  end
  object OpenPCadFile: TOpenDialog
    Filter = 'PCad PCB Board ASCII |*.pcb|PCad PCB Library ASCII |*.lia'
    Options = [ofEnableSizing]
    Left = 536
  end
  object XMLDoc: TXMLDocument
    Left = 504
    DOMVendorDesc = 'Open XML'
  end
end
