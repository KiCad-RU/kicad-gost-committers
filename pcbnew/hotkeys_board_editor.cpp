/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jp.charras@wanadoo.fr
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
 * @file hotkeys_board_editor.cpp
 */

#include <fctsys.h>
#include <wxPcbStruct.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <pcbcommon.h>

#include <class_board.h>
#include <class_module.h>
#include <class_track.h>

#include <pcbnew.h>
#include <pcbnew_id.h>
#include <hotkeys.h>

/* How to add a new hotkey:
 * see hotkeys.cpp
 */


void PCB_EDIT_FRAME::RecordMacros(wxDC* aDC, int aNumber)
{
    wxASSERT( aNumber >= 0 && aNumber < 10 );
    wxString msg;

    if( m_RecordingMacros < 0 )
    {
        m_RecordingMacros = aNumber;
        m_Macros[aNumber].m_StartPosition = GetCrossHairPosition( false );
        m_Macros[aNumber].m_Record.clear();

        msg.Printf( _( "Recording macro %d" ), aNumber );
        SetStatusText( msg );
    }
    else
    {
        m_RecordingMacros = -1;

        msg.Printf( _( "Macro %d recorded" ), aNumber );
        SetStatusText( msg );
    }
}


void PCB_EDIT_FRAME::CallMacros( wxDC* aDC, const wxPoint& aPosition, int aNumber )
{
    wxPoint tPosition;

    wxString msg;

    msg.Printf( _( "Call macro %d" ), aNumber );
    SetStatusText( msg );

    wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );
    cmd.SetEventObject( this );

    tPosition = GetNearestGridPosition( aPosition );

    m_canvas->CrossHairOff( aDC );
    SetMousePosition( tPosition );
    GeneralControl( aDC, tPosition );

    for( std::list<MACROS_RECORD>::iterator i = m_Macros[aNumber].m_Record.begin();
         i != m_Macros[aNumber].m_Record.end();
         i++ )
    {
        wxPoint tmpPos = GetNearestGridPosition( tPosition + i->m_Position );

        SetMousePosition( tmpPos );

        GeneralControl( aDC, tmpPos, i->m_HotkeyCode );
    }

    cmd.SetId( ID_ZOOM_REDRAW );
    GetEventHandler()->ProcessEvent( cmd );

    m_canvas->CrossHairOn( aDC );
}


