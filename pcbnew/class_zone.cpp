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

/**
 * @file class_zone.cpp
 * @brief Implementation of class to handle copper zones.
 */

#include <fctsys.h>
#include <wxstruct.h>
#include <trigo.h>
#include <class_pcb_screen.h>
#include <class_drawpanel.h>
#include <kicad_string.h>
#include <pcbcommon.h>
#include <colors_selection.h>
#include <richio.h>
#include <macros.h>
#include <wxBasePcbFrame.h>
#include <msgpanel.h>

#include <protos.h>
#include <class_board.h>
#include <class_zone.h>

#include <pcbnew.h>
#include <zones.h>
#include <math_for_graphics.h>
#include <polygon_test_point_inside.h>


ZONE_CONTAINER::ZONE_CONTAINER( BOARD* aBoard ) :
    BOARD_CONNECTED_ITEM( aBoard, PCB_ZONE_AREA_T )
{
    SetNet( -1 );                               // Net number for fast comparisons
    m_CornerSelection = -1;
    m_IsFilled = false;                         // fill status : true when the zone is filled
    m_FillMode = 0;                             // How to fill areas: 0 = use filled polygons, != 0 fill with segments
    m_priority = 0;
    m_smoothedPoly = NULL;
    m_cornerSmoothingType = ZONE_SETTINGS::SMOOTHING_NONE;
    SetIsKeepout( false );
    SetDoNotAllowCopperPour( false );           // has meaning only if m_isKeepout == true
    SetDoNotAllowVias( true );                  // has meaning only if m_isKeepout == true
    SetDoNotAllowTracks( true );                // has meaning only if m_isKeepout == true
    m_cornerRadius = 0;
    utility    = 0;                             // flags used in polygon calculations
    m_Poly     = new CPolyLine();               // Outlines
    aBoard->GetZoneSettings().ExportSetting( *this );
}


ZONE_CONTAINER::ZONE_CONTAINER( const ZONE_CONTAINER& aZone ) :
    BOARD_CONNECTED_ITEM( aZone )
{
    // Should the copy be on the same net?
    SetNet( aZone.GetNet() );
    m_Poly = new CPolyLine( *aZone.m_Poly );

    // For corner moving, corner index to drag, or -1 if no selection
    m_CornerSelection = -1;
    m_IsFilled = aZone.m_IsFilled;
    m_ZoneClearance = aZone.m_ZoneClearance;     // clearance value
    m_ZoneMinThickness = aZone.m_ZoneMinThickness;
    m_FillMode = aZone.m_FillMode;               // Filling mode (segments/polygons)
    m_priority = aZone.m_priority;
    m_ArcToSegmentsCount = aZone.m_ArcToSegmentsCount;
    m_PadConnection = aZone.m_PadConnection;
    m_ThermalReliefGap = aZone.m_ThermalReliefGap;
    m_ThermalReliefCopperBridge = aZone.m_ThermalReliefCopperBridge;
    m_FilledPolysList = aZone.m_FilledPolysList;
    m_FillSegmList = aZone.m_FillSegmList;

    m_isKeepout = aZone.m_isKeepout;
    m_doNotAllowCopperPour = aZone.m_doNotAllowCopperPour;
    m_doNotAllowVias = aZone.m_doNotAllowVias;
    m_doNotAllowTracks = aZone.m_doNotAllowTracks;

    m_cornerSmoothingType = aZone.m_cornerSmoothingType;
    m_cornerRadius = aZone.m_cornerRadius;


    utility    = aZone.utility;
}


ZONE_CONTAINER::~ZONE_CONTAINER()
{
    delete m_Poly;
    m_Poly = NULL;
}


EDA_ITEM* ZONE_CONTAINER::Clone() const
{
    return new ZONE_CONTAINER( *this );
}


bool ZONE_CONTAINER::UnFill()
{
    bool change = ( m_FilledPolysList.size() > 0 ) || ( m_FillSegmList.size() > 0 );

    m_FilledPolysList.clear();
    m_FillSegmList.clear();
    m_IsFilled = false;

    return change;
}


