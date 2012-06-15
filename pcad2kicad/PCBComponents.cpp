/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2007, 2008, 2012 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file PCBComponents.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <PCBComponents.h>

/*
unit PCBComponents;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom,
  XMLToObjectCommonProceduresUnit;


type

implementation

function KiCadLayerMask(mask:string;l:integer):string;
{
Sometimes, a mask layer parameter is used.
It is a 32 bits mask used to indicate a layer group usage (0 up to 32 layers).
A mask layer parameter is given in hexadecimal form.
Bit 0 is the copper layer, bit 1 is the inner 1 layer, and so on...(Bit 27 is the Edge layer).
Mask layer is the ORed mask of the used layers
}
function OrMask(m1,m2:char):char;
{I know, stupid implementation, but there is no time to improve it.....}
var i:integer;
    s:string;

function ToInt(c:char):integer;
begin
  if c in ['0'..'9'] then Result:=Byte(c)-Byte('0')
  else  Result:=(Byte(c)-Byte('A')+10);
end;
*/

int ToInt(char c) {
    if (c <= '0' && c >= '9') return (c - '0');
    else return (c - 'A' + 10);
}

/*
begin
   if m1=m2 then Result:=m1
   else
   begin
     i:=ToInt(m1);
     i:=i+ToInt(m2);
     s:=IntToHex(i,1);
     result:=s[1];
   end;
end;
*/

// The former mentioned here that the following implementation needs to be revised
char OrMask(char m1, char m2) {
    wxString s;
    int i;

    if (m1 == m2) return m1;

    i = ToInt(m1);
    i += ToInt(m2);
    s = wxString::Format("%X", i);
    return s[0];
}

/*
var newmask:string;
    i:integer;
begin
    newmask:='00000000';        // default
    if l=0  then newmask:='00000001';
    if l=15 then newmask:='00008000';
    if l=16 then newmask:='00010000';
    if l=17 then newmask:='00020000';
    if l=18 then newmask:='00040000';
    if l=19 then newmask:='00080000';
    if l=20 then newmask:='00100000';
    if l=21 then newmask:='00200000';
    if l=22 then newmask:='00400000';
    if l=23 then newmask:='00800000';
    if l=24 then newmask:='01000000';
    if l=25 then newmask:='02000000';
    if l=26 then newmask:='04000000';
    if l=27 then newmask:='08000000';
    if l=28 then newmask:='10000000';
  for i:=1 to 8 do
  begin
    NewMask[i]:=OrMask(Mask[i],NewMAsk[i]);
  end;
  result:=newmask;
end;
*/

/*
0 Copper layer
1 to 14   Inner layers
15 Component layer
16 Copper side adhesive layer    Technical layers
17 Component side adhesive layer
18 Copper side Solder paste layer
19 Component Solder paste layer
20 Copper side Silk screen layer
21 Component Silk screen layer
22 Copper side Solder mask layer
23 Component Solder mask layer
24 Draw layer (Used for general drawings)
25 Comment layer (Other layer used for general drawings)
26 ECO1 layer (Other layer used for general drawings)
26 ECO2 layer (Other layer used for general drawings)
27 Edge layer. Items on Edge layer are seen on all layers
*/

wxString KiCadLayerMask(wxString mask, int l) {
    wxString newmask;
    int i;
    /* Sometimes, a mask layer parameter is used.
       It is a 32 bits mask used to indicate a layer group usage (0 up to 32 layers).
       A mask layer parameter is given in hexadecimal form.
       Bit 0 is the copper layer, bit 1 is the inner 1 layer, and so on...(Bit 27 is the Edge layer).
       Mask layer is the ORed mask of the used layers */

    newmask = wxT("00000000");        // default
    if (l == 0) newmask = wxT("00000001");
    if (l == 15) newmask= wxT("00008000");
    if (l == 16) newmask= wxT("00010000");
    if (l == 17) newmask= wxT("00020000");
    if (l == 18) newmask= wxT("00040000");
    if (l == 19) newmask= wxT("00080000");
    if (l == 20) newmask= wxT("00100000");
    if (l == 21) newmask= wxT("00200000");
    if (l == 22) newmask= wxT("00400000");
    if (l == 23) newmask= wxT("00800000");
    if (l == 24) newmask= wxT("01000000");
    if (l == 25) newmask= wxT("02000000");
    if (l == 26) newmask= wxT("04000000");
    if (l == 27) newmask= wxT("08000000");
    if (l == 28) newmask= wxT("10000000");

    for (i = 0; i < 8; i++)
        newmask[i] = OrMask(mask[i], newmask[i]);

    return newmask;
}

/*
function FlipLayers(l:Integer):integer;
begin
  result:=l; // no swap default....
  // routed layers
  if l=0 then result:=15;
  if l=15 then result:=0;
  // Silk
  if l=21 then result:=20;
  if l=20 then result:=21;
  //Paste
  if l=19 then result:=18;
  if l=18 then result:=19;
  // Mask
  if l=23 then result:=22;
  if l=22 then result:=23;
end;
*/

int FlipLayers(int l) {
    int result = l; // no swap default....

    // routed layers
    if (l == 0) result = 15;
    if (l == 15) result = 0;

    // Silk
    if (l == 21) result = 20;
    if (l == 20) result = 21;

    //Paste
    if (l == 19) result = 18;
    if (l == 18) result = 19;

    // Mask
    if (l == 23) result = 22;
    if (l == 22) result = 23;

    return result;
}

/*
constructor THNet.Create(iname:string);
begin;
  Name:=iName;
  NetNodes:=TList.Create();
end;
*/


CNetNode::CNetNode() {
    m_compRef = wxEmptyString;
    m_pinRef = wxEmptyString;
}

CNetNode::~CNetNode() {
}

CNet::CNet(wxString iName) {
    m_name = iName;
}

/*
destructor THNet.Free;
begin
   NetNodes.Free;
end;
*/

CNet::~CNet() {
}

/*
constructor THPCBComponent.Create();
begin
   timestamp:=0;
end;
*/