void PCB_EDIT_FRAME::OnHotKey( wxDC* aDC, int aHotkeyCode, const wxPoint& aPosition,
                               EDA_ITEM* aItem )
{
    if( aHotkeyCode == 0 )
        return;

    bool itemCurrentlyEdited = GetCurItem() && GetCurItem()->GetFlags();
    MODULE* module = NULL;
    int evt_type = 0;       //Used to post a wxCommandEvent on demand
    PCB_SCREEN* screen = GetScreen();

    /* Convert lower to upper case
     * (the usual toupper function has problem with non ascii codes like function keys
     */
    if( (aHotkeyCode >= 'a') && (aHotkeyCode <= 'z') )
        aHotkeyCode += 'A' - 'a';

    EDA_HOTKEY* HK_Descr = GetDescriptorFromHotkey( aHotkeyCode, common_Hotkey_List );

    if( HK_Descr == NULL )
        HK_Descr = GetDescriptorFromHotkey( aHotkeyCode, board_edit_Hotkey_List );


    if( HK_Descr == NULL )
        return;

    int hk_id = HK_Descr->m_Idcommand;

    if( (m_RecordingMacros != -1) &&
        !( hk_id > HK_MACRO_ID_BEGIN && hk_id < HK_MACRO_ID_END) )
    {
        MACROS_RECORD macros_record;
        macros_record.m_HotkeyCode = aHotkeyCode;
        macros_record.m_Idcommand = HK_Descr->m_Idcommand;
        macros_record.m_Position  = GetNearestGridPosition( aPosition ) -
                                    m_Macros[m_RecordingMacros].m_StartPosition;
        m_Macros[m_RecordingMacros].m_Record.push_back( macros_record );
        wxString msg;
        msg.Printf( _( "Add key [%c] in macro %d" ), aHotkeyCode, m_RecordingMacros );
        SetStatusText( msg );
    }

    // Create a wxCommandEvent that will be posted in some hot keys functions
    wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );
    cmd.SetEventObject( this );

    LAYER_NUM      ll;

    switch( hk_id )
    {
    default:
    case HK_NOT_FOUND:
        return;
        break;

    case HK_LEFT_CLICK:
        OnLeftClick( aDC, aPosition );
        break;

    case HK_LEFT_DCLICK:    // Simulate a double left click: generate 2 events
        OnLeftClick( aDC, aPosition );
        OnLeftDClick( aDC, aPosition );
        break;

    case HK_RECORD_MACROS_0:
    case HK_RECORD_MACROS_1:
    case HK_RECORD_MACROS_2:
    case HK_RECORD_MACROS_3:
    case HK_RECORD_MACROS_4:
    case HK_RECORD_MACROS_5:
    case HK_RECORD_MACROS_6:
    case HK_RECORD_MACROS_7:
    case HK_RECORD_MACROS_8:
    case HK_RECORD_MACROS_9:
        RecordMacros( aDC, hk_id - HK_RECORD_MACROS_0 );
        break;

    case HK_CALL_MACROS_0:
    case HK_CALL_MACROS_1:
    case HK_CALL_MACROS_2:
    case HK_CALL_MACROS_3:
    case HK_CALL_MACROS_4:
    case HK_CALL_MACROS_5:
    case HK_CALL_MACROS_6:
    case HK_CALL_MACROS_7:
    case HK_CALL_MACROS_8:
    case HK_CALL_MACROS_9:
        CallMacros( aDC, GetCrossHairPosition( false ), hk_id - HK_CALL_MACROS_0 );
        break;

    case HK_SWITCH_TRACK_WIDTH_TO_NEXT:
        if( GetCanvas()->IsMouseCaptured() )
            GetCanvas()->CallMouseCapture( aDC, wxDefaultPosition, false );

        if( GetDesignSettings().GetTrackWidthIndex() < GetDesignSettings().m_TrackWidthList.size() - 1 )
            GetDesignSettings().SetTrackWidthIndex( GetDesignSettings().GetTrackWidthIndex() + 1 );
        else
            GetDesignSettings().SetTrackWidthIndex( 0 );

        if( GetCanvas()->IsMouseCaptured() )
            GetCanvas()->CallMouseCapture( aDC, wxDefaultPosition, false );

        break;

    case HK_SWITCH_TRACK_WIDTH_TO_PREVIOUS:
        if( GetCanvas()->IsMouseCaptured() )
            GetCanvas()->CallMouseCapture( aDC, wxDefaultPosition, false );

        if( GetDesignSettings().GetTrackWidthIndex() <= 0 )
            GetDesignSettings().SetTrackWidthIndex( GetDesignSettings().m_TrackWidthList.size() -1 );
        else
            GetDesignSettings().SetTrackWidthIndex( GetDesignSettings().GetTrackWidthIndex() - 1 );

        if( GetCanvas()->IsMouseCaptured() )
            GetCanvas()->CallMouseCapture( aDC, wxDefaultPosition, false );

        break;

    case HK_SWITCH_GRID_TO_FASTGRID1:
        SetFastGrid1();
        break;

    case HK_SWITCH_GRID_TO_FASTGRID2:
        SetFastGrid2();
        break;

    case HK_SWITCH_GRID_TO_NEXT:
        SetNextGrid();
        break;

    case HK_SWITCH_GRID_TO_PREVIOUS:
        SetPrevGrid();
        break;

    case HK_SWITCH_LAYER_TO_PREVIOUS:
        ll = GetActiveLayer();

        if( (ll <= LAYER_N_BACK) || (ll > LAYER_N_FRONT) )
            break;

        if( GetBoard()->GetCopperLayerCount() < 2 ) // Single layer
            ll = LAYER_N_BACK;
        else if( ll == LAYER_N_FRONT )
            ll = std::max( LAYER_N_BACK, FIRST_COPPER_LAYER + GetBoard()->GetCopperLayerCount() - 2 );
        else
            --ll;

        SwitchLayer( aDC, ll );
        break;

    case HK_SWITCH_LAYER_TO_NEXT:
        ll = GetActiveLayer();

        if( (ll < LAYER_N_BACK) || (ll >= LAYER_N_FRONT) )
            break;

        if( GetBoard()->GetCopperLayerCount() < 2 ) // Single layer
            ll = LAYER_N_BACK;
        else if( ll >= GetBoard()->GetCopperLayerCount() - 2 )
            ll = LAYER_N_FRONT;
        else
            ++ll;

        SwitchLayer( aDC, ll );
        break;

    case HK_SWITCH_LAYER_TO_COMPONENT:
        SwitchLayer( aDC, LAYER_N_FRONT );
        break;

    case HK_SWITCH_LAYER_TO_COPPER:
        SwitchLayer( aDC, LAYER_N_BACK );
        break;

    case HK_SWITCH_LAYER_TO_INNER1:
        SwitchLayer( aDC, LAYER_N_2 );
        break;

    case HK_SWITCH_LAYER_TO_INNER2:
        SwitchLayer( aDC, LAYER_N_3 );
        break;

    case HK_SWITCH_LAYER_TO_INNER3:
        SwitchLayer( aDC, LAYER_N_4 );
        break;

    case HK_SWITCH_LAYER_TO_INNER4:
        SwitchLayer( aDC, LAYER_N_5 );
        break;

    case HK_SWITCH_LAYER_TO_INNER5:
        SwitchLayer( aDC, LAYER_N_6 );
        break;

    case HK_SWITCH_LAYER_TO_INNER6:
        SwitchLayer( aDC, LAYER_N_7 );
        break;

    case HK_HELP: // Display Current hotkey list
        DisplayHotkeyList( this, g_Board_Editor_Hokeys_Descr );
        break;

    case HK_ZOOM_IN:
        evt_type = ID_POPUP_ZOOM_IN;
        break;

    case HK_ZOOM_OUT:
        evt_type = ID_POPUP_ZOOM_OUT;
        break;

    case HK_ZOOM_REDRAW:
        evt_type = ID_ZOOM_REDRAW;
        break;

    case HK_ZOOM_AUTO:
        evt_type = ID_ZOOM_PAGE;
        break;

    case HK_ZOOM_CENTER:
        evt_type = ID_POPUP_ZOOM_CENTER;
        break;

    case HK_ADD_MODULE:
        evt_type = ID_PCB_MODULE_BUTT;
        break;

    case HK_UNDO:
    case HK_REDO:
        if( !itemCurrentlyEdited )
        {
            wxCommandEvent event( wxEVT_COMMAND_TOOL_CLICKED, HK_Descr->m_IdMenuEvent );
            wxPostEvent( this, event );
        }

        break;

    case HK_RESET_LOCAL_COORD:  // Set the relative coord
        GetScreen()->m_O_Curseur = GetCrossHairPosition();
        break;

    case HK_SET_GRID_ORIGIN:
        SetGridOrigin( GetCrossHairPosition() );
        OnModify();     // because grid origin is saved in board, show as modified
        m_canvas->Refresh();
        break;

    case HK_RESET_GRID_ORIGIN:
        SetGridOrigin( wxPoint(0,0) );
        OnModify();     // because grid origin is saved in board, show as modified
        m_canvas->Refresh();
        break;

    case HK_SWITCH_UNITS:
        evt_type = (g_UserUnit == INCHES) ?
                    ID_TB_OPTIONS_SELECT_UNIT_MM : ID_TB_OPTIONS_SELECT_UNIT_INCH;
        break;

    case HK_SWITCH_TRACK_DISPLAY_MODE:
        DisplayOpt.DisplayPcbTrackFill = !DisplayOpt.DisplayPcbTrackFill;
        m_DisplayPcbTrackFill = DisplayOpt.DisplayPcbTrackFill;
        m_canvas->Refresh();
        break;

    case HK_DELETE:
        OnHotkeyDeleteItem( aDC );
        break;

    case HK_BACK_SPACE:
        if( IsCopperLayer( GetActiveLayer() ) )
        {
            if( !itemCurrentlyEdited )
            {
                // no track is currently being edited - select a segment and remove it.
                // @todo: possibly? pass the HK command code to PcbGeneralLocateAndDisplay()
                // so it can restrict its search to specific item types.
                BOARD_ITEM * item = PcbGeneralLocateAndDisplay();

                // don't let backspace delete modules!!
                if( item && item->IsTrack() )
                {
                    Delete_Segment( aDC, (TRACK*) item );
                    SetCurItem( NULL );
                }

                OnModify();
            }
            else if( GetCurItem()->IsTrack() )
            {
                // then an element is being edited - remove the last segment.
                // simple lines for debugger:
                TRACK* track = (TRACK*) GetCurItem();
                track = Delete_Segment( aDC, track );
                SetCurItem( track );
                OnModify();
            }
        }

        break;

    case HK_GET_AND_MOVE_FOOTPRINT:
        if( !itemCurrentlyEdited )
            evt_type = ID_POPUP_PCB_GET_AND_MOVE_MODULE_REQUEST;

        break;

    case HK_FIND_ITEM:
        if( !itemCurrentlyEdited )
            evt_type = ID_FIND_ITEMS;

        break;

    case HK_LOAD_BOARD:
        if( !itemCurrentlyEdited )
            evt_type = ID_LOAD_FILE ;

        break;

    case HK_SAVE_BOARD:
        if( !itemCurrentlyEdited )
            evt_type = ID_SAVE_BOARD;

        break;

    case HK_ADD_MICROVIA: // Place a micro via if a track is in progress
        if( GetToolId() != ID_TRACK_BUTT )
            return;

        if( !itemCurrentlyEdited )                         // no track in progress: nothing to do
            break;

        if( GetCurItem()->Type() != PCB_TRACE_T )           // Should not occur
            return;

        if( !GetCurItem()->IsNew() )
            return;

        // place micro via and switch layer
        if( IsMicroViaAcceptable() )
            evt_type = ID_POPUP_PCB_PLACE_MICROVIA;

        break;

    case HK_ADD_BLIND_BURIED_VIA:
    case HK_ADD_THROUGH_VIA: // Switch to alternate layer and Place a via if a track is in progress
        if( GetBoard()->GetDesignSettings().m_BlindBuriedViaAllowed &&
            hk_id == HK_ADD_BLIND_BURIED_VIA  )
            GetBoard()->GetDesignSettings().m_CurrentViaType = VIA_BLIND_BURIED;
        else
            GetBoard()->GetDesignSettings().m_CurrentViaType = VIA_THROUGH;

        if( !itemCurrentlyEdited ) // no track in progress: switch layer only
        {
            Other_Layer_Route( NULL, aDC );
            if( DisplayOpt.ContrastModeDisplay )
                m_canvas->Refresh();
            break;
        }

        if( GetToolId() != ID_TRACK_BUTT )
            return;

        if( GetCurItem()->Type() != PCB_TRACE_T )
            return;

        if( !GetCurItem()->IsNew() )
            return;

        evt_type = hk_id == HK_ADD_BLIND_BURIED_VIA ?
            ID_POPUP_PCB_PLACE_BLIND_BURIED_VIA : ID_POPUP_PCB_PLACE_THROUGH_VIA;
        break;

    case HK_SEL_LAYER_AND_ADD_THROUGH_VIA:
    case HK_SEL_LAYER_AND_ADD_BLIND_BURIED_VIA:
        if( GetCurItem() == NULL || !GetCurItem()->IsNew() ||
            GetCurItem()->Type() != PCB_TRACE_T )
            break;

        evt_type = hk_id == HK_SEL_LAYER_AND_ADD_BLIND_BURIED_VIA ?
            ID_POPUP_PCB_SELECT_CU_LAYER_AND_PLACE_BLIND_BURIED_VIA :
            ID_POPUP_PCB_SELECT_CU_LAYER_AND_PLACE_THROUGH_VIA;
        break;

    case HK_SWITCH_TRACK_POSTURE:
        /* change the position of initial segment when creating new tracks
         * switch from _/  to -\ .
         */
        evt_type = ID_POPUP_PCB_SWITCH_TRACK_POSTURE ;
        break;

    case HK_DRAG_TRACK_KEEP_SLOPE:
        OnHotkeyMoveItem( HK_DRAG_TRACK_KEEP_SLOPE );
        break;

    case HK_PLACE_ITEM:
        OnHotkeyPlaceItem( aDC );
        break;

    case HK_ADD_NEW_TRACK: // Start new track, if possible
        OnHotkeyBeginRoute( aDC );
        break;

    case HK_EDIT_ITEM:      // Edit board item
        OnHotkeyEditItem( HK_EDIT_ITEM );
        break;

    // Footprint edition:
    case HK_LOCK_UNLOCK_FOOTPRINT: // toggle module "MODULE_is_LOCKED" status:
        // get any module, locked or not locked and toggle its locked status
        if( !itemCurrentlyEdited )
        {
            wxPoint pos = RefPos( true );
            module = GetBoard()->GetFootprint( pos, screen->m_Active_Layer, true );
        }
        else if( GetCurItem()->Type() == PCB_MODULE_T )
        {
            module = (MODULE*) GetCurItem();
        }

        if( module )
        {
            SetCurItem( module );
            module->SetLocked( !module->IsLocked() );
            OnModify();
            SetMsgPanel( module );
        }
        break;

    case HK_DRAG_ITEM:    // Start drag module or track segment
        OnHotkeyMoveItem( HK_DRAG_ITEM );
        break;

    case HK_MOVE_ITEM:                  // Start move item
        OnHotkeyMoveItem( HK_MOVE_ITEM );
        break;

    case HK_COPY_ITEM:
        evt_type = OnHotkeyCopyItem();
        break;

    case HK_ROTATE_ITEM:        // Rotation
        OnHotkeyRotateItem( HK_ROTATE_ITEM );
        break;

    case HK_FLIP_ITEM:
        OnHotkeyRotateItem( HK_FLIP_ITEM );
        break;

    case HK_SWITCH_HIGHCONTRAST_MODE: // switch to high contrast mode and refresh the canvas
        DisplayOpt.ContrastModeDisplay = !DisplayOpt.ContrastModeDisplay;
        m_canvas->Refresh();
        break;

    case HK_CANVAS_CAIRO:
        evt_type = ID_MENU_CANVAS_CAIRO;
        break;

    case HK_CANVAS_OPENGL:
        evt_type = ID_MENU_CANVAS_OPENGL;
        break;

    case HK_CANVAS_DEFAULT:
        evt_type = ID_MENU_CANVAS_DEFAULT;
        break;
    }

    if( evt_type != 0 )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED );
        evt.SetEventObject( this );
        evt.SetId( evt_type );
        GetEventHandler()->ProcessEvent( evt );
    }
}


