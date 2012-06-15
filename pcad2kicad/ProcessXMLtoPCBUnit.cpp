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

#include <common.h>
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
*/

CPCBPadViaShape *CreatePCBPadShape(wxXmlNode *iNode, CPCB *pcb, wxString actualConversion) {
    wxString str, s;
    long num;
    int minX, maxX, minY, maxY, x, y;
    wxXmlNode *lNode;

    CPCBPadViaShape *pcbPadShape = new CPCBPadViaShape;

    lNode = FindNode(iNode->GetChildren(), wxT("padShapeType"));
    if (lNode) {
        str = lNode->GetNodeContent();
        str.Trim(false);
        pcbPadShape->m_shape = str;
    }

    lNode = FindNode(iNode->GetChildren(), wxT("layerNumRef"));
    if (lNode) {
        lNode->GetNodeContent().ToLong(&num);
        pcbPadShape->m_PCadLayer = (int)num;
    }

    pcbPadShape->m_KiCadLayer = pcb->m_layersMap[pcbPadShape->m_PCadLayer];
    if (pcbPadShape->m_shape == wxT("Oval") ||
        pcbPadShape->m_shape == wxT("Rect") ||
        pcbPadShape->m_shape == wxT("Ellipse") ||
        pcbPadShape->m_shape == wxT("RndRect"))
    {
        lNode = FindNode(iNode->GetChildren(), wxT("shapeWidth"));
        if (lNode) SetWidth(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &pcbPadShape->m_width, actualConversion);
        lNode = FindNode(iNode->GetChildren(), wxT("shapeHeight"));
        if (lNode) SetWidth(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &pcbPadShape->m_height, actualConversion);
    }

    if (pcbPadShape->m_shape == wxT("Polygon")) {
        // aproximation to simplier pad shape .....
        lNode = FindNode(iNode->GetChildren(), wxT("shapeOutline"));
        if (lNode) lNode = FindNode(lNode->GetChildren(), wxT("pt"));
        minX = 0; maxX = 0; minY = 0; maxY = 0;
        while (lNode) {
            s = lNode->GetNodeContent();
            SetPosition(s, pcb->m_defaultMeasurementUnit, &x, &y, actualConversion);
            if (minX > x) minX = x;
            if (maxX < x) maxX = x;
            if (minY > y) minY = y;
            if (maxY < y) maxY = y;
            lNode = lNode->GetNext();
        }

        pcbPadShape->m_width = maxX - minX;
        pcbPadShape->m_height = maxY - minY;
    }

    return pcbPadShape;
}

/*
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
*/

CPCBPadViaShape *CreatePCBViaShape(wxXmlNode *iNode, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString str;
    long num;
    CPCBPadViaShape *pcbViaShape = new CPCBPadViaShape;

    lNode = FindNode(iNode->GetChildren(), wxT("viaShapeType"));
    if (lNode) {
        str = lNode->GetNodeContent();
        str.Trim(false);
        pcbViaShape->m_shape = str;
    }

    lNode = FindNode(iNode->GetChildren(), wxT("layerNumRef"));
    if (lNode) {
        lNode->GetNodeContent().ToLong(&num);
        pcbViaShape->m_PCadLayer = (int)num;
    }

    pcbViaShape->m_KiCadLayer = pcb->m_layersMap[pcbViaShape->m_PCadLayer];
    lNode = FindNode(iNode->GetChildren(), wxT("shapeWidth"));
    if (lNode) SetWidth(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &pcbViaShape->m_width, actualConversion);

    lNode = FindNode(iNode->GetChildren(), wxT("shapeHeight"));
    if (lNode) SetWidth(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &pcbViaShape->m_height, actualConversion);

    return pcbViaShape;
}

/*
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
*/

