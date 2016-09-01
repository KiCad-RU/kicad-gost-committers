
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2016 KiCad Developers, see CHANGELOG.TXT for contributors.
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

#include <kiway.h>
#include <kiway_player.h>

#if defined(DEBUG)
 #include <typeinfo>
#endif


PROJECT& KIWAY_HOLDER::Prj() const
{
    return Kiway().Prj();
}


// this is not speed critical, hide it out of line.
void KIWAY_HOLDER::SetKiway( wxWindow* aDest, KIWAY* aKiway )
{
#if defined(DEBUG)
    // offer a trap point for debugging most any window
    wxASSERT( aDest );
    if( !strcmp( typeid(aDest).name(), "DIALOG_EDIT_LIBENTRY_FIELDS_IN_LIB" ) )
    {
        int breakhere=1;
        (void) breakhere;
    }
#endif

    (void) aDest;

    m_kiway = aKiway;
}
