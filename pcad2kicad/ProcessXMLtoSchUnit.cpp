/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file ProcessXMLtoSchUnit.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/xml/xml.h>
#include <ProcessXMLtoSchUnit.h>
#include <XMLtoObjectCommonProceduresUnit.h>
#include <common.h>

/*
unit ProcessXMLToSCHUnit;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom,
  PCBComponents,SCHComponents,PCBPolygonsUnit,XMLToObjectCommonProceduresUnit;

function ProcessXMLToSCH(StatusBar:TStatusBar;XMLFileNAme:string):THSCH;


implementation


function ProcessXMLToSCH(StatusBar:TStatusBar;XMLFileNAme:string):THSCH;
var
 SCH:THSCH;
 xmlDoc: IxmlDocument;
 ANode,cNode: IXMLNode;
 i,j,k: Integer;
 SCHModule:THSCHModule;
 SCHPin:THSCHPin;
 SCHLine:THSCHLine;
 SCHArc:THSCHArc;
 SCHSymbol:THSCHSymbol;
 SCHJunction:THSCHJunction;
 MaxX,MaxY:Integer;
 IsJunction:boolean;

function CreatePin(inode:IXMLNode):THSCHPin;
begin
    SCHPin:=THSCHPin.Create();
    SCHPin.ObjType:='P';
//    SCHLine.PartNum:=SymbolIndex;
    SCHPin.Number.Text:=iNode.Attributes['Name'];
    SCHPin.pinNum.Text:='0'; // Default
    SCHPin.IsVisible:=0; // Default is not visible
//    SCHPin.pinName.Text:='~'; // Default
    if Assigned(iNode.ChildNodes.FindNode('symPinNum')) then SCHPin.pinNum.Text:=Trim(iNode.ChildNodes.FindNode('symPinNum').Text);
//    SCHPin.pinName.Text:=SCHPin.pinNum.Text; // Default
    if Assigned(iNode.ChildNodes.FindNode('pinName')) then SCHPin.pinName.Text:=Trim(iNode.ChildNodes.FindNode('pinName').Attributes['Name']);
    if Assigned(iNode.ChildNodes.FindNode('pinType')) then SCHPin.pinType:=Trim(iNode.ChildNodes.FindNode('pinType').Text);
    if Assigned(iNode.ChildNodes.FindNode('partNum')) then SCHPin.PartNum:=StrToInt(iNode.ChildNodes.FindNode('partNum').Text);
    if length(SCHPin.pinName.Text)=0 then SCHPin.pinName.Text:='~'; // Default
    result:=SCHPin;
end;
*/

CSchPin *CreatePin(wxXmlNode *iNode) {
    wxString str, propValue;
    long num;
    CSchPin *schPin = new CSchPin();

    schPin->m_objType = 'P';
//    SCHLine.PartNum:=SymbolIndex;
    iNode->GetPropVal(wxT("Name"), &schPin->m_number.text);
    schPin->m_pinNum.text = '0'; // Default
    schPin->m_isVisible = 0; // Default is not visible
//    SCHPin.pinName.Text:='~'; // Default
    if (FindNode(iNode->GetChildren(), wxT("symPinNum"))) {
        str = FindNode(iNode->GetChildren(), wxT("symPinNum"))->GetNodeContent();
        str.Trim(false);
        str.Trim(true);
        schPin->m_pinNum.text = str;
    }
//    SCHPin.pinName.Text:=SCHPin.pinNum.Text; // Default
    if (FindNode(iNode->GetChildren(), wxT("pinName"))) {
        FindNode(iNode->GetChildren(), wxT("pinName"))->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        schPin->m_pinName.text = propValue;
    }
    if (FindNode(iNode->GetChildren(), wxT("pinType"))) {
        str = FindNode(iNode->GetChildren(), wxT("pinType"))->GetNodeContent();
        str.Trim(false);
        str.Trim(true);
        schPin->m_pinType = str;
    }
    if (FindNode(iNode->GetChildren(), wxT("partNum"))) {
        FindNode(iNode->GetChildren(), wxT("partNum"))->GetNodeContent().ToLong(&num);
        schPin->m_partNum = (int)num;
    }

    if (schPin->m_pinName.text.Len() == 0) schPin->m_pinName.text = '~'; // Default

    return schPin;
}

/*
function CreateLine(inode:IXMLNode;SymbolIndex:integer):THSCHLine;
var lNode:iXMLNode;
begin
    SCHLine:=THSCHLine.Create();
    SCHLine.ObjType:='L';
    SCHLine.PartNum:=SymbolIndex;
    if iNode.NodeName='line' then SCHLine.LineType:='W'; // wire
    if Assigned(iNode.ChildNodes.FindNode('width')) then
         SCHLine.Width:=StrToIntUnits(iNode.ChildNodes.FindNode('width').Text,' ')
    else SCHLine.Width:=1; //default
    lNode:=iNode.ChildNodes.FindNode('pt');
    if Assigned(lNode) then
        begin
            SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHLine.PositionX,SCHLine.PositionY);
            lNode:=lNode.NextSibling;
            while (Assigned(lNode) and (lnode.NodeName<>'pt')) do lNode:=lNode.NextSibling;
            if Assigned(lNode) then
               SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHLine.ToX,SCHLine.ToY);
        end;
    if Assigned(iNode.ChildNodes.FindNode('netNameRef')) then
         SCHLine.Net:=trim(iNode.ChildNodes.FindNode('netNameRef').Attributes['Name']);
    SCHLine.LabelText.TextIsVisible:=0;   // LABELS
    SCHLine.LabelText.Text:=SCHLine.Net;  // can be better ?
    lNode:=iNode.ParentNode;
    if lNode.NodeName='wire' then
    begin
      lNode:=lNode.ChildNodes.FindNode('dispName');
      if Assigned(lNode) then
      begin
         if (Trim(lNode.Text)='True') then SCHLine.LabelText.TextIsVisible:=1;
         lNode:=lNode.ParentNode;
         lNode:=lNode.ChildNodes.FindNode('text');
         if Assigned(lNode) then
         begin
           if Assigned(lNode.ChildNodes.FindNode('pt')) then
                           SetPosition(lNode.ChildNodes.FindNode('pt').Text,SCH.DefaultMeasurementUnit,SCHLine.LabelText.TextPositionX,SCHLine.LabelText.TextPositionY);
           if Assigned(lNode.ChildNodes.FindNode('rotation')) then
                           SCHLine.LabelText.TextRotation:= StrToInt1Units(lNode.ChildNodes.FindNode('rotation').text) ;
         end;
      end;
    end;  
    result:=SCHLine;
end;
*/

CSchLine *CreateLine(wxXmlNode *iNode, int symbolIndex, CSch *sch, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString propValue, str;
    CSchLine *schLine = new CSchLine();
    schLine->m_objType = 'L';
    schLine->m_partNum = symbolIndex;

    if (iNode->GetName() == wxT("line")) schLine->m_lineType = 'W'; // wire
    if (FindNode(iNode->GetChildren(), wxT("width")))
         schLine->m_width = StrToIntUnits(FindNode(iNode->GetChildren(), wxT("width"))->GetNodeContent(), ' ', actualConversion);
    else schLine->m_width = 1; //default
    lNode = FindNode(iNode->GetChildren(), wxT("pt"));
    if (lNode) {
        SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                    &schLine->m_positionX, &schLine->m_positionY, actualConversion);

        lNode = lNode->GetNext();
        while (lNode && lNode->GetName() != wxT("pt"))
            lNode = lNode->GetNext();

        if (lNode)
            SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                        &schLine->m_toX, &schLine->m_toY, actualConversion);
    }

    if (FindNode(iNode->GetChildren(), wxT("netNameRef"))) {
        FindNode(iNode->GetChildren(), wxT("netNameRef"))->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        schLine->m_net = propValue;
    }

    schLine->m_labelText.textIsVisible = 0; // LABELS
    schLine->m_labelText.text = schLine->m_net;  // can be better ?

    lNode = iNode->GetParent();
    if (lNode->GetName() == wxT("wire")) {
        lNode = FindNode(lNode->GetChildren(), wxT("dispName"));
        if (lNode) {
            str = lNode->GetNodeContent();
            str.Trim(false);
            str.Trim(true);
            if (str == wxT("True")) schLine->m_labelText.textIsVisible = 1;
            lNode = lNode->GetParent();
            lNode = FindNode(lNode->GetChildren(), wxT("text"));
            if (lNode) {
                if (FindNode(lNode->GetChildren(), wxT("pt")))
                    SetPosition(FindNode(lNode->GetChildren(), wxT("pt"))->GetNodeContent(),
                                sch->m_defaultMeasurementUnit, &schLine->m_labelText.textPositionX, &schLine->m_labelText.textPositionY, actualConversion);

                if (FindNode(lNode->GetChildren(), wxT("rotation")))
                    schLine->m_labelText.textRotation = StrToInt1Units(FindNode(lNode->GetChildren(), wxT("rotation"))->GetNodeContent());
            }
        }
    }

    return schLine;
}

