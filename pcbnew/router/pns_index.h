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

#ifndef __PNS_INDEX_H
#define __PNS_INDEX_H

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

#include <list>
#include <geometry/shape_index.h>

#include "pns_item.h"

/**
 * Class PNS_INDEX
 *
 * Custom spatial index, holding our board items and allowing for very fast searches. Items
 * are assigned to separate R-Tree subundices depending on their type and spanned layers, reducing
 * overlap and improving search time.
 **/

class PNS_INDEX
{
public:
    typedef std::list<PNS_ITEM*>            NetItemsList;
    typedef SHAPE_INDEX<PNS_ITEM*>          ItemShapeIndex;
    typedef boost::unordered_set<PNS_ITEM*> ItemSet;

    PNS_INDEX();
    ~PNS_INDEX();

    void Add( PNS_ITEM* aItem );
    void Remove( PNS_ITEM* aItem );
    void Replace( PNS_ITEM* aOldItem, PNS_ITEM* aNewItem );

    template<class Visitor>
    int Query( const PNS_ITEM* aItem, int aMinDistance, Visitor& v );

    template<class Visitor>
    int Query( const SHAPE* aShape, int aMinDistance, Visitor& v );

    void Clear();

    NetItemsList* GetItemsForNet( int aNet );

    ItemSet::iterator begin() { return m_allItems.begin(); }
    ItemSet::iterator end() { return m_allItems.end(); }

    bool Contains( PNS_ITEM* aItem ) const
    {
        return m_allItems.find( aItem ) != m_allItems.end();
    }

    int Size() const { return m_allItems.size(); }

private:
    static const int    MaxSubIndices   = 64;
    static const int    SI_Multilayer   = 2;
    static const int    SI_SegDiagonal  = 0;
    static const int    SI_SegStraight  = 1;
    static const int    SI_Traces   = 3;
    static const int    SI_PadsTop  = 0;
    static const int    SI_PadsBottom = 1;

    template <class Visitor>
    int querySingle( int index, const SHAPE* aShape, int aMinDistance, Visitor& v );

    ItemShapeIndex* getSubindex( const PNS_ITEM* aItem );

    ItemShapeIndex* m_subIndices[MaxSubIndices];
    std::map<int, NetItemsList> m_netMap;
    ItemSet m_allItems;
};


PNS_INDEX::PNS_INDEX()
{
    memset( m_subIndices, 0, sizeof( m_subIndices ) );
}


PNS_INDEX::ItemShapeIndex* PNS_INDEX::getSubindex( const PNS_ITEM* aItem )
{
    int idx_n = -1;

    const PNS_LAYERSET l = aItem->GetLayers();

    switch( aItem->GetKind() )
    {
    case PNS_ITEM::VIA:
        idx_n = SI_Multilayer;
        break;

    case PNS_ITEM::SOLID:
        {
            if( l.IsMultilayer() )
                idx_n = SI_Multilayer;
            else if( l.Start() == 0 ) // fixme: use kicad layer codes
                idx_n = SI_PadsTop;
            else if( l.Start() == 15 )
                idx_n = SI_PadsBottom;

            break;
        }

    case PNS_ITEM::SEGMENT:
    case PNS_ITEM::LINE:
        idx_n = SI_Traces + 2 * l.Start() + SI_SegStraight;
        break;

    default:
        break;
    }

    assert( idx_n >= 0 && idx_n < MaxSubIndices );

    if( !m_subIndices[idx_n] )
        m_subIndices[idx_n] = new ItemShapeIndex;

    return m_subIndices[idx_n];
}


void PNS_INDEX::Add( PNS_ITEM* aItem )
{
    ItemShapeIndex* idx = getSubindex( aItem );

    idx->Add( aItem );
    m_allItems.insert( aItem );
    int net = aItem->GetNet();

    if( net >= 0 )
    {
        m_netMap[net].push_back( aItem );
    }
}


void PNS_INDEX::Remove( PNS_ITEM* aItem )
{
    ItemShapeIndex* idx = getSubindex( aItem );

    idx->Remove( aItem );
    m_allItems.erase( aItem );

    int net = aItem->GetNet();

    if( net >= 0 && m_netMap.find( net ) != m_netMap.end() )
        m_netMap[net].remove( aItem );
}


void PNS_INDEX::Replace( PNS_ITEM* aOldItem, PNS_ITEM* aNewItem )
{
    Remove( aOldItem );
    Add( aNewItem );
}


template<class Visitor>
int PNS_INDEX::querySingle( int index, const SHAPE* aShape, int aMinDistance, Visitor& v )
{
    if( !m_subIndices[index] )
        return 0;

    return m_subIndices[index]->Query( aShape, aMinDistance, v, false );
}


template<class Visitor>
int PNS_INDEX::Query( const PNS_ITEM* aItem, int aMinDistance, Visitor& v )
{
    const SHAPE* shape = aItem->GetShape();
    int total = 0;

    total += querySingle( SI_Multilayer, shape, aMinDistance, v );

    const PNS_LAYERSET layers = aItem->GetLayers();

    if( layers.IsMultilayer() )
    {
        total += querySingle( SI_PadsTop, shape, aMinDistance, v );
        total += querySingle( SI_PadsBottom, shape, aMinDistance, v );

        for( int i = layers.Start(); i <= layers.End(); ++i )
            total += querySingle( SI_Traces + 2 * i + SI_SegStraight, shape, aMinDistance, v );
    }
    else
    {
        int l = layers.Start();

        if( l == 0 )
            total += querySingle( SI_PadsTop, shape, aMinDistance, v );
        else if( l == 15 )
            total += querySingle( SI_PadsBottom, shape, aMinDistance, v );

        total += querySingle(  SI_Traces + 2 * l + SI_SegStraight, shape, aMinDistance, v );
    }

    return total;
}


template<class Visitor>
int PNS_INDEX::Query( const SHAPE* aShape, int aMinDistance, Visitor& v )
{
    int total = 0;

    for( int i = 0; i < MaxSubIndices; i++ )
        total += querySingle( i, aShape, aMinDistance, v );

    return total;
}


void PNS_INDEX::Clear()
{
    for( int i = 0; i < MaxSubIndices; ++i )
    {
        ItemShapeIndex* idx = m_subIndices[i];

        if( idx )
            delete idx;

        m_subIndices[i] = NULL;
    }
}


PNS_INDEX::~PNS_INDEX()
{
    Clear();
}


PNS_INDEX::NetItemsList* PNS_INDEX::GetItemsForNet( int aNet )
{
    if( m_netMap.find( aNet ) == m_netMap.end() )
        return NULL;

    return &m_netMap[aNet];
}

#endif

