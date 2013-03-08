/**
 * @file common_plotDXF_functions.cpp
 * @brief KiCad: Common plot DXF Routines.
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <trigo.h>
#include <wxstruct.h>
#include <base_struct.h>
#include <plot_common.h>
#include <macros.h>
#include <kicad_string.h>

/**
 * Oblique angle for DXF native text
 * (I don't remember if 15 degrees is the ISO value... it looks nice anyway)
 */
static const double DXF_OBLIQUE_ANGLE = 15;

/**
 * Set the scale/position for the DXF plot
 * The DXF engine doesn't support line widths and mirroring. The output
 * coordinate system is in the first quadrant (in mm)
 */
void DXF_PLOTTER::SetViewport( const wxPoint& aOffset, double aIusPerDecimil,
                               double aScale, bool aMirror )
{
    wxASSERT( !outputFile );
    plotOffset  = aOffset;
    plotScale   = aScale;

    /* DXF paper is 'virtual' so there is no need of a paper size.
       Also this way we can handle the aux origin which can be useful
       (for example when aligning to a mechanical drawing) */
    paperSize.x = 0;
    paperSize.y = 0;

    /* Like paper size DXF units are abstract too. Anyway there is a
     * system variable (MEASUREMENT) which will be set to 1 to indicate
     * metric units */
    m_IUsPerDecimil = aIusPerDecimil;
    iuPerDeviceUnit = 1.0 / aIusPerDecimil; // Gives a DXF in decimils
    iuPerDeviceUnit *= 0.00254;             // ... now in mm

    SetDefaultLineWidth( 0 );               // No line width on DXF
    plotMirror = false;                     // No mirroring on DXF
    currentColor = BLACK;
}

/**
 * Opens the DXF plot with a skeleton header
 */
bool DXF_PLOTTER::StartPlot()
{
    wxASSERT( outputFile );

    // DXF HEADER - Boilerplate
    // Defines the minimum for drawing i.e. the angle system and the
    // continuous linetype
    fputs( "  0\n"
           "SECTION\n"
           "  2\n"
           "HEADER\n"
           "  9\n"
           "$ANGBASE\n"
           "  50\n"
           "0.0\n"
           "  9\n"
           "$ANGDIR\n"
           "  70\n"
           "  1\n"
           "  9\n"
           "$MEASUREMENT\n"
           "  70\n"
           "0\n"
           "  0\n"              // This means 'metric units'
           "ENDSEC\n"
           "  0\n"
           "SECTION\n"
           "  2\n"
           "TABLES\n"
           "  0\n"
           "TABLE\n"
           "  2\n"
           "LTYPE\n"
           "  70\n"
           "1\n"
           "  0\n"
           "LTYPE\n"
           "  2\n"
           "CONTINUOUS\n"
           "  70\n"
           "0\n"
           "  3\n"
           "Solid line\n"
           "  72\n"
           "65\n"
           "  73\n"
           "0\n"
           "  40\n"
           "0.0\n"
           "  0\n"
           "ENDTAB\n",
            outputFile );

    // Text styles table
    // Defines 4 text styles, one for each bold/italic combination
    fputs( "  0\n"
           "TABLE\n"
           "  2\n"
           "STYLE\n"
           "  70\n"
           "4\n", outputFile );

    static const char *style_name[4] = {"KICAD", "KICADB", "KICADI", "KICADBI"};
    for(int i = 0; i < 4; i++ )
    {
        fprintf( outputFile,
                 "  0\n"
                 "STYLE\n"
                 "  2\n"
                 "%s\n"         // Style name
                 "  70\n"
                 "0\n"          // Standard flags
                 "  40\n"
                 "0\n"          // Non-fixed height text
                 "  41\n"
                 "1\n"          // Width factor (base)
                 "  42\n"
                 "1\n"          // Last height (mandatory)
                 "  50\n"
                 "%g\n"         // Oblique angle
                 "  71\n"
                 "0\n"          // Generation flags (default)
                 "  3\n"
                 // The standard ISO font (when kicad is build with it
                 // the dxf text in acad matches *perfectly*)
                 "isocp.shx\n", // Font name (when not bigfont)
                 // Apply a 15 degree angle to italic text
                 style_name[i], i < 2 ? 0 : DXF_OBLIQUE_ANGLE );
    }


    // Layer table - one layer per color
    fprintf( outputFile,
             "  0\n"
             "ENDTAB\n"
             "  0\n"
             "TABLE\n"
             "  2\n"
             "LAYER\n"
             "  70\n"
             "%d\n", NBCOLOR );

    /* The layer/colors palette. The acad/DXF palette is divided in 3 zones:

       - The primary colors (1 - 9)
       - An HSV zone (10-250, 5 values x 2 saturations x 10 hues
       - Greys (251 - 255)

       The is *no* black... the white does it on paper, usually, and
       anyway it depends on the plotter configuration, since DXF colors
       are meant to be logical only (they represent *both* line color and
       width); later version with plot styles only complicate the matter!

       As usual, brown and magenta/purple are difficult to place since
       they are actually variations of other colors.
     */
    static const struct {
        const char *name;
        int color;
    } dxf_layer[NBCOLOR] = {
        { "BLACK",              250 },
        { "BLUE",               5 },
        { "GREEN",              3 },
        { "CYAN",               4 },
        { "RED",                1 },
        { "MAGENTA",            6 },
        { "BROWN",              54 },
        { "LIGHTGRAY",          9 },
        { "DARKGRAY",           8 },
        { "LIGHTBLUE",          171 },
        { "LIGHTGREEN",         91 },
        { "LIGHTCYAN",          131 },
        { "LIGHTRED",           11 },
        { "LIGHTMAGENTA",       221 },
        { "YELLOW",             2 },
        { "WHITE",              7 },
        { "DARKDARKGRAY",       251 },
        { "DARKBLUE",           178 },
        { "DARKGREEN",          98 },
        { "DARKCYAN",           138 },
        { "DARKRED",            18 },
        { "DARKMAGENTA",        228 },
        { "DARKBROWN",          58 },
        { "LIGHTYELLOW",        51 },
    };

    for( int i = 0; i < NBCOLOR; i++ )
    {
        wxString cname = ColorRefs[i].m_Name;
        fprintf( outputFile,
                 "  0\n"
                 "LAYER\n"
                 "  2\n"
                 "%s\n"         // Layer name
                 "  70\n"
                 "0\n"          // Standard flags
                 "  62\n"
                 "%d\n"         // Color number
                 "  6\n"
                 "CONTINUOUS\n",// Linetype name
                 dxf_layer[i].name, dxf_layer[i].color );
    }

    // End of layer table, begin entities
    fputs( "  0\n"
           "ENDTAB\n"
           "  0\n"
           "ENDSEC\n"
           "  0\n"
           "SECTION\n"
           "  2\n"
           "ENTITIES\n", outputFile );

    return true;
}


