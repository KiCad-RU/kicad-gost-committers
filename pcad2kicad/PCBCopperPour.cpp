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
 * @file PCBCopperPour.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <PCBCopperPour.h>

CPCBCopperPour::CPCBCopperPour(CPCBLayersMap *aLayersMap) : CPCBPolygon(aLayersMap) {
}

CPCBCopperPour::~CPCBCopperPour() {
}

bool CPCBCopperPour::Parse(wxXmlNode *aNode, int aPCadLayer,
    wxString aDefaultMeasurementUnit, wxString aActualConversion, wxStatusBar* aStatusBar)
{
    wxXmlNode *lNode, *tNode, *cNode;
    wxString pourType, str, propValue;
    int pourSpacing, thermalWidth;
    CVerticesArray *island, *cutout;

    aStatusBar->SetStatusText(aStatusBar->GetStatusText() + wxT(" CooperPour..."));
    m_PCadLayer = aPCadLayer;
    m_KiCadLayer = GetKiCadLayer();
    m_timestamp = GetNewTimestamp();

    str = FindNode(aNode->GetChildren(), wxT("pourType"))->GetNodeContent();
    str.Trim(false);
    pourType = str.MakeUpper();

    lNode = FindNode(aNode->GetChildren(), wxT("netNameRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        m_net = propValue;
    }

    SetWidth(FindNode(aNode->GetChildren(), wxT("width"))->GetNodeContent(),
        aDefaultMeasurementUnit, &m_width, aActualConversion);
    if (FindNode(aNode->GetChildren(), wxT("pourSpacing")))
        SetWidth(FindNode(aNode->GetChildren(), wxT("pourSpacing"))->GetNodeContent(),
            aDefaultMeasurementUnit, &pourSpacing, aActualConversion);
    if (FindNode(aNode->GetChildren(), wxT("thermalWidth")))
        SetWidth(FindNode(aNode->GetChildren(), wxT("thermalWidth"))->GetNodeContent(),
            aDefaultMeasurementUnit, &thermalWidth, aActualConversion);

    lNode = FindNode(aNode->GetChildren(), wxT("pcbPoly"));
    if (lNode) {
        // retrieve copper pour outline
        FormPolygon(lNode, &m_outline, aDefaultMeasurementUnit, aActualConversion);

        m_positionX = m_outline[0]->x;
        m_positionY = m_outline[0]->y;

        lNode = FindNode(aNode->GetChildren(), wxT("island"));
        while (lNode) {
            tNode = FindNode(lNode->GetChildren(), wxT("islandOutline"));
            if (tNode) {
                island = new CVerticesArray;
                FormPolygon(tNode, island, aDefaultMeasurementUnit, aActualConversion);
                m_islands.Add(island);
                tNode = FindNode(lNode->GetChildren(), wxT("cutout"));
                while (tNode) {
                    cNode = FindNode(tNode->GetChildren(), wxT("cutoutOutline"));
                    if (cNode) {
                        cutout = new CVerticesArray;
                        FormPolygon(cNode, cutout, aDefaultMeasurementUnit, aActualConversion);
                        m_cutouts.Add(cutout);
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
    else {
        return false;
    }

    return true;
}

