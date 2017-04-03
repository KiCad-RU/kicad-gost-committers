/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2016 CERN
 * Copyright (C) 2016 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <class_undoredo_container.h>
#include <class_board.h>
#include <class_board_connected_item.h>
#include <class_module.h>
#include <class_track.h>
#include <board_commit.h>
#include <ratsnest_data.h>
#include <layers_id_colors_and_visibility.h>
#include <geometry/convex_hull.h>
#include <wxPcbStruct.h>

#include <unordered_set>
#include <unordered_map>

#include <view/view.h>
#include <view/view_item.h>
#include <view/view_group.h>
#include <gal/graphics_abstraction_layer.h>

#include <pcb_painter.h>

#include <geometry/shape.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_rect.h>
#include <geometry/shape_circle.h>
#include <geometry/shape_convex.h>
#include <geometry/convex_hull.h>


#include "pns_kicad_iface.h"
#include "pns_routing_settings.h"
#include "pns_sizes_settings.h"
#include "pns_item.h"
#include "pns_solid.h"
#include "pns_segment.h"
#include "pns_solid.h"
#include "pns_itemset.h"
#include "pns_node.h"
#include "pns_topology.h"
#include "pns_router.h"
#include "pns_debug_decorator.h"
#include "router_preview_item.h"

class PNS_PCBNEW_RULE_RESOLVER : public PNS::RULE_RESOLVER
{
public:
    PNS_PCBNEW_RULE_RESOLVER( BOARD* aBoard, PNS::ROUTER* aRouter );
    virtual ~PNS_PCBNEW_RULE_RESOLVER();

    virtual int Clearance( const PNS::ITEM* aA, const PNS::ITEM* aB ) const override;
    virtual int Clearance( int aNetCode ) const override;
    virtual void OverrideClearance( bool aEnable, int aNetA = 0, int aNetB = 0, int aClearance = 0 ) override;
    virtual void UseDpGap( bool aUseDpGap ) override { m_useDpGap = aUseDpGap; }
    virtual int DpCoupledNet( int aNet ) override;
    virtual int DpNetPolarity( int aNet ) override;
    virtual bool DpNetPair( PNS::ITEM* aItem, int& aNetP, int& aNetN ) override;

private:
    struct CLEARANCE_ENT
    {
        int coupledNet;
        int clearance;
    };

    int localPadClearance( const PNS::ITEM* aItem ) const;
    int matchDpSuffix( wxString aNetName, wxString& aComplementNet, wxString& aBaseDpName );

    PNS::ROUTER* m_router;
    BOARD*       m_board;

    std::vector<CLEARANCE_ENT> m_netClearanceCache;
    std::unordered_map<const D_PAD*, int> m_localClearanceCache;
    int m_defaultClearance;
    bool m_overrideEnabled;
    int m_overrideNetA, m_overrideNetB;
    int m_overrideClearance;
    bool m_useDpGap;
};


PNS_PCBNEW_RULE_RESOLVER::PNS_PCBNEW_RULE_RESOLVER( BOARD* aBoard, PNS::ROUTER* aRouter ) :
    m_router( aRouter ),
    m_board( aBoard )
{
    PNS::NODE* world = m_router->GetWorld();

    PNS::TOPOLOGY topo( world );
    m_netClearanceCache.resize( m_board->GetNetCount() );

    // Build clearance cache for net classes
    for( unsigned int i = 0; i < m_board->GetNetCount(); i++ )
    {
        NETINFO_ITEM* ni = m_board->FindNet( i );

        if( ni == NULL )
            continue;

        CLEARANCE_ENT ent;
        ent.coupledNet = DpCoupledNet( i );

        wxString netClassName = ni->GetClassName();
        NETCLASSPTR nc = m_board->GetDesignSettings().m_NetClasses.Find( netClassName );

        int clearance = nc->GetClearance();
        ent.clearance = clearance;
        m_netClearanceCache[i] = ent;

        wxLogTrace( "PNS", "Add net %u netclass %s clearance %d", i, netClassName.mb_str(), clearance );
    }

    // Build clearance cache for pads
    for( MODULE* mod = m_board->m_Modules; mod ; mod = mod->Next() )
    {
        auto moduleClearance = mod->GetLocalClearance();

        for( D_PAD* pad = mod->Pads(); pad; pad = pad->Next() )
        {
            int padClearance = pad->GetLocalClearance();

            if( padClearance > 0 )
                m_localClearanceCache[ pad ] = padClearance;

            else if( moduleClearance > 0 )
                m_localClearanceCache[ pad ] = moduleClearance;
        }
    }

    //printf("DefaultCL : %d\n",  m_board->GetDesignSettings().m_NetClasses.Find ("Default clearance")->GetClearance());

    m_overrideEnabled = false;
    m_defaultClearance = Millimeter2iu( 0.254 );    // m_board->m_NetClasses.Find ("Default clearance")->GetClearance();
    m_overrideNetA = 0;
    m_overrideNetB = 0;
    m_overrideClearance = 0;
    m_useDpGap = false;
}