/*
function CreateBus(inode:IXMLNode):THSCHLine;
var lNode:iXMLNode;
begin
  SCHLine:=THSCHLine.Create();
  SCHLine.ObjType:='L';
  SCHLine.LineType:='B'; // Bus
//  SCHLine.Net:=trim(iNode.Attributes['Name']);
  SCHLine.LabelText.TextIsVisible:=0;
  SCHLine.LabelText.Text:=trim(iNode.Attributes['Name']);
  lNode:=iNode.ChildNodes.FindNode('pt');
  if Assigned(lNode) then SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHLine.PositionX,SCHLine.PositionY);
  if Assigned(lNode) then lNode := lNode.NextSibling;
  if Assigned(lNode) then SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHLine.ToX,SCHLine.ToY);
  lNode:=iNode.ChildNodes.FindNode('dispName');
  if Assigned(lNode) then
       if (Trim(lNode.Text)='True') then SCHLine.LabelText.TextIsVisible:=1;
  lNode:=iNode.ChildNodes.FindNode('text');
  if Assigned(lNode) then
   begin
      if Assigned(lNode.ChildNodes.FindNode('pt')) then
                     SetPosition(lNode.ChildNodes.FindNode('pt').Text,SCH.DefaultMeasurementUnit,SCHLine.LabelText.TextPositionX,SCHLine.LabelText.TextPositionY);
      if Assigned(lNode.ChildNodes.FindNode('rotation')) then
                     SCHLine.LabelText.TextRotation:= StrToInt1Units(lNode.ChildNodes.FindNode('rotation').text) ;
   end;
  Result:=SCHLine;
end;
*/

CSchLine *CreateBus(wxXmlNode *iNode, CSch *sch, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString propValue;
    CSchLine *schLine = new CSchLine();
    schLine->m_objType = 'L';
    schLine->m_lineType = 'B'; // Bus
    schLine->m_labelText.textIsVisible = 0;
    iNode->GetPropVal(wxT("Name"), &schLine->m_labelText.text);
    schLine->m_labelText.text.Trim(false);
    schLine->m_labelText.text.Trim(true);

    lNode = FindNode(iNode->GetChildren(), wxT("pt"));

    if (lNode)
        SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                    &schLine->m_positionX, &schLine->m_positionY, actualConversion);

    if (lNode) lNode = lNode->GetNext();

    if (lNode)
        SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                    &schLine->m_toX, &schLine->m_toY, actualConversion);

    lNode = FindNode(iNode->GetChildren(), wxT("dispName"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        if (propValue == wxT("True"))
            schLine->m_labelText.textIsVisible = 1;
    }

    lNode = FindNode(iNode->GetChildren(), wxT("text"));
    if (lNode) {
        if (FindNode(lNode->GetChildren(), wxT("pt")))
            SetPosition(FindNode(lNode->GetChildren(), wxT("pt"))->GetNodeContent(),
                        sch->m_defaultMeasurementUnit, &schLine->m_labelText.textPositionX, &schLine->m_labelText.textPositionY, actualConversion);

        if (FindNode(lNode->GetChildren(), wxT("rotation")))
            schLine->m_labelText.textRotation = StrToInt1Units(FindNode(lNode->GetChildren(), wxT("rotation"))->GetNodeContent());
    }

    return schLine;
}

/*
function CreateJunction(inode:IXMLNode):THSCHJunction;
var lNode:iXMLNode;
begin
  IsJunction:=true;
  SCHJunction:=THSCHJunction.Create();
  SCHJunction.ObjType:='J';
  if Assigned(iNode.ChildNodes.FindNode('pt')) then SetPosition(iNode.ChildNodes.FindNode('pt').Text,SCH.DefaultMeasurementUnit,SCHJunction.PositionX,SCHJunction.PositionY);
  if Assigned(iNode.ChildNodes.FindNode('netNameRef')) then
         SCHJunction.Net:=trim(iNode.ChildNodes.FindNode('netNameRef').Attributes['Name']);
  Result:=SCHJunction;
end;
*/

CSchJunction *CreateJunction(wxXmlNode *iNode, CSch *sch, bool *isJunction, wxString actualConversion) {
    wxString propValue;
    CSchJunction *schJunction = new CSchJunction();

    *isJunction = true;
    schJunction->m_objType = 'J';
    if (FindNode(iNode->GetChildren(), wxT("pt"))) {
        SetPosition(FindNode(iNode->GetChildren(), wxT("pt"))->GetNodeContent(),
                    sch->m_defaultMeasurementUnit, &schJunction->m_positionX, &schJunction->m_positionY, actualConversion);
    }
    if (FindNode(iNode->GetChildren(), wxT("netNameRef"))) {
        FindNode(iNode->GetChildren(), wxT("netNameRef"))->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(true);
        propValue.Trim(false);
        schJunction->m_net = propValue;
    }

    return schJunction;
}

/*
function CreateArc(inode:IXMLNode;SymbolIndex:integer):THSCHArc;
var lNode,tNode:iXMLNode;
    si,so:string;
    r,a:integer;
begin
    SCHArc:=THSCHArc.Create();
    SCHArc.ObjType:='A';
    SCHArc.PartNum:=SymbolIndex;
    if Assigned(iNode.ChildNodes.FindNode('width')) then
         SCHArc.Width:=StrToIntUnits(iNode.ChildNodes.FindNode('width').Text,' ')
    else SCHArc.Width:=1; //default
    if iNode.NodeName='triplePointArc' then
    begin
       // origin
       lNode:=iNode.ChildNodes.FindNode('pt');
         if Assigned(lNode) then SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHArc.PositionX,SCHArc.PositionY);
       // First - starting point in PCAS is ENDING point in KiCAd
       lNode:=lNode.NextSibling;
         if Assigned(lNode) then
            SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHArc.ToX,SCHArc.ToY);
       // Second - ending point in PCAS is STARTING point in KiCAd
       lNode:=lNode.NextSibling;
         if Assigned(lNode) then
            SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHArc.StartX,SCHArc.StartY);
           // now temporary, it can be fixed later.....
          //SCHArc.StartAngle:=0;
          //SCHArc.SweepAngle:=3600;
    end;
    if iNode.NodeName='arc' then
    begin
       lNode:=iNode.ChildNodes.FindNode('pt');
       if Assigned(lNode) then SetPosition(lNode.Text,SCH.DefaultMeasurementUnit,SCHArc.PositionX,SCHArc.PositionY);
       lNode:=iNode.ChildNodes.FindNode('radius');
       if Assigned(iNode.ChildNodes.FindNode('radius')) then SCHArc.Radius:=StrToIntUnits(iNode.ChildNodes.FindNode('radius').Text,' ');
       r:=StrToIntUnits(lNode.Text,' ');
//       a:=StrToInt1Units(iNode.ChildNodes.FindNode('startAngle').Text);
       SCHArc.StartAngle:=StrToInt1Units(iNode.ChildNodes.FindNode('startAngle').Text);
       SCHArc.StartX:=Round(SCHArc.PositionX+r*sin((SCHArc.StartAngle-900)*Pi/1800));
       SCHArc.StartY:=Round(SCHArc.PositionY-r*cos((SCHArc.StartAngle-900)*Pi/1800));
       SCHArc.SweepAngle:=StrToInt1Units(iNode.ChildNodes.FindNode('sweepAngle').Text);
       SCHArc.ToX:=Round(SCHArc.PositionX+r*sin((SCHArc.StartAngle+SCHArc.SweepAngle-900)*Pi/1800));
       SCHArc.ToY:=Round(SCHArc.PositionY-r*cos((SCHArc.StartAngle+SCHArc.SweepAngle-900)*Pi/1800));
    end;
   result:=SCHArc;
end;
*/

CSchArc *CreateArc(wxXmlNode *iNode, int symbolIndex, CSch *sch, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString propValue;
    double r;
    CSchArc *schArc = new CSchArc();

    schArc->m_objType = 'A';
    schArc->m_partNum = symbolIndex;
    if (FindNode(iNode->GetChildren(), wxT("width")))
         schArc->m_width = StrToIntUnits(FindNode(iNode->GetChildren(), wxT("width"))->GetNodeContent(), ' ', actualConversion);
    else schArc->m_width = 1; //default

    if (iNode->GetName() == wxT("triplePointArc")) {
        // origin
        lNode = FindNode(iNode->GetChildren(), wxT("pt"));
        if (lNode)
            SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                    &schArc->m_positionX, &schArc->m_positionY, actualConversion);
        // First - starting point in PCAS is ENDING point in KiCAd
        lNode = lNode->GetNext();
        if (lNode)
            SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                    &schArc->m_toX, &schArc->m_toY, actualConversion);
        // Second - ending point in PCAS is STARTING point in KiCAd
        lNode = lNode->GetNext();
        if (lNode)
            SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                    &schArc->m_startX, &schArc->m_startY, actualConversion);
        // now temporary, it can be fixed later.....
        //SCHArc.StartAngle:=0;
        //SCHArc.SweepAngle:=3600;
    }

    if (iNode->GetName() == wxT("arc")) {
        lNode = FindNode(iNode->GetChildren(), wxT("pt"));
        if (lNode)
            SetPosition(lNode->GetNodeContent(), sch->m_defaultMeasurementUnit,
                    &schArc->m_positionX, &schArc->m_positionY, actualConversion);

        lNode = FindNode(iNode->GetChildren(), wxT("radius"));
        if (lNode)
            schArc->m_radius = StrToIntUnits(lNode->GetNodeContent(), ' ', actualConversion);
        r = StrToIntUnits(lNode->GetNodeContent(), ' ', actualConversion);
        schArc->m_startAngle = StrToInt1Units(FindNode(iNode->GetChildren(), wxT("startAngle"))->GetNodeContent());
        schArc->m_startX = KiROUND(schArc->m_positionX + r * sin((schArc->m_startAngle - 900.0) * M_PI / 1800.0));
        schArc->m_startY = KiROUND(schArc->m_positionY - r * cos((schArc->m_startAngle - 900) * M_PI / 1800.0));
        schArc->m_sweepAngle = StrToInt1Units(FindNode(iNode->GetChildren(), wxT("sweepAngle"))->GetNodeContent());
        schArc->m_toX = KiROUND(schArc->m_positionX + r * sin((schArc->m_startAngle + schArc->m_sweepAngle - 900.0) * M_PI / 1800.0));
        schArc->m_toY = KiROUND(schArc->m_positionY - r * cos((schArc->m_startAngle + schArc->m_sweepAngle - 900.0) * M_PI / 1800.0));
    }

    return schArc;
}

