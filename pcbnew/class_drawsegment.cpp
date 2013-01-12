/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jean-pierre.charras@gipsa-lab.inpg.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file class_drawsegment.cpp
 * @brief Class and functions to handle a graphic segments.
 */

#include <fctsys.h>
#include <macros.h>
#include <wxstruct.h>
#include <gr_basic.h>
#include <bezier_curves.h>
#include <class_drawpanel.h>
#include <class_pcb_screen.h>
#include <kicad_string.h>
#include <colors_selection.h>
#include <trigo.h>
#include <richio.h>
#include <pcbcommon.h>
#include <msgpanel.h>

#include <pcbnew.h>
#include <protos.h>

#include <class_board.h>
#include <class_module.h>
#include <class_drawsegment.h>
#include <base_units.h>


DRAWSEGMENT::DRAWSEGMENT( BOARD_ITEM* aParent, KICAD_T idtype ) :
    BOARD_ITEM( aParent, idtype )
{
    m_Width = m_Flags = m_Type = m_Angle = 0;
    m_Shape = S_SEGMENT;
}


DRAWSEGMENT::~DRAWSEGMENT()
{
}


const DRAWSEGMENT& DRAWSEGMENT::operator = ( const DRAWSEGMENT& rhs )
{
    // skip the linked list stuff, and parent

    m_Type         = rhs.m_Type;
    m_Layer        = rhs.m_Layer;
    m_Width        = rhs.m_Width;
    m_Start        = rhs.m_Start;
    m_End          = rhs.m_End;
    m_Shape        = rhs.m_Shape;
    m_Angle        = rhs.m_Angle;
    m_TimeStamp    = rhs.m_TimeStamp;
    m_BezierC1     = rhs.m_BezierC1;
    m_BezierC2     = rhs.m_BezierC1;
    m_BezierPoints = rhs.m_BezierPoints;

    return *this;
}


void DRAWSEGMENT::Copy( DRAWSEGMENT* source )
{
    if( source == NULL )    // who would do this?
        return;

    *this = *source;    // operator = ()
}

void DRAWSEGMENT::Rotate( const wxPoint& aRotCentre, double aAngle )
{
    RotatePoint( &m_Start, aRotCentre, aAngle );
    RotatePoint( &m_End, aRotCentre, aAngle );
}


void DRAWSEGMENT::Flip( const wxPoint& aCentre )
{
    m_Start.y  = aCentre.y - (m_Start.y - aCentre.y);
    m_End.y  = aCentre.y - (m_End.y - aCentre.y);

    if( m_Shape == S_ARC )
    {
        NEGATE( m_Angle );
    }

    SetLayer( BOARD::ReturnFlippedLayerNumber( GetLayer() ) );
}

const wxPoint DRAWSEGMENT::GetArcEnd() const
{
    wxPoint endPoint;         // start of arc

    switch( m_Shape )
    {
    case S_ARC:
        // rotate the starting point of the arc, given by m_End, through the
        // angle m_Angle to get the ending point of the arc.
        // m_Start is the arc centre
        endPoint  = m_End;         // m_End = start point of arc
        RotatePoint( &endPoint, m_Start, -m_Angle );
        break;

    default:
        ;
    }

    return endPoint;   // after rotation, the end of the arc.
}

const double DRAWSEGMENT::GetArcAngleStart() const
{
    // due to the Y axis orient atan2 needs - y value
    double angleStart = atan2( (double)(GetArcStart().y - GetCenter().y),
                               (double)(GetArcStart().x - GetCenter().x) );
    // angleStart is in radians, convert it in 1/10 degrees
    angleStart = angleStart / M_PI * 1800.0;

    // Normalize it to 0 ... 360 deg, to avoid discontinuity for angles near 180 deg
    // because 180 deg and -180 are very near angles when ampping betewwen -180 ... 180 deg.
    // and this is not easy to handle in calculations
    if( angleStart < 0 )
        angleStart += 3600.0;

    return angleStart;
}

void DRAWSEGMENT::SetAngle( double aAngle )
{
    NORMALIZE_ANGLE_360( aAngle );

    m_Angle = (int) aAngle;
}


