/**
 * @file class_board_connected_item.cpp
 * @brief BOARD_CONNECTED_ITEM class functions.
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <fctsys.h>
#include <pcbnew.h>

#include <class_board.h>
#include <class_board_item.h>


BOARD_CONNECTED_ITEM::BOARD_CONNECTED_ITEM( BOARD_ITEM* aParent, KICAD_T idtype ) :
    BOARD_ITEM( aParent, idtype )
{
    m_NetCode    = 0;
    m_Subnet     = 0;
    m_ZoneSubnet = 0;
}


BOARD_CONNECTED_ITEM::BOARD_CONNECTED_ITEM( const BOARD_CONNECTED_ITEM& aItem ) :
    BOARD_ITEM( aItem )
{
    m_NetCode = aItem.m_NetCode;
    m_Subnet = aItem.m_Subnet;
    m_ZoneSubnet = aItem.m_ZoneSubnet;
}


/**
 * Function GetNet
 * @return int - the net code.
 */
int BOARD_CONNECTED_ITEM::GetNet() const
{
    return m_NetCode;
}


void BOARD_CONNECTED_ITEM::SetNet( int aNetCode )
{
    m_NetCode = aNetCode;
}


/**
 * Function GetSubNet
 * @return int - the sub net code.
 */
int BOARD_CONNECTED_ITEM::GetSubNet() const
{
    return m_Subnet;
}


void BOARD_CONNECTED_ITEM::SetSubNet( int aSubNetCode )
{
    m_Subnet = aSubNetCode;
}


/**
 * Function GetZoneSubNet
 * @return int - the sub net code in zone connections.
 */
int BOARD_CONNECTED_ITEM::GetZoneSubNet() const
{
    return m_ZoneSubnet;
}


void BOARD_CONNECTED_ITEM::SetZoneSubNet( int aSubNetCode )
{
    m_ZoneSubnet = aSubNetCode;
}


int BOARD_CONNECTED_ITEM::GetClearance( BOARD_CONNECTED_ITEM* aItem ) const
{
    NETCLASS*   myclass  = GetNetClass();

    // DO NOT use wxASSERT, because GetClearance is called inside an OnPaint event
    // and a call to wxASSERT can crash the application.
    if( myclass )
    {
        int myClearance  = myclass->GetClearance();
        // @todo : after GetNetClass() is reliably not returning NULL, remove the
        // tests for if( myclass )

        if( aItem )
        {
            int hisClearance = aItem->GetClearance();
            return std::max( hisClearance, myClearance );
        }

        return myClearance;
    }
    else
    {
#ifdef __WXDEBUG__
        wxLogWarning(wxT("BOARD_CONNECTED_ITEM::GetClearance():NULL netclass,type %d"), Type() );
#endif
    }

    return 0;
}


/** return a pointer to the netclass of the zone
 * if the net is not found (can happen when a netlist is reread,
 * and the net name is not existant, return the default net class
 * So should not return a null pointer
 */
NETCLASS* BOARD_CONNECTED_ITEM::GetNetClass() const
{
    // It is important that this be implemented without any sequential searching.
    // Simple array lookups should be fine, performance-wise.
    BOARD*  board = GetBoard();

    // DO NOT use wxASSERT, because GetNetClass is called inside an OnPaint event
    // and a call to wxASSERT can crash the application.

    if( board == NULL )     // Should not occur
    {
#ifdef __WXDEBUG__
        wxLogWarning( wxT("BOARD_CONNECTED_ITEM::GetNetClass():NULL board,type %d"), Type() );
#endif
        return NULL;
    }

    NETCLASS*       netclass = NULL;
    int             netcode  = GetNet();
    NETINFO_ITEM*   net = board->FindNet( netcode );

    if( net )
    {
        netclass = net->GetNetClass();

#ifdef __WXDEBUG__
        if( netclass == NULL )
        {
            wxLogWarning( wxT("BOARD_CONNECTED_ITEM::GetNetClass():NULL netclass,type %d"), Type() );
        }
#endif
    }

    if( netclass )
        return netclass;
    else
        return board->m_NetClasses.GetDefault();
}

/**
 * Function GetNetClassName
 * @return the Net Class name of this item
 */
wxString BOARD_CONNECTED_ITEM::GetNetClassName() const
{
    wxString    name;
    NETCLASS*   myclass = GetNetClass();

    if( myclass )
        name = myclass->GetName();
    else
    {
        BOARD*  board = GetBoard();
        name = board->m_NetClasses.GetDefault()->GetName();
    }

    return name;
}
