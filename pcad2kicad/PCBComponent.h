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
 * @file PCBComponent.h
 */

#ifndef PCBCOMPONENT_H_
#define PCBCOMPONENT_H_

#include <wx/wx.h>

#include <XMLtoObjectCommonProceduresUnit.h>
#include <PCBLayersMap.h>


// basic parent class for PCB objects
class CPCBComponent : public wxObject
{
public:
    int m_tag;
    char m_objType;
    int m_PCadLayer;
    int m_KiCadLayer;
    int m_timestamp;
    int m_positionX;
    int m_positionY;
    int m_rotation;
    TTextValue m_name;   // name has also privete positions, rotations nand so on....
    wxString m_net;
    wxString m_compRef;  // internal ussage for XL parsing
    wxString m_patGraphRefName;  // internal ussage for XL parsing

    CPCBComponent(CPCBLayersMap *aLayersMap);
    ~CPCBComponent();

    virtual void WriteToFile(wxFile *f, char ftype);
    virtual void SetPosOffset(int x_offs, int y_offs);

    int GetKiCadLayer() { return m_layersMap->GetKiCadLayer(m_PCadLayer); }

private:
    CPCBLayersMap *m_layersMap;
};

WX_DEFINE_ARRAY(CPCBComponent *, CPCBComponentsArray);

#endif // PCBCOMPONENT_H_