MODULE* DRAWSEGMENT::GetParentModule() const
{
    if( m_Parent->Type() != PCB_MODULE_T )
        return NULL;

    return (MODULE*) m_Parent;
}


void DRAWSEGMENT::Draw( EDA_DRAW_PANEL* panel, wxDC* DC, GR_DRAWMODE draw_mode,
                        const wxPoint& aOffset )
{
    int ux0, uy0, dx, dy;
    int l_trace;
    int mode;
    int radius;
    int curr_layer = ( (PCB_SCREEN*) panel->GetScreen() )->m_Active_Layer;
    EDA_COLOR_T color;

    BOARD * brd =  GetBoard( );

    if( brd->IsLayerVisible( GetLayer() ) == false )
        return;

    color = brd->GetLayerColor( GetLayer() );

    if( ( draw_mode & GR_ALLOW_HIGHCONTRAST ) &&  DisplayOpt.ContrastModeDisplay )
    {
        if( !IsOnLayer( curr_layer ) && !IsOnLayer( EDGE_N ) )
            ColorTurnToDarkDarkGray( &color );
    }


    GRSetDrawMode( DC, draw_mode );
    l_trace = m_Width >> 1;  /* half trace width */

    // Line start point or Circle and Arc center
    ux0 = m_Start.x + aOffset.x;
    uy0 = m_Start.y + aOffset.y;

    // Line end point or circle and arc start point
    dx = m_End.x + aOffset.x;
    dy = m_End.y + aOffset.y;

    mode = DisplayOpt.DisplayDrawItems;

    if( m_Flags & FORCE_SKETCH )
        mode = SKETCH;

    if( l_trace < DC->DeviceToLogicalXRel( MIN_DRAW_WIDTH ) )
        mode = LINE;

    switch( m_Shape )
    {
    case S_CIRCLE:
        radius = (int) hypot( (double) (dx - ux0), (double) (dy - uy0) );

        if( mode == LINE )
        {
            GRCircle( panel->GetClipBox(), DC, ux0, uy0, radius, color );
        }
        else if( mode == SKETCH )
        {
            GRCircle( panel->GetClipBox(), DC, ux0, uy0, radius - l_trace, color );
            GRCircle( panel->GetClipBox(), DC, ux0, uy0, radius + l_trace, color );
        }
        else
        {
            GRCircle( panel->GetClipBox(), DC, ux0, uy0, radius, m_Width, color );
        }

        break;

    case S_ARC:
        int StAngle, EndAngle;
        radius    = (int) hypot( (double) (dx - ux0), (double) (dy - uy0) );
        StAngle  = (int) ArcTangente( dy - uy0, dx - ux0 );
        EndAngle = StAngle + m_Angle;

        if( !panel->GetPrintMirrored() )
        {
            if( StAngle > EndAngle )
                EXCHG( StAngle, EndAngle );
        }
        else    // Mirrored mode: arc orientation is reversed
        {
            if( StAngle < EndAngle )
                EXCHG( StAngle, EndAngle );
        }


        if( mode == LINE )
            GRArc( panel->GetClipBox(), DC, ux0, uy0, StAngle, EndAngle, radius, color );

        else if( mode == SKETCH )
        {
            GRArc( panel->GetClipBox(), DC, ux0, uy0, StAngle, EndAngle,
                   radius - l_trace, color );
            GRArc( panel->GetClipBox(), DC, ux0, uy0, StAngle, EndAngle,
                   radius + l_trace, color );
        }
        else
        {
            GRArc( panel->GetClipBox(), DC, ux0, uy0, StAngle, EndAngle,
                   radius, m_Width, color );
        }
        break;

    case S_CURVE:
        m_BezierPoints = Bezier2Poly(m_Start, m_BezierC1, m_BezierC2, m_End);

        for (unsigned int i=1; i < m_BezierPoints.size(); i++) {
            if( mode == LINE )
                GRLine( panel->GetClipBox(), DC,
                        m_BezierPoints[i].x, m_BezierPoints[i].y,
                        m_BezierPoints[i-1].x, m_BezierPoints[i-1].y, 0,
                        color );
            else if( mode == SKETCH )
            {
                GRCSegm( panel->GetClipBox(), DC,
                         m_BezierPoints[i].x, m_BezierPoints[i].y,
                         m_BezierPoints[i-1].x, m_BezierPoints[i-1].y,
                         m_Width, color );
            }
            else
            {
                GRFillCSegm( panel->GetClipBox(), DC,
                             m_BezierPoints[i].x, m_BezierPoints[i].y,
                             m_BezierPoints[i-1].x, m_BezierPoints[i-1].y,
                             m_Width, color );
            }
        }

        break;

    default:
        if( mode == LINE )
        {
            GRLine( panel->GetClipBox(), DC, ux0, uy0, dx, dy, 0, color );
        }
        else if( mode == SKETCH )
        {
            GRCSegm( panel->GetClipBox(), DC, ux0, uy0, dx, dy, m_Width, color );
        }
        else
        {
            GRFillCSegm( panel->GetClipBox(), DC, ux0, uy0, dx, dy, m_Width, color );
        }

        break;
    }
}


