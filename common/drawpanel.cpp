/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jean-pierre.charras@gipsa-lab.inpg.fr
 * Copyright (C) 2007-2011 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file drawpanel.cpp
 */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <gr_basic.h>
#include <common.h>
#include <macros.h>
#include <id.h>
#include <class_drawpanel.h>
#include <class_base_screen.h>
#include <wxstruct.h>

#include <kicad_device_context.h>

static const int CURSOR_SIZE = 12; ///< Cursor size in pixels

#define CLIP_BOX_PADDING 2

// keys to store options in config:
#define ENBL_ZOOM_NO_CENTER_KEY wxT( "ZoomNoCenter" )
#define ENBL_MIDDLE_BUTT_PAN_KEY wxT( "MiddleButtonPAN" )
#define MIDDLE_BUTT_PAN_LIMITED_KEY wxT( "MiddleBtnPANLimited" )
#define ENBL_AUTO_PAN_KEY wxT( "AutoPAN" )

/* Definitions for enabling and disabling debugging features in drawpanel.cpp.
 * Please don't forget to turn these off before making any commits to Launchpad.
 */
#define DEBUG_SHOW_CLIP_RECT       0  // Set to 1 to draw clipping rectangle.

/**
 * Trace mask used to enable or disable the trace output of coordinates during drawing
 * functions.  The coordinate dumping can be turned on by setting the WXTRACE environment
 * variable to "kicad_dump_coords".  See the wxWidgets documentation on wxLogTrace for
 * more information.
 */
#define KICAD_TRACE_COORDS wxT( "kicad_dump_coords" )


// Events used by EDA_DRAW_PANEL
BEGIN_EVENT_TABLE( EDA_DRAW_PANEL, wxScrolledWindow )
    EVT_LEAVE_WINDOW( EDA_DRAW_PANEL::OnMouseLeaving )
    EVT_MOUSEWHEEL( EDA_DRAW_PANEL::OnMouseWheel )
    EVT_MOUSE_EVENTS( EDA_DRAW_PANEL::OnMouseEvent )
    EVT_CHAR( EDA_DRAW_PANEL::OnKeyEvent )
    EVT_CHAR_HOOK( EDA_DRAW_PANEL::OnCharHook )
    EVT_PAINT( EDA_DRAW_PANEL::OnPaint )
    EVT_ERASE_BACKGROUND( EDA_DRAW_PANEL::OnEraseBackground )
    EVT_SCROLLWIN( EDA_DRAW_PANEL::OnScroll )
    EVT_ACTIVATE( EDA_DRAW_PANEL::OnActivate )
    EVT_MENU_RANGE( ID_PAN_UP, ID_PAN_RIGHT, EDA_DRAW_PANEL::OnPan )
END_EVENT_TABLE()


/***********************************************************************/
/* EDA_DRAW_PANEL base functions (EDA_DRAW_PANEL is the main panel)*/
/***********************************************************************/

EDA_DRAW_PANEL::EDA_DRAW_PANEL( EDA_DRAW_FRAME* parent, int id,
                                const wxPoint& pos, const wxSize& size ) :
    wxScrolledWindow( parent, id, pos, size, wxBORDER | wxHSCROLL | wxVSCROLL | wxALWAYS_SHOW_SB )
{
    wxASSERT( parent );

    m_scrollIncrementX = std::min( size.x / 8, 10 );
    m_scrollIncrementY = std::min( size.y / 8, 10 );

    SetBackgroundColour( MakeColour( g_DrawBgColor ) );

#if KICAD_USE_BUFFERED_DC || KICAD_USE_BUFFERED_PAINTDC
    SetBackgroundStyle( wxBG_STYLE_CUSTOM );
#endif

    m_ClipBox.SetSize( size );
    m_ClipBox.SetX( 0 );
    m_ClipBox.SetY( 0 );
    m_canStartBlock = -1;       // Command block can start if >= 0
    m_abortRequest = false;
    m_enableMiddleButtonPan = false;
    m_enableZoomNoCenter = false;
    m_panScrollbarLimits = false;
    m_enableAutoPan = true;
    m_ignoreMouseEvents = false;
    m_ignoreNextLeftButtonRelease = false;

    m_mouseCaptureCallback = NULL;
    m_endMouseCaptureCallback = NULL;

    if( wxGetApp().GetSettings() )
    {
        wxGetApp().GetSettings()->Read( ENBL_MIDDLE_BUTT_PAN_KEY, &m_enableMiddleButtonPan, false );
        wxGetApp().GetSettings()->Read( ENBL_ZOOM_NO_CENTER_KEY, &m_enableZoomNoCenter, false );
        wxGetApp().GetSettings()->Read( MIDDLE_BUTT_PAN_LIMITED_KEY, &m_panScrollbarLimits, false );
        wxGetApp().GetSettings()->Read( ENBL_AUTO_PAN_KEY, &m_enableAutoPan, true );
    }

    m_requestAutoPan = false;
    m_enableBlockCommands = false;
    m_minDragEventCount = 0;

#ifdef __WXMAC__
    m_defaultCursor = m_currentCursor = wxCURSOR_CROSS;
    m_showCrossHair = false;
#else
    m_defaultCursor = m_currentCursor = wxCURSOR_ARROW;
    m_showCrossHair = true;
#endif

    m_cursorLevel = 0;
    m_PrintIsMirrored = false;
}


EDA_DRAW_PANEL::~EDA_DRAW_PANEL()
{
    wxGetApp().GetSettings()->Write( ENBL_MIDDLE_BUTT_PAN_KEY, m_enableMiddleButtonPan );
    wxGetApp().GetSettings()->Write( ENBL_ZOOM_NO_CENTER_KEY, m_enableZoomNoCenter );
    wxGetApp().GetSettings()->Write( MIDDLE_BUTT_PAN_LIMITED_KEY, m_panScrollbarLimits );
    wxGetApp().GetSettings()->Write( ENBL_AUTO_PAN_KEY, m_enableAutoPan );
}

EDA_DRAW_FRAME* EDA_DRAW_PANEL::GetParent()
{
    return ( EDA_DRAW_FRAME* ) wxWindow::GetParent();
}


BASE_SCREEN* EDA_DRAW_PANEL::GetScreen()
{
    EDA_DRAW_FRAME* parentFrame = GetParent();

    return parentFrame->GetScreen();
}

