/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2014 CERN
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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PNS_ROUTER_H
#define __PNS_ROUTER_H

#include <list>

#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>

#include <geometry/shape_line_chain.h>

#include "pns_routing_settings.h"
#include "pns_sizes_settings.h"
#include "pns_item.h"
#include "pns_itemset.h"
#include "pns_node.h"

class PNS_DEBUG_DECORATOR;
class PNS_NODE;
class PNS_DIFF_PAIR_PLACER;
class PNS_PLACEMENT_ALGO;
class PNS_LINE_PLACER;
class PNS_ITEM;
class PNS_LINE;
class PNS_SOLID;
class PNS_SEGMENT;
class PNS_JOINT;
class PNS_VIA;
class PNS_RULE_RESOLVER;
class PNS_SHOVE;
class PNS_DRAGGER;

namespace KIGFX
{
    class VIEW;
    class VIEW_GROUP;
};


enum PNS_ROUTER_MODE {
    PNS_MODE_ROUTE_SINGLE = 1,
    PNS_MODE_ROUTE_DIFF_PAIR,
    PNS_MODE_TUNE_SINGLE,
    PNS_MODE_TUNE_DIFF_PAIR,
    PNS_MODE_TUNE_DIFF_PAIR_SKEW
};

/**
 * Class PNS_ROUTER
 *
 * Main router class.
 */

 class PNS_ROUTER_IFACE
 {
 public:
        PNS_ROUTER_IFACE() {};
        virtual ~PNS_ROUTER_IFACE() {};

        virtual void SetRouter( PNS_ROUTER* aRouter ) = 0;
        virtual void SyncWorld( PNS_NODE* aNode ) = 0;
        virtual void AddItem( PNS_ITEM* aItem ) = 0;
        virtual void RemoveItem( PNS_ITEM* aItem ) = 0;
        virtual void DisplayItem( const PNS_ITEM* aItem, int aColor = -1, int aClearance = -1 ) = 0;
        virtual void HideItem( PNS_ITEM* aItem ) = 0;
        virtual void Commit() = 0;
//        virtual void Abort () = 0;

        virtual void EraseView() = 0;
        virtual void UpdateNet( int aNetCode ) = 0;

        virtual PNS_RULE_RESOLVER* GetRuleResolver() = 0;
        virtual PNS_DEBUG_DECORATOR* GetDebugDecorator() = 0;
};

class PNS_ROUTER
{
private:
    enum RouterState
    {
        IDLE,
        DRAG_SEGMENT,
        ROUTE_TRACK
    };

public:
    PNS_ROUTER();
    ~PNS_ROUTER();

    void SetInterface( PNS_ROUTER_IFACE* aIface );
    void SetMode ( PNS_ROUTER_MODE aMode );
    PNS_ROUTER_MODE Mode() const { return m_mode; }

    static PNS_ROUTER* GetInstance();

    void ClearWorld();
    void SyncWorld();

    void SetView( KIGFX::VIEW* aView );

    bool RoutingInProgress() const;
    bool StartRouting( const VECTOR2I& aP, PNS_ITEM* aItem, int aLayer );
    void Move( const VECTOR2I& aP, PNS_ITEM* aItem );
    bool FixRoute( const VECTOR2I& aP, PNS_ITEM* aItem );

    void StopRouting();

    int GetClearance( const PNS_ITEM* aA, const PNS_ITEM* aB ) const;

    PNS_NODE* GetWorld() const
    {
        return m_world;
    }

    void FlipPosture();

    void DisplayItem( const PNS_ITEM* aItem, int aColor = -1, int aClearance = -1 );
    void DisplayItems( const PNS_ITEMSET& aItems );
    void DeleteTraces( PNS_ITEM* aStartItem, bool aWholeTrack );
    void SwitchLayer( int layer );

    void ToggleViaPlacement();
    void SetOrthoMode ( bool aEnable );

    int GetCurrentLayer() const;
    const std::vector<int> GetCurrentNets() const;

    void DumpLog();

    PNS_RULE_RESOLVER* GetRuleResolver() const
    {
        return m_iface->GetRuleResolver();
    }

