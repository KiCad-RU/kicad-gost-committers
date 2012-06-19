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
 * @file SchComponents.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/filename.h>

#include <common.h>

#include <SchComponents.h>
#include <SchArc.h>
#include <SchJunction.h>
#include <SchLine.h>
#include <SchModule.h>
#include <SchPin.h>

CSchComponent::CSchComponent() {
    m_objType = '?';
    m_positionX = 0;
    m_positionY = 0;
    m_rotation = 0;
    m_mirror = 0;
    m_partNum = 0;
    m_width = 0;
    m_isVisible = 0;
}

CSchComponent::~CSchComponent() {
}

void CSchComponent::WriteToFile(wxFile *f, char ftype) {
}

CSch::CSch() {
    m_defaultMeasurementUnit = wxEmptyString;
    m_sizeX = 0;
    m_sizeY = 0;
}

CSch::~CSch() {
    int i;

    for (i = 0; i < (int)m_schComponents.GetCount(); i++)
        delete m_schComponents[i];
}

void CSch::WriteToFile(wxString fileName, char ftype) {
    wxFile f;
    int i;

    if (ftype == 'L') {
        // LIBRARY
        f.Open(fileName, wxFile::write);
        f.Write(wxT("EESchema-LIBRARY Version 2.3  Date: 01/1/2001-01:01:01\n"));
        f.Write(wxT("\n"));
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'M')
                m_schComponents[i]->WriteToFile(&f, ftype);
        }
        f.Write(wxT("# End Library\n"));
        f.Close();    // also modules descriptions

        wxFileName dcmFile(fileName);
        dcmFile.SetExt(wxT("dcm"));
        f.Open(dcmFile.GetFullPath(), wxFile::write);
        f.Write(wxT("EESchema-DOCLIB  Version 2.0  Date: 01/01/2000-01:01:01\n"));
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'M') {
                f.Write(wxT("#\n"));
                f.Write(wxT("$CMP ") + ValidateName(((CSchModule *)m_schComponents[i])->m_name.text) + wxT("\n"));
                f.Write(wxT("D ") + ((CSchModule *)m_schComponents[i])->m_moduleDescription + wxT("\n"));
                f.Write(wxT("K \n")); //no information available
                f.Write(wxT("$ENDCMP\n"));
            }
        }
        f.Write(wxT("#\n"));
        f.Write(wxT("#End Doc Library\n"));
        f.Close();
    } // LIBRARY

    if (ftype == 'S') {
        // SCHEMATICS
        f.Open(fileName, wxFile::write);
        f.Write(wxT("EESchema Schematic File Version 1\n"));
        wxFileName tmpFile(fileName);
        tmpFile.SetExt(wxEmptyString);
        f.Write(wxT("LIBS:") + tmpFile.GetFullPath() + wxT("\n"));
        f.Write(wxT("EELAYER 43  0\n"));
        f.Write(wxT("EELAYER END\n"));
        f.Write(wxT("$Descr User ") + wxString::Format("%d", m_sizeX) + ' ' + wxString::Format("%d", m_sizeY) + wxT("\n"));
        f.Write(wxT("$EndDescr\n"));
        // Junctions
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'J')
                m_schComponents[i]->WriteToFile(&f, ftype);
        }
        // Lines
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'L')
                m_schComponents[i]->WriteToFile(&f, ftype);
        }
        // Labels of lines - line and bus names
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'L')
                ((CSchLine *)m_schComponents[i])->WriteLabelToFile(&f, ftype);
        }
        // Symbols
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'S') {
                f.Write(wxT("$Comp\n"));
                m_schComponents[i]->WriteToFile(&f, ftype);
                f.Write(wxT("$EndComp\n"));
            }
        }

        f.Write(wxT("$EndSCHEMATC\n"));
        f.Close();
    } // SCHEMATICS
}