wxPoint EDA_DRAW_PANEL::ToDeviceXY( const wxPoint& pos )
{
    wxPoint ret;
    INSTALL_UNBUFFERED_DC( dc, this );
    ret.x = dc.LogicalToDeviceX( pos.x );
    ret.y = dc.LogicalToDeviceY( pos.y );
    return ret;
}

wxPoint EDA_DRAW_PANEL::ToLogicalXY( const wxPoint& pos )
{
    wxPoint ret;
    INSTALL_UNBUFFERED_DC( dc, this );
    ret.x = dc.DeviceToLogicalX( pos.x );
    ret.y = dc.DeviceToLogicalY( pos.y );
    return ret;
}

void EDA_DRAW_PANEL::DrawCrossHair( wxDC* aDC, EDA_COLOR_T aColor )
{
    if( m_cursorLevel != 0 || aDC == NULL || !m_showCrossHair )
        return;

    wxPoint cursor = GetScreen()->GetCrossHairPosition();

    GRSetDrawMode( aDC, GR_XOR );

    if( GetParent()->m_cursorShape != 0 )    // Draws full screen crosshair.
    {
        wxSize  clientSize = GetClientSize();

        // Y axis
        wxPoint lineStart( cursor.x, aDC->DeviceToLogicalY( 0 ) );
        wxPoint lineEnd(   cursor.x, aDC->DeviceToLogicalY( clientSize.y ) );

        GRLine( &m_ClipBox, aDC, lineStart, lineEnd, 0, aColor );

        // X axis
        lineStart = wxPoint( aDC->DeviceToLogicalX( 0 ), cursor.y );
        lineEnd   = wxPoint( aDC->DeviceToLogicalX( clientSize.x ), cursor.y );

        GRLine( &m_ClipBox, aDC, lineStart, lineEnd, 0, aColor );
    }
    else
    {
        int len = aDC->DeviceToLogicalXRel( CURSOR_SIZE );

        GRLine( &m_ClipBox, aDC, cursor.x - len, cursor.y,
                cursor.x + len, cursor.y, 0, aColor );
        GRLine( &m_ClipBox, aDC, cursor.x, cursor.y - len,
                cursor.x, cursor.y + len, 0, aColor );
    }
}


void EDA_DRAW_PANEL::CrossHairOff( wxDC* DC )
{
    DrawCrossHair( DC );
    --m_cursorLevel;
}


void EDA_DRAW_PANEL::CrossHairOn( wxDC* DC )
{
    ++m_cursorLevel;
    DrawCrossHair( DC );

    if( m_cursorLevel > 0 )  // Shouldn't happen, but just in case ..
        m_cursorLevel = 0;
}


double EDA_DRAW_PANEL::GetZoom()
{
    return GetScreen()->GetZoom();
}


void EDA_DRAW_PANEL::SetZoom( double zoom )
{
    GetScreen()->SetZoom( zoom );
}


wxRealPoint EDA_DRAW_PANEL::GetGrid()
{
    return GetScreen()->GetGridSize();
}


bool EDA_DRAW_PANEL::IsPointOnDisplay( const wxPoint& aPosition )
{
    wxPoint  pos;
    EDA_RECT display_rect;

    INSTALL_UNBUFFERED_DC( dc, this );  // Refresh the clip box to the entire screen size.
    SetClipBox( dc );

    display_rect = m_ClipBox;

    // Slightly decreased the size of the useful screen area to avoid drawing limits.
    #define PIXEL_MARGIN 8
    display_rect.Inflate( -PIXEL_MARGIN );

    return display_rect.Contains( aPosition );
}


void EDA_DRAW_PANEL::RefreshDrawingRect( const EDA_RECT& aRect, bool aEraseBackground )
{
    INSTALL_UNBUFFERED_DC( dc, this );

    wxRect rect = aRect;

    rect.x = dc.LogicalToDeviceX( rect.x );
    rect.y = dc.LogicalToDeviceY( rect.y );
    rect.width = dc.LogicalToDeviceXRel( rect.width );
    rect.height = dc.LogicalToDeviceYRel( rect.height );

    wxLogTrace( KICAD_TRACE_COORDS,
                wxT( "Refresh area: drawing (%d, %d, %d, %d), device (%d, %d, %d, %d)" ),
                aRect.GetX(), aRect.GetY(), aRect.GetWidth(), aRect.GetHeight(),
                rect.x, rect.y, rect.width, rect.height );

    RefreshRect( rect, aEraseBackground );
}


wxPoint EDA_DRAW_PANEL::GetScreenCenterLogicalPosition()
{
    wxSize size = GetClientSize() / 2;
    INSTALL_UNBUFFERED_DC( dc, this );

    return wxPoint( dc.DeviceToLogicalX( size.x ), dc.DeviceToLogicalY( size.y ) );
}


void EDA_DRAW_PANEL::MoveCursorToCrossHair()
{
    MoveCursor( GetScreen()->GetCrossHairPosition() );
}


void EDA_DRAW_PANEL::MoveCursor( const wxPoint& aPosition )
{
    int     x, y, xPpu, yPpu;
    wxPoint screenPos, drawingPos;
    wxRect  clientRect( wxPoint( 0, 0 ), GetClientSize() );

    INSTALL_UNBUFFERED_DC( dc, this );
    screenPos.x = dc.LogicalToDeviceX( aPosition.x );
    screenPos.y = dc.LogicalToDeviceY( aPosition.y );

    // Scroll if the requested mouse position cursor is outside the drawing area.
    if( !clientRect.Contains( screenPos ) )
    {
        GetViewStart( &x, &y );
        GetScrollPixelsPerUnit( &xPpu, &yPpu );
        CalcUnscrolledPosition( screenPos.x, screenPos.y, &drawingPos.x, &drawingPos.y );

        wxLogTrace( KICAD_TRACE_COORDS,
                    wxT( "MoveCursor() initial screen position(%d, %d) " ) \
                    wxT( "rectangle(%d, %d, %d, %d) view(%d, %d)" ),
                    screenPos.x, screenPos.y, clientRect.x, clientRect.y,
                    clientRect.width, clientRect.height, x, y );

        if( screenPos.y < clientRect.GetTop() )
            y -= m_scrollIncrementY * yPpu;
        else if( screenPos.y > clientRect.GetBottom() )
            y += m_scrollIncrementY * yPpu;
        else if( clientRect.GetRight() < screenPos.x )
            x += m_scrollIncrementX * xPpu;
        else
            x -= m_scrollIncrementX * xPpu;

        Scroll( x, y );
        CalcScrolledPosition( drawingPos.x, drawingPos.y, &screenPos.x, &screenPos.y );

        wxLogTrace( KICAD_TRACE_COORDS,
                    wxT( "MoveCursor() scrolled screen position(%d, %d) view(%d, %d)" ),
                    screenPos.x, screenPos.y, x, y );
    }

    WarpPointer( screenPos.x, screenPos.y );
}


