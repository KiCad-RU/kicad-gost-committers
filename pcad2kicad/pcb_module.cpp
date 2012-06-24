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
 * @file pcb_module.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <pcb_arc.h>
#include <pcb_copper_pour.h>
#include <pcb_cutout.h>
#include <pcb_line.h>
#include <pcb_module.h>
#include <pcb_pad.h>
#include <pcb_polygon.h>
#include <pcb_text.h>
#include <pcb_via.h>


PCB_MODULE::PCB_MODULE(PCB_CALLBACKS *aCallbacks) : PCB_COMPONENT(aCallbacks) {
    InitTTextValue(&m_value);
    m_mirror = 0;
    m_objType = 'M';  // MODULE
    m_KiCadLayer = 21; // default
}

PCB_MODULE::~PCB_MODULE() {
    int i;

    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        delete m_moduleObjects[i];
    }
}

wxXmlNode * PCB_MODULE::FindModulePatternDefName(wxXmlNode *aNode, wxString aName) {
    wxXmlNode *result, *lNode;
    wxString propValue1, propValue2;

    result = NULL;
    lNode = FindNode(aNode->GetChildren(), wxT("patternDef"));
    while (lNode) {
        if (lNode->GetName() == wxT("patternDef")) {
            lNode->GetPropVal(wxT("Name"), &propValue1);
            FindNode(lNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue2);
            if (ValidateName(propValue1) == aName ||
                ValidateName(propValue2) == aName) {
                result = lNode;
                lNode = NULL;
            }
        }

        if (lNode) lNode = lNode->GetNext();
    }

    if (result == NULL) {
        lNode = FindNode(aNode->GetChildren(), wxT("patternDefExtended")); // New file format
        while (lNode) {
            if (lNode->GetName() == wxT("patternDefExtended")) {
                lNode->GetPropVal(wxT("Name"), &propValue1);
                if (ValidateName(propValue1) == aName) {
                    result = lNode;
                    lNode = NULL;
                }
            }

            if (lNode) lNode = lNode->GetNext();
        }
    }

    return result;
}