bool PCB_EDIT_FRAME::OnHotkeyDeleteItem( wxDC* aDC )
{
    BOARD_ITEM* item = GetCurItem();
    bool ItemFree = (item == NULL) || (item->GetFlags() == 0);

    switch( GetToolId() )
    {
    case ID_TRACK_BUTT:
        if( GetActiveLayer() > LAYER_N_FRONT )
            return false;

        if( ItemFree )
        {
            item = PcbGeneralLocateAndDisplay();

            if( item && !item->IsTrack() )
                return false;

            Delete_Track( aDC, (TRACK*) item );
        }
        else if( item->IsTrack( ) )
        {
            // simple lines for debugger:
            TRACK* track = (TRACK*) item;
            track = Delete_Segment( aDC, track );
            SetCurItem( track );
            OnModify();
            return true;
        }
        break;

    case ID_PCB_MODULE_BUTT:
        if( ItemFree )
        {
            wxPoint pos    = RefPos( false );
            MODULE* module = GetBoard()->GetFootprint( pos, UNDEFINED_LAYER, false );

            if( module == NULL )
                return false;

            RemoveStruct( module, aDC );
        }
        else
            return false;
        break;

    default:
        if( ItemFree )
        {
            item = PcbGeneralLocateAndDisplay();

            if( item == NULL )
                return false;

            RemoveStruct( item, aDC );
        }
        else
        return false;
    }

    OnModify();
    SetCurItem( NULL );
    return true;
}


