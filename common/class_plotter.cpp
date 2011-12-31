/******************************************
* class_plotter.cpp
* the class PLOTTER handle basic functions to plot schematic and boards
* with different plot formats.
* currently formats are:*
* HPGL
* POSTSCRIPT
* GERBER
* DXF
******************************************/

#include "fctsys.h"

#include "trigo.h"
#include "wxstruct.h"
#include "base_struct.h"
#include "common.h"
#include "plot_common.h"
#include "worksheet.h"
#include "macros.h"
#include "class_base_screen.h"
#include "drawtxt.h"

PLOTTER::PLOTTER( PlotFormat aPlotType )
{
    m_PlotType = aPlotType;
    plot_scale = 1;
    default_pen_width = 0;
    current_pen_width = -1;     /* To-be-set marker */
    pen_state = 'Z';            /* End-of-path idle */
    plotMirror = 0;    /* Mirror flag */
    output_file   = 0;
    color_mode    = false;      /* Start as a BW plot */
    negative_mode = false;
}


/* Modifies coordinates pos.x and pos.y trace according to the orientation,
 * scale factor, and offsets trace
 */
void PLOTTER::user_to_device_coordinates( wxPoint& pos )
{
    pos.x = (int) ( (pos.x - plot_offset.x) * plot_scale * device_scale );

    if( plotMirror )
        pos.y = (int) ( ( pos.y - plot_offset.y ) * plot_scale * device_scale );
    else
        pos.y = (int) ( ( paper_size.y - ( pos.y - plot_offset.y )
                          * plot_scale ) * device_scale );
}


/* Generic arc rendered as a polyline */
void PLOTTER::arc( wxPoint centre, int StAngle, int EndAngle, int radius,
                   FILL_T fill, int width )
{
    wxPoint   start, end;
    const int delta = 50;   /* increment (in 0.1 degrees) to draw circles */
    double    alpha;

    if( StAngle > EndAngle )
        EXCHG( StAngle, EndAngle );

    set_current_line_width( width );
    /* Please NOTE the different sign due to Y-axis flip */
    alpha   = StAngle / 1800.0 * M_PI;
    start.x = centre.x + (int) ( radius * cos( -alpha ) );
    start.y = centre.y + (int) ( radius * sin( -alpha ) );
    move_to( start );
    for( int ii = StAngle + delta; ii < EndAngle; ii += delta )
    {
        alpha = ii / 1800.0 * M_PI;
        end.x = centre.x + (int) ( radius * cos( -alpha ) );
        end.y = centre.y + (int) ( radius * sin( -alpha ) );
        line_to( end );
    }

    alpha = EndAngle / 1800.0 * M_PI;
    end.x = centre.x + (int) ( radius * cos( -alpha ) );
    end.y = centre.y + (int) ( radius * sin( -alpha ) );
    finish_to( end );
}


/* Modifies size size.x and size.y trace according to the scale factor. */
void PLOTTER::user_to_device_size( wxSize& size )
{
    size.x = (int) ( size.x * plot_scale * device_scale );
    size.y = (int) ( size.y * plot_scale * device_scale );
}


double PLOTTER::user_to_device_size( double size )
{
    return size * plot_scale * device_scale;
}


void PLOTTER::center_square( const wxPoint& position, int diametre, FILL_T fill )
{
    int radius     = wxRound( diametre / 2.8284 );
    static std::vector< wxPoint > corner_list;
    corner_list.clear();
    wxPoint corner;
    corner.x = position.x + radius;
    corner.y = position.y + radius;
    corner_list.push_back( corner );
    corner.x = position.x + radius;
    corner.y = position.y - radius;
    corner_list.push_back( corner );
    corner.x = position.x - radius;
    corner.y = position.y - radius;
    corner_list.push_back( corner );
    corner.x = position.x - radius;
    corner.y = position.y + radius;
    corner_list.push_back( corner );
    corner.x = position.x + radius;
    corner.y = position.y + radius;
    corner_list.push_back( corner );

    PlotPoly( corner_list, fill );
}


void PLOTTER::center_lozenge( const wxPoint& position, int diametre, FILL_T fill )
{
    int radius     = diametre / 2;
    static std::vector< wxPoint > corner_list;
    corner_list.clear();
    wxPoint corner;
    corner.x = position.x;
    corner.y = position.y + radius;
    corner_list.push_back( corner );
    corner.x = position.x + radius;
    corner.y = position.y,
    corner_list.push_back( corner );
    corner.x = position.x;
    corner.y = position.y - radius;
    corner_list.push_back( corner );
    corner.x = position.x - radius;
    corner.y = position.y;
    corner_list.push_back( corner );
    corner.x = position.x;
    corner.y = position.y + radius;
    corner_list.push_back( corner );

    PlotPoly( corner_list, fill );
}


