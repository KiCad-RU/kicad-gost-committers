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
 * @file ProcessXMLtoPCBUnit.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <ProcessXMLtoPCBUnit.h>

/*
unit ProcessXMLToPCBUnit;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom,
  PCBComponents,PCBPolygonsUnit,XMLToObjectCommonProceduresUnit;

function ProcessXMLToPCBLib(StatusBar:TStatusBar;XMLFileNAme:string):THPCB;


implementation


function ProcessXMLToPCBLib(StatusBar:TStatusBar;XMLFileNAme:string):THPCB;
var
 PCB:THPCB;
 xmlDoc: IxmlDocument;
 ANode,cNode: IXMLNode;
 i,j,k: Integer;
 PCBModule:THPCBModule;
 PCBPad:THPCBPad;
 PCBVia:THPCBVia;
 PCBPadShape:THPCBPadShape;
 PCBViaShape:THPCBViaShape;
 ComponentLine:THPCBLine;
 ComponentText:THPCBText;
 ComponentArc:THPCBArc;
 ComponentPolygon:THPCBPolygon;
 NetNode:THNetNode;
 Net:THNet;
 cr,pr:string;
 //MaxX,MaxY,MinX,MinY:integer;

function CreatePCBPadShape(inode:IXMLNode):THPCBPadShape;
var lNode:iXMLNode;
    s:string;
    MinX,MinY,MaxX,MaxY,x,y:integer;
begin
    PCBPadShape:=THPCBPadShape.Create();
    lNode:=iNode.ChildNodes.FindNode('padShapeType');
    if Assigned(lNode) then PCBPadShape.Shape:=TrimLeft(lNode.Text);
    lNode:=iNode.ChildNodes.FindNode('layerNumRef');
    if Assigned(lNode) then PCBPadShape.PCadLayer:=StrToInt(lNode.Text);
    PCBPadShape.KiCadLayer:=PCB.LayersMap[PCBPadShape.PCadLayer];
    if ((PCBPadShape.Shape='Oval')
        or (PCBPadShape.Shape='Rect')
        or (PCBPadShape.Shape='Ellipse')
        or (PCBPadShape.Shape='RndRect') ) then
        begin
          lNode:=iNode.ChildNodes.FindNode('shapeWidth');
          if Assigned(lNode) then SetWidth(lNode.Text,PCB.DefaultMeasurementUnit,PCBPadShape.Width);
          lNode:=iNode.ChildNodes.FindNode('shapeHeight');
          if Assigned(lNode) then SetWidth(lNode.Text,PCB.DefaultMeasurementUnit,PCBPadShape.Height);
        end;
    if (PCBPadShape.Shape='Polygon') then
      begin
        // aproximation to simplier pad shape .....
         lNode:=iNode.ChildNodes.FindNode('shapeOutline');
         if assigned(lNode) then lNode:=lNode.ChildNodes.FindNode('pt');
         minX:=0;MAxX:=0;MinY:=0;MAxY:=0;
         while assigned(lNode) do
          begin
            s:=lNode.Text;
            SetPosition(s,PCB.DefaultMeasurementUnit,x,y);
            if (minX>x) then Minx:=x;
            if (MaxX<x) then MaxX:=x;
            if (minY>y) then MinY:=y;
            if (MaxY<y) then MaxY:=y;
             lNode:=lNode.NextSibling;
          end;
          PCBPadShape.Width:=MaxX-MinX;
          PCBPadShape.Height:=MaxY-MinY;
      end;
   result:=PCBPadShape;
end;

function CreatePCBViaShape(inode:IXMLNode):THPCBViaShape;
var lNode:iXMLNode;
    s:string;
begin
    PCBViaShape:=THPCBPadShape.Create();
    lNode:=iNode.ChildNodes.FindNode('viaShapeType');
    if Assigned(lNode) then PCBViaShape.Shape:=TrimLeft(lNode.Text);
    lNode:=iNode.ChildNodes.FindNode('layerNumRef');
    if Assigned(lNode) then  PCBViaShape.PCadLayer:=StrToInt(lNode.Text);
    PCBViaShape.KiCadLayer:=PCB.LayersMap[PCBViaShape.PCadLayer];
    lNode:=iNode.ChildNodes.FindNode('shapeWidth');
    if Assigned(lNode) then SetWidth(lNode.Text,PCB.DefaultMeasurementUnit,PCBViaShape.Width);
    lNode:=iNode.ChildNodes.FindNode('shapeHeight');
    if Assigned(lNode) then SetWidth(lNode.Text,PCB.DefaultMeasurementUnit,PCBViaShape.Height);
    result:=PCBViaShape;
end;

function CreatePCBPad(inode:IXMLNode):THPCBPad;
var lNode:iXMLNode;
    si,so:string;

begin
    PCBPad:=THPCBPad.Create('');
    PCBPad.Rotation:=0;
    lNode:=iNode.ChildNodes.FindNode('padNum');
    if Assigned(lNode) then PCBPad.Number:=StrToInt(lNode.Text);
    lNode:=iNode.ChildNodes.FindNode('padStyleRef');
    if Assigned(lNode) then PCBPad.Name.Text:=TrimLeft(lNode.Attributes['Name']);
    lNode:=iNode.ChildNodes.FindNode('pt');
    if Assigned(lNode) then SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,PCBPad.PositionX,PCBPad.PositionY);
    lNode:=iNode.ChildNodes.FindNode('rotation');
    if Assigned(lNode) then PCBPad.Rotation:=StrToInt1Units(TrimLeft(lNode.Text));
    lNode:=iNode;
    while (lNode.NodeName<>'www.lura.sk') do lNode:=LNode.ParentNode;
    lNode:=lNode.ChildNodes.FindNode('library');
    lNode:=lNode.ChildNodes.FindNode('padStyleDef');
    while (lNode.Attributes['Name']<>PCBPad.Name.Text) do lNode:=LNode.NextSibling;
    lNode:=lNode.ChildNodes.FindNode('holeDiam');
    if Assigned(lNode) then SetWidth(lNode.Text,PCB.DefaultMeasurementUnit,PCBPad.Hole);
    lNode:=lNode.ParentNode;
    lNode:=lNode.ChildNodes.FindNode('padShape');
    while Assigned(lNode) do
     begin
         if  (lNode.NodeName='padShape') then
           begin
             // we support only PAds on specific layers......
             // we do not support pads on "Plane", "NonSignal" , "Signal" ... layerr
                if Assigned(lNode.ChildNodes.FindNode('layerNumRef')) then
                             PCBPad.Shapes.Add(CreatePCBPadShape(lNode));
           end;
         lNode := lNode.NextSibling;
     end;
    result:=PCBPad;
end;

function CreateVia(inode:IXMLNode):THPCBVia;
var lNode,tNode:iXMLNode;
    si,so:string;
begin
    PCBVia:=THPCBVia.Create();
    PCBVia.Rotation:=0;
    lNode:=iNode.ChildNodes.FindNode('viaStyleRef');
    if Assigned(lNode) then PCBVia.Name.Text:=Trim(lNode.Attributes['Name']);
    lNode:=iNode.ChildNodes.FindNode('pt');
    if Assigned(lNode) then SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,PCBVia.PositionX,PCBVia.PositionY);
    lNode:=iNode.ChildNodes.FindNode('netNameRef');
    if Assigned(lNode) then PCBVia.Net:=Trim(lNode.Attributes['Name']);
    lNode:=iNode;
    while (lNode.NodeName<>'www.lura.sk') do lNode:=lNode.ParentNode;
    lNode:=lNode.ChildNodes.FindNode('library');
    lNode:=lNode.ChildNodes.FindNode('viaStyleDef');
    if Assigned(lnode) then
    begin
      while (Assigned(lNode) and (lNode.Attributes['Name']<>PCBVia.Name.text)) do lNode:=LNode.NextSibling;
    end;
    if  assigned(lNode) then
    begin
       tNode:=lNode;
       lNode:=tNode.ChildNodes.FindNode('holeDiam');
       if Assigned(lNode) then SetWidth(lNode.Text,PCB.DefaultMeasurementUnit,PCBVia.Hole);
       lNode:=tNode.ChildNodes.FindNode('viaShape');
       while Assigned(lNode) do
       begin
         if  (lNode.NodeName='viaShape') then
           begin
             // we support only Vias on specific layers......
             // we do not support pads on "Plane", "NonSignal" , "Signal" ... layerr
                if Assigned(lNode.ChildNodes.FindNode('layerNumRef')) then
                             PCBVia.Shapes.Add(CreatePCBViaShape(lNode));
           end;
       lNode := lNode.NextSibling;
       end;
     end;
    result:=PCBVia;
end;

function CreateComponentLine(inode:IXMLNode;l:integer):THPCBLine;
var lNode,tNode:iXMLNode;
    si,so:string;
begin
    ComponentLine:=THPCBLine.Create();
    ComponentLine.PCAdLayer:=l;
    ComponentLine.KiCadLayer:=PCB.LayersMap[ComponentLine.PCadLayer];
    ComponentLine.PositionX:=0;
    ComponentLine.PositionY:=0;
    ComponentLine.ToX:=0;
    ComponentLine.ToY:=0;
    ComponentLine.Width:=0;
    lNode:=iNode.ChildNodes.FindNode('pt');
    if Assigned(lNode) then SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,ComponentLine.PositionX,ComponentLine.PositionY);
    lNode := lNode.NextSibling;
    if Assigned(lNode) then SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,ComponentLine.ToX,ComponentLine.ToY);
    lNode:=iNode.ChildNodes.FindNode('width');
    if Assigned(lNode) then SetWidth(lNode.Text,PCB.DefaultMeasurementUnit,ComponentLine.width);
    lNode:=iNode.ChildNodes.FindNode('netNameRef');
    if Assigned(lNode) then ComponentLine.Net:=Trim(lNode.Attributes['Name']);
   result:=ComponentLine;
end;

function CreateComponentText(inode:IXMLNode;l:integer):THPCBText;
var lNode,tNode:iXMLNode;
    si,so:string;
begin
    ComponentText:=THPCBText.Create();
    ComponentText.PCadLayer:=l;
    ComponentText.KiCadLayer:=PCB.LayersMap[ComponentText.PCadLayer];
    ComponentText.PositionX:=0;
    ComponentText.PositionY:=0;
    ComponentText.Name.Mirror:=0;      //Normal, not mirrored
    lNode:=iNode.ChildNodes.FindNode('pt');
    if Assigned(lNode) then SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,ComponentText.PositionX,ComponentText.PositionY);
    lNode:=iNode.ChildNodes.FindNode('rotation');
    if Assigned(lNode) then
          ComponentText.Rotation:=StrToInt1Units(TrimLeft(lNode.Text));
    lNode:=iNode.ChildNodes.FindNode('value');
    if Assigned(lNode) then
         ComponentText.Name.Text:=lNode.Text;
    lNode:=iNode.ChildNodes.FindNode('isFlipped');
    if Assigned(lNode) then
         if (Trim(lNode.Text)='True') then ComponentText.Name.Mirror:=1;
    lNode:=iNode.ChildNodes.FindNode('textStyleRef');
    if Assigned(lNode) then  SetFontProperty(lNode,ComponentText.Name,PCB.DefaultMeasurementUnit);
   result:=ComponentText;
end;

//Alexander Lunev added (begin)
procedure FormPolygonLines(inode:IXMLNode; var lines:TList; PCADlayer:integer;
                            width:integer);
var lNode:iXMLNode;
    i:integer;
    line:THPCBLine;
begin
    lNode:=iNode.ChildNodes.FindNode('pt');
    while Assigned(lNode) do
    begin
      // Start points of lines and num of lines=num of polygon points
      if lNode.NodeName='pt' then
      begin
          line:=THPCBLine.Create;
          line.PCadLayer:=PCADlayer;
          line.Width:=width;
          line.KiCadLayer:=PCB.LayersMap[PCADlayer];
          lines.Add(line);
          SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,line.PositionX,line.PositionY);
      end;
      lNode:=lNode.NextSibling;
    end;

   // next is to complete polygon or island outline ....
   if lines.Count>1 then
   begin
     for i:=0 to lines.Count-2 do
     begin
       THPCBLine(lines[i]).ToX:=THPCBLine(lines[i+1]).PositionX;
       THPCBLine(lines[i]).ToY:=THPCBLine(lines[i+1]).PositionY;
      end;
    //last line
    THPCBLine(lines[lines.Count-1]).ToX:=THPCBLine(lines[0]).PositionX;
    THPCBLine(lines[lines.Count-1]).ToY:=THPCBLine(lines[0]).PositionY;
   end;
end;

procedure DrawThermal(inode:IXMLNode; var lines:TList; PCADlayer:integer; width:integer;
                        timestamp:integer);
var lNode:iXMLNode;
    start:boolean;
    line:THPCBLine;
    x_end,y_end:integer;
begin
    start:=true;
    lNode:=iNode.ChildNodes.FindNode('pt');
    line:=THPCBLine.Create;
    line.PCadLayer:=PCADlayer;
    line.Width:=width;
    line.KiCadLayer:=PCB.LayersMap[PCADlayer];
    line.timestamp:=timestamp;

    while Assigned(lNode) do
    begin
      if lNode.NodeName='pt' then
      begin
          if (start) then
          begin
            SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,line.PositionX,line.PositionY);
            start:=false;
          end
          else SetPosition(lNode.Text, PCB.DefaultMeasurementUnit, x_end, y_end);
      end;
      lNode:=lNode.NextSibling;
    end;

    line.ToX:=x_end;
    line.ToY:=y_end;
    lines.Add(line);
end;

procedure FormPolygonIsland(inode:IXMLNode; var island:TList);
var lNode:iXMLNode;
    i:integer;
    vertex:TVertex;
begin
    //StatusBar.SimpleText:=StatusBar.SimpleText+' Island...';
    lNode:=iNode.ChildNodes.FindNode('pt');
    while Assigned(lNode) do
    begin
      if lNode.NodeName='pt' then
      begin
          vertex:=TVertex.Create;
          island.Add(vertex);
          SetDoublePrecisionPosition(lNode.Text,PCB.DefaultMeasurementUnit,vertex.x,vertex.y);
      end;
      lNode:=lNode.NextSibling;
    end;
end;

function CreateComponentPolygon(inode:IXMLNode;PCADlayer:integer):THPCBPolygon;
var lNode,tNode:iXMLNode;
    s:string;
begin
    s:=StatusBar.SimpleText;
    StatusBar.SimpleText:=s+' Polygon...';
    ComponentPolygon:=THPCBPolygon.Create();
    ComponentPolygon.PCadLayer:=PCADlayer;
    ComponentPolygon.KiCadLayer:=PCB.LayersMap[ComponentPolygon.PCadLayer];

    lNode:=iNode.ChildNodes.FindNode('netNameRef');
    if Assigned(lNode) then ComponentPolygon.Net:=Trim(lNode.Attributes['Name']);
    FormPolygonLines(inode, componentPolygon.outline, PCADlayer, 50);
    componentPolygon.PositionX:=THPCBLine(componentPolygon.outline[0]).PositionX;
    componentPolygon.PositionY:=THPCBLine(componentPolygon.outline[0]).PositionY;
    result:=ComponentPolygon;
end;

function CreateComponentCopperPour(inode:IXMLNode;PCADlayer:integer):THPCBCopperPour;
var lNode,tNode,cNode:iXMLNode;
    x,y,i:integer;
    componentCopperPour:THPCBCopperPour;
    island, cutouts, cutout:TList;
    width, pourSpacing, thermalWidth:integer;
    pourType:string;

begin
    StatusBar.SimpleText:=StatusBar.SimpleText+' CooperPour...';
    componentCopperPour:=THPCBCopperPour.Create();
    componentCopperPour.PCadLayer:=PCADlayer;
    componentCopperPour.KiCadLayer:=PCB.LayersMap[PCADlayer];
    componentCopperPour.timestamp:=PCB.GetNewTimestamp();

    pourType:=UpperCase(TrimLeft(iNode.ChildNodes.FindNode('pourType').Text));

    lNode:=iNode.ChildNodes.FindNode('netNameRef');
    if Assigned(lNode) then componentCopperPour.Net:=Trim(lNode.Attributes['Name']);

    SetWidth(iNode.ChildNodes.FindNode('width').Text,PCB.DefaultMeasurementUnit,width);
    if Assigned(iNode.ChildNodes.FindNode('pourSpacing')) then
        SetWidth(iNode.ChildNodes.FindNode('pourSpacing').Text,PCB.DefaultMeasurementUnit,pourSpacing);
    if Assigned(iNode.ChildNodes.FindNode('thermalWidth')) then
        SetWidth(iNode.ChildNodes.FindNode('thermalWidth').Text,PCB.DefaultMeasurementUnit,thermalWidth);

    lNode:=iNode.ChildNodes.FindNode('pcbPoly');
    if Assigned(lNode) then
    begin
      FormPolygonLines(lNode, componentCopperPour.outline, PCADlayer, width);
      componentCopperPour.PositionX:=THPCBLine(componentCopperPour.outline[0]).PositionX;
      componentCopperPour.PositionY:=THPCBLine(componentCopperPour.outline[0]).PositionY;

      lNode:=iNode.ChildNodes.FindNode('island');
      while  Assigned(lNode) do
      begin
        componentCopperPour.islands:=TList.Create();
        tNode:=lNode.ChildNodes.FindNode('islandOutline');
        if Assigned(tNode) then
        begin
          island:=TList.Create();
          cutouts:=TList.Create();
          FormPolygonIsland(tNode, island);
          componentCopperPour.islands.Add(island);
          tNode:=lNode.ChildNodes.FindNode('cutout');
          while  Assigned(tNode) do
          begin
            componentCopperPour.islands:=TList.Create();
            cNode:=tNode.ChildNodes.FindNode('cutoutOutline');
            if Assigned(cNode) then
            begin
              cutout:=TList.Create();
              FormPolygonIsland(cNode, cutout);
              cutouts.Add(cutout);
            end;

            tNode := tNode.NextSibling;
          end;

          FillZone(island, cutouts, componentCopperPour.fill_lines, pourSpacing, width,
                    pourType, PCADlayer, componentCopperPour.KiCadLayer,
                    componentCopperPour.timestamp);
        end;

        tNode:=lNode.ChildNodes.FindNode('thermal');
        while  Assigned(tNode) do
        begin
          DrawThermal(tNode, componentCopperPour.fill_lines, PCADlayer, thermalWidth,
                      componentCopperPour.timestamp);
          tNode := tNode.NextSibling;
        end;

        lNode := lNode.NextSibling;
      end;

      result:=componentCopperPour;
    end
    else result:=nil;
end;

function CreateComponentCutout(inode:IXMLNode;PCADlayer:integer):THPCBCutout;
var lNode,tNode:iXMLNode;
    ComponentCutout:THPCBCutout;
begin
    ComponentCutout:=THPCBCutout.Create();
    ComponentCutout.PCadLayer:=PCADlayer;
    ComponentCutout.KiCadLayer:=PCB.LayersMap[PCADlayer];
    FormPolygonLines(inode, componentCutout.outline, PCADlayer, 0);
    componentCutout.PositionX:=THPCBLine(componentCutout.outline[0]).PositionX;
    componentCutout.PositionY:=THPCBLine(componentCutout.outline[0]).PositionY;
    result:=ComponentCutout;
end;
//Alexander Lunev added (end)



function CreateComponentArc(inode:IXMLNode;l:integer):THPCBArc;
var lNode,tNode:iXMLNode;
    si,so:string;
    r,a:integer;
begin
    ComponentArc:=THPCBArc.Create();
    ComponentArc.PCadLayer:=l;
    ComponentArc.KiCadLayer:=PCB.LayersMap[ComponentArc.PCadLayer];
    SetWidth(iNode.ChildNodes.FindNode('width').Text,PCB.DefaultMeasurementUnit,ComponentArc.Width);
    if iNode.NodeName='triplePointArc' then
    begin
       // origin
       lNode:=iNode.ChildNodes.FindNode('pt');
         if Assigned(lNode) then SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,ComponentArc.PositionX,ComponentArc.PositionY);
       lNode:=lNode.NextSibling;
         if Assigned(lNode) then
            SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,ComponentArc.StartX,ComponentArc.StartY);
           // now temporary, it can be fixed later.....
          ComponentArc.Angle:=3600;

    end;
    if iNode.NodeName='arc' then
    begin
       lNode:=iNode.ChildNodes.FindNode('pt');
       if Assigned(lNode) then SetPosition(lNode.Text,PCB.DefaultMeasurementUnit,ComponentArc.PositionX,ComponentArc.PositionY);
       lNode:=iNode.ChildNodes.FindNode('radius');
       r:=StrToIntUnits(lNode.Text,' ');
       a:=StrToInt1Units(iNode.ChildNodes.FindNode('startAngle').Text);
       ComponentArc.StartX:=Round(ComponentArc.PositionX+r*sin((a-900)*Pi/1800));
       ComponentArc.StartY:=Round(ComponentArc.PositionY-r*cos((a-900)*Pi/1800));
       ComponentArc.Angle:=StrToInt1Units(iNode.ChildNodes.FindNode('sweepAngle').Text);
    end;
   result:=ComponentArc;
end;

//Alexander Lunev modified (begin)
procedure DoLayerContentsObjects(iNode:IXMLNode;List:TList);
var lNode,tNode:IXMLNode;
    PCADlayer:integer;
    i:longint;
    poly:THPCBPolygon;
begin
   i:=0;
   StatusBar.SimpleText:='Processing LAYER CONTENT OBJECTS ';
   PCADlayer:=StrToInt(iNode.ChildNodes.FindNode('layerNumRef').Text);
   lNode:=iNode.ChildNodes.First;
   while  Assigned(lNode) do
     begin
       inc(i);
       StatusBar.SimpleText:='Processing LAYER CONTENT OBJECTS :'+IntToStr(i);
       if lNode.NodeName='line' then  List.Add(CreateComponentLine(lNode,PCADlayer));
       if lNode.NodeName='text' then  List.Add(CreateComponentText(lNode,PCADlayer));
      // added  as Sergeys request 02/2008
       if lNode.NodeName='attr' then
           begin  // assign fonts to Module Name,Value,Type,....s
             if Trim(lNode.Attributes['Name'])='Type' then
              begin
                  tNode:=lNode.ChildNodes.FindNode('textStyleRef');
                  if Assigned(tNode) then
                       SetFontProperty(tNode, PCBModule.Name,PCB.DefaultMeasurementUnit);
              end;
           end;
      // added  as Sergeys request 02/2008
       if lNode.NodeName='arc' then  List.Add(CreateComponentArc(lNode,PCADlayer));
       if lNode.NodeName='triplePointArc' then  List.Add(CreateComponentArc(lNode,PCADlayer));
       if lNode.NodeName='pcbPoly' then  List.Add(CreateComponentPolygon(lNode,PCADlayer));
       if lNode.NodeName='copperPour95' then
       begin
          poly:=CreateComponentCopperPour(lNode,PCADlayer);
          if (poly<>nil) then List.Add(poly);
       end;
       if lNode.NodeName='polyCutOut' then
            begin
             // list of polygons....
             tNode:=lNode;
             tNode:=tNode.ChildNodes.FindNode('pcbPoly');
             if Assigned(tNode) then
                   List.Add(CreateComponentCutout(tNode,PCADlayer));
            end;
       lNode := lNode.NextSibling;
     end;
end;
//Alexander Lunev modified (end)
*/

