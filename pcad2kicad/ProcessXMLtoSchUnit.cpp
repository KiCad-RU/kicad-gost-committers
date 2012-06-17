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

#include <SchArc.h>

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
                    if (FindNode(FindNode(iNode->GetChildren(), wxT("pinName"))->GetChildren(), wxT("text"))) {
                        SetTextParameters(FindNode(FindNode(iNode->GetChildren(), wxT("pinName"))->GetChildren(), wxT("text")),
                                          &lpn,
                                          sch->m_defaultMeasurementUnit,
                                          actualConversion);
                    }
                }
                if (FindNode(iNode->GetChildren(), wxT("pinDes"))) {
                    lpn = schPin->m_pinName;
                    if (FindNode(FindNode(iNode->GetChildren(), wxT("pinDes"))->GetChildren(), wxT("text"))) {
                        SetTextParameters(FindNode(FindNode(iNode->GetChildren(), wxT("pinDes"))->GetChildren(), wxT("text")),
                                          &lpn,
                                          sch->m_defaultMeasurementUnit,
                                          actualConversion);
                    }
                }
            }
        }
    }
}

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
                        iSchModule->m_moduleObjects.Add(new CSchArc(tNode, symbolIndex, sch->m_defaultMeasurementUnit, actualConversion));
                    tNode = tNode->GetNext();
                }
                tNode = FindNode(ttNode->GetChildren(), wxT("triplePointArc"));
                while (tNode) {
                    if (tNode->GetName() == wxT("triplePointArc"))
                        iSchModule->m_moduleObjects.Add(new CSchArc(tNode, symbolIndex, sch->m_defaultMeasurementUnit, actualConversion));
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

CSchSymbol *CreateSCHSymbol(wxXmlNode *iNode, CSch *sch, wxString actualConversion) {
    wxXmlNode *lNode;
    wxString propValue, str;
    long num;
    bool t, r;
    CSchSymbol *schSymbol = new CSchSymbol();
    schSymbol->m_objType = 'S';

    if (FindNode(iNode->GetChildren(), wxT("partNum"))) {
        FindNode(iNode->GetChildren(), wxT("partNum"))->GetNodeContent().ToLong(&num);
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
                t = true;
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

// LinesIntersect procedure is copied from website
// http://www.pdas.com/lineint.htm
// extended format replaced by integer
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

CSchJunction *CJunction(int x, int y, wxString net) {
    CSchJunction *result = new CSchJunction(); // middle of line intersection
    result->m_objType = 'J';
    result->m_positionX = x;
    result->m_positionY = y;
    result->m_net = net;

    return result;
}

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

void ProcessXMLtoSch(CSch *sch, wxStatusBar* statusBar, wxString XMLFileName, wxString actualConversion) {
    wxXmlDocument xmlDoc;
    wxXmlNode *aNode;
    CSchComponent *schComp;
    CSchJunction *schJunction;
    bool isJunction;
    int i;

    if (!xmlDoc.Load(XMLFileName)) return;

    // Defaut measurement units
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("asciiHeader"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("fileUnits"));
        if (aNode) {
            sch->m_defaultMeasurementUnit = aNode->GetNodeContent();
            sch->m_defaultMeasurementUnit.Trim(true);
            sch->m_defaultMeasurementUnit.Trim(false);
        }
    }

    // Allways also library file
    isJunction = false;
    actualConversion = wxT("SCHLIB");

    DoLibrary(&xmlDoc, sch, statusBar, actualConversion);

    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("schematicDesign"));
    if (aNode) {
        //  SCHEMATIC FILE
        // aNode is schematicDesign node actually
        actualConversion = wxT("SCH");
        aNode = FindNode(aNode->GetChildren(), wxT("sheet"))->GetChildren();
        while (aNode) {
            if (aNode->GetName() == wxT("symbol"))
                sch->m_schComponents.Add(CreateSCHSymbol(aNode, sch, actualConversion));

            if (aNode->GetName() == wxT("wire"))
                if (FindNode(aNode->GetChildren(), wxT("line")))
                    sch->m_schComponents.Add(CreateLine(FindNode(aNode->GetChildren(), wxT("line")), 0, sch, actualConversion));

            if (aNode->GetName() == wxT("bus"))
                sch->m_schComponents.Add(CreateBus(aNode, sch, actualConversion));

            if (aNode->GetName() == wxT("junction"))
                sch->m_schComponents.Add(CreateJunction(aNode, sch, &isJunction, actualConversion));

            aNode = aNode->GetNext();
        }

        // POSTPROCESS -- SET/OPTIMIZE NEW SCH POSITION

        sch->m_sizeX = 1000000; sch->m_sizeY = 0;
        for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
            schComp = sch->m_schComponents[i];
            if (schComp->m_positionY < sch->m_sizeY)
                sch->m_sizeY = schComp->m_positionY;
            if (schComp->m_positionX < sch->m_sizeX && schComp->m_positionX > 0)
                sch->m_sizeX = schComp->m_positionX;
        }
        // correction
        sch->m_sizeY = sch->m_sizeY - 1000;
        sch->m_sizeX = sch->m_sizeX - 1000;
        for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
            schComp = sch->m_schComponents[i];
            schComp->m_positionY -= sch->m_sizeY;
            schComp->m_positionX -= sch->m_sizeX;
            if (schComp->m_objType == 'L') {
                ((CSchLine *)schComp)->m_toY -= sch->m_sizeY;
                ((CSchLine *)schComp)->m_labelText.textPositionY -= sch->m_sizeY;
                ((CSchLine *)schComp)->m_toX -= sch->m_sizeX;
                ((CSchLine *)schComp)->m_labelText.textPositionX -= sch->m_sizeX;
            }
        }
        // final sheet settings
        for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
            schComp = sch->m_schComponents[i];
            if (schComp->m_positionY < sch->m_sizeY)
                sch->m_sizeY = schComp->m_positionY;
            if (schComp->m_positionX > sch->m_sizeX)
                sch->m_sizeX = schComp->m_positionX;
        }
        sch->m_sizeY = -sch->m_sizeY; // is in absolute units
        sch->m_sizeX = sch->m_sizeX + 1000;
        sch->m_sizeY = sch->m_sizeY + 1000;
        // A4 is minimum $Descr A4 11700 8267
        if (sch->m_sizeX < 11700) sch->m_sizeX = 11700;
        if (sch->m_sizeY < 8267) sch->m_sizeY = 8267;

        // POSTPROCESS -- CREATE JUNCTIONS FROM NEWTLIST
        if (!isJunction) {
            if (wxMessageBox(wxT("There are not JUNCTIONS in your schematics file ."
                                 " It can be, that your design is without Junctions."
                                 " But it can be that your input file is in obsolete format."
                                 " Would you like to run postprocess and create junctions from Netlist/Wires information ?"
                                 "    YOU HAVE TO CHECK/CORRECT Juntions in converted design, placement is only approximation !"),
                             wxEmptyString, wxYES_NO) == wxYES)
            {
                for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
                    schComp = sch->m_schComponents[i];
                    if (schComp->m_objType == 'L') {
                        schJunction = CheckJunction((CSchLine *)schComp, i, sch);
                        if (schJunction)
                            sch->m_schComponents.Add(schJunction);
                    }
                }
            }
        }

    } //  SCHEMATIC FILE
}
