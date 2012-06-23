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
 * @file pcb_line.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <pcb_line.h>


PCB_LINE::PCB_LINE(PCB_CALLBACKS *aCallbacks) : PCB_COMPONENT(aCallbacks) {
    m_width = 0;
    m_toX = 0;
    m_toY = 0;
    m_objType = 'L';
}

PCB_LINE::~PCB_LINE() {
}

void PCB_LINE::Parse(wxXmlNode *aNode, int aLayer, wxString aDefaultMeasurementUnit, wxString aActualConversion) {
    wxXmlNode *lNode;
    wxString propValue;

    m_PCadLayer = aLayer;
    m_KiCadLayer = GetKiCadLayer();
    m_positionX = 0;
    m_positionY = 0;
    m_toX = 0;
    m_toY = 0;
    m_width = 0;
    lNode = FindNode(aNode->GetChildren(), wxT("pt"));
    if (lNode)
        SetPosition(lNode->GetNodeContent(), aDefaultMeasurementUnit,
                    &m_positionX, &m_positionY, aActualConversion);

    lNode = lNode->GetNext();
    if (lNode)
        SetPosition(lNode->GetNodeContent(), aDefaultMeasurementUnit,
                    &m_toX, &m_toY, aActualConversion);

    lNode = FindNode(aNode->GetChildren(), wxT("width"));
    if (lNode) SetWidth(lNode->GetNodeContent(), aDefaultMeasurementUnit, &m_width, aActualConversion);

    lNode = FindNode(aNode->GetChildren(), wxT("netNameRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        m_net = propValue;
    }
}

void PCB_LINE::SetPosOffset(int x_offs, int y_offs) {
    PCB_COMPONENT::SetPosOffset(x_offs, y_offs);
    m_toX += x_offs;
    m_toY += y_offs;
}

void PCB_LINE::WriteToFile(wxFile *aFile, char aFileType) {
    if (aFileType == 'L') { // Library
        aFile->Write(wxString::Format("DS %d %d %d %d %d %d\n", m_positionX, m_positionY,
                 m_toX, m_toY, m_width, m_KiCadLayer)); // Position
    }

    if (aFileType == 'P') { // PCB
        aFile->Write(wxString::Format("Po 0 %d %d %d %d %d\n", m_positionX, m_positionY,
                 m_toX, m_toY, m_width));
        if (m_timestamp == 0)
            aFile->Write(wxString::Format("De %d 0 0 0 0\n", m_KiCadLayer));
        else aFile->Write(wxString::Format("De %d 0 0 %8X 0\n", m_KiCadLayer, m_timestamp));
    }
}
