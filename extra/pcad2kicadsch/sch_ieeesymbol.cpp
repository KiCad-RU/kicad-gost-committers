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
 * @file sch_ieeesymbol.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_ieeesymbol.h>

namespace PCAD2KICAD {

SCH_IEEESYMBOL::SCH_IEEESYMBOL()
{
    m_objType   = wxT( "ieeeSymbol" );
    m_type      = None;
    m_height    = 0;
}


SCH_IEEESYMBOL::~SCH_IEEESYMBOL()
{
}


void SCH_IEEESYMBOL::Parse( XNODE* aNode, int aSymbolIndex, wxString aDefaultMeasurementUnit,
                            wxString aActualConversion )
{
    XNODE*      lNode;
    wxString    propValue;

    m_partNum   = aSymbolIndex;

    propValue = aNode->GetNodeContent();
    propValue.Trim( true );
    propValue.Trim( false );
    if( propValue == wxT( "Adder" ) )
        m_type = Adder;
    else if( propValue == wxT( "Amplifier" ) )
        m_type = Amplifier;
    else if( propValue == wxT( "Astable" ) )
        m_type = Astable;
    else if( propValue == wxT( "Complex" ) )
        m_type = Complex;
    else if( propValue == wxT( "Generator" ) )
        m_type = Generator;
    else if( propValue == wxT( "Hysteresis" ) )
        m_type = Hysteresis;
    else if( propValue == wxT( "Multiplier" ) )
        m_type = Multiplier;

    lNode = FindNode( aNode, wxT( "pt" ) );
    if( lNode )
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_positionX, &m_positionY, aActualConversion );

    lNode = FindNode( aNode, wxT( "height" ) );
    if( lNode )
        SetHeight( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                   &m_height, aActualConversion );

    lNode = FindNode( aNode, wxT( "rotation" ) );
    if( lNode )
        m_rotation = StrToInt1Units( lNode->GetNodeContent() );

    propValue = FindNodeGetContent( aNode, wxT( "isFlipped" ) );
    m_mirror = ( propValue == wxT( "True" ) );
}


void SCH_IEEESYMBOL::WriteToFile( wxFile* aFile, char aFileType )
{
    Points points;

    switch( m_type )
    {
    case Adder:
        AppendAdderPoints( points );
        break;
    case Amplifier:
        AppendAmplifierPoints( points );
        break;
    case Astable:
        AppendAstablePoints( points );
        break;
    case Complex:
        AppendComplexPoints( points );
        WriteComplexCircleToFile( aFile );
        break;
    case Generator:
        AppendGeneratorPoints( points );
        break;
    case Hysteresis:
        AppendHysteresisPoints( points );
        break;
    case Multiplier:
        AppendMultiplierPoints( points );
        break;
    default:
        return;
    }

    TransformPoints( points );

    WritePointsToFile( aFile, points );
}


void SCH_IEEESYMBOL::AppendAdderPoints( Points& aPoints )
{
    const int halfHeight = KIROUND( (double) m_height / 2. );
    const int quarterHeight = KIROUND( (double) m_height / 4. );

    aPoints.push_back( Point( m_height, m_height - quarterHeight ) );
    aPoints.push_back( Point( m_height, m_height ) );
    aPoints.push_back( Point( 0, m_height ) );
    aPoints.push_back( Point( halfHeight, halfHeight ) );
    aPoints.push_back( Point( 0, 0 ) );
    aPoints.push_back( Point( m_height, 0 ) );
    aPoints.push_back( Point( m_height, quarterHeight ) );
}


void SCH_IEEESYMBOL::AppendAmplifierPoints( Points& aPoints )
{
    const int halfHeight = KIROUND( (double) m_height / 2. );

    aPoints.push_back( Point( 0, 0 ) );
    aPoints.push_back( Point( m_height, halfHeight ) );
    aPoints.push_back( Point( 0, m_height ) );
    aPoints.push_back( Point( 0, 0 ) );
}


void SCH_IEEESYMBOL::AppendAstablePoints( Points& aPoints )
{
    const int halfHeight = KIROUND( (double) m_height / 2. );

    aPoints.push_back( Point( 0, 0 ) );
    aPoints.push_back( Point( halfHeight, 0 ) );
    aPoints.push_back( Point( halfHeight, m_height ) );
    aPoints.push_back( Point( m_height, m_height ) );
    aPoints.push_back( Point( m_height, 0 ) );
    aPoints.push_back( Point( m_height + halfHeight, 0 ) );
    aPoints.push_back( Point( m_height + halfHeight, m_height ) );
    aPoints.push_back( Point( 2 * m_height, m_height ) );
    aPoints.push_back( Point( 2 * m_height, 0 ) );
    aPoints.push_back( Point( 2 * m_height + halfHeight, 0 ) );
}