CPCBComponent::CPCBComponent() {
    m_tag = 0;
    m_objType = '?';
    m_PCadLayer = 0;
    m_KiCadLayer = 0;
    m_timestamp = 0;
    m_positionX = 0;
    m_positionY = 0;
    m_rotation = 0;
    InitTTextValue(&m_name);
    m_net = wxEmptyString;
    m_compRef = wxEmptyString;
    m_patGraphRefName = wxEmptyString;
}

CPCBComponent::~CPCBComponent() {
}

void CPCBComponent::WriteToFile(wxFile *f, char ftype) {
}

/*
procedure THPCBComponent.SetPosOffset(x_offs:integer;y_offs:integer);
begin
  PositionX:=PositionX + x_offs;
  PositionY:=PositionY + y_offs;
end;
*/

void CPCBComponent::SetPosOffset(int x_offs, int y_offs) {
    m_positionX += x_offs;
    m_positionY += y_offs;
}

CPCBPadViaShape::CPCBPadViaShape() {
    CPCBComponent::CPCBComponent();
    m_shape = wxEmptyString;
    m_width = 0;
    m_height = 0;
}

CPCBPadViaShape::~CPCBPadViaShape() {
}

/*
constructor THPCBLine.Create();
begin
   inherited;
   ObjType:='L';
end;
*/

CPCBLine::CPCBLine() {
    CPCBComponent::CPCBComponent();
    m_width = 0;
    m_toX = 0;
    m_toY = 0;
    m_objType = 'L';
}

CPCBLine::~CPCBLine() {
}

/*
procedure THPCBLine.SetPosOffset(x_offs:integer;y_offs:integer);
begin
  inherited;
  ToX:=ToX + x_offs;
  ToY:=ToY + y_offs;
end;
*/

void CPCBLine::SetPosOffset(int x_offs, int y_offs) {
    CPCBComponent::SetPosOffset(x_offs, y_offs);
    m_toX += x_offs;
    m_toY += y_offs;
}

/*
procedure THPCBLine.WriteToFile(var f:text;ftype:char);
var     i:integer;
        s,l:string;
 begin
   if fType='L' then // Library
     Writeln(f,'DS '+IntToStr(PositionX)+' '+IntToStr(PositionY)
                +' '+IntToStr(ToX)+' '+IntToStr(ToY)+' '+IntToStr(width)
                +' '+IntToStr(KiCadLayer)); // Position
   if fType='P' then // PCB
   begin
        Writeln(f,'Po 0 '+IntToStr(PositionX)+' '+IntToStr(PositionY)
                +' '+IntToStr(ToX)+' '+IntToStr(ToY)+' '+IntToStr(width));
        if (timestamp=0) then
          Writeln(f,'De '+IntToStr(KiCadLayer)+' 0 0 0 0')
        else Writeln(f,'De '+IntToStr(KiCadLayer)+' 0 0 ' + IntToHex(timestamp, 8) + ' 0');
   end;
 end;
*/

void CPCBLine::WriteToFile(wxFile *f, char ftype) {
    if (ftype == 'L') { // Library
        f->Write(wxString::Format("DS %d %d %d %d %d %d\n", m_positionX, m_positionY,
                 m_toX, m_toY, m_width, m_KiCadLayer)); // Position
    }

    if (ftype == 'P') { // PCB
        f->Write(wxString::Format("Po 0 %d %d %d %d %d\n", m_positionX, m_positionY,
                 m_toX, m_toY, m_width));
        if (m_timestamp == 0)
            f->Write(wxString::Format("De %d 0 0 0 0\n", m_KiCadLayer));
        else f->Write(wxString::Format("De %d 0 0 %8X 0\n", m_KiCadLayer, m_timestamp));
    }
}

/*
//Alexander Lunev modified (begin)
constructor THPCBPolygon.Create();
begin
   inherited;
   ObjType:='Z';
   outline:=TList.Create();
   fill_lines:=TList.Create();
end;
*/

CPCBPolygon::CPCBPolygon() {
    CPCBComponent::CPCBComponent();
    m_width = 0;
    m_objType = 'Z';
}

CPCBPolygon::~CPCBPolygon() {
}

/*
procedure THPCBPolygon.WriteToFile(var f:text;ftype:char);
var     i:integer;
 begin
  if fill_lines.Count>0 then
  begin
   for i:=0 to fill_lines.Count-1 do
    begin
       THPCBLine(fill_lines[i]).WriteToFile(f,ftype);
    end;
  end;

 end;
  //Alexander Lunev modified (end)
*/

void CPCBPolygon::WriteToFile(wxFile *f, char ftype) {
}

/*
//Alexander Lunev added (begin)
procedure THPCBPolygon.WriteOutlineToFile(var f:text;ftype:char);
var     i:integer;
        c:char;
begin
  if outline.Count>0 then
  begin
   Writeln(f,'$CZONE_OUTLINE');
   Writeln(f,'ZInfo ' + IntToHex(timestamp, 8) + ' 0 "' + Net + '"');
   Writeln(f,'ZLayer ' + IntToStr(KiCadLayer));
   Writeln(f,'ZAux ' + IntToStr(outline.Count) + ' E');
   c:='0';
   for i:=0 to outline.Count-1 do
   begin
      if (i=outline.Count-1) then c:='1';
      Writeln(f,'ZCorner ' + IntToStr(THPCBLine(outline[i]).PositionX) + ' ' +
                IntToStr(THPCBLine(outline[i]).PositionY) + ' ' + c);
   end;
   Writeln(f,'$endCZONE_OUTLINE');
  end;
end;
*/

void CPCBPolygon::WriteOutlineToFile(wxFile *f, char ftype) {
}

/*
procedure THPCBPolygon.SetPosOffset(x_offs:integer;y_offs:integer);
var i:integer;
begin
  inherited;

  for i:=0 to fill_lines.Count-1 do
     THPCBLine(fill_lines[i]).SetPosOffset(x_offs, y_offs);

  for i:=0 to outline.Count-1 do
     THPCBLine(outline[i]).SetPosOffset(x_offs, y_offs);
end;
*/

void CPCBPolygon::SetPosOffset(int x_offs, int y_offs) {
    CPCBComponent::SetPosOffset(x_offs, y_offs);
}

CPCBCopperPour::CPCBCopperPour() : CPCBPolygon() {
    CPCBPolygon::CPCBPolygon();
}

CPCBCopperPour::~CPCBCopperPour() {
}