void EDA_DRAW_PANEL::OnActivate( wxActivateEvent& event )
{
    m_canStartBlock = -1;   // Block Command can't start
    event.Skip();
}


void EDA_DRAW_PANEL::OnScroll( wxScrollWinEvent& event )
{
    int id = event.GetEventType();
    int dir;
    int x, y;
    int ppux, ppuy;
    int csizeX, csizeY;
    int unitsX, unitsY;
    int maxX, maxY;

    GetViewStart( &x, &y );
    GetScrollPixelsPerUnit( &ppux, &ppuy );
    GetClientSize( &csizeX, &csizeY );
    GetVirtualSize( &unitsX, &unitsY );

    int tmpX = x;
    int tmpY = y;

    csizeX /= ppux;
    csizeY /= ppuy;

    unitsX /= ppux;
    unitsY /= ppuy;

    maxX = unitsX - csizeX;
    maxY = unitsY - csizeY;

    dir = event.GetOrientation();   // wxHORIZONTAL or wxVERTICAL

    if( id == wxEVT_SCROLLWIN_LINEUP )
    {
        if( dir == wxHORIZONTAL )
        {
            x -= m_scrollIncrementX;

            if( x < 0 )
                x = 0;
        }
        else
        {
            y -= m_scrollIncrementY;

            if( y < 0 )
                y = 0;
        }
    }
    else if( id == wxEVT_SCROLLWIN_LINEDOWN )
    {
        if( dir == wxHORIZONTAL )
        {
            x += m_scrollIncrementX;
            if( x > maxX )
                x = maxX;
        }
        else
        {
            y += m_scrollIncrementY;

            if( y > maxY )
                y = maxY;
        }
    }
    else if( id == wxEVT_SCROLLWIN_THUMBTRACK )
    {
        if( dir == wxHORIZONTAL )
            x = event.GetPosition();
        else
            y = event.GetPosition();
    }
    else
    {
        event.Skip();
        return;
    }

    wxLogTrace( KICAD_TRACE_COORDS,
                wxT( "Setting scroll bars ppuX=%d, ppuY=%d, unitsX=%d, unitsY=%d, posX=%d, posY=%d" ),
                ppux, ppuy, unitsX, unitsY, x, y );

    double scale = GetParent()->GetScreen()->GetScalingFactor();

    wxPoint center = GetParent()->GetScreen()->GetScrollCenterPosition();
    center.x += KiROUND( (double) ( x - tmpX ) / scale );
    center.y += KiROUND( (double) ( y - tmpY ) / scale );
    GetParent()->GetScreen()->SetScrollCenterPosition( center );

    Scroll( x, y );
    event.Skip();
}


void EDA_DRAW_PANEL::SetClipBox( wxDC& aDC, const wxRect* aRect )
{
    wxRect clipBox;

    // Use the entire visible device area if no clip area was defined.
    if( aRect == NULL )
    {
        BASE_SCREEN* Screen = GetScreen();

        if( !Screen )
            return;

        Screen->m_StartVisu = CalcUnscrolledPosition( wxPoint( 0, 0 ) );
        clipBox.SetSize( GetClientSize() );

        int scrollX, scrollY;

        double scalar = Screen->GetScalingFactor();
        scrollX = KiROUND( Screen->GetGridSize().x * scalar );
        scrollY = KiROUND( Screen->GetGridSize().y * scalar );

        m_scrollIncrementX = std::max( GetClientSize().x / 8, scrollX );
        m_scrollIncrementY = std::max( GetClientSize().y / 8, scrollY );
        Screen->m_ScrollbarPos.x = GetScrollPos( wxHORIZONTAL );
        Screen->m_ScrollbarPos.y = GetScrollPos( wxVERTICAL );
    }
    else
    {
        clipBox = *aRect;
    }

    // Pad clip box in device units.
    clipBox.Inflate( CLIP_BOX_PADDING );

    // Convert from device units to drawing units.
    m_ClipBox.SetOrigin( wxPoint( aDC.DeviceToLogicalX( clipBox.x ),
                                  aDC.DeviceToLogicalY( clipBox.y ) ) );
    m_ClipBox.SetSize( wxSize( aDC.DeviceToLogicalXRel( clipBox.width ),
                               aDC.DeviceToLogicalYRel( clipBox.height ) ) );

    wxLogTrace( KICAD_TRACE_COORDS,
                wxT( "Device clip box=(%d, %d, %d, %d), Logical clip box=(%d, %d, %d, %d)" ),
                clipBox.x, clipBox.y, clipBox.width, clipBox.height,
                m_ClipBox.GetX(), m_ClipBox.GetY(), m_ClipBox.GetWidth(), m_ClipBox.GetHeight() );
}


void EDA_DRAW_PANEL::EraseScreen( wxDC* DC )
{
    GRSetDrawMode( DC, GR_COPY );

    GRSFilledRect( NULL, DC, m_ClipBox.GetX(), m_ClipBox.GetY(),
                   m_ClipBox.GetRight(), m_ClipBox.GetBottom(),
                   0, g_DrawBgColor, g_DrawBgColor );

    // Set to one (1) to draw bounding box validate bounding box calculation.
#if DEBUG_SHOW_CLIP_RECT
    EDA_RECT bBox = m_ClipBox;
    GRRect( NULL, DC, bBox.GetOrigin().x, bBox.GetOrigin().y,
            bBox.GetEnd().x, bBox.GetEnd().y, 0, LIGHTMAGENTA );
#endif
}


