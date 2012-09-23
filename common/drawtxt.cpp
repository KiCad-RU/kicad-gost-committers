/**
 * Functions to draw and plot text on screen
 * @file drawtxt.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 Wayne Stambaugh <stambaughw@verizon.net>
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
#include <gr_basic.h>
#include <common.h>
#include <plot_common.h>
#include <eda_text.h>               // EDA_TEXT_HJUSTIFY_T and EDA_TEXT_VJUSTIFY_T
#include <trigo.h>
#include <macros.h>
#include <class_drawpanel.h>
#include <class_base_screen.h>

#ifndef DEFAULT_SIZE_TEXT
#   define DEFAULT_SIZE_TEXT 50
#endif

#define EDA_DRAWBASE
#include <newstroke_font.h>
#include <plot_common.h>

/* factor used to calculate actual size of shapes from hershey fonts (could be adjusted depending on the font name)
 * Its value is choosen in order to have letters like M, P .. vertical size equal to the vertical char size parameter
 * Of course some shapes can be bigger or smaller than the vertical char size parameter
 */
#define HERSHEY_SCALE_FACTOR 1 / 21.0
double s_HerscheyScaleFactor = HERSHEY_SCALE_FACTOR;


/* Helper function for texts with over bar
 */
int OverbarPositionY( int size_v, int thickness )
{
    return KiROUND( ( (double) size_v * 1.1 ) + ( (double) thickness * 1.5 ) );
}

/**
 * Function GetPensizeForBold
 * @return the "best" value for a pen size to draw/plot a bold text
 * @param aTextSize = the char size (height or width)
 */
int GetPenSizeForBold( int aTextSize )
{
    return KiROUND( aTextSize / 5.0 );
}


/**
 * Function  Clamp_Text_PenSize
 * As a rule, pen width should not be >1/4em, otherwise the character
 * will be cluttered up in its own fatness
 * so pen width max is aSize/4 for bold text, and aSize/6 for normal text
 * The "best" pen width is aSize/5 for bold texts,
 * so the clamp is consistant with bold option.
 * @param aPenSize = the pen size to clamp
 * @param aSize the char size (height or width)
 * @param aBold = true if text accept bold pen size
 * @return the max pen size allowed
 */
int Clamp_Text_PenSize( int aPenSize, int aSize, bool aBold )
{
    int    penSize  = aPenSize;
    double scale    = aBold ? 4.0 : 6.0;
    int    maxWidth = KiROUND( std::abs( aSize ) / scale );

    if( penSize > maxWidth )
        penSize = maxWidth;
    return penSize;
}


int Clamp_Text_PenSize( int aPenSize, wxSize aSize, bool aBold )
{
    int size = std::min( std::abs( aSize.x ), std::abs( aSize.y ) );

    return Clamp_Text_PenSize( aPenSize, size, aBold );
}


/* Functions to draw / plot a string.
 *  texts have only one line.
 *  They can be in italic.
 *  Horizontal and Vertical justification are handled.
 *  Texts can be rotated
 *  substrings between ~ markers can be "negated" (i.e. with an over bar
 */

/**
 * Function NegableTextLength
 * Return the text length of a negable string, excluding the ~ markers */
int NegableTextLength( const wxString& aText )
{
    int char_count = aText.length();

    /* Fix the character count, removing the ~ found */
    for( int i = char_count - 1; i >= 0; i-- )
    {
        if( aText[i] == '~' )
        {
            char_count--;
        }
    }

    return char_count;
}


/* Function GetHersheyShapeDescription
 * return a pointer to the shape corresponding to unicode value AsciiCode
 * Note we use the same font for Bold and Normal texts
 * because KiCad handles a variable pen size to do that
 * that gives better results in XOR draw mode.
 */
static const char* GetHersheyShapeDescription( int AsciiCode )
{
    /* calculate font length */
    int font_length_max = newstroke_font_bufsize;

    if( AsciiCode >= (32 + font_length_max) )
        AsciiCode = '?';
    if( AsciiCode < 32 )
        AsciiCode = 32;                 /* Clamp control chars */
    AsciiCode -= 32;

    return newstroke_font[AsciiCode];
}


int ReturnGraphicTextWidth( const wxString& aText, int aXSize, bool aItalic, bool aWidth )
{
    int tally = 0;
    int char_count = aText.length();

    for( int i = 0; i < char_count; i++ )
    {
        int AsciiCode = aText[i];

        if( AsciiCode == '~' ) /* Skip the negation marks */
        {
            continue;
        }

        const char* ptcar = GetHersheyShapeDescription( AsciiCode );
        /* Get metrics */
        int         xsta = *ptcar++ - 'R';
        int         xsto = *ptcar++ - 'R';
        tally += KiROUND( aXSize * (xsto - xsta) * s_HerscheyScaleFactor );
    }

    /* Italic correction, 1/8em */
    if( aItalic )
    {
        tally += KiROUND( aXSize * 0.125 );
    }
    return tally;
}