void DoLayerContentsObjects(wxXmlNode *iNode, CPCBComponentsArray *list, wxStatusBar* statusBar) {
}

/*
function FindModulePatternDefName(iNode:IXMLNode;iName:string):IXMLNode;
var lNode:IXMLNode;
begin
   result:=nil;
   lNode:=iNode.ChildNodes.FindNode('patternDef');    // Old file format
   while Assigned(lNode) do
     begin
         if (lNode.NodeName= 'patternDef') then
         begin
                if (ValidateName(lNode.Attributes['Name'])=iName)
                  or (ValidateName(LNode.ChildNodes.FindNode('originalName').Attributes['Name'])=iName)
                then
                begin
                       result:=lNode;
                       lNode:=nil;
                end;
         end;
         if Assigned(lNode) then lNode := lNode.NextSibling;
     end;
    if (result=nil) then
    begin
     lNode:=iNode.ChildNodes.FindNode('patternDefExtended');    // New file format
     while Assigned(lNode) do
       begin
         if (lNode.NodeName= 'patternDefExtended') then
                if ValidateName(lNode.Attributes['Name'])=iName then
                     begin
                       result:=lNode;
                       lNode:=nil;
                     end;
         if Assigned(lNode) then lNode := lNode.NextSibling;
       end;
    end;
end;


procedure   SetPadName(pin,name:string;mc:THPCBModule);
var i:integer;
begin
   for i:=0 to mc.ModuleObjects.Count-1 do
    begin
      if THPCBComponent(mc.ModuleObjects[i]).ObjType='P' then
          if (THPCBPad(mc.ModuleObjects[i]).Number=StrToInt(pin))
                 then THPCBPad(mc.ModuleObjects[i]).Name.Text:=name;
    end;
end;



function CreatePCBModule(inode:IXMLNode):THPCBModule;
var lNode,tNode,ttNode:iXMLNode;
    AttachedPatternName:string;
    i:integer;

function FindPatternMultilayerSection(iNode:IXMLNode;var iPatGraphRefName:string):IXMLNode;
var lNode,pNode:IXMLNode;
    patName:string;
begin
   result:=nil;
   pNode:=iNode; //pattern;
   lNode:=iNode;
   if lNode.NodeName='compDef' then // calling from library  conversion we need to find pattern
    begin
        patName:=ValidateName(TrimLeft(lNode.Attributes['Name']));
        if Assigned(lNode.ChildNodes.FindNode('attachedPattern')) then
            patName:=ValidateName(Trim(lNode.ChildNodes.FindNode('attachedPattern').ChildNodes.FindNode('patternName').Attributes['Name']));
        lNode:=FindModulePatternDefName(lNode.ParentNode,patName);
        pNode:=lNode; //pattern;
    end;
   lNode:=nil;
   if Assigned(pNode) then
   begin
       lNode:=pNode.ChildNodes.FindNode('multiLayer');  //Old file format
   end;
   iPatGraphRefName:='';  //default
   if Assigned(lNode) then result:=lNode
     else
      begin
       // New file format
        if assigned(iNode.ChildNodes.FindNode('patternGraphicsNameRef')) then
        begin
              iPatGraphRefName:=iNode.ChildNodes.FindNode('patternGraphicsNameRef').Attributes['Name'];
        end;
///////////////////////////////////////////////////////////////////////
//        lNode:=iNode.ChildNodes.FindNode('patternGraphicsDef');  before
//        Fixed 02/08, Sergeys imput file format
//        Did it work before  ????
//        lNode:=pNode.ChildNodes.FindNode('patternGraphicsDef');  Nw for some files
//////////////////////////////////////////////////////////////////////
        if Assigned(iNode.ChildNodes.FindNode('patternGraphicsDef')) then
             lNode:=iNode.ChildNodes.FindNode('patternGraphicsDef')
        else
             lNode:=pNode.ChildNodes.FindNode('patternGraphicsDef');
        if iPatGraphRefName='' then  // no patern delection, the first is actual...
               begin
                   if Assigned(lnode) then
                   begin
                     result:=lNode.ChildNodes.FindNode('multiLayer');
                     lNode:=nil;
                   end;
               end;
        while Assigned(lNode) do   // selected by name
           begin
              if lNode.NodeName='patternGraphicsDef' then
                begin
                    if lNode.ChildNodes.FindNode('patternGraphicsNameDef').Attributes['Name']=iPatGraphRefName then
                      begin
                        result:=lNode.ChildNodes.FindNode('multiLayer');
                        lNode:=nil;
                      end
                      else
                        lNode:=lNode.NextSibling;
                end
                else
                  lNode:=lNode.NextSibling;
           end;
      end;
end;

begin
   PCBModule:=THPCBModule.Create(TrimLeft(iNode.ChildNodes.FindNode('originalName').Attributes['Name']));
   StatusBar.SimpleText:='Creating Component : '+PCBModule.Name.Text;
   lNode:=iNode;
   lNode:= FindPatternMultilayerSection(lNode,PCBModule.PatGraphRefName);
   if Assigned(lNode) then
   begin
           tNode:=lNode;
           tNode:=tNode.ChildNodes.First;
           while Assigned(tNode) do
            begin
              if (tNode.NodeName='pad') then PCBModule.ModuleObjects.Add(CreatePCBPad(tNode));
              if (tNode.NodeName='via') then PCBModule.ModuleObjects.Add(CreateVia(tNode));
             tNode := tNode.NextSibling;
            end;
   end;
   lNode:=lNode.ParentNode;
   lNode:=lNode.ChildNodes.FindNode('layerContents');
   while Assigned(lNode) do
   begin
            if (lNode.NodeName='layerContents') then
                          DoLayerContentsObjects(lNode,PCBModule.ModuleObjects);
            lNode:=lNode.NextSibling
   end;
  // map pins
   lNode:=FindPinMap(iNode);
   if Assigned(lNode) and (lnode.ChildNodes.Count>0) then
                 begin
                            for i:=0 to ((lnode.ChildNodes.Count-1) div 2) do
                              begin
                                 if (lNode.ChildNodes[i*2].NodeName='padNum') then
                                        SetPadName(lNode.ChildNodes[i*2].Text,lNode.ChildNodes[i*2+1].Attributes['Name'],PCBModule);
                               end;
                  end;
   result:=PCBModule;
end;
*/

