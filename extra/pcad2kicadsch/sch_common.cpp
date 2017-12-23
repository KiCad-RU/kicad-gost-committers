/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Eldar Khayrullin <eldar.khayrullin@mail.ru>
 * Copyright (C) 2017 KiCad Developers, see CHANGELOG.TXT for contributors.
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
 * @file sch_common.cpp
 */

#include <wx/wx.h>

#include <common.h>
#include <pcad2kicad_common.h>
#include <sch_common.h>

namespace PCAD2KICAD {

int GetCorrectedHeight( int aHeight, bool isTrueType )
{
    return KIROUND( (double) aHeight *
                    ( ( isTrueType ) ? TRUETYPE_HEIGHT_TO_SIZE : STROKE_HEIGHT_TO_SIZE ) );
}


void CorrectLibText( TTEXTVALUE* aValue )
{
    if( aValue->textRotation == 1800 || aValue->textRotation == 2700 )
        RotateJustify180( &aValue->justify );

    if( aValue->mirror )
        MirrorJustify( &aValue->justify );

    if( aValue->textRotation == 1800 )
        aValue->textRotation = 0;
    else if( aValue->textRotation == 2700 )
        aValue->textRotation = 900;
}


void EscapeTextQuotes( wxString& aStr )
{
    aStr.Replace( wxT( "\"" ), wxT( "\\\"" ) );
}


void ReplaceTextQuotes( wxString& aStr )
{
    aStr.Replace( wxT( "\"" ), wxT( "_" ) );
}


void ReplaceTextSlashes( wxString& aStr )
{
    aStr.Replace( wxT( "\\" ), wxT( "_" ) );
    aStr.Replace( wxT( "/" ), wxT( "_" ) );
}


wxString GetJustifyString( const TTEXTVALUE* aValue )
{
    switch( aValue->justify )
    {
    case LowerLeft:
        return wxT( "L B" );
    case LowerCenter:
        return wxT( "C B" );
    case LowerRight:
        return wxT( "R B" );
    case Left:
        return wxT( "L C" );
    case Center:
        return wxT( "C C" );
    case Right:
        return wxT( "R C" );
    case UpperLeft:
        return wxT( "L T" );
    case UpperCenter:
        return wxT( "C T" );
    case UpperRight:
        return wxT( "R T" );
    default:
        return wxT( "L B" );
    }
}


void MirrorJustify( TTEXT_JUSTIFY* aJustify )
{
    switch( *aJustify )
    {
    case LowerLeft:
        *aJustify = LowerRight;
        break;
    case LowerRight:
        *aJustify = LowerLeft;
        break;
    case Left:
        *aJustify = Right;
        break;
    case Right:
        *aJustify = Left;
        break;
    case UpperLeft:
        *aJustify = UpperRight;
        break;
    case UpperRight:
        *aJustify = UpperLeft;
        break;
    default:
        break;
    }
}


void RotateJustify180( TTEXT_JUSTIFY* aJustify )
{
    switch( *aJustify )
    {
    case LowerLeft:
        *aJustify = UpperRight;
        break;
    case LowerCenter:
        *aJustify = UpperCenter;
        break;
    case LowerRight:
        *aJustify = UpperLeft;
        break;
    case Left:
        *aJustify = Right;
        break;
    case Right:
        *aJustify = Left;
        break;
    case UpperLeft:
        *aJustify = LowerRight;
        break;
    case UpperCenter:
        *aJustify = LowerCenter;
        break;
    case UpperRight:
        *aJustify = LowerLeft;
        break;
    default:
        break;
    }
}


//See drawtxt.cpp
int GetPenSizeForBold( int aTextSize )
{
    return KiROUND( aTextSize / 5.0 );
}

} // namespace PCAD2KICAD