/* Helper function for drawing character polygons */
static void DrawGraphicTextPline(
    EDA_RECT* aClipBox,
    wxDC* aDC,
    EDA_COLOR_T aColor,
    int aWidth,
    bool aSketchMode,
    int point_count,
    wxPoint* coord,
    void (* aCallback)(int x0, int y0, int xf, int yf ),
    PLOTTER* aPlotter )
{
    if( aPlotter )
    {
        aPlotter->MoveTo( coord[0] );
        for( int ik = 1; ik < point_count; ik++ )
        {
            aPlotter->LineTo( coord[ik] );
        }

        aPlotter->PenFinish();
    }
    else if( aCallback )
    {
        for( int ik = 0; ik < (point_count - 1); ik++ )
        {
            aCallback( coord[ik].x, coord[ik].y,
                       coord[ik + 1].x, coord[ik + 1].y );
        }
    }
    else if( aSketchMode )
    {
        for( int ik = 0; ik < (point_count - 1); ik++ )
            GRCSegm( aClipBox, aDC, coord[ik].x, coord[ik].y,
                     coord[ik + 1].x, coord[ik + 1].y, aWidth, aColor );
    }
    else
        GRPoly( aClipBox, aDC, point_count, coord, 0,
                aWidth, aColor, aColor );
}


/**
 * Function DrawGraphicText
 * Draw a graphic text (like module texts)
 *  @param aPanel = the current m_canvas. NULL if draw within a 3D GL Canvas
 *  @param aDC = the current Device Context. NULL if draw within a 3D GL Canvas
 *  @param aPos = text position (according to h_justify, v_justify)
 *  @param aColor (enum EDA_COLOR_T) = text color
 *  @param aText = text to draw
 *  @param aOrient = angle in 0.1 degree
 *  @param aSize = text size (size.x or size.y can be < 0 for mirrored texts)
 *  @param aH_justify = horizontal justification (Left, center, right)
 *  @param aV_justify = vertical justification (bottom, center, top)
 *  @param aWidth = line width (pen width) (use default width if aWidth = 0)
 *      if width < 0 : draw segments in sketch mode, width = abs(width)
 *      Use a value min(aSize.x, aSize.y) / 5 for a bold text
 *  @param aItalic = true to simulate an italic font
 *  @param aBold = true to use a bold font. Useful only with default width value (aWidth = 0)
 *  @param aCallback() = function called (if non null) to draw each segment.
 *                  used to draw 3D texts or for plotting, NULL for normal drawings
 *  @param aPlotter = a pointer to a PLOTTER instance, when this function is used to plot
 *                  the text. NULL to draw this text.
 */