const wxPoint& ZONE_CONTAINER::GetPosition() const
{
    static const wxPoint dummy;

    return m_Poly ? GetCornerPosition( 0 ) : dummy;
}


void ZONE_CONTAINER::SetNet( int aNetCode )
{
    BOARD_CONNECTED_ITEM::SetNet( aNetCode );

    if( aNetCode < 0 )
        return;

    BOARD* board = GetBoard();

    if( board )
    {
        NETINFO_ITEM* net = board->FindNet( aNetCode );

        if( net )
            m_Netname = net->GetNetname();
        else
            m_Netname.Empty();
    }
    else
    {
        m_Netname.Empty();
    }
}


void ZONE_CONTAINER::Draw( EDA_DRAW_PANEL* panel, wxDC* DC, GR_DRAWMODE aDrawMode, const wxPoint& offset )
{
    if( DC == NULL )
        return;

    wxPoint seg_start, seg_end;
    int     curr_layer = ( (PCB_SCREEN*) panel->GetScreen() )->m_Active_Layer;

    BOARD*  brd   = GetBoard();
    EDA_COLOR_T color = brd->GetLayerColor( m_Layer );

    if( brd->IsLayerVisible( m_Layer ) == false && ( color & HIGHLIGHT_FLAG ) != HIGHLIGHT_FLAG )
        return;

    GRSetDrawMode( DC, aDrawMode );

    if( DisplayOpt.ContrastModeDisplay )
    {
        if( !IsOnLayer( curr_layer ) )
            ColorTurnToDarkDarkGray( &color );
    }

    if( aDrawMode & GR_HIGHLIGHT )
        ColorChangeHighlightFlag( &color, !(aDrawMode & GR_AND) );

    if( color & HIGHLIGHT_FLAG )
        color = ColorRefs[color & MASKCOLOR].m_LightColor;

    SetAlpha( &color, 150 );

    // draw the lines
    int i_start_contour = 0;
    std::vector<wxPoint> lines;
    lines.reserve( (GetNumCorners() * 2) + 2 );

    for( int ic = 0; ic < GetNumCorners(); ic++ )
    {
        seg_start = GetCornerPosition( ic ) + offset;

        if( m_Poly->m_CornersList[ic].end_contour == false && ic < GetNumCorners() - 1 )
        {
            seg_end = GetCornerPosition( ic + 1 ) + offset;
        }
        else
        {
            seg_end = GetCornerPosition( i_start_contour ) + offset;
            i_start_contour = ic + 1;
        }

        lines.push_back( seg_start );
        lines.push_back( seg_end );
    }

    GRLineArray( panel->GetClipBox(), DC, lines, 0, color );

    // draw hatches
    lines.clear();
    lines.reserve( (m_Poly->m_HatchLines.size() * 2) + 2 );

    for( unsigned ic = 0; ic < m_Poly->m_HatchLines.size(); ic++ )
    {
        seg_start = m_Poly->m_HatchLines[ic].m_Start + offset;
        seg_end   = m_Poly->m_HatchLines[ic].m_End + offset;
        lines.push_back( seg_start );
        lines.push_back( seg_end );
    }

    GRLineArray( panel->GetClipBox(), DC, lines, 0, color );
}