/*
constructor THPCBCutout.Create();
begin
   inherited;
   ObjType:='C';
   outline:=TList.Create();
end;
*/

CPCBCutout::CPCBCutout() {
    CPCBComponent::CPCBComponent();
    m_objType = 'C';
}

CPCBCutout::~CPCBCutout() {
}

/*
procedure THPCBCutout.WriteToFile(var f:text;ftype:char);
begin
  //no operation
end;
//Alexander Lunev added (end)
*/

void CPCBCutout::WriteToFile(wxFile *f, char ftype) {
}

/*
constructor THPCBPad.Create(iname:string);
begin
   ObjType:='P';
   Name.Text:=iName; Hole:=0;
   Shapes:=TList.Create();
end;
*/

CPCBPad::CPCBPad(wxString iName) {
    CPCBComponent::CPCBComponent();
    m_objType = 'P';
    m_number = 0;
    m_hole = 0;
    m_name.text = iName;
}

/*
procedure THPCBPad.WriteToFile(var f:text;ftype:char;r:integer);
var     i:integer;
        s,l,LayerMask,PadType:string;
        lc,ls:integer;
begin
 if fType='P' then // PCB
 begin
     for i:=0 to Shapes.Count-1 do
       begin
         with THPCBPadShape(Shapes.Items[i]) do
          begin
            if ((width>0) or (Height>0)) then // maybe should not to be filtered ????
            begin
             //if (Shape='Oval') then
             //  begin
             //     if (Width<>Height) then s:='O' else s:='C';
             //  end
             //  else if (Shape='Ellipse') then s:='O'
             //            else if (Shape='Rect') then s:='R';
               s:='3' ; // default
               Writeln(f,'Po '+s+' '+IntToStr(self.PositionX)+' '+IntToStr(self.PositionY)
                                +' '+IntToStr(self.PositionX)+' '+IntToStr(self.PositionY)
                                +' '+IntToStr(Height)+' '+IntToStr(self.Hole));
               Writeln(f,'De '+IntToStr(KiCadLayer)+' 1 0 0 0');
            end;
          end;
       end;
 end
 else
 begin    // Library
     lc:=0;ls:=0;
   // Is it SMD pad , or not ?
     for i:=0 to Shapes.Count-1 do
       begin
         with THPCBPadShape(Shapes.Items[i]) do
          begin
            if (Width>0) and (Height>0) then
            begin
              if KiCadLayer=15 then Inc(lc); // Component
              if KiCadLayer=0  then Inc(Ls);  // Cooper
            end;
          end;
       end;
     // And default layers mask
     LayerMask:='00C08001'; //Comp,Coop,SoldCmp,SoldCoop
     PadType:='STD';
     if ((lc=0) or (ls=0)) then
       begin
         if (Hole=0) then
         begin
             PadType:='SMD';
              if (ls>0) then LayerMask:='00440001';
              if (lc>0) then LayerMask:='00888000';
         end
         else
         begin
           if (ls>0) then LayerMask:='00400001';
           if (lc>0) then LayerMask:='00808000';
         end;
       end;
     // Go out
     for i:=0 to Shapes.Count-1 do
       begin
         with THPCBPadShape(Shapes.Items[i]) do
          begin
            if ((width>0) or (Height>0)) then // maybe should not to be filtered ????
            begin
              if (Shape='Oval') then
               begin
                  if (Width<>Height) then s:='O' else s:='C';
               end
               else
                 if (Shape='Ellipse') then s:='O'
                 else if (Shape='Rect') or (Shape='RndRect') then s:='R'
                      else if (Shape='Polygon') then s:='R'; // approximation.....
              Writeln(f,'$PAD');
              Writeln(f,'Sh "'+self.name.Text+'" '+s+' '+IntToStr(Width)+' '+ IntToStr(Height)
                              +' 0 0 '+IntToStr(self.Rotation+r)); // Name, Shape, Xsize Ysize Xdelta Ydelta Orientation
              Writeln(f,'Dr '+IntToStr(self.Hole)+' 0 0'); // Hole size , OffsetX, OffsetY
              LayerMask:=KiCadLayerMask(LayerMask,THPCBPadShape(self.Shapes.Items[i]).KiCadLayer);
              Writeln(f,'At '+PadType+' N '+LayerMask); // <Pad type> N <layer mask>
              Writeln(f,'Ne 0 "'+self.Net+'"'); // Reference
              Writeln(f,'Po '+IntToStr(self.PositionX)+' '+IntToStr(self.PositionY)); // Position
              Writeln(f,'$EndPAD');
            end;
          end;
       end;
 end;
end;
*/

