/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2017 CERN
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

#ifndef __ROUTER_TOOL_H
#define __ROUTER_TOOL_H

#include "pns_tool_base.h"

class APIEXPORT ROUTER_TOOL : public PNS::TOOL_BASE
{
public:
    ROUTER_TOOL();
    ~ROUTER_TOOL();

    bool Init() override;
    void Reset( RESET_REASON aReason ) override;

    int RouteSingleTrace( const TOOL_EVENT& aEvent );
    int RouteDiffPair( const TOOL_EVENT& aEvent );
    int InlineDrag( const TOOL_EVENT& aEvent );

    // TODO make this private?
    int DpDimensionsDialog( const TOOL_EVENT& aEvent );
    int SettingsDialog( const TOOL_EVENT& aEvent );
    int CustomTrackWidthDialog( const TOOL_EVENT& aEvent );

    void SetTransitions() override;

private:
    int mainLoop( PNS::ROUTER_MODE aMode );

    int getDefaultWidth( int aNetCode );

    void performRouting();
    void performDragging();

    void getNetclassDimensions( int aNetCode, int& aWidth, int& aViaDiameter, int& aViaDrill );
    void handleCommonEvents( const TOOL_EVENT& evt );

    int getStartLayer( const PNS::ITEM* aItem );
    void switchLayerOnViaPlacement();

    int onViaCommand( const TOOL_EVENT& aEvent );
    int onTrackViaSizeChanged( const TOOL_EVENT& aEvent );
    int onLayerChanged( const TOOL_EVENT& aEvent );

    bool prepareInteractive();
    bool finishInteractive();
};

#endif