PNS_PCBNEW_RULE_RESOLVER::~PNS_PCBNEW_RULE_RESOLVER()
{
}


int PNS_PCBNEW_RULE_RESOLVER::localPadClearance( const PNS::ITEM* aItem ) const
{
    if( !aItem->Parent() || aItem->Parent()->Type() != PCB_PAD_T )
        return 0;

    const D_PAD* pad = static_cast<D_PAD*>( aItem->Parent() );

    auto i = m_localClearanceCache.find( pad );

    if( i == m_localClearanceCache.end() )
        return 0;

    return i->second;
}


int PNS_PCBNEW_RULE_RESOLVER::Clearance( const PNS::ITEM* aA, const PNS::ITEM* aB ) const
{
    int net_a = aA->Net();
    int cl_a = ( net_a >= 0 ? m_netClearanceCache[net_a].clearance : m_defaultClearance );
    int net_b = aB->Net();
    int cl_b = ( net_b >= 0 ? m_netClearanceCache[net_b].clearance : m_defaultClearance );

    bool linesOnly = aA->OfKind( PNS::ITEM::SEGMENT_T | PNS::ITEM::LINE_T )
                  && aB->OfKind( PNS::ITEM::SEGMENT_T | PNS::ITEM::LINE_T );

    if( linesOnly && net_a >= 0 && net_b >= 0 && m_netClearanceCache[net_a].coupledNet == net_b )
    {
        cl_a = cl_b = m_router->Sizes().DiffPairGap() - 2 * PNS_HULL_MARGIN;
    }

    int pad_a = localPadClearance( aA );
    int pad_b = localPadClearance( aB );

    if( pad_a > 0 )
        cl_a = pad_a;

    if( pad_b > 0 )
        cl_b = pad_b;

    return std::max( cl_a, cl_b );
}


int PNS_PCBNEW_RULE_RESOLVER::Clearance( int aNetCode ) const
{
    if( aNetCode > 0 && aNetCode < (int) m_netClearanceCache.size() )
        return m_netClearanceCache[aNetCode].clearance;

    return m_defaultClearance;
}


// fixme: ugly hack to make the optimizer respect gap width for currently routed differential pair.
void PNS_PCBNEW_RULE_RESOLVER::OverrideClearance( bool aEnable, int aNetA, int aNetB , int aClearance )
{
    m_overrideEnabled = aEnable;
    m_overrideNetA = aNetA;
    m_overrideNetB = aNetB;
    m_overrideClearance = aClearance;
}


int PNS_PCBNEW_RULE_RESOLVER::matchDpSuffix( wxString aNetName, wxString& aComplementNet, wxString& aBaseDpName )
{
    int rv = 0;

    if( aNetName.EndsWith( "+" ) )
    {
        aComplementNet = "-";
        rv = 1;
    }
    else if( aNetName.EndsWith( "_P" ) )
    {
        aComplementNet = "_N";
        rv = 1;
    }
    else if( aNetName.EndsWith( "-" ) )
    {
        aComplementNet = "+";
        rv = -1;
    }
    else if( aNetName.EndsWith( "_N" ) )
    {
        aComplementNet = "_P";
        rv = -1;
    }

    if( rv != 0 )
    {
        aBaseDpName = aNetName.Left( aNetName.Length() - aComplementNet.Length() );
        aComplementNet = aBaseDpName + aComplementNet;
    }

    return rv;
}


int PNS_PCBNEW_RULE_RESOLVER::DpCoupledNet( int aNet )
{
    wxString refName = m_board->FindNet( aNet )->GetNetname();
    wxString dummy, coupledNetName;

    if( matchDpSuffix( refName, coupledNetName, dummy ) )
    {
        NETINFO_ITEM* net = m_board->FindNet( coupledNetName );

        if( !net )
            return -1;

        return net->GetNet();
    }

    return -1;
}