bool DXF_PLOTTER::EndPlot()
{
    wxASSERT( outputFile );

    // DXF FOOTER
    fputs( "  0\n"
           "ENDSEC\n"
           "  0\n"
           "EOF\n", outputFile );
    fclose( outputFile );
    outputFile = NULL;

    return true;
}


/**
 * The DXF exporter handles 'colors' as layers...
 */
void DXF_PLOTTER::SetColor( EDA_COLOR_T color )
{
    wxASSERT( outputFile );
    if( ( color >= 0 && colorMode )
       || ( color == BLACK )
       || ( color == WHITE ) )
    {
        currentColor = color;
    }
    else
        currentColor = BLACK;
}

/**
 * DXF rectangle: fill not supported
 */
void DXF_PLOTTER::Rect( const wxPoint& p1, const wxPoint& p2, FILL_T fill, int width )
{
    wxASSERT( outputFile );
    MoveTo( p1 );
    LineTo( wxPoint( p1.x, p2.y ) );
    LineTo( wxPoint( p2.x, p2.y ) );
    LineTo( wxPoint( p2.x, p1.y ) );
    FinishTo( wxPoint( p1.x, p1.y ) );
}


/**
 * DXF circle: full functionality; it even does 'fills' drawing a
 * circle with a dual-arc polyline wide as the radius.
 *
 * I could use this trick to do other filled primitives
 */
