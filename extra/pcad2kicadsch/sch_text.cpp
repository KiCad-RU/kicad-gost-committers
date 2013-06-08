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
 * @file sch_text.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_text.h>

namespace PCAD2KICAD {

SCH_TEXT::SCH_TEXT( int aSymbolIndex )
{
    m_objType   = wxT( "text" );

    InitTTextValue( &m_text );
    m_partNum   = aSymbolIndex;
}


SCH_TEXT::~SCH_TEXT()
{
}


void SCH_TEXT::Parse( XNODE*  aNode,
                      wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    XNODE*      lNode;

    lNode = FindNode( aNode, wxT( "pt" ) );

    if( lNode )
    {
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_positionX, &m_positionY, aActualConversion );
    }

    if( FindNode( lNode, wxT( "rotation" ) ) )
        m_text.textRotation = StrToInt1Units(
            FindNode( lNode, wxT( "rotation" ) )->GetNodeContent() );

    if( aNode->GetName() == wxT( "text" ) )
    {
        aNode->GetAttribute( wxT( "Name" ), &m_text.text );

        SetTextParameters( aNode, &m_text,
                           aDefaultMeasurementUnit, aActualConversion );
    }
}


void SCH_TEXT::WriteToFile( wxFile* aFile, char aFileType )
{
    if( aFileType == wxT( 'L' ) ) // library
    {
        aFile->Write( wxString::Format( wxT( "T %d %d %d %d 0 %d 0 " ), m_text.textRotation,
                                        m_text.textPositionX, m_text.textPositionY,
                                        m_text.textHeight, m_partNum ) +
                      m_text.text + wxT( " Normal 0 C C\n" ) );
    }
    else // schematic
    {
        wxString lr;

        m_text.text.Replace( wxT( "\n" ), wxT( "\\n" ), true );
        m_text.text.Replace( wxT( "\r" ), wxT( "\\r" ), true );
        m_text.text.Replace( wxT( "\t" ), wxT( "\\t" ), true );

        if( m_text.textRotation == 0 )
            lr = wxT( '0' );
        else
            lr = wxT( '1' );

        aFile->Write( wxString::Format( wxT( "Text Notes %d %d" ),
                                        m_positionX, m_positionY ) +
                      wxT( ' ' ) + lr + wxT( ' ' ) + wxT( " 60 ~\n" ) );
        aFile->Write( m_text.text + wxT( "\n" ) );
    }
}

} // namespace PCAD2KICAD
