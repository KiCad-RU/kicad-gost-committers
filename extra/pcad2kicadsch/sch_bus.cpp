/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012-2013 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file sch_bus.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_bus.h>

namespace PCAD2KICAD {

SCH_BUS::SCH_BUS()
{
    m_objType   = wxT( "line" );
    m_lineType  = wxT( 'B' ); // Bus
}


void SCH_BUS::Parse( XNODE*   aNode, wxString aDefaultMeasurementUnit,
                     wxString aActualConversion )
{
    XNODE*      lNode;
    wxString    propValue;

    m_labelText.textIsVisible = 0;
    aNode->GetAttribute( wxT( "Name" ), &m_labelText.text );
    m_labelText.text.Trim( false );
    m_labelText.text.Trim( true );

    lNode = FindNode( aNode, wxT( "pt" ) );

    if( lNode )
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_positionX, &m_positionY, aActualConversion );

    if( lNode )
        lNode = lNode->GetNext();

    if( lNode )
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_toX, &m_toY, aActualConversion );

    lNode = FindNode( aNode, wxT( "dispName" ) );

    if( lNode )
    {
        lNode->GetAttribute( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );

        if( propValue == wxT( "True" ) )
            m_labelText.textIsVisible = 1;
    }

    lNode = FindNode( aNode, wxT( "text" ) );

    if( lNode )
    {
        if( FindNode( lNode, wxT( "pt" ) ) )
            SetPosition( FindNode( lNode, wxT( "pt" ) )->GetNodeContent(),
                         aDefaultMeasurementUnit, &m_labelText.textPositionX,
                         &m_labelText.textPositionY, aActualConversion );

        if( FindNode( lNode, wxT( "rotation" ) ) )
            m_labelText.textRotation = StrToInt1Units(
                FindNode( lNode, wxT( "rotation" ) )->GetNodeContent() );
    }
}

} // namespace PCAD2KICAD
