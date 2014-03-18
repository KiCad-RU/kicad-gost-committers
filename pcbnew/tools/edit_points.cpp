/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#include <boost/foreach.hpp>

#include <gal/graphics_abstraction_layer.h>
#include "edit_points.h"

bool EDIT_POINT::WithinPoint( const VECTOR2I& aPoint, unsigned int aSize ) const
{
    // Corners of the EDIT_POINT square
    VECTOR2I topLeft = GetPosition() - aSize;
    VECTOR2I bottomRight = GetPosition() + aSize;

    return ( aPoint.x > topLeft.x && aPoint.y > topLeft.y &&
             aPoint.x < bottomRight.x && aPoint.y < bottomRight.y );
}


EDIT_POINTS::EDIT_POINTS( EDA_ITEM* aParent ) :
    EDA_ITEM( NOT_USED ), m_parent( aParent )
{
}


EDIT_POINT* EDIT_POINTS::FindPoint( const VECTOR2I& aLocation )
{
    float size = m_view->ToWorld( EDIT_POINT::POINT_SIZE );

    std::deque<EDIT_POINT>::iterator pit, pitEnd;
    for( pit = m_points.begin(), pitEnd = m_points.end(); pit != pitEnd; ++pit )
    {
        EDIT_POINT& point = *pit;

        if( point.WithinPoint( aLocation, size ) )
            return &point;
    }

    std::deque<EDIT_LINE>::iterator lit, litEnd;
    for( lit = m_lines.begin(), litEnd = m_lines.end(); lit != litEnd; ++lit )
    {
        EDIT_LINE& point = *lit;

        if( point.WithinPoint( aLocation, size ) )
            return &point;
    }

    return NULL;
}


EDIT_POINT* EDIT_POINTS::Previous( const EDIT_POINT& aPoint )
{
    for( unsigned int i = 0; i < m_points.size(); ++i )
    {
        if( m_points[i] == aPoint )
        {
            if( i == 0 )
                return &m_points[m_points.size() - 1];
            else
                return &m_points[i - 1];
        }
    }

    for( unsigned int i = 0; i < m_lines.size(); ++i )
    {
        if( m_lines[i] == aPoint )
        {
            if( i == 0 )
                return &m_lines[m_lines.size() - 1];
            else
                return &m_lines[i - 1];
        }
    }

    return NULL;
}


EDIT_POINT* EDIT_POINTS::Next( const EDIT_POINT& aPoint )
{
    for( unsigned int i = 0; i < m_points.size(); ++i )
    {
        if( m_points[i] == aPoint )
        {
            if( i == m_points.size() - 1 )
                return &m_points[0];
            else
                return &m_points[i + 1];
        }
    }

    for( unsigned int i = 0; i < m_lines.size(); ++i )
    {
        if( m_lines[i] == aPoint )
        {
            if( i == m_lines.size() - 1 )
                return &m_lines[0];
            else
                return &m_lines[i + 1];
        }
    }

    return NULL;
}


void EDIT_POINTS::ViewDraw( int aLayer, KIGFX::GAL* aGal ) const
{
    aGal->SetFillColor( KIGFX::COLOR4D( 1.0, 1.0, 1.0, 1.0 ) );
    aGal->SetIsFill( true );
    aGal->SetIsStroke( false );
    aGal->PushDepth();
    aGal->SetLayerDepth( -512.0 );      // TODO no hardcoded depths?

    float size = m_view->ToWorld( EDIT_POINT::POINT_SIZE );

    BOOST_FOREACH( const EDIT_POINT& point, m_points )
        aGal->DrawRectangle( point.GetPosition() - size / 2, point.GetPosition() + size / 2 );

    BOOST_FOREACH( const EDIT_LINE& line, m_lines )
        aGal->DrawCircle( line.GetPosition(), size / 2 );

    aGal->PopDepth();
}