CPCBPad *CreatePCBPad(wxXmlNode *iNode, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    long num;
    wxString propValue, str;
    CPCBPad *pcbPad = new CPCBPad(wxEmptyString);

    pcbPad->m_rotation = 0;
    lNode = FindNode(iNode->GetChildren(), wxT("padNum"));
    if (lNode) {
        lNode->GetNodeContent().ToLong(&num);
        pcbPad->m_number = (int)num;
    }

    lNode = FindNode(iNode->GetChildren(), wxT("padStyleRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        pcbPad->m_name.text = propValue;
    }

    lNode = FindNode(iNode->GetChildren(), wxT("pt"));
    if (lNode) SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                           &pcbPad->m_positionX, &pcbPad->m_positionY, actualConversion);

    lNode = FindNode(iNode->GetChildren(), wxT("rotation"));
    if (lNode) {
        str = lNode->GetNodeContent();
        str.Trim(false);
        pcbPad->m_rotation = StrToInt1Units(str);
    }

    lNode = iNode;
    while (lNode->GetName() != wxT("www.lura.sk"))
        lNode = lNode->GetParent();

    lNode = FindNode(lNode->GetChildren(), wxT("library"));
    lNode = FindNode(lNode->GetChildren(), wxT("padStyleDef"));

    while (true) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        if (propValue == pcbPad->m_name.text) break;
        lNode = lNode->GetNext();
    }

    lNode = FindNode(lNode->GetChildren(), wxT("holeDiam"));
    if (lNode) SetWidth(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &pcbPad->m_hole, actualConversion);

    lNode = lNode->GetParent();
    lNode = FindNode(lNode->GetChildren(), wxT("padShape"));

    while (lNode) {
        if  (lNode->GetName() == wxT("padShape")) {
            // we support only Pads on specific layers......
            // we do not support pads on "Plane", "NonSignal" , "Signal" ... layerr
            if (FindNode(lNode->GetChildren(), wxT("layerNumRef")))
                pcbPad->m_shapes.Add(CreatePCBPadShape(lNode, pcb, actualConversion));
        }

        lNode = lNode->GetNext();
    }

    return pcbPad;
}

/*
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
*/

CPCBVia *CreateVia(wxXmlNode *iNode, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode, *tNode;
    wxString propValue;
    CPCBVia *pcbVia = new CPCBVia;

    pcbVia->m_rotation = 0;
    lNode = FindNode(iNode->GetChildren(), wxT("viaStyleRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        pcbVia->m_name.text = propValue;
    }

    lNode = FindNode(iNode->GetChildren(), wxT("pt"));
    if (lNode) SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                           &pcbVia->m_positionX, &pcbVia->m_positionY, actualConversion);

    lNode = FindNode(iNode->GetChildren(), wxT("netNameRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        pcbVia->m_net = propValue;
    }

    lNode = iNode;
    while (lNode->GetName() != wxT("www.lura.sk"))
        lNode = lNode->GetParent();

    lNode = FindNode(lNode->GetChildren(), wxT("library"));
    lNode = FindNode(lNode->GetChildren(), wxT("viaStyleDef"));
    if (lNode) {
        while (lNode) {
            lNode->GetPropVal(wxT("Name"), &propValue);
            if (propValue == pcbVia->m_name.text) break;
            lNode = lNode->GetNext();
        }
    }

    if (lNode) {
        tNode = lNode;
        lNode = FindNode(tNode->GetChildren(), wxT("holeDiam"));
        if (lNode) SetWidth(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &pcbVia->m_hole, actualConversion);

        lNode = FindNode(tNode->GetChildren(), wxT("viaShape"));
        while (lNode) {
            if  (lNode->GetName() == wxT("viaShape")) {
                // we support only Vias on specific layers......
                // we do not support vias on "Plane", "NonSignal" , "Signal" ... layerr
                if (FindNode(lNode->GetChildren(), wxT("layerNumRef")))
                    pcbVia->m_shapes.Add(CreatePCBViaShape(lNode, pcb, actualConversion));
            }

            lNode = lNode->GetNext();
        }
    }

    return pcbVia;
}

/*
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
*/

CPCBLine *CreateComponentLine(wxXmlNode *iNode, int l, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString propValue;
    CPCBLine *componentLine = new CPCBLine;

    componentLine->m_PCadLayer = l;
    componentLine->m_KiCadLayer = pcb->m_layersMap[componentLine->m_PCadLayer];
    componentLine->m_positionX = 0;
    componentLine->m_positionY = 0;
    componentLine->m_toX = 0;
    componentLine->m_toY = 0;
    componentLine->m_width = 0;
    lNode = FindNode(iNode->GetChildren(), wxT("pt"));
    if (lNode)
        SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                    &componentLine->m_positionX, &componentLine->m_positionY, actualConversion);

    lNode = lNode->GetNext();
    if (lNode)
        SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                    &componentLine->m_toX, &componentLine->m_toY, actualConversion);

    lNode = FindNode(iNode->GetChildren(), wxT("width"));
    if (lNode) SetWidth(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &componentLine->m_width, actualConversion);

    lNode = FindNode(iNode->GetChildren(), wxT("netNameRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        componentLine->m_net = propValue;
    }

    return componentLine;
}

