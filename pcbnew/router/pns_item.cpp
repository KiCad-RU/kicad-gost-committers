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

#include "pns_item.h"
#include "pns_line.h"

bool PNS_ITEM::collideSimple( const PNS_ITEM* aOther, int aClearance, bool aNeedMTV,
        VECTOR2I& aMTV ) const
{
    // same nets? no collision!
    if( m_net == aOther->m_net )
        return false;

    // check if we are not on completely different layers first
    if( !m_layers.Overlaps( aOther->m_layers ) )
        return false;

    return GetShape()->Collide( aOther->GetShape(), aClearance );

    // fixme: MTV
}


bool PNS_ITEM::Collide( const PNS_ITEM* aOther, int aClearance, bool aNeedMTV,
        VECTOR2I& aMTV ) const
{
    if( collideSimple( aOther, aClearance, aNeedMTV, aMTV ) )
        return true;

    // special case for "head" line with a via attached at the end.
    if( aOther->m_kind == LINE )
    {
        const PNS_LINE* line = static_cast<const PNS_LINE*>( aOther );

        if( line->EndsWithVia() )
            return collideSimple( &line->GetVia(), aClearance - line->GetWidth() / 2, aNeedMTV,
                    aMTV );
    }

    return false;
}


const std::string PNS_ITEM::GetKindStr() const
{
    switch( m_kind )
    {
    case LINE:
        return "line";

    case SEGMENT:
        return "segment";

    case VIA:
        return "via";

    case JOINT:
        return "joint";

    case SOLID:
        return "solid";

    default:
        return "unknown";
    }
}


PNS_ITEM::~PNS_ITEM()
{
}