void CPCBPad::WriteToFile(wxFile *f, char ftype, int r) {
    CPCBPadViaShape *padShape;
    wxString s, layerMask, padType;
    int i, lc, ls;

    if (ftype == 'P') { // PCB
        for (i = 0; i < (int)m_shapes.GetCount(); i++) {
            padShape = m_shapes[i];

            if (padShape->m_width > 0 || padShape->m_height > 0) { // maybe should not to be filtered ????
                s = wxT("3") ; // default
                f->Write(wxT("Po ") + s + wxString::Format(" %d %d %d %d %d %d\n", m_positionX, m_positionY,
                         m_positionX, m_positionY, padShape->m_height, m_hole));
                f->Write(wxString::Format("De %d 1 0 0 0\n", m_KiCadLayer));
            }
        }
    }
    else {
        // Library
        lc = 0 ; ls = 0;
        // Is it SMD pad , or not ?
        for (i = 0; i < (int)m_shapes.GetCount(); i++) {
            padShape = m_shapes[i];
            if (padShape->m_width > 0 && padShape->m_height > 0) {
                if (m_KiCadLayer == 15) lc++; // Component
                if (m_KiCadLayer == 0) ls++;  // Cooper
            }
        }

        // And default layers mask
        layerMask = wxT("00C08001"); //Comp,Coop,SoldCmp,SoldCoop
        padType = wxT("STD");
        if (lc == 0 || ls == 0) {
            if (m_hole == 0) {
                padType = wxT("SMD");
                if (ls > 0) layerMask = wxT("00440001");
                if (lc > 0) layerMask = wxT("00888000");
            }
            else {
                if (ls > 0) layerMask = wxT("00400001");
                if (lc > 0) layerMask = wxT("00808000");
            }
        }

        // Go out
        for (i = 0; i < (int)m_shapes.GetCount(); i++) {
            padShape = m_shapes[i];
            if (padShape->m_width > 0 || padShape->m_height > 0) { // maybe should not to be filtered ????
                if (padShape->m_shape == wxT("Oval")) {
                    if (padShape->m_width != padShape->m_height) s = wxT("O");
                    else s = wxT("C");
                }
                else if (padShape->m_shape == wxT("Ellipse")) s = wxT("O");
                else if (padShape->m_shape == wxT("Rect") || padShape->m_shape == wxT("RndRect")) s = wxT("R");
                else if (padShape->m_shape == wxT("Polygon")) s = wxT("R"); // approximation.....

                f->Write(wxT("$PAD\n"));
                f->Write(wxT("Sh \"") + m_name.text + wxT("\" ") + s +
                         wxString::Format(" %d %d 0 0 %d\n",
                             padShape->m_width, padShape->m_height, m_rotation + r)); // Name, Shape, Xsize Ysize Xdelta Ydelta Orientation

                f->Write(wxString::Format("Dr %d 0 0\n", m_hole)); // Hole size , OffsetX, OffsetY

                layerMask = KiCadLayerMask(layerMask, padShape->m_KiCadLayer);
                f->Write(wxT("At ") + padType + wxT(" N ") + layerMask + wxT("\n")); // <Pad type> N <layer mask>
                f->Write(wxT("Ne 0 \"") + m_net + "\"\n"); // Reference
                f->Write(wxString::Format("Po %d %d\n", m_positionX, m_positionY)); // Position
                f->Write(wxT("$EndPAD\n"));
            }
        }
    }
}

/*
destructor THPCBPad.Free;
begin
  Shapes.Free;
end;
*/

CPCBPad::~CPCBPad() {
}

/*
constructor THPCBVia.Create();
begin
   inherited Create('');
   ObjType:='V';
end;
*/

CPCBVia::CPCBVia() : CPCBPad(wxEmptyString) {
    m_objType = 'V';
}

CPCBVia::~CPCBVia() {
}

/*
constructor THPCBText.Create();
begin
   ObjType:='T';
end;
*/

CPCBText::CPCBText() {
    CPCBComponent::CPCBComponent();
    m_objType = 'T';
}

CPCBText::~CPCBText() {
}

/*
procedure THPCBText.WriteToFile(var f:text;ftype:char);
var     i:integer;
        CorrectedPositionX,CorrectedPositionY:Integer;
        visibility,mirrored:Char;
 begin
   if (Name.TextIsVisible=1) then visibility:='V' else visibility:='I';
   if (Name.Mirror=1) then mirrored:='M' else mirrored:='N';

   {Simple, not the best, but acceptable text positioning.....}
   CorrectTextPosition(Name,Rotation);
   //Name.CorrectedPositionX:=PositionX;
   //Name.CorrectedPositionY:=PositionY;
   //if (Rotation = 0) or (Rotation = 1800) or (Rotation = 3600) then
   //     if (Name.Mirror=0) then Name.CorrectedPositionX:=Round(PositionX+(length(Name.text)/1.4)*(Name.TextHeight/2))
   //                   else Name.CorrectedPositionX:=Round(PositionX-(length(Name.text)/1.4)*(Name.TextHeight/2));
   //if (Rotation = 900) or (Rotation = 2700) then Name.CorrectedPositionY:=Round(PositionY-(length(Name.text)/1.4)*(Name.TextHeight/2));

   // Go out
   if fType='L' then // Library component
   begin
     Writeln(f,'T'+IntToStr(Tag)+' '+IntToStr(Name.CorrectedPositionX)+' '+IntToStr(Name.CorrectedPositionY)
               +' '+IntToStr(Round(Name.TextHeight/2))+' '+IntToStr(Round(Name.TextHeight/1.1))
               +' '+IntToStr(Rotation)+' '+IntToStr(Name.TextstrokeWidth)+' '+mirrored+' '+visibility
               +' '+IntToStr(KiCadlayer)+' "'+Name.text+'"'); // ValueString
   end;
   if fType='P' then // Library component
   begin
     if (Name.Mirror=1) then mirrored:='0' else mirrored:='1';
     Writeln(f,'Te "'+Name.text+'"');
     Writeln(f,'Po '+IntToStr(Name.CorrectedPositionX)+' '+IntToStr(Name.CorrectedPositionY)
               +' '+IntToStr(Round(Name.TextHeight/2))+' '+IntToStr(Round(Name.TextHeight/1.1))
               +' '+IntToStr(Name.TextstrokeWidth)+' '+IntToStr(Rotation));
     Writeln(f,'De '+IntToStr(KiCadlayer)+' '+mirrored+' 0 0');
   end;
end;
*/