void EDA_DRAW_PANEL::DoPrepareDC( wxDC& dc )
{
    wxScrolledWindow::DoPrepareDC( dc );

    if( GetScreen() != NULL )
    {
        double scale = GetScreen()->GetScalingFactor();
        dc.SetUserScale( scale, scale );

        wxPoint pt = GetScreen()->m_DrawOrg;
        dc.SetLogicalOrigin( pt.x, pt.y );
    }

    SetClipBox( dc );                         // Reset the clip box to the entire screen.
    GRResetPenAndBrush( &dc );
    dc.SetBackgroundMode( wxTRANSPARENT );
}


void EDA_DRAW_PANEL::OnPaint( wxPaintEvent& event )
{
    if( GetScreen() == NULL )
    {
        event.Skip();
        return;
    }

    INSTALL_PAINTDC( paintDC, this );

    wxRect region = GetUpdateRegion().GetBox();
    SetClipBox( paintDC, &region );
    ReDraw( &paintDC, true );
}


void EDA_DRAW_PANEL::ReDraw( wxDC* DC, bool erasebg )
{
    BASE_SCREEN* Screen = GetScreen();

    if( Screen == NULL )
        return;

    if( ( g_DrawBgColor != WHITE ) && ( g_DrawBgColor != BLACK ) )
        g_DrawBgColor = BLACK;

    if( g_DrawBgColor == WHITE )
    {
        g_XorMode    = GR_NXOR;
        g_GhostColor = BLACK;
    }
    else
    {
        g_XorMode    = GR_XOR;
        g_GhostColor = WHITE;
    }

    if( erasebg )
        EraseScreen( DC );

    GRResetPenAndBrush( DC );

    DC->SetBackground( *wxBLACK_BRUSH );
    DC->SetBackgroundMode( wxSOLID );
    GetParent()->RedrawActiveWindow( DC, erasebg );

    // Verfies that the clipping is working correctly.  If these two sets of numbers are
    // not the same or really close.  The clipping algorithms are broken.
    wxLogTrace( KICAD_TRACE_COORDS,
                wxT( "Clip box: (%d, %d, %d, %d), Draw extents (%d, %d, %d, %d)" ),
                m_ClipBox.GetX(), m_ClipBox.GetY(), m_ClipBox.GetRight(), m_ClipBox.GetBottom(),
                DC->MinX(), DC->MinY(), DC->MaxX(), DC->MaxY() );
}


void EDA_DRAW_PANEL::DrawBackGround( wxDC* DC )
{
    EDA_COLOR_T axis_color = BLUE;

    GRSetDrawMode( DC, GR_COPY );

    if( GetParent()->IsGridVisible() )
        DrawGrid( DC );

    // Draw axis
    if( GetParent()->m_showAxis )
    {
        wxSize  pageSize = GetParent()->GetPageSizeIU();

        // Draw the Y axis
        GRDashedLine( &m_ClipBox, DC, 0, -pageSize.y,
                      0, pageSize.y, 0, axis_color );

        // Draw the X axis
        GRDashedLine( &m_ClipBox, DC, -pageSize.x, 0,
                      pageSize.x, 0, 0, axis_color );
    }

    if( GetParent()->m_showOriginAxis )
        DrawAuxiliaryAxis( DC, GR_COPY );

    if( GetParent()->m_showGridAxis )
        DrawGridAxis( DC, GR_COPY );
}


void EDA_DRAW_PANEL::DrawGrid( wxDC* aDC )
{
    #define MIN_GRID_SIZE 10        // min grid size in pixels to allow drawing
    BASE_SCREEN* screen = GetScreen();
    wxRealPoint  gridSize;
    wxSize       screenSize;
    wxPoint      org;
    wxRealPoint  screenGridSize;

    /* The grid must be visible. this is possible only is grid value
     * and zoom value are sufficient
     */
    gridSize = screen->GetGridSize();
    screen->m_StartVisu = CalcUnscrolledPosition( wxPoint( 0, 0 ) );
    screenSize = GetClientSize();

    screenGridSize.x = aDC->LogicalToDeviceXRel( KiROUND( gridSize.x ) );
    screenGridSize.y = aDC->LogicalToDeviceYRel( KiROUND( gridSize.y ) );

    org = m_ClipBox.GetPosition();

    if( screenGridSize.x < MIN_GRID_SIZE || screenGridSize.y < MIN_GRID_SIZE )
    {
        screenGridSize.x *= 2.0;
        screenGridSize.y *= 2.0;
        gridSize.x *= 2.0;
        gridSize.y *= 2.0;
    }

    if( screenGridSize.x < MIN_GRID_SIZE || screenGridSize.y < MIN_GRID_SIZE )
        return;

    org = screen->GetNearestGridPosition( org, &gridSize );

    // Setting the nearest grid position can select grid points outside the clip box.
    // Incrementing the start point by one grid step should prevent drawing grid points
    // outside the clip box.
    if( org.x < m_ClipBox.GetX() )
        org.x += KiROUND( gridSize.x );

    if( org.y < m_ClipBox.GetY() )
        org.y += KiROUND( gridSize.y );

#if ( defined( __WXMAC__ ) || 1 )
    // Use a pixel based draw to display grid.  There are a lot of calls, so the cost is
    // high and grid is slowly drawn on some platforms.  Please note that this should
    // always be enabled until the bitmap based solution below is fixed.
#ifndef __WXMAC__
    GRSetColorPen( aDC, GetParent()->GetGridColor() );
#else
    // On mac (Cocoa), a point isn't a pixel and being of size 1 don't survive to antialiasing
    GRSetColorPen( aDC, GetParent()->GetGridColor(), aDC->DeviceToLogicalXRel(2) );
#endif

    int xpos;
    double right = ( double ) m_ClipBox.GetRight();
    double bottom = ( double ) m_ClipBox.GetBottom();

    for( double x = (double) org.x; x <= right; x += gridSize.x )
    {
        xpos = KiROUND( x );

        for( double y = (double) org.y; y <= bottom; y += gridSize.y )
        {
            aDC->DrawPoint( xpos, KiROUND( y )  );
        }
    }
#else
    /* This is fast only if the Blit function is fast.  Not true on all platforms.
     *
     * A first grid column is drawn in a temporary bitmap, and after is duplicated using
     * the Blit function (copy from a screen area to an other screen area).
     */
    wxMemoryDC tmpDC;
    wxBitmap tmpBM( 1, aDC->LogicalToDeviceYRel( m_ClipBox.GetHeight() ) );
    tmpDC.SelectObject( tmpBM );
    tmpDC.SetLogicalFunction( wxCOPY );
    tmpDC.SetBackground( wxBrush( GetBackgroundColour() ) );
    tmpDC.Clear();
    tmpDC.SetPen( MakeColour( GetParent()->GetGridColor() ) );

    double usx, usy;
    int lox, loy, dox, doy;

    aDC->GetUserScale( &usx, &usy );
    aDC->GetLogicalOrigin( &lox, &loy );
    aDC->GetDeviceOrigin( &dox, &doy );

    // Create a dummy DC for coordinate translation because the actual DC scale and origin
    // must be reset in order to work correctly.
    wxBitmap tmpBitmap( 1, 1 );
    wxMemoryDC scaleDC( tmpBitmap );
    scaleDC.SetUserScale( usx, usy );
    scaleDC.SetLogicalOrigin( lox, loy );
    scaleDC.SetDeviceOrigin( dox, doy );

    double bottom = ( double ) m_ClipBox.GetBottom();

    // Draw a column of grid points.
    for( double y = (double) org.y; y <= bottom; y += gridSize.y )
    {
        tmpDC.DrawPoint( 0, scaleDC.LogicalToDeviceY( KiROUND( y ) ) );
    }

    // Reset the device context scale and origin and restore on exit.
    EDA_BLIT_NORMALIZER blitNorm( aDC );

    // Mask of everything but the grid points.
    tmpDC.SelectObject( wxNullBitmap );
    tmpBM.SetMask( new wxMask( tmpBM, GetBackgroundColour() ) );
    tmpDC.SelectObject( tmpBM );

    double right = m_ClipBox.GetRight();

    // Blit the column for each row of the damaged region.
    for( double x = (double) org.x; x <= right; x += gridSize.x )
    {
        aDC->Blit( scaleDC.LogicalToDeviceX( KiROUND( x ) ),
                   scaleDC.LogicalToDeviceY( m_ClipBox.GetY() ),
                   1, tmpBM.GetHeight(), &tmpDC, 0, 0, wxCOPY, true );
    }
#endif
}