/*
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
*/

CPCBText *CreateComponentText(wxXmlNode *iNode, int l, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString str;
    CPCBText *componentText = new CPCBText;

    componentText->m_PCadLayer = l;
    componentText->m_KiCadLayer = pcb->m_layersMap[componentText->m_PCadLayer];
    componentText->m_positionX = 0;
    componentText->m_positionY = 0;
    componentText->m_name.mirror = 0;      //Normal, not mirrored
    lNode = FindNode(iNode->GetChildren(), wxT("pt"));
    if (lNode) SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                           &componentText->m_positionX, &componentText->m_positionY, actualConversion);
    lNode = FindNode(iNode->GetChildren(), wxT("rotation"));
    if (lNode) {
        str = lNode->GetNodeContent();
        str.Trim(false);
        componentText->m_rotation = StrToInt1Units(str);
    }

    lNode = FindNode(iNode->GetChildren(), wxT("value"));
    if (lNode)
        componentText->m_name.text = lNode->GetNodeContent();

    lNode = FindNode(iNode->GetChildren(), wxT("isFlipped"));
    if (lNode) {
        str = lNode->GetNodeContent();
        str.Trim(false);
        str.Trim(true);
        if (str == wxT("True")) componentText->m_name.mirror = 1;
    }

    lNode = FindNode(iNode->GetChildren(), wxT("textStyleRef"));
    if (lNode) SetFontProperty(lNode, &componentText->m_name, pcb->m_defaultMeasurementUnit, actualConversion);

    return componentText;
}

/*
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
*/

CPCBPolygon *CreateComponentPolygon(wxXmlNode *iNode, int PCadLayer) {
    CPCBPolygon *componentPolygon = new CPCBPolygon;

    return componentPolygon;
}

/*
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
*/

CPCBCopperPour *CreateComponentCopperPour(wxXmlNode *iNode, int PCadLayer) {
    CPCBCopperPour *componentCopperPour = new CPCBCopperPour;

    return componentCopperPour;
}

/*
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
*/

CPCBCutout *CreateComponentCutout(wxXmlNode *iNode, int PCadLayer) {
    CPCBCutout *componentCutout = new CPCBCutout;

    return componentCutout;
}

/*
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
*/

CPCBArc *CreateComponentArc(wxXmlNode *iNode, int l, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    double r, a;
    CPCBArc *componentArc = new CPCBArc;

    componentArc->m_PCadLayer = l;
    componentArc->m_KiCadLayer = pcb->m_layersMap[componentArc->m_PCadLayer];
    SetWidth(FindNode(iNode->GetChildren(), wxT("width"))->GetNodeContent(),
             pcb->m_defaultMeasurementUnit, &componentArc->m_width, actualConversion);

    if (iNode->GetName() == wxT("triplePointArc")) {
        // origin
        lNode = FindNode(iNode->GetChildren(), wxT("pt"));
        if (lNode) SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                               &componentArc->m_positionX, &componentArc->m_positionY, actualConversion);
        lNode = lNode->GetNext();
        if (lNode)
            SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                        &componentArc->m_startX, &componentArc->m_startY, actualConversion);
        // now temporary, it can be fixed later.....
        componentArc->m_angle = 3600;
    }

    if (iNode->GetName() == wxT("arc")) {
        lNode = FindNode(iNode->GetChildren(), wxT("pt"));
        if (lNode) SetPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit,
                               &componentArc->m_positionX, &componentArc->m_positionY, actualConversion);
        lNode = FindNode(iNode->GetChildren(), wxT("radius"));
        r = StrToIntUnits(lNode->GetNodeContent(), ' ', actualConversion);
        a = StrToInt1Units(FindNode(iNode->GetChildren(), wxT("startAngle"))->GetNodeContent());
        componentArc->m_startX = KiROUND(componentArc->m_positionX + r * sin((a - 900.0) * M_PI / 1800.0));
        componentArc->m_startY = KiROUND(componentArc->m_positionY - r * cos((a - 900.0) * M_PI / 1800.0));
        componentArc->m_angle = StrToInt1Units(FindNode(iNode->GetChildren(), wxT("sweepAngle"))->GetNodeContent());
    }

    return componentArc;
}