void ZONE_CONTAINER::DrawFilledArea( EDA_DRAW_PANEL* panel,
                                     wxDC* DC, GR_DRAWMODE aDrawMode, const wxPoint& offset )
{
    static std::vector <char>    CornersTypeBuffer;
    static std::vector <wxPoint> CornersBuffer;

    // outline_mode is false to show filled polys,
    // and true to show polygons outlines only (test and debug purposes)
    bool outline_mode = DisplayOpt.DisplayZonesMode == 2 ? true : false;

    if( DC == NULL )
        return;

    if( DisplayOpt.DisplayZonesMode == 1 )     // Do not show filled areas
        return;

    if( m_FilledPolysList.size() == 0 )  // Nothing to draw
        return;

    BOARD* brd = GetBoard();
    int    curr_layer = ( (PCB_SCREEN*) panel->GetScreen() )->m_Active_Layer;
    EDA_COLOR_T color = brd->GetLayerColor( m_Layer );

    if( brd->IsLayerVisible( m_Layer ) == false && ( color & HIGHLIGHT_FLAG ) != HIGHLIGHT_FLAG )
        return;

    GRSetDrawMode( DC, aDrawMode );

    if( DisplayOpt.ContrastModeDisplay )
    {
        if( !IsOnLayer( curr_layer ) )
            ColorTurnToDarkDarkGray( &color );
    }

    if( aDrawMode & GR_HIGHLIGHT )
        ColorChangeHighlightFlag( &color, !(aDrawMode & GR_AND) );

    if( color & HIGHLIGHT_FLAG )
        color = ColorRefs[color & MASKCOLOR].m_LightColor;

    SetAlpha( &color, 150 );

    CornersTypeBuffer.clear();
    CornersBuffer.clear();

    // Draw all filled areas
    int imax = m_FilledPolysList.size() - 1;

    for( int ic = 0; ic <= imax; ic++ )
    {
        CPolyPt* corner = &m_FilledPolysList[ic];

        wxPoint  coord( corner->x + offset.x, corner->y + offset.y );

        CornersBuffer.push_back( coord );

        CornersTypeBuffer.push_back( (char) corner->m_utility );

        if( (corner->end_contour) || (ic == imax) ) // the last corner of a filled area is found: draw it
        {
            /* Draw the current filled area: draw segments outline first
             * Curiously, draw segments outline first and after draw filled polygons
             * with outlines thickness = 0 is a faster than
             * just draw filled polygons but with outlines thickness = m_ZoneMinThickness
             * So DO NOT use draw filled polygons with outlines having a thickness  > 0
             * Note: Extra segments ( added to joint holes with external outline) flagged by
             * m_utility != 0 are not drawn
             * Note not all polygon libraries provide a flag for these extra-segments, therefore
             * the m_utility member can be always 0
             */
            {
                // Draw outlines:
                if( (m_ZoneMinThickness > 1) || outline_mode )
                {
                    int ilim = CornersBuffer.size() - 1;

                    for(  int is = 0, ie = ilim; is <= ilim; ie = is, is++ )
                    {
                        int x0 = CornersBuffer[is].x;
                        int y0 = CornersBuffer[is].y;
                        int x1 = CornersBuffer[ie].x;
                        int y1 = CornersBuffer[ie].y;

                        if( CornersTypeBuffer[ie] == 0 )   // Draw only basic outlines, not extra segments
                        {
                            if( !DisplayOpt.DisplayPcbTrackFill || GetState( FORCE_SKETCH ) )
                                GRCSegm( panel->GetClipBox(), DC,
                                         x0, y0, x1, y1,
                                         m_ZoneMinThickness, color );
                            else
                                GRFillCSegm( panel->GetClipBox(), DC,
                                             x0, y0, x1, y1,
                                             m_ZoneMinThickness, color );
                        }
                    }
                }

                // Draw areas:
                if( m_FillMode==0  && !outline_mode )
                    GRPoly( panel->GetClipBox(), DC, CornersBuffer.size(), &CornersBuffer[0],
                            true, 0, color, color );
            }

            CornersTypeBuffer.clear();
            CornersBuffer.clear();
        }
    }

    if( m_FillMode == 1  && !outline_mode )     // filled with segments
    {
        for( unsigned ic = 0; ic < m_FillSegmList.size(); ic++ )
        {
            wxPoint start = m_FillSegmList[ic].m_Start + offset;
            wxPoint end   = m_FillSegmList[ic].m_End + offset;

            if( !DisplayOpt.DisplayPcbTrackFill || GetState( FORCE_SKETCH ) )
                GRCSegm( panel->GetClipBox(), DC, start.x, start.y, end.x, end.y,
                         m_ZoneMinThickness, color );
            else
                GRFillCSegm( panel->GetClipBox(), DC, start.x, start.y, end.x, end.y,
                             m_ZoneMinThickness, color );
        }
    }
}


