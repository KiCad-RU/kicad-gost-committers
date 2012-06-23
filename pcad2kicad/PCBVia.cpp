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
 * @file PCBVia.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <PCBVia.h>
#include <PCBViaShape.h>


CPCBVia::CPCBVia(CPCBCallbacks *aCallbacks) : CPCBPad(aCallbacks, wxEmptyString) {
    m_objType = 'V';
}

CPCBVia::~CPCBVia() {
}

void CPCBVia::Parse(wxXmlNode *aNode, wxString aDefaultMeasurementUnit, wxString aActualConversion) {
    wxXmlNode *lNode, *tNode;
    wxString propValue;
    CPCBViaShape *viaShape;

    m_rotation = 0;
    lNode = FindNode(aNode->GetChildren(), wxT("viaStyleRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        m_name.text = propValue;
    }

    lNode = FindNode(aNode->GetChildren(), wxT("pt"));
    if (lNode) SetPosition(lNode->GetNodeContent(), aDefaultMeasurementUnit,
                           &m_positionX, &m_positionY, aActualConversion);

    lNode = FindNode(aNode->GetChildren(), wxT("netNameRef"));
    if (lNode) {
        lNode->GetPropVal(wxT("Name"), &propValue);
        propValue.Trim(false);
        propValue.Trim(true);
        m_net = propValue;
    }

    lNode = aNode;
    while (lNode->GetName() != wxT("www.lura.sk"))
        lNode = lNode->GetParent();

    lNode = FindNode(lNode->GetChildren(), wxT("library"));
    lNode = FindNode(lNode->GetChildren(), wxT("viaStyleDef"));
    if (lNode) {
        while (lNode) {
            lNode->GetPropVal(wxT("Name"), &propValue);
            if (propValue == m_name.text) break;
            lNode = lNode->GetNext();
        }
    }

    if (lNode) {
        tNode = lNode;
        lNode = FindNode(tNode->GetChildren(), wxT("holeDiam"));
        if (lNode) SetWidth(lNode->GetNodeContent(), aDefaultMeasurementUnit, &m_hole, aActualConversion);

        lNode = FindNode(tNode->GetChildren(), wxT("viaShape"));
        while (lNode) {
            if  (lNode->GetName() == wxT("viaShape")) {
                // we support only Vias on specific layers......
                // we do not support vias on "Plane", "NonSignal" , "Signal" ... layerr
                if (FindNode(lNode->GetChildren(), wxT("layerNumRef"))) {
                    viaShape = new CPCBViaShape(m_callbacks);
                    viaShape->Parse(lNode, aDefaultMeasurementUnit, aActualConversion);
                    m_shapes.Add(viaShape);
                }
            }

            lNode = lNode->GetNext();
        }
    }
}