/*
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

void DoLayerContentsObjects(wxXmlNode *iNode, CPCBComponentsArray *list, wxStatusBar* statusBar, CPCB *pcb, wxString actualConversion) {
    CPCBPolygon *poly;
    wxXmlNode *lNode, *tNode;
    wxString propValue;
    long long i;
    int PCadLayer;
    long num;

    i = 0;
    statusBar->SetStatusText(wxT("Processing LAYER CONTENT OBJECTS "));
    FindNode(iNode->GetChildren(), wxT("layerNumRef"))->GetNodeContent().ToLong(&num);
    PCadLayer = (int)num;
    lNode = iNode->GetChildren();
    while (lNode) {
        i++;
        statusBar->SetStatusText(wxString::Format("Processing LAYER CONTENT OBJECTS :%lld", i));
        if (lNode->GetName() == wxT("line")) list->Add(CreateComponentLine(lNode, PCadLayer, pcb, actualConversion));
        if (lNode->GetName() == wxT("text")) list->Add(CreateComponentText(lNode, PCadLayer, pcb, actualConversion));
        // added  as Sergeys request 02/2008
        if (lNode->GetName() == wxT("attr")) {
            // assign fonts to Module Name,Value,Type,....s
            lNode->GetPropVal(wxT("Name"), &propValue);
            propValue.Trim(false);
            propValue.Trim(true);
            if (propValue == wxT("Type")) {
                tNode = FindNode(lNode->GetChildren(), wxT("textStyleRef"));
                if (tNode) {
                    //TODO: to understand and repair
                    // Alexander Lunev: The string below is commented because there is a strange access
                    //  to pcbModule->m_name (it was global variable in Delphi version of the project).
                    //SetFontProperty(tNode, pcbModule->m_name, pcb->m_defaultMeasurementUnit);
                }
            }
        }

        // added  as Sergeys request 02/2008
        if (lNode->GetName() == wxT("arc")) list->Add(CreateComponentArc(lNode, PCadLayer, pcb, actualConversion));
        if (lNode->GetName() == wxT("triplePointArc")) list->Add(CreateComponentArc(lNode, PCadLayer, pcb, actualConversion));
        if (lNode->GetName() == wxT("pcbPoly")) list->Add(CreateComponentPolygon(lNode, PCadLayer));
        if (lNode->GetName() == wxT("copperPour95")) {
            poly = CreateComponentCopperPour(lNode, PCadLayer);
            if (poly != NULL) list->Add(poly);
        }

        if (lNode->GetName() == wxT("polyCutOut")) {
            // list of polygons....
            tNode = lNode;
            tNode = FindNode(tNode->GetChildren(), wxT("pcbPoly"));
            if (tNode)
                list->Add(CreateComponentCutout(tNode, PCadLayer));
        }

        lNode = lNode->GetNext();
    }
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
*/

wxXmlNode *FindModulePatternDefName(wxXmlNode *iNode, wxString iName) {
    wxXmlNode *result, *lNode;
    wxString propValue1, propValue2;

    result = NULL;
    lNode = FindNode(iNode->GetChildren(), wxT("patternDef"));
    while (lNode) {
        if (lNode->GetName() == wxT("patternDef")) {
            lNode->GetPropVal(wxT("Name"), &propValue1);
            FindNode(lNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue2);
            if (ValidateName(propValue1) == iName ||
                ValidateName(propValue2) == iName) {
                result = lNode;
                lNode = NULL;
            }
        }

        if (lNode) lNode = lNode->GetNext();
    }

    if (result == NULL) {
        lNode = FindNode(iNode->GetChildren(), wxT("patternDefExtended")); // New file format
        while (lNode) {
            if (lNode->GetName() == wxT("patternDefExtended")) {
                lNode->GetPropVal(wxT("Name"), &propValue1);
                if (ValidateName(propValue1) == iName) {
                    result = lNode;
                    lNode = NULL;
                }
            }

            if (lNode) lNode = lNode->GetNext();
        }
    }

    return result;
}

