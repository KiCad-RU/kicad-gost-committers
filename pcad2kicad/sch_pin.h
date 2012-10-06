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
 * @file sch_pin.h
 */

#ifndef SCH_PIN_H_
#define SCH_PIN_H_

#include <wx/wx.h>

#include <sch_component.h>
#include <XMLtoObjectCommonProceduresUnit.h>

namespace PCAD2KICAD {

#define DEFAULT_SYMBOL_PIN_LENGTH 300

class SCH_PIN : public SCH_COMPONENT
{
public:
    TTEXTVALUE  m_pinNum;
    TTEXTVALUE  m_pinName;
    TTEXTVALUE  m_number;
    wxString    m_pinType;
    wxString    m_edgeStyle;
    int         m_pinLength;
    bool        m_pinNumVisible;
    bool        m_pinNameVisible;

    SCH_PIN();
    ~SCH_PIN();

    virtual void    Parse( XNODE* aNode );

    void            ParsePinProperties( XNODE*      aNode,
                                        int         aSymbolIndex,
                                        wxString    aDefaultMeasurementUnit,
                                        wxString    aActualConversion );

    virtual void    WriteToFile( wxFile* aFile, char aFileType );
};

} // namespace PCAD2KICAD

#endif    // SCH_PIN_H_
