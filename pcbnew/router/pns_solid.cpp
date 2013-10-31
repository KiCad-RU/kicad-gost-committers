/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013  CERN
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.or/licenses/>.
 */

#include <math/vector2d.h>

#include <geometry/shape.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_rect.h>
#include <geometry/shape_circle.h>

#include "pns_solid.h"
#include "pns_utils.h"

const SHAPE_LINE_CHAIN PNS_SOLID::Hull( int aClearance, int aWalkaroundThickness ) const
{
    switch( m_shape->Type() )
    {
    case SH_RECT:
        {
            SHAPE_RECT* rect = static_cast<SHAPE_RECT*>( m_shape );
            return OctagonalHull( rect->GetPosition(), rect->GetSize(),
                    aClearance + 1, 0.2 * aClearance );
        }

    case SH_CIRCLE:
        {
            SHAPE_CIRCLE* circle = static_cast<SHAPE_CIRCLE*>( m_shape );
            int r = circle->GetRadius();
            return OctagonalHull( circle->GetCenter() - VECTOR2I( r, r ), VECTOR2I( 2 * r, 2 * r ),
                    aClearance + 1, 0.52 * (r + aClearance) );
        }

    default:
        break;
    }

    return SHAPE_LINE_CHAIN();
}


PNS_ITEM* PNS_SOLID::Clone() const
{
    // solids are never cloned as the shove algorithm never moves them
    assert( false );

    return NULL;
}