void CPCBText::WriteToFile(wxFile *f, char ftype) {
    char visibility, mirrored;

    if (m_name.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_name.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    // Simple, not the best, but acceptable text positioning.....
    CorrectTextPosition(&m_name, m_rotation);

    // Go out
    if (ftype == 'L') { // Library component
        f->Write(wxString::Format("T%d %d %d %d %d %d %d ", m_tag, m_name.correctedPositionX, m_name.correctedPositionY,
                 KiROUND(m_name.textHeight / 2), KiROUND(m_name.textHeight / 1.1),
                 m_rotation, m_name.textstrokeWidth) + mirrored + ' ' + visibility +
                 wxString::Format(" %d \"", m_KiCadLayer) + m_name.text + wxT("\"\n")); // ValueString
    }

    if (ftype == 'P') { // Library component
        if (m_name.mirror == 1) mirrored = '0';
        else mirrored = '1';

        f->Write(wxT("Te \"") + m_name.text + wxT("\"\n"));

        f->Write(wxString::Format("Po %d %d %d %d %d %d\n", m_name.correctedPositionX, m_name.correctedPositionY,
                 KiROUND(m_name.textHeight / 2), KiROUND(m_name.textHeight / 1.1),
                 m_name.textstrokeWidth, m_rotation));

        f->Write(wxString::Format("De %d ", m_KiCadLayer) + mirrored + wxT(" 0 0\n"));
    }
}

/*
procedure THPCBText.SetPosOffset(x_offs:integer;y_offs:integer);
begin
  inherited;
  Name.TextPositionX:=Name.TextPositionX + x_offs;
  Name.TextPositionY:=Name.TextPositionY + y_offs;
end;
*/

void CPCBText::SetPosOffset(int x_offs, int y_offs) {
    CPCBComponent::SetPosOffset(x_offs, y_offs);
    m_name.textPositionX += x_offs;
    m_name.textPositionY += y_offs;
}

/*
constructor THPCBArc.Create();
begin
   ObjType:='A';
end;
*/

CPCBArc::CPCBArc() {
    CPCBComponent::CPCBComponent();
    m_objType = 'A';
    m_startX = 0;
    m_startY = 0;
    m_angle = 0;
    m_width = 0;
}

CPCBArc::~CPCBArc() {
}

/*
procedure THPCBArc.WriteToFile(var f:text;ftype:char);
var     i:integer;
 begin
{
 DC ox oy fx fy w  DC is a Draw Circle  DC Xcentre Ycentre Xpoint Ypoint Width Layer
 DA x0 y0 x1 y1 angle width layer  DA is a Draw ArcX0,y0 = Start point x1,y1 = end point
 }
   if fType='L' then // Library component
   begin
     Writeln(f,'DA '+IntToStr(PositionX)+' '+IntToStr(PositionY)+' '+IntToStr(StartX)+
                     ' '+IntToStr(StartY)+' '+IntToStr(Angle)+' '+IntToStr(width)+' ' +IntToStr(KiCadLayer)); // ValueString
   end;
   if fType='P' then // PCB
   begin
        Writeln(f,'Po 2 '+IntToStr(PositionX)+' '+IntToStr(PositionY)
                +' '+IntToStr(StartX)+' '+IntToStr(StartY)+' '+IntToStr(width));
        Writeln(f,'De '+IntToStr(KiCadLayer)+' 0 '+IntToStr(-Angle)+' 0 0');
   end;
end;
*/

void CPCBArc::WriteToFile(wxFile *f, char ftype) {
/*
 DC ox oy fx fy w  DC is a Draw Circle  DC Xcentre Ycentre Xpoint Ypoint Width Layer
 DA x0 y0 x1 y1 angle width layer  DA is a Draw ArcX0,y0 = Start point x1,y1 = end point
*/
    if (ftype == 'L') { // Library component
        f->Write(wxString::Format("DA %d %d %d %d %d %d %d\n", m_positionX, m_positionY, m_startX,
                 m_startY, m_angle, m_width, m_KiCadLayer)); // ValueString
    }

    if (ftype == 'P') { // PCB
        f->Write(wxString::Format("Po 2 %d %d %d %d %d", m_positionX, m_positionY,
                 m_startX, m_startY, m_width));
        f->Write(wxString::Format("De %d 0 %d 0 0\n", m_KiCadLayer, -m_angle));
    }
}

/*
procedure THPCBArc.SetPosOffset(x_offs:integer;y_offs:integer);
begin
  inherited;
  StartX:=StartX + x_offs;
  StartY:=StartY + y_offs;
end;
*/

void CPCBArc::SetPosOffset(int x_offs, int y_offs) {
    CPCBComponent::SetPosOffset(x_offs, y_offs);
    m_startX += x_offs;
    m_startY += y_offs;
}

/*
constructor THPCBModule.Create(iname:string);
begin
   ObjType:='M';  // MODULE
   Name.Text:=iName;
   KiCadLayer:=21; // default
   ModuleObjects:=TList.Create;
end;
*/

CPCBModule::CPCBModule(wxString iName) {
    CPCBComponent::CPCBComponent();
    InitTTextValue(&m_value);
    m_mirror = 0;
    m_objType = 'M';  // MODULE
    m_name.text = iName;
    m_KiCadLayer = 21; // default
}

/*
procedure THPCBModule.WriteToFile(var f:text;ftype:char);
var     i:integer;
        visibility,mirrored:Char;

 function ModuleLAyer():string;
 begin
 /////NOT !   {IntToStr(KiCadLayer)}    NOT !
 ///  MODULES ARE HARD PLACED ON COMPONENT OR COOPER LAYER.
 ///  IsFLIPPED--> MIRROR attribute is decision Point!!!
   if Mirror=0 then Result:='15' //Components side
   else Result:= '0'; // Cooper side
 end;
*/

wxString ModuleLayer(int mirror) {
    wxString result;

 /////NOT !   {IntToStr(KiCadLayer)}    NOT !
 ///  MODULES ARE HARD PLACED ON COMPONENT OR COOPER LAYER.
 ///  IsFLIPPED--> MIRROR attribute is decision Point!!!

    if (mirror == 0) result = wxT("15"); //Components side
    else result = wxT("0"); // Cooper side

    return result;
}

/*
 begin
     { transphorm text positions ....}
     CorrectTextPosition(Name,Rotation);
     CorrectTextPosition(Value,Rotation);
     // Go out
     Writeln(f,'');
     Writeln(f,'$MODULE '+Name.text);
     Writeln(f,'Po '+IntToStr(PositionX)+' '+IntToStr(PositionY)
                    +' '+IntToStr(Rotation)+' '
                    +ModuleLayer() +' 00000000 00000000 ~~'); // Position
     Writeln(f,'Li '+Name.Text);    // Modulename
     Writeln(f,'Sc 00000000'); // Timestamp
     Writeln(f,'Op 0 0 0'); // Orientation
     Writeln(f,'At SMD');   // ??
     // MODULE STRINGS
     if (Name.TextIsVisible=1) then visibility:='V' else visibility:='I';
     if (Name.Mirror=1) then mirrored:='M' else mirrored:='N';
     Writeln(f,'T0 '+IntToStr(Name.CorrectedPositionX)+' '+ IntToStr(Name.CorrectedPositionY)
               +' '+IntToStr(Round(Name.TextHeight/2))+' '+IntToStr(Round(Name.TextHeight/1.5))
               +' '+ IntToStr(Name.TextRotation)+' '+IntToStr(value.TextstrokeWidth)
               +' '+mirrored+' '+visibility+' '+IntToStr(KiCadLayer)+' "'+Name.Text+'"'); // NameString
     if (VAlue.TextIsVisible=1) then visibility:='V' else visibility:='I';
     if (VAlue.Mirror=1) then mirrored:='M' else mirrored:='N';
     Writeln(f,'T1 '+IntToStr(Value.CorrectedPositionX)+' '+IntToStr(Value.CorrectedPositionY)
              +' '+IntToStr(Round(Value.TextHeight/2))+' '+IntToStr(Round(Value.TextHeight/1.5))
              +' '+ IntToStr(Value.TextRotation)+' '+IntToStr(value.TextstrokeWidth)
              +' '+mirrored+' '+visibility+' '+IntToStr(KiCadLayer)+' "'+Value.Text+'"'); // ValueString
    // TEXTS
        for i:=0 to  ModuleObjects.Count-1 do
         begin
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='T' then
                 begin
                      THPCBText(ModuleObjects.Items[i]).Tag:=i+2;
                      THPCBText(ModuleObjects.Items[i]).WriteToFile(f,fType);
                 end;
         end;
     // MODULE LINES
      for i:=0 to ModuleObjects.Count-1 do
       begin
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='L' then
                   THPCBLine(ModuleObjects.Items[i]).WriteToFile(f,fType);
       end;
     // MODULE Arcs
      for i:=0 to ModuleObjects.Count-1 do
       begin
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='A' then
                  THPCBArc(ModuleObjects.Items[i]).WriteToFile(f,fType);
       end;
     // PADS
      for i:=0 to ModuleObjects.Count-1 do
       begin
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='P' then
                  THPCBPad(ModuleObjects.Items[i]).WriteToFile(f,fType,Rotation);
       end;
     // VIAS
      for i:=0 to ModuleObjects.Count-1 do
       begin
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='V' then
                THPCBVia(ModuleObjects.Items[i]).WriteToFile(f,fType,Rotation);
       end;
     // END
     Writeln(f,'$EndMODULE '+Name.Text);
 end;
*/

void CPCBModule::WriteToFile(wxFile *f, char ftype) {
    char visibility, mirrored;
    int i;

    // transform text positions ....
    CorrectTextPosition(&m_name, m_rotation);
    CorrectTextPosition(&m_value, m_rotation);
    // Go out
    f->Write(wxT("\n"));
    f->Write(wxT("$MODULE ") + m_name.text + wxT("\n"));
    f->Write(wxString::Format("Po %d %d %d ", m_positionX, m_positionY, m_rotation) +
            ModuleLayer(m_mirror) + wxT(" 00000000 00000000 ~~\n")); // Position
    f->Write(wxT("Li ") + m_name.text + wxT("\n"));   // Modulename
    f->Write(wxT("Sc 00000000\n")); // Timestamp
    f->Write(wxT("Op 0 0 0\n")); // Orientation
    f->Write(wxT("At SMD\n"));   // ??

    // MODULE STRINGS
    if (m_name.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_name.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    f->Write(wxString::Format("T0 %d %d %d %d %d %d", m_name.correctedPositionX, m_name.correctedPositionY,
               KiROUND(m_name.textHeight / 2), KiROUND(m_name.textHeight / 1.5),
               m_name.textRotation, m_value.textstrokeWidth /* TODO: Is it correct to use m_value.textstrokeWidth here? */) +
               ' ' + mirrored + ' ' + visibility + wxString::Format(" %d \"", m_KiCadLayer) + m_name.text + wxT("\"\n")); // NameString

    if (m_value.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_value.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    f->Write(wxString::Format("T1 %d %d %d %d %d %d", m_value.correctedPositionX, m_value.correctedPositionY,
               KiROUND(m_value.textHeight / 2), KiROUND(m_value.textHeight / 1.5),
               m_value.textRotation, m_value.textstrokeWidth) +
               ' ' + mirrored + ' ' + visibility + wxString::Format(" %d \"", m_KiCadLayer) + m_value.text + wxT("\"\n")); // ValueString

    // TEXTS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'T') {
            ((CPCBText *)m_moduleObjects[i])->m_tag = i + 2;
            m_moduleObjects[i]->WriteToFile(f, ftype);
        }
    }

    // MODULE LINES
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'L')
            m_moduleObjects[i]->WriteToFile(f, ftype);
    }

    // MODULE Arcs
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'A')
            m_moduleObjects[i]->WriteToFile(f, ftype);
    }

    // PADS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'P')
            ((CPCBPad *)m_moduleObjects[i])->WriteToFile(f, ftype, m_rotation);
    }

    // VIAS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'V')
            ((CPCBVia *)m_moduleObjects[i])->WriteToFile(f, ftype, m_rotation);
    }

    // END
    f->Write(wxT("$EndMODULE ") + m_name.text + wxT("\n"));
}