void DXF_PLOTTER::Circle( const wxPoint& centre, int diameter, FILL_T fill, int width )
{
    wxASSERT( outputFile );
    double radius = userToDeviceSize( diameter / 2 );
    DPOINT centre_dev = userToDeviceCoordinates( centre );
    if( radius > 0 )
    {
        wxString cname = ColorRefs[currentColor].m_Name;
        if (!fill)
        {
            fprintf( outputFile, "0\nCIRCLE\n8\n%s\n10\n%g\n20\n%g\n40\n%g\n",
                    TO_UTF8( cname ),
                    centre_dev.x, centre_dev.y, radius );
        }
        if (fill == FILLED_SHAPE)
        {
            double r = radius*0.5;
            fprintf( outputFile, "0\nPOLYLINE\n");
            fprintf( outputFile, "8\n%s\n66\n1\n70\n1\n", TO_UTF8( cname ));
            fprintf( outputFile, "40\n%g\n41\n%g\n", radius, radius);
            fprintf( outputFile, "0\nVERTEX\n8\n%s\n", TO_UTF8( cname ));
            fprintf( outputFile, "10\n%g\n 20\n%g\n42\n1.0\n",
                    centre_dev.x-r, centre_dev.y );
            fprintf( outputFile, "0\nVERTEX\n8\n%s\n", TO_UTF8( cname ));
            fprintf( outputFile, "10\n%g\n 20\n%g\n42\n1.0\n",
                    centre_dev.x+r, centre_dev.y );
            fprintf( outputFile, "0\nSEQEND\n");
        }
    }
}


/**
 * DXF polygon: doesn't fill it but at least it close the filled ones
 */
void DXF_PLOTTER::PlotPoly( const std::vector< wxPoint >& aCornerList,
                            FILL_T aFill, int aWidth)
{
    if( aCornerList.size() <= 1 )
        return;

    MoveTo( aCornerList[0] );
    for( unsigned ii = 1; ii < aCornerList.size(); ii++ )
        LineTo( aCornerList[ii] );

    // Close polygon if 'fill' requested
    if( aFill )
    {
        unsigned ii = aCornerList.size() - 1;
        if( aCornerList[ii] != aCornerList[0] )
            LineTo( aCornerList[0] );
    }
    PenFinish();
}


void DXF_PLOTTER::PenTo( const wxPoint& pos, char plume )
{
    wxASSERT( outputFile );
    if( plume == 'Z' )
    {
        return;
    }
    DPOINT pos_dev = userToDeviceCoordinates( pos );
    DPOINT pen_lastpos_dev = userToDeviceCoordinates( penLastpos );

    if( penLastpos != pos && plume == 'D' )
    {
        // DXF LINE
        wxString cname = ColorRefs[currentColor].m_Name;
        fprintf( outputFile, "0\nLINE\n8\n%s\n10\n%g\n20\n%g\n11\n%g\n21\n%g\n",
                 TO_UTF8( cname ),
                 pen_lastpos_dev.x, pen_lastpos_dev.y, pos_dev.x, pos_dev.y );
    }
    penLastpos = pos;
}


/**
 * Dashed lines are not (yet) supported by DXF_PLOTTER
 */
void DXF_PLOTTER::SetDash( bool dashed )
{
    // NOP for now
}


void DXF_PLOTTER::ThickSegment( const wxPoint& aStart, const wxPoint& aEnd, int aWidth,
                                EDA_DRAW_MODE_T aPlotMode )
{
    if( aPlotMode == LINE )  // In line mode, just a line is OK
    {
        MoveTo( aStart );
        FinishTo( aEnd );
    }
    else
    {
        segmentAsOval( aStart, aEnd, aWidth, aPlotMode );
    }
}

/** Plot an arc in DXF format
 * Filling is not supported
 */
void DXF_PLOTTER::Arc( const wxPoint& centre, int StAngle, int EndAngle, int radius,
                       FILL_T fill, int width )
{
    wxASSERT( outputFile );

    if( radius <= 0 )
        return;

    DPOINT centre_dev = userToDeviceCoordinates( centre );
    double radius_dev = userToDeviceSize( radius );

    // Emit a DXF ARC entity
    wxString cname = ColorRefs[currentColor].m_Name;
    fprintf( outputFile,
             "0\nARC\n8\n%s\n10\n%g\n20\n%g\n40\n%g\n50\n%g\n51\n%g\n",
             TO_UTF8( cname ),
             centre_dev.x, centre_dev.y, radius_dev,
             StAngle / 10.0, EndAngle / 10.0 );
}

/**
 * DXF oval pad: always done in sketch mode
 */
void DXF_PLOTTER::FlashPadOval( const wxPoint& pos, const wxSize& aSize, int orient,
                                EDA_DRAW_MODE_T trace_mode )
{
    wxASSERT( outputFile );
    wxSize size( aSize );

    /* The chip is reduced to an oval tablet with size.y > size.x
     * (Oval vertical orientation 0) */
    if( size.x > size.y )
    {
        EXCHG( size.x, size.y );
        orient += 900;
        if( orient >= 3600 )
            orient -= 3600;
    }
    sketchOval( pos, size, orient, -1 );
}