wxXmlNode * PCB_MODULE::FindPatternMultilayerSection(wxXmlNode *aNode, wxString *aPatGraphRefName) {
    wxXmlNode *result, *pNode, *lNode;
    wxString propValue, patName;

    result = NULL;
    pNode = aNode; //pattern;
    lNode = aNode;
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

    *aPatGraphRefName = wxEmptyString;  //default
    if (lNode) result = lNode;
    else {
        // New file format
        if (FindNode(aNode->GetChildren(), wxT("patternGraphicsNameRef"))) {
            FindNode(aNode->GetChildren(), wxT("patternGraphicsNameRef"))->GetPropVal(wxT("Name"), aPatGraphRefName);
        }
///////////////////////////////////////////////////////////////////////
//        lNode:=iNode.ChildNodes.FindNode('patternGraphicsDef');  before
//        Fixed 02/08, Sergeys input file format
//        Did it work before  ????
//        lNode:=pNode.ChildNodes.FindNode('patternGraphicsDef');  Nw for some files
//////////////////////////////////////////////////////////////////////
        if (FindNode(aNode->GetChildren(), wxT("patternGraphicsDef")))
            lNode = FindNode(aNode->GetChildren(), wxT("patternGraphicsDef"));
        else
            lNode = FindNode(pNode->GetChildren(), wxT("patternGraphicsDef"));

        if (*aPatGraphRefName == wxEmptyString) {  // no patern delection, the first is actual...
            if (lNode) {
                result = FindNode(lNode->GetChildren(), wxT("multiLayer"));
                lNode = NULL;
            }
        }

        while (lNode) {   // selected by name
            if (lNode->GetName() == wxT("patternGraphicsDef")) {
                FindNode(lNode->GetChildren(), wxT("patternGraphicsNameDef"))->GetPropVal(wxT("Name"), &propValue);
                if (propValue == *aPatGraphRefName) {
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

void PCB_MODULE::DoLayerContentsObjects(wxXmlNode *aNode, PCB_MODULE *aPCBModule,
    PCB_COMPONENTS_ARRAY *aList, wxStatusBar* aStatusBar,
    wxString aDefaultMeasurementUnit, wxString aActualConversion)
{
    PCB_ARC *arc;
    PCB_POLYGON *polygon;
    PCB_COPPER_POUR *copperPour;
    PCB_CUTOUT *cutout;
    PCB_LINE *line;
    PCB_TEXT *text;
    wxXmlNode *lNode, *tNode;
    wxString propValue;
    long long i;
    int PCadLayer;
    long num;

    i = 0;
    aStatusBar->SetStatusText(wxT("Processing LAYER CONTENT OBJECTS "));
    FindNode(aNode->GetChildren(), wxT("layerNumRef"))->GetNodeContent().ToLong(&num);
    PCadLayer = (int)num;
    lNode = aNode->GetChildren();
    while (lNode) {
        i++;
        aStatusBar->SetStatusText(wxString::Format("Processing LAYER CONTENT OBJECTS :%lld", i));
        if (lNode->GetName() == wxT("line")) {
            line = new PCB_LINE(m_callbacks);
            line->Parse(lNode, PCadLayer, aDefaultMeasurementUnit, aActualConversion);
            aList->Add(line);
        }

        if (lNode->GetName() == wxT("text")) {
            text = new PCB_TEXT(m_callbacks);
            text->Parse(lNode, PCadLayer, aDefaultMeasurementUnit, aActualConversion);
            aList->Add(text);
        }

        // added  as Sergeys request 02/2008
        if (lNode->GetName() == wxT("attr")) {
            // assign fonts to Module Name,Value,Type,....s
            lNode->GetPropVal(wxT("Name"), &propValue);
            propValue.Trim(false);
            propValue.Trim(true);
            if (propValue == wxT("Type")) {
                tNode = FindNode(lNode->GetChildren(), wxT("textStyleRef"));
                if (tNode && aPCBModule) {
                    //TODO: to understand and may be repair
                    // Alexander Lunev: originally in Delphi version of the project there was a strange access
                    //  to pcbModule->m_name (it was global variable). This access is necessary
                    //  when the function DoLayerContentsObjects() is called from function CreatePCBModule().
                    //  However it is not clear whether the access is required when the function DoLayerContentsObjects()
                    //  is called from function ProcessXMLtoPCBLib().
                    SetFontProperty(tNode, &aPCBModule->m_name, aDefaultMeasurementUnit, aActualConversion);
                }
            }
        }

        // added  as Sergeys request 02/2008
        if (lNode->GetName() == wxT("arc") || lNode->GetName() == wxT("triplePointArc")) {
            arc = new PCB_ARC(m_callbacks);
            arc->Parse(lNode, PCadLayer, aDefaultMeasurementUnit, aActualConversion);
            aList->Add(arc);
        }

        if (lNode->GetName() == wxT("pcbPoly")) {
            polygon = new PCB_POLYGON(m_callbacks);
            polygon->Parse(lNode, PCadLayer, aDefaultMeasurementUnit, aActualConversion, aStatusBar);
            aList->Add(polygon);
        }

        if (lNode->GetName() == wxT("copperPour95")) {
            copperPour = new PCB_COPPER_POUR(m_callbacks);
            if (copperPour->Parse(lNode, PCadLayer, aDefaultMeasurementUnit, aActualConversion, aStatusBar))
                aList->Add(copperPour);
            else delete copperPour;
        }

        if (lNode->GetName() == wxT("polyCutOut")) {
            // It seems that the same cutouts (with the same vertices) are inside of copper pour objects

            // list of polygons....
            tNode = lNode;
            tNode = FindNode(tNode->GetChildren(), wxT("pcbPoly"));
            if (tNode) {
                cutout = new PCB_CUTOUT(m_callbacks);
                cutout->Parse(tNode, PCadLayer, aDefaultMeasurementUnit, aActualConversion);
                aList->Add(cutout);
            }
        }

        lNode = lNode->GetNext();
    }
}

void PCB_MODULE::SetPadName(wxString aPin, wxString aName) {
    int i;
    long num;

    aPin.ToLong(&num);
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'P')
            if (((PCB_PAD *)m_moduleObjects[i])->m_number == num)
                ((PCB_PAD *)m_moduleObjects[i])->m_name.text = aName;
    }
}

void PCB_MODULE::Parse(wxXmlNode *aNode, wxStatusBar* aStatusBar,
    wxString aDefaultMeasurementUnit, wxString aActualConversion)
{
    wxXmlNode *lNode, *tNode, *mNode;
    PCB_PAD *pad;
    PCB_VIA *via;
    wxString propValue, str;

    FindNode(aNode->GetChildren(), wxT("originalName"))->GetPropVal(wxT("Name"), &propValue);
    propValue.Trim(false);
    m_name.text = propValue;

    aStatusBar->SetStatusText(wxT("Creating Component : ") + m_name.text);
    lNode = aNode;
    lNode = FindPatternMultilayerSection(lNode, &m_patGraphRefName);
    if (lNode) {
        tNode = lNode;
        tNode = tNode->GetChildren();
        while (tNode) {
            if (tNode->GetName() == wxT("pad")) {
                pad = new PCB_PAD(m_callbacks, wxEmptyString);
                pad->Parse(tNode, aDefaultMeasurementUnit, aActualConversion);
                m_moduleObjects.Add(pad);
            }

            if (tNode->GetName() == wxT("via")) {
                via = new PCB_VIA(m_callbacks);
                via->Parse(tNode, aDefaultMeasurementUnit, aActualConversion);
                m_moduleObjects.Add(via);
            }

            tNode = tNode->GetNext();
        }
    }

    lNode = lNode->GetParent();
    lNode = FindNode(lNode->GetChildren(), wxT("layerContents"));
    while (lNode) {
        if (lNode->GetName() == wxT("layerContents"))
            DoLayerContentsObjects(lNode, this, &m_moduleObjects, aStatusBar,
                aDefaultMeasurementUnit, aActualConversion);
        lNode = lNode->GetNext();
    }

    // map pins
    lNode = FindPinMap(aNode);

    if (lNode) {
        mNode = lNode->GetChildren();
        while (mNode) {
            if (mNode->GetName() == wxT("padNum")) {
                str = mNode->GetNodeContent();
                mNode = mNode->GetNext();
                if (!mNode) break;
                mNode->GetPropVal(wxT("Name"), &propValue);
                SetPadName(str, propValue);
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

wxString PCB_MODULE::ModuleLayer(int aMirror) {
    wxString result;

 /////NOT !   {IntToStr(KiCadLayer)}    NOT !
 ///  MODULES ARE HARD PLACED ON COMPONENT OR COPPER LAYER.
 ///  IsFLIPPED--> MIRROR attribute is decision Point!!!

    if (aMirror == 0) result = wxT("15"); //Components side
    else result = wxT("0"); // Copper side

    return result;
}

void PCB_MODULE::WriteToFile(wxFile *aFile, char aFileType) {
    char visibility, mirrored;
    int i;

    // transform text positions ....
    CorrectTextPosition(&m_name, m_rotation);
    CorrectTextPosition(&m_value, m_rotation);
    // Go out
    aFile->Write(wxT("\n"));
    aFile->Write(wxT("$MODULE ") + m_name.text + wxT("\n"));
    aFile->Write(wxString::Format("Po %d %d %d ", m_positionX, m_positionY, m_rotation) +
            ModuleLayer(m_mirror) + wxT(" 00000000 00000000 ~~\n")); // Position
    aFile->Write(wxT("Li ") + m_name.text + wxT("\n"));   // Modulename
    aFile->Write(wxT("Sc 00000000\n")); // Timestamp
    aFile->Write(wxT("Op 0 0 0\n")); // Orientation
    aFile->Write(wxT("At SMD\n"));   // ??

    // MODULE STRINGS
    if (m_name.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_name.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    aFile->Write(wxString::Format("T0 %d %d %d %d %d %d", m_name.correctedPositionX, m_name.correctedPositionY,
               KiROUND(m_name.textHeight / 2), KiROUND(m_name.textHeight / 1.5),
               m_name.textRotation, m_value.textstrokeWidth /* TODO: Is it correct to use m_value.textstrokeWidth here? */) +
               ' ' + mirrored + ' ' + visibility + wxString::Format(" %d \"", m_KiCadLayer) + m_name.text + wxT("\"\n")); // NameString

    if (m_value.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_value.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    aFile->Write(wxString::Format("T1 %d %d %d %d %d %d", m_value.correctedPositionX, m_value.correctedPositionY,
               KiROUND(m_value.textHeight / 2), KiROUND(m_value.textHeight / 1.5),
               m_value.textRotation, m_value.textstrokeWidth) +
               ' ' + mirrored + ' ' + visibility + wxString::Format(" %d \"", m_KiCadLayer) + m_value.text + wxT("\"\n")); // ValueString

    // TEXTS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'T') {
            ((PCB_TEXT *)m_moduleObjects[i])->m_tag = i + 2;
            m_moduleObjects[i]->WriteToFile(aFile, aFileType);
        }
    }

    // MODULE LINES
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'L')
            m_moduleObjects[i]->WriteToFile(aFile, aFileType);
    }

    // MODULE Arcs
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'A')
            m_moduleObjects[i]->WriteToFile(aFile, aFileType);
    }

    // PADS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'P')
            ((PCB_PAD *)m_moduleObjects[i])->WriteToFile(aFile, aFileType, m_rotation);
    }

    // VIAS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'V')
            ((PCB_VIA *)m_moduleObjects[i])->WriteToFile(aFile, aFileType, m_rotation);
    }

    // END
    aFile->Write(wxT("$EndMODULE ") + m_name.text + wxT("\n"));
}

int PCB_MODULE::FlipLayers(int aLayer) {
    int result = aLayer; // no swap default....

    // routed layers
    if (aLayer == 0) result = 15;
    if (aLayer == 15) result = 0;

    // Silk
    if (aLayer == 21) result = 20;
    if (aLayer == 20) result = 21;

    //Paste
    if (aLayer == 19) result = 18;
    if (aLayer == 18) result = 19;

    // Mask
    if (aLayer == 23) result = 22;
    if (aLayer == 22) result = 23;

    return result;
}

void PCB_MODULE::Flip() {
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
                ((PCB_LINE *)m_moduleObjects[i])->m_toX = -((PCB_LINE *)m_moduleObjects[i])->m_toX;
                m_moduleObjects[i]->m_KiCadLayer = FlipLayers(m_moduleObjects[i]->m_KiCadLayer);
            }

            // MODULE Arcs
            if (m_moduleObjects[i]->m_objType == 'A') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                ((PCB_ARC *)m_moduleObjects[i])->m_startX = -((PCB_ARC *)m_moduleObjects[i])->m_startX;
                m_moduleObjects[i]->m_KiCadLayer = FlipLayers(m_moduleObjects[i]->m_KiCadLayer);
            }

            // PADS
            if (m_moduleObjects[i]->m_objType == 'P') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                m_moduleObjects[i]->m_rotation = -m_moduleObjects[i]->m_rotation;

                for (j = 0; j < (int)((PCB_PAD *)m_moduleObjects[i])->m_shapes.GetCount(); j++)
                    ((PCB_PAD *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer =
                        FlipLayers(((PCB_PAD *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer);
            }

            // VIAS
            if (m_moduleObjects[i]->m_objType == 'V') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;

                for (j = 0; j < (int)((PCB_VIA *)m_moduleObjects[i])->m_shapes.GetCount(); j++)
                    ((PCB_VIA *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer =
                        FlipLayers(((PCB_VIA *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer);
            }
        }
    }
}
