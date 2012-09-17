/**
 * @file common_plotPS_functions.cpp
 * @brief Kicad: Common plot SVG functions
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jp.charras at wanadoo.fr
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

/* Some info on basic items SVG format, used here:
 *  The root element of all SVG files is the <svg> element.
 *
 *  The <g> element is used to group SVG shapes together.
 *  Once grouped you can transform the whole group of shapes as if it was a single shape.
 *  This is an advantage compared to a nested <svg> element
 *  which cannot be the target of transformation by itself.
 *
 *  The <rect> element represents a rectangle.
 *  Using this element you can draw rectangles of various width, height,
 *  with different stroke (outline) and fill colors, with sharp or rounded corners etc.
 *
 *  <svg xmlns="http://www.w3.org/2000/svg"
 *    xmlns:xlink="http://www.w3.org/1999/xlink">
 *
 *   <rect x="10" y="10" height="100" width="100"
 *         style="stroke:#006600; fill: #00cc00"/>
 *
 *  </svg>
 *
 *  The <circle> element is used to draw circles.
 *   <circle cx="40" cy="40" r="24" style="stroke:#006600; fill:#00cc00"/>
 *
 *  The <ellipse> element is used to draw ellipses.
 *  An ellipse is a circle that does not have equal height and width.
 *  Its radius in the x and y directions are different, in other words.
 *  <ellipse cx="40" cy="40" rx="30" ry="15"
 *          style="stroke:#006600; fill:#00cc00"/>
 *
 *  The <line> element is used to draw lines.
 *
 *   <line x1="0"  y1="10" x2="0"   y2="100" style="stroke:#006600;"/>
 *   <line x1="10" y1="10" x2="100" y2="100" style="stroke:#006600;"/>
 *
 *  The <polyline> element is used to draw multiple connected lines
 *  Here is a simple example:
 *
 *   <polyline points="0,0  30,0  15,30" style="stroke:#006600;"/>
 *
 *  The <polygon> element is used to draw with multiple (3 or more) sides / edges.
 *  Here is a simple example:
 *
 *   <polygon points="0,0  50,0  25,50" style="stroke:#660000; fill:#cc3333;"/>
 *
 *  The <path> element is used to draw advanced shapes combined from lines and archs,
 *  with or without fill.
 *  It is probably the most advanced and versatile SVG shape of them all.
 *  It is probably also the hardest element to master.
 *   <path d="M50,50
 *            A30,30 0 0,1 35,20
 *            L100,100
 *            M110,110
 *            L100,0"
 *         style="stroke:#660000; fill:none;"/>
 */
#include <fctsys.h>
#include <trigo.h>
#include <wxstruct.h>
#include <base_struct.h>
#include <common.h>
#include <plot_common.h>
#include <macros.h>
#include <kicad_string.h>

SVG_PLOTTER::SVG_PLOTTER()
{
    m_graphics_changed = true;
    SetTextMode( PLOTTEXTMODE_STROKE );
    m_fillMode = NO_FILL;               // or FILLED_SHAPE or FILLED_WITH_BG_BODYCOLOR
    m_pen_rgb_color = 0;                // current color value (black)
    m_brush_rgb_color = 0;              // current color value (black)
}


void SVG_PLOTTER::SetViewport( const wxPoint& aOffset, double aIusPerDecimil,
                               double aScale, bool aMirror )
{
    wxASSERT( !outputFile );
    plotMirror  = not aMirror;      // unlike other plotters, SVG has Y axis reversed
    plotOffset  = aOffset;
    plotScale   = aScale;
    m_IUsPerDecimil = aIusPerDecimil;
    iuPerDeviceUnit = 1.0 / aIusPerDecimil;
    /* Compute the paper size in IUs */
    paperSize   = pageInfo.GetSizeMils();
    paperSize.x *= 10.0 * aIusPerDecimil;
    paperSize.y *= 10.0 * aIusPerDecimil;
    SetDefaultLineWidth( 100 * aIusPerDecimil );    // arbitrary default
}


