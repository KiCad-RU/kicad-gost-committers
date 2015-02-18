/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
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

#ifndef __EDIT_TOOL_H
#define __EDIT_TOOL_H

#include <math/vector2d.h>
#include <tool/tool_interactive.h>
#include <view/view_group.h>

class BOARD_ITEM;
class SELECTION_TOOL;

namespace KIGFX
{
class VIEW_GROUP;
}

/**
 * Class EDIT_TOOL
 *
 * The interactive edit tool. Allows to move, rotate, flip and change properties of items selected
 * using the pcbnew.InteractiveSelection tool.
 */

class EDIT_TOOL : public TOOL_INTERACTIVE
{
public:
    EDIT_TOOL();

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason );

    /// @copydoc TOOL_INTERACTIVE::Init()
    bool Init();

    /**
     * Function Main()
     *
     * Main loop in which events are handled.
     * @param aEvent is the handled event.
     */
    int Main( const TOOL_EVENT& aEvent );

    /**
     * Function Edit()
     *
     * Displays properties window for the selected object.
     */
    int Properties( const TOOL_EVENT& aEvent );

    /**
     * Function Rotate()
     *
     * Rotates currently selected items.
     */
    int Rotate( const TOOL_EVENT& aEvent );

    /**
     * Function Flip()
     *
     * Rotates currently selected items. The rotation point is the current cursor position.
     */
    int Flip( const TOOL_EVENT& aEvent );

    /**
     * Function Remove()
     *
     * Deletes currently selected items. The rotation point is the current cursor position.
     */
    int Remove( const TOOL_EVENT& aEvent );

    /**
     * Function EditModules()
     *
     * Toggles edit module mode. When enabled, one may select parts of modules individually
     * (graphics, pads, etc.), so they can be modified.
     * @param aEnabled decides if the mode should be enabled.
     */
    void EditModules( bool aEnabled )
    {
        m_editModules = aEnabled;
    }

private:
    ///> Selection tool used for obtaining selected items
    SELECTION_TOOL* m_selectionTool;

    ///> Flag determining if anything is being dragged right now
    bool m_dragging;

    ///> Offset from the dragged item's center (anchor)
    wxPoint m_offset;

    ///> Last cursor position (needed for getModificationPoint() to avoid changes
    ///> of edit reference point).
    VECTOR2I m_cursor;

    /// Edit module mode flag
    bool m_editModules;

    ///> Removes and frees a single BOARD_ITEM.
    void remove( BOARD_ITEM* aItem );

    ///> Sets up handlers for various events.
    void setTransitions();

    ///> The required update flag for modified items
    KIGFX::VIEW_ITEM::VIEW_UPDATE_FLAGS m_updateFlag;

    ///> Enables higher order update flag
    void enableUpdateFlag( KIGFX::VIEW_ITEM::VIEW_UPDATE_FLAGS aFlag )
    {
        if( m_updateFlag < aFlag )
            m_updateFlag = aFlag;
    }

    ///> Updates ratsnest for selected items.
    ///> @param aRedraw says if selected items should be drawn using the simple mode (e.g. one line
    ///> per item).
    void updateRatsnest( bool aRedraw );

    ///> Returns the right modification point (e.g. for rotation), depending on the number of
    ///> selected items.
    wxPoint getModificationPoint( const SELECTION& aSelection );

    ///> If there are no items currently selected, it tries to choose the item that is under
    ///> the cursor or displays a disambiguation menu if there are multpile items.
    bool hoverSelection( const SELECTION& aSelection, bool aSanitize = true );

    ///> Updates view with the changes in the list.
    void processChanges( const PICKED_ITEMS_LIST* aList );

    int editFootprintInFpEditor( const TOOL_EVENT& aEvent );

    bool invokeInlineRouter();

    template<class T> T* uniqueSelected()
    {
        const SELECTION& selection = m_selectionTool->GetSelection();

        if( selection.items.GetCount() > 1 )
            return NULL;

        BOARD_ITEM* item = selection.Item<BOARD_ITEM>( 0 );
        return dyn_cast<T*>( item );
    }
};

#endif