CPCBModule *CreatePCBModule(wxXmlNode *iNode) {
    CPCBModule *pcbModule;

    return pcbModule;
}

/*
procedure DoPCBComponents(iNode:IXMLNode);
var lNode,tNode:IXMLNode;
    i,l:integer;
    mc:THPCBModule;
    cn,so,si:string;

function FindCompDefName(iNode:IXMLNode;iName:string):IXMLNode;
var lNode:IXMLNode;
begin
   result:=nil;
   lNode:=iNode.ChildNodes.FindNode('compDef');
   while Assigned(lNode) do
     begin
         if (lNode.NodeName= 'compDef') then
         begin
                if ValidateName(lNode.Attributes['Name'])=iName then
                     begin
                       result:=lNode;
                       lNode:=nil;
                     end;
         end;
         if Assigned(lNode) then lNode := lNode.NextSibling;
     end;
end;


procedure SetTextProperty(iNode:IXMLNode;var tv:HTextValue; iPatGraphRefName:string; xmlName:string);
var tNode,t1NOde:IXMLNode;
    pn,n:string;
//    i:integer;
begin
//     if iPatGraphRefName<>'' then
{  iNode is pattern now}
     tNode:=iNode;
     t1Node:=iNode;
     n:=xmlName;
     if Assigned (tNode.ChildNodes.FindNode('patternGraphicsNameRef')) then // new file foramat version
        begin
          pn:=trim(tNode.ChildNodes.FindNode('patternGraphicsNameRef').Attributes['Name']);
          tNode:=tNode.ChildNodes.FindNode('patternGraphicsRef');
          while Assigned(tNode) do
            begin
              if tNode.NodeName='patternGraphicsRef' then
                  begin
                    if Assigned(tNode.ChildNodes.FindNode('patternGraphicsNameRef')) then
                        begin
                           if (tNode.ChildNodes.FindNode('patternGraphicsNameRef').Attributes['Name']=pn) then
                            begin
                              t1Node:=tNode;  // find correct section with same name.
                              n:=n+' '+trim(tv.Text); // changed in new file version.....
                              tNode:=nil;
                            end;
                        end;
                  end;
              if assigned(tNode) then tNode:=tNode.NextSibling;
            end;
        end;
     // old version and compatibile fr both from this point
     tNode:=t1Node.ChildNodes.FindNode('attr');
     while (Assigned(tNode) and (trim(tNode.Attributes['Name'])<>n)) do tNode:=tNode.NextSibling;
     if Assigned(tNode) then
     begin
        t1Node:=tNode;
        SetTextParameters(tNode,tv,PCB.DefaultMeasurementUnit);
     end;
end;

begin
   lNode:=iNode.ChildNodes.First;
   while  Assigned(lNode) do
     begin
       mc:=nil;
       if lNode.NodeName='pattern' then
       begin
            cn:=lNode.ChildNodes.FindNode('patternRef').Attributes['Name'];
            cn:=ValidateName(cn);
            tNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('library');
            if ((Assigned(tNode)) and (length(cn)>0)) then
              begin    //
                  tNode:=FindModulePatternDefName(tNode,cn);
                  if Assigned(tNode) then mc:=CreatePCBModule(tNode);
             end;
             if Assigned(mc) then
              begin
                mc.compRef:=cn; // default - in new version of file it is updated later....
                tNode:=lNode.ChildNodes.FindNode('refDesRef');
                if Assigned(tNode) then
                   begin
                     mc.Name.Text:=tNode.Attributes['Name'];
                     SetTextProperty(lNode,mc.Name,mc.PatGraphRefName,'RefDes');
                     SetTextProperty(lNode,mc.Value,mc.PatGraphRefName,'Value');
                   end;
                tNode:=lNode.ChildNodes.FindNode('pt');
                if Assigned(tNode) then SetPosition(tNode.Text,PCB.DefaultMeasurementUnit,mc.PositionX,mc.PositionY);
                tNode:=lNode.ChildNodes.FindNode('rotation');
                if Assigned(tNode) then  mc.Rotation:=StrToInt1Units(TrimLeft(tNode.Text));
                tNode:=lNode.ChildNodes.FindNode('isFlipped');
                if Assigned(tNode) then if (Trim(tNode.Text)='True') then mc.Mirror:=1;
                tNode:=iNode;
                while (tNode.NodeName<>'www.lura.sk') do tNode:=tNode.ParentNode;
                tNode:=tNode.ChildNodes.FindNode('netlist');
                 if Assigned(tNode) then
                  begin
                     tNode:=tNode.ChildNodes.FindNode('compInst');
                      while (Assigned(tNode)) do
                        begin
                          if (tNode.Attributes['Name']=mc.Name.Text) then
                              begin
                                 if Assigned(tNode.ChildNodes.FindNode('compValue')) then
                                     mc.Value.Text:=Trim(tNode.ChildNodes.FindNode('compValue').Attributes['Name']);
                                 if Assigned(tNode.ChildNodes.FindNode('compRef')) then
                                     mc.compRef:=Trim(tNode.ChildNodes.FindNode('compRef').Attributes['Name']);
                                 tNode:=nil;
                              end
                          else
                          tNode := tNode.NextSibling;
                        end;
                   end;

                  // map pins
                  tNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('library');
                     tNode:=FindCompDefName(tNode,mc.compRef);
                     if assigned(tNode) then
                      begin
                        tNode:=FindPinMap(tNode);
                        if Assigned(tNode) and (tnode.ChildNodes.Count>0) then
                         begin
                            for i:=0 to ((tnode.ChildNodes.Count-1) div 2) do
                              begin
                                 if (tNode.ChildNodes[i*2].NodeName='padNum') then
                                        SetPadName(tNode.ChildNodes[i*2].Text,tNode.ChildNodes[i*2+1].Attributes['Name'],mc);
                               end;
                         end;
                      end;

               PCB.PCBComponents.Add(mc);
              end;
     end
     else
       if (lNode.NodeName='via') then PCB.PCBComponents.Add(CreateVia(lNode));
       lNode := lNode.NextSibling;
     end;
end;
*/