bool PCB_EDIT_FRAME::OnHotkeyEditItem( int aIdCommand )
{
    BOARD_ITEM* item = GetCurItem();
    bool itemCurrentlyEdited = item && item->GetFlags();

    if( itemCurrentlyEdited )
        return false;

    item = PcbGeneralLocateAndDisplay();

    if( item == NULL )
        return false;

    SetCurItem( item );

    int evt_type = 0;       //Used to post a wxCommandEvent on demand

    switch( item->Type() )
    {
    case PCB_TRACE_T:
    case PCB_VIA_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_TRACKSEG;

        break;

    case PCB_TEXT_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_TEXTEPCB;

        break;

    case PCB_MODULE_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_MODULE_PRMS;

        break;

    case PCB_PAD_T:
        // Until dec 2012 a EDIT_MODULE event is posted here to prevent pads
        // from being edited by hotkeys.
        // Process_Special_Functions takes care of finding the parent.
        // After dec 2012 a EDIT_PAD event is posted, because there is no
        // reason to not allow pad edit by hotkey
        // (pad coordinates are no more modified by rounding, in nanometer version
        // when using inches or mm in dialog)
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_PAD;

        break;

    case PCB_TARGET_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_MIRE;

        break;

    case PCB_DIMENSION_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_DIMENSION;

        break;

    case PCB_MODULE_TEXT_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_TEXTMODULE;

        break;

    case PCB_LINE_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_DRAWING;

        break;

    case PCB_ZONE_AREA_T:
        if( aIdCommand == HK_EDIT_ITEM )
            evt_type = ID_POPUP_PCB_EDIT_ZONE_PARAMS;

        break;

    default:
        break;
    }

    if( evt_type != 0 )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED );
        evt.SetEventObject( this );
        evt.SetId( evt_type );
        GetEventHandler()->ProcessEvent( evt );
        return true;
    }

    return false;
}