/*
function CreateSCHModule(inode:IXMLNode):THSCHModule;
var lNode,tNode,ttNode:iXMLNode;
    AttachedPatternName,s:string;
    i:integer;

procedure FindAndProcessSymbolDef(iNode:IXMLNode;iSCHModule:THSCHModule;SymbolIndex:Integer);
var tNode,ttNode:IXMLNode;

procedure SetPinProperties(iNode:IXMLNode;SymbolIndex:integer);
var i:integer;
    pn,t:string;
    lpn:HTextValue;
begin
   pn:=Trim(tNode.ChildNodes.FindNode('pinNum').Text);
   //THSCHPin(iSCHModule.ModuleObjects[i]).PinLength:=10; //Default pin length
   for i:=0 to iSCHModule.ModuleObjects.Count-1 do
    begin
      if THSCHComponent(iSCHModule.ModuleObjects[i]).ObjType='P' then
          if ((THSCHPin(iSCHModule.ModuleObjects[i]).PinNum.Text=pn)
          and (THSCHPin(iSCHModule.ModuleObjects[i]).PartNum=SymbolIndex))
          then
          begin
             THSCHPin(iSCHModule.ModuleObjects[i]).IsVisible:=1; // This pin is described, it means is visible
             THSCHPin(iSCHModule.ModuleObjects[i]).PinLength:= DefaultSymbolPinLength; // Default value
             if Assigned(tNode.ChildNodes.FindNode('pinLength')) then
             begin
                   t:=tNode.ChildNodes.FindNode('pinLength').Text;
                       THSCHPin(iSCHModule.ModuleObjects[i]).PinLength:=
                           StrToIntUnits(GetAndCutWordWithMeasureUnits(t,SCH.DefaultMeasurementUnit),' ');
             end;
             if Assigned(tNode.ChildNodes.FindNode('outsideEdgeStyle')) then
                   THSCHPin(iSCHModule.ModuleObjects[i]).EdgeStyle:= Trim(tNode.ChildNodes.FindNode('outsideEdgeStyle').Text) ;
             if Assigned(tNode.ChildNodes.FindNode('rotation')) then
                   THSCHPin(iSCHModule.ModuleObjects[i]).Rotation:= StrToInt1Units(tNode.ChildNodes.FindNode('rotation').Text) ;
             if Assigned(tNode.ChildNodes.FindNode('pt')) then
                   SetPosition(tNode.ChildNodes.FindNode('pt').Text,
                               SCH.DefaultMeasurementUnit,
                               THSCHPin(iSCHModule.ModuleObjects[i]).PositionX,
                               THSCHPin(iSCHModule.ModuleObjects[i]).PositionY);
             if Assigned(tNode.ChildNodes.FindNode('isFlipped')) then
                    if (Trim(tNode.ChildNodes.FindNode('isFlipped').Text)='True') then THSCHPin(iSCHModule.ModuleObjects[i]).Mirror:=1;
             if Assigned(tNode.ChildNodes.FindNode('pinName')) then
                  begin
                   // ttNode:=tNode.ChildNodes.FindNode('pinName').ChildNodes.FindNode('text');
                    lpn:=THSCHPin(iSCHModule.ModuleObjects[i]).Number;
                    if Assigned(tNode.ChildNodes.FindNode('pinName').ChildNodes.FindNode('text')) then
                        SetTextParameters(tNode.ChildNodes.FindNode('pinName').ChildNodes.FindNode('text'),
                                           lpn
                                          ,SCH.DefaultMeasurementUnit);
                  end;
             if Assigned(tNode.ChildNodes.FindNode('pinDes')) then
                  begin
                    //ttNode:=tNode.ChildNodes.FindNode('pinDes').ChildNodes.FindNode('text');
                    lpn:=THSCHPin(iSCHModule.ModuleObjects[i]).PinName;
                    if Assigned(tNode.ChildNodes.FindNode('pinDes').ChildNodes.FindNode('text')) then
                        SetTextParameters(tNode.ChildNodes.FindNode('pinDes').ChildNodes.FindNode('text'),
                                           lpn
                                           ,SCH.DefaultMeasurementUnit);
                  end;
          end;
    end;
end;
*/

static void SetPinProperties(wxXmlNode *iNode, CSchModule *iSchModule, int symbolIndex, CSch *sch, wxString actualConversion) {
    CSchPin *schPin;
    wxString pn, t, str;
    TTextValue lpn;
    int i;

    pn = FindNode(iNode->GetChildren(), wxT("pinNum"))->GetNodeContent();
    pn.Trim(false);
    pn.Trim(true);

    for (i = 0; i < (int)iSchModule->m_moduleObjects.GetCount(); i++) {
        if ((iSchModule->m_moduleObjects[i])->m_objType == 'P') {
            schPin = (CSchPin *)iSchModule->m_moduleObjects[i];
            if (schPin->m_pinNum.text == pn &&
                schPin->m_partNum == symbolIndex) {
                schPin->m_isVisible = 1; // This pin is described, it means is visible
                schPin->m_pinLength = DEFAULT_SYMBOL_PIN_LENGTH; // Default value
                if (FindNode(iNode->GetChildren(), wxT("pinLength"))) {
                    t = FindNode(iNode->GetChildren(), wxT("pinLength"))->GetNodeContent();
                    schPin->m_pinLength =
                        StrToIntUnits(GetAndCutWordWithMeasureUnits(&t, sch->m_defaultMeasurementUnit), ' ', actualConversion);
                }
                if (FindNode(iNode->GetChildren(), wxT("outsideEdgeStyle"))) {
                    str = FindNode(iNode->GetChildren(), wxT("outsideEdgeStyle"))->GetNodeContent();
                    str.Trim(false);
                    str.Trim(true);
                    schPin->m_edgeStyle = str;
                }
                if (FindNode(iNode->GetChildren(), wxT("rotation")))
                    schPin->m_rotation = StrToInt1Units(FindNode(iNode->GetChildren(), wxT("rotation"))->GetNodeContent());
                if (FindNode(iNode->GetChildren(), wxT("pt"))) {
                    SetPosition(FindNode(iNode->GetChildren(), wxT("pt"))->GetNodeContent(),
                               sch->m_defaultMeasurementUnit,
                               &schPin->m_positionX,
                               &schPin->m_positionY,
                               actualConversion);
                }
                if (FindNode(iNode->GetChildren(), wxT("isFlipped"))) {
                    str = FindNode(iNode->GetChildren(), wxT("isFlipped"))->GetNodeContent();
                    str.Trim(false);
                    str.Trim(true);
                    if (str == wxT("True")) schPin->m_mirror = 1;
                }
                if (FindNode(iNode->GetChildren(), wxT("pinName"))) {
                    lpn = schPin->m_number;
                    if (FindNode(FindNode(iNode->GetChildren(), wxT("pinName")), wxT("text"))) {
                        SetTextParameters(FindNode(FindNode(iNode->GetChildren(), wxT("pinName")), wxT("text")),
                                          &lpn,
                                          sch->m_defaultMeasurementUnit,
                                          actualConversion);
                    }
                }
                if (FindNode(iNode->GetChildren(), wxT("pinDes"))) {
                    lpn = schPin->m_pinName;
                    if (FindNode(FindNode(iNode->GetChildren(), wxT("pinDes")), wxT("text"))) {
                        SetTextParameters(FindNode(FindNode(iNode->GetChildren(), wxT("pinDes")), wxT("text")),
                                          &lpn,
                                          sch->m_defaultMeasurementUnit,
                                          actualConversion);
                    }
                }
            }
        }
    }
}

