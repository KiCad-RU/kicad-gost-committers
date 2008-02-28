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

unit TextToXMLUnit;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom;

procedure TextToXML(StatusBar:TStatusBar;XMLFileName:String;tLines:TMemo);

implementation
procedure TextToXML(StatusBar:TStatusBar;XMLFileName:String;tLines:TMemo);
var
    xmlDoc : TXMLDocument;
    iNode : IXMLNode;
    i:integer;
//    TotalLines:integer;

 procedure ProcessLine(iNode:IXMLNode);
 var ls:string;
    cNode : IXMLNode;
    i:integer;
    Go:boolean;
 begin
  Application.ProcessMessages;
  if tLines.Lines.Count=0 then exit;
  StatusBar.SimpleText:='Creating XML file : '+IntToStr(tLines.Lines.Count);
  ls:=tLines.Lines.Strings[tLines.Lines.Count-1];
  tLines.Lines.Delete(tLines.Lines.Count-1);
  if (ls='GoUP') then
     ProcessLine(iNode.GetPArentNode)
  else
  if (ls='GoDOWN') then
   begin
      ls:=tLines.Lines.Strings[tLines.Lines.Count-1];
      tLines.Lines.Delete(tLines.Lines.Count-1);
      cNode := iNode.AddChild(ls);
      cNode.Text :='';
      while tLines.Lines.Count>0 do
      begin
        ls:=tLines.Lines.Strings[tLines.Lines.Count-1];
        if ((ls='GoUP') or (ls='GoDOWN')) then ProcessLine(cNode)
        else
        begin
          if ((length(ls)>0) and (ls[1]='"')) then
           begin
              ls:=copy(ls,2,length(ls)-2);
              if (cNode.Attributes['Name']<>null) then
                   cNode.Attributes['Name'] := cNode.Attributes['Name']+' '+ls
              else cNode.Attributes['Name']:=ls;
           end
           else
             begin
                cNode.Text :=cNode.Text + ' ' + ls;
             end;
          tLines.Lines.Delete(tLines.Lines.Count-1);
        end;
      end;
   end
   else
   begin
      cNode := iNode.AddChild('value');
      if ((length(ls)>0) and (ls[1]='"')) then
           begin
              ls:=copy(ls,2,length(ls)-2);
           end;
      cNode.Text :=ls;
      ProcessLine(iNode);
   end;
 end;
begin
   try
     xmlDoc := TXMLDocument.Create(nil) ;
     xmlDoc.Active := true;
     iNode := XMLDoc.AddChild('www.lura.sk');
     while tLines.Lines.Count>0 do
       begin
          ProcessLine(iNode);
      end;
     StatusBar.SimpleText:='Saving XML file : '+XMLFileName;
     XMLDoc.SaveToFile(XMLFileName);
   finally
     xmlDoc.Active := false;
   end;
end;
end.
