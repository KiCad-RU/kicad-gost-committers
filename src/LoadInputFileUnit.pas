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

unit LoadInputFileUnit;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom,
  XMLToObjectCommonProceduresUnit;

function LoadInputFile(Sender:TWinControl;StatusBar:TStatusBar;InputFileName:string):TMemo;

implementation
function LoadInputFile(Sender:TWinControl;StatusBar:TStatusBar;InputFileName:string):TMemo;
var f:TextFile;
    S,W:String;
    FileLine,i,depth:integer;
    Lines,tLines:TMemo;
    Apostrophe:boolean;

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

function GetLine():string; // SKIP UNCONVERTED LINES
  function LineIsOK(l:string):boolean;
  var i:integer;
  const
    DummyLines: array [1..44] of string =
      (
         //'(padShape (layerType Signal)',
         //'(padShape (layerType Plane)',
         //'(padShape (layerType NonSignal)',
         '(textStyleDisplayTType ',
         '(textStyleAllowTType ',
         '(attr "PadToPadClearance"',
         '(attr "PadToLineClearance"',
         '(attr "LineToLineClearance"',
         '(attr "ViaToPadClearance"',
         '(attr "ViaToLineClearance"',
         '(attr "ViaToViaClearance"',
         '(fieldSetRef ',
         '(attr "SimType"',
         '(attr "SimPins"',
         '(attr "SimNetlist"',
         '(attr "SimField1"',
         '(attr "SimField2"',
         '(attr "SimField3"',
         '(attr "SimField4"',
         '(attr "SimField5"',
         '(attr "SimField6"',
         '(attr "SimField7"',
         '(attr "SimField8"',
         '(attr "SimField9"',
         '(attr "SimField10"',
         '(attr "SimField11"',
         '(attr "SimField12"',
         '(attr "SimField13"',
         '(attr "SimField14"',
         '(attr "SimField15"',
         '(attr "SimField16"',
         '(attr "Part Field 1"',
         '(attr "Part Field 2"',
         '(attr "Part Field 3"',
         '(attr "Part Field 4"',
         '(attr "Part Field 5"',
         '(attr "Part Field 6"',
         '(attr "Part Field 7"',
         '(attr "Part Field 8"',
         '(attr "Part Field 9"',
         '(attr "Part Field 10"',
         '(attr "Part Field 11"',
         '(attr "Part Field 12"',
         '(attr "Part Field 13"',
         '(attr "Part Field 14"',
         '(attr "Part Field 15"',
         '(attr "Part Field 16"'
      );
  begin
    result:=true;
    for i:=1  to 44  do
        if (AnsiPos(DummyLines[i],l)>0) then result:=false;
  end;
begin
  result:='END OF INPUT FILE';
  while ( not eof(f)) do
  begin
          Readln(f,result);
          if LineIsOk(result) then exit;
  end;
  if Eof(f) then result:='END OF INPUT FILE';
end;

begin
     StatusBar.SimpleText:='Opening file : '+InputFileName;
     Lines:=TMemo.Create(Sender);
     Lines.Parent:=Sender;
     Lines.Visible:=False;
     Lines.WordWrap:=False;
     Lines.Clear;
     tLines:=TMemo.Create(Sender);
     tLines.Parent:=Sender;
     tLines.Visible:=False;
     tLines.WordWrap:=False;
     tLines.Clear;
     FileLine:=0;
     AssignFile(f,InputFileName);
     Reset(f);
     depth:=0;
     i:=0;
     s:=GetLine();
     while s<>'END OF INPUT FILE' do
      begin
        Application.ProcessMessages;
        inc(FileLine);
        StatusBar.SimpleText:='Processing input file - actual line : '+IntToStr(FileLine) +'/'+ IntToStr(Lines.Lines.Count)
                                  + s;
        s:=TrimLeft(s);
        //if ((depth>0) or (((depth=0) and (length(s)>0) and (s[1]='(')))) then
        //if (length(s)>0) then
        //begin
        while length(s)>0 do
          begin
             i:=DoDepth(s);
             depth:=depth+i;
             if (i=-1)     then Lines.Lines.Add('GoUP');
             if (i=1)  then Lines.Lines.Add('GoDOWN');
             if (i=0) then
                  begin
                  w:=GetWord(s);
                  if (length(w)>0) then
                  begin
                    if w[1]='-' then w:='_'+w;
                    if w[1]='+' then w:='_'+w;
                      Lines.Lines.Add(w);
                   end;
              end;

          end;
        //end;
        s:=GetLine();
      end;
    StatusBar.SimpleText:='Input file processed  : '+IntToStr(FileLine) +' lines.';
    //AssignFile(f,ChangeFileExt(InputFileName,'.dbg'));//
    //Rewrite(f);                                       //
    while Lines.Lines.Count>0 do
      begin
        StatusBar.SimpleText:='Optimizing  : '+IntToStr(Lines.Lines.Count);
        tLines.Lines.Add(Lines.Lines[Lines.Lines.Count-1]);
       // Writeln(f,Lines.Lines[Lines.Lines.Count-1]);//
        Lines.Lines.Delete(Lines.Lines.Count-1);
        Application.ProcessMessages;
      end;
    //CloseFile(f);//
    Lines.Free;
    Result:=tLines;
end;

end.