/*
begin
  tNode:=iNode;
  while (tNode.NodeName<>'www.lura.sk') do tNode:=tNode.ParentNode;
  tNode:=tNode.ChildNodes.FindNode('library');
  if Assigned(tNode) then
      begin
        tNode:=tNode.ChildNodes.FindNode('symbolDef');
        while Assigned(tNode) do
           begin
             if ((tNode.NodeName='symbolDef')
                 and
                 ((tNode.Attributes['Name']=iSCHModule.AttachedSymbols[SymbolIndex])
                    or ( Assigned(tNode.ChildNodes.FindNode('originalName'))
                         and (tNode.ChildNodes.FindNode('originalName').Attributes['Name']=iSCHModule.AttachedSymbols[SymbolIndex])
                  )))
                 then
                 begin
                 ttNode:=tNode;
                 tNode:=ttNode.ChildNodes.FindNode('pin');
                 while Assigned(tNode) do
                    begin
                      if (tNode.NodeName='pin') and (Assigned(tNode.ChildNodes.FindNode('pinNum'))) then
                           SetPinProperties(tNode,SymbolIndex);
                      tNode:=tNode.NextSibling;
                    end;
                 tNode:=ttNode.ChildNodes.FindNode('line');
                 while Assigned(tNode) do
                    begin
                      if tNode.NodeName='line' then iSCHModule.ModuleObjects.Add(CreateLine(tNode,SymbolIndex));
                      tNode:=tNode.NextSibling;
                    end;
                tNode:=ttNode.ChildNodes.FindNode('arc');
                while Assigned(tNode) do
                    begin
                      if tNode.NodeName='arc' then iSCHModule.ModuleObjects.Add(CreateArc(tNode,SymbolIndex));
                      tNode:=tNode.NextSibling;
                    end;
                tNode:=ttNode.ChildNodes.FindNode('triplePointArc');
                while Assigned(tNode) do
                    begin
                      if tNode.NodeName='triplePointArc' then iSCHModule.ModuleObjects.Add(CreateArc(tNode,SymbolIndex));
                      tNode:=tNode.NextSibling;
                    end;
                tNode:=ttNode.ChildNodes.first;
                while Assigned(tNode) do
                    begin
                      if tNode.NodeName='attr' then
                        begin
                          // Reference
                          if Trim(tNode.Attributes['Name'])='RefDes' then
                               SetTextParameters(tNode,iSCHModule.Reference,SCH.DefaultMeasurementUnit);
                         // Type
                          if Trim(tNode.Attributes['Name'])='Type {Type}' then
                               SetTextParameters(tNode,iSCHModule.Name,SCH.DefaultMeasurementUnit);
                          // OR
                          if Trim(tNode.Attributes['Name'])='Type' then
                               SetTextParameters(tNode,iSCHModule.Name,SCH.DefaultMeasurementUnit);
                        end;
                      tNode:=tNode.NextSibling;
                    end;
                end;
             if assigned(tNode) then tNode:=tNode.NextSibling;
           end;
      end;
end;
*/

static void FindAndProcessSymbolDef(wxXmlNode *iNode, CSchModule *iSchModule, int symbolIndex, CSch *sch, wxString actualConversion) {
    wxXmlNode *tNode, *ttNode;
    wxString propValue, propValue2;

    tNode = iNode;
    while (tNode->GetName() != wxT("www.lura.sk"))
        tNode = tNode->GetParent();

    tNode = FindNode(tNode->GetChildren(), wxT("library"));
    if (tNode) {
        tNode = FindNode(tNode->GetChildren(), wxT("symbolDef"));
        while (tNode) {
            tNode->GetPropVal(wxT("Name"), &propValue);
            if (FindNode(tNode->GetChildren(), wxT("originalName")))
                FindNode(tNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue2);

            if (tNode->GetName() == wxT("symbolDef") &&
                 (propValue == iSchModule->m_attachedSymbols[symbolIndex] ||
                 (FindNode(tNode->GetChildren(), wxT("originalName")) &&
                  propValue2 == iSchModule->m_attachedSymbols[symbolIndex])))
            {
                ttNode = tNode;
                tNode = FindNode(ttNode->GetChildren(), wxT("pin"));
                while (tNode) {
                    if (tNode->GetName() == wxT("pin") && FindNode(tNode->GetChildren(), wxT("pinNum")))
                        SetPinProperties(tNode, iSchModule, symbolIndex, sch, actualConversion);
                    tNode = tNode->GetNext();
                }
                tNode = FindNode(ttNode->GetChildren(), wxT("line"));
                while (tNode) {
                    if (tNode->GetName() == wxT("line"))
                        iSchModule->m_moduleObjects.Add(CreateLine(tNode, symbolIndex, sch, actualConversion));
                    tNode = tNode->GetNext();
                }
                tNode = FindNode(ttNode->GetChildren(), wxT("arc"));
                while (tNode) {
                    if (tNode->GetName() == wxT("arc"))
                        iSchModule->m_moduleObjects.Add(CreateArc(tNode, symbolIndex, sch, actualConversion));
                    tNode = tNode->GetNext();
                }
                tNode = FindNode(ttNode->GetChildren(), wxT("triplePointArc"));
                while (tNode) {
                    if (tNode->GetName() == wxT("triplePointArc"))
                        iSchModule->m_moduleObjects.Add(CreateArc(tNode, symbolIndex, sch, actualConversion));
                    tNode = tNode->GetNext();
                }

                tNode = ttNode->GetChildren();
                while (tNode) {
                    if (tNode->GetName() == wxT("attr")) {
                          // Reference
                          tNode->GetPropVal(wxT("Name"), &propValue);
                          propValue.Trim(false);
                          propValue.Trim(true);
                          if (propValue == wxT("RefDes"))
                              SetTextParameters(tNode, &iSchModule->m_reference, sch->m_defaultMeasurementUnit, actualConversion);
                          // Type
                          if (propValue == wxT("Type {Type}"))
                              SetTextParameters(tNode, &iSchModule->m_name, sch->m_defaultMeasurementUnit, actualConversion);
                          // OR
                          if (propValue == wxT("Type"))
                              SetTextParameters(tNode, &iSchModule->m_name, sch->m_defaultMeasurementUnit, actualConversion);
                    }
                    tNode = tNode->GetNext();
                }
            }
            if (tNode) tNode = tNode->GetNext();
        }
    }
}

/*
begin
   SCHModule:=THSCHModule.Create();
   SCHModule.Name.Text:=TrimLeft(iNode.ChildNodes.FindNode('originalName').Attributes['Name']);
   SCHModule.ObjType:='M';
   StatusBar.SimpleText:='Creating Component : '+SCHModule.Name.Text;
   //lNode:=iNode.ChildNodes.FindNode('compHeader');
   if Assigned(iNode.ChildNodes.FindNode('compHeader')) then
         begin
            with iNode.ChildNodes.FindNode('compHeader') do
            begin
               if Assigned(ChildNodes.FindNode('refDesPrefix')) then
                     SCHModule.Reference.text:=ChildNodes.FindNode('refDesPrefix').Attributes['Name'];

               if Assigned(ChildNodes.FindNode('numParts')) then
                        SCHModule.NumParts:=StrToInt(ChildNodes.FindNode('numParts').Text);
            end;
         end;
//   lNode:= FindPatternMultilayerSection(lNode,SCHBModule.PatGraphRefName);
//   if Assigned(iNode.ChildNodes.FindNode('compPin')) then
     tNode:=iNode.ChildNodes.First;
     while Assigned(tNode) do
        begin
              if (tNode.NodeName='compPin') then
                    SCHModule.ModuleObjects.Add(CreatePin(tNode));
              if (tNode.NodeName='attachedSymbol') then
                  if (Assigned(tNode.ChildNodes.FindNode('altType'))
                         and (Trim(tNode.ChildNodes.FindNode('altType').Text)='Normal')) then
                            SCHModule.AttachedSymbols[StrToInt(tNode.ChildNodes.FindNode('partNum').Text)]:=tNode.ChildNodes.FindNode('symbolName').Attributes['Name'];
              if (tNode.NodeName='attachedPattern') then
                     SCHModule.AttachedPattern:=tNode.ChildNodes.FindNode('patternName').Attributes['Name'];
              if (tNode.NodeName='attr') then
                 begin
                     if (length(tNode.Attributes['Name'])>13)
                         and (copy(tNode.Attributes['Name'],1,12)='Description ') then
                            SCHModule.ModuleDescription:=copy(tNode.Attributes['Name'],13);
                 end;
             tNode := tNode.NextSibling;
       end;
   for i:=1 to  SCHModule.NumParts do
                 FindAndProcessSymbolDef(iNode,SCHModule,i);
   result:=SCHModule;
end;
*/