void EDA_DRAW_PANEL::DrawAuxiliaryAxis( wxDC* aDC, GR_DRAWMODE aDrawMode )
{
    wxPoint origin = GetParent()->GetOriginAxisPosition();

    if( origin == wxPoint( 0, 0 ) )
        return;

    EDA_COLOR_T color = DARKRED;
    wxSize  pageSize = GetParent()->GetPageSizeIU();

    GRSetDrawMode( aDC, aDrawMode );

    // Draw the Y axis
    GRDashedLine( &m_ClipBox, aDC,
                  origin.x,
                  -pageSize.y,
                  origin.x,
                  pageSize.y,
                  0, color );

    // Draw the X axis
    GRDashedLine( &m_ClipBox, aDC,
                  -pageSize.x,
                  origin.y,
                  pageSize.x,
                  origin.y,
                  0, color );
}


void EDA_DRAW_PANEL::DrawGridAxis( wxDC* aDC, GR_DRAWMODE aDrawMode )
{
    BASE_SCREEN* screen = GetScreen();

    if( !GetParent()->m_showGridAxis
        || ( screen->m_GridOrigin.x == 0 && screen->m_GridOrigin.y == 0 ) )
        return;

    EDA_COLOR_T color = GetParent()->GetGridColor();
    wxSize  pageSize = GetParent()->GetPageSizeIU();

    GRSetDrawMode( aDC, aDrawMode );

    // Draw the Y axis
    GRDashedLine( &m_ClipBox, aDC,
                  screen->m_GridOrigin.x,
                  -pageSize.y,
                  screen->m_GridOrigin.x,
                  pageSize.y,
                  0, color );

    // Draw the X axis
    GRDashedLine( &m_ClipBox, aDC,
                  -pageSize.x,
                  screen->m_GridOrigin.y,
                  pageSize.x,
                  screen->m_GridOrigin.y,
                  0, color );
}


bool EDA_DRAW_PANEL::OnRightClick( wxMouseEvent& event )
{
    wxPoint pos;
    wxMenu  MasterMenu;

    INSTALL_UNBUFFERED_DC( dc, this );

    pos = event.GetLogicalPosition( dc );

    if( !GetParent()->OnRightClick( pos, &MasterMenu ) )
        return false;

    GetParent()->AddMenuZoomAndGrid( &MasterMenu );

    pos = event.GetPosition();
    m_ignoreMouseEvents = true;
    PopupMenu( &MasterMenu, pos );
    MoveCursorToCrossHair();
    m_ignoreMouseEvents = false;

    return true;
}


void EDA_DRAW_PANEL::OnMouseLeaving( wxMouseEvent& event )
{
    if( m_mouseCaptureCallback == NULL )          // No command in progress.
        m_requestAutoPan = false;

    if( !m_enableAutoPan || !m_requestAutoPan || m_ignoreMouseEvents )
        return;

    // Auto pan if mouse is leave working area:
    wxSize size = GetClientSize();

    if( ( size.x < event.GetX() ) || ( size.y < event.GetY() )
       || ( event.GetX() <= 0) || ( event.GetY() <= 0 ) )
    {
        wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED, ID_POPUP_ZOOM_CENTER );
        cmd.SetEventObject( this );
        GetEventHandler()->ProcessEvent( cmd );
    }

    event.Skip();
}


