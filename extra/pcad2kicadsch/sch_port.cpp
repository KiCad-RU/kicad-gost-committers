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
 * @file sch_port.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_port.h>

namespace PCAD2KICAD {

SCH_PORT::SCH_PORT()
{
    InitTTextValue( &m_labelText );
}


void SCH_PORT::Parse( XNODE*   aNode,
                      wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    XNODE*      lNode;
    wxString    propValue;

    m_objType   = wxT( "port" );

    lNode = FindNode( aNode, wxT( "pt" ) );

    if( lNode )
    {
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_positionX, &m_positionY, aActualConversion );
    }

    if( FindNode( aNode, wxT( "netNameRef" ) ) )
    {
        FindNode( aNode,
                  wxT( "netNameRef" ) )->GetAttribute( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_labelText.text = propValue;
    }

    if( FindNode( lNode, wxT( "rotation" ) ) )
        m_labelText.textRotation = StrToInt1Units(
            FindNode( lNode, wxT( "rotation" ) )->GetNodeContent() );
}


SCH_PORT::~SCH_PORT()
{
}

void SCH_PORT::WriteToFile( wxFile* aFile, char aFileType )
{
    wxString lr;

    if( m_labelText.textRotation == 0 )
        lr = wxT( '0' );
    else
        lr = wxT( '1' );

    aFile->Write( wxString::Format( wxT( "Text Label %d %d" ),
                                    m_positionX, m_positionY ) +
                  wxT( ' ' ) + lr + wxT( ' ' ) + wxT( " 60 ~\n" ) );
    aFile->Write( m_labelText.text + wxT( "\n" ) );
}

} // namespace PCAD2KICAD