// see pcbstruct.h
void DRAWSEGMENT::GetMsgPanelInfo( std::vector< MSG_PANEL_ITEM >& aList )
{
    wxString msg;
    wxString coords;

    BOARD*   board = (BOARD*) m_Parent;
    wxASSERT( board );

    msg = wxT( "DRAWING" );

    aList.push_back( MSG_PANEL_ITEM( _( "Type" ), msg, DARKCYAN ) );

    wxString    shape = _( "Shape" );

    switch( m_Shape )
    {
    case S_CIRCLE:
        aList.push_back( MSG_PANEL_ITEM( shape, _( "Circle" ), RED ) );
        break;

    case S_ARC:
        aList.push_back( MSG_PANEL_ITEM( shape, _( "Arc" ), RED ) );
        msg.Printf( wxT( "%.1f" ), (double)m_Angle/10 );
        aList.push_back( MSG_PANEL_ITEM( _("Angle"), msg, RED ) );
        break;

    case S_CURVE:
        aList.push_back( MSG_PANEL_ITEM( shape, _( "Curve" ), RED ) );
        break;

    default:
        aList.push_back( MSG_PANEL_ITEM( shape, _( "Segment" ), RED ) );
    }

    wxString start;
    start << GetStart();

    wxString end;
    end << GetEnd();

    aList.push_back( MSG_PANEL_ITEM( start, end, DARKGREEN ) );
    aList.push_back( MSG_PANEL_ITEM( _( "Layer" ), board->GetLayerName( m_Layer ), DARKBROWN ) );
    msg = ::CoordinateToString( m_Width );
    aList.push_back( MSG_PANEL_ITEM( _( "Width" ), msg, DARKCYAN ) );
}


EDA_RECT DRAWSEGMENT::GetBoundingBox() const
{
    EDA_RECT bbox;

    bbox.SetOrigin( m_Start );

    switch( m_Shape )
    {
    case S_SEGMENT:
        bbox.SetEnd( m_End );
        break;

    case S_CIRCLE:
        bbox.Inflate( GetRadius() );
        break;

    case S_ARC:
        {
            bbox.Merge( m_End );
            wxPoint end = m_End;
            RotatePoint( &end, m_Start, -m_Angle );
            bbox.Merge( end );
        }
        break;

    case S_POLYGON:
        {
            wxPoint p_end;
            MODULE* module = GetParentModule();

            for( unsigned ii = 0; ii < m_PolyPoints.size(); ii++ )
            {
                wxPoint pt = m_PolyPoints[ii];

                if( module ) // Transform, if we belong to a module
                {
                    RotatePoint( &pt, module->GetOrientation() );
                    pt += module->m_Pos;
                }

                if( ii == 0 )
                    p_end = pt;

                bbox.SetX( std::min( bbox.GetX(), pt.x ) );
                bbox.SetY( std::min( bbox.GetY(), pt.y ) );
                p_end.x   = std::max( p_end.x, pt.x );
                p_end.y   = std::max( p_end.y, pt.y );
            }

            bbox.SetEnd( p_end );
        }
        break;

    default:
        ;
    }

    bbox.Inflate( ((m_Width+1) / 2) + 1 );
    bbox.Normalize();

    return bbox;
}