int PCB_EDIT_FRAME::OnHotkeyCopyItem()
{
    BOARD_ITEM* item = GetCurItem();
    bool itemCurrentlyEdited = item && item->GetFlags();

    if( itemCurrentlyEdited )
        return 0;

    item = PcbGeneralLocateAndDisplay();

    if( item == NULL )
        return 0;

    SetCurItem( item );

    int eventId = 0;

    switch( item->Type() )
    {
    case PCB_TEXT_T:
        eventId = ID_POPUP_PCB_COPY_TEXTEPCB;
        break;
    default:
        eventId = 0;
        break;
    }

    return eventId;
}


bool PCB_EDIT_FRAME::OnHotkeyMoveItem( int aIdCommand )
{
    BOARD_ITEM* item = GetCurItem();
    bool itemCurrentlyEdited = item && item->GetFlags();

    if( itemCurrentlyEdited )
        return false;

    item = PcbGeneralLocateAndDisplay();

    if( item == NULL )
        return false;

    SetCurItem( item );

    int evt_type = 0;       // Used to post a wxCommandEvent on demand

    switch( item->Type() )
    {
    case PCB_TRACE_T:
    case PCB_VIA_T:
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_TRACK_NODE;

        if( aIdCommand == HK_DRAG_ITEM )
            evt_type = ID_POPUP_PCB_DRAG_TRACK_SEGMENT;

        if( aIdCommand == HK_DRAG_TRACK_KEEP_SLOPE )
            evt_type = ID_POPUP_PCB_DRAG_TRACK_SEGMENT_KEEP_SLOPE;

        break;

    case PCB_MODULE_T:
        {
            if( aIdCommand == HK_MOVE_ITEM )
                evt_type = ID_POPUP_PCB_MOVE_MODULE_REQUEST;

            if( aIdCommand == HK_DRAG_ITEM )
                evt_type = ID_POPUP_PCB_DRAG_MODULE_REQUEST;
        }
        break;

    case PCB_PAD_T:
        // Post MODULE_REQUEST events here to prevent pads
        // from being moved or dragged by hotkeys.
        // Process_Special_Functions takes care of finding
        // the parent.
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_MODULE_REQUEST;

        if( aIdCommand == HK_DRAG_ITEM )
            evt_type = ID_POPUP_PCB_DRAG_MODULE_REQUEST;

        break;

    case PCB_TEXT_T:
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_TEXTEPCB_REQUEST;

        break;

    case PCB_TARGET_T:
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_MIRE_REQUEST;

        break;

    case PCB_ZONE_AREA_T:
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_ZONE_OUTLINES;

        if( aIdCommand == HK_DRAG_ITEM )
            evt_type = ID_POPUP_PCB_DRAG_ZONE_OUTLINE_SEGMENT;

        break;

    case PCB_MODULE_TEXT_T:
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_TEXTMODULE_REQUEST;

        break;

    case PCB_LINE_T:
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_DRAWING_REQUEST;

        break;

    case PCB_DIMENSION_T:
        if( aIdCommand == HK_MOVE_ITEM )
            evt_type = ID_POPUP_PCB_MOVE_TEXT_DIMENSION_REQUEST;
        break;

    default:
        break;
    }

    if( evt_type != 0 )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED );
        evt.SetEventObject( this );
        evt.SetId( evt_type );
        GetEventHandler()->ProcessEvent( evt );
        return true;
    }

    return false;
}