void EDA_DRAW_PANEL::OnMouseWheel( wxMouseEvent& event )
{
    if( m_ignoreMouseEvents )
        return;

    wxRect rect = wxRect( wxPoint( 0, 0 ), GetClientSize() );

    // Ignore scroll events if the cursor is outside the drawing area.
    if( event.GetWheelRotation() == 0 || !GetParent()->IsEnabled()
       || !rect.Contains( event.GetPosition() ) )
    {
        wxLogTrace( KICAD_TRACE_COORDS,
                    wxT( "OnMouseWheel() position(%d, %d) rectangle(%d, %d, %d, %d)" ),
                    event.GetPosition().x, event.GetPosition().y,
                    rect.x, rect.y, rect.width, rect.height );
        event.Skip();
        return;
    }

    INSTALL_UNBUFFERED_DC( dc, this );
    GetScreen()->SetCrossHairPosition( event.GetLogicalPosition( dc ) );

    wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );
    cmd.SetEventObject( this );

    // This is a zoom in or out command
    if( event.GetWheelRotation() > 0 )
    {
        if( event.ShiftDown() && !event.ControlDown() )
            cmd.SetId( ID_PAN_UP );
        else if( event.ControlDown() && !event.ShiftDown() )
            cmd.SetId( ID_PAN_LEFT );
        else if( (event.ControlDown() && event.ShiftDown() )
                 || m_enableZoomNoCenter)
            cmd.SetId( ID_OFFCENTER_ZOOM_IN );
        else
            cmd.SetId( ID_POPUP_ZOOM_IN );
    }
    else if( event.GetWheelRotation() < 0 )
    {
        if( event.ShiftDown() && !event.ControlDown() )
            cmd.SetId( ID_PAN_DOWN );
        else if( event.ControlDown() && !event.ShiftDown() )
            cmd.SetId( ID_PAN_RIGHT );
        else if( event.AltDown() || m_enableZoomNoCenter)
            cmd.SetId( ID_OFFCENTER_ZOOM_OUT );
        else
            cmd.SetId( ID_POPUP_ZOOM_OUT );
    }

    GetEventHandler()->ProcessEvent( cmd );
    event.Skip();
}


