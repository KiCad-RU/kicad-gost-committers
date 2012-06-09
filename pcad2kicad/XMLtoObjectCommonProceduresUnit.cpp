/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2008, 2012 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2012 KiCad Developers, see CHANGELOG.TXT for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file XMLtoObjectCommonProceduresUnit.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <XMLtoObjectCommonProceduresUnit.h>

/*
unit XMLToObjectCommonProceduresUnit;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom;


type

function StrToDoublePrecisionUnits(s:string;axe:char):double;
function StrToIntUnits(s:string;axe:char):integer;
function GetAndCutWordWithMeasureUnits(var i,DefaultMeasurementUnit:string):string;
function StrToInt1Units(s:string):integer;
function ValidateName(n:string):string;
procedure SetWidth(t,DefaultMeasurementUnit:string;var width:integer);
procedure SetPosition(t,DefaultMeasurementUnit:string;var x,y:integer);
procedure SetDoublePrecisionPosition(t,DefaultMeasurementUnit:string;var x,y:double);
Procedure SetTextParameters(iNode:IXMLNode;var tv:HTextValue;DefaultMeasurementUnit:string);
procedure SetFontProperty(iNode:IXMLNode;var tv:HTextValue;DefaultMeasurementUnit:string);
function FindPinMap(iNode:IXMLNode):IXMLNode;
procedure CorrectTextPosition(var value:HTextValue;Rotation:integer);
function GetWord(var s:string):string;

var  ActualConversion:string;

implementation



function GetWord(var s:string):string;
begin
  result:='';
  s:=TrimLeft(s);
  if (length(s)=0) then exit;
  if ((s[1]='"')) then
  begin
    result:=result+s[1];
    s:=copy(s,2);      // remove Frot apostrofe
    while ((length(s)>0) and (s[1]<>'"')) do
      begin
            result:=result+s[1];
            s:=copy(s,2);
      end;
    if ((length(s)>0) and (s[1]='"')) then
      begin
            result:=result+s[1];
            s:=copy(s,2);   // remove ending apostrophe
      end;
  end
  else
  begin
     while ((length(s)>0) and not(s[1] in [' ','(',')'])) do
     begin
        result:=result+s[1];
        s:=copy(s,2);
     end;
  end;
 result:=trim(result);
//  if (length(result)>2) then
//  begin
//    if ((result[1]='"') and (result[2]='(')) then result[2]:='_';
//    if ((result[length(result)-1]=')') and (result[length(result)]='"')) then result[length(result)-1]:='_';
//  end;
end;
*/

wxString GetWord(wxString *s) {
    wxString result = wxEmptyString;
    (*s) = s->Trim(false);
    if (s->Len() == 0) return result;
    if ((*s)[0] == '"') {
        result += (*s)[0];
        *s = s->Mid(1); // remove Frot apostrofe
        while (s->Len() > 0 && (*s)[0] != '"') {
            result += (*s)[0];
            *s = s->Mid(1);
        }

        if (s->Len() > 0 && (*s)[0] == '"') {
            result += (*s)[0];
            *s = s->Mid(1); // remove ending apostrophe
        }
    }
    else {
        while (s->Len() > 0 && !( (*s)[0] == ' ' || (*s)[0] == '(' || (*s)[0] == ')' )) {
            result += (*s)[0];
            *s = s->Mid(1);
        }
    }

    result.Trim(true);
    result.Trim(false);

    return result;
}