bool PCB_EDIT_FRAME::OnHotkeyPlaceItem( wxDC* aDC )
{
    BOARD_ITEM* item = GetCurItem();
    bool no_tool = GetToolId() == ID_NO_TOOL_SELECTED;
    bool itemCurrentlyEdited = item && item->GetFlags();

    m_canvas->SetAutoPanRequest( false );

    if( itemCurrentlyEdited )
    {
        m_canvas->SetIgnoreMouseEvents( true );
        m_canvas->CrossHairOff( aDC );

        switch( item->Type() )
        {
        case PCB_TRACE_T:
        case PCB_VIA_T:
            if( item->IsDragging() )
                PlaceDraggedOrMovedTrackSegment( (TRACK*) item, aDC );

            break;

        case PCB_TEXT_T:
            Place_Texte_Pcb( (TEXTE_PCB*) item, aDC );
            break;

        case PCB_MODULE_TEXT_T:
            PlaceTexteModule( (TEXTE_MODULE*) item, aDC );
            break;

        case PCB_PAD_T:
            PlacePad( (D_PAD*) item, aDC );
            break;

        case PCB_MODULE_T:
            PlaceModule( (MODULE*) item, aDC );
            break;

        case PCB_TARGET_T:
            PlaceTarget( (PCB_TARGET*) item, aDC );
            break;

        case PCB_LINE_T:
            if( no_tool )   // when no tools: existing item moving.
                Place_DrawItem( (DRAWSEGMENT*) item, aDC );

            break;

        default:
            break;
        }

        m_canvas->SetIgnoreMouseEvents( false );
        m_canvas->CrossHairOn( aDC );

        return true;
    }

    return false;
}


