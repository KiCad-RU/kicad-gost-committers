/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
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
 * @file sch_text.h
 */

#ifndef SCH_TEXT_H_
#define SCH_TEXT_H_

#include <wx/wx.h>
#include <XMLtoObjectCommonProceduresUnit.h>

#include <sch_component.h>

namespace PCAD2KICAD {

class SCH_TEXT : public SCH_COMPONENT
{
public:
    TTEXTVALUE      m_text;

    SCH_TEXT();
    ~SCH_TEXT();

    virtual void    Parse( XNODE* aNode, int aSymbolIndex,
                           wxString aDefaultMeasurementUnit, wxString aActualConversion );

    virtual void    WriteToFile( wxFile* aFile, char aFileType );
};

} // namespace PCAD2KICAD

#endif    // SCH_TEXT_H_
