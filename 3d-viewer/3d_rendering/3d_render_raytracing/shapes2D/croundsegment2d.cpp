/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Mario Luzeiro <mrluzeiro@ua.pt>
 * Copyright (C) 1992-2015 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file  croundsegment2d.cpp
 * @brief
 */

#include "croundsegment2d.h"
#include <wx/debug.h>


CROUNDSEGMENT2D::CROUNDSEGMENT2D( const SFVEC2F &aStart, const SFVEC2F &aEnd, float aWidth,
                                  const BOARD_ITEM &aBoardItem ) :
    COBJECT2D( OBJ2D_ROUNDSEG, aBoardItem ),
    m_segment( aStart, aEnd )
{
    m_radius = (aWidth / 2.0f);
    m_radius_squared = m_radius * m_radius;
    m_width = aWidth;

    SFVEC2F leftRadiusOffset( -m_segment.m_Dir.y * m_radius, m_segment.m_Dir.x * m_radius);
    m_leftStart = aStart + leftRadiusOffset;
    m_leftEnd   = aEnd   + leftRadiusOffset;
    m_leftEnd_minus_start = m_leftEnd - m_leftStart;
    m_leftDir   = glm::normalize( m_leftEnd_minus_start );

    SFVEC2F rightRadiusOffset( -leftRadiusOffset.x, -leftRadiusOffset.y );
    m_rightStart = aEnd   + rightRadiusOffset;
    m_rightEnd   = aStart + rightRadiusOffset;
    m_rightEnd_minus_start = m_rightEnd - m_rightStart;
    m_rightDir   = glm::normalize( m_rightEnd_minus_start );

    m_bbox.Reset();
    m_bbox.Set( aStart, aEnd );
    m_bbox.Set( m_bbox.Min() - SFVEC2F( m_radius, m_radius ), m_bbox.Max() + SFVEC2F( m_radius, m_radius ) );
    m_bbox.ScaleNextUp();
    m_centroid = m_bbox.GetCenter();

    wxASSERT( m_bbox.IsInitialized() );
}


bool CROUNDSEGMENT2D::Intersects( const CBBOX2D &aBBox ) const
{
    if( !m_bbox.Intersects( aBBox ) )
        return false;

    if( (aBBox.Max().x > m_bbox.Max().x) &&
        (aBBox.Max().y > m_bbox.Max().x) &&
        (aBBox.Min().x < m_bbox.Min().x) &&
        (aBBox.Min().y < m_bbox.Min().y)
        )
        return true;

    SFVEC2F v[4];

    v[0] = aBBox.Min();
    v[1] = SFVEC2F( aBBox.Min().x, aBBox.Max().y );
    v[2] = aBBox.Max();
    v[3] = SFVEC2F( aBBox.Max().x, aBBox.Min().y );

    // Test against the main rectangle segment
    if( IntersectSegment( m_leftStart, m_leftEnd_minus_start, v[0], v[1] - v[0] ) ) return true;
    if( IntersectSegment( m_leftStart, m_leftEnd_minus_start, v[1], v[2] - v[1] ) ) return true;
    if( IntersectSegment( m_leftStart, m_leftEnd_minus_start, v[2], v[3] - v[2] ) ) return true;
    if( IntersectSegment( m_leftStart, m_leftEnd_minus_start, v[3], v[0] - v[3] ) ) return true;

    if( IntersectSegment( m_rightStart, m_rightEnd_minus_start, v[0], v[1] - v[0] ) ) return true;
    if( IntersectSegment( m_rightStart, m_rightEnd_minus_start, v[1], v[2] - v[1] ) ) return true;
    if( IntersectSegment( m_rightStart, m_rightEnd_minus_start, v[2], v[3] - v[2] ) ) return true;
    if( IntersectSegment( m_rightStart, m_rightEnd_minus_start, v[3], v[0] - v[3] ) ) return true;

    // Test the two circles
    if( aBBox.Intersects( m_segment.m_Start, m_radius_squared ) ) return true;
    if( aBBox.Intersects( m_segment.m_End,   m_radius_squared ) ) return true;

    return false;
}


bool CROUNDSEGMENT2D::Overlaps( const CBBOX2D &aBBox ) const
{
    // NOT IMPLEMENTED
    return false;
}