void EDA_DRAW_PANEL::OnMouseEvent( wxMouseEvent& event )
{
    int          localrealbutt = 0, localbutt = 0;
    BASE_SCREEN* screen = GetScreen();

    if( !screen )
        return;

    /* Adjust value to filter mouse displacement before consider the drag
     * mouse is really a drag command, not just a movement while click
     */
#define MIN_DRAG_COUNT_FOR_START_BLOCK_COMMAND 5

    if( event.Leaving() )
        m_canStartBlock = -1;

    if( !IsMouseCaptured() )          // No mouse capture in progress.
        m_requestAutoPan = false;

    if( GetParent()->IsActive() )
        SetFocus();
    else
        return;

    if( !event.IsButton() && !event.Moving() && !event.Dragging() )
        return;

    if( event.RightDown() )
    {
        OnRightClick( event );
        return;
    }

    if( m_ignoreMouseEvents )
        return;

    if( event.LeftIsDown() )
        localrealbutt |= GR_M_LEFT_DOWN;

    if( event.MiddleIsDown() )
        localrealbutt |= GR_M_MIDDLE_DOWN;

    if( event.LeftDown() )
        localbutt = GR_M_LEFT_DOWN;

    if( event.ButtonDClick( 1 ) )
        localbutt = GR_M_LEFT_DOWN | GR_M_DCLICK;

    if( event.MiddleDown() )
        localbutt = GR_M_MIDDLE_DOWN;

    localrealbutt |= localbutt;     // compensation default wxGTK

    INSTALL_UNBUFFERED_DC( DC, this );
    DC.SetBackground( *wxBLACK_BRUSH );

    // Compute the cursor position in drawing (logical) units.
    screen->SetMousePosition( event.GetLogicalPosition( DC ) );

    int kbstat = 0;

    if( event.ShiftDown() )
        kbstat |= GR_KB_SHIFT;

    if( event.ControlDown() )
        kbstat |= GR_KB_CTRL;

    if( event.AltDown() )
        kbstat |= GR_KB_ALT;

    // Calling Double Click and Click functions :
    if( localbutt == (int) ( GR_M_LEFT_DOWN | GR_M_DCLICK ) )
    {
        GetParent()->OnLeftDClick( &DC, screen->RefPos( true ) );

        // inhibit a response to the mouse left button release,
        // because we have a double click, and we do not want a new
        // OnLeftClick command at end of this Double Click
        m_ignoreNextLeftButtonRelease = true;
    }
    else if( event.LeftUp() )
    {
        // A block command is in progress: a left up is the end of block
        // or this is the end of a double click, already seen
        // Note also m_ignoreNextLeftButtonRelease can be set by
        // the call to OnLeftClick(), so do not change it after calling OnLeftClick
        bool ignoreEvt = m_ignoreNextLeftButtonRelease;
        m_ignoreNextLeftButtonRelease = false;

        if( screen->m_BlockLocate.GetState() == STATE_NO_BLOCK && !ignoreEvt )
            GetParent()->OnLeftClick( &DC, screen->RefPos( true ) );

    }
    else if( !event.LeftIsDown() )
    {
        /* be sure there is a response to a left button release command
         * even when a LeftUp event is not seen.  This happens when a
         * double click opens a dialog box, and the release mouse button
         * is made when the dialog box is opened.
         */
        m_ignoreNextLeftButtonRelease = false;
    }

    if( event.ButtonDown( wxMOUSE_BTN_MIDDLE ) && m_enableMiddleButtonPan )
    {
        if( m_panScrollbarLimits )
        {
            int ppux, ppuy;
            GetScrollPixelsPerUnit( &ppux, &ppuy );
            GetViewStart( &m_PanStartCenter.x, &m_PanStartCenter.y );
            m_PanStartCenter.x *= ppux;
            m_PanStartCenter.y *= ppuy;
        }
        else
            m_PanStartCenter = GetParent()->GetScreen()->GetScrollCenterPosition();

        m_PanStartEventPosition = event.GetPosition();

        INSTALL_UNBUFFERED_DC( dc, this );
        CrossHairOff( &dc );
    }

    if( event.ButtonUp( wxMOUSE_BTN_MIDDLE ) && m_enableMiddleButtonPan )
    {
        INSTALL_UNBUFFERED_DC( dc, this );
        CrossHairOn( &dc );
    }

    if( event.MiddleIsDown() && m_enableMiddleButtonPan )
    {
        wxPoint currentPosition = event.GetPosition();
        if( m_panScrollbarLimits )
        {
            int x, y;
            int tmpX, tmpY;
            int ppux, ppuy;
            int maxX, maxY;
            int vsizeX, vsizeY;
            int csizeX, csizeY;

            GetViewStart( &tmpX, &tmpY );
            GetScrollPixelsPerUnit( &ppux, &ppuy );
            GetVirtualSize( &vsizeX, &vsizeY );
            GetClientSize( &csizeX, &csizeY );

            maxX = vsizeX - csizeX;
            maxY = vsizeY - csizeY;

            x = m_PanStartCenter.x + m_PanStartEventPosition.x - currentPosition.x;
            y = m_PanStartCenter.y + m_PanStartEventPosition.y - currentPosition.y;

            bool shouldMoveCursor = false;

            if( x < 0 )
            {
                currentPosition.x += x;
                x = 0;
                shouldMoveCursor = true;
            }

            if( y < 0 )
            {
                currentPosition.y += y;
                y = 0;
                shouldMoveCursor = true;
            }

            if( x > maxX )
            {
                currentPosition.x += ( x - maxX );
                x = maxX;
                shouldMoveCursor = true;
            }

            if( y > maxY )
            {
                currentPosition.y += ( y - maxY );
                y = maxY;
                shouldMoveCursor = true;
            }

            if ( shouldMoveCursor )
                WarpPointer( currentPosition.x, currentPosition.y );

            Scroll( x/ppux, y/ppuy );

            double scale = GetParent()->GetScreen()->GetScalingFactor();

            wxPoint center = GetParent()->GetScreen()->GetScrollCenterPosition();
            center.x += KiROUND( (double) ( x - tmpX ) / scale ) / ppux;
            center.y += KiROUND( (double) ( y - tmpY ) / scale ) / ppuy;
            GetParent()->GetScreen()->SetScrollCenterPosition( center );

            Refresh();
            Update();
        }
        else
        {
            double scale = GetParent()->GetScreen()->GetScalingFactor();
            int x = m_PanStartCenter.x +
                    KiROUND( (double) ( m_PanStartEventPosition.x - currentPosition.x ) / scale );
            int y = m_PanStartCenter.y +
                    KiROUND( (double) ( m_PanStartEventPosition.y - currentPosition.y ) / scale );

            GetParent()->RedrawScreen( wxPoint( x, y ), false );
        }
    }

    if( event.ButtonUp( wxMOUSE_BTN_MIDDLE ) && !m_enableMiddleButtonPan &&
        (screen->m_BlockLocate.GetState() == STATE_NO_BLOCK) )
    {
        // The middle button has been released, with no block command:
        // We use it for a zoom center at cursor position command
        wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED, ID_POPUP_ZOOM_CENTER );
        cmd.SetEventObject( this );
        GetEventHandler()->ProcessEvent( cmd );
    }

    // Calling the general function on mouse changes (and pseudo key commands)
    GetParent()->GeneralControl( &DC, event.GetLogicalPosition( DC ), 0 );

    /*******************************/
    /* Control of block commands : */
    /*******************************/

    // Command block can't start if mouse is dragging a new panel
    static EDA_DRAW_PANEL* lastPanel;
    if( lastPanel != this )
    {
        m_minDragEventCount = 0;
        m_canStartBlock   = -1;
    }

    /* A new command block can start after a release buttons
     * and if the drag is enough
     * This is to avoid a false start block when a dialog box is dismissed,
     * or when changing panels in hierarchy navigation
     * or when clicking while and moving mouse
     */
    if( !event.LeftIsDown() && !event.MiddleIsDown() )
    {
        m_minDragEventCount = 0;
        m_canStartBlock   = 0;

        /* Remember the last cursor position when a drag mouse starts
         * this is the last position ** before ** clicking a button
         * this is useful to start a block command from the point where the
         * mouse was clicked first
         * (a filter creates a delay for the real block command start, and
         * we must remember this point)
         */
        m_CursorStartPos = screen->GetCrossHairPosition();
    }

    if( m_enableBlockCommands && !(localbutt & GR_M_DCLICK) )
    {
        if( !screen->IsBlockActive() )
        {
            screen->m_BlockLocate.SetOrigin( m_CursorStartPos );
        }

        if( event.LeftDown() || ( !m_enableMiddleButtonPan && event.MiddleDown() ) )
        {
            if( screen->m_BlockLocate.GetState() == STATE_BLOCK_MOVE )
            {
                m_requestAutoPan = false;
                GetParent()->HandleBlockPlace( &DC );
                m_ignoreNextLeftButtonRelease = true;
            }
        }
        else if( ( m_canStartBlock >= 0 )
                && ( event.LeftIsDown() || ( !m_enableMiddleButtonPan && event.MiddleIsDown() ) )
                && !IsMouseCaptured() )
        {
            // Mouse is dragging: if no block in progress,  start a block command.
            if( screen->m_BlockLocate.GetState() == STATE_NO_BLOCK )
            {
                //  Start a block command
                int cmd_type = kbstat;

                if( !m_enableMiddleButtonPan && event.MiddleIsDown() )
                    cmd_type |= MOUSE_MIDDLE;

                // A block command is started if the drag is enough.  A small
                // drag is ignored (it is certainly a little mouse move when
                // clicking) not really a drag mouse
                if( m_minDragEventCount < MIN_DRAG_COUNT_FOR_START_BLOCK_COMMAND )
                    m_minDragEventCount++;
                else
                {
                    if( !GetParent()->HandleBlockBegin( &DC, cmd_type, m_CursorStartPos ) )
                    {
                        // should not occur: error
                        GetParent()->DisplayToolMsg(
                            wxT( "EDA_DRAW_PANEL::OnMouseEvent() Block Error" ) );
                    }
                    else
                    {
                        m_requestAutoPan = true;
                        SetCursor( wxCURSOR_SIZING );
                    }
                }
            }
        }

        if( event.ButtonUp( wxMOUSE_BTN_LEFT ) ||
            ( !m_enableMiddleButtonPan && event.ButtonUp( wxMOUSE_BTN_MIDDLE ) ) )
        {
            /* Release the mouse button: end of block.
             * The command can finish (DELETE) or have a next command (MOVE,
             * COPY).  However the block command is canceled if the block
             * size is small because a block command filtering is already
             * made, this case happens, but only when the on grid cursor has
             * not moved.
             */
            #define BLOCK_MINSIZE_LIMIT 1
            bool BlockIsSmall =
                ( std::abs( screen->m_BlockLocate.GetWidth() ) < BLOCK_MINSIZE_LIMIT )
                && ( std::abs( screen->m_BlockLocate.GetHeight() ) < BLOCK_MINSIZE_LIMIT );

            if( (screen->m_BlockLocate.GetState() != STATE_NO_BLOCK) && BlockIsSmall )
            {
                if( m_endMouseCaptureCallback )
                {
                    m_endMouseCaptureCallback( this, &DC );
                    m_requestAutoPan = false;
                }

                SetCursor( (wxStockCursor) m_currentCursor );
           }
            else if( screen->m_BlockLocate.GetState() == STATE_BLOCK_END )
            {
                m_requestAutoPan = false;
                GetParent()->HandleBlockEnd( &DC );
                SetCursor( (wxStockCursor) m_currentCursor );
                if( screen->m_BlockLocate.GetState() == STATE_BLOCK_MOVE )
                {
                    m_requestAutoPan = true;
                    SetCursor( wxCURSOR_HAND );
                }
           }
        }
    }

    // End of block command on a double click
    // To avoid an unwanted block move command if the mouse is moved while double clicking
    if( localbutt == (int) ( GR_M_LEFT_DOWN | GR_M_DCLICK ) )
    {
        if( !screen->IsBlockActive() && IsMouseCaptured() )
        {
            m_endMouseCaptureCallback( this, &DC );
        }
    }