/* Draw a pattern shape number aShapeId, to coord x0, y0.
 * x0, y0 = coordinates tables
 * Diameter diameter = (coord table) hole
 * AShapeId = index (used to generate forms characters)
 */
void PLOTTER::marker( const wxPoint& position, int diametre, int aShapeId )
{
    int radius = diametre / 2;

    int x0, y0;

    x0 = position.x; y0 = position.y;

    switch( aShapeId )
    {
    case 0:     /* vias : X shape */
        move_to( wxPoint( x0 - radius, y0 - radius ) );
        line_to( wxPoint( x0 + radius, y0 + radius ) );
        move_to( wxPoint( x0 + radius, y0 - radius ) );
        finish_to( wxPoint( x0 - radius, y0 + radius ) );
        break;

    case 1:     /* Circle */
        circle( position, diametre, NO_FILL );
        break;

    case 2:     /* + shape */
        move_to( wxPoint( x0, y0 - radius ) );
        line_to( wxPoint( x0, y0 + radius ) );
        move_to( wxPoint( x0 + radius, y0 ) );
        finish_to( wxPoint( x0 - radius, y0 ) );
        break;

    case 3:     /* X shape in circle */
        circle( position, diametre, NO_FILL );
        move_to( wxPoint( x0 - radius, y0 - radius ) );
        line_to( wxPoint( x0 + radius, y0 + radius ) );
        move_to( wxPoint( x0 + radius, y0 - radius ) );
        finish_to( wxPoint( x0 - radius, y0 + radius ) );
        break;

    case 4:     /* circle with bar - shape */
        circle( position, diametre, NO_FILL );
        move_to( wxPoint( x0 - radius, y0 ) );
        finish_to( wxPoint( x0 + radius, y0 ) );
        break;

    case 5:     /* circle with bar | shape */
        circle( position, diametre, NO_FILL );
        move_to( wxPoint( x0, y0 - radius ) );
        finish_to( wxPoint( x0, y0 + radius ) );
        break;

    case 6:     /* square */
        center_square( position, diametre, NO_FILL );
        break;

    case 7:     /* diamond */
        center_lozenge( position, diametre, NO_FILL );
        break;

    case 8:     /* square with an X*/
        center_square( position, diametre, NO_FILL );
        move_to( wxPoint( x0 - radius, y0 - radius ) );
        line_to( wxPoint( x0 + radius, y0 + radius ) );
        move_to( wxPoint( x0 + radius, y0 - radius ) );
        finish_to( wxPoint( x0 - radius, y0 + radius ) );
        break;

    case 9:     /* diamond with a +*/
        center_lozenge( position, diametre, NO_FILL );
        move_to( wxPoint( x0, y0 - radius ) );
        line_to( wxPoint( x0, y0 + radius ) );
        move_to( wxPoint( x0 + radius, y0 ) );
        finish_to( wxPoint( x0 - radius, y0 ) );
        break;

    case 10:     /* square with a '/' */
        center_square( position, diametre, NO_FILL );
        move_to( wxPoint( x0 - radius, y0 - radius ) );
        finish_to( wxPoint( x0 + radius, y0 + radius ) );
        break;

    case 11:     /* square with a |*/
        center_lozenge( position, diametre, NO_FILL );
        move_to( wxPoint( x0, y0 - radius ) );
        finish_to( wxPoint( x0, y0 + radius ) );
        break;

    case 12:     /* square with a -*/
        center_lozenge( position, diametre, NO_FILL );
        move_to( wxPoint( x0 - radius, y0 ) );
        finish_to( wxPoint( x0 + radius, y0 ) );
        break;

    default:
        circle( position, diametre, NO_FILL );
        break;
    }
}


/* Convert a thick segment and plot it as an oval */
void PLOTTER::segment_as_oval( wxPoint start, wxPoint end, int width,
                               GRTraceMode tracemode )
{
    wxPoint center( (start.x + end.x) / 2, (start.y + end.y) / 2 );
    wxSize  size( end.x - start.x, end.y - start.y );
    int     orient;

    if( size.y == 0 )
        orient = 0;
    else if( size.x == 0 )
        orient = 900;
    else
        orient = -(int) ( atan2( (double) size.y,
                                 (double) size.x ) * 1800.0 / M_PI );
    size.x = (int) sqrt( ( (double) size.x * size.x )
                       + ( (double) size.y * size.y ) ) + width;
    size.y = width;

    flash_pad_oval( center, size, orient, tracemode );
}