int PNS_PCBNEW_RULE_RESOLVER::DpNetPolarity( int aNet )
{
    wxString refName = m_board->FindNet( aNet )->GetNetname();
    wxString dummy1, dummy2;

    return matchDpSuffix( refName, dummy1, dummy2 );
}


bool PNS_PCBNEW_RULE_RESOLVER::DpNetPair( PNS::ITEM* aItem, int& aNetP, int& aNetN )
{
    if( !aItem || !aItem->Parent() || !aItem->Parent()->GetNet() )
        return false;

    wxString netNameP = aItem->Parent()->GetNet()->GetNetname();
    wxString netNameN, netNameCoupled, netNameBase;

    int r = matchDpSuffix( netNameP, netNameCoupled, netNameBase );

    if( r == 0 )
        return false;
    else if( r == 1 )
    {
        netNameN = netNameCoupled;
    }
    else
    {
        netNameN = netNameP;
        netNameP = netNameCoupled;
    }

//    wxLogTrace( "PNS","p %s n %s base %s\n", (const char *)netNameP.c_str(), (const char *)netNameN.c_str(), (const char *)netNameBase.c_str() );

    NETINFO_ITEM* netInfoP = m_board->FindNet( netNameP );
    NETINFO_ITEM* netInfoN = m_board->FindNet( netNameN );

    //wxLogTrace( "PNS","ip %p in %p\n", netInfoP, netInfoN);

    if( !netInfoP || !netInfoN )
        return false;

    aNetP = netInfoP->GetNet();
    aNetN = netInfoN->GetNet();

    return true;
}


class PNS_PCBNEW_DEBUG_DECORATOR: public PNS::DEBUG_DECORATOR
{
public:
    PNS_PCBNEW_DEBUG_DECORATOR( KIGFX::VIEW* aView = NULL ): PNS::DEBUG_DECORATOR(),
        m_view( NULL ), m_items( NULL )
    {
        SetView( aView );
    }

    ~PNS_PCBNEW_DEBUG_DECORATOR()
    {
        Clear();
        delete m_items;
    }

    void SetView( KIGFX::VIEW* aView )
    {
        Clear();
        delete m_items;
        m_items = NULL;
        m_view = aView;

        if( m_view == NULL )
            return;

        m_items = new KIGFX::VIEW_GROUP( m_view );
        m_items->SetLayer( LAYER_GP_OVERLAY ) ;
        m_view->Add( m_items );
    }

    void AddPoint( VECTOR2I aP, int aColor ) override
    {
        SHAPE_LINE_CHAIN l;

        l.Append( aP - VECTOR2I( -50000, -50000 ) );
        l.Append( aP + VECTOR2I( -50000, -50000 ) );

        AddLine( l, aColor, 10000 );

        l.Clear();
        l.Append( aP - VECTOR2I( 50000, -50000 ) );
        l.Append( aP + VECTOR2I( 50000, -50000 ) );

        AddLine( l, aColor, 10000 );
    }

    void AddBox( BOX2I aB, int aColor ) override
    {
        SHAPE_LINE_CHAIN l;

        VECTOR2I o = aB.GetOrigin();
        VECTOR2I s = aB.GetSize();

        l.Append( o );
        l.Append( o.x + s.x, o.y );
        l.Append( o.x + s.x, o.y + s.y );
        l.Append( o.x, o.y + s.y );
        l.Append( o );

        AddLine( l, aColor, 10000 );
    }

    void AddSegment( SEG aS, int aColor ) override
    {
        SHAPE_LINE_CHAIN l;

        l.Append( aS.A );
        l.Append( aS.B );

        AddLine( l, aColor, 10000 );
    }

    void AddDirections( VECTOR2D aP, int aMask, int aColor ) override
    {
        BOX2I b( aP - VECTOR2I( 10000, 10000 ), VECTOR2I( 20000, 20000 ) );

        AddBox( b, aColor );
        for( int i = 0; i < 8; i++ )
        {
            if( ( 1 << i ) & aMask )
            {
                VECTOR2I v = DIRECTION_45( ( DIRECTION_45::Directions ) i ).ToVector() * 100000;
                AddSegment( SEG( aP, aP + v ), aColor );
            }
        }
    }