TRACK * PCB_EDIT_FRAME::OnHotkeyBeginRoute( wxDC* aDC )
{
    if( GetActiveLayer() > LAYER_N_FRONT )
        return NULL;

    bool itemCurrentlyEdited = (GetCurItem() && GetCurItem()->GetFlags());

    // Ensure the track tool is active
    if( GetToolId() != ID_TRACK_BUTT && !itemCurrentlyEdited )
    {
        wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );
        cmd.SetEventObject( this );
        cmd.SetId( ID_TRACK_BUTT );
        GetEventHandler()->ProcessEvent( cmd );
    }

    if( GetToolId() != ID_TRACK_BUTT )
        return NULL;

    TRACK* track = NULL;

    if( !itemCurrentlyEdited )     // no track in progress:
    {
        track = Begin_Route( NULL, aDC );
        SetCurItem( track );

        if( track )
            m_canvas->SetAutoPanRequest( true );
    }
    else if( GetCurItem()->IsNew() )
    {
        track = Begin_Route( (TRACK*) GetCurItem(), aDC );

        // SetCurItem() must not write to the msg panel
        // because a track info is displayed while moving the mouse cursor
        if( track )      // A new segment was created
            SetCurItem( track, false );

        m_canvas->SetAutoPanRequest( true );
    }

    return track;
}