/*
procedure THPCBModule.Flip;
var i:integer;
    j:integer;
begin
  if (Mirror=1) then
  begin  // Flipped
      KiCadLayer:=FlipLayers(KiCadLAyer);
      Rotation:=-Rotation;
      Name.TextPositionX:=-Name.TextPositionX;
      Name.Mirror:=Mirror;
      Value.TextPositionX:=-VAlue.TextPositionX;
      Value.Mirror:=Mirror;
      for i:=0 to ModuleObjects.Count-1 do
       begin
           // MODULE LINES
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='L' then
                 begin
                   THPCBLine(ModuleObjects.Items[i]).PositionX:=-THPCBLine(ModuleObjects.Items[i]).PositionX;
                   THPCBLine(ModuleObjects.Items[i]).ToX:=-THPCBLine(ModuleObjects.Items[i]).ToX;
                   THPCBLine(ModuleObjects.Items[i]).KiCadLayer:=FlipLayers(THPCBLine(ModuleObjects.Items[i]).KiCadLayer);
                 end;
           // MODULE Arcs
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='A' then
                 begin
                  THPCBArc(ModuleObjects.Items[i]).PositionX:=-THPCBArc(ModuleObjects.Items[i]).PositionX;
                  THPCBArc(ModuleObjects.Items[i]).StartX:=-THPCBArc(ModuleObjects.Items[i]).StartX;
                  THPCBArc(ModuleObjects.Items[i]).KiCadLayer:=FlipLayers(THPCBArc(ModuleObjects.Items[i]).KiCadLayer);
                 end;
           // PADS
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='P' then
                 begin
                  THPCBPad(ModuleObjects.Items[i]).PositionX:=-THPCBPad(ModuleObjects.Items[i]).PositionX;
                  THPCBPad(ModuleObjects.Items[i]).Rotation:=-THPCBPad(ModuleObjects.Items[i]).Rotation;
                  for j:=0 to THPCBPad(ModuleObjects.Items[i]).Shapes.Count-1 do
                      THPCBPadShape(THPCBPad(ModuleObjects.Items[i]).Shapes[j]).KiCadLayer:=FlipLayers(THPCBPadShape(THPCBPad(ModuleObjects.Items[i]).Shapes[j]).KiCadLayer);
                 end;
           // VIAS
           if THPCBComponent(ModuleObjects.Items[i]).ObjType='V' then
                begin
                  THPCBVia(ModuleObjects.Items[i]).PositionX:=-THPCBVia(ModuleObjects.Items[i]).PositionX;
                  for j:=0 to THPCBVia(ModuleObjects.Items[i]).Shapes.Count-1 do
                      THPCBViaShape(THPCBVia(ModuleObjects.Items[i]).Shapes[j]).KiCadLayer:=FlipLayers(THPCBViaShape(THPCBVia(ModuleObjects.Items[i]).Shapes[j]).KiCadLayer);
                end;
       end;
  end;
end;
*/

