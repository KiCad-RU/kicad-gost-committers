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
 * @file PCBPolygon.h
 */

#ifndef PCBPOLYGON_H_
#define PCBPOLYGON_H_

#include <wx/wx.h>

#include <PCBComponent.h>

WX_DEFINE_ARRAY(wxRealPoint *, CVerticesArray);
WX_DEFINE_ARRAY(CVerticesArray *, CIslandsArray);

class CPCBPolygon : public CPCBComponent
{
public:
    int m_width;
    CVerticesArray m_outline;  // collection of boundary/outline lines - objects
    //CPCBLinesArray m_fill_lines;
    CIslandsArray m_islands;
    CIslandsArray m_cutouts;

    CPCBPolygon(CPCBCallbacks *aCallbacks);
    ~CPCBPolygon();

    virtual bool Parse(wxXmlNode *aNode, int aPCadLayer,
        wxString aDefaultMeasurementUnit, wxString aActualConversion, wxStatusBar* aStatusBar);

    virtual void WriteToFile(wxFile *aFile, char aFileType);
    virtual void WriteOutlineToFile(wxFile *aFile, char aFileType);
    virtual void SetPosOffset(int aX_offs, int aY_offs);

//protected:
    void FormPolygon(wxXmlNode *aNode, CVerticesArray *aPolygon,
        wxString aDefaultMeasurementUnit, wxString actualConversion);

};

#endif // PCBPOLYGON_H_
