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
 * @file sch_arc.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_arc.h>

namespace PCAD2KICAD {

SCH_ARC::SCH_ARC()
{
    m_objType       = wxT( "arc" );
    m_startX        = 0;
    m_startY        = 0;
    m_toX           = 0;
    m_toY           = 0;
    m_startAngle    = 0;
    m_sweepAngle    = 0;
    m_radius        = 0;
}


SCH_ARC::~SCH_ARC()
{
}


void SCH_ARC::Parse( XNODE*   aNode, int aSymbolIndex,
                     wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    XNODE*      lNode;

    m_partNum   = aSymbolIndex;

    lNode = FindNode( aNode, wxT( "width" ) );

    if( lNode )
        SetWidth( lNode->GetNodeContent(), aDefaultMeasurementUnit, &m_width, aActualConversion );

    if( aNode->GetName() == wxT( "triplePointArc" ) )
    {
        // origin
        lNode = FindNode( aNode, wxT( "pt" ) );

        if( lNode )
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_positionX, &m_positionY, aActualConversion );

        lNode = lNode->GetNext();

        if( lNode )
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_startX, &m_startY, aActualConversion );

        lNode = lNode->GetNext();

        if( lNode )
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_toX, &m_toY, aActualConversion );

        m_radius = KIROUND( GetLineLength( wxPoint( m_positionX, m_positionY ),
                                           wxPoint( m_startX, m_startY ) ) );

        if( ( m_startX == m_toX ) && ( m_startY == m_toY ) )
        {
            m_sweepAngle = 3600;
        }
        else
        {
            m_startAngle = KIROUND( ArcTangente( m_startY - m_positionY,
                                                 m_startX - m_positionX ) );
            m_sweepAngle = m_startAngle -
                           KIROUND( ArcTangente( m_toY - m_positionY, m_toX - m_positionX ) );
        }
    }
    else if( aNode->GetName() == wxT( "arc" ) )
    {
        lNode = FindNode( aNode, wxT( "pt" ) );

        if( lNode )
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_positionX, &m_positionY, aActualConversion );

        lNode = FindNode( aNode, wxT( "radius" ) );

        if( lNode )
        {
            wxString propValue = lNode->GetNodeContent();
            m_radius = StrToIntUnits(
                           GetAndCutWordWithMeasureUnits( &propValue, aDefaultMeasurementUnit ),
                           wxT( ' ' ), aActualConversion );
        }

        lNode = FindNode( aNode, wxT( "startAngle" ) );

        if( lNode )
            m_startAngle = StrToInt1Units( lNode->GetNodeContent() );

        m_startX = KiROUND( m_positionX +
                            m_radius * sin( ( m_startAngle - 900. ) * M_PI / 1800. ) );
        m_startY = KiROUND( m_positionY -
                            m_radius * cos( ( m_startAngle - 900. ) * M_PI / 1800. ) );

        lNode = FindNode( aNode, wxT( "sweepAngle" ) );

        if( lNode )
            m_sweepAngle = StrToInt1Units( lNode->GetNodeContent() );

        m_toX = KiROUND( m_positionX +
                         m_radius * sin( ( m_startAngle + m_sweepAngle - 900. ) * M_PI / 1800. ) );
        m_toY = KiROUND( m_positionY -
                         m_radius * cos( ( m_startAngle + m_sweepAngle - 900. ) * M_PI / 1800. ) );
    }
}


void SCH_ARC::WriteToFile( wxFile* aFile, char aFileType )
{
    if( m_sweepAngle == 3600 )
    {
        aFile->Write( wxString::Format( wxT( "C %d %d %d %d 1 %d N\n" ),
                                        m_positionX, m_positionY, m_radius, m_partNum, m_width ) );
    }
    else
    {
        int startAngle = m_startAngle;
        int endAngle = m_startAngle + m_sweepAngle;

        CorrectAngles( startAngle, endAngle );

        aFile->Write( wxString::Format( wxT( "A %d %d %d %d %d %d 0 %d N %d %d %d %d\n" ),
                                        m_positionX, m_positionY, m_radius, startAngle, endAngle,
                                        m_partNum, m_width, m_startX, m_startY, m_toX, m_toY ) );
    }
}


void SCH_ARC::CorrectAngles( int& aStartAngle, int& aEndAngle )
{
    NORMALIZE_ANGLE_POS( aStartAngle );
    NORMALIZE_ANGLE_POS( aEndAngle );  // angles = 0 .. 3600

    // Restrict angle to less than 180 to avoid PBS display mirror Trace because it is
    // assumed that the arc is less than 180 deg to find orientation after rotate or mirror.
    // See LIB_ARC::calcRadiusAngles().
    if( ( aEndAngle - aStartAngle ) > 1800 )
        aEndAngle -= 3600;
    else if( ( aEndAngle - aStartAngle ) <= -1800 )
        aEndAngle += 3600;

    while( ( aEndAngle - aStartAngle ) >= 1800 )
    {
        aEndAngle--;
        aStartAngle++;
    }

    while( ( aStartAngle - aEndAngle ) >= 1800 )
    {
        aEndAngle++;
        aStartAngle--;
    }

    NORMALIZE_ANGLE_POS( aStartAngle );
    NORMALIZE_ANGLE_POS( aEndAngle );
}

} // namespace PCAD2KICAD