bool PCB_EDIT_FRAME::OnHotkeyRotateItem( int aIdCommand )
{
    BOARD_ITEM* item = GetCurItem();
    bool        itemCurrentlyEdited = item && item->GetFlags();
    int         evt_type = 0; // Used to post a wxCommandEvent on demand

    // Allows block rotate operation on hot key.
    if( GetScreen()->m_BlockLocate.GetState() != STATE_NO_BLOCK )
    {
        evt_type = ID_POPUP_ROTATE_BLOCK;
    }
    else
    {
        if( !itemCurrentlyEdited )
            item = PcbGeneralLocateAndDisplay();

        if( item == NULL )
            return false;

        SetCurItem( item );

        switch( item->Type() )
        {
        case PCB_MODULE_T:
            if( aIdCommand == HK_ROTATE_ITEM )                      // Rotation
                evt_type = ID_POPUP_PCB_ROTATE_MODULE_COUNTERCLOCKWISE;

            if( aIdCommand == HK_FLIP_ITEM )                        // move to other side
                evt_type = ID_POPUP_PCB_CHANGE_SIDE_MODULE;
            break;

        case PCB_TEXT_T:
            if( aIdCommand == HK_ROTATE_ITEM )                      // Rotation
                evt_type = ID_POPUP_PCB_ROTATE_TEXTEPCB;
            else if( aIdCommand == HK_FLIP_ITEM )
                evt_type = ID_POPUP_PCB_FLIP_TEXTEPCB;

            break;

        case PCB_MODULE_TEXT_T:
            if( aIdCommand == HK_ROTATE_ITEM )                      // Rotation
                evt_type = ID_POPUP_PCB_ROTATE_TEXTMODULE;

            break;

        default:
            break;
        }
    }

    if( evt_type != 0 )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED );
        evt.SetEventObject( this );
        evt.SetId( evt_type );
        GetEventHandler()->ProcessEvent( evt );
        return true;
    }

    return false;
}