    bool IsPlacingVia() const;

    const PNS_ITEMSET   QueryHoverItems( const VECTOR2I& aP );
    const VECTOR2I      SnapToItem( PNS_ITEM* aItem, VECTOR2I aP, bool& aSplitsSegment );

    bool StartDragging( const VECTOR2I& aP, PNS_ITEM* aItem );

    void SetIterLimit( int aX ) { m_iterLimit = aX; }
    int GetIterLimit() const { return m_iterLimit; };

    void SetShowIntermediateSteps( bool aX, int aSnapshotIter = -1 )
    {
        m_showInterSteps = aX;
        m_snapshotIter = aSnapshotIter;
    }

    bool GetShowIntermediateSteps() const { return m_showInterSteps; }
    int GetShapshotIter() const { return m_snapshotIter; }

    PNS_ROUTING_SETTINGS& Settings() { return m_settings; }

    void CommitRouting( PNS_NODE* aNode );

    /**
     * Applies stored settings.
     * @see Settings()
     */
    void UpdateSizes( const PNS_SIZES_SETTINGS& aSizes );

    /**
     * Changes routing settings to ones passed in the parameter.
     * @param aSettings are the new settings.
     */
    void LoadSettings( const PNS_ROUTING_SETTINGS& aSettings )
    {
        m_settings = aSettings;
    }

    void EnableSnapping( bool aEnable )
    {
        m_snappingEnabled = aEnable;
    }

    bool SnappingEnabled() const
    {
        return m_snappingEnabled;
    }

    PNS_SIZES_SETTINGS& Sizes()
    {
        return m_sizes;
    }

    PNS_ITEM* QueryItemByParent( const BOARD_ITEM* aItem ) const;


    void SetFailureReason ( const wxString& aReason ) { m_failureReason = aReason; }
    const wxString& FailureReason() const { return m_failureReason; }

    PNS_PLACEMENT_ALGO* Placer() { return m_placer; }

    PNS_ROUTER_IFACE* GetInterface() const
    {
        return m_iface;
    }

private:
    void movePlacing( const VECTOR2I& aP, PNS_ITEM* aItem );
    void moveDragging( const VECTOR2I& aP, PNS_ITEM* aItem );

    void eraseView();
    void updateView( PNS_NODE* aNode, PNS_ITEMSET& aCurrent );

    void clearViewFlags();

    // optHoverItem queryHoverItemEx(const VECTOR2I& aP);

    PNS_ITEM* pickSingleItem( PNS_ITEMSET& aItems ) const;
    void splitAdjacentSegments( PNS_NODE* aNode, PNS_ITEM* aSeg, const VECTOR2I& aP );

    PNS_ITEM* syncPad( D_PAD* aPad );
    PNS_ITEM* syncTrack( TRACK* aTrack );
    PNS_ITEM* syncVia( VIA* aVia );

    void commitPad( PNS_SOLID* aPad );
    void commitSegment( PNS_SEGMENT* aTrack );
    void commitVia( PNS_VIA* aVia );

    void highlightCurrent( bool enabled );

    void markViolations( PNS_NODE* aNode, PNS_ITEMSET& aCurrent, PNS_NODE::ITEM_VECTOR& aRemoved );

    VECTOR2I m_currentEnd;
    RouterState m_state;

    PNS_NODE* m_world;
    PNS_NODE* m_lastNode;
    PNS_PLACEMENT_ALGO * m_placer;
    PNS_DRAGGER* m_dragger;
    PNS_SHOVE* m_shove;
    PNS_ROUTER_IFACE* m_iface;

    int m_iterLimit;
    bool m_showInterSteps;
    int m_snapshotIter;

    KIGFX::VIEW* m_view;

    bool m_snappingEnabled;
    bool m_violation;

    PNS_ROUTING_SETTINGS m_settings;
    ///> Stores list of modified items in the current operation
    PNS_SIZES_SETTINGS m_sizes;
    PNS_ROUTER_MODE m_mode;

    wxString m_toolStatusbarName;
    wxString m_failureReason;
};

#endif
