/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#ifndef PCBNEW_CONTROL_H
#define PCBNEW_CONTROL_H

#include <tool/tool_interactive.h>

class PCB_BASE_FRAME;

/**
 * Class PCBNEW_CONTROL
 *
 * Handles actions that are shared between different frames in pcbnew.
 */

class PCBNEW_CONTROL : public TOOL_INTERACTIVE
{
public:
    PCBNEW_CONTROL();

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason );

    /// @copydoc TOOL_INTERACTIVE::Init()
    bool Init();

    // View controls
    int ZoomInOut( const TOOL_EVENT& aEvent );
    int ZoomInOutCenter( const TOOL_EVENT& aEvent );
    int ZoomCenter( const TOOL_EVENT& aEvent );
    int ZoomFitScreen( const TOOL_EVENT& aEvent );

    // Display modes
    int TrackDisplayMode( const TOOL_EVENT& aEvent );
    int PadDisplayMode( const TOOL_EVENT& aEvent );
    int ViaDisplayMode( const TOOL_EVENT& aEvent );
    int ZoneDisplayMode( const TOOL_EVENT& aEvent );
    int HighContrastMode( const TOOL_EVENT& aEvent );
    int HighContrastInc( const TOOL_EVENT& aEvent );
    int HighContrastDec( const TOOL_EVENT& aEvent );

    // Layer control
    int LayerSwitch( const TOOL_EVENT& aEvent );
    int LayerNext( const TOOL_EVENT& aEvent );
    int LayerPrev( const TOOL_EVENT& aEvent );
    int LayerAlphaInc( const TOOL_EVENT& aEvent );
    int LayerAlphaDec( const TOOL_EVENT& aEvent );

    // Grid control
    int GridFast1( const TOOL_EVENT& aEvent );
    int GridFast2( const TOOL_EVENT& aEvent );
    int GridNext( const TOOL_EVENT& aEvent );
    int GridPrev( const TOOL_EVENT& aEvent );
    int GridSetOrigin( const TOOL_EVENT& aEvent );

    // Miscellaneous
    int ResetCoords( const TOOL_EVENT& aEvent );
    int SwitchCursor( const TOOL_EVENT& aEvent );
    int SwitchUnits( const TOOL_EVENT& aEvent );
    int ShowHelp( const TOOL_EVENT& aEvent );
    int ToBeDone( const TOOL_EVENT& aEvent );

private:
    ///> Sets up handlers for various events.
    void setTransitions();

    ///> Pointerto the currently used edit frame.
    PCB_BASE_FRAME* m_frame;
};

#endif
