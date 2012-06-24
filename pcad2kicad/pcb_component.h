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
 * @file pcb_component.h
 */

#ifndef PCB_COMPONENT_H_
#define PCB_COMPONENT_H_

#include <wx/wx.h>

#include <XMLtoObjectCommonProceduresUnit.h>
#include <pcb_callbacks.h>


// basic parent class for PCB objects
class PCB_COMPONENT : public wxObject
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
    TTEXTVALUE m_name;   // name has also privete positions, rotations nand so on....
    wxString m_net;
    wxString m_compRef;  // internal ussage for XL parsing
    wxString m_patGraphRefName;  // internal ussage for XL parsing

    PCB_COMPONENT(PCB_CALLBACKS *aCallbacks);
    ~PCB_COMPONENT();

    virtual void WriteToFile(wxFile *aFile, char aFileType);
    virtual void SetPosOffset(int aX_offs, int aY_offs);

    int GetKiCadLayer() { return m_callbacks->GetKiCadLayer(m_PCadLayer); }
    int GetNewTimestamp() { return m_callbacks->GetNewTimestamp(); }

protected:
    PCB_CALLBACKS *m_callbacks;
};

WX_DEFINE_ARRAY(PCB_COMPONENT *, PCB_COMPONENTS_ARRAY);

#endif // PCB_COMPONENT_H_