CSchModule *CreateSchModule(wxXmlNode *iNode, wxStatusBar* statusBar, CSch *sch, wxString actualConversion) {
    wxString propValue, str;
    wxXmlNode *lNode, *tNode;
    long num;
    int i;
    CSchModule *schModule = new CSchModule();

    FindNode(iNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue);
    propValue.Trim(false);
    schModule->m_name.text = propValue;
    schModule->m_objType = 'M';
    statusBar->SetStatusText(wxT("Creating Component : ") + schModule->m_name.text);

    lNode = FindNode(iNode->GetChildren(), wxT("compHeader"));
    if (lNode) {
        if (FindNode(lNode->GetChildren(), wxT("refDesPrefix")))
            FindNode(lNode->GetChildren(), wxT("refDesPrefix"))->GetPropVal(wxT("Name"), &schModule->m_reference.text);

        if (FindNode(lNode->GetChildren(), wxT("numParts"))) {
            FindNode(lNode->GetChildren(), wxT("numParts"))->GetNodeContent().ToLong(&num);
            schModule->m_numParts = (int)num;
        }
    }

    tNode = iNode->GetChildren();
    while (tNode) {
        if (tNode->GetName() == wxT("compPin"))
            schModule->m_moduleObjects.Add(CreatePin(tNode));
        if (tNode->GetName() == wxT("attachedSymbol")) {
            if (FindNode(tNode->GetChildren(), wxT("altType"))) {
                str = FindNode(tNode->GetChildren(), wxT("altType"))->GetNodeContent();
                str.Trim(false);
                str.Trim(true);
                if (str == wxT("Normal"))
                {
                    FindNode(tNode->GetChildren(), wxT("partNum"))->GetNodeContent().ToLong(&num);
                    FindNode(tNode->GetChildren(), wxT("symbolName"))->GetPropVal(wxT("Name"),
                        &schModule->m_attachedSymbols[(int)num]);
                }
            }
        }
        if (tNode->GetName() == wxT("attachedPattern")) {
            FindNode(tNode->GetChildren(), wxT("patternName"))->GetPropVal(wxT("Name"), &schModule->m_attachedPattern);
        }
        if (tNode->GetName() == wxT("attr")) {
            tNode->GetPropVal(wxT("Name"), &propValue);
            if (propValue.Len() > 13 && propValue.Left(12) == wxT("Description "))
                schModule->m_moduleDescription = propValue.Left(13);
        }
        tNode = tNode->GetNext();
    }

    for (i = 0; i < schModule->m_numParts; i++)
        FindAndProcessSymbolDef(iNode, schModule, i + 1, sch, actualConversion);

    return schModule;
}

/*
procedure DoAlias(alias:string);
var i:integer;
    a,n:string;
begin
  For i:=0 to SCH.SCHComponents.Count-1 do
  begin
     if (THSCHComponent(SCH.SCHComponents.Items[i]).ObjType='M') then
        begin
           n:=alias;
           a:=GetWord(n);
           if THSCHModule(SCH.SCHComponents.Items[i]).Name.Text=Trim(n) then
             begin
               THSCHModule(SCH.SCHComponents.Items[i]).Alias:=THSCHModule(SCH.SCHComponents.Items[i]).Alias+' '+a;
             end;
        end;
  end;
end;
*/

void DoAlias(wxString alias, CSch *sch) {
    int i;
    wxString a, n;

    for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
        if ((sch->m_schComponents[i])->m_objType == 'M') {
            n = alias;
            a = GetWord(&n);
            n.Trim(true);
            n.Trim(false);
            if (((CSchModule *)sch->m_schComponents[i])->m_name.text == n) {
                ((CSchModule *)sch->m_schComponents[i])->m_alias = ((CSchModule *)sch->m_schComponents[i])->m_alias + ' ' + a;
            }
        }
    }
}

/*
function CreateSCHSymbol(iNode:IXMLNode):THSCHSymbol;
var lNode:IXMLNode;
    t,r:boolean;
begin
  SCHSymbol:=THSCHSymbol.Create();
  SCHSymbol.ObjType:='S';
  if Assigned(iNode.ChildNodes.FindNode('partNum')) then
                        SCHSymbol.PartNum:=StrToInt(iNode.ChildNodes.FindNode('partNum').Text);
  if Assigned(iNode.ChildNodes.FindNode('symbolRef')) then
                        SCHSymbol.Module.Text:=Trim(iNode.ChildNodes.FindNode('symbolRef').Attributes['Name']);
  if Assigned(iNode.ChildNodes.FindNode('refDesRef')) then
                        SCHSymbol.Reference.Text:=Trim(iNode.ChildNodes.FindNode('refDesRef').Attributes['Name']);
  if Assigned(iNode.ChildNodes.FindNode('pt')) then
                   SetPosition(iNode.ChildNodes.FindNode('pt').Text,
                               SCH.DefaultMeasurementUnit,
                               SCHSymbol.PositionX,
                               SCHSymbol.PositionY);
  if Assigned(iNode.ChildNodes.FindNode('isFlipped')) then
                    if (Trim(iNode.ChildNodes.FindNode('isFlipped').Text)='True') then SCHSymbol.Mirror:=1;
  if Assigned(iNode.ChildNodes.FindNode('rotation')) then
                   SCHSymbol.Rotation:= StrToInt1Units(iNode.ChildNodes.FindNode('rotation').Text);

  lNode:=iNode.ChildNodes.first;
  // update physical symbol textts positions
  t:=false;r:=false;
                while Assigned(lNode) do
                    begin
                      if lNode.NodeName='attr' then
                        begin
                          if Trim(lNode.Attributes['Name'])='Value' then  // WHY ???? IS IT SWITCHED IN PCAD ?
                             begin
                               r:=true;
                               SetTextParameters(lNode,SCHSymbol.Reference,SCH.DefaultMeasurementUnit);
                             end;
                          if Trim(lNode.Attributes['Name'])='RefDes' then // WHY ???? IS IT SWITCHED IN PCAD ?
                             begin
                               t:=true;
                               SetTextParameters(lNode,SCHSymbol.Typ,SCH.DefaultMeasurementUnit);
                             end;
                        end;
                      lNode:=lNode.NextSibling;
                    end;

   //if t=false then

   lNode:=iNode;
   // also symbol from library as name of component as is known in schematics library
   while (lNode.NodeName<>'www.lura.sk') do lNode:=lNode.ParentNode;
   lNode:=lNode.ChildNodes.FindNode('netlist');
   if Assigned(lNode) then
   begin
      lNode:=lNode.ChildNodes.FindNode('compInst');
      while Assigned(lNode) do
      begin
         if (lNode.NodeName='compInst') and (lNode.Attributes['Name']=SCHSymbol.Reference.Text) then
              begin
                // Type - or Value , depends on version
                if Assigned(lNode.ChildNodes.FindNode('compValue')) then
                      SCHSymbol.Typ.Text:=Trim(lNode.ChildNodes.FindNode('compValue').Attributes['Name'])
                else
                  if Assigned(lNode.ChildNodes.FindNode('originalName')) then
                      SCHSymbol.Typ.Text:=lNode.ChildNodes.FindNode('originalName').Attributes['Name'];
                // Pattern
                if Assigned(lNode.ChildNodes.FindNode('patternName')) then
                      SCHSymbol.AttachedPattern:=lNode.ChildNodes.FindNode('patternName').Attributes['Name']
                else
                  if Assigned(lNode.ChildNodes.FindNode('originalName')) then
                      SCHSymbol.AttachedPattern:=lNode.ChildNodes.FindNode('originalName').Attributes['Name'];
                // Symbol
                if Assigned(lNode.ChildNodes.FindNode('originalName')) then
                      SCHSymbol.AttachedSymbol:=lNode.ChildNodes.FindNode('originalName').Attributes['Name']
                else
                  if Assigned(lNode.ChildNodes.FindNode('compRef')) then
                      SCHSymbol.AttachedSymbol:=lNode.ChildNodes.FindNode('compRef').Attributes['Name'];

              end;
         lNode:=lNode.NextSibling;
      end;
   end;

  // is reference position missing ?
  lNode:=iNode;
  if (r=false) or (t=false) then
  begin
   while (lNode.NodeName<>'www.lura.sk') do lNode:=lNode.ParentNode;
   lNode:=lNode.ChildNodes.FindNode('library');
   if Assigned(lNode) then
   begin
      lNode:=lNode.ChildNodes.FindNode('symbolDef');
      while Assigned(lNode) do
      begin
         if (lNode.NodeName='symbolDef') and (lNode.Attributes['Name']=SCHSymbol.Module.Text) then
              begin
                lNode:=lNode.ChildNodes.First;
                while Assigned(lNode) do
                    begin
                      if lNode.NodeName='attr' then
                        begin
                          // Reference
                          if Trim(lNode.Attributes['Name'])='RefDes' then
                               SetTextParameters(lNode,SCHSymbol.Reference,SCH.DefaultMeasurementUnit);
                         // Type
                          if Trim(lNode.Attributes['Name'])='Type {Type}' then
                               SetTextParameters(lNode,SCHSymbol.Typ,SCH.DefaultMeasurementUnit);
                          // OR
                          if Trim(lNode.Attributes['Name'])='Type' then
                               SetTextParameters(lNode,SCHSymbol.Typ,SCH.DefaultMeasurementUnit);
                        end;
                      lNode:=lNode.NextSibling;
                   end;
              end;
         if Assigned(lNode) then  lNode:=lNode.NextSibling;
      end;
   end;
  end;
  result:=SCHSymbol;
end;
*/