void CPCBModule::Flip() {
    int i, j;

    if (m_mirror == 1) {
        // Flipped
        m_KiCadLayer = FlipLayers(m_KiCadLayer);
        m_rotation = -m_rotation;
        m_name.textPositionX = -m_name.textPositionX;
        m_name.mirror = m_mirror;
        m_value.textPositionX = -m_value.textPositionX;
        m_value.mirror = m_mirror;

        for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
            // MODULE LINES
            if (m_moduleObjects[i]->m_objType == 'L') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                ((CPCBLine *)m_moduleObjects[i])->m_toX = -((CPCBLine *)m_moduleObjects[i])->m_toX;
                m_moduleObjects[i]->m_KiCadLayer = FlipLayers(m_moduleObjects[i]->m_KiCadLayer);
            }

            // MODULE Arcs
            if (m_moduleObjects[i]->m_objType == 'A') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                ((CPCBArc *)m_moduleObjects[i])->m_startX = -((CPCBArc *)m_moduleObjects[i])->m_startX;
                m_moduleObjects[i]->m_KiCadLayer = FlipLayers(m_moduleObjects[i]->m_KiCadLayer);
            }

            // PADS
            if (m_moduleObjects[i]->m_objType == 'P') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                m_moduleObjects[i]->m_rotation = -m_moduleObjects[i]->m_rotation;

                for (j = 0; j < (int)((CPCBPad *)m_moduleObjects[i])->m_shapes.GetCount(); j++)
                    ((CPCBPad *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer =
                        FlipLayers(((CPCBPad *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer);
            }

            // VIAS
            if (m_moduleObjects[i]->m_objType == 'V') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;

                for (j = 0; j < (int)((CPCBVia *)m_moduleObjects[i])->m_shapes.GetCount(); j++)
                    ((CPCBVia *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer =
                        FlipLayers(((CPCBVia *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer);
            }
        }
    }
}

/*
destructor THPCBModule.Free;
begin
   ModuleObjects.free;
end;
*/

CPCBModule::~CPCBModule() {
}

/*
constructor THPCB.Create();
var i:integer;
begin
   DefaultMeasurementUnit:='mil';
   PCBComponents:=TList.Create;
   PCBNetlist:=TList.Create;
   for i:=0 to 27 do LayersMap[i]:=23; // default
   LayersMap[1]:=15;
   LayersMap[2]:=0;
   LayersMap[3]:=27;
   LayersMap[6]:=21;
   LayersMap[7]:=20;
   timestamp_cnt:=$10000000;
end;
*/

CPCB::CPCB() {
    int i;

    m_defaultMeasurementUnit = wxT("mil");
    for (i = 0; i < 28; i++)
        m_layersMap[i] = 23; // default

    m_sizeX = 0;
    m_sizeY = 0;

    m_layersMap[1] = 15;
    m_layersMap[2] = 0;
    m_layersMap[3] = 27;
    m_layersMap[6] = 21;
    m_layersMap[7] = 20;
    m_timestamp_cnt = 0x10000000;
}

/*
destructor THPCB.Free;
begin
  PCBComponents.Free;
  PCBNetlist.Free;
end;
*/

CPCB::~CPCB() {
}

/*
procedure THPCB.WriteToFile(FileName:string;ftype:char);
var f:text;
    i:integer;
 begin
  AssignFile(f,FileName);
  Rewrite(f);
  if ftype='L' then
    begin // LIBRARY
    Writeln(f,'PCBNEW-LibModule-V1  01/01/2001-01:01:01');
     Writeln(f,'');
    Writeln(f,'$INDEX');
     for i:=0 to PCBComponents.Count-1 do
       begin
          Application.ProcessMessages;
          if (THPCBComponent(PCBComponents.Items[i]).ObjType='M') then
                 Writeln(f,THPCBModule(PCBComponents.Items[i]).Name.Text);
       end;
    Writeln(f,'$EndINDEX');
     for i:=0 to PCBComponents.Count-1 do
       begin
         Application.ProcessMessages;
         if (THPCBComponent(PCBComponents.Items[i]).ObjType='M') then
                  THPCBModule(PCBComponents.Items[i]).WriteToFile(f,'L');
       end;
    Writeln(f,'$EndLIBRARY');
    end;   // LIBRARY
  if ftype='P' then
    begin // PCB
     Writeln(f,'PCBNEW-BOARD Version 1 date 01/1/2000-01:01:01');
     Writeln(f,'$SHEETDESCR');
     Writeln(f,'$Sheet User '+IntToStr(SizeX)+' '+IntToStr(SizeY));
     Writeln(f,'$EndSHEETDESCR');
     // MODULES
     for i:=0 to PCBComponents.Count-1 do
       begin
         Application.ProcessMessages;
         if (THPCBComponent(PCBComponents.Items[i]).ObjType='M') then THPCBModule(PCBComponents.Items[i]).WriteToFile(f,'L');
       end;
    // TEXTS
    Writeln(f,'');
     for i:=0 to PCBComponents.Count-1 do
       begin
         if (THPCBComponent(PCBComponents.Items[i]).ObjType='T') then
         begin
           Application.ProcessMessages;
            Writeln(f,'$TEXTPCB');
             THPCBText(PCBComponents.Items[i]).WriteToFile(f,'P');
            Writeln(f,'$EndTEXTPCB');
         end;
       end;
   // SEGMENTS
    Writeln(f,'');
     for i:=0 to PCBComponents.Count-1 do
       begin
         if (((THPCBComponent(PCBComponents.Items[i]).ObjType='L')
             or (THPCBComponent(PCBComponents.Items[i]).ObjType='A'))
         and not(THPCBComponent(PCBComponents.Items[i]).KiCadLayer in [0,15])) then
         begin
             Application.ProcessMessages;
             Writeln(f,'$DRAWSEGMENT');
                 THPCBComponent(PCBComponents.Items[i]).WriteToFile(f,'P');
             Writeln(f,'$EndDRAWSEGMENT');
         end;
       end;
    // TRACKS
    Writeln(f,'');
    Writeln(f,'$TRACK');
     for i:=0 to PCBComponents.Count-1 do
       begin
         Application.ProcessMessages;
         // LINES
         if ((THPCBComponent(PCBComponents.Items[i]).ObjType='L')
         and (THPCBComponent(PCBComponents.Items[i]).KiCAdLayer in [0,15])) then
                 THPCBLine(PCBComponents.Items[i]).WriteToFile(f,'P');
         // VIAS
         if (THPCBComponent(PCBComponents.Items[i]).ObjType='V') then
                 THPCBVia(PCBComponents.Items[i]).WriteToFile(f,'P',0);
       end;
    Writeln(f,'$EndTRACK');
    // ZONES
    Writeln(f,'');
    Writeln(f,'$ZONE');
    for i:=0 to PCBComponents.Count-1 do
       begin
         Application.ProcessMessages;
         if ((THPCBComponent(PCBComponents.Items[i]).ObjType='Z')) then
                 THPCBPolygon(PCBComponents.Items[i]).WriteToFile(f,'P');
       end;
    Writeln(f,'$EndZONE');
    for i:=0 to PCBComponents.Count-1 do
       begin
         Application.ProcessMessages;
         if ((THPCBComponent(PCBComponents.Items[i]).ObjType='Z')) then
                 THPCBPolygon(PCBComponents.Items[i]).WriteOutlineToFile(f,'P');
       end;

    Writeln(f,'');
    Writeln(f,'$EndBOARD');
  end;
  CloseFile(f);
 end;
*/

void CPCB::WriteToFile(wxString fileName, char ftype) {
    wxFile f;
    int i;

    f.Open(fileName, wxFile::write);
    if (ftype == 'L') {
        // LIBRARY
        f.Write(wxT("PCBNEW-LibModule-V1  01/01/2001-01:01:01\n"));
        f.Write(wxT("\n"));
        f.Write(wxT("$INDEX\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'M')
                f.Write(m_pcbComponents[i]->m_name.text + wxT("\n"));
        }

        f.Write(wxT("$EndINDEX\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'M')
                m_pcbComponents[i]->WriteToFile(&f, 'L');
        }

        f.Write(wxT("$EndLIBRARY\n"));
    } // LIBRARY

    if (ftype == 'P') {
        // PCB
        f.Write(wxT("PCBNEW-BOARD Version 1 date 01/1/2000-01:01:01\n"));
        f.Write(wxT("$SHEETDESCR\n"));
        f.Write(wxString::Format("$Sheet User %d %d\n", m_sizeX, m_sizeY));
        f.Write(wxT("$EndSHEETDESCR\n"));

        // MODULES
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'M') m_pcbComponents[i]->WriteToFile(&f, 'L');
        }

        // TEXTS
        f.Write(wxT("\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'T') {
                f.Write(wxT("$TEXTPCB\n"));
                m_pcbComponents[i]->WriteToFile(&f, 'P');
                f.Write(wxT("$EndTEXTPCB\n"));
            }
        }

        // SEGMENTS
        f.Write(wxT("\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if ((m_pcbComponents[i]->m_objType == 'L' ||
                 m_pcbComponents[i]->m_objType == 'A') &&
                !(m_pcbComponents[i]->m_KiCadLayer >= 0 && m_pcbComponents[i]->m_KiCadLayer <= 15))
            {
                f.Write(wxT("$DRAWSEGMENT\n"));
                m_pcbComponents[i]->WriteToFile(&f, 'P');
                f.Write(wxT("$EndDRAWSEGMENT\n"));
            }
        }

        // TRACKS
        f.Write(wxT("\n"));
        f.Write(wxT("$TRACK\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            // LINES
            if (m_pcbComponents[i]->m_objType == 'L' &&
                (m_pcbComponents[i]->m_KiCadLayer >= 0 && m_pcbComponents[i]->m_KiCadLayer <= 15))
            {
                m_pcbComponents[i]->WriteToFile(&f, 'P');
            }

            // VIAS
            if (m_pcbComponents[i]->m_objType == 'V')
                ((CPCBVia *)m_pcbComponents[i])->WriteToFile(&f, 'P', 0);
        }

        f.Write(wxT("$EndTRACK\n"));
        // ZONES
        f.Write(wxT("\n"));
        f.Write(wxT("$ZONE\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'Z')
                m_pcbComponents[i]->WriteToFile(&f, 'P');
        }

        f.Write(wxT("$EndZONE\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'Z')
                ((CPCBPolygon *)m_pcbComponents[i])->WriteOutlineToFile(&f, 'P');
        }

        f.Write(wxT("\n"));
        f.Write(wxT("$EndBOARD\n"));
    }

    f.Close();
}

/*
function THPCB.GetNewTimestamp():integer;
begin
  result:=timestamp_cnt;
  inc(timestamp_cnt);
end;

end.
*/

int CPCB::GetNewTimestamp() {
    return m_timestamp_cnt++;
}
