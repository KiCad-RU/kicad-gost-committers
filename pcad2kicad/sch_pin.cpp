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
 * @file sch_pin.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_pin.h>


SCH_PIN::SCH_PIN()
{
    InitTTextValue( &m_pinNum );
    InitTTextValue( &m_pinName );
    InitTTextValue( &m_number );
    m_pinType   = wxEmptyString;
    m_edgeStyle = wxEmptyString;
    m_pinLength = 0;
    m_pinNumVisible = true;
    m_pinNameVisible = false;
}


SCH_PIN::~SCH_PIN()
{
}


void SCH_PIN::Parse( wxXmlNode* aNode )
{
    wxString    str, propValue;
    long        num;

    m_objType = wxT( "pin" );
// SCHLine.PartNum:=SymbolIndex;
    aNode->GetAttribute( wxT( "Name" ), &m_number.text );
    m_pinNum.text   = '0';  // Default
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
        m_pinName.text = '~'; // Default
}


void SCH_PIN::ParsePinProperties( wxXmlNode* aNode, int aSymbolIndex,
                                  wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    wxString    pn, t, str;
    TTEXTVALUE  lpn;
    wxXmlNode* cNode;

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
                                         ' ', aActualConversion );
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
        {
            lpn = m_number;

            if( FindNode( FindNode( aNode,
                                    wxT( "pinName" ) )->GetChildren(), wxT( "text" ) ) )
            {
                SetTextParameters( FindNode( FindNode( aNode, wxT( "pinName" ) ), wxT( "text" ) ),
                                   &lpn, aDefaultMeasurementUnit, aActualConversion );
            }
        }

        if( FindNode( aNode, wxT( "pinDes" ) ) )
        {
            lpn = m_pinName;

            if( FindNode( FindNode( aNode, wxT( "pinDes" ) ), wxT( "text" ) ) )
            {
                SetTextParameters( FindNode( FindNode( aNode, wxT( "pinDes" ) ), wxT( "text" ) ),
                                   &lpn, aDefaultMeasurementUnit, aActualConversion );
            }
        }

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
    char        orientation, pinType;
    wxString    shape;

    orientation = 'L';

    if( m_rotation == 0 )
    {
        orientation = 'L';
        m_positionX += m_pinLength;    // Set corrected to KiCad position
    }

    if( m_rotation == 900 )
    {
        orientation = 'D';
        m_positionY += m_pinLength;    // Set corrected to KiCad position
    }

    if( m_rotation == 1800 )
    {
        orientation = 'R';
        m_positionX -= m_pinLength;    // Set corrected to KiCad position
    }

    if( m_rotation == 2700 )
    {
        orientation = 'U';
        m_positionY -= m_pinLength;    // Set corrected to KiCad position
    }

    pinType = 'U';    // Default

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
        pinType = 'P';

    if( m_pinType == wxT( "Input" ) )
        pinType = 'I';

    if( m_pinType == wxT( "Output" ) )
        pinType = 'O';

    if( m_pinType == wxT( "Power" ) )
        pinType = 'W';

    if( m_pinType == wxT( "Bidirectional" ) )
        pinType = 'B';

    shape = wxEmptyString;    // Default , standard line without shape

    if( m_edgeStyle == wxT( "Dot" ) )
        shape = 'I'; // Invert

    if( m_edgeStyle == wxT( "Clock" ) )
        shape = 'C'; // Clock

    if( m_edgeStyle == wxT( "???" ) )
        shape = wxT( "IC" ); // Clock Invert

    if( m_isVisible == 0 )
        shape += 'N'; // Invisible

    // unit = 0 if common to the parts; if not, number of part (1. .n).
    // convert = 0 so common to the representations, if not 1 or 2.
    // Go out
    aFile->Write( wxT( "X " ) + m_pinName.text + ' ' + m_number.text + ' ' +
                  wxString::Format( "%d %d %d",
                                    m_positionX,
                                    m_positionY,
                                    m_pinLength ) + ' ' + orientation +
                  wxString::Format( " 30 30 %d 0 ", m_partNum ) +
                  pinType + ' ' + shape + wxT( "\n" ) );
}