    void AddLine( const SHAPE_LINE_CHAIN& aLine, int aType, int aWidth ) override
    {
        ROUTER_PREVIEW_ITEM* pitem = new ROUTER_PREVIEW_ITEM( NULL, m_view );

        pitem->Line( aLine, aWidth, aType );
        m_items->Add( pitem ); // Should not be needed, as m_items has been passed as a parent group in alloc;
        m_view->Update( m_items );
    }

    void Clear() override
    {
        if( m_view && m_items )
        {
            m_items->FreeItems();
            m_view->Update( m_items );
        }
    }

private:
    KIGFX::VIEW* m_view;
    KIGFX::VIEW_GROUP* m_items;
};


PNS::DEBUG_DECORATOR* PNS_KICAD_IFACE::GetDebugDecorator()
{
    return m_debugDecorator;
}


PNS_KICAD_IFACE::PNS_KICAD_IFACE()
{
    m_ruleResolver = nullptr;
    m_board = nullptr;
    m_frame = nullptr;
    m_view = nullptr;
    m_previewItems = nullptr;
    m_world = nullptr;
    m_router = nullptr;
    m_debugDecorator = nullptr;
    m_dispOptions = nullptr;
}


PNS_KICAD_IFACE::~PNS_KICAD_IFACE()
{
    delete m_ruleResolver;
    delete m_debugDecorator;

    if( m_previewItems )
    {
        m_previewItems->FreeItems();
        delete m_previewItems;
    }
}