void DoPCBComponents(wxXmlNode *iNode, CPCB *pcb) {
}

/*
procedure DoPCBNet(iNode:IXMLNode);
var lNode:iXMLNode;
    s1,s2:string;

begin
    Net:=THNet.Create(trim(iNode.Attributes['Name']));
    lNode:=iNode.ChildNodes.FindNode('node');
    while Assigned(lNode) do
      begin
        s2:=TrimLeft(lNode.Attributes['Name']);
        s1:='';
        while ((length(s2)>0) and (s2[1]<>' ')) do
           begin
            s1:=s1+s2[1];
            s2:=copy(s2,2);
           end;
         NetNode:=THNetNode.Create;
         NetNode.CompRef:=trim(s1);
         NetNode.PinRef:=trim(s2);
         Net.NetNodes.Add(NetNode);
         lNode:=lNode.NextSibling;
      end;
    PCB.PCBNetlist.Add(Net);
end;
*/

void DoPCBNet(wxXmlNode *iNode, CPCB *pcb) {
    wxString propValue, s1, s2;
    CNet *net;
    CNetNode *netNode;
    wxXmlNode *lNode;

    iNode->GetPropVal(wxT("Name"), &propValue);
    propValue.Trim(false);
    propValue.Trim(true);
    net = new CNet(propValue);

    lNode = FindNode(iNode->GetChildren(), wxT("node"));
    while (lNode) {
        lNode->GetPropVal(wxT("Name"), &s2);
        s2.Trim(false);
        s1 = wxEmptyString;
        while (s2.Len() > 0 && s2[0] != ' ') {
            s1 = s1 + s2[0];
            s2.Mid(1);
        }

        netNode = new CNetNode;
        s1.Trim(false);
        s1.Trim(true);
        netNode->m_compRef = s1;

        s2.Trim(false);
        s2.Trim(true);
        netNode->m_pinRef = s2;
        net->m_netNodes.Add(netNode);
        lNode = lNode->GetNext();
    }
    pcb->m_pcbNetlist.Add(net);
}

