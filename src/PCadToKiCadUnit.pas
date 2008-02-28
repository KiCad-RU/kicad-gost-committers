{*
 * PCadToKiCad, PCad ASCII to KiCad conversion utility
 * Copyright (C) 2007, 2008 Lubo Racko <lubo@lura.sk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *}

unit PCadToKiCadUnit;

{
 PCad ASCII file description:
      www.eltm.ru/store/Altium/PCAD_2006_ASCII.pdf
}

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom,TextToXMLUnit,
  PCBComponents,SCHComponents,ProcessXMLToPCBUnit,ProcessXMLToSCHUnit,LoadInputFileUnit,
  XMLToObjectCommonProceduresUnit;

type
  TPCadToKiCadPCBForm = class(TForm)
    OpenPCadFile: TOpenDialog;
    Button1: TButton;
    StatusBar: TStatusBar;
    XMLDoc: TXMLDocument;
    InputFile: TLabel;
    Label1: TLabel;
    Button2: TButton;
    procedure ButtonPCBClick(Sender: TObject);
    procedure ButtonSCHClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
  public
  end;
                      
var
  PCadToKiCadPCBForm: TPCadToKiCadPCBForm;

implementation

{$R *.dfm}

procedure TPCadToKiCadPCBForm.ButtonPCBClick(Sender: TObject);
var f:Tfilename;
    Lines:tMemo;
    PCB:THPCB;
begin
 OpenPCadFile.Filter:='PCad PCB Board ASCII |*.pcb|PCad PCB Library ASCII |*.lia';
 if OpenPCadFile.Execute then
  begin
    ActualConversion:='PCB';
    f:=OpenPCAdFile.FileName;
    f:=AnsiUpperCase(f);
    InputFile.Caption:=f;
    Lines:=LoadInputFile(Application.MainForm,StatusBar,f);
    TextToXML(StatusBar,ChangeFileExt(f,'.XML'),Lines);
    PCB:=ProcessXMLToPCBLib(StatusBar,ChangeFileExt(f,'.XML'));
    StatusBar.SimpleText:='Generating output file.... ';
    if ExtractFileExt(f) = '.LIA' then PCB.WriteToFile(ChangeFileExt(f,'.MOD'),'L');
    if ExtractFileExt(f) = '.PCB' then PCB.WriteToFile(ChangeFileExt(f,'.BRD'),'P');
    StatusBar.SimpleText:='Done.';
    Lines.free;
    PCB.Free;
    ActualConversion:='';
  end;
end;

procedure TPCadToKiCadPCBForm.ButtonSCHClick(Sender: TObject);
var f:Tfilename;
    Lines:tMemo;
    SCH:THSCH;
begin
 OpenPCadFile.Filter:='PCad SCH Schematics ASCII |*.sch|PCad SCH Library ASCII |*.lia';
 if OpenPCadFile.Execute then
  begin
    ActualConversion:='SCH';
    if ExtractFileExt(f) = '.LIA' then ActualConversion:='SCHLIB';
    f:=OpenPCAdFile.FileName;
    f:=AnsiUpperCase(f);
    InputFile.Caption:=f;
    Lines:=LoadInputFile(Application.MainForm,StatusBar,f);
    TextToXML(StatusBar,ChangeFileExt(f,'.XML'),Lines);
    SCH:=ProcessXMLToSCH(StatusBar,ChangeFileExt(f,'.XML'));
    StatusBar.SimpleText:='Generating output file.... ';
    if ExtractFileExt(f) = '.LIA' then SCH.WriteToFile(ChangeFileExt(f,'.LIB'),'L');
    if ExtractFileExt(f) = '.SCH' then
       begin // we convert also library for schematics file
         SCH.WriteToFile(ChangeFileExt(f,'.KiCad.LIB'),'L');
         SCH.WriteToFile(ChangeFileExt(f,'.KiCad.SCH'),'S');
       end;
    StatusBar.SimpleText:='Done.';
    Lines.free;
    SCH.Free;
    ActualConversion:='';
  end;
end;


procedure TPCadToKiCadPCBForm.FormCreate(Sender: TObject);
var x,s:string;
    i:integer;

function DoDepth(var s:string):integer;
begin
  s:=TrimLeft(s);
  result:=0;
  if (length(s)>0) then
     if (s[1]='(') then
     begin
        result:=1;
        s:=copy(s,2);
     end;
  if (length(s)>0) then
     if (s[1]=')') then
     begin
        result:=-1;
        s:=copy(s,2);
     end;
end;

begin
  DecimalSeparator:='.';
  InputFile.Caption:='Software version 0.9.05 :-) . Please be tollerant in case of any bug... or not perfect conversion...';
{
//  s:='(wire (line (pt 6600 5200) (pt 5800 5200) (width 10.0) (netNameRef "R\\S\\T\\_\\R\\S\\") )';

  s:='(padShape (layerNumRef 1) (padShapeType Polygon) (sides 8) (rotation 0.0) (outsideDiam 62mil) (shapeOutline';
  i:=0;
  while length(s)>0 do
  begin
    i:=i+DoDepth(s);
    x:=GetWord(s);
  end;
  s:='(pt 15.5mil 31mil)';
  while length(s)>0 do
  begin
    i:=i+DoDepth(s);
    x:=GetWord(s);
  end;
  s:=')';
  while length(s)>0 do
  begin
    i:=i+DoDepth(s);
    x:=GetWord(s);
  end;


  s:=s+IntToStr(i);
  x:=GetWord(s);
}
end;
end.
