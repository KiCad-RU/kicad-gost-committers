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

#include <PCBArc.h>
#include <PCBCopperPour.h>
#include <PCBCutout.h>
#include <PCBLine.h>
#include <PCBPadShape.h>
#include <PCBViaShape.h>
#include <PCBPad.h>
#include <PCBText.h>
#include <PCBVia.h>


void DoLayerContentsObjects(wxXmlNode *iNode, CPCBModule *pcbModule, CPCBComponentsArray *list, wxStatusBar* statusBar, CPCB *pcb, wxString actualConversion) {
    CPCBArc *arc;
    CPCBPolygon *polygon;
    CPCBCopperPour *copperPour;
    CPCBCutout *cutout;
    CPCBLine *line;
    CPCBText *text;
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
        if (lNode->GetName() == wxT("line")) {
            line = new CPCBLine(pcb);
            line->Parse(lNode, PCadLayer, pcb->m_defaultMeasurementUnit, actualConversion);
            list->Add(line);
        }

        if (lNode->GetName() == wxT("text")) {
            text = new CPCBText(pcb);
            text->Parse(lNode, PCadLayer, pcb->m_defaultMeasurementUnit, actualConversion);
            list->Add(text);
        }

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
        if (lNode->GetName() == wxT("arc") || lNode->GetName() == wxT("triplePointArc")) {
            arc = new CPCBArc(pcb);
            arc->Parse(lNode, PCadLayer, pcb->m_defaultMeasurementUnit, actualConversion);
            list->Add(arc);
        }

        if (lNode->GetName() == wxT("pcbPoly")) {
            polygon = new CPCBPolygon(pcb);
            polygon->Parse(lNode, PCadLayer, pcb->m_defaultMeasurementUnit, actualConversion, statusBar);
            list->Add(polygon);
        }

        if (lNode->GetName() == wxT("copperPour95")) {
            copperPour = new CPCBCopperPour(pcb);
            if (copperPour->Parse(lNode, PCadLayer, pcb->m_defaultMeasurementUnit, actualConversion, statusBar))
                list->Add(copperPour);
            else delete copperPour;
        }

        if (lNode->GetName() == wxT("polyCutOut")) {
            // It seems that the same cutouts (with the same vertices) are inside of copper pour objects

            // list of polygons....
            tNode = lNode;
            tNode = FindNode(tNode->GetChildren(), wxT("pcbPoly"));
            if (tNode) {
                cutout = new CPCBCutout(pcb);
                cutout->Parse(tNode, PCadLayer, pcb->m_defaultMeasurementUnit, actualConversion);
                list->Add(cutout);
            }
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
    CPCBPad *pad;
    CPCBVia *via;
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
            if (tNode->GetName() == wxT("pad")) {
                pad = new CPCBPad(pcb, wxEmptyString);
                pad->Parse(tNode, pcb->m_defaultMeasurementUnit, actualConversion);
                pcbModule->m_moduleObjects.Add(pad);
            }

            if (tNode->GetName() == wxT("via")) {
                via = new CPCBVia(pcb);
                via->Parse(tNode, pcb->m_defaultMeasurementUnit, actualConversion);
                pcbModule->m_moduleObjects.Add(via);
            }

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
    CPCBVia *via;
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
        else if (lNode->GetName() == wxT("via")) {
            via = new CPCBVia(pcb);
            via->Parse(lNode, pcb->m_defaultMeasurementUnit, actualConversion);
            pcb->m_pcbComponents.Add(via);
        }

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