#if 0
    wxString msg_debug;
    msg_debug.Printf( " block state %d, cmd %d",
                      screen->m_BlockLocate.GetState(),
                      screen->m_BlockLocate.GetCommand() );
    GetParent()->PrintMsg( msg_debug );
#endif

    lastPanel = this;
}



void EDA_DRAW_PANEL::OnCharHook( wxKeyEvent& event )
{
    event.Skip();
}

void EDA_DRAW_PANEL::OnKeyEvent( wxKeyEvent& event )
{
    int localkey;
    wxPoint pos;

    localkey = event.GetKeyCode();

    switch( localkey )
    {
    default:
        break;

    case WXK_ESCAPE:
        m_abortRequest = true;

        if( IsMouseCaptured() )
            EndMouseCapture();
        else
            EndMouseCapture( ID_NO_TOOL_SELECTED, m_defaultCursor, wxEmptyString );
        break;
    }

    if( event.ControlDown() )
        localkey |= GR_KB_CTRL;
    if( event.AltDown() )
        localkey |= GR_KB_ALT;
    if( event.ShiftDown() && (event.GetKeyCode() > 256) )
        localkey |= GR_KB_SHIFT;

    /* Normalize keys code to easily handle keys from Ctrl+A to Ctrl+Z
     * They have an ascii code from 1 to 27 remapped
     * to GR_KB_CTRL + 'A' to GR_KB_CTRL + 'Z'
     */
    if( (localkey > GR_KB_CTRL) && (localkey <= GR_KB_CTRL+26) )
        localkey += 'A' - 1;

    INSTALL_UNBUFFERED_DC( DC, this );

    BASE_SCREEN* Screen = GetScreen();

    // Some key commands use the current mouse position: refresh it.
    pos = wxGetMousePosition() - GetScreenPosition();

    // Compute the cursor position in drawing units.  Also known as logical units to wxDC.
    pos = wxPoint( DC.DeviceToLogicalX( pos.x ), DC.DeviceToLogicalY( pos.y ) );
    Screen->SetMousePosition( pos );

    GetParent()->GeneralControl( &DC, pos, localkey );

}


void EDA_DRAW_PANEL::OnPan( wxCommandEvent& event )
{
    int x, y;
    int ppux, ppuy;
    int unitsX, unitsY;
    int maxX, maxY;

    GetViewStart( &x, &y );
    GetScrollPixelsPerUnit( &ppux, &ppuy );
    GetVirtualSize( &unitsX, &unitsY );
    maxX = unitsX;
    maxY = unitsY;
    unitsX /= ppux;
    unitsY /= ppuy;

    switch( event.GetId() )
    {
    case ID_PAN_UP:
        y -= m_scrollIncrementY;
        break;

    case ID_PAN_DOWN:
        y += m_scrollIncrementY;
        break;

    case ID_PAN_LEFT:
        x -= m_scrollIncrementX;
        break;

    case ID_PAN_RIGHT:
        x += m_scrollIncrementX;
        break;

    default:
        wxLogDebug( wxT( "Unknown ID %d in EDA_DRAW_PANEL::OnPan()." ), event.GetId() );
    }

    if( x < 0 )
        x = 0;

    if( y < 0 )
        y = 0;

    if( x > maxX )
        x = maxX;

    if( y > maxY )
        y = maxY;

    Scroll( x/ppux, y/ppuy );
}


void EDA_DRAW_PANEL::EndMouseCapture( int id, int cursor, const wxString& title,
                                      bool aCallEndFunc )
{
    if( m_mouseCaptureCallback && m_endMouseCaptureCallback && aCallEndFunc )
    {
        INSTALL_UNBUFFERED_DC( dc, this );
        m_endMouseCaptureCallback( this, &dc );
    }

    m_mouseCaptureCallback = NULL;
    m_endMouseCaptureCallback = NULL;
    m_requestAutoPan = false;

    if( id != -1 && cursor != -1 )
    {
        wxASSERT( cursor > wxCURSOR_NONE && cursor < wxCURSOR_MAX );
        GetParent()->SetToolID( id, cursor, title );
    }
}


void EDA_DRAW_PANEL::CallMouseCapture( wxDC* aDC, const wxPoint& aPosition, bool aErase )
{
    wxCHECK_RET( aDC != NULL, wxT( "Invalid device context." ) );
    wxCHECK_RET( m_mouseCaptureCallback != NULL, wxT( "Mouse capture callback not set." ) );

    m_mouseCaptureCallback( this, aDC, aPosition, aErase );
}


void EDA_DRAW_PANEL::CallEndMouseCapture( wxDC* aDC )
{
    wxCHECK_RET( aDC != NULL, wxT( "Invalid device context." ) );
    wxCHECK_RET( m_endMouseCaptureCallback != NULL, wxT( "End mouse capture callback not set." ) );

    m_endMouseCaptureCallback( this, aDC );
}