/*
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
*/

void SetPadName(wxString pin, wxString name, CPCBModule *mc) {
    int i;
    long num;

    pin.ToLong(&num);
    for (i = 0; i < (int)mc->m_moduleObjects.GetCount(); i++) {
        if (mc->m_moduleObjects[i]->m_objType == 'P')
            if (((CPCBPad *)mc->m_moduleObjects[i])->m_number == num)
                ((CPCBPad *)mc->m_moduleObjects[i])->m_name.text = name;
    }
}

/*
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
*/

wxXmlNode *FindPatternMultilayerSection(wxXmlNode *iNode, wxString *iPatGraphRefName) {
    wxXmlNode *result, *pNode, *lNode;
    wxString propValue, patName;

    result = NULL;
    pNode = iNode; //pattern;
    lNode = iNode;
    if (lNode->GetName() == wxT("compDef")) { // calling from library  conversion we need to find pattern
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        patName = ValidateName(propValue);
        if (FindNode(lNode->GetChildren(), wxT("attachedPattern"))) {
            FindNode(FindNode(lNode->GetChildren(), wxT("attachedPattern"))->GetChildren(), wxT("patternName"))->GetPropVal(wxT("Name"), &propValue);
            propValue.Trim(false);
            propValue.Trim(true);
            patName = ValidateName(propValue);
        }

        lNode = FindModulePatternDefName(lNode->GetParent(), patName);
        pNode = lNode; //pattern;
    }

    lNode = NULL;
    if (pNode)
       lNode = FindNode(pNode->GetChildren(), wxT("multiLayer"));  //Old file format

    *iPatGraphRefName = wxEmptyString;  //default
    if (lNode) result = lNode;
    else {
        // New file format
        if (FindNode(iNode->GetChildren(), wxT("patternGraphicsNameRef"))) {
            FindNode(iNode->GetChildren(), wxT("patternGraphicsNameRef"))->GetPropVal(wxT("Name"), iPatGraphRefName);
        }
///////////////////////////////////////////////////////////////////////
//        lNode:=iNode.ChildNodes.FindNode('patternGraphicsDef');  before
//        Fixed 02/08, Sergeys input file format
//        Did it work before  ????
//        lNode:=pNode.ChildNodes.FindNode('patternGraphicsDef');  Nw for some files
//////////////////////////////////////////////////////////////////////
        if (FindNode(iNode->GetChildren(), wxT("patternGraphicsDef")))
            lNode = FindNode(iNode->GetChildren(), wxT("patternGraphicsDef"));
        else
            lNode = FindNode(pNode->GetChildren(), wxT("patternGraphicsDef"));

        if (*iPatGraphRefName == wxEmptyString) {  // no patern delection, the first is actual...
            if (lNode) {
                result = FindNode(lNode->GetChildren(), wxT("multiLayer"));
                lNode = NULL;
            }
        }

        while (lNode) {   // selected by name
            if (lNode->GetName() == wxT("patternGraphicsDef")) {
                FindNode(lNode->GetChildren(), wxT("patternGraphicsNameDef"))->GetPropVal(wxT("Name"), &propValue);
                if (propValue == *iPatGraphRefName) {
                    result = FindNode(lNode->GetChildren(), wxT("multiLayer"));
                    lNode = NULL;
                }
                else lNode = lNode->GetNext();
            }
            else lNode = lNode->GetNext();
        }
    }

    return result;
}

