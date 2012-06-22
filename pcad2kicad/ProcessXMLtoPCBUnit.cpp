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
#include <PCBModule.h>
#include <PCBPadShape.h>
#include <PCBViaShape.h>
#include <PCBPad.h>
#include <PCBText.h>
#include <PCBVia.h>


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

void CPCB::DoPCBComponents(wxXmlNode *iNode, CPCB *pcb, wxXmlDocument *xmlDoc, wxString actualConversion, wxStatusBar* statusBar) {
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
                if (tNode) {
                    mc = new CPCBModule(this);
                    mc->Parse(tNode, statusBar, pcb->m_defaultMeasurementUnit, actualConversion);
                }
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
                                mc->SetPadName(str, propValue);
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

void CPCB::Parse(CPCB *pcb, wxStatusBar* statusBar, wxString XMLFileName, wxString actualConversion) {
    wxXmlDocument xmlDoc;
    wxXmlNode *aNode;
    CNet *net;
    CPCBComponent *comp;
    CPCBModule *module;
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
                DoLayerContentsObjects(aNode, NULL, &pcb->m_pcbComponents, statusBar, this, m_defaultMeasurementUnit, actualConversion);
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
                if (aNode->GetName() == wxT("compDef")) {
                    module = new CPCBModule(this);
                    module->Parse(aNode, statusBar, m_defaultMeasurementUnit, actualConversion);
                    pcb->m_pcbComponents.Add(module);
                }

                aNode = aNode->GetNext();
            }
        }
    }
}