CSchSymbol *CreateSCHSymbol(wxXmlNode *iNode, CSch *sch, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString propValue, str;
    long num;
    bool t, r;
    CSchSymbol *schSymbol = new CSchSymbol();
    schSymbol->m_objType = 'S';

    if (FindNode(iNode->GetChildren(), wxT("partNum"))) {
        FindNode(iNode->GetChildren(), wxT("partNum'"))->GetNodeContent().ToLong(&num);
        schSymbol->m_partNum = (int)num;
    }

    if (FindNode(iNode->GetChildren(), wxT("symbolRef"))) {
        FindNode(iNode->GetChildren(), wxT("symbolRef"))->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        schSymbol->m_module.text = propValue;
    }

    if (FindNode(iNode->GetChildren(), wxT("refDesRef"))) {
        FindNode(iNode->GetChildren(), wxT("refDesRef"))->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        schSymbol->m_reference.text = propValue;
    }

    if (FindNode(iNode->GetChildren(), wxT("pt"))) {
        SetPosition(FindNode(iNode->GetChildren(), wxT("pt"))->GetNodeContent(),
                    sch->m_defaultMeasurementUnit, &schSymbol->m_positionX, &schSymbol->m_positionY, actualConversion);
    }

    if (FindNode(iNode->GetChildren(), wxT("isFlipped"))) {
        str = FindNode(iNode->GetChildren(), wxT("isFlipped"))->GetNodeContent();
        str.Trim(false);
        str.Trim(true);
        if (str == wxT("True")) schSymbol->m_mirror = 1;
    }

    if (FindNode(iNode->GetChildren(), wxT("rotation")))
        schSymbol->m_rotation = StrToInt1Units(FindNode(iNode->GetChildren(), wxT("rotation"))->GetNodeContent());


    lNode = iNode->GetChildren();
    // update physical symbol textts positions
    t = false; r = false;
    while (lNode) {
        if (lNode->GetName() == wxT("attr")) {
            lNode->GetPropVal(wxT("Name"), &propValue);
            propValue.Trim(false);
            propValue.Trim(true);
            if (propValue == wxT("Value")) { // WHY ???? IS IT SWITCHED IN PCAD ?
                r = true;
                SetTextParameters(lNode, &schSymbol->m_reference, sch->m_defaultMeasurementUnit, actualConversion);
            }

            if (propValue == wxT("RefDes")) { // WHY ???? IS IT SWITCHED IN PCAD ?
                r = true;
                SetTextParameters(lNode, &schSymbol->m_typ, sch->m_defaultMeasurementUnit, actualConversion);
            }
        }
        lNode = lNode->GetNext();
    }

    lNode = iNode;
    // also symbol from library as name of component as is known in schematics library
    while (lNode->GetName() != wxT("www.lura.sk"))
        lNode = lNode->GetParent();

    lNode = FindNode(lNode->GetChildren(), wxT("netlist"));
    if (lNode) {
        lNode = FindNode(lNode->GetChildren(), wxT("compInst"));
        while (lNode) {
            lNode->GetPropVal(wxT("Name"), &propValue);
            if (lNode->GetName() == wxT("compInst") && propValue == schSymbol->m_reference.text) {
                // Type - or Value , depends on version
                if (FindNode(lNode->GetChildren(), wxT("compValue"))) {
                    FindNode(lNode->GetChildren(), wxT("compValue"))->GetPropVal(wxT("Name"), &propValue);
                    propValue.Trim(false);
                    propValue.Trim(true);
                    schSymbol->m_typ.text = propValue;
                }
                else if (FindNode(lNode->GetChildren(), wxT("originalName"))) {
                    FindNode(lNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue);
                    schSymbol->m_typ.text = propValue;
                }
                // Pattern
                if (FindNode(lNode->GetChildren(), wxT("patternName"))) {
                    FindNode(lNode->GetChildren(), wxT("patternName"))->GetPropVal(wxT("Name"), &propValue);
                    schSymbol->m_attachedPattern = propValue;
                }
                else if (FindNode(lNode->GetChildren(), wxT("originalName"))) {
                    FindNode(lNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue);
                    schSymbol->m_attachedPattern = propValue;
                }
                // Symbol
                if (FindNode(lNode->GetChildren(), wxT("originalName"))) {
                    FindNode(lNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue);
                    schSymbol->m_attachedSymbol = propValue;
                }
                else if (FindNode(lNode->GetChildren(), wxT("compRef"))) {
                    FindNode(lNode->GetChildren(), wxT("compRef"))->GetPropVal(wxT("Name"), &propValue);
                    schSymbol->m_attachedSymbol = propValue;
                }
            }
            lNode = lNode->GetNext();
        }
    }

    // is reference position missing ?
    lNode = iNode;
    if (r == false || t == false) {
        while (lNode->GetName() != wxT("www.lura.sk"))
            lNode = lNode->GetParent();

        lNode = FindNode(lNode->GetChildren(), wxT("library"));
        if (lNode) {
            lNode = FindNode(lNode->GetChildren(), wxT("symbolDef"));
            while (lNode) {
                lNode->GetPropVal(wxT("Name"), &propValue);
                if (lNode->GetName() == wxT("symbolDef") && propValue == schSymbol->m_module.text) {
                    lNode = lNode->GetChildren();
                    while (lNode) {
                        if (lNode->GetName() == wxT("attr")) {
                            // Reference
                            lNode->GetPropVal(wxT("Name"), &propValue);
                            propValue.Trim(false);
                            propValue.Trim(true);
                            if (propValue == wxT("RefDes"))
                                SetTextParameters(lNode, &schSymbol->m_reference, sch->m_defaultMeasurementUnit, actualConversion);
                            // Type
                            if (propValue == wxT("Type {Type}"))
                                SetTextParameters(lNode, &schSymbol->m_typ, sch->m_defaultMeasurementUnit, actualConversion);
                            // OR
                            if (propValue == wxT("Type"))
                                SetTextParameters(lNode, &schSymbol->m_typ, sch->m_defaultMeasurementUnit, actualConversion);
                        }
                        lNode = lNode->GetNext();
                    }
                }

                if (lNode) lNode = lNode->GetNext();
            }
        }
    }

    return schSymbol;
}

/*
function CreateSCHLine(iNode:IXMLNode):THSCHLine;
var lNode:IXMLNode;
begin
  SCHLine:=THSCHLine.Create();
  SCHLine.ObjType:='W';
  if Assigned(iNode.ChildNodes.FindNode('line')) then
  begin
  end;
      //                  SCHWire.PartNum:=StrToInt(iNode.ChildNodes.FindNode('line').Text);
  result:=SCHLine;
end;
*/

/*
procedure DoLibrary(aNode:IXMLNode);  // FULL PROCES OF LIBRARY CONVERSION
begin
       aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('library');
       if Assigned(aNode) then // ORIGINAL  Modules
       begin
         aNode:=aNode.ChildNodes.First;
          while Assigned(aNode) do
            begin
              if (aNode.NodeName= 'compDef') then
                    SCH.SCHComponents.Add(CreateSCHModule(aNode));
              aNode := aNode.NextSibling;
           Application.ProcessMessages;
           end;
       end;
       aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('library');
       if Assigned(aNode) then // ALIASes
       begin
         aNode:=aNode.ChildNodes.First;
          while Assigned(aNode) do
            begin
              if (aNode.NodeName= 'compAlias') then
                  begin
                    DoAlias(aNode.Attributes['Name']);
                  end;
              aNode := aNode.NextSibling;
           Application.ProcessMessages;
           end;
       end;
end;
*/

// FULL PROCESS OF LIBRARY CONVERSION
void DoLibrary(wxXmlDocument *xmlDoc, CSch *sch, wxStatusBar* statusBar, wxString actualConversion) {
    wxXmlNode *aNode;
    wxString propValue;

    aNode = FindNode(xmlDoc->GetRoot()->GetChildren(), wxT("library"));
    if (aNode) { // ORIGINAL Modules
        aNode = aNode->GetChildren();
        while (aNode) {
            if (aNode->GetName() == wxT("compDef"))
                sch->m_schComponents.Add(CreateSchModule(aNode, statusBar, sch, actualConversion));
            aNode = aNode->GetNext();
        }
    }

    aNode = FindNode(xmlDoc->GetRoot()->GetChildren(), wxT("library"));
    if (aNode) { // ALIASes
        aNode = aNode->GetChildren();
        while (aNode) {
            if (aNode->GetName() == wxT("compAlias")) {
                aNode->GetPropVal(wxT("Name"), &propValue);
                DoAlias(propValue, sch);
            }
            aNode = aNode->GetNext();
        }
    }
}