/*
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

CPCBModule *CreatePCBModule(wxXmlNode *iNode, wxStatusBar* statusBar, CPCB *pcb, wxString actualConversion) {
    CPCBModule *pcbModule;
    wxXmlNode *lNode, *tNode, *mNode;
    wxString propValue, str;
    int i;

    FindNode(iNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue);
    propValue.Trim(false);
    pcbModule = new CPCBModule(propValue);

    statusBar->SetStatusText(wxT("Creating Component : ") + pcbModule->m_name.text);
    lNode = iNode;
    lNode = FindPatternMultilayerSection(lNode, &pcbModule->m_patGraphRefName);
    if (lNode) {
        tNode = lNode;
        tNode = tNode->GetChildren();
        while (tNode) {
            if (tNode->GetName() == wxT("pad")) pcbModule->m_moduleObjects.Add(CreatePCBPad(tNode, pcb, actualConversion));
            if (tNode->GetName() == wxT("via")) pcbModule->m_moduleObjects.Add(CreateVia(tNode, pcb, actualConversion));
            tNode = tNode->GetNext();
        }
    }

    lNode = lNode->GetParent();
    lNode = FindNode(lNode->GetChildren(), wxT("layerContents"));
    while (lNode) {
        if (lNode->GetName() == wxT("layerContents"))
            DoLayerContentsObjects(lNode, &pcbModule->m_moduleObjects, statusBar, pcb, actualConversion);
        lNode = lNode->GetNext();
    }

    // map pins
    lNode = FindPinMap(iNode);

    if (lNode && lNode->GetDepth() > 0) {
        mNode = lNode->GetChildren();
        for (i = 0; i <= ((tNode->GetDepth() - 1) / 2); i++) {
            if (mNode->GetName() == wxT("padNum")) {
                str = mNode->GetNodeContent();
                mNode = mNode->GetNext();
                mNode->GetPropVal(wxT("Name"), &propValue);
                SetPadName(str, propValue, pcbModule);
                mNode = mNode->GetNext();
            }
        }
    }

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
*/

wxXmlNode *FindCompDefName(wxXmlNode *iNode, wxString iName) {
    wxXmlNode *result = NULL, *lNode;
    wxString propValue;

    lNode = FindNode(iNode->GetChildren(), wxT("compDef"));
    while (lNode) {
        if (lNode->GetName() == wxT("compDef")) {
            lNode->GetPropVal(wxT("Name"), &propValue);
            if (ValidateName(propValue) == iName) {
                result = lNode;
                lNode = NULL;
            }
        }

        if (lNode) lNode = lNode->GetNext();
    }

    return result;
}

/*
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
*/

void SetTextProperty(wxXmlNode *iNode, TTextValue *tv, wxString iPatGraphRefName, wxString xmlName, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *tNode, *t1Node;
    wxString n, pn, propValue, str;

    // iNode is pattern now
    tNode = iNode;
    t1Node = iNode;
    n = xmlName;
    if (FindNode(tNode->GetChildren(), wxT("patternGraphicsNameRef"))) { // new file foramat version
        FindNode(tNode->GetChildren(), wxT("patternGraphicsNameRef"))->GetPropVal(wxT("Name"), &pn);
        pn.Trim(false);
        pn.Trim(true);
        tNode = FindNode(tNode->GetChildren(), wxT("patternGraphicsRef"));
        while (tNode) {
            if (tNode->GetName() == wxT("patternGraphicsRef")) {
                if (FindNode(tNode->GetChildren(), wxT("patternGraphicsNameRef"))) {
                    FindNode(tNode->GetChildren(), wxT("patternGraphicsNameRef"))->GetPropVal(wxT("Name"), &propValue);
                    if (propValue == pn) {
                        t1Node = tNode;  // find correct section with same name.
                        str = tv->text;
                        str.Trim(false);
                        str.Trim(true);
                        n = n + ' ' + str; // changed in new file version.....
                        tNode = NULL;
                    }
                }
            }

            if (tNode) tNode = tNode->GetNext();
        }
    }

    // old version and compatibile fr both from this point
    tNode = FindNode(t1Node->GetChildren(), wxT("attr"));
    while (tNode) {
        tNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        if (propValue == n) break;

        tNode = tNode->GetNext();
    }

    if (tNode)
        SetTextParameters(tNode, tv, pcb->m_defaultMeasurementUnit, actualConversion);
}