std::unique_ptr<PNS::SOLID> PNS_KICAD_IFACE::syncPad( D_PAD* aPad )
{
    LAYER_RANGE layers( 0, MAX_CU_LAYERS - 1 );

    // ignore non-copper pads
    if( ( aPad->GetLayerSet() & LSET::AllCuMask()).none() )
        return NULL;

    switch( aPad->GetAttribute() )
    {
    case PAD_ATTRIB_STANDARD:
        break;

    case PAD_ATTRIB_SMD:
    case PAD_ATTRIB_HOLE_NOT_PLATED:
    case PAD_ATTRIB_CONN:
        {
            LSET lmsk = aPad->GetLayerSet();
            bool is_copper = false;

            for( int i = 0; i < MAX_CU_LAYERS; i++ )
            {
                if( lmsk[i] )
                {
                    is_copper = true;

                    if( aPad->GetAttribute() != PAD_ATTRIB_HOLE_NOT_PLATED )
                        layers = LAYER_RANGE( i );

                    break;
                }
            }

            if( !is_copper )
                return NULL;
        }
        break;

    default:
        wxLogTrace( "PNS", "unsupported pad type 0x%x", aPad->GetAttribute() );
        return NULL;
    }

    std::unique_ptr< PNS::SOLID > solid( new PNS::SOLID );

    solid->SetLayers( layers );
    solid->SetNet( aPad->GetNetCode() );
    solid->SetParent( aPad );

    wxPoint wx_c = aPad->ShapePos();
    wxSize  wx_sz = aPad->GetSize();
    wxPoint offset = aPad->GetOffset();

    VECTOR2I c( wx_c.x, wx_c.y );
    VECTOR2I sz( wx_sz.x, wx_sz.y );

    RotatePoint( &offset, aPad->GetOrientation() );

    solid->SetPos( VECTOR2I( c.x - offset.x, c.y - offset.y ) );
    solid->SetOffset( VECTOR2I( offset.x, offset.y ) );

    double orient = aPad->GetOrientation() / 10.0;

    if( aPad->GetShape() == PAD_SHAPE_CIRCLE )
    {
        solid->SetShape( new SHAPE_CIRCLE( c, sz.x / 2 ) );
    }
    else
    {
        if( orient == 0.0 || orient == 90.0 || orient == 180.0 || orient == 270.0 )
        {
            if( orient == 90.0 || orient == 270.0 )
                sz = VECTOR2I( sz.y, sz.x );

            switch( aPad->GetShape() )
            {
            case PAD_SHAPE_OVAL:
                if( sz.x == sz.y )
                    solid->SetShape( new SHAPE_CIRCLE( c, sz.x / 2 ) );
                else
                {
                    VECTOR2I delta;

                    if( sz.x > sz.y )
                        delta = VECTOR2I( ( sz.x - sz.y ) / 2, 0 );
                    else
                        delta = VECTOR2I( 0, ( sz.y - sz.x ) / 2 );

                    SHAPE_SEGMENT* shape = new SHAPE_SEGMENT( c - delta, c + delta,
                                                              std::min( sz.x, sz.y ) );
                    solid->SetShape( shape );
                }
                break;

            case PAD_SHAPE_RECT:
                solid->SetShape( new SHAPE_RECT( c - sz / 2, sz.x, sz.y ) );
                break;

            case PAD_SHAPE_TRAPEZOID:
            {
                wxPoint coords[4];
                aPad->BuildPadPolygon( coords, wxSize( 0, 0 ), aPad->GetOrientation() );
                SHAPE_CONVEX* shape = new SHAPE_CONVEX();

                for( int ii = 0; ii < 4; ii++ )
                {
                    shape->Append( wx_c + coords[ii] );
                }

                solid->SetShape( shape );
                break;
            }

            case PAD_SHAPE_ROUNDRECT:
            {
                SHAPE_POLY_SET outline;
                const int segmentToCircleCount = 64;

                aPad->BuildPadShapePolygon( outline, wxSize( 0, 0 ), segmentToCircleCount, 1.0 );

                // TransformRoundRectToPolygon creates only one convex polygon
                SHAPE_LINE_CHAIN& poly = outline.Outline( 0 );
                SHAPE_CONVEX* shape = new SHAPE_CONVEX();

                for( int ii = 0; ii < poly.PointCount(); ++ii )
                {
                    shape->Append( wxPoint( poly.Point( ii ).x, poly.Point( ii ).y ) );
                }

                solid->SetShape( shape );
            }
                break;

            default:
                wxLogTrace( "PNS", "unsupported pad shape" );
                return nullptr;
            }
        }
        else
        {
            switch( aPad->GetShape() )
            {
            // PAD_SHAPE_CIRCLE already handled above

            case PAD_SHAPE_OVAL:
                if( sz.x == sz.y )
                    solid->SetShape( new SHAPE_CIRCLE( c, sz.x / 2 ) );
                else
                {
                    wxPoint start;
                    wxPoint end;
                    wxPoint corner;

                    SHAPE_CONVEX* shape = new SHAPE_CONVEX();

                    int w = aPad->BuildSegmentFromOvalShape( start, end, 0.0, wxSize( 0, 0 ) );

                    if( start.y == 0 )
                        corner = wxPoint( start.x, -( w / 2 ) );
                    else
                        corner = wxPoint( w / 2, start.y );

                    RotatePoint( &start, aPad->GetOrientation() );
                    RotatePoint( &corner, aPad->GetOrientation() );
                    shape->Append( wx_c + corner );

                    for( int rot = 100; rot <= 1800; rot += 100 )
                    {
                        wxPoint p( corner );
                        RotatePoint( &p, start, rot );
                        shape->Append( wx_c + p );
                    }

                    if( end.y == 0 )
                        corner = wxPoint( end.x, w / 2 );
                    else
                        corner = wxPoint( -( w / 2 ), end.y );

                    RotatePoint( &end, aPad->GetOrientation() );
                    RotatePoint( &corner, aPad->GetOrientation() );
                    shape->Append( wx_c + corner );

                    for( int rot = 100; rot <= 1800; rot += 100 )
                    {
                        wxPoint p( corner );
                        RotatePoint( &p, end, rot );
                        shape->Append( wx_c + p );
                    }

                    solid->SetShape( shape );
                }
                break;

            case PAD_SHAPE_RECT:
            case PAD_SHAPE_TRAPEZOID:
            {
                wxPoint coords[4];
                aPad->BuildPadPolygon( coords, wxSize( 0, 0 ), aPad->GetOrientation() );

                SHAPE_CONVEX* shape = new SHAPE_CONVEX();
                for( int ii = 0; ii < 4; ii++ )
                {
                    shape->Append( wx_c + coords[ii] );
                }

                solid->SetShape( shape );
                break;
            }

            case PAD_SHAPE_ROUNDRECT:
            {
                SHAPE_POLY_SET outline;
                const int segmentToCircleCount = 32;
                aPad->BuildPadShapePolygon( outline, wxSize( 0, 0 ),
                                            segmentToCircleCount, 1.0 );

                // TransformRoundRectToPolygon creates only one convex polygon
                SHAPE_LINE_CHAIN& poly = outline.Outline( 0 );
                SHAPE_CONVEX* shape = new SHAPE_CONVEX();

                for( int ii = 0; ii < poly.PointCount(); ++ii )
                {
                    shape->Append( wxPoint( poly.Point( ii ).x, poly.Point( ii ).y ) );
                }

                solid->SetShape( shape );
                break;
            }

            default:
                wxLogTrace( "PNS", "unsupported pad shape" );
                return nullptr;
            }
        }
    }
    return solid;
}


