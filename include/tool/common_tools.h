/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014-2016 CERN
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

#ifndef _COMMON_TOOLS_H
#define _COMMON_TOOLS_H

#include <tool/tool_interactive.h>

class EDA_DRAW_FRAME;

/**
 * Class COMMON_TOOLS
 *
 * Handles actions that are shared between different applications
 */

class COMMON_TOOLS : public TOOL_INTERACTIVE
{
public:
    COMMON_TOOLS();
    ~COMMON_TOOLS();

    /// @copydoc TOOL_BASE::Reset()
    void Reset( RESET_REASON aReason ) override;

    // View controls
    int ZoomInOut( const TOOL_EVENT& aEvent );
    int ZoomInOutCenter( const TOOL_EVENT& aEvent );
    int ZoomCenter( const TOOL_EVENT& aEvent );
    int ZoomFitScreen( const TOOL_EVENT& aEvent );
    int ZoomPreset( const TOOL_EVENT& aEvent );

    // Cursor control
    int ToggleCursor( const TOOL_EVENT& aEvent );

    // Grid control
    int GridNext( const TOOL_EVENT& aEvent );
    int GridPrev( const TOOL_EVENT& aEvent );
    int GridPreset( const TOOL_EVENT& aEvent );

    ///> Sets up handlers for various events.
    void SetTransitions() override;

private:
    ///> Pointer to the currently used edit frame.
    EDA_DRAW_FRAME* m_frame;

    ///> Applies the legacy canvas grid settings for GAL.
    void updateGrid();
};

#endif