void SCH_IEEESYMBOL::AppendComplexPoints( Points& aPoints )
{
    const int halfHeight = KIROUND( (double) m_height / 2. );

    aPoints.push_back( Point( 0, 0 ) );
    aPoints.push_back( Point( m_height, 0 ) );
    aPoints.push_back( BREAK_POINT );
    aPoints.push_back( Point( halfHeight, 0 ) );
    aPoints.push_back( Point( halfHeight, m_height ) );
    aPoints.push_back( BREAK_POINT );
    aPoints.push_back( Point( 0, m_height ) );
    aPoints.push_back( Point( m_height, m_height ) );
}


void SCH_IEEESYMBOL::AppendGeneratorPoints( Points& aPoints )
{
    const int halfHeight = KIROUND( (double) m_height / 2. );

    aPoints.push_back( Point( 0, 0 ) );
    aPoints.push_back( Point( halfHeight, 0 ) );
    aPoints.push_back( Point( halfHeight, m_height ) );
    aPoints.push_back( Point( m_height, m_height ) );
    aPoints.push_back( Point( m_height, 0 ) );
    aPoints.push_back( Point( m_height + halfHeight, 0 ) );
}


void SCH_IEEESYMBOL::AppendHysteresisPoints( Points& aPoints )
{
    const int halfHeight = KIROUND( (double) m_height / 2. );

    aPoints.push_back( Point( 0, 0 ) );
    aPoints.push_back( Point( halfHeight + m_height, 0 ) );
    aPoints.push_back( Point( halfHeight + m_height, m_height ) );
    aPoints.push_back( Point( halfHeight, m_height ) );
    aPoints.push_back( Point( halfHeight, 0 ) );
    aPoints.push_back( BREAK_POINT );
    aPoints.push_back( Point( halfHeight + m_height, m_height ) );
    aPoints.push_back( Point( 2 * m_height, m_height ) );
}


void SCH_IEEESYMBOL::AppendMultiplierPoints( Points& aPoints )
{
    const int thirdHeight = KIROUND( (double) m_height / 3. );

    aPoints.push_back( Point( 0, m_height ) );
    aPoints.push_back( Point( m_height, m_height ) );
    aPoints.push_back( BREAK_POINT );
    aPoints.push_back( Point( thirdHeight, 0 ) );
    aPoints.push_back( Point( thirdHeight, m_height ) );
    aPoints.push_back( BREAK_POINT );
    aPoints.push_back( Point( 2 * thirdHeight, 0 ) );
    aPoints.push_back( Point( 2 * thirdHeight, m_height ) );
}


void SCH_IEEESYMBOL::TransformPoints( Points& aPoints )
{
    for( auto point = aPoints.begin(); point != aPoints.end(); point++ )
    {
        if( *point == BREAK_POINT )
            continue;

        RotatePoint( &point->m_x, &point->m_y, -m_rotation );

        //FIXME check in PCAD
        if( m_mirror )
            point->m_x = -point->m_x;

        point->m_x += m_positionX;
        point->m_y += m_positionY;
    }
}


void SCH_IEEESYMBOL::WriteComplexCircleToFile( wxFile* aFile )
{
    const int halfHeight = KIROUND( (double) m_height / 2. );
    const int radius = KIROUND( (double) m_height / 8. );
    int posX = halfHeight;
    int posY = halfHeight;

    RotatePoint( &posX, &posY, -m_rotation );

    //FIXME check in PCAD
    if( m_mirror )
        posX = -posX;

    posX += m_positionX;
    posY += m_positionY;

    aFile->Write( wxString::Format( wxT( "C %d %d %d %d 1 0 N\n" ),
                                    posX, posY, radius, m_partNum ) );
}


void SCH_IEEESYMBOL::WritePointsToFile( wxFile* aFile, const Points& aPoints )
{
    const Point* begin = &aPoints[0];
    const Point* end;

    for( size_t i = 1; i < aPoints.size(); i++ )
    {
        end = &aPoints[i];

        if( *begin == BREAK_POINT || *end == BREAK_POINT )
        {
            begin = end;
            continue;
        }

        aFile->Write( wxString::Format( wxT( "P 2 %d 0 0 %d %d %d %d N\n" ),
                                        m_partNum, begin->m_x, begin->m_y, end->m_x, end->m_y ) );

        begin = end;
    }
}

} // namespace PCAD2KICAD
