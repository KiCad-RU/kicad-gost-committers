/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#include <gal/definitions.h>

#include <view/view_item.h>
#include <view/view.h>

using namespace KiGfx;

void VIEW_ITEM::ViewSetVisible( bool aIsVisible )
{
    bool update = false;

    if( m_viewVisible != aIsVisible )
    {
        update = true;
    }

    m_viewVisible = aIsVisible;

    // update only if the visibility has really changed
    if( update )
    {
        ViewUpdate( APPEARANCE );
    }
}


void VIEW_ITEM::ViewUpdate( int aUpdateFlags, bool aForceImmediateRedraw )
{
    m_view->invalidateItem( this, aUpdateFlags );

    if( aForceImmediateRedraw )
    {
        m_view->Redraw();
    }
}


void VIEW_ITEM::ViewRelease()
{
    if( m_view && m_view->IsDynamic() )
    {
        m_view->Remove( this );
    }
}