void SVG_PLOTTER::SetColor( EDA_COLOR_T color )
{
    PSLIKE_PLOTTER::SetColor( color );
}


void SVG_PLOTTER::setFillMode( FILL_T fill )
{
    if( m_fillMode != fill )
    {
        m_graphics_changed = true;
        m_fillMode = fill;
    }
}


void SVG_PLOTTER::setSVGPlotStyle()
{
    fputs( "</g>\n<g style=\"", outputFile );
    fputs( "fill:#", outputFile );
    // output the background fill color
    fprintf( outputFile, "%6.6lX; ", m_brush_rgb_color );

    switch( m_fillMode )
    {
    case NO_FILL:
        fputs( "fill-opacity:0.0;\n", outputFile );
        break;

    case FILLED_SHAPE:
        fputs( "fill-opacity:1.0;\n", outputFile );
        break;

    case FILLED_WITH_BG_BODYCOLOR:
        fputs( "fill-opacity:0.3;\n", outputFile );
        break;
    }

    // output the pen color (RVB values in hex) and opacity
    double pen_opacity = 1.0;      // 0.0 (transparent to 1.0 (solid)
    fprintf( outputFile, "  stroke:#%6.6lX; stroke-opacity:%g;\n",
             m_pen_rgb_color, pen_opacity );

    // output the pen cap
    int pen_cap = 0;    // round, square, butt (currenly not used)

    switch( pen_cap )
    {
    case  1:
        fputs( "stroke-linecap:square; ", outputFile );
        break;

    case  2:
        fputs( "stroke-linecap:butt; ", outputFile );
        break;

    case    0:
    default:
        fputs( "stroke-linecap:round; ", outputFile );
    }

    fputs( "stroke-linejoin:round; ", outputFile );

    int pen_w = (int) userToDeviceSize( GetCurrentLineWidth() );
    fprintf( outputFile,
             "stroke-width:%d\" \n   transform=\"translate(%.2g %.2g) scale(%.2g %.2g)\">\n",
             pen_w,
             userToDeviceSize( plotOffset.x ), userToDeviceSize( plotOffset.y ),
             plotScale, plotScale  );

    m_graphics_changed = false;
}


/* Set the current line width (in IUs) for the next plot
 */
void SVG_PLOTTER::SetCurrentLineWidth( int width )
{
    int pen_width;

    if( width >= 0 )
        pen_width = width;
    else
        pen_width = defaultPenWidth;

    if( pen_width != currentPenWidth )
    {
        m_graphics_changed  = true;
        currentPenWidth     = pen_width;
    }

    if( m_graphics_changed )
        setSVGPlotStyle();
}


/* initialize m_red, m_green, m_blue ( 0 ... 255)
 * from reduced values r, g ,b ( 0.0 to 1.0 )
 */
void SVG_PLOTTER::emitSetRGBColor( double r, double g, double b )
{
    int red     = (int) ( 255.0 * r );
    int green   = (int) ( 255.0 * g );
    int blue    = (int) ( 255.0 * b );
    long rgb_color = (red << 16) | (green << 8) | blue;

    if( m_pen_rgb_color != rgb_color )
    {
        m_graphics_changed = true;
        m_pen_rgb_color = rgb_color;

        // Currently, use the same color for brush and pen
        // (i.e. to draw and fill a contour)
        m_brush_rgb_color = rgb_color;
    }
}


/**
 * SVG supports dashed lines
 */
void SVG_PLOTTER::SetDash( bool dashed )
{
}


void SVG_PLOTTER::Rect( const wxPoint& p1, const wxPoint& p2, FILL_T fill, int width )
{
    DPOINT  p1_dev  = userToDeviceCoordinates( p1 );
    DPOINT  p2_dev  = userToDeviceCoordinates( p2 );

    setFillMode( fill );
    SetCurrentLineWidth( width );

    fprintf( outputFile,
             "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" rx=\"%d\" />\n",
             (int) p1_dev.x, (int) p1_dev.y,                            // origin
             (int) (p2_dev.x - p1_dev.x), (int) (p2_dev.y - p1_dev.y),  // size
             0                                                          // radius of rounded corners
             );
}