EDA_RECT ZONE_CONTAINER::GetBoundingBox() const
{
    const int PRELOAD = 0x7FFFFFFF;     // Biggest integer (32 bits)

    int       ymax = -PRELOAD;
    int       ymin = PRELOAD;
    int       xmin = PRELOAD;
    int       xmax = -PRELOAD;

    int       count = GetNumCorners();

    for( int i = 0; i<count; ++i )
    {
        wxPoint corner = GetCornerPosition( i );

        ymax = std::max( ymax, corner.y );
        xmax = std::max( xmax, corner.x );
        ymin = std::min( ymin, corner.y );
        xmin = std::min( xmin, corner.x );
    }

    EDA_RECT ret( wxPoint( xmin, ymin ), wxSize( xmax - xmin + 1, ymax - ymin + 1 ) );

    return ret;
}


void ZONE_CONTAINER::DrawWhileCreateOutline( EDA_DRAW_PANEL* panel, wxDC* DC,
                                             GR_DRAWMODE draw_mode )
{
    GR_DRAWMODE current_gr_mode  = draw_mode;
    bool    is_close_segment = false;
    wxPoint seg_start, seg_end;

    if( DC == NULL )
        return;

    int    curr_layer = ( (PCB_SCREEN*) panel->GetScreen() )->m_Active_Layer;
    BOARD* brd   = GetBoard();
    EDA_COLOR_T color = brd->GetLayerColor( m_Layer );

    if( DisplayOpt.ContrastModeDisplay )
    {
        if( !IsOnLayer( curr_layer ) )
            ColorTurnToDarkDarkGray( &color );
    }

    // draw the lines
    wxPoint start_contour_pos = GetCornerPosition( 0 );
    int     icmax = GetNumCorners() - 1;

    for( int ic = 0; ic <= icmax; ic++ )
    {
        int xi = GetCornerPosition( ic ).x;
        int yi = GetCornerPosition( ic ).y;
        int xf, yf;

        if( m_Poly->m_CornersList[ic].end_contour == false && ic < icmax )
        {
            is_close_segment = false;
            xf = GetCornerPosition( ic + 1 ).x;
            yf = GetCornerPosition( ic + 1 ).y;

            if( (m_Poly->m_CornersList[ic + 1].end_contour) || (ic == icmax - 1) )
                current_gr_mode = GR_XOR;
            else
                current_gr_mode = draw_mode;
        }
        else    // Draw the line from last corner to the first corner of the current contour
        {
            is_close_segment = true;
            current_gr_mode  = GR_XOR;
            xf = start_contour_pos.x;
            yf = start_contour_pos.y;

            // Prepare the next contour for drawing, if exists
            if( ic < icmax )
                start_contour_pos = GetCornerPosition( ic + 1 );
        }

        GRSetDrawMode( DC, current_gr_mode );

        if( is_close_segment )
            GRLine( panel->GetClipBox(), DC, xi, yi, xf, yf, 0, WHITE );
        else
            GRLine( panel->GetClipBox(), DC, xi, yi, xf, yf, 0, color );
    }
}


int ZONE_CONTAINER::GetThermalReliefGap( D_PAD* aPad ) const
{
    if( aPad == NULL || aPad->GetThermalGap() == 0 )
        return m_ThermalReliefGap;
    else
        return aPad->GetThermalGap();
}


int ZONE_CONTAINER::GetThermalReliefCopperBridge( D_PAD* aPad ) const
{
    if( aPad == NULL || aPad->GetThermalWidth() == 0 )
        return m_ThermalReliefCopperBridge;
    else
        return aPad->GetThermalWidth();
}


bool ZONE_CONTAINER::HitTest( const wxPoint& aPosition )
{
    if( HitTestForCorner( aPosition ) )
        return true;

    if( HitTestForEdge( aPosition ) )
        return true;

    return false;
}