/*
function CheckJunction(iSCHLine:THSCHLine;index:integer):THSCHJunction;
var i,j,k,code:integer;
    x,y:integer;
    p:boolean;
// LinesIntersect procedure is copied from website
// http://www.pdas.com/lineint.htm
// extended format replaced by integer
PROCEDURE LinesIntersect(const x1,y1,x2,y2: integer; // first line
                         const x3,y3,x4,y4: integer; // second line
                         VAR code : INTEGER; // =0 OK; =1 lines parallel
                         VAR x,y : integer); // intersection point

VAR
    a1, a2, b1, b2, c1, c2 : Extended; // Coefficients of line eqns./
    denom : extended;

BEGIN
  a1:= y2-y1;
  b1:= x1-x2;
  c1:= x2*y1 - x1*y2;  // a1*x + b1*y + c1 = 0 is line 1

  a2:= y4-y3;
  b2:= x3-x4;
  c2:= x4*y3 - x3*y4;  // a2*x + b2*y + c2 = 0 is line 2

  denom:= a1*b2 - a2*b1;
  IF denom = 0 THEN
    BEGIN
      code:=1;
      EXIT
    END;

  x:=Round((b1*c2 - b2*c1)/denom);
  y:=Round((a2*c1 - a1*c2)/denom);
  code:=0
END;
*/

void LinesIntersect(const int x1, const int y1, const int x2, const int y2, // first line
                    const int x3, const int y3, const int x4, const int y4, // second line
                    int *code, // =0 OK; =1 lines parallel
                    int *x, int *y) { // intersection point
    long double a1, a2, b1, b2, c1, c2; // Coefficients of line eqns./
    long double denom;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = (long double)x2 * (long double)y1 - (long double)x1 * (long double)y2;  // a1*x + b1*y + c1 = 0 is line 1

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = (long double)x4 * (long double)y3 - (long double)x3 * (long double)y4;  // a2*x + b2*y + c2 = 0 is line 2

    denom = a1 * b2 - a2 * b1;
    if (denom == 0) {
        *code = 1;
    }
    else {
        *x = KiROUND((b1 * c2 - b2 * c1) / denom);
        *y = KiROUND((a2 * c1 - a1 * c2) / denom);
        *code = 0;
    }
}

/*
function IsPointOnLine(x,y:integer;l:THSCHLine):boolean;
VAR
    px,py:boolean;
    a,b,c : Extended; // Coefficients of line eqns./
begin
  a:= l.ToY-l.PositionY;
  b:= l.PositionX-l.ToX;
  c:= l.ToX*l.PositionY - l.PositionX*l.ToY;  // a1*x + b1*y + c1 = 0 is line 1
  px:=false; py:=false;
  if (a*x+b*y+c)=0 then
  begin
       if ((l.PositionX<=l.ToX) and (x>=l.PositionX) and (x<=l.ToX)) then
                          px:=true;
       if ((l.ToX<=l.PositionX) and (x>=l.ToX) and (x<=l.PositionX)) then
                       px:=true;
       if ((l.PositionY<=l.ToY) and (y>=l.PositionY) and (y<=l.ToY)) then
                          py:=true;
       if ((l.ToY<=l.PositionY) and (y>=l.ToY) and (y<=l.PositionY)) then
                         py:=true;
       if (px and py) then  Result:=true
  end
  else result:=false;
end;
*/

bool IsPointOnLine(int x, int y, CSchLine *l) {
    long double a, b, c; // Coefficients of line eqns./
    bool px, py, result;

    a = l->m_toY - l->m_positionY;
    b = l->m_positionX - l->m_toX;
    c = (long double)l->m_toX * (long double)l->m_positionY -
        (long double)l->m_positionX * (long double)l->m_toY;  // a1*x + b1*y + c1 = 0 is line 1
    px = false; py = false;
    if (a * (long double)x + b * (long double)y + c == 0) {
        if (l->m_positionX <= l->m_toX && x >= l->m_positionX && x <= l->m_toX)
            px = true;
        if (l->m_toX <= l->m_positionX && x >= l->m_toX && x <= l->m_positionX)
            px = true;
        if (l->m_positionY <= l->m_toY && y >= l->m_positionY && y <= l->m_toY)
            py = true;
        if (l->m_toY <= l->m_positionY && y >= l->m_toY && y <= l->m_positionY)
            py = true;
        if (px && py) result = true;
    }
    else result = false;

    return result;
}

/*
function CJunction(x,y:integer;net:string):THSCHJunction;
begin
  result:=THSCHJunction.Create(); // midle of line intersection
  result.ObjType:='J';
  result.PositionX:=x;
  result.PositionY:=y;
  result.Net:=iSCHLine.Net;
end;
*/

CSchJunction *CJunction(int x, int y, wxString net) {
    CSchJunction *result = new CSchJunction(); // middle of line intersection
    result->m_objType = 'J';
    result->m_positionX = x;
    result->m_positionY = y;
    result->m_net = net;

    return result;
}

/*
begin
 result:=nil;
 for i:=0 to SCH.SCHComponents.Count-1 do
 begin
   if ((THSCHComponent(SCH.SCHComponents[i]).ObjType='L')
      and (index<>i)) then // not  for itself
       begin
          if (iSCHLine.Net=THSCHLine(SCH.SCHComponents[i]).net) then
          begin
            // IS JUNCTION ????
            LinesIntersect(iSCHLine.PositionX,iSCHLine.PositionY,iSCHLine.ToX,iSCHLine.ToY,
                           THSCHLine(SCH.SCHComponents[i]).PositionX,THSCHLine(SCH.SCHComponents[i]).PositionY,
                           THSCHLine(SCH.SCHComponents[i]).ToX,THSCHLine(SCH.SCHComponents[i]).ToY,
                           code,x,y);
            if code=0 then
            // there is intersection, is it inside line segment ?
            begin
              if IsPointOnLine(x,y,iSCHLine) then
              begin
              // Point Candidate
              // Firstly to check if the otrher line with dufferent NET is crossing this point, if so, there is not point !
               p:=true;
               for j:=0 to SCH.SCHComponents.Count-1 do
               begin
                  if ((THSCHComponent(SCH.SCHComponents[j]).ObjType='L')
                       and p ) then // not  for itself
                  begin
                     if THSCHLine(SCH.SCHComponents[j]).Net<>THSCHLine(SCH.SCHComponents[i]).Net then
                         if IsPointOnLine(x,y,THSCHLine(SCH.SCHComponents[j])) then
                                           p:=false; // NOT POINT - net cross
                  end;
               end;
               if p then result:=CJunction(x,Y,iSCHLine.Net);
            end;
           end;
         end;
      end;
 end;
end;
*/

CSchJunction *CheckJunction(CSchLine *iSchLine, int index, CSch *sch) {
    CSchJunction *result = NULL;
    int i, j, code, x, y;
    bool p;

    for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
        if ((sch->m_schComponents[i])->m_objType == 'L' &&
             index != i) { // not  for itself
            if (iSchLine->m_net == ((CSchLine *)sch->m_schComponents[i])->m_net) {
                // IS JUNCTION ????
                LinesIntersect(iSchLine->m_positionX, iSchLine->m_positionY, iSchLine->m_toX, iSchLine->m_toY,
                               ((CSchLine *)sch->m_schComponents[i])->m_positionX,
                               ((CSchLine *)sch->m_schComponents[i])->m_positionY,
                               ((CSchLine *)sch->m_schComponents[i])->m_toX,
                               ((CSchLine *)sch->m_schComponents[i])->m_toY,
                               &code, &x, &y);
                if (code == 0) {
                    // there is intersection, is it inside line segment ?
                    if (IsPointOnLine(x, y, iSchLine)) {
                        // Point Candidate
                        // Firstly to check if the otrher line with dufferent NET is crossing this point, if so, there is not point !
                        p = true;
                        for (j = 0; j < (int)sch->m_schComponents.GetCount(); j++) {
                            if ((sch->m_schComponents[j])->m_objType == 'L'
                                 && p) { // not  for itself
                                if (((CSchLine *)sch->m_schComponents[j])->m_net != ((CSchLine *)sch->m_schComponents[i])->m_net)
                                    if (IsPointOnLine(x, y, (CSchLine *)sch->m_schComponents[j]))
                                           p = false; // NOT POINT - net cross
                            }
                        }
                        if (p) result = CJunction(x, y, iSchLine->m_net);
                    }
                }
            }
        }
    }

    return result;
}