bool DRAWSEGMENT::HitTest( const wxPoint& aPosition )
{
    switch( m_Shape )
    {
    case S_CIRCLE:
    case S_ARC:
        {
            wxPoint relPos = aPosition - GetCenter();
            int radius = GetRadius();
            int dist  = (int) hypot( (double) relPos.x, (double) relPos.y );

            if( abs( radius - dist ) <= ( m_Width / 2 ) )
            {
                if( m_Shape == S_CIRCLE )
                    return true;

                // For arcs, the test point angle must be >= arc angle start
                // and <= arc angle end
                // However angle values > 360 deg are not easy to handle
                // so we calculate the relative angle between arc start point and teast point
                // this relative arc should be < arc angle if arc angle > 0 (CW arc)
                // and > arc angle if arc angle < 0 (CCW arc)
                double arc_angle_start = GetArcAngleStart();    // Always 0.0 ... 360 deg, in 0.1 deg

                double arc_hittest = atan2( (double) relPos.y, (double) relPos.x );
                arc_hittest = arc_hittest / M_PI * 1800;    // angles are in 1/10 deg

                // Calculate relative angle between the starting point of the arc, and the test point
                arc_hittest -= arc_angle_start;

                // Normalise arc_hittest between 0 ... 360 deg
                NORMALIZE_ANGLE_POS( arc_hittest );

                // Check angle: inside the arc angle when it is > 0
                // and outside the not drawn arc when it is < 0
                if( GetAngle() >= 0.0 )
                {
                    if( arc_hittest <= GetAngle() )
                        return true;
                }
                else
                {
                    if( arc_hittest >= (3600.0 + GetAngle()) )
                        return true;
                }
            }
        }
        break;

    case S_CURVE:
        for( unsigned int i= 1; i < m_BezierPoints.size(); i++)
        {
            if( TestSegmentHit( aPosition, m_BezierPoints[i-1], m_BezierPoints[i-1], m_Width / 2 ) )
                return true;
        }
        break;

    case S_SEGMENT:
        if( TestSegmentHit( aPosition, m_Start, m_End, m_Width / 2 ) )
            return true;
        break;

    default:
        wxASSERT( 0 );
        break;
    }
    return false;
}


bool DRAWSEGMENT::HitTest( const EDA_RECT& aRect ) const
{
    switch( m_Shape )
    {
    case S_CIRCLE:
        {
            int radius = GetRadius();

            // Text if area intersects the circle:
            EDA_RECT area = aRect;
            area.Inflate( radius );

            if( area.Contains( m_Start ) )
                return true;
        }
        break;

    case S_ARC:
    case S_SEGMENT:
        if( aRect.Contains( GetStart() ) )
            return true;

        if( aRect.Contains( GetEnd() ) )
            return true;
        break;

    default:
        ;
    }
    return false;
}


wxString DRAWSEGMENT::GetSelectMenuText() const
{
    wxString text;
    wxString temp = ::LengthDoubleToString( GetLength() );

    text.Printf( _( "Pcb Graphic: %s length: %s on %s" ),
                 GetChars( ShowShape( (STROKE_T) m_Shape ) ),
                 GetChars( temp ), GetChars( GetLayerName() ) );

    return text;
}


EDA_ITEM* DRAWSEGMENT::Clone() const
{
    return new DRAWSEGMENT( *this );
}


#if defined(DEBUG)
void DRAWSEGMENT::Show( int nestLevel, std::ostream& os ) const
{
    NestedSpace( nestLevel, os ) << '<' << GetClass().Lower().mb_str() <<

    " shape=\"" << m_Shape << '"' <<
/*
    " layer=\"" << GetLayer() << '"' <<
    " width=\"" << m_Width << '"' <<
    " angle=\"" << m_Angle << '"' <<  // Used only for Arcs: Arc angle in 1/10 deg
*/
    '>' <<
    "<start" << m_Start << "/>" <<
    "<end"   << m_End << "/>"
    "<GetStart" << GetStart() << "/>" <<
    "<GetEnd"   << GetEnd() << "/>"
    ;

    os << "</" << GetClass().Lower().mb_str() << ">\n";
}
#endif
