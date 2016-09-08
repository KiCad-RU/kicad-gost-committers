/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2014 CERN
 * Copyright (C) 2016 KiCad Developers, see AUTHORS.txt for contributors.
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * Author: Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PNS_TOOL_BASE_H
#define __PNS_TOOL_BASE_H

#include <import_export.h>

#include <math/vector2d.h>
#include <tool/tool_interactive.h>

#include <msgpanel.h>

#include "pns_router.h"

class GRID_HELPER;

class PNS_KICAD_IFACE;
class PNS_TUNE_STATUS_POPUP;

namespace PNS {

class APIEXPORT TOOL_BASE : public TOOL_INTERACTIVE
{
public:
    static TOOL_ACTION ACT_RouterOptions;

    TOOL_BASE( const std::string& aToolName );
    virtual ~TOOL_BASE();

    virtual void Reset( RESET_REASON aReason );

    const ROUTING_SETTINGS& PNSSettings() const
    {
        return m_savedSettings;
    }

    ROUTER* Router() const;

protected:

    const VECTOR2I snapToItem( ITEM* aItem, VECTOR2I aP, bool& aSplitsSegment );
    virtual ITEM* pickSingleItem( const VECTOR2I& aWhere, int aNet = -1, int aLayer = -1 );
    virtual void highlightNet( bool aEnabled, int aNetcode = -1 );
    virtual void updateStartItem( TOOL_EVENT& aEvent );
    virtual void updateEndItem( TOOL_EVENT& aEvent );
    void deleteTraces( ITEM* aStartItem, bool aWholeTrack );

    MSG_PANEL_ITEMS m_panelItems;

    ROUTING_SETTINGS m_savedSettings;     ///< Stores routing settings between router invocations
    SIZES_SETTINGS m_savedSizes;          ///< Stores sizes settings between router invocations
    ITEM* m_startItem;
    int m_startLayer;
    VECTOR2I m_startSnapPoint;

    ITEM* m_endItem;
    VECTOR2I m_endSnapPoint;

    PCB_EDIT_FRAME* m_frame;
    KIGFX::VIEW_CONTROLS* m_ctls;
    BOARD* m_board;
    GRID_HELPER* m_gridHelper;
    PNS_KICAD_IFACE* m_iface;
    ROUTER* m_router;
};

}

#endif
