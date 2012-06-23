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
 * @file sch.h
 */

#ifndef SCH_H_
#define SCH_H_

#include <wx/wx.h>
#include <sch_component.h>
#include <sch_junction.h>
#include <sch_line.h>

class SCH : public wxObject
{
public:
    SCH_COMPONENTS_ARRAY m_schComponents;
    wxString m_defaultMeasurementUnit;
    int m_sizeX, m_sizeY;

    SCH();
    ~SCH();

    void Parse(wxStatusBar* aStatusBar, wxString aXMLFileName, wxString aActualConversion);

    virtual void WriteToFile(wxString aFileName, char aFileType);

private:
    void DoAlias(wxString aAlias);
    void DoLibrary(wxXmlDocument *aXmlDoc, wxStatusBar* aStatusBar, wxString aActualConversion);

    void LinesIntersect(const int aX1, const int aY1, const int aX2, const int aY2,
                    const int aX3, const int aY3, const int aX4, const int aY4,
                    int *aCode, int *aX, int *aY);

    bool IsPointOnLine(int aX, int aY, SCH_LINE *aLine);
    SCH_JUNCTION *CheckJunction(SCH_LINE *aSchLine, int aIndex);
};

#endif // SCH_H_