/*
function FindPinMap(iNode:IXMLNode):IXMLNode;
var n:string;
    lNode:IXMLNode;
begin
   result:=nil;
   lNode:=iNode.ChildNodes.FindNode('attachedPattern');
   if Assigned(lNode) then
                      result:=lNode.ChildNodes.FindNode('padPinMap');
end;



//Alexander Lunev modified (begin)
function StrToDoublePrecisionUnits(s:string;axe:char):double;
var ls:string;
    i:double;
    precision:integer;
    u:char;
begin
  ls:=trim(s);
  precision:=1;
  if ActualConversion='PCB' then precision:=10;
  if ActualConversion='SCH' then precision:=1;
  try
     if length(ls)>0 then
     begin
       u:=ls[length(ls)];
       while (length(ls)>0) and not(ls[length(ls)] in ['.',',','0'..'9']) do ls:=copy(ls,1,length(ls)-1);
       while (length(ls)>0) and not(ls[1] in ['-','+','.',',','0'..'9']) do ls:=copy(ls,2);
       if (pos(',',ls)>0) then DecimalSeparator:=',' else DecimalSeparator:='.';
       if (u='m') then
          i:=StrToFloat(ls)*precision/0.0254
       else
          i:=StrToFloat(ls)*precision;
     end
     else
       i:=0;
  except
     i:=0;
  end;
  if (((ActualConversion='PCB') or (ActualConversion='SCH')) and (axe='Y')) then result:=-i
  else result:=i; // Y axe is mirrored in compare with PCAD
end;

function StrToIntUnits(s:string;axe:char):integer;
begin
  result:=Round(StrToDoublePrecisionUnits(s, axe));
end;
//Alexander Lunev modified (end)


function GetAndCutWordWithMeasureUnits(var i,DefaultMeasurementUnit:string):string;
var s1,s2:String;
begin
    i:=TrimLeft(i);
    result:='';
    // value
    while ((length(i)>0) and (i[1]<>' ')) do
           begin
            result:=result+i[1];
            i:=copy(i,2);
           end;
    i:=TrimLeft(i);
    // if there is also measurement unit
    while ((length(i)>0) and (i[1] in ['a'..'z','A'..'Z'])) do
       begin
            result:=result+i[1];
            i:=copy(i,2);
       end;
    // and if not, add default....
    if ((length(result)>0) and (result[length(result)] in ['.',',','0'..'9'])) then
           result:=result+DefaultMeasurementUnit;
end;


function StrToInt1Units(s:string):integer;
var precision:integer;
begin
  precision:=10;
  //if ActualConversion='PCB' then precision:=10;
  //if ActualConversion='SCH' then precision:=1;
  if (pos(',',s)>0) then DecimalSeparator:=',' else DecimalSeparator:='.';
  result:=Round(StrToFloat(s)*precision);
end;


function ValidateName(n:string):string;
var i:integer;
    o:string;
begin
   o:='';
   for i:=1 to length(n) do
    begin
       if (n[i] in [' ']) then o:=o+'_' else o:=o+n[i];
    end;
    result:=o;
end;

//Alexander Lunev added (begin)
procedure SetWidth(t,DefaultMeasurementUnit:string;var width:integer);
begin
    width:=StrToIntUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),' ');
end;
//Alexander Lunev added (end)

procedure SetHeight(t,DefaultMeasurementUnit:string;var height:integer);
begin
    height:=StrToIntUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),' ');
end;

procedure SetPosition(t,DefaultMeasurementUnit:string;var x,y:integer);
begin
    X:=StrToIntUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),'X');
    Y:=StrToIntUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),'Y');
end;

//Alexander Lunev added (begin)
procedure SetDoublePrecisionPosition(t,DefaultMeasurementUnit:string;var x,y:double);
begin
    X:=StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),'X');
    Y:=StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),'Y');
end;
//Alexander Lunev added (end)


procedure SetTextParameters(iNode:IXMLNode;var tv:HTextValue;DefaultMeasurementUnit:string);
var tNode:IXMLNode;
begin
     tNode:=iNode.ChildNodes.FindNode('pt');
        if Assigned(tNode) then SetPosition(tNode.Text,DefaultMeasurementUnit,tv.TextPositionX,tv.TextPositionY);
     tNode:=iNode.ChildNodes.FindNode('rotation');
        if Assigned(tNode) then tv.TextRotation:=StrToInt1Units(TrimLeft(tNode.Text));
        tv.TextIsVisible:=1;
     tNode:=iNode.ChildNodes.FindNode('isVisible');
        if Assigned(tNode) then
                if trim(tNode.Text)='True' then tv.TextIsVisible:=1 else  tv.TextIsVisible:=0;
     tNode:=iNode.ChildNodes.FindNode('textStyleRef');
        if Assigned(tNode) then  SetFontProperty(tNode,tv,DefaultMeasurementUnit);
end;


procedure SetFontProperty(iNode:IXMLNode;var tv:HTextValue;DefaultMeasurementUnit:string);
var n:string;
begin
       n:=iNode.Attributes['Name'];
       while (iNode.NodeName<>'www.lura.sk') do iNode:=iNode.ParentNode;
       iNode:=iNode.ChildNodes.FindNode('library');
       if Assigned (iNode) then iNode:=iNode.ChildNodes.FindNode('textStyleDef');
       if Assigned(iNode) then
             begin
               while (trim(iNode.Attributes['Name'])<>n) do iNode:=iNode.NextSibling;
               if Assigned(iNode) then
                    begin
                    iNode:=iNode.ChildNodes.FindNode('font');
                    if Assigned(iNode) then
                      begin
                       if (iNode.ChildNodes.FindNode('fontHeight')<>nil) then
                            ////SetWidth(iNode.ChildNodes.FindNode('fontHeight').Text,DefaultMeasurementUnit,tv.TextHeight);
                            // Fixed By Lubo, 02/2008
                            SetHeight(iNode.ChildNodes.FindNode('fontHeight').Text,DefaultMeasurementUnit,tv.TextHeight);
                        if (iNode.ChildNodes.FindNode('strokeWidth')<>nil) then
                            SetWidth(iNode.ChildNodes.FindNode('strokeWidth').Text,DefaultMeasurementUnit,tv.TextstrokeWidth);
                      end;
                    end;
             end;

end;


procedure CorrectTextPosition(var VAlue:HTextValue;Rotation:integer);
begin
     Value.CorrectedPositionX:=Value.TextPositionX;
     Value.CorrectedPositionY:=Value.TextPositionY;
     Value.CorrectedPositionY:=Value.CorrectedPositionY-Round(value.TextHeight/3);
     Value.CorrectedPositionX:=Value.CorrectedPositionX+Round((length(Value.text)/1.4)*(Value.TextHeight/1.8));
     if (Rotation = 900) then
        begin
           VAlue.CorrectedPositionX:=-Value.TextPositionY;
           Value.CorrectedPositionY:=Value.TextPositionX;
           VAlue.CorrectedPositionX:=VAlue.CorrectedPositionX+Round(Value.TextHeight/3);
           Value.CorrectedPositionY:=Value.CorrectedPositionY+Round((length(Value.text)/1.4)*(Value.TextHeight/1.8));
        end;
     if (Rotation = 1800) then
        begin
           VAlue.CorrectedPositionX:=-VAlue.TextPositionX;
           VAlue.CorrectedPositionY:=-VAlue.TextPositionY;
           Value.CorrectedPositionY:=Value.CorrectedPositionY+Round(Value.TextHeight/3);
           VAlue.CorrectedPositionX:=VAlue.CorrectedPositionX-Round((length(VAlue.text)/1.4)*(VAlue.TextHeight/1.8));
        end;
     if (Rotation = 2700) then
        begin
           VAlue.CorrectedPositionX:=Value.TextPositionY;
           Value.CorrectedPositionY:=-Value.TextPositionX;
           VAlue.CorrectedPositionX:=VAlue.CorrectedPositionX+Round(Value.TextHeight);
           Value.CorrectedPositionY:=Value.CorrectedPositionY-Round((length(Value.text)/3.4)*(Value.TextHeight/1.8));
        end;
end;


end.
*/

wxXmlNode *FindNode(wxXmlNode *child, wxString tag) {
    while (child) {
        if (child->GetName() == tag) return child;

        child = child->GetNext();
    }

    return NULL;
}