std::unique_ptr<PNS::SEGMENT> PNS_KICAD_IFACE::syncTrack( TRACK* aTrack )
{
    std::unique_ptr< PNS::SEGMENT > segment(
        new PNS::SEGMENT( SEG( aTrack->GetStart(), aTrack->GetEnd() ), aTrack->GetNetCode() )
    );

    segment->SetWidth( aTrack->GetWidth() );
    segment->SetLayers( LAYER_RANGE( aTrack->GetLayer() ) );
    segment->SetParent( aTrack );

    if( aTrack->IsLocked() )
        segment->Mark( PNS::MK_LOCKED );

    return segment;
}


std::unique_ptr<PNS::VIA> PNS_KICAD_IFACE::syncVia( VIA* aVia )
{
    PCB_LAYER_ID top, bottom;
    aVia->LayerPair( &top, &bottom );
    std::unique_ptr<PNS::VIA> via( new PNS::VIA(
            aVia->GetPosition(),
            LAYER_RANGE( top, bottom ),
            aVia->GetWidth(),
            aVia->GetDrillValue(),
            aVia->GetNetCode(),
            aVia->GetViaType() )
    );

    via->SetParent( aVia );

    if( aVia->IsLocked() )
        via->Mark( PNS::MK_LOCKED );

    return via;
}


void PNS_KICAD_IFACE::SetBoard( BOARD* aBoard )
{
    m_board = aBoard;
    wxLogTrace( "PNS", "m_board = %p", m_board );
}


void PNS_KICAD_IFACE::SyncWorld( PNS::NODE *aWorld )
{
    if( !m_board )
    {
        wxLogTrace( "PNS", "No board attached, aborting sync." );
        return;
    }

    for( MODULE* module = m_board->m_Modules; module; module = module->Next() )
    {
        for( D_PAD* pad = module->Pads(); pad; pad = pad->Next() )
        {
            std::unique_ptr< PNS::SOLID > solid = syncPad( pad );

            if( solid )
                aWorld->Add( std::move( solid ) );
        }
    }

    for( TRACK* t = m_board->m_Track; t; t = t->Next() )
    {
        KICAD_T type = t->Type();

        if( type == PCB_TRACE_T ) {
            std::unique_ptr< PNS::SEGMENT > segment = syncTrack( t );
            if( segment ) {
                aWorld->Add( std::move( segment ) );
            }
        } else if( type == PCB_VIA_T ) {
            std::unique_ptr< PNS::VIA > via = syncVia( static_cast<VIA*>( t ) );
            if( via ) {
                aWorld->Add( std::move( via ) );
            }
        }
    }

    int worstClearance = m_board->GetDesignSettings().GetBiggestClearanceValue();

    delete m_ruleResolver;
    m_ruleResolver = new PNS_PCBNEW_RULE_RESOLVER( m_board, m_router );

    aWorld->SetRuleResolver( m_ruleResolver );
    aWorld->SetMaxClearance( 4 * worstClearance );
}


void PNS_KICAD_IFACE::EraseView()
{
    for( auto item : m_hiddenItems )
        m_view->SetVisible( item, true );

    m_hiddenItems.clear();

    if( m_previewItems )
    {
        m_previewItems->FreeItems();
        m_view->Update( m_previewItems );
    }

    if( m_debugDecorator )
        m_debugDecorator->Clear();
}


void PNS_KICAD_IFACE::DisplayItem( const PNS::ITEM* aItem, int aColor, int aClearance )
{
    wxLogTrace( "PNS", "DisplayItem %p", aItem );

    ROUTER_PREVIEW_ITEM* pitem = new ROUTER_PREVIEW_ITEM( aItem, m_view );

    if( aColor >= 0 )
        pitem->SetColor( KIGFX::COLOR4D( aColor ) );

    if( aClearance >= 0 )
    {
        pitem->SetClearance( aClearance );

        if( m_dispOptions )
        {
            auto clearanceDisp = m_dispOptions->m_ShowTrackClearanceMode;
            pitem->ShowTrackClearance( clearanceDisp != DO_NOT_SHOW_CLEARANCE );
            pitem->ShowViaClearance( clearanceDisp != DO_NOT_SHOW_CLEARANCE
                    && clearanceDisp != SHOW_CLEARANCE_NEW_TRACKS );
        }
    }


    m_previewItems->Add( pitem );
    m_view->Update( m_previewItems );
}