/*
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

void DoPCBComponents(wxXmlNode *iNode, CPCB *pcb, wxXmlDocument *xmlDoc, wxString actualConversion, wxStatusBar* statusBar) {
    wxXmlNode *lNode, *tNode, *mNode;
    CPCBModule *mc;
    wxString cn, str, propValue;
    int i;

    lNode = iNode->GetChildren();
    while (lNode) {
        mc = NULL;
        if (lNode->GetName() == wxT("pattern")) {
            FindNode(lNode->GetChildren(), wxT("patternRef"))->GetPropVal(wxT("Name"), &cn);
            cn = ValidateName(cn);
            tNode = FindNode(xmlDoc->GetRoot()->GetChildren(), wxT("library"));
            if (tNode && cn.Len() > 0) {
                tNode = FindModulePatternDefName(tNode, cn);
                if (tNode) mc = CreatePCBModule(tNode, statusBar, pcb, actualConversion);
            }

            if (mc) {
                mc->m_compRef = cn; // default - in new version of file it is updated later....
                tNode = FindNode(lNode->GetChildren(), wxT("refDesRef"));
                if (tNode) {
                    tNode->GetPropVal(wxT("Name"), &mc->m_name.text);
                    SetTextProperty(lNode, &mc->m_name, mc->m_patGraphRefName, wxT("RefDes"), pcb, actualConversion);
                    SetTextProperty(lNode, &mc->m_value, mc->m_patGraphRefName, wxT("Value"), pcb, actualConversion);
                }

                tNode = FindNode(lNode->GetChildren(), wxT("pt"));
                if (tNode) SetPosition(tNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &mc->m_positionX, &mc->m_positionY, actualConversion);

                tNode = FindNode(lNode->GetChildren(), wxT("rotation"));
                if (tNode) {
                    str = tNode->GetNodeContent();
                    str.Trim(false);
                    mc->m_rotation = StrToInt1Units(str);
                }

                tNode = FindNode(lNode->GetChildren(), wxT("isFlipped"));
                if (tNode) {
                    str = tNode->GetNodeContent();
                    str.Trim(false);
                    str.Trim(true);
                    if (str == wxT("True")) mc->m_mirror = 1;
                }

                tNode = iNode;
                while (tNode->GetName() != wxT("www.lura.sk"))
                    tNode = tNode->GetParent();

                tNode = FindNode(tNode->GetChildren(), wxT("netlist"));
                if (tNode) {
                    tNode = FindNode(tNode->GetChildren(), wxT("compInst"));
                    while (tNode) {
                        tNode->GetPropVal(wxT("Name"), &propValue);
                        if (propValue == mc->m_name.text) {
                            if (FindNode(tNode->GetChildren(), wxT("compValue"))) {
                                FindNode(tNode->GetChildren(), wxT("compValue"))->GetPropVal(wxT("Name"), &mc->m_value.text);
                                mc->m_value.text.Trim(false);
                                mc->m_value.text.Trim(true);
                            }

                            if (FindNode(tNode->GetChildren(), wxT("compRef"))) {
                                FindNode(tNode->GetChildren(), wxT("compRef"))->GetPropVal(wxT("Name"), &mc->m_compRef);
                                mc->m_compRef.Trim(false);
                                mc->m_compRef.Trim(true);
                            }

                            tNode = NULL;
                        }
                        else tNode = tNode->GetNext();
                    }
                }

                // map pins
                tNode = FindNode(xmlDoc->GetRoot()->GetChildren(), wxT("library"));
                tNode = FindCompDefName(tNode, mc->m_compRef);
                if (tNode) {
                    tNode = FindPinMap(tNode);
                    if (tNode && tNode->GetDepth() > 0) {
                        mNode = tNode->GetChildren();
                        for (i = 0; i <= ((tNode->GetDepth() - 1) / 2); i++) {
                            if (mNode->GetName() == wxT("padNum")) {
                                str = mNode->GetNodeContent();
                                mNode = mNode->GetNext();
                                mNode->GetPropVal(wxT("Name"), &propValue);
                                SetPadName(str, propValue, mc);
                                mNode = mNode->GetNext();
                            }
                        }
                    }
                }

                pcb->m_pcbComponents.Add(mc);
            }
        }
        else if (lNode->GetName() == wxT("via")) pcb->m_pcbComponents.Add(CreateVia(lNode, pcb, actualConversion));

        lNode = lNode->GetNext();
    }
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
            s2 = s2.Mid(1);
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

CPCB ProcessXMLtoPCBLib(wxStatusBar* statusBar, wxString XMLFileName, wxString actualConversion) {
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
                DoPCBComponents(aNode, &pcb, &xmlDoc, actualConversion, statusBar);
            // objects
            if (aNode->GetName() == wxT("layerContents"))
                DoLayerContentsObjects(aNode, &pcb.m_pcbComponents, statusBar, &pcb, actualConversion);
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
                    pcb.m_pcbComponents.Add(CreatePCBModule(aNode, statusBar, &pcb, actualConversion));

                aNode = aNode->GetNext();
            }
        }
    }

    return pcb;
}
