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
 * @file sch_pin.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_pin.h>

namespace PCAD2KICAD {

SCH_PIN::SCH_PIN()
{
    m_objType = wxT( "pin" );
    InitTTextValue( &m_pinNum );
    InitTTextValue( &m_pinName );
    InitTTextValue( &m_number );
    m_pinType   = wxEmptyString;
    m_edgeStyle = wxEmptyString;
    m_pinLength = 0;
    m_pinNumVisible = true;
    m_pinNameVisible = false;
    m_isPower = false;
}


SCH_PIN::~SCH_PIN()
{
}


void SCH_PIN::Parse( XNODE* aNode )
{
    wxString    str, propValue;
    long        num;

// SCHLine.PartNum:=SymbolIndex;
    aNode->GetAttribute( wxT( "Name" ), &m_number.text );
    m_pinNum.text   = wxT( '0' );  // Default
    m_isVisible     = 0;    // Default is not visible

// SCHPin.pinName.Text:='~'; // Default
    m_pinNum.text = FindNodeGetContent( aNode, wxT( "symPinNum" ) );

// SCHPin.pinName.Text:=SCHPin.pinNum.Text; // Default
    if( FindNode( aNode, wxT( "pinName" ) ) )
    {
        FindNode( aNode, wxT( "pinName" ) )->GetAttribute( wxT( "Name" ),
                                                           &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_pinName.text = propValue;
    }

    m_pinType = FindNodeGetContent( aNode, wxT( "pinType" ) );

    if( FindNode( aNode, wxT( "partNum" ) ) )
    {
        FindNode( aNode, wxT( "partNum" ) )->GetNodeContent().ToLong( &num );
        m_partNum = (int) num;
    }

    if( m_pinName.text.Len() == 0 )
        m_pinName.text = wxT( '~' ); // Default
}


void SCH_PIN::ParsePinProperties( XNODE*   aNode, int aSymbolIndex,
                                  wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    wxString    pn, t, str;
    XNODE* cNode;

    pn = FindNodeGetContent( aNode, wxT( "pinNum" ) );

    if( m_pinNum.text == pn
        && m_partNum == aSymbolIndex )
    {
        m_isVisible = 1;                            // This pin is described, it means is visible
        m_pinLength = DEFAULT_SYMBOL_PIN_LENGTH;    // Default value

        if( FindNode( aNode, wxT( "pinLength" ) ) )
        {
            t = FindNode( aNode, wxT( "pinLength" ) )->GetNodeContent();
            m_pinLength = StrToIntUnits( GetAndCutWordWithMeasureUnits( &t,
                                                                        aDefaultMeasurementUnit ),
                                         wxT( ' ' ), aActualConversion );
        }

        m_edgeStyle = FindNodeGetContent( aNode, wxT( "outsideEdgeStyle" ) );

        if( FindNode( aNode, wxT( "rotation" ) ) )
            m_rotation = StrToInt1Units(
                FindNode( aNode, wxT( "rotation" ) )->GetNodeContent() );

        if( FindNode( aNode, wxT( "pt" ) ) )
        {
            SetPosition( FindNode( aNode, wxT( "pt" ) )->GetNodeContent(),
                         aDefaultMeasurementUnit,
                         &m_positionX,
                         &m_positionY,
                         aActualConversion );
        }

        str = FindNodeGetContent( aNode, wxT( "isFlipped" ) );
        if( str == wxT( "True" ) )
            m_mirror = 1;

        if( FindNode( aNode, wxT( "pinName" ) ) )
            if( FindNode( FindNode( aNode, wxT( "pinName" ) ), wxT( "text" ) ) )
                SetTextParameters( FindNode( FindNode( aNode, wxT( "pinName" ) ), wxT( "text" ) ),
                                   &m_pinName, aDefaultMeasurementUnit, aActualConversion );

        if( FindNode( aNode, wxT( "pinDes" ) ) )
            if( FindNode( FindNode( aNode, wxT( "pinDes" ) ), wxT( "text" ) ) )
                SetTextParameters( FindNode( FindNode( aNode, wxT( "pinDes" ) ), wxT( "text" ) ),
                                   &m_pinNum, aDefaultMeasurementUnit, aActualConversion );

        cNode = FindNode( aNode, wxT( "pinDisplay" ) );
        if( cNode )
        {
            str = FindNodeGetContent( cNode, wxT( "dispPinDes" ) );

            if( str == wxT( "False" ) )
                m_pinNumVisible = false;

            str = FindNodeGetContent( cNode, wxT( "dispPinName" ) );

            if( str == wxT( "True" ) )
                m_pinNameVisible = true;
        }
    }
}


void SCH_PIN::WriteToFile( wxFile* aFile, char aFileType )
{
    wxString orientation, pinType, shape;

    orientation = wxT( 'L' );

    if( m_rotation == 0 )
    {
        orientation = wxT( 'L' );
        m_positionX += m_pinLength;    // Set corrected to KiCad position
    }
    else if( m_rotation == 900 )
    {
        orientation = wxT( 'D' );
        m_positionY += m_pinLength;    // Set corrected to KiCad position
    }
    else if( m_rotation == 1800 )
    {
        orientation = wxT( 'R' );
        m_positionX -= m_pinLength;    // Set corrected to KiCad position
    }
    else if( m_rotation == 2700 )
    {
        orientation = wxT( 'U' );
        m_positionY -= m_pinLength;    // Set corrected to KiCad position
    }

    pinType = wxT( 'U' );    // Default

/*  E  Open E
 *   C Open C
 *   w Power Out
 *   W Power In
 *   U Unspec
 *   P Pasive
 *   T 3 State
 *   B BiDi
 *   O Output
 *   I Input */
    if( m_pinType == wxT( "Pasive" ) )
        pinType = wxT( 'P' );
    else if( m_pinType == wxT( "Input" ) )
        pinType = wxT( 'I' );
    else if( m_pinType == wxT( "Output" ) )
        pinType = wxT( 'O' );
    else if( m_pinType == wxT( "Power" ) )
        pinType = wxT( 'W' );
    else if( m_pinType == wxT( "Bidirectional" ) )
        pinType = wxT( 'B' );

    shape = wxEmptyString;    // Default , standard line without shape

    if( m_isPower )
    {
        shape = wxT( 'N' ); // Invisible
    }
    else
    {
        if( m_edgeStyle == wxT( "Dot" ) )
            shape = wxT( 'I' ); // Invert

        if( m_edgeStyle == wxT( "Clock" ) )
            shape = wxT( 'C' ); // Clock

        if( m_edgeStyle == wxT( "???" ) )
            shape = wxT( "IC" ); // Clock Invert

        if( m_isVisible == 0 )
            shape += wxT( 'N' ); // Invisible
    }

    // unit = 0 if common to the parts; if not, number of part (1. .n).
    // convert = 0 so common to the representations, if not 1 or 2.
    // Go out
    if( m_isPower )
    {
        int toX = m_positionX;
        int toY = m_positionY;

        switch( m_rotation )
        {
        case 0:
            toX += m_pinLength;
            break;
        case 900:
            toY -= m_pinLength;
            break;
        case 1800:
            toX -= m_pinLength;
            break;
        case 2700:
            toY += m_pinLength;
            break;
        }

        aFile->Write( wxString::Format( wxT( "P 2 %d 0 %d %d %d %d %d N\n" ),
                                        0,
                                        0,
                                        m_positionX,
                                        m_positionY,
                                        toX,
                                        toY ) );
        m_pinLength = 0;
    }

    if( !m_pinNumVisible )
        m_pinNum.textHeight = 0;

    if( !m_pinNameVisible )
        m_pinName.textHeight = 0;

    aFile->Write( wxT( "X " ) + m_pinName.text + wxT( ' ' ) + m_number.text + wxT( ' ' ) +
                  wxString::Format( wxT( "%d %d %d" ),
                                    m_positionX,
                                    m_positionY,
                                    m_pinLength ) + wxT( ' ' ) + orientation +
                  wxString::Format( wxT( " %d %d %d 0 " ),
                                    KIROUND( (double) m_pinNum.textHeight * TEXT_HEIGHT_TO_SIZE ),
                                    KIROUND( (double) m_pinName.textHeight * TEXT_HEIGHT_TO_SIZE ),
                                    m_partNum ) +
                  pinType + wxT( ' ' ) + shape + wxT( "\n" ) );
}

} // namespace PCAD2KICAD