void DrawGraphicText( EDA_DRAW_PANEL* aPanel,
                      wxDC* aDC,
                      const wxPoint& aPos,
                      EDA_COLOR_T aColor,
                      const wxString& aText,
                      int aOrient,
                      const wxSize& aSize,
                      enum EDA_TEXT_HJUSTIFY_T aH_justify,
                      enum EDA_TEXT_VJUSTIFY_T aV_justify,
                      int aWidth,
                      bool aItalic,
                      bool aBold,
                      void (* aCallback)( int x0, int y0, int xf, int yf ),
                      PLOTTER* aPlotter )
{
    int       AsciiCode;
    int       x0, y0;
    int       size_h, size_v;
    unsigned  ptr;
    int       dx, dy;                       // Draw coordinate for segments to draw. also used in some other calculation
    wxPoint   current_char_pos;             // Draw coordinates for the current char
    wxPoint   overbar_pos;                  // Start point for the current overbar
    int       overbar_italic_comp;          // Italic compensation for overbar
    EDA_RECT* clipBox;                      // Clip box used in basic draw functions

    clipBox = aPanel ? aPanel->GetClipBox() : NULL;
    #define        BUF_SIZE 100
    wxPoint coord[BUF_SIZE + 1];                // Buffer coordinate used to draw polylines (one char shape)
    bool    sketch_mode    = false;
    bool    italic_reverse = false;             // true for mirrored texts with m_Size.x < 0

    size_h = aSize.x;                           /* PLEASE NOTE: H is for HORIZONTAL not for HEIGHT */
    size_v = aSize.y;

    if( aWidth == 0 && aBold )       // Use default values if aWidth == 0
        aWidth = GetPenSizeForBold( std::min( aSize.x, aSize.y ) );

    if( aWidth < 0 )
    {
        aWidth = -aWidth;
        sketch_mode = true;
    }

#ifdef CLIP_PEN      // made by draw and plot functions
    aWidth = Clamp_Text_PenSize( aWidth, aSize, aBold );
#endif

    if( size_h < 0 )       // text is mirrored using size.x < 0 (mirror / Y axis)
        italic_reverse = true;

    unsigned char_count = NegableTextLength( aText );
    if( char_count == 0 )
        return;

    current_char_pos = aPos;

    dx = ReturnGraphicTextWidth( aText, size_h, aItalic, aWidth );
    dy = size_v;

    /* Do not draw the text if out of draw area! */
    if( aPanel )
    {
        int xm, ym, ll, xc, yc;
        ll = std::abs( dx );

        xc = current_char_pos.x;
        yc = current_char_pos.y;

        x0 = aPanel->GetClipBox()->GetX() - ll;
        y0 = aPanel->GetClipBox()->GetY() - ll;
        xm = aPanel->GetClipBox()->GetRight() + ll;
        ym = aPanel->GetClipBox()->GetBottom() + ll;

        if( xc < x0 )
            return;
        if( yc < y0 )
            return;
        if( xc > xm )
            return;
        if( yc > ym )
            return;
    }


    /* Compute the position of the first letter of the text
     * this position is the position of the left bottom point of the letter
     * this is the same as the text position only for a left and bottom justified text
     * In others cases, this position must be calculated from the text position ans size
     */

    switch( aH_justify )
    {
    case GR_TEXT_HJUSTIFY_CENTER:
        current_char_pos.x -= dx / 2;
        break;

    case GR_TEXT_HJUSTIFY_RIGHT:
        current_char_pos.x -= dx;
        break;

    case GR_TEXT_HJUSTIFY_LEFT:
        break;
    }

    switch( aV_justify )
    {
    case GR_TEXT_VJUSTIFY_CENTER:
        current_char_pos.y += dy / 2;
        break;

    case GR_TEXT_VJUSTIFY_TOP:
        current_char_pos.y += dy;
        break;

    case GR_TEXT_VJUSTIFY_BOTTOM:
        break;
    }

    // Note: if aPanel == NULL, we are using a GL Canvas that handle scaling
    if( aSize.x == 0 )
        return;

    /* if a text size is too small, the text cannot be drawn, and it is drawn as a single
     * graphic line */
    if( std::abs( aSize.x ) < 3 )
    {
        /* draw the text as a line always vertically centered */
        wxPoint end( current_char_pos.x + dx, current_char_pos.y );

        RotatePoint( &current_char_pos, aPos, aOrient );
        RotatePoint( &end, aPos, aOrient );

        if( aPlotter )
        {
            aPlotter->MoveTo( current_char_pos );
            aPlotter->FinishTo( end );
        }
        else if( aCallback )
        {
            aCallback( current_char_pos.x, current_char_pos.y, end.x, end.y );
        }
        else
            GRLine( clipBox, aDC,
                    current_char_pos.x, current_char_pos.y, end.x, end.y, aWidth, aColor );

        return;
    }

    if( aItalic )
    {
        overbar_italic_comp = OverbarPositionY( size_v, aWidth ) / 8;
        if( italic_reverse )
        {
            overbar_italic_comp = -overbar_italic_comp;
        }
    }
    else
    {
        overbar_italic_comp = 0;
    };

    int overbars = 0;       // Number of ~ seen
    ptr = 0;   /* ptr = text index */
    while( ptr < char_count )
    {
        if( aText[ptr + overbars] == '~' )
        {
            /* Found an overbar, adjust the pointers */
            overbars++;

            if( overbars & 1 )      // odd overbars count
            {
                /* Starting the overbar */
                overbar_pos    = current_char_pos;
                overbar_pos.x += overbar_italic_comp;
                overbar_pos.y -= OverbarPositionY( size_v, aWidth );
                RotatePoint( &overbar_pos, aPos, aOrient );
            }
            else
            {
                /* Ending the overbar */
                coord[0]       = overbar_pos;
                overbar_pos    = current_char_pos;
                overbar_pos.x += overbar_italic_comp;
                overbar_pos.y -= OverbarPositionY( size_v, aWidth );
                RotatePoint( &overbar_pos, aPos, aOrient );
                coord[1] = overbar_pos;
                /* Plot the overbar segment */
                DrawGraphicTextPline( clipBox, aDC, aColor, aWidth,
                                      sketch_mode, 2, coord, aCallback, aPlotter );
            }
            continue; /* Skip ~ processing */
        }

        AsciiCode = aText.GetChar( ptr + overbars );

        const char* ptcar = GetHersheyShapeDescription( AsciiCode );
        /* Get metrics */
        int         xsta = *ptcar++ - 'R';
        int         xsto = *ptcar++ - 'R';
        int         point_count = 0;
        bool        endcar = false;
        while( !endcar )
        {
            int hc1, hc2;
            hc1 = *ptcar++;
            if( hc1 )
            {
                hc2 = *ptcar++;
            }
            else
            {
                // End of character, insert a synthetic pen up:
                hc1    = ' ';
                hc2    = 'R';
                endcar = true;
            }
            // Do the Hershey decode thing: coordinates values are coded as <value> + 'R'
            hc1 -= 'R'; hc2 -= 'R';

            /* Pen up request */
            if( hc1 == -50 && hc2 == 0 )
            {
                if( point_count )
                {
                    if( aWidth <= 1 )
                        aWidth = 0;
                    DrawGraphicTextPline( clipBox, aDC, aColor, aWidth,
                                          sketch_mode, point_count, coord,
                                          aCallback, aPlotter );
                }
                point_count = 0;
            }
            else
            {
                wxPoint currpoint;
                hc1 -= xsta; hc2 -= 11; /* Align the midpoint */
                hc1  = KiROUND( hc1 * size_h * s_HerscheyScaleFactor );
                hc2  = KiROUND( hc2 * size_v * s_HerscheyScaleFactor );

                // To simulate an italic font, add a x offset depending on the y offset
                if( aItalic )
                    hc1 -= KiROUND( italic_reverse ? -hc2 / 8.0 : hc2 / 8.0 );
                currpoint.x = hc1 + current_char_pos.x;
                currpoint.y = hc2 + current_char_pos.y;

                RotatePoint( &currpoint, aPos, aOrient );
                coord[point_count] = currpoint;
                if( point_count < BUF_SIZE - 1 )
                    point_count++;
            }
        }

        /* end draw 1 char */

        ptr++;

        // Apply the advance width
        current_char_pos.x += KiROUND( size_h * (xsto - xsta) * s_HerscheyScaleFactor );
    }

    if( overbars % 2 )
    {
        /* Close the last overbar */
        coord[0]       = overbar_pos;
        overbar_pos    = current_char_pos;
        overbar_pos.y -= OverbarPositionY( size_v, aWidth );
        RotatePoint( &overbar_pos, aPos, aOrient );
        coord[1] = overbar_pos;
        /* Plot the overbar segment */
        DrawGraphicTextPline( clipBox, aDC, aColor, aWidth,
                              sketch_mode, 2, coord, aCallback, aPlotter );
    }
}