// Zones outlines have no thickness, so it Hit Test functions
// we must have a default distance between the test point
// and a corner or a zone edge:
#define MIN_DIST_IN_MILS 10

bool ZONE_CONTAINER::HitTestForCorner( const wxPoint& refPos )
{
    m_CornerSelection = -1;         // Set to not found

    // distance (in internal units) to detect a corner in a zone outline.
    int min_dist = MIN_DIST_IN_MILS*IU_PER_MILS;

    wxPoint delta;
    unsigned lim = m_Poly->m_CornersList.size();

    for( unsigned item_pos = 0; item_pos < lim; item_pos++ )
    {
        delta.x = refPos.x - m_Poly->m_CornersList[item_pos].x;
        delta.y = refPos.y - m_Poly->m_CornersList[item_pos].y;

        // Calculate a distance:
        int dist = std::max( abs( delta.x ), abs( delta.y ) );

        if( dist < min_dist )  // this corner is a candidate:
        {
            m_CornerSelection = item_pos;
            min_dist = dist;
        }
    }

    return m_CornerSelection >= 0;
}


bool ZONE_CONTAINER::HitTestForEdge( const wxPoint& refPos )
{
    unsigned lim = m_Poly->m_CornersList.size();

    m_CornerSelection = -1;     // Set to not found

    // distance (in internal units) to detect a zone outline
    int min_dist = MIN_DIST_IN_MILS*IU_PER_MILS;

    unsigned first_corner_pos = 0;

    for( unsigned item_pos = 0; item_pos < lim; item_pos++ )
    {
        unsigned end_segm = item_pos + 1;

        /* the last corner of the current outline is tested
         * the last segment of the current outline starts at current corner, and ends
         * at the first corner of the outline
         */
        if( m_Poly->m_CornersList[item_pos].end_contour || end_segm >= lim )
        {
            unsigned tmp = first_corner_pos;
            first_corner_pos = end_segm;    // first_corner_pos is now the beginning of the next outline
            end_segm = tmp;                 // end_segm is the beginning of the current outline
        }

        /* test the dist between segment and ref point */
        int dist = (int) GetPointToLineSegmentDistance( refPos.x,
                                                        refPos.y,
                                                        m_Poly->m_CornersList[item_pos].x,
                                                        m_Poly->m_CornersList[item_pos].y,
                                                        m_Poly->m_CornersList[end_segm].x,
                                                        m_Poly->m_CornersList[end_segm].y );

        if( dist < min_dist )
        {
            m_CornerSelection = item_pos;
            min_dist = dist;
        }
    }

    return m_CornerSelection >= 0;
}


bool ZONE_CONTAINER::HitTest( const EDA_RECT& aRect ) const
{
    bool  is_out_of_box = false;

    CRect rect = m_Poly->GetCornerBounds();

    if( rect.left < aRect.GetX() )
        is_out_of_box = true;

    if( rect.top < aRect.GetY() )
        is_out_of_box = true;

    if( rect.right > aRect.GetRight() )
        is_out_of_box = true;

    if( rect.bottom > aRect.GetBottom() )
        is_out_of_box = true;

    return is_out_of_box ? false : true;
}


int ZONE_CONTAINER::GetClearance( BOARD_CONNECTED_ITEM* aItem ) const
{
    int         myClearance = m_ZoneClearance;

#if 0   // Maybe the netclass clearance should not come into play for a zone?
        // At least the policy decision can be controlled by the zone
        // itself, i.e. here.  On reasons of insufficient documentation,
        // the user will be less bewildered if we simply respect the
        // "zone clearance" setting in the zone properties dialog.  (At least
        // until there is a UI boolean for this.)

    NETCLASS*   myClass  = GetNetClass();

    if( myClass )
        myClearance = std::max( myClearance, myClass->GetClearance() );
#endif

    if( aItem )
    {
        int hisClearance = aItem->GetClearance( NULL );
        myClearance = std::max( hisClearance, myClearance );
    }

    return myClearance;
}