bool CROUNDSEGMENT2D::Intersect( const RAYSEG2D &aSegRay, float *aOutT, SFVEC2F *aNormalOut ) const
{
    wxASSERT( aOutT );
    wxASSERT( aNormalOut );

    bool start_is_inside = IsPointInside( aSegRay.m_Start );
    bool end_is_inside = IsPointInside( aSegRay.m_End );

    // If segment if inside there are no hits
    if( start_is_inside && end_is_inside )
        return false;

    bool hitted = false;

    float closerHitT = FLT_MAX;
    float farHitT = FLT_MAX;

    SFVEC2F closerHitNormal;
    SFVEC2F farHitNormal;

    float leftSegT;
    bool leftSegmentHit = aSegRay.IntersectSegment( m_leftStart, m_leftEnd_minus_start, &leftSegT );

    if( leftSegmentHit )
    {
        hitted = true;
        closerHitT = leftSegT;
        farHitT    = leftSegT;

        closerHitNormal = SFVEC2F( -m_leftDir.y,  m_leftDir.x );
        farHitNormal    = SFVEC2F( -m_leftDir.y,  m_leftDir.x );
    }

    float rightSegT;
    bool rightSegmentHit = aSegRay.IntersectSegment( m_rightStart, m_rightEnd_minus_start, &rightSegT );

    if( rightSegmentHit )
    {
        if( !start_is_inside )
        if( (hitted == false) || (rightSegT < closerHitT) )
        {
            closerHitT = rightSegT;
            closerHitNormal = SFVEC2F( -m_rightDir.y,  m_rightDir.x );
        }

        if( start_is_inside )
        if( (hitted == false) || (rightSegT > farHitT) )
        {
            farHitT = rightSegT;
            farHitNormal = SFVEC2F( -m_rightDir.y,  m_rightDir.x );
        }

        hitted = true;
    }

    float   circleStart_T0;
    float   circleStart_T1;
    SFVEC2F circleStart_N0;
    SFVEC2F circleStart_N1;

    bool startCircleHit = aSegRay.IntersectCircle( m_segment.m_Start, m_radius,
                                                   &circleStart_T0, &circleStart_T1,
                                                   &circleStart_N0, &circleStart_N1 );

    if( startCircleHit )
    {
        if( circleStart_T0 > 0.0f )
        {
            if( !start_is_inside )
            if( (hitted == false) || (circleStart_T0 < closerHitT) )
            {
                closerHitT = circleStart_T0;
                closerHitNormal = circleStart_N0;
            }

            if( start_is_inside )
            if( (hitted == false) || (circleStart_T1 > farHitT) )
            {
                farHitT = circleStart_T1;
                farHitNormal = circleStart_N1;
            }
        }
        else
        {
            // This can only happen if the ray starts inside
            if( (hitted == false) || (circleStart_T1 > farHitT) )
            {
                farHitT = circleStart_T1;
                farHitNormal = circleStart_N1;
            }
        }

        hitted = true;
    }

    float   circleEnd_T0;
    float   circleEnd_T1;
    SFVEC2F circleEnd_N0;
    SFVEC2F circleEnd_N1;

    bool rightCircleHit = aSegRay.IntersectCircle( m_segment.m_End, m_radius,
                                                   &circleEnd_T0, &circleEnd_T1,
                                                   &circleEnd_N0, &circleEnd_N1 );
    if( rightCircleHit )
    {
        if( circleEnd_T0 > 0.0f )
        {
            if( !start_is_inside )
            if( (hitted == false) || (circleEnd_T0 < closerHitT) )
            {
                closerHitT = circleEnd_T0;
                closerHitNormal = circleEnd_N0;
            }

            if( start_is_inside )
            if( (hitted == false) || (circleEnd_T1 > farHitT) )
            {
                farHitT = circleEnd_T1;
                farHitNormal = circleEnd_N1;
            }
        }
        else
        {
            // This can only happen if the ray starts inside
            if( (hitted == false) || (circleEnd_T1 > farHitT) )
            {
                farHitT = circleEnd_T1;
                farHitNormal = circleEnd_N1;
            }
        }

        hitted = true;
    }

    if( hitted )
    {
        if( !start_is_inside )
        {
            *aOutT = closerHitT;
            //wxASSERT( (closerHitT > 0.0f) && (closerHitT <= 1.0f) );
            *aNormalOut = closerHitNormal;
        }
        else
        {
            wxASSERT( (farHitT > 0.0f) && (farHitT <= 1.0f) );
            *aOutT = farHitT;
            *aNormalOut = farHitNormal;
        }
    }

    return hitted;
}


INTERSECTION_RESULT CROUNDSEGMENT2D::IsBBoxInside( const CBBOX2D &aBBox ) const
{
    if( !m_bbox.Intersects( aBBox ) )
        return INTR_MISSES;

    SFVEC2F v[4];

    v[0] = aBBox.Min();
    v[1] = aBBox.Max();
    v[2] = SFVEC2F( aBBox.Min().x, aBBox.Max().y );
    v[3] = SFVEC2F( aBBox.Max().x, aBBox.Min().y );

    bool isInside[4];

    isInside[0] = IsPointInside( v[0] );
    isInside[1] = IsPointInside( v[1] );
    isInside[2] = IsPointInside( v[2] );
    isInside[3] = IsPointInside( v[3] );

    // Check if all points are inside the circle
    if( isInside[0] &&
        isInside[1] &&
        isInside[2] &&
        isInside[3] )
        return INTR_FULL_INSIDE;

    // Check if any point is inside the circle
    if( isInside[0] ||
        isInside[1] ||
        isInside[2] ||
        isInside[3] )
        return INTR_INTERSECTS;

    return INTR_MISSES;
}


bool CROUNDSEGMENT2D::IsPointInside( const SFVEC2F &aPoint ) const
{
    float dSquared = m_segment.DistanceToPointSquared( aPoint );

    if( dSquared <= m_radius_squared )
        return true;

    return false;
}