void PLOTTER::sketch_oval( wxPoint pos, wxSize size, int orient,
                           int width )
{
    set_current_line_width( width );
    width = current_pen_width;
    int radius, deltaxy, cx, cy;

    if( size.x > size.y )
    {
        EXCHG( size.x, size.y );
        orient += 900;
        if( orient >= 3600 )
            orient -= 3600;
    }

    deltaxy = size.y - size.x;       /* distance between centers of the oval */
    radius   = ( size.x - width ) / 2;
    cx = -radius;
    cy = -deltaxy / 2;
    RotatePoint( &cx, &cy, orient );
    move_to( wxPoint( cx + pos.x, cy + pos.y ) );
    cx = -radius;
    cy = deltaxy / 2;
    RotatePoint( &cx, &cy, orient );
    finish_to( wxPoint( cx + pos.x, cy + pos.y ) );

    cx = radius;
    cy = -deltaxy / 2;
    RotatePoint( &cx, &cy, orient );
    move_to( wxPoint( cx + pos.x, cy + pos.y ) );
    cx = radius;
    cy = deltaxy / 2;
    RotatePoint( &cx, &cy, orient );
    finish_to( wxPoint( cx + pos.x, cy + pos.y ) );

    cx = 0;
    cy = deltaxy / 2;
    RotatePoint( &cx, &cy, orient );
    arc( wxPoint( cx + pos.x, cy + pos.y ),
         orient + 1800, orient + 3600,
         radius, NO_FILL );
    cx = 0;
    cy = -deltaxy / 2;
    RotatePoint( &cx, &cy, orient );
    arc( wxPoint( cx + pos.x, cy + pos.y ),
         orient, orient + 1800,
         radius, NO_FILL );
}


/* Plot 1 segment like a track segment
 */
void PLOTTER::thick_segment( wxPoint start, wxPoint end, int width,
                             GRTraceMode tracemode )
{
    switch( tracemode )
    {
    case FILLED:
    case FILAIRE:
        set_current_line_width( tracemode==FILLED ? width : -1 );
        move_to( start );
        finish_to( end );
        break;

    case SKETCH:
        set_current_line_width( -1 );
        segment_as_oval( start, end, width, tracemode );
        break;
    }
}


void PLOTTER::thick_arc( wxPoint centre, int StAngle, int EndAngle, int radius,
                         int width, GRTraceMode tracemode )
{
    switch( tracemode )
    {
    case FILAIRE:
        set_current_line_width( -1 );
        arc( centre, StAngle, EndAngle, radius, NO_FILL, -1 );
        break;

    case FILLED:
        arc( centre, StAngle, EndAngle, radius, NO_FILL, width );
        break;

    case SKETCH:
        set_current_line_width( -1 );
        arc( centre, StAngle, EndAngle,
             radius - ( width - current_pen_width ) / 2, NO_FILL, -1 );
        arc( centre, StAngle, EndAngle,
             radius + ( width - current_pen_width ) / 2, NO_FILL, -1 );
        break;
    }
}


void PLOTTER::thick_rect( wxPoint p1, wxPoint p2, int width,
                          GRTraceMode tracemode )
{
    switch( tracemode )
    {
    case FILAIRE:
        rect( p1, p2, NO_FILL, -1 );
        break;

    case FILLED:
        rect( p1, p2, NO_FILL, width );
        break;

    case SKETCH:
        set_current_line_width( -1 );
        p1.x -= (width - current_pen_width) / 2;
        p1.y -= (width - current_pen_width) / 2;
        p2.x += (width - current_pen_width) / 2;
        p2.y += (width - current_pen_width) / 2;
        rect( p1, p2, NO_FILL, -1 );
        p1.x += (width - current_pen_width);
        p1.y += (width - current_pen_width);
        p2.x -= (width - current_pen_width);
        p2.y -= (width - current_pen_width);
        rect( p1, p2, NO_FILL, -1 );
        break;
    }
}


void PLOTTER::thick_circle( wxPoint pos, int diametre, int width,
                            GRTraceMode tracemode )
{
    switch( tracemode )
    {
    case FILAIRE:
        circle( pos, diametre, NO_FILL, -1 );
        break;

    case FILLED:
        circle( pos, diametre, NO_FILL, width );
        break;

    case SKETCH:
        set_current_line_width( -1 );
        circle( pos, diametre - width + current_pen_width, NO_FILL, -1 );
        circle( pos, diametre + width - current_pen_width, NO_FILL, -1 );
        break;
    }
}


void PLOTTER::SetPageSettings( const PAGE_INFO& aPageSettings )
{
    wxASSERT( !output_file );
    pageInfo = aPageSettings;

    // PAGE_INFO is in mils, plotter works with deci-mils
    paper_size = pageInfo.GetSizeMils() * 10;
}

