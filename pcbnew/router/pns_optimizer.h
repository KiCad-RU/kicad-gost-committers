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

#ifndef __PNS_OPTIMIZER_H
#define __PNS_OPTIMIZER_H

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <geometry/shape_index_list.h>
#include <geometry/shape_line_chain.h>

class PNS_NODE;
class PNS_LINE;
class PNS_ROUTER;

/**
 * Class PNS_COST_ESTIMATOR
 *
 * Calculates the cost of a given line, taking corner angles and total length into account.
 **/

class PNS_COST_ESTIMATOR
{
public:
    PNS_COST_ESTIMATOR() :
        m_lengthCost( 0 ),
        m_cornerCost( 0 )
    {};

    PNS_COST_ESTIMATOR( const PNS_COST_ESTIMATOR& b ) :
        m_lengthCost( b.m_lengthCost ),
        m_cornerCost( b.m_cornerCost )
    {};

    ~PNS_COST_ESTIMATOR() {};

    static int CornerCost( const SEG& a, const SEG& b );
    static int CornerCost( const SHAPE_LINE_CHAIN& aLine );
    static int CornerCost( const PNS_LINE& aLine );

    void Add( PNS_LINE& aLine );
    void Remove( PNS_LINE& aLine );
    void Replace( PNS_LINE& aOldLine, PNS_LINE& aNewLine );

    bool IsBetter( PNS_COST_ESTIMATOR& aOther, double aLengthTollerance,
            double aCornerTollerace ) const;

    double GetLengthCost() const { return m_lengthCost; }
    double GetCornerCost() const { return m_cornerCost; }

private:
    double m_lengthCost;
    int m_cornerCost;
};

/**
 * Class PNS_OPTIMIZER
 *
 * Performs various optimizations of the lines being routed, attempting to make the lines shorter
 * and less cornery. There are 3 kinds of optimizations so far:
 * - Merging obtuse segments (MERGE_OBTUSE): tries to join together as many
 *   obtuse segments as possible without causing collisions
 * - Rerouting path between pair of line corners with a 2-segment "\__" line and iteratively repeating
 *   the procedure as long as the total cost of the line keeps decreasing
 * - "Smart Pads" - that is, rerouting pad/via exits to make them look nice (SMART_PADS).
 **/

class PNS_OPTIMIZER
{
public:
    enum OptimizationEffort
    {
        MERGE_SEGMENTS  = 0x01,
        SMART_PADS      = 0x02,
        MERGE_OBTUSE    = 0x04
    };

    PNS_OPTIMIZER( PNS_NODE* aWorld );
    ~PNS_OPTIMIZER();

    ///> a quick shortcut to optmize a line without creating and setting up an optimizer
    static bool Optimize( PNS_LINE* aLine, int aEffortLevel, PNS_NODE* aWorld = NULL );

    bool Optimize( PNS_LINE* aLine, PNS_LINE* aResult = NULL,
            int aStartVertex = 0, int aEndVertex = -1 );

    void SetWorld( PNS_NODE* aNode ) { m_world = aNode; }
    void CacheStaticItem( PNS_ITEM* aItem );
    void CacheRemove( PNS_ITEM* aItem );
    void ClearCache( bool aStaticOnly = false );

    void SetCollisionMask( int aMask )
    {
        m_collisionKindMask = aMask;
    }

    void SetEffortLevel( int aEffort )
    {
        m_effortLevel = aEffort;
    }

private:
    static const int MaxCachedItems = 256;

    typedef std::vector<SHAPE_LINE_CHAIN> BreakoutList;

    struct CacheVisitor;

    struct CachedItem
    {
        int hits;
        bool isStatic;
    };

    bool mergeObtuse( PNS_LINE* aLine );
    bool mergeFull( PNS_LINE* aLine );
    bool removeUglyCorners( PNS_LINE* aLine );
    bool runSmartPads( PNS_LINE* aLine );
    bool mergeStep( PNS_LINE* aLine, SHAPE_LINE_CHAIN& aCurrentLine, int step );

    bool checkColliding( PNS_ITEM* aItem, bool aUpdateCache = true );
    bool checkColliding( PNS_LINE* aLine, const SHAPE_LINE_CHAIN& aOptPath );


    void cacheAdd( PNS_ITEM* aItem, bool aIsStatic );
    void removeCachedSegments( PNS_LINE* aLine, int aStartVertex = 0, int aEndVertex = -1 );

    BreakoutList circleBreakouts( int aWidth, const SHAPE* aShape, bool aPermitDiagonal ) const;
    BreakoutList rectBreakouts( int aWidth, const SHAPE* aShape, bool aPermitDiagonal ) const;
    BreakoutList ovalBreakouts( int aWidth, const SHAPE* aShape, bool aPermitDiagonal ) const;
    BreakoutList computeBreakouts( int aWidth, const PNS_ITEM* aItem,
            bool aPermitDiagonal ) const;

    int smartPadsSingle( PNS_LINE* aLine, PNS_ITEM* aPad, bool aEnd, int aEndVertex );

    PNS_ITEM* findPadOrVia( int aLayer, int aNet, const VECTOR2I& aP ) const;

    SHAPE_INDEX_LIST<PNS_ITEM*> m_cache;

    typedef boost::unordered_map<PNS_ITEM*, CachedItem> CachedItemTags;
    CachedItemTags m_cacheTags;
    PNS_NODE* m_world;
    int m_collisionKindMask;
    int m_effortLevel;
    bool m_keepPostures;
};

#endif
