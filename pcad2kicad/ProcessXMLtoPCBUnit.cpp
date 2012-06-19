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
#include <PCBPadShape.h>
#include <PCBViaShape.h>


CPCBPad *CreatePCBPad(wxXmlNode *iNode, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    long num;
    wxString propValue, str;
    CPCBPadShape *padShape;
    CPCBPad *pcbPad = new CPCBPad(pcb, wxEmptyString);

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
            if (FindNode(lNode->GetChildren(), wxT("layerNumRef"))) {
                padShape = new CPCBPadShape(pcb);
                padShape->Parse(lNode, pcb->m_defaultMeasurementUnit, actualConversion);
                pcbPad->m_shapes.Add(padShape);
            }
        }

        lNode = lNode->GetNext();
    }

    return pcbPad;
}

CPCBVia *CreateVia(wxXmlNode *iNode, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode, *tNode;
    wxString propValue;
    CPCBViaShape *viaShape;
    CPCBVia *pcbVia = new CPCBVia(pcb);

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
                if (FindNode(lNode->GetChildren(), wxT("layerNumRef"))) {
                    viaShape = new CPCBViaShape(pcb);
                    viaShape->Parse(lNode, pcb->m_defaultMeasurementUnit, actualConversion);
                    pcbVia->m_shapes.Add(viaShape);
                }
            }

            lNode = lNode->GetNext();
        }
    }

    return pcbVia;
}

