/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Torsten Hueter, torstenhtr <at> gmx.de
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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
 * @file wx_view_controls.h
 * @brief WX_VIEW_CONTROLS class definition.
 */

#ifndef __WX_VIEW_CONTROLS_H
#define __WX_VIEW_CONTROLS_H

#include <wx/wx.h>
#include <wx/event.h>

#include <view/view_controls.h>

class EDA_DRAW_PANEL_GAL;

namespace KiGfx
{
/**
 * Class WX_VIEW_CONTROLS
 * is a specific implementation of class VIEW_CONTROLS for wxWidgets library.
 */
class WX_VIEW_CONTROLS : public VIEW_CONTROLS, public wxEvtHandler
{
public:
    WX_VIEW_CONTROLS( VIEW* aView, wxWindow* aParentPanel );
    ~WX_VIEW_CONTROLS() {};

    /// Handler functions
    void onWheel( wxMouseEvent& aEvent );
    void onMotion( wxMouseEvent& aEvent );
    void onButton( wxMouseEvent& aEvent );
    void onEnter( wxMouseEvent& WXUNUSED( aEvent ) );
    void onTimer( wxTimerEvent& WXUNUSED( aEvent ) );

    /**
     * Function SetGrabMouse()
     * Enables/disables mouse cursor grabbing (limits the movement field only to the panel area).
     *
     * @param aEnabled says whether the option should be enabled or disabled.
     */
    void SetGrabMouse( bool aEnabled );

    /**
     * Function SetAutoPan()
     * Enables/disables autopanning (panning when mouse cursor reaches the panel border).
     *
     * @param aEnabled says whether the option should enabled or disabled.
     */
    void SetAutoPan( bool aEnabled )
    {
        m_autoPanEnabled = aEnabled;
        if( m_state == AUTO_PANNING )
            m_state = IDLE;
    }

    /// @copydoc VIEW_CONTROLS::GetMousePosition()
    virtual const VECTOR2D GetMousePosition() const;

    /// @copydoc VIEW_CONTROLS::GetCursorPosition()
    virtual const VECTOR2D GetCursorPosition() const;

private:
    /// Possible states for WX_VIEW_CONTROLS
    enum State {
        IDLE = 1,
        DRAG_PANNING,
        AUTO_PANNING,
    };

    /**
     * Function handleAutoPanning()
     * Computes new viewport settings while in autopanning mode.
     *
     * @param aEvent is an event to be processed and decide if autopanning should happen.
     * @return true if it is currently autopanning (ie. autopanning is active and mouse cursor
     * is in the area that causes autopanning to happen).
     */
    bool handleAutoPanning( const wxMouseEvent& aEvent );

    /// Current state of VIEW_CONTROLS
    State       m_state;

    /// Panel that is affected by VIEW_CONTROLS
    wxWindow*   m_parentPanel;

    /// Stores information about point where dragging has started
    VECTOR2D    m_dragStartPoint;

    /// Stores information about the center of viewport when dragging has started
    VECTOR2D    m_lookStartPoint;

    /// Current direction of panning (only autopanning mode)
    VECTOR2D    m_panDirection;

    /// Used for determining time intervals between scroll & zoom events
    wxLongLong  m_timeStamp;

    /// Timer repsonsible for handling autopanning
    wxTimer     m_panTimer;
};
} // namespace KiGfx

#endif