/*
procedure ConnectPinToNet(cr,pr,netname:string);
var i,j:integer;
    cp:THPCBPad  ;
begin
       for i:=0 to PCB.PCBComponents.Count-1 do
         begin
          with THPCBComponent(PCB.PCBComponents[i]) do
            begin
               if ((ObjType='M') and (THPCBModule(PCB.PCBComponents[i]).Name.Text=cr)) then
                     with THPCBModule(PCB.PCBComponents[i]).ModuleObjects do
                     begin
                       for j:=0 to Count-1 do
                       begin
                         if (THPCBComponent(Items[j]).ObjType='P') then
                           begin
                             cp:=THPCBPad(Items[j]);
                             if (cp.Name.text=pr) then
                                     THPCBPad(Items[j]).Net:=netname;
                           end;
                       end;
                     end;
            end;
        end;
end;
*/

void ConnectPinToNet(CPCB *pcb, wxString cr, wxString pr, wxString netname) {
    CPCBModule *module;
    CPCBPad *cp;
    int i, j;

    for (i = 0; i < (int)pcb->m_pcbComponents.GetCount(); i++) {
        module = (CPCBModule *)pcb->m_pcbComponents[i];
        if (module->m_objType == 'M' && module->m_name.text == cr) {
            for (j = 0; j < (int)module->m_moduleObjects.GetCount(); j++) {
                if (module->m_moduleObjects[j]->m_objType == 'P') {
                    cp = (CPCBPad *)module->m_moduleObjects[j];
                    if (cp->m_name.text == pr)
                        cp->m_net = netname;
                }
            }
        }
    }
}