/**
 * DXF round pad: always done in sketch mode; it could be filled but it isn't
 * pretty if other kinds of pad aren't...
 */
void DXF_PLOTTER::FlashPadCircle( const wxPoint& pos, int diametre,
                                    EDA_DRAW_MODE_T trace_mode )
{
    wxASSERT( outputFile );
    Circle( pos, diametre, NO_FILL );
}


/**
 * DXF rectangular pad: alwayd done in sketch mode
 */
void DXF_PLOTTER::FlashPadRect( const wxPoint& pos, const wxSize& padsize,
                                int orient, EDA_DRAW_MODE_T trace_mode )
{
    wxASSERT( outputFile );
    wxSize size;
    int    ox, oy, fx, fy;

    size.x = padsize.x / 2;
    size.y = padsize.y / 2;

    if( size.x < 0 )
        size.x = 0;
    if( size.y < 0 )
        size.y = 0;

    // If a dimension is zero, the trace is reduced to 1 line
    if( size.x == 0 )
    {
        ox = pos.x;
        oy = pos.y - size.y;
        RotatePoint( &ox, &oy, pos.x, pos.y, orient );
        fx = pos.x;
        fy = pos.y + size.y;
        RotatePoint( &fx, &fy, pos.x, pos.y, orient );
        MoveTo( wxPoint( ox, oy ) );
        FinishTo( wxPoint( fx, fy ) );
        return;
    }
    if( size.y == 0 )
    {
        ox = pos.x - size.x;
        oy = pos.y;
        RotatePoint( &ox, &oy, pos.x, pos.y, orient );
        fx = pos.x + size.x;
        fy = pos.y;
        RotatePoint( &fx, &fy, pos.x, pos.y, orient );
        MoveTo( wxPoint( ox, oy ) );
        FinishTo( wxPoint( fx, fy ) );
        return;
    }

    ox = pos.x - size.x;
    oy = pos.y - size.y;
    RotatePoint( &ox, &oy, pos.x, pos.y, orient );
    MoveTo( wxPoint( ox, oy ) );

    fx = pos.x - size.x;
    fy = pos.y + size.y;
    RotatePoint( &fx, &fy, pos.x, pos.y, orient );
    LineTo( wxPoint( fx, fy ) );

    fx = pos.x + size.x;
    fy = pos.y + size.y;
    RotatePoint( &fx, &fy, pos.x, pos.y, orient );
    LineTo( wxPoint( fx, fy ) );

    fx = pos.x + size.x;
    fy = pos.y - size.y;
    RotatePoint( &fx, &fy, pos.x, pos.y, orient );
    LineTo( wxPoint( fx, fy ) );

    FinishTo( wxPoint( ox, oy ) );
}


/**
 * DXF trapezoidal pad: only sketch mode is supported
 */
void DXF_PLOTTER::FlashPadTrapez( const wxPoint& aPadPos, const wxPoint *aCorners,
                                  int aPadOrient, EDA_DRAW_MODE_T aTrace_Mode )
{
    wxASSERT( outputFile );
    wxPoint coord[4];       /* coord actual corners of a trapezoidal trace */

    for( int ii = 0; ii < 4; ii++ )
    {
        coord[ii] = aCorners[ii];
        RotatePoint( &coord[ii], aPadOrient );
        coord[ii] += aPadPos;
    }

    // Plot edge:
    MoveTo( coord[0] );
    LineTo( coord[1] );
    LineTo( coord[2] );
    LineTo( coord[3] );
    FinishTo( coord[0] );
}

/**
 * Checks if a given string contains non-ASCII characters.
 * FIXME: the performance of this code is really poor, but in this case it can be
 * acceptable because the plot operation is not called very often.
 * @param string String to check
 * @return true if it contains some non-ASCII character, false if all characters are
 *         inside ASCII range (<=255).
 */
bool containsNonAsciiChars( const wxString& string )
{
    for( unsigned i = 0; i < string.length(); i++ )
    {
        wchar_t ch = string[i];
        if( ch > 255 )
            return true;
    }
    return false;
}