bool ZONE_CONTAINER::HitTestFilledArea( const wxPoint& aRefPos ) const
{
    unsigned indexstart = 0, indexend;
    bool     inside     = false;

    for( indexend = 0; indexend < m_FilledPolysList.size(); indexend++ )
    {
        if( m_FilledPolysList[indexend].end_contour )       // end of a filled sub-area found
        {
            if( TestPointInsidePolygon( m_FilledPolysList, indexstart, indexend, aRefPos.x,
                                        aRefPos.y ) )
            {
                inside = true;
                break;
            }

            // Prepare test of next area which starts after the current index end (if exists)
            indexstart = indexend + 1;
        }
    }

    return inside;
}


void ZONE_CONTAINER::GetMsgPanelInfo( std::vector< MSG_PANEL_ITEM >& aList )
{
    wxString msg;

    BOARD*   board = (BOARD*) m_Parent;

    wxASSERT( board );

    msg = _( "Zone Outline" );

    // Display Cutout instead of Outline for holes inside a zone
    // i.e. when num contour !=0
    int ncont = m_Poly->GetContour( m_CornerSelection );

    if( ncont )
        msg << wxT( " " ) << _( "(Cutout)" );

    aList.push_back( MSG_PANEL_ITEM( _( "Type" ), msg, DARKCYAN ) );

    if( GetIsKeepout() )
    {
        msg.Empty();

        if( GetDoNotAllowVias() )
            msg = _("No via");

        if( GetDoNotAllowTracks() )
        {
            if( !msg.IsEmpty() )
                msg += wxT(", ");

            msg += _("No track");
        }

        if( GetDoNotAllowCopperPour() )
        {
            if( !msg.IsEmpty() )
                msg += wxT(", ");

            msg += _("No copper pour");
        }

        aList.push_back( MSG_PANEL_ITEM( _( "Keepout" ), msg, RED ) );
    }
    else if( IsOnCopperLayer() )
    {
        if( GetNet() >= 0 )
        {
            NETINFO_ITEM* equipot = board->FindNet( GetNet() );

            if( equipot )
                msg = equipot->GetNetname();
            else
                msg = wxT( "<noname>" );
        }
        else // a netcode < 0 is an error
        {
            msg = wxT( " [" );
            msg << m_Netname + wxT( "]" );
            msg << wxT( " <" ) << _( "Not Found" ) << wxT( ">" );
        }

        aList.push_back( MSG_PANEL_ITEM( _( "NetName" ), msg, RED ) );

#if 1
        // Display net code : (useful in test or debug)
        msg.Printf( wxT( "%d" ), GetNet() );
        aList.push_back( MSG_PANEL_ITEM( _( "NetCode" ), msg, RED ) );
#endif

        // Display priority level
        msg.Printf( wxT( "%d" ), GetPriority() );
        aList.push_back( MSG_PANEL_ITEM( _( "Priority" ), msg, BLUE ) );
    }
    else
    {
        aList.push_back( MSG_PANEL_ITEM( _( "Non Copper Zone" ), wxEmptyString, RED ) );
    }

    msg = board->GetLayerName( m_Layer );
    aList.push_back( MSG_PANEL_ITEM( _( "Layer" ), msg, BROWN ) );

    msg.Printf( wxT( "%d" ), (int) m_Poly->m_CornersList.size() );
    aList.push_back( MSG_PANEL_ITEM( _( "Corners" ), msg, BLUE ) );

    if( m_FillMode )
        msg = _( "Segments" );
    else
        msg = _( "Polygons" );

    aList.push_back( MSG_PANEL_ITEM( _( "Fill mode" ), msg, BROWN ) );

    // Useful for statistics :
    msg.Printf( wxT( "%d" ), (int) m_Poly->m_HatchLines.size() );
    aList.push_back( MSG_PANEL_ITEM( _( "Hatch lines" ), msg, BLUE ) );

    if( m_FilledPolysList.size() )
    {
        msg.Printf( wxT( "%d" ), (int) m_FilledPolysList.size() );
        aList.push_back( MSG_PANEL_ITEM( _( "Corners in DrawList" ), msg, BLUE ) );
    }
}


