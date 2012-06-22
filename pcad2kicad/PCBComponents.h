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
 * @file PCBComponents.h
 */

#ifndef PCBCOMPONENTS_H_
#define PCBCOMPONENTS_H_

#include <wx/wx.h>

#include <XMLtoObjectCommonProceduresUnit.h>
#include <PCBPadShape.h>
#include <PCBPad.h>
#include <PCBPolygon.h>
#include <PCBLayersMap.h>
#include <PCBModule.h>


class CNetNode : public wxObject
{
public:
    wxString m_compRef;
    wxString m_pinRef;

    CNetNode();
    ~CNetNode();
};

WX_DEFINE_ARRAY(CNetNode *, CNetNodesArray);

class CNet : public wxObject
{
public:
    wxString m_name;
    CNetNodesArray m_netNodes;

    CNet(wxString iName);
    ~CNet();
};

WX_DEFINE_ARRAY(CNet *, CNetsArray);

class CPCB : /*public wxObject*/ public CPCBModule, public CPCBLayersMap
{
public:
    CPCBComponentsArray m_pcbComponents;  // CPCB Modules,Lines,Routes,Texts, .... and so on
    CNetsArray m_pcbNetlist;  // net objects collection
    wxString m_defaultMeasurementUnit;
    int m_layersMap[28]; // flexible layers mapping
    int m_sizeX;
    int m_sizeY;

    CPCB();
    ~CPCB();

    int GetKiCadLayer(int aPCadLayer);
    int GetNewTimestamp();

    void Parse(CPCB *pcb, wxStatusBar* statusBar, wxString XMLFileName, wxString actualConversion);

    virtual void WriteToFile(wxString fileName, char ftype);

private:
    int m_timestamp_cnt;
    void DoPCBComponents(wxXmlNode *iNode, CPCB *pcb, wxXmlDocument *xmlDoc, wxString actualConversion, wxStatusBar* statusBar);
};

#endif // PCBCOMPONENTS_H_