void SVG_PLOTTER::Circle( const wxPoint& pos, int diametre, FILL_T fill, int width )
{
    DPOINT  pos_dev = userToDeviceCoordinates( pos );
    double  radius  = userToDeviceSize( diametre / 2.0 );

    setFillMode( fill );
    SetCurrentLineWidth( width );

    fprintf( outputFile,
             "<circle cx=\"%g\" cy=\"%g\" r=\"%g\" /> \n",
             pos_dev.x, pos_dev.y, radius );
}


void SVG_PLOTTER::Arc( const wxPoint& centre, int StAngle, int EndAngle, int radius,
                       FILL_T fill, int width )
{
    /* Draws an arc of a circle, centred on (xc,yc), with starting point
     *  (x1, y1) and ending at (x2, y2). The current pen is used for the outline
     *  and the current brush for filling the shape.
     *
     *  The arc is drawn in an anticlockwise direction from the start point to
     *  the end point
     */

    if( radius <= 0 )
        return;

    if( StAngle > EndAngle )
        EXCHG( StAngle, EndAngle );

    setFillMode( fill );
    SetCurrentLineWidth( width );

    // Calculate start point.
    DPOINT  centre_dev  = userToDeviceCoordinates( centre );
    double  radius_dev  = userToDeviceSize( radius );

    if( plotMirror )
    {
        int tmp = StAngle;
        StAngle     = -EndAngle;
        EndAngle    = -tmp;
    }

    DPOINT  start = centre_dev;
    start.x += radius_dev;
    DPOINT  end = start;
    RotatePoint( &start.x, &start.y, StAngle );
    RotatePoint( &end.x, &end.y, EndAngle );

    double theta1 = StAngle * M_PI / 1800.0;

    if( theta1 < 0 )
        theta1 = theta1 + M_PI * 2;

    double theta2 = EndAngle * M_PI / 1800.0;

    if( theta2 < 0 )
        theta2 = theta2 + M_PI * 2;

    if( theta2 < theta1 )
        theta2 = theta2 + M_PI * 2;

    int flg_arc = 0;    // flag for large or small arc. 0 means less than 180 degrees

    if( fabs( theta2 - theta1 ) > M_PI )
        flg_arc = 1;

    int flg_sweep = 0;             // flag for sweep always 0

    // Draw a single arc: an arc is one of 3 curve commands (2 other are 2 bezier curves)
    // params are start point, radius1, radius2, X axe rotation,
    // flag arc size (0 = small arc > 180 deg, 1 = large arc > 180 deg),
    // sweep arc ( 0 = CCW, 1 = CW),
    // end point,
    // center point (optional, needed to draw a pie
    fprintf( outputFile, "<path d=\"M%d %d A%d %d 0.0 %d %d %d %d \" /> \n",
             (int) start.x, (int) start.y,
             (int) radius_dev, (int) radius_dev,
             flg_arc, flg_sweep,
             (int) end.x, (int) end.y  );
}


void SVG_PLOTTER::PlotPoly( const std::vector<wxPoint>& aCornerList,
                            FILL_T aFill, int aWidth )
{
    if( aCornerList.size() <= 1 )
        return;

    setFillMode( aFill );
    SetCurrentLineWidth( aWidth );

    fprintf( outputFile, "<polygon style=\"fill-rule:evenodd;\"\n" );

    DPOINT pos = userToDeviceCoordinates( aCornerList[0] );
    fprintf( outputFile, "points=\"%d,%d\n", (int) pos.x, (int) pos.y );

    for( unsigned ii = 1; ii < aCornerList.size(); ii++ )
    {
        pos = userToDeviceCoordinates( aCornerList[ii] );
        fprintf( outputFile, "%d,%d\n", (int) pos.x, (int) pos.y );
    }

    // Close/(fill) the path
    fprintf( outputFile, "\" /> \n" );
}


/**
 * Postscript-likes at the moment are the only plot engines supporting bitmaps...
 */