/* Geometric transforms: */

void ZONE_CONTAINER::Move( const wxPoint& offset )
{
    /* move outlines */
    for( unsigned ii = 0; ii < m_Poly->m_CornersList.size(); ii++ )
    {
        SetCornerPosition( ii, GetCornerPosition( ii ) + offset );
    }

    m_Poly->Hatch();

    /* move filled areas: */
    for( unsigned ic = 0; ic < m_FilledPolysList.size(); ic++ )
    {
        CPolyPt* corner = &m_FilledPolysList[ic];
        corner->x += offset.x;
        corner->y += offset.y;
    }

    for( unsigned ic = 0; ic < m_FillSegmList.size(); ic++ )
    {
        m_FillSegmList[ic].m_Start += offset;
        m_FillSegmList[ic].m_End   += offset;
    }
}


void ZONE_CONTAINER::MoveEdge( const wxPoint& offset )
{
    int ii = m_CornerSelection;

    // Move the start point of the selected edge:
    SetCornerPosition( ii, GetCornerPosition( ii ) + offset );

    // Move the end point of the selected edge:
    if( m_Poly->m_CornersList[ii].end_contour || ii == GetNumCorners() - 1 )
    {
        int icont = m_Poly->GetContour( ii );
        ii = m_Poly->GetContourStart( icont );
    }
    else
    {
        ii++;
    }

    SetCornerPosition( ii, GetCornerPosition( ii ) + offset );

    m_Poly->Hatch();
}


void ZONE_CONTAINER::Rotate( const wxPoint& centre, double angle )
{
    wxPoint pos;

    for( unsigned ii = 0; ii < m_Poly->m_CornersList.size(); ii++ )
    {
        pos.x = m_Poly->m_CornersList[ii].x;
        pos.y = m_Poly->m_CornersList[ii].y;
        RotatePoint( &pos, centre, angle );
        m_Poly->m_CornersList[ii].x = pos.x;
        m_Poly->m_CornersList[ii].y = pos.y;
    }

    m_Poly->Hatch();

    /* rotate filled areas: */
    for( unsigned ic = 0; ic < m_FilledPolysList.size(); ic++ )
    {
        CPolyPt* corner = &m_FilledPolysList[ic];
        pos.x = corner->x;
        pos.y = corner->y;
        RotatePoint( &pos, centre, angle );
        corner->x = pos.x;
        corner->y = pos.y;
    }

    for( unsigned ic = 0; ic < m_FillSegmList.size(); ic++ )
    {
        RotatePoint( &m_FillSegmList[ic].m_Start, centre, angle );
        RotatePoint( &m_FillSegmList[ic].m_End, centre, angle );
    }
}


void ZONE_CONTAINER::Flip( const wxPoint& aCentre )
{
    Mirror( aCentre );
    SetLayer( BOARD::ReturnFlippedLayerNumber( GetLayer() ) );
}


void ZONE_CONTAINER::Mirror( const wxPoint& mirror_ref )
{
    for( unsigned ii = 0; ii < m_Poly->m_CornersList.size(); ii++ )
    {
        m_Poly->m_CornersList[ii].y -= mirror_ref.y;
        NEGATE( m_Poly->m_CornersList[ii].y );
        m_Poly->m_CornersList[ii].y += mirror_ref.y;
    }

    m_Poly->Hatch();

    /* mirror filled areas: */
    for( unsigned ic = 0; ic < m_FilledPolysList.size(); ic++ )
    {
        CPolyPt* corner = &m_FilledPolysList[ic];
        corner->y -= mirror_ref.y;
        NEGATE( corner->y );
        corner->y += mirror_ref.y;
    }

    for( unsigned ic = 0; ic < m_FillSegmList.size(); ic++ )
    {
        m_FillSegmList[ic].m_Start.y -= mirror_ref.y;
        NEGATE( m_FillSegmList[ic].m_Start.y );
        m_FillSegmList[ic].m_Start.y += mirror_ref.y;
        m_FillSegmList[ic].m_End.y   -= mirror_ref.y;
        NEGATE( m_FillSegmList[ic].m_End.y );
        m_FillSegmList[ic].m_End.y += mirror_ref.y;
    }
}


void ZONE_CONTAINER::Copy( ZONE_CONTAINER* src )
{
    m_Parent = src->m_Parent;
    m_Layer  = src->m_Layer;
    SetNet( src->GetNet() );
    SetTimeStamp( src->m_TimeStamp );
    m_Poly->RemoveAllContours();
    m_Poly->Copy( src->m_Poly );                // copy outlines
    m_CornerSelection  = -1;                    // For corner moving, corner index to drag, or -1 if no selection
    m_ZoneClearance    = src->m_ZoneClearance;  // clearance value
    m_ZoneMinThickness = src->m_ZoneMinThickness;
    m_FillMode = src->m_FillMode;               // Filling mode (segments/polygons)
    m_ArcToSegmentsCount = src->m_ArcToSegmentsCount;
    m_PadConnection = src->m_PadConnection;
    m_ThermalReliefGap = src->m_ThermalReliefGap;
    m_ThermalReliefCopperBridge = src->m_ThermalReliefCopperBridge;
    m_Poly->SetHatchStyle( src->m_Poly->GetHatchStyle() );
    m_Poly->SetHatchPitch( src->m_Poly->GetHatchPitch() );
    m_Poly->m_HatchLines = src->m_Poly->m_HatchLines;   // Copy vector <CSegment>
    m_FilledPolysList.clear();
    m_FilledPolysList = src->m_FilledPolysList;
    m_FillSegmList.clear();
    m_FillSegmList = src->m_FillSegmList;
}


bool ZONE_CONTAINER::SetNetNameFromNetCode( void )
{
    NETINFO_ITEM* net;

    if( m_Parent && ( net = ( (BOARD*) m_Parent )->FindNet( GetNet() ) ) )
    {
        m_Netname = net->GetNetname();
        return true;
    }

    return false;
}


ZoneConnection ZONE_CONTAINER::GetPadConnection( D_PAD* aPad ) const
{
    if( aPad == NULL || aPad->GetZoneConnection() == UNDEFINED_CONNECTION )
        return m_PadConnection;
    else
        return aPad->GetZoneConnection();
}


void ZONE_CONTAINER::AddPolygon( std::vector< wxPoint >& aPolygon )
{
    if( aPolygon.empty() )
        return;

    for( unsigned i = 0;  i < aPolygon.size();  i++ )
    {
        if( i == 0 )
            m_Poly->Start( GetLayer(), aPolygon[i].x, aPolygon[i].y, GetHatchStyle() );
        else
            AppendCorner( aPolygon[i] );
    }

    m_Poly->CloseLastContour();
}



wxString ZONE_CONTAINER::GetSelectMenuText() const
{
    wxString text;
    NETINFO_ITEM* net;
    BOARD* board = GetBoard();

    text = _( "Zone Outline" );

    int ncont = m_Poly->GetContour( m_CornerSelection );

    if( ncont )
        text << wxT( " " ) << _( "(Cutout)" );

    if( GetIsKeepout() )
        text << wxT( " " ) << _( "(Keepout)" );

    text << wxT( " " );
    text << wxString::Format( wxT( "(%08lX)" ), m_TimeStamp );

    // Display net name for copper zones
    if( !GetIsKeepout() )
    {
        if( GetNet() >= 0 )
        {
            if( board )
            {
                net = board->FindNet( GetNet() );

                if( net )
                {
                    text << wxT( " [" ) << net->GetNetname() << wxT( "]" );
                }
            }
            else
            {
                text << _( "** NO BOARD DEFINED **" );
            }
        }
        else
        {   // A netcode < 0 is an error:
            // Netname not found or area not initialised
            text << wxT( " [" ) << m_Netname << wxT( "]" );
            text << wxT( " <" ) << _( "Not Found" ) << wxT( ">" );
        }
    }

    text << _( " on layer " ) << GetLayerName();

    return text;
}