/*
begin
  SCH:=THSCH.Create;
  try
   XMLDoc:=TXMLDocument.Create(XMLFileNAme);
   XMLDoc.Active := True;
   // Defaut measurement units
    aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('asciiHeader');
    if Assigned(aNode) then
    begin
      aNode:=aNode.ChildNodes.FindNode('fileUnits');
      if Assigned(aNode) then
          SCH.DefaultMeasurementUnit:=trim(aNode.Text);
    end;
    // Allways also library file
    IsJunction:=False;
    ActualConversion:='SCHLIB';
    DoLibrary(aNode);
    aNode:=XMLDoc.DocumentElement.ChildNodes.FindNode('schematicDesign');
    if Assigned(aNode) then
    begin //  SCHEMATIC FILE
      // aNode is schematicDesign node actually
      ActualConversion:='SCH';
      aNode:=aNode.ChildNodes.FindNode('sheet').ChildNodes.First();
      while Assigned(aNode) do
      begin
       if (aNode.NodeName= 'symbol') then
                    SCH.SCHComponents.Add(CreateSCHSymbol(aNode));
       if (aNode.NodeName= 'wire') then
           if Assigned(aNode.ChildNodes.FindNode('line')) then
                         SCH.SCHComponents.Add(CreateLine(aNode.ChildNodes.FindNode('line'),0));
       if (aNode.NodeName= 'bus') then
                         SCH.SCHComponents.Add(CreateBus(aNode));
       if (aNode.NodeName= 'junction') then
                         SCH.SCHComponents.Add(CreateJunction(aNode));
       anode:=aNode.NextSibling;
      end;
       // POSTPROCESS -- SET/OPTIMIZE NEW SCH POSITION

       SCH.SizeX:=1000000;SCH.SizeY:=0;
       for i:=0 to SCH.SCHComponents.Count-1 do
         begin
          with THSCHComponent(SCH.SCHComponents[i]) do
            begin
               if (PositionY<SCH.SizeY) then SCH.SizeY:=PositionY;
               if (PositionX<SCH.SizeX) and (PositionX>0) then SCH.SizeX:=PositionX;
            end;
        end;
       // correction
       SCH.SizeY:=SCH.SizeY-1000;
       SCH.SizeX:=SCH.SizeX-1000;
       for i:=0 to SCH.SCHComponents.Count-1 do
        begin
          with THSCHComponent(SCH.SCHComponents[i]) do
            begin
               PositionY:=PositionY-SCH.SizeY;
               PositionX:=PositionX-SCH.SizeX;
               if ObjType='L' then
                    begin
                       THSCHLine(SCH.SCHComponents[i]).ToY:=THSCHLine(SCH.SCHComponents[i]).ToY-SCH.SizeY;
                       THSCHLine(SCH.SCHComponents[i]).LabelText.TextPositionY:=THSCHLine(SCH.SCHComponents[i]).LabelText.TextPositionY-SCH.SizeY;
                       THSCHLine(SCH.SCHComponents[i]).ToX:=THSCHLine(SCH.SCHComponents[i]).ToX-SCH.SizeX;
                       THSCHLine(SCH.SCHComponents[i]).LabelText.TextPositionX:=THSCHLine(SCH.SCHComponents[i]).LabelText.TextPositionX-SCH.SizeX;
                    end;
            end;
         end;
       // final sheet settings
       for i:=0 to SCH.SCHComponents.Count-1 do
         begin
          with THSCHComponent(SCH.SCHComponents[i]) do
            begin
               if (PositionY<SCH.SizeY) then SCH.SizeY:=PositionY;
               if (PositionX>SCH.SizeX) then SCH.SizeX:=PositionX;
            end;
        end;
       SCH.SizeY:=-SCH.SizeY; // is in absolute units
       SCH.SizeX:=SCH.SizeX+1000;
       SCH.SizeY:=SCH.SizeY+1000;
// A4 is minimum $Descr A4 11700 8267
       if SCH.SizeX<11700 then SCH.SizeX:=11700;
       if SCH.SizeY<8267 then SCH.SizeY:=8267;

      // POSTPROCESS -- CREATE JUNCTIONS FROM NEWTLIST
      if not (IsJunction) then
      begin
        if (Application.MessageBox(('There are not JUNCTIONS in your schematics file .'
                                   +' It can be, that your design is without Junctions.'
                                   +' But it can be that your input file is in obsolete format.'
                                   +' Would you like to run postprocess and create junctions from Netlist/Wires information ?'
                                   +'    YOU HAVE TO CHECK/CORRECT Juntions in converted design, placement is only approximation !' )
                    ,'Question', MB_YESNO)=IDYES)
        then
         for i:=0 to SCH.SCHComponents.Count-1 do
         begin
          if THSCHComponent(SCH.SCHComponents[i]).ObjType='L' then
           begin
             SCHJunction:=CheckJunction(THSCHLine(SCH.SCHComponents[i]),i);
             if Assigned(SCHJunction) then
                 SCH.SCHComponents.Add(SCHJunction);
           end;
         end;
      end;

    end;  //  SCHEMATIC FILE
   XMLDoc.Active := False;
  finally
  end;
  result:=SCH;
end;


end.
*/

CSch ProcessXMLtoSch(wxStatusBar* statusBar, wxString XMLFileName, wxString actualConversion) {
    CSch sch;
    wxXmlDocument xmlDoc;
    wxXmlNode *aNode;
    CSchComponent *schComp;
    CSchJunction *schJunction;
    bool isJunction;
    int i;

    if (!xmlDoc.Load(XMLFileName)) return sch;

    // Defaut measurement units
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("asciiHeader"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("fileUnits"));
        if (aNode) {
            sch.m_defaultMeasurementUnit = aNode->GetNodeContent();
            sch.m_defaultMeasurementUnit.Trim(true);
            sch.m_defaultMeasurementUnit.Trim(false);
        }
    }

    // Allways also library file
    isJunction = false;
    actualConversion = wxT("SCHLIB");

    DoLibrary(&xmlDoc, &sch, statusBar, actualConversion);

    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("schematicDesign"));
    if (aNode) {
        //  SCHEMATIC FILE
        // aNode is schematicDesign node actually
        actualConversion = wxT("SCH");
        aNode = FindNode(aNode->GetChildren(), wxT("sheet"))->GetChildren();
        while (aNode) {
            if (aNode->GetName() == wxT("symbol"))
                sch.m_schComponents.Add(CreateSCHSymbol(aNode, &sch, actualConversion));

            if (aNode->GetName() == wxT("wire"))
                if (FindNode(aNode->GetChildren(), wxT("Line")))
                    sch.m_schComponents.Add(CreateLine(FindNode(aNode->GetChildren(), wxT("Line")), 0, &sch, actualConversion));

            if (aNode->GetName() == wxT("bus"))
                sch.m_schComponents.Add(CreateBus(aNode, &sch, actualConversion));

            if (aNode->GetName() == wxT("junction"))
                sch.m_schComponents.Add(CreateJunction(aNode, &sch, &isJunction, actualConversion));

            aNode = aNode->GetNext();
        }

        // POSTPROCESS -- SET/OPTIMIZE NEW SCH POSITION

        sch.m_sizeX = 1000000; sch.m_sizeY = 0;
        for (i = 0; i < (int)sch.m_schComponents.GetCount(); i++) {
            schComp = sch.m_schComponents[i];
            if (schComp->m_positionY < sch.m_sizeY)
                sch.m_sizeY = schComp->m_positionY;
            if (schComp->m_positionX < sch.m_sizeX && schComp->m_positionX > 0)
                sch.m_sizeX = schComp->m_positionX;
        }
        // correction
        sch.m_sizeY = sch.m_sizeY - 1000;
        sch.m_sizeX = sch.m_sizeX - 1000;
        for (i = 0; i < (int)sch.m_schComponents.GetCount(); i++) {
            schComp = sch.m_schComponents[i];
            schComp->m_positionY -= sch.m_sizeY;
            schComp->m_positionX -= sch.m_sizeX;
            if (schComp->m_objType == 'L') {
                ((CSchLine *)schComp)->m_toY -= sch.m_sizeY;
                ((CSchLine *)schComp)->m_labelText.textPositionY -= sch.m_sizeY;
                ((CSchLine *)schComp)->m_toX -= sch.m_sizeX;
                ((CSchLine *)schComp)->m_labelText.textPositionX -= sch.m_sizeX;
            }
        }
        // final sheet settings
        for (i = 0; i < (int)sch.m_schComponents.GetCount(); i++) {
            schComp = sch.m_schComponents[i];
            if (schComp->m_positionY < sch.m_sizeY)
                sch.m_sizeY = schComp->m_positionY;
            if (schComp->m_positionX > sch.m_sizeX)
                sch.m_sizeX = schComp->m_positionX;
        }
        sch.m_sizeY = -sch.m_sizeY; // is in absolute units
        sch.m_sizeX = sch.m_sizeX + 1000;
        sch.m_sizeY = sch.m_sizeY + 1000;
        // A4 is minimum $Descr A4 11700 8267
        if (sch.m_sizeX < 11700) sch.m_sizeX = 11700;
        if (sch.m_sizeY < 8267) sch.m_sizeY = 8267;

        // POSTPROCESS -- CREATE JUNCTIONS FROM NEWTLIST
        if (!isJunction) {
            if (wxMessageBox(wxT("There are not JUNCTIONS in your schematics file ."
                                 " It can be, that your design is without Junctions."
                                 " But it can be that your input file is in obsolete format."
                                 " Would you like to run postprocess and create junctions from Netlist/Wires information ?"
                                 "    YOU HAVE TO CHECK/CORRECT Juntions in converted design, placement is only approximation !"),
                             wxEmptyString, wxYES_NO) == wxYES)
            {
                for (i = 0; i < (int)sch.m_schComponents.GetCount(); i++) {
                    schComp = sch.m_schComponents[i];
                    if (schComp->m_objType == 'L') {
                        schJunction = CheckJunction((CSchLine *)schComp, i, &sch);
                        if (schJunction)
                            sch.m_schComponents.Add(schJunction);
                    }
                }
            }
        }

    } //  SCHEMATIC FILE

    return sch;
}
