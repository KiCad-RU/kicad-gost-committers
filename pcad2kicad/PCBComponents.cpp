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

#include <PCBArc.h>
#include <PCBComponents.h>
#include <PCBLine.h>
#include <PCBText.h>
#include <PCBVia.h>


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

CNetNode::CNetNode() {
    m_compRef = wxEmptyString;
    m_pinRef = wxEmptyString;
}

CNetNode::~CNetNode() {
}

CNet::CNet(wxString iName) {
    m_name = iName;
}

CNet::~CNet() {
    int i;

    for (i = 0; i < (int)m_netNodes.GetCount(); i++) {
        delete m_netNodes[i];
    }
}

CPCBModule::CPCBModule(CPCBLayersMap *aLayersMap, wxString iName) : CPCBComponent(aLayersMap) {
    InitTTextValue(&m_value);
    m_mirror = 0;
    m_objType = 'M';  // MODULE
    m_name.text = iName;
    m_KiCadLayer = 21; // default
}

wxString ModuleLayer(int mirror) {
    wxString result;

 /////NOT !   {IntToStr(KiCadLayer)}    NOT !
 ///  MODULES ARE HARD PLACED ON COMPONENT OR COPPER LAYER.
 ///  IsFLIPPED--> MIRROR attribute is decision Point!!!

    if (mirror == 0) result = wxT("15"); //Components side
    else result = wxT("0"); // Copper side

    return result;
}

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

CPCBModule::~CPCBModule() {
    int i;

    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        delete m_moduleObjects[i];
    }
}

int CPCB::GetKiCadLayer(int aPCadLayer) {
    assert (aPCadLayer >= 0 && aPCadLayer <= 28);
    return m_layersMap[aPCadLayer];
}

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

CPCB::~CPCB() {
    int i;

    for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
        delete m_pcbComponents[i];
    }

    for (i = 0; i < (int)m_pcbNetlist.GetCount(); i++) {
        delete m_pcbNetlist[i];
    }
}

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

int CPCB::GetNewTimestamp() {
    return m_timestamp_cnt++;
}
