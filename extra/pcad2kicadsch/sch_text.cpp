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
 * @file sch_text.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_common.h>
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
        m_text.text.Replace( "\r", "" );

        SetTextParameters( aNode, &m_text,
                           aDefaultMeasurementUnit, aActualConversion );
    }
}


void SCH_TEXT::WriteToFile( wxFile* aFile, char aFileType )
{
    wxString boldStr;
    wxString italicStr;

    m_text.text.Replace( wxT( "\n" ), wxT( "\\n" ), true );
    m_text.text.Replace( wxT( "\r" ), wxT( "\\r" ), true );
    m_text.text.Replace( wxT( "\t" ), wxT( "\\t" ), true );

    if( aFileType == wxT( 'L' ) ) // library
    {
        boldStr = m_text.isBold ? wxT( "1" ) : wxT( "0" );
        italicStr = m_text.isItalic ? wxT( "Italic" ) : wxT( "Normal" );

        aFile->Write( wxString::Format( wxT( "T %d %d %d %d 0 %d 0 " ), m_text.textRotation,
                                        m_text.textPositionX, m_text.textPositionY,
                                        KiROUND( (double) m_text.textHeight *
                                                 TEXT_HEIGHT_TO_SIZE ),
                                        m_partNum ) +
                      m_text.text + wxT( " " ) + italicStr + wxT( " " ) + boldStr +
                      wxT( " C C\n" ) );
    }
    else // schematic
    {
        int spin_style = CorrectSchTextPosition( &m_text );

        m_text.textHeight = KiROUND( (double) m_text.textHeight * TEXT_HEIGHT_TO_SIZE );

        if( m_text.isBold )
            boldStr = wxString::Format( wxT( "%i" ), GetPenSizeForBold( m_text.textHeight ) );
        else
            boldStr = wxT( "0" );

        italicStr = m_text.isItalic ? wxT( "Italic" ) : wxT( "~" );

        aFile->Write( wxString::Format( wxT( "Text Notes %d %d %d %d " ),
                                        m_text.correctedPositionX, m_text.correctedPositionY,
                                        spin_style, m_text.textHeight ) +
                      italicStr + wxT( ' ' ) + boldStr + wxT( "\n" ) );
        aFile->Write( m_text.text + wxT( "\n" ) );
    }
}


/* Correct text position and get Spin Style. See description of SCH_TEXT::SetLabelSpinStyle() */
int SCH_TEXT::CorrectSchTextPosition( TTEXTVALUE* aValue )
{
    int spin_style = 0;
    // sizes of justify correction
    int cl = KiROUND( (double) CalculateTextLengthSize( aValue ) / 2.0 );
    int ch = KiROUND( (double) aValue->textHeight );

    aValue->correctedPositionX = m_positionX;
    aValue->correctedPositionY = m_positionY;

    switch( aValue->textRotation )
    {
    case 0:
        if( aValue->justify == LowerLeft ||
            aValue->justify == Left ||
            aValue->justify == UpperLeft )
        {
            spin_style = aValue->mirror ? 2 : 0;
        }
        else if( aValue->justify == LowerRight ||
                 aValue->justify == Right ||
                 aValue->justify == UpperRight )
        {
            spin_style = aValue->mirror ? 0 : 2;
        }
        else
        {
            aValue->correctedPositionX -= cl;
            spin_style = 0;
        }

        if( aValue->justify == Left ||
            aValue->justify == Center ||
            aValue->justify == Right )
            aValue->correctedPositionY += ch / 2;
        else if( aValue->justify == UpperLeft ||
                 aValue->justify == UpperCenter ||
                 aValue->justify == UpperRight )
            aValue->correctedPositionY += ch;
        break;
    case 900:
        if( aValue->justify == LowerLeft ||
            aValue->justify == Left ||
            aValue->justify == UpperLeft )
        {
            spin_style = 1;
        }
        else if( aValue->justify == LowerRight ||
                 aValue->justify == Right ||
                 aValue->justify == UpperRight )
        {
            spin_style = 3;
        }
        else
        {
            aValue->correctedPositionY += cl;
            spin_style = 1;
        }

        if( aValue->justify == Left ||
            aValue->justify == Center ||
            aValue->justify == Right )
            aValue->correctedPositionX += ch / 2;
        else if( ( aValue->mirror &&
                   ( aValue->justify == LowerLeft ||
                     aValue->justify == LowerCenter ||
                     aValue->justify == LowerRight ) ) ||
                 ( !aValue->mirror &&
                   ( aValue->justify == UpperLeft ||
                     aValue->justify == UpperCenter ||
                     aValue->justify == UpperRight ) ) )
            aValue->correctedPositionX += ch;
        break;
    case 1800:
        if( aValue->justify == LowerLeft ||
            aValue->justify == Left ||
            aValue->justify == UpperLeft )
        {
            spin_style = aValue->mirror ? 0 : 2;
        }
        else if( aValue->justify == LowerRight ||
                 aValue->justify == Right ||
                 aValue->justify == UpperRight )
        {
            spin_style = aValue->mirror ? 2 : 0;
        }
        else
        {
            aValue->correctedPositionX -= cl;
            spin_style = 0;
        }

        if( aValue->justify == Left ||
            aValue->justify == Center ||
            aValue->justify == Right )
            aValue->correctedPositionY += ch / 2;
        else if( aValue->justify == LowerLeft ||
                 aValue->justify == LowerCenter ||
                 aValue->justify == LowerRight )
            aValue->correctedPositionY += ch;
        break;
    case 2700:
        if( aValue->justify == LowerLeft ||
            aValue->justify == Left ||
            aValue->justify == UpperLeft )
        {
            spin_style = 3;
        }
        else if( aValue->justify == LowerRight ||
                 aValue->justify == Right ||
                 aValue->justify == UpperRight )
        {
            spin_style = 1;
        }
        else
        {
            aValue->correctedPositionY += cl;
            spin_style = 1;
        }

        if( aValue->justify == Left ||
            aValue->justify == Center ||
            aValue->justify == Right )
            aValue->correctedPositionX += ch / 2;
        else if( ( !aValue->mirror &&
                   ( aValue->justify == LowerLeft ||
                     aValue->justify == LowerCenter ||
                     aValue->justify == LowerRight ) ) ||
                 ( aValue->mirror &&
                   ( aValue->justify == UpperLeft ||
                     aValue->justify == UpperCenter ||
                     aValue->justify == UpperRight ) ) )
            aValue->correctedPositionX += ch;
        break;
    default:
        break;
    }

    return spin_style;
}

} // namespace PCAD2KICAD