/*
procedure MapLayer(iNode:IXMLNode;PCB:THPCB);
var lname:string;
    KiCadLayer,Index:integer;
{ KiCad layers
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
26 ECO1 layer (Other layer used for general drawings)       // BUG
26 ECO2 layer (Other layer used for general drawings)       // BUG      27
27 Edge layer. Items on Edge layer are seen on all layers   // BUG     28
}
begin
  lName:=aNode.Attributes['Name'];
  lname:=UpperCase(lName);
  KiCadLayer:=24; // defaullt
  if lname='TOP ASSY'  then  ; //?
  if lname='TOP SILK'  then KiCadLayer:=21;
  if lname='TOP PASTE' then KiCadLayer:=19;
  if lname='TOP MASK'  then KiCadLayer:=23;
  if lname='TOP'       then KiCadLayer:=15;
  if lname='BOTTOM'    then KiCadLayer:=0;
  if lname='BOT MASK'  then KiCadLayer:=22;
  if lname='BOT PASTE' then KiCadLayer:=18;
  if lname='BOT SILK'  then KiCadLayer:=20;
  if lname='BOT ASSY'  then  ; //?
  if lname='BOARD'     then KiCadLayer:=28;
  Index:=StrToInt(iNode.ChildNodes.FindNode('layerNum').Text);
  PCB.LayersMap[Index]:=KiCadLayer;
end;
*/