/**
 * Function PlotGraphicText
 *  same as DrawGraphicText, but plot graphic text insteed of draw it
 *  @param aPos = text position (according to aH_justify, aV_justify)
 *  @param aColor (enum EDA_COLOR_T) = text color
 *  @param aText = text to draw
 *  @param aOrient = angle in 0.1 degree
 *  @param aSize = text size (size.x or size.y can be < 0 for mirrored texts)
 *  @param aH_justify = horizontal justification (Left, center, right)
 *  @param aV_justify = vertical justification (bottom, center, top)
 *  @param aWidth = line width (pen width) (default = 0)
 *      if width < 0 : draw segments in sketch mode, width = abs(width)
 *      Use a value min(aSize.x, aSize.y) / 5 for a bold text
 *  @param aItalic = true to simulate an italic font
 *  @param aBold = true to use a bold font Useful only with default width value (aWidth = 0)
 */
void PLOTTER::Text( const wxPoint&              aPos,
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
    int textPensize = aWidth;

    if( textPensize == 0 && aBold )      // Use default values if aWidth == 0
        textPensize = GetPenSizeForBold( std::min( aSize.x, aSize.y ) );

    if( textPensize >= 0 )
        textPensize = Clamp_Text_PenSize( aWidth, aSize, aBold );
    else
        textPensize = -Clamp_Text_PenSize( -aWidth, aSize, aBold );

    SetCurrentLineWidth( textPensize );


    if( aColor >= 0 )
        SetColor( aColor );

    DrawGraphicText( NULL, NULL, aPos, aColor, aText,
                     aOrient, aSize,
                     aH_justify, aV_justify,
                     textPensize, aItalic,
                     aBold,
                     NULL,
                     this );

    if( aWidth != textPensize )
        SetCurrentLineWidth( aWidth );
}
