/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
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
 * @file sch_port.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_port.h>

namespace PCAD2KICAD {

SCH_PORT::SCH_PORT()
{
    m_objType   = wxT( "port" );

    InitTTextValue( &m_labelText );
}


void SCH_PORT::Parse( XNODE*   aNode,
                      wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    XNODE*      lNode;
    wxString    propValue;

    if( ( lNode = FindNode( aNode, wxT( "pt" ) ) ) )
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_positionX, &m_positionY, aActualConversion );

    if( ( lNode = FindNode( aNode, wxT( "netNameRef" ) ) ) )
    {
        lNode->GetAttribute( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_labelText.text = propValue;
    }

    if( ( lNode = FindNode( aNode, wxT( "rotation" ) ) ) )
        m_labelText.textRotation = StrToInt1Units( lNode->GetNodeContent() );
    else
        m_labelText.textRotation = 0;

    propValue = FindNodeGetContent( aNode, wxT( "portType" ) );
    if( propValue.EndsWith( wxT( "_Horz" ) ) )
        m_isHorizontal = true;
    else
        m_isHorizontal = false;

    propValue = FindNodeGetContent( aNode, wxT( "isFlipped" ) );
    if( propValue == wxT( "True" ) )
        m_labelText.mirror = 1;
    else
        m_labelText.mirror = 0;

    SetPortFontProperty( aNode, &m_labelText, aDefaultMeasurementUnit, aActualConversion );
}


SCH_PORT::~SCH_PORT()
{
}

void SCH_PORT::WriteToFile( wxFile* aFile, char aFileType )
{
    int lr;

    if( m_isHorizontal )
    {
        switch( m_labelText.textRotation )
        {
        case 0:
            lr = 0;
            break;
        case 900:
            lr = 1;
            break;
        case 1800:
            lr = 2;
            break;
        case 2700:
            lr = 3;
            break;
        }

        if( m_labelText.mirror )
        {
            if( lr == 0 )
                lr = 2;
            else if( lr == 2 )
                lr = 0;
        }
    }
    else
    {
        if( m_labelText.textRotation == 0 || m_labelText.textRotation == 1800 )
        {
            lr = 0;
            m_positionX -= KIROUND( (double) CalculateTextLengthSize( &m_labelText ) / 2. );
        }
        else
        {
            lr = 1;
            m_positionY += KIROUND( (double) CalculateTextLengthSize( &m_labelText ) / 2. );
        }
    }

    aFile->Write( wxString::Format( wxT( "Text Label %d %d %d %d ~\n" ),
                                    m_positionX, m_positionY, lr,
                                    KIROUND( (double) m_labelText.textHeight *
                                             TEXT_HEIGHT_TO_SIZE ) ) );
    aFile->Write( m_labelText.text + wxT( "\n" ) );
}


void SCH_PORT::SetPortFontProperty( XNODE*        aNode,
                                    TTEXTVALUE*   aTextValue,
                                    wxString      aDefaultMeasurementUnit,
                                    wxString      aActualConversion )
{
    wxString n = wxT( "(PortStyle)" );
    wxString propValue;

    while( aNode->GetName() != wxT( "www.lura.sk" ) )
        aNode = aNode->GetParent();

    aNode = FindNode( aNode, wxT( "library" ) );

    if( aNode )
        aNode = FindNode( aNode, wxT( "textStyleDef" ) );

    while( aNode )
    {
        aNode->GetAttribute( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );

        if( propValue == n )
            break;

        aNode = aNode->GetNext();
    }

    if( aNode )
    {
        bool isTrueType;
        wxString fontType;

        propValue = FindNodeGetContent( aNode, wxT( "textStyleDisplayTType" ) );
        if( propValue == wxT( "True" ) )
            isTrueType = true;
        else
            isTrueType = false;

        aNode = FindNode( aNode, wxT( "font" ) );
        fontType = FindNodeGetContent( aNode, wxT( "fontType" ) );
        if( ( isTrueType && ( fontType != wxT( "TrueType" ) ) ) ||
            ( !isTrueType && ( fontType != wxT( "Stroke" ) ) ) )
            aNode = aNode->GetNext();

        if( aNode )
        {
            if( FindNode( aNode, wxT( "fontHeight" ) ) )
                SetHeight( FindNode( aNode, wxT( "fontHeight" ) )->GetNodeContent(),
                           aDefaultMeasurementUnit, &aTextValue->textHeight,
                           wxT( "FONT" ) );

            if( FindNode( aNode, wxT( "strokeWidth" ) ) )
                SetWidth( FindNode( aNode, wxT( "strokeWidth" ) )->GetNodeContent(),
                          aDefaultMeasurementUnit, &aTextValue->textstrokeWidth,
                          wxT( "FONT" ) );
        }
    }
}

} // namespace PCAD2KICAD