/* KiCad layers
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
26 ECO1 layer (Other layer used for general drawings)       // BUG
26 ECO2 layer (Other layer used for general drawings)       // BUG      27
27 Edge layer. Items on Edge layer are seen on all layers   // BUG     28
*/
void MapLayer(wxXmlNode *iNode, CPCB *pcb) {
    wxString lName;
    int KiCadLayer;
    long num;

    iNode->GetPropVal(wxT("Name"), &lName);
    lName = lName.MakeLower();
    KiCadLayer = 24; // defaullt
    if (lName == wxT("TOP ASSY"))  {} //?
    if (lName == wxT("TOP SILK"))  KiCadLayer = 21;
    if (lName == wxT("TOP PASTE")) KiCadLayer = 19;
    if (lName == wxT("TOP MASK"))  KiCadLayer = 23;
    if (lName == wxT("TOP"))       KiCadLayer = 15;
    if (lName == wxT("BOTTOM"))    KiCadLayer = 0;
    if (lName == wxT("BOT MASK"))  KiCadLayer = 22;
    if (lName == wxT("BOT PASTE")) KiCadLayer = 18;
    if (lName == wxT("BOT SILK"))  KiCadLayer = 20;
    if (lName == wxT("BOT ASSY"))  {} //?
    if (lName == wxT("BOARD"))     KiCadLayer = 28;
    FindNode(iNode->GetChildren(), wxT("layerNum"))->GetNodeContent().ToLong(&num);
    pcb->m_layersMap[(int)num] = KiCadLayer;
}

/*
begin
  PCB:=THPCB.Create;
  try
   XMLDoc:=TXMLDocument.Create(XMLFileNAme);
   XMLDoc.Active := True;
   // Defaut measurement units
    aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('asciiHeader');
    if Assigned(aNode) then
    begin
      aNode:=aNode.ChildNodes.FindNode('fileUnits');
      if Assigned(aNode) then
          PCB.DefaultMeasurementUnit:=trim(aNode.Text);
    end;
    // Layers mapping
    aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('pcbDesign');
    if Assigned(aNode) then
    begin
      aNode:=aNode.ChildNodes.FindNode('layerDef');
      while Assigned(aNode) do
      begin
          if (aNode.NodeName='layerDef') then  MapLayer(aNode,PCB);
          aNode:=aNode.NextSibling;
      end;
    end;
    // NETLIST
    StatusBar.SimpleText:='Loading NETLIST ';
    aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('netlist');
    if Assigned(aNode) then
    begin
      aNode:=aNode.ChildNodes.FindNode('net');
       while Assigned(aNode) do
         begin
            DoPCBNet(aNode);
            aNode := aNode.NextSibling;
            Application.ProcessMessages;
         end;
      end;
    //BOARD FILE
    StatusBar.SimpleText:='Loading BOARD DEFINITION ';
    aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('pcbDesign');
    if Assigned(aNode) then
    begin
      // COMPONENTS AND OBJECTS
      aNode:=aNode.ChildNodes.First();
       while Assigned(aNode) do
         begin
           // Components/modules
           if (aNode.NodeName= 'multiLayer') then
                  DoPCBComponents(aNode);
           // objects
           if (aNode.NodeName= 'layerContents') then
                  DoLayerContentsObjects(aNode,PCB.PCBComponents);
           aNode := aNode.NextSibling;
           Application.ProcessMessages;
         end;
       // POSTPROCESS -- SET NETLIST REFERENCES
       StatusBar.SimpleText:='Processing NETLIST ';
       for i:=0 to PCB.PCBNetlist.Count-1 do
        begin
          with THNet(PCB.PCBNetlist[i]) do
            begin
              for j:=0 to NetNodes.Count-1 do
                begin
                   cr:=trim(THNetNode(NetNodes[j]).CompRef);
                   pr:=trim(THNetNode(NetNodes[j]).PinRef);
                   ConnectPinToNet(cr,pr,THNet(PCB.PCBNetlist[i]).Name);
                end;
            end;
        end;
       // POSTPROCESS -- FLIP COMPONENTS
       for i:=0 to PCB.PCBComponents.Count-1 do
        begin
          if THPCBComponent(PCB.PCBComponents[i]).ObjType='M' then THPCBModule(PCB.PCBComponents[i]).Flip;
        end;
       // POSTPROCESS -- SET/OPTIMIZE NEW PCB POSITION
       StatusBar.SimpleText:='Optimizing BOARD POSITION ';
       PCB.SizeX:=10000000;PCB.SizeY:=0;
       for i:=0 to PCB.PCBComponents.Count-1 do
         begin
          with THPCBComponent(PCB.PCBComponents[i]) do
            begin
               if (PositionY<PCB.SizeY) then PCB.SizeY:=PositionY; // max Y
               if (PositionX<PCB.SizeX) and (PositionX>0)then PCB.SizeX:=PositionX; // Min X
            end;
        end;
       PCB.SizeY:=PCB.SizeY-10000;
       PCB.SizeX:=PCB.SizeX-10000;
       StatusBar.SimpleText:=' POSITIONING POSTPROCESS ';
       for i:=0 to PCB.PCBComponents.Count-1 do
          THPCBComponent(PCB.PCBComponents[i]).SetPosOffset(-PCB.SizeX, -PCB.SizeY);

       PCB.SizeX:=0;PCB.SizeY:=0;
       for i:=0 to PCB.PCBComponents.Count-1 do
         begin
          with THPCBComponent(PCB.PCBComponents[i]) do
            begin
               if (PositionY<PCB.SizeY) then PCB.SizeY:=PositionY;
               if (PositionX>PCB.SizeX) then PCB.SizeX:=PositionX;
            end;
        end;
       // SHEET SIZE CALCULATION
       PCB.SizeY:=-PCB.SizeY; // is in absolute units
       PCB.SizeX:=PCB.SizeX+10000;
       PCB.SizeY:=PCB.SizeY+10000;
// A4 is minimum $Descr A4 11700 8267
       if PCB.SizeX<11700 then PCB.SizeX:=11700;
       if PCB.SizeY<8267 then PCB.SizeY:=8267;
    end
    else
    begin
      // LIBRARY FILE
       StatusBar.SimpleText:='Processing LIBRARY FILE ';
       aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('library');
       if Assigned(aNode) then
       begin
         aNode:=aNode.ChildNodes.FindNode('compDef');
          while Assigned(aNode) do
            begin
              StatusBar.SimpleText:='Processing COMPONENTS ';
              if (aNode.NodeName= 'compDef') then
                    PCB.PCBComponents.Add(CreatePCBModule(aNode));
              aNode := aNode.NextSibling;
           Application.ProcessMessages;
           end;
       end;
    end;
   XMLDoc.Active := False;
  finally
  end;
  result:=PCB;
end;


end.
*/