void PNS_KICAD_IFACE::HideItem( PNS::ITEM* aItem )
{
    BOARD_CONNECTED_ITEM* parent = aItem->Parent();

    if( parent )
    {
        if( m_view->IsVisible( parent ) )
            m_hiddenItems.insert( parent );

        m_view->SetVisible( parent, false );
        m_view->Update( parent, KIGFX::APPEARANCE );
    }
}


void PNS_KICAD_IFACE::RemoveItem( PNS::ITEM* aItem )
{
    BOARD_CONNECTED_ITEM* parent = aItem->Parent();

    if( parent )
    {
        m_commit->Remove( parent );
    }
}


void PNS_KICAD_IFACE::AddItem( PNS::ITEM* aItem )
{
    BOARD_CONNECTED_ITEM* newBI = NULL;

    switch( aItem->Kind() )
    {
    case PNS::ITEM::SEGMENT_T:
    {
        PNS::SEGMENT* seg = static_cast<PNS::SEGMENT*>( aItem );
        TRACK* track = new TRACK( m_board );
        const SEG& s = seg->Seg();
        track->SetStart( wxPoint( s.A.x, s.A.y ) );
        track->SetEnd( wxPoint( s.B.x, s.B.y ) );
        track->SetWidth( seg->Width() );
        track->SetLayer( ToLAYER_ID( seg->Layers().Start() ) );
        track->SetNetCode( seg->Net() > 0 ? seg->Net() : 0 );
        newBI = track;
        break;
    }

    case PNS::ITEM::VIA_T:
    {
        VIA* via_board = new VIA( m_board );
        PNS::VIA* via = static_cast<PNS::VIA*>( aItem );
        via_board->SetPosition( wxPoint( via->Pos().x, via->Pos().y ) );
        via_board->SetWidth( via->Diameter() );
        via_board->SetDrill( via->Drill() );
        via_board->SetNetCode( via->Net() > 0 ? via->Net() : 0 );
        via_board->SetViaType( via->ViaType() ); // MUST be before SetLayerPair()
        via_board->SetLayerPair( ToLAYER_ID( via->Layers().Start() ),
                                 ToLAYER_ID( via->Layers().End() ) );
        newBI = via_board;
        break;
    }

    default:
        break;
    }

    if( newBI )
    {
        aItem->SetParent( newBI );
        newBI->ClearFlags();

        m_commit->Add( newBI );
    }
}


void PNS_KICAD_IFACE::Commit()
{
    EraseView();
    m_commit->Push( wxT( "Added a track" ) );
    m_commit.reset( new BOARD_COMMIT( m_frame ) );
}


void PNS_KICAD_IFACE::SetView( KIGFX::VIEW* aView )
{
    wxLogTrace( "PNS", "SetView %p", aView );

    if( m_previewItems )
    {
        m_previewItems->FreeItems();
        delete m_previewItems;
    }

    m_view = aView;
    m_previewItems = new KIGFX::VIEW_GROUP( m_view );
    m_previewItems->SetLayer( LAYER_GP_OVERLAY ) ;
    m_view->Add( m_previewItems );

    delete m_debugDecorator;
    m_debugDecorator = new PNS_PCBNEW_DEBUG_DECORATOR();
    m_debugDecorator->SetView( m_view );
}


void PNS_KICAD_IFACE::UpdateNet( int aNetCode )
{
    wxLogTrace( "PNS", "Update-net %d", aNetCode );
}


PNS::RULE_RESOLVER* PNS_KICAD_IFACE::GetRuleResolver()
{
    return m_ruleResolver;
}


void PNS_KICAD_IFACE::SetRouter( PNS::ROUTER* aRouter )
{
    m_router = aRouter;
}


void PNS_KICAD_IFACE::SetHostFrame( PCB_EDIT_FRAME* aFrame )
{
    m_frame = aFrame;

    m_commit.reset( new BOARD_COMMIT( m_frame ) );
    m_dispOptions = (DISPLAY_OPTIONS*) m_frame->GetDisplayOptions();
}