void DXF_PLOTTER::Text( const wxPoint&              aPos,
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
    if( textAsLines || containsNonAsciiChars( aText ) )
        /* output text as graphics */
        PLOTTER::Text( aPos, aColor, aText, aOrient, aSize, aH_justify, aV_justify,
                aWidth, aItalic, aBold );
    else
    {
        /* Emit text as a text entity. This loses formatting and shape but it's
           more useful as a CAD object */
        DPOINT origin_dev = userToDeviceCoordinates( aPos );
        SetColor( aColor );
        wxString cname = ColorRefs[currentColor].m_Name;
        DPOINT size_dev = userToDeviceSize( aSize );
        int h_code = 0, v_code = 0;
        switch( aH_justify )
        {
        case GR_TEXT_HJUSTIFY_LEFT:
            h_code = 0;
            break;
        case GR_TEXT_HJUSTIFY_CENTER:
            h_code = 1;
            break;
        case GR_TEXT_HJUSTIFY_RIGHT:
            h_code = 2;
            break;
        }
        switch( aV_justify )
        {
        case GR_TEXT_VJUSTIFY_TOP:
            v_code = 3;
            break;
        case GR_TEXT_VJUSTIFY_CENTER:
            v_code = 2;
            break;
        case GR_TEXT_VJUSTIFY_BOTTOM:
            v_code = 1;
            break;
        }

        // Position, size, rotation and alignment
        // The two alignment point usages is somewhat idiot (see the DXF ref)
        // Anyway since we don't use the fit/aligned options, they're the same
        fprintf( outputFile,
                "  0\n"
                "TEXT\n"
                "  7\n"
                "%s\n"          // Text style
                "  8\n"
                "%s\n"          // Layer name
                "  10\n"
                "%g\n"          // First point X
                "  11\n"
                "%g\n"          // Second point X
                "  20\n"
                "%g\n"          // First point Y
                "  21\n"
                "%g\n"          // Second point Y
                "  40\n"
                "%g\n"          // Text height
                "  41\n"
                "%g\n"          // Width factor
                "  50\n"
                "%g\n"          // Rotation
                "  51\n"
                "%g\n"          // Oblique angle
                "  71\n"
                "%d\n"          // Mirror flags
                "  72\n"
                "%d\n"          // H alignment
                "  73\n"
                "%d\n",         // V alignment
                aBold ? (aItalic ? "KICADBI" : "KICADB")
                      : (aItalic ? "KICADI" : "KICAD"),
                TO_UTF8( cname ),
                origin_dev.x, origin_dev.x,
                origin_dev.y, origin_dev.y,
                size_dev.y, fabs( size_dev.x / size_dev.y ),
                aOrient / 10.0,
                aItalic ? DXF_OBLIQUE_ANGLE : 0,
                size_dev.x < 0 ? 2 : 0, // X mirror flag
                h_code, v_code );

        /* There are two issue in emitting the text:
           - Our overline character (~) must be converted to the appropriate
           control sequence %%O or %%o
           - Text encoding in DXF is more or less unspecified since depends on
           the DXF declared version, the acad version reading it *and* some
           system variables to be put in the header handled only by newer acads
           Also before R15 unicode simply is not supported (you need to use
           bigfonts which are a massive PITA). Common denominator solution:
           use Latin1 (and however someone could choke on it, anyway). Sorry
           for the extended latin people. If somewant want to try fixing this
           recent version seems to use UTF-8 (and not UCS2 like the rest of
           Windows)

           XXX Actually there is a *third* issue: older DXF formats are limited
           to 255 bytes records (it was later raised to 2048); since I'm lazy
           and text so long is not probable I just don't implement this rule.
           If someone is interested in fixing this, you have to emit the first
           partial lines with group code 3 (max 250 bytes each) and then finish
           with a group code 1 (less than 250 bytes). The DXF refs explains it
           in no more details...
         */

        bool overlining = false;
        fputs( "  1\n", outputFile );
        for( unsigned i = 0; i < aText.length(); i++ )
        {
            /* Here I do a bad thing: writing the output one byte at a time!
               but today I'm lazy and I have no idea on how to coerce a Unicode
               wxString to spit out latin1 encoded text ...

               Atleast stdio is *supposed* to do output buffering, so there is
               hope is not too slow */
            wchar_t ch = aText[i];
            if( ch > 255 )
            {
                // I can't encode this...
                putc( '?', outputFile );
            }
            else
            {
                if( ch == '~' )
                {
                    // Handle the overline toggle
                    fputs( overlining ? "%%o" : "%%O", outputFile );
                    overlining = !overlining;
                }
                else
                {
                    putc( ch, outputFile );
                }
            }
        }
        putc( '\n', outputFile );
    }
}

