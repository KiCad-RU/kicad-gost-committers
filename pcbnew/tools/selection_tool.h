/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#ifndef __SELECTION_TOOL_H
#define __SELECTION_TOOL_H

#include <math/vector2d.h>
#include <tool/tool_interactive.h>
#include <tool/context_menu.h>
#include <class_undoredo_container.h>

class SELECTION_AREA;
class BOARD_ITEM;
class GENERAL_COLLECTOR;

namespace KIGFX
{
class VIEW_GROUP;
}

/**
 * Class SELECTION_TOOL
 *
 * Our sample selection tool: currently supports:
 * - pick single objects (click LMB)
 * - add objects to existing selection (Shift+LMB)
 * - draw selection box (drag LMB)
 * - handles MODULEs properly (ie. selects either MODULE or its PADs, TEXTs, etc.)
 * - takes into account high-contrast & layer visibility settings
 * - invokes InteractiveEdit tool when user starts to drag selected items
 */

class SELECTION_TOOL : public TOOL_INTERACTIVE
{
public:
    SELECTION_TOOL();
    ~SELECTION_TOOL();

    struct SELECTION
    {
        /// Set of selected items
        PICKED_ITEMS_LIST items;

        /// VIEW_GROUP that holds currently selected items
        KIGFX::VIEW_GROUP* group;

        /// Checks if there is anything selected
        bool Empty() const
        {
            return ( items.GetCount() == 0 );
        }

        /// Returns the number of selected parts
        int Size() const
        {
            return items.GetCount();
        }

    private:
        /// Clears both the VIEW_GROUP and set of selected items. Please note that it does not
        /// change properties of selected items (e.g. selection flag).
        void clear();

        friend class SELECTION_TOOL;
    };

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason );

    /**
     * Function Main()
     *
     * The main loop.
     */
    int Main( TOOL_EVENT& aEvent );

    /**
     * Function GetSelection()
     *
     * Returns the set of currently selected items.
     */
    const SELECTION& GetSelection() const
    {
        return m_selection;
    }

    /**
     * Function AddMenuItem()
     *
     * Adds a menu entry to run a TOOL_ACTION on selected items.
     * @param aAction is a menu entry to be added.
     */
    void AddMenuItem( const TOOL_ACTION& aAction );

    ///> Event sent after an item is selected.
    const TOOL_EVENT SelectedEvent;

    ///> Event sent after an item is deselected.
    const TOOL_EVENT DeselectedEvent;

    ///> Event sent after selection is cleared.
    const TOOL_EVENT ClearedEvent;

private:
    /**
     * Function selectSingle()
     * Selects an item pointed by the parameter aWhere. If there is more than one item at that
     * place, there is a menu displayed that allows to choose the item.
     *
     * @param aWhere is the place where the item should be selected.
     */
    void selectSingle( const VECTOR2I& aWhere );

    /**
     * Function selectMultiple()
     * Handles drawing a selection box that allows to select many items at the same time.
     *
     * @return true if the function was cancelled (ie. CancelEvent was received).
     */
    bool selectMultiple();

    /**
     * Function ClearSelection()
     * Clears the current selection.
     */
    void clearSelection();

    /**
     * Function disambiguationMenu()
     * Handles the menu that allows to select one of many items in case there is more than one
     * item at the selected point (@see selectSingle()).
     *
     * @param aItems contains list of items that are displayed to the user.
     */
    BOARD_ITEM* disambiguationMenu( GENERAL_COLLECTOR* aItems );

    /**
     * Function pickSmallestComponent()
     * Allows to find the smallest (in terms of bounding box area) item from the list.
     *
     * @param aCollector containes the list of items.
     */
    BOARD_ITEM* pickSmallestComponent( GENERAL_COLLECTOR* aCollector );

    /**
     * Function toggleSelection()
     * Changes selection status of a given item.
     *
     * @param aItem is the item to have selection status changed.
     */
    void toggleSelection( BOARD_ITEM* aItem );

    /**
     * Function selectable()
     * Checks conditions for an item to be selected.
     *
     * @return True if the item fulfills conditions to be selected.
     */
    bool selectable( const BOARD_ITEM* aItem ) const;

    /**
     * Function selectItem()
     * Takes necessary action mark an item as selected.
     *
     * @param aItem is an item to be selected.
     */
    void select( BOARD_ITEM* aItem );

    /**
     * Function deselectItem()
     * Takes necessary action mark an item as deselected.
     *
     * @param aItem is an item to be deselected.
     */
    void deselect( BOARD_ITEM* aItem );

    /**
     * Function deselectVisually()
     * Marks item as selected, but does not add it to the ITEMS_PICKED_LIST.
     * @param aItem is an item to be be marked.
     */
    void selectVisually( BOARD_ITEM* aItem ) const;

    /**
     * Function deselectVisually()
     * Marks item as selected, but does not add it to the ITEMS_PICKED_LIST.
     * @param aItem is an item to be be marked.
     */
    void deselectVisually( BOARD_ITEM* aItem ) const;

    /**
     * Function containsSelected()
     * Checks if the given point is placed within any of selected items' bounding box.
     *
     * @return True if the given point is contained in any of selected items' bouding box.
     */
    bool selectionContains( const VECTOR2I& aPoint ) const;

    /// Visual representation of selection box
    SELECTION_AREA* m_selArea;

    /// Current state of selection
    SELECTION m_selection;

    /// Flag saying if items should be added to the current selection or rather replace it
    bool m_additive;

    /// Flag saying if multiple selection mode is active
    bool m_multiple;

    /// Right click popup menu
    CONTEXT_MENU m_menu;
};

#endif