CPCBLine *CreateComponentLine(wxXmlNode *iNode, int l, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString propValue;
    CPCBLine *componentLine = new CPCBLine(pcb);

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

CPCBText *CreateComponentText(wxXmlNode *iNode, int l, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString str;
    CPCBText *componentText = new CPCBText(pcb);

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

static void FormPolygon(wxXmlNode *iNode, CVerticesArray *polygon, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    double x, y;

    lNode = FindNode(iNode->GetChildren(), wxT("pt"));
    while (lNode) {
        if (lNode->GetName() == wxT("pt")) {
            SetDoublePrecisionPosition(lNode->GetNodeContent(), pcb->m_defaultMeasurementUnit, &x, &y, actualConversion);
            polygon->Add(new wxRealPoint(x, y));
        }

        lNode = lNode->GetNext();
    }
}

CPCBPolygon *CreateComponentPolygon(wxXmlNode *iNode, int PCadLayer, CPCB *pcb, wxString actualConversion, wxStatusBar* statusBar) {
    wxXmlNode *lNode;
    wxString propValue;
    CPCBPolygon *componentPolygon = new CPCBPolygon(pcb);

    statusBar->SetStatusText(statusBar->GetStatusText() + wxT(" Polygon..."));
    componentPolygon->m_PCadLayer = PCadLayer;
    componentPolygon->m_KiCadLayer = pcb->m_layersMap[PCadLayer];
    componentPolygon->m_timestamp = pcb->GetNewTimestamp();

    lNode = FindNode(iNode->GetChildren(), wxT("netNameRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        componentPolygon->m_net = propValue;
    }

    // retrieve polygon outline
    FormPolygon(iNode, &componentPolygon->m_outline, pcb, actualConversion);

    componentPolygon->m_positionX = componentPolygon->m_outline[0]->x;
    componentPolygon->m_positionY = componentPolygon->m_outline[0]->y;

    // fill the polygon with the same contour as its outline is
    componentPolygon->m_islands.Add(new CVerticesArray);
    FormPolygon(iNode, componentPolygon->m_islands[0], pcb, actualConversion);

    return componentPolygon;
}

CPCBCopperPour *CreateComponentCopperPour(wxXmlNode *iNode, int PCadLayer, CPCB *pcb, wxString actualConversion, wxStatusBar* statusBar) {
    wxXmlNode *lNode, *tNode, *cNode;
    wxString pourType, str, propValue;
    int pourSpacing, thermalWidth;
    CVerticesArray *island, *cutout;
    CPCBCopperPour *componentCopperPour = new CPCBCopperPour(pcb);

    statusBar->SetStatusText(statusBar->GetStatusText() + wxT(" CooperPour..."));
    componentCopperPour->m_PCadLayer = PCadLayer;
    componentCopperPour->m_KiCadLayer = pcb->m_layersMap[PCadLayer];
    componentCopperPour->m_timestamp = pcb->GetNewTimestamp();

    str = FindNode(iNode->GetChildren(), wxT("pourType"))->GetNodeContent();
    str.Trim(false);
    pourType = str.MakeUpper();

    lNode = FindNode(iNode->GetChildren(), wxT("netNameRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        componentCopperPour->m_net = propValue;
    }

    SetWidth(FindNode(iNode->GetChildren(), wxT("width"))->GetNodeContent(), pcb->m_defaultMeasurementUnit, &componentCopperPour->m_width, actualConversion);
    if (FindNode(iNode->GetChildren(), wxT("pourSpacing")))
        SetWidth(FindNode(iNode->GetChildren(), wxT("pourSpacing"))->GetNodeContent(), pcb->m_defaultMeasurementUnit, &pourSpacing, actualConversion);
    if (FindNode(iNode->GetChildren(), wxT("thermalWidth")))
        SetWidth(FindNode(iNode->GetChildren(), wxT("thermalWidth"))->GetNodeContent(), pcb->m_defaultMeasurementUnit, &thermalWidth, actualConversion);

    lNode = FindNode(iNode->GetChildren(), wxT("pcbPoly"));
    if (lNode) {
        // retrieve copper pour outline
        FormPolygon(lNode, &componentCopperPour->m_outline, pcb, actualConversion);

        componentCopperPour->m_positionX = componentCopperPour->m_outline[0]->x;
        componentCopperPour->m_positionY = componentCopperPour->m_outline[0]->y;

        lNode = FindNode(iNode->GetChildren(), wxT("island"));
        while (lNode) {
            tNode = FindNode(lNode->GetChildren(), wxT("islandOutline"));
            if (tNode) {
                island = new CVerticesArray;
                FormPolygon(tNode, island, pcb, actualConversion);
                componentCopperPour->m_islands.Add(island);
                tNode = FindNode(lNode->GetChildren(), wxT("cutout"));
                while (tNode) {
                    cNode = FindNode(tNode->GetChildren(), wxT("cutoutOutline"));
                    if (cNode) {
                        cutout = new CVerticesArray;
                        FormPolygon(cNode, cutout, pcb, actualConversion);
                        componentCopperPour->m_cutouts.Add(cutout);
                    }

                    tNode  = tNode->GetNext();
                }
            }

            /*tNode:=lNode.ChildNodes.FindNode('thermal');
            while  Assigned(tNode) do
            begin
                DrawThermal(tNode, componentCopperPour.fill_lines, PCADlayer, thermalWidth,
                            componentCopperPour.timestamp);
                tNode := tNode.NextSibling;
            end;*/

            lNode = lNode->GetNext();
        }
    }
    else componentCopperPour = NULL;

    return componentCopperPour;
}

// It seems that the same cutouts (with the same vertices) are inside of copper pour objects
CPCBCutout *CreateComponentCutout(wxXmlNode *iNode, int PCadLayer, CPCB *pcb, wxString actualConversion) {
    CPCBCutout *componentCutout = new CPCBCutout(pcb);

    componentCutout->m_PCadLayer = PCadLayer;
    componentCutout->m_KiCadLayer = pcb->m_layersMap[PCadLayer];

    // retrieve cutout outline
    FormPolygon(iNode, &componentCutout->m_outline, pcb, actualConversion);

    componentCutout->m_positionX = componentCutout->m_outline[0]->x;
    componentCutout->m_positionY = componentCutout->m_outline[0]->y;

    return componentCutout;
}

CPCBArc *CreateComponentArc(wxXmlNode *iNode, int l, CPCB *pcb, wxString actualConversion) {
    wxXmlNode *lNode;
    double r, a;
    CPCBArc *componentArc = new CPCBArc(pcb);

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

void DoLayerContentsObjects(wxXmlNode *iNode, CPCBModule *pcbModule, CPCBComponentsArray *list, wxStatusBar* statusBar, CPCB *pcb, wxString actualConversion) {
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
                if (tNode && pcbModule) {
                    //TODO: to understand and may be repair
                    // Alexander Lunev: originally in Delphi version of the project there was a strange access
                    //  to pcbModule->m_name (it was global variable). This access is necessary
                    //  when the function DoLayerContentsObjects() is called from function CreatePCBModule().
                    //  However it is not clear whether the access is required when the function DoLayerContentsObjects()
                    //  is called from function ProcessXMLtoPCBLib().
                    SetFontProperty(tNode, &pcbModule->m_name, pcb->m_defaultMeasurementUnit, actualConversion);
                }
            }
        }

        // added  as Sergeys request 02/2008
        if (lNode->GetName() == wxT("arc"))
            list->Add(CreateComponentArc(lNode, PCadLayer, pcb, actualConversion));
        if (lNode->GetName() == wxT("triplePointArc"))
            list->Add(CreateComponentArc(lNode, PCadLayer, pcb, actualConversion));
        if (lNode->GetName() == wxT("pcbPoly"))
            list->Add(CreateComponentPolygon(lNode, PCadLayer, pcb, actualConversion, statusBar));
        if (lNode->GetName() == wxT("copperPour95")) {
            poly = CreateComponentCopperPour(lNode, PCadLayer, pcb, actualConversion, statusBar);
            if (poly != NULL) list->Add(poly);
        }

        if (lNode->GetName() == wxT("polyCutOut")) {
            // It seems that the same cutouts (with the same vertices) are inside of copper pour objects

            // list of polygons....
            tNode = lNode;
            tNode = FindNode(tNode->GetChildren(), wxT("pcbPoly"));
            if (tNode)
                list->Add(CreateComponentCutout(tNode, PCadLayer, pcb, actualConversion));
        }

        lNode = lNode->GetNext();
    }
}

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

CPCBModule *CreatePCBModule(wxXmlNode *iNode, wxStatusBar* statusBar, CPCB *pcb, wxString actualConversion) {
    CPCBModule *pcbModule;
    wxXmlNode *lNode, *tNode, *mNode;
    wxString propValue, str;

    FindNode(iNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue);
    propValue.Trim(false);
    pcbModule = new CPCBModule(pcb, propValue);

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
            DoLayerContentsObjects(lNode, pcbModule, &pcbModule->m_moduleObjects, statusBar, pcb, actualConversion);
        lNode = lNode->GetNext();
    }

    // map pins
    lNode = FindPinMap(iNode);

    if (lNode) {
        mNode = lNode->GetChildren();
        while (mNode) {
            if (mNode->GetName() == wxT("padNum")) {
                str = mNode->GetNodeContent();
                mNode = mNode->GetNext();
                if (!mNode) break;
                mNode->GetPropVal(wxT("Name"), &propValue);
                SetPadName(str, propValue, pcbModule);
                mNode = mNode->GetNext();
            }
            else {
                mNode = mNode->GetNext();
                if (!mNode) break;
                mNode = mNode->GetNext();
            }
        }
    }

    return pcbModule;
}

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

void DoPCBComponents(wxXmlNode *iNode, CPCB *pcb, wxXmlDocument *xmlDoc, wxString actualConversion, wxStatusBar* statusBar) {
    wxXmlNode *lNode, *tNode, *mNode;
    CPCBModule *mc;
    wxString cn, str, propValue;

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
                    if (tNode) {
                        mNode = tNode->GetChildren();
                        while (mNode) {
                            if (mNode->GetName() == wxT("padNum")) {
                                str = mNode->GetNodeContent();
                                mNode = mNode->GetNext();
                                if (!mNode) break;
                                mNode->GetPropVal(wxT("Name"), &propValue);
                                SetPadName(str, propValue, mc);
                                mNode = mNode->GetNext();
                            }
                            else {
                                mNode = mNode->GetNext();
                                if (!mNode) break;
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
    lName = lName.MakeUpper();
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

void ProcessXMLtoPCBLib(CPCB *pcb, wxStatusBar* statusBar, wxString XMLFileName, wxString actualConversion) {
    wxXmlDocument xmlDoc;
    wxXmlNode *aNode;
    CNet *net;
    CPCBComponent *comp;
    wxString cr, pr;
    int i, j;

    if (!xmlDoc.Load(XMLFileName)) return;

    // Defaut measurement units
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("asciiHeader"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("fileUnits"));
        if (aNode) {
            pcb->m_defaultMeasurementUnit = aNode->GetNodeContent();
            pcb->m_defaultMeasurementUnit.Trim(true);
            pcb->m_defaultMeasurementUnit.Trim(false);
        }
    }

    // Layers mapping
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("pcbDesign"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("layerDef"));
        while (aNode) {
            if (aNode->GetName() == wxT("layerDef"))
                MapLayer(aNode, pcb);
            aNode = aNode->GetNext();
        }
    }

    // NETLIST
    statusBar->SetStatusText(wxT("Loading NETLIST "));
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("netlist"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("net"));
        while (aNode) {
            DoPCBNet(aNode, pcb);
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
                DoPCBComponents(aNode, pcb, &xmlDoc, actualConversion, statusBar);
            // objects
            if (aNode->GetName() == wxT("layerContents"))
                DoLayerContentsObjects(aNode, NULL, &pcb->m_pcbComponents, statusBar, pcb, actualConversion);
            aNode = aNode->GetNext();
        }

        // POSTPROCESS -- SET NETLIST REFERENCES
        statusBar->SetStatusText(wxT("Processing NETLIST "));
        for (i = 0; i < (int)pcb->m_pcbNetlist.GetCount(); i++) {
            net = pcb->m_pcbNetlist[i];
            for (j = 0; j < (int)net->m_netNodes.GetCount(); j++) {
                cr = net->m_netNodes[j]->m_compRef;
                cr.Trim(false);
                cr.Trim(true);
                pr = net->m_netNodes[j]->m_pinRef;
                pr.Trim(false);
                pr.Trim(true);
                ConnectPinToNet(pcb, cr, pr, net->m_name);
            }
        }

        // POSTPROCESS -- FLIP COMPONENTS
        for (i = 0; i < (int)pcb->m_pcbComponents.GetCount(); i++) {
            if (pcb->m_pcbComponents[i]->m_objType == 'M')
                ((CPCBModule *)pcb->m_pcbComponents[i])->Flip();
        }

        // POSTPROCESS -- SET/OPTIMIZE NEW PCB POSITION
        statusBar->SetStatusText(wxT("Optimizing BOARD POSITION "));
        pcb->m_sizeX = 10000000; pcb->m_sizeY = 0;
        for (i = 0; i < (int)pcb->m_pcbComponents.GetCount(); i++) {
            comp = pcb->m_pcbComponents[i];
            if (comp->m_positionY < pcb->m_sizeY) pcb->m_sizeY = comp->m_positionY; // max Y
            if (comp->m_positionX < pcb->m_sizeX && comp->m_positionX > 0) pcb->m_sizeX = comp->m_positionX; // Min X
        }
        pcb->m_sizeY -= 10000;
        pcb->m_sizeX -= 10000;
        statusBar->SetStatusText(wxT(" POSITIONING POSTPROCESS "));
        for (i = 0; i < (int)pcb->m_pcbComponents.GetCount(); i++)
            pcb->m_pcbComponents[i]->SetPosOffset(-pcb->m_sizeX, -pcb->m_sizeY);

        pcb->m_sizeX = 0; pcb->m_sizeY = 0;
        for (i = 0; i < (int)pcb->m_pcbComponents.GetCount(); i++) {
            comp = pcb->m_pcbComponents[i];
            if (comp->m_positionY < pcb->m_sizeY) pcb->m_sizeY = comp->m_positionY; // max Y
            if (comp->m_positionX > pcb->m_sizeX) pcb->m_sizeX = comp->m_positionX; // Min X
        }

        // SHEET SIZE CALCULATION
        pcb->m_sizeY = -pcb->m_sizeY; // it is in absolute units
        pcb->m_sizeX += 10000;
        pcb->m_sizeY += 10000;
        // A4 is minimum $Descr A4 11700 8267
        if (pcb->m_sizeX < 11700) pcb->m_sizeX = 11700;
        if (pcb->m_sizeY < 8267) pcb->m_sizeY = 8267;
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
                    pcb->m_pcbComponents.Add(CreatePCBModule(aNode, statusBar, pcb, actualConversion));

                aNode = aNode->GetNext();
            }
        }
    }
}
