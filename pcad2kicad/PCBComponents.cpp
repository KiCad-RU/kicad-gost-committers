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

int CPCB::GetKiCadLayer(int aPCadLayer) {
    assert (aPCadLayer >= 0 && aPCadLayer <= 28);
    return m_layersMap[aPCadLayer];
}

CPCB::CPCB() : CPCBModule(NULL) {
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