void SVG_PLOTTER::PlotImage( const wxImage& aImage, const wxPoint& aPos,
                             double aScaleFactor )
{
    // in svg file we must insert a link to a png image file to plot an image
    // the image itself is not included in the svg file.
    // So we prefer skip the image, and just draw a rectangle,
    // like othe plotter which do not support images

    PLOTTER::PlotImage( aImage, aPos, aScaleFactor );

}


void SVG_PLOTTER::PenTo( const wxPoint& pos, char plume )
{
    if( plume == 'Z' )
    {
        if( penState != 'Z' )
        {
            fputs( "\" />\n", outputFile );
            penState        = 'Z';
            penLastpos.x    = -1;
            penLastpos.y    = -1;
        }

        return;
    }

    if( penState == 'Z' )    // here plume = 'D' or 'U'
    {
        DPOINT pos_dev = userToDeviceCoordinates( pos );
        fprintf( outputFile, "<path d=\"M%d %d\n",
                 (int) pos_dev.x, (int) pos_dev.y );
    }
    else if( penState != plume || pos != penLastpos )
    {
        DPOINT pos_dev = userToDeviceCoordinates( pos );
        fprintf( outputFile, "L%d %d\n",
                 (int) pos_dev.x, (int) pos_dev.y );
    }

    penState    = plume;
    penLastpos  = pos;
}


/**
 * The code within this function
 * creates SVG files header
 */
bool SVG_PLOTTER::StartPlot( FILE* fout )
{
    wxASSERT( !outputFile );
    wxString            msg;

    outputFile = fout;
    static const char*  header[] =
    {
        "<?xml version=\"1.0\" standalone=\"no\"?>\n",
        " <!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \n",
        " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"> \n",
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" \n",
        NULL
    };

    // Write header.
    for( int ii = 0; header[ii] != NULL; ii++ )
    {
        fputs( header[ii], outputFile );
    }

    // Write viewport pos and size
    wxPoint origin;    // TODO set to actual value
    fprintf( outputFile,
             "    width=\"%gcm\" height=\"%gcm\" viewBox=\"%d %d %d %d \">\n",
             (double) paperSize.x / m_IUsPerDecimil * 2.54 / 10000,
             (double) paperSize.y / m_IUsPerDecimil * 2.54 / 10000,
             origin.x, origin.y,
             (int) ( paperSize.x / m_IUsPerDecimil ),
             (int) ( paperSize.y / m_IUsPerDecimil) );

    // Write title
    char    date_buf[250];
    time_t  ltime = time( NULL );
    strftime( date_buf, 250, "%Y/%m/%d %H:%M:%S",
              localtime( &ltime ) );

    fprintf( outputFile,
             "<title>SVG Picture created as %s date %s </title>\n",
             TO_UTF8( wxFileName( filename ).GetFullName() ), date_buf );
    // End of header
    fprintf( outputFile, "  <desc>Picture generated by %s </desc>\n",
             TO_UTF8( creator ) );
    fputs( "<g style=\"fill:black; stroke:black; stroke-width:1\">\n",
           outputFile );
    setSVGPlotStyle();

    return true;
}


bool SVG_PLOTTER::EndPlot()
{
    fputs( "</g> \n</svg>\n", outputFile );
    fclose( outputFile );
    outputFile = NULL;

    return true;
}


void SVG_PLOTTER::Text( const wxPoint&              aPos,
                        enum EDA_COLOR_T            aColor,
                        const wxString&             aText,
                        int                         aOrient,
                        const wxSize&               aSize,
                        enum EDA_TEXT_HJUSTIFY_T    aH_justify,
                        enum EDA_TEXT_VJUSTIFY_T    aV_justify,
                        int                         aWidth,
                        bool                        aItalic,
                        bool                        aBold )
{
    setFillMode( NO_FILL );
    SetColor( aColor );
    SetCurrentLineWidth( aWidth );

    // TODO: see if the postscript native text code can be used in SVG plotter

    PLOTTER::Text( aPos, aColor, aText, aOrient, aSize, aH_justify, aV_justify,
                   aWidth, aItalic, aBold );
}