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
 * @file SchArc.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/filename.h>

#include <common.h>

#include <SchArc.h>

CSchArc::CSchArc(wxXmlNode *aNode, int aSymbolIndex,
    wxString aDefaultMeasurementUnit, wxString aActualConversion)
{
    wxXmlNode *lNode;
    wxString propValue;
    double r;

    m_startX = 0;
    m_startY = 0;
    m_toX = 0;
    m_toY = 0;
    m_startAngle = 0;
    m_sweepAngle = 0;
    m_radius = 0;

    m_objType = 'A';
    m_partNum = aSymbolIndex;

    if (FindNode(aNode->GetChildren(), wxT("width")))
        m_width = StrToIntUnits(FindNode(aNode->GetChildren(), wxT("width"))->GetNodeContent(),
            ' ', aActualConversion);
    else m_width = 1; //default

    if (aNode->GetName() == wxT("triplePointArc")) {
        // origin
        lNode = FindNode(aNode->GetChildren(), wxT("pt"));
        if (lNode)
            SetPosition(lNode->GetNodeContent(), aDefaultMeasurementUnit,
                    &m_positionX, &m_positionY, aActualConversion);
        // First - starting point in PCAS is ENDING point in KiCAd
        lNode = lNode->GetNext();
        if (lNode)
            SetPosition(lNode->GetNodeContent(), aDefaultMeasurementUnit,
                    &m_toX, &m_toY, aActualConversion);
        // Second - ending point in PCAS is STARTING point in KiCAd
        lNode = lNode->GetNext();
        if (lNode)
            SetPosition(lNode->GetNodeContent(), aDefaultMeasurementUnit,
                    &m_startX, &m_startY, aActualConversion);
        // now temporary, it can be fixed later.....
        //SCHArc.StartAngle:=0;
        //SCHArc.SweepAngle:=3600;
    }

    if (aNode->GetName() == wxT("arc")) {
        lNode = FindNode(aNode->GetChildren(), wxT("pt"));
        if (lNode)
            SetPosition(lNode->GetNodeContent(), aDefaultMeasurementUnit,
                    &m_positionX, &m_positionY, aActualConversion);

        lNode = FindNode(aNode->GetChildren(), wxT("radius"));
        if (lNode)
            m_radius = StrToIntUnits(lNode->GetNodeContent(), ' ', aActualConversion);
        r = StrToIntUnits(lNode->GetNodeContent(), ' ', aActualConversion);
        m_startAngle = StrToInt1Units(
            FindNode(aNode->GetChildren(), wxT("startAngle"))->GetNodeContent());
        m_startX = KiROUND(m_positionX + r * sin((m_startAngle - 900.0) * M_PI / 1800.0));
        m_startY = KiROUND(m_positionY - r * cos((m_startAngle - 900) * M_PI / 1800.0));
        m_sweepAngle = StrToInt1Units(
            FindNode(aNode->GetChildren(), wxT("sweepAngle"))->GetNodeContent());
        m_toX = KiROUND(m_positionX +
            r * sin((m_startAngle + m_sweepAngle - 900.0) * M_PI / 1800.0));
        m_toY = KiROUND(m_positionY -
            r * cos((m_startAngle + m_sweepAngle - 900.0) * M_PI / 1800.0));
    }
}

CSchArc::~CSchArc() {
}

void CSchArc::WriteToFile(wxFile *aFile, char aFileType) {
    aFile->Write(wxString::Format("A %d %d %d %d %d %d 0 %d N %d %d %d %d\n",
        m_positionX, m_positionY, m_radius, m_startAngle, m_sweepAngle,
        m_partNum, m_width, m_startX, m_startY, m_toX, m_toY));
}
