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

#include <tool/tool_action.h>
#include <boost/optional.hpp>

class TOOL_EVENT;

/**
 * Class COMMON_ACTIONS
 *
 * Gathers all the actions that are shared by tools. The instance of COMMON_ACTION is created
 * inside of ACTION_MANAGER object that registers the actions.
 */
class COMMON_ACTIONS
{
public:
    // Selection Tool
    /// Activation of the selection tool
    static TOOL_ACTION selectionActivate;

    /// Select a single item under the cursor position
    static TOOL_ACTION selectionSingle;

    /// Clears the current selection
    static TOOL_ACTION selectionClear;

    // Edit Tool
    /// Activation of the edit tool
    static TOOL_ACTION editActivate;

    /// Rotation of selected objects
    static TOOL_ACTION rotate;

    /// Flipping of selected objects
    static TOOL_ACTION flip;

    /// Activation of the edit tool
    static TOOL_ACTION properties;

    /// Deleting a BOARD_ITEM
    static TOOL_ACTION remove;

    // Drawing Tool
    /// Activation of the drawing tool (line)
    static TOOL_ACTION drawLine;

    /// Activation of the drawing tool (circle)
    static TOOL_ACTION drawCircle;

    /// Activation of the drawing tool (arc)
    static TOOL_ACTION drawArc;

    /// Activation of the drawing tool (text)
    static TOOL_ACTION placeText;

    /// Activation of the drawing tool (dimension)
    static TOOL_ACTION drawDimension;

    /// Activation of the drawing tool (drawing a ZONE)
    static TOOL_ACTION drawZone;

    /// Activation of the drawing tool (drawing a keepout area)
    static TOOL_ACTION drawKeepout;

    /// Activation of the drawing tool (placing a TARGET)
    static TOOL_ACTION placeTarget;

    /// Activation of the drawing tool (placing a MODULE)
    static TOOL_ACTION placeModule;

    /// Activation of the drawing tool (placing a drawing from DXF file)
    static TOOL_ACTION placeDXF;

    /// Activation of the drawing tool (placing the footprint anchor)
    static TOOL_ACTION setAnchor;

    /// Increase width of currently drawn line
    static TOOL_ACTION incWidth;

    /// Decrease width of currently drawn line
    static TOOL_ACTION decWidth;

    /// Switch posture when drawing arc
    static TOOL_ACTION arcPosture;

    // Push and Shove Router Tool
    /// Activation of the Push and Shove router
    static TOOL_ACTION routerActivate;

    // Point Editor
    /// Update edit points
    static TOOL_ACTION pointEditorUpdate;

    /// Break outline (insert additional points to an edge)
    static TOOL_ACTION pointEditorBreakOutline;

    // Placement tool
    /// Align items to the top edge of selection bounding box
    static TOOL_ACTION alignTop;

    /// Align items to the bottom edge of selection bounding box
    static TOOL_ACTION alignBottom;

    /// Align items to the left edge of selection bounding box
    static TOOL_ACTION alignLeft;

    /// Align items to the right edge of selection bounding box
    static TOOL_ACTION alignRight;

    /// Distributes items evenly along the horizontal axis
    static TOOL_ACTION distributeHorizontally;

    /// Distributes items evenly along the vertical axis
    static TOOL_ACTION distributeVertically;

    // View controls
    static TOOL_ACTION zoomIn;
    static TOOL_ACTION zoomOut;
    static TOOL_ACTION zoomInCenter;
    static TOOL_ACTION zoomOutCenter;
    static TOOL_ACTION zoomCenter;
    static TOOL_ACTION zoomFitScreen;

    // Display modes
    static TOOL_ACTION trackDisplayMode;
    static TOOL_ACTION padDisplayMode;
    static TOOL_ACTION viaDisplayMode;
    static TOOL_ACTION zoneDisplayEnable;
    static TOOL_ACTION zoneDisplayDisable;
    static TOOL_ACTION zoneDisplayOutlines;
    static TOOL_ACTION highContrastMode;
    static TOOL_ACTION highContrastInc;
    static TOOL_ACTION highContrastDec;

    // Layer control
    static TOOL_ACTION layerTop;
    static TOOL_ACTION layerInner1;
    static TOOL_ACTION layerInner2;
    static TOOL_ACTION layerInner3;
    static TOOL_ACTION layerInner4;
    static TOOL_ACTION layerInner5;
    static TOOL_ACTION layerInner6;
    static TOOL_ACTION layerBottom;
    static TOOL_ACTION layerNext;
    static TOOL_ACTION layerPrev;
    static TOOL_ACTION layerAlphaInc;
    static TOOL_ACTION layerAlphaDec;

    static TOOL_ACTION layerChanged;        // notification

    // Grid control
    static TOOL_ACTION gridFast1;
    static TOOL_ACTION gridFast2;
    static TOOL_ACTION gridNext;
    static TOOL_ACTION gridPrev;
    static TOOL_ACTION gridSetOrigin;

    // Track & via size control
    static TOOL_ACTION trackWidthInc;
    static TOOL_ACTION trackWidthDec;
    static TOOL_ACTION viaSizeInc;
    static TOOL_ACTION viaSizeDec;

    static TOOL_ACTION trackViaSizeChanged;   // notification

    // Zone actions
    static TOOL_ACTION zoneFill;
    static TOOL_ACTION zoneFillAll;
    static TOOL_ACTION zoneUnfill;

    // Module editor tools
    /// Activation of the drawing tool (placing a PAD)
    static TOOL_ACTION placePad;

    /// Tool for quick pad enumeration
    static TOOL_ACTION enumeratePads;

    /// Copying module items to clipboard
    static TOOL_ACTION copyItems;

    /// Pasting module items from clipboard
    static TOOL_ACTION pasteItems;

    /// Display module edges as outlines
    static TOOL_ACTION moduleEdgeOutlines;

    /// Display module texts as outlines
    static TOOL_ACTION moduleTextOutlines;

    // Miscellaneous
    static TOOL_ACTION selectionTool;
    static TOOL_ACTION resetCoords;
    static TOOL_ACTION switchCursor;
    static TOOL_ACTION switchUnits;
    static TOOL_ACTION showHelp;
    static TOOL_ACTION toBeDone;

    /// Find an item
    static TOOL_ACTION find;

    /// Find an item and start moving
    static TOOL_ACTION findMove;

    /// Blocks CTRL+F, it is handled by wxWidgets
    static TOOL_ACTION findDummy;
    /**
     * Function TranslateLegacyId()
     * Translates legacy tool ids to the corresponding TOOL_ACTION name.
     * @param aId is legacy tool id to be translated.
     * @return std::string is name of the corresponding TOOL_ACTION. It may be empty, if there is
     * no corresponding TOOL_ACTION.
     */
    static boost::optional<TOOL_EVENT> TranslateLegacyId( int aId );
};