CPCB ProcessXMLtoPCBLib(wxStatusBar* statusBar, wxString XMLFileName, wxString *actualConversion) {
    CPCB pcb;
    wxXmlDocument xmlDoc;
    wxXmlNode *aNode;
    CNet *net;
    CPCBComponent *comp;
    wxString cr, pr;
    int i, j;

    if (!xmlDoc.Load(XMLFileName)) return pcb;

    // Defaut measurement units
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("asciiHeader"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("fileUnits"));
        if (aNode) {
            pcb.m_defaultMeasurementUnit = aNode->GetNodeContent();
            pcb.m_defaultMeasurementUnit.Trim(true);
            pcb.m_defaultMeasurementUnit.Trim(false);
        }
    }

    // Layers mapping
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("pcbDesign"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("layerDef"));
        while (aNode) {
            if (aNode->GetName() == wxT("layerDef"))
                MapLayer(aNode, &pcb);
            aNode = aNode->GetNext();
        }
    }

    // NETLIST
    statusBar->SetStatusText(wxT("Loading NETLIST "));
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("netlist"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("net"));
        while (aNode) {
            DoPCBNet(aNode, &pcb);
            aNode = aNode->GetNext();
        }
    }

    //BOARD FILE
    statusBar->SetStatusText(wxT("Loading BOARD DEFINITION "));
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("pcbDesign"));
    if (aNode) {
        // COMPONENTS AND OBJECTS
        aNode = aNode->GetChildren();
        while (aNode) {
            // Components/modules
            if (aNode->GetName() == wxT("multiLayer"))
                DoPCBComponents(aNode, &pcb);
            // objects
            if (aNode->GetName() == wxT("layerContents"))
                DoLayerContentsObjects(aNode, &pcb.m_pcbComponents, statusBar);
            aNode = aNode->GetNext();
        }

        // POSTPROCESS -- SET NETLIST REFERENCES
        statusBar->SetStatusText(wxT("Processing NETLIST "));
        for (i = 0; i < (int)pcb.m_pcbNetlist.GetCount(); i++) {
            net = pcb.m_pcbNetlist[i];
            for (j = 0; j < (int)net->m_netNodes.GetCount(); j++) {
                cr = net->m_netNodes[j]->m_compRef;
                cr.Trim(false);
                cr.Trim(true);
                pr = net->m_netNodes[j]->m_pinRef;
                pr.Trim(false);
                pr.Trim(true);
                ConnectPinToNet(&pcb, cr, pr, net->m_name);
            }
        }

        // POSTPROCESS -- FLIP COMPONENTS
        for (i = 0; i < (int)pcb.m_pcbComponents.GetCount(); i++) {
            if (pcb.m_pcbComponents[i]->m_objType == 'M')
                ((CPCBModule *)pcb.m_pcbComponents[i])->Flip();
        }

        // POSTPROCESS -- SET/OPTIMIZE NEW PCB POSITION
        statusBar->SetStatusText(wxT("Optimizing BOARD POSITION "));
        pcb.m_sizeX = 10000000; pcb.m_sizeY = 0;
        for (i = 0; i < (int)pcb.m_pcbComponents.GetCount(); i++) {
            comp = pcb.m_pcbComponents[i];
            if (comp->m_positionY < pcb.m_sizeY) pcb.m_sizeY = comp->m_positionY; // max Y
            if (comp->m_positionX < pcb.m_sizeX && comp->m_positionX > 0) pcb.m_sizeX = comp->m_positionX; // Min X
        }
        pcb.m_sizeY -= 10000;
        pcb.m_sizeX -= 10000;
        statusBar->SetStatusText(wxT(" POSITIONING POSTPROCESS "));
        for (i = 0; i < (int)pcb.m_pcbComponents.GetCount(); i++)
            pcb.m_pcbComponents[i]->SetPosOffset(-pcb.m_sizeX, -pcb.m_sizeY);

        pcb.m_sizeX = 0; pcb.m_sizeY = 0;
        for (i = 0; i < (int)pcb.m_pcbComponents.GetCount(); i++) {
            comp = pcb.m_pcbComponents[i];
            if (comp->m_positionY < pcb.m_sizeY) pcb.m_sizeY = comp->m_positionY; // max Y
            if (comp->m_positionX < pcb.m_sizeX) pcb.m_sizeX = comp->m_positionX; // Min X
        }

        // SHEET SIZE CALCULATION
        pcb.m_sizeY = -pcb.m_sizeY; // it is in absolute units
        pcb.m_sizeX += 10000;
        pcb.m_sizeY += 10000;
        // A4 is minimum $Descr A4 11700 8267
        if (pcb.m_sizeX < 11700) pcb.m_sizeX = 11700;
        if (pcb.m_sizeY < 8267) pcb.m_sizeY = 8267;
    }
    else {
        // LIBRARY FILE
        statusBar->SetStatusText(wxT("Processing LIBRARY FILE "));
        aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("library"));
        if (aNode) {
            aNode = FindNode(aNode->GetChildren(), wxT("compDef"));
            while (aNode) {
                statusBar->SetStatusText(wxT("Processing COMPONENTS "));
                if (aNode->GetName() == wxT("compDef"))
                    pcb.m_pcbComponents.Add(CreatePCBModule(aNode));

                aNode = aNode->GetNext();
            }
        }
    }

    return pcb;
}
