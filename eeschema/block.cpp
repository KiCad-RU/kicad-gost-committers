/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2009-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file eeschema/block.cpp
 */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <wxEeschemaStruct.h>

#include <general.h>
#include <class_library.h>
#include <lib_pin.h>
#include <protos.h>
#include <sch_bus_entry.h>
#include <sch_marker.h>
#include <sch_junction.h>
#include <sch_line.h>
#include <sch_no_connect.h>
#include <sch_text.h>
#include <sch_component.h>
#include <sch_sheet.h>

#include <boost/foreach.hpp>


// Imported functions:
extern void SetSchItemParent( SCH_ITEM* Struct, SCH_SCREEN* Screen );
extern void MoveItemsInList( PICKED_ITEMS_LIST& aItemsList, const wxPoint aMoveVector );
extern void RotateListOfItems( PICKED_ITEMS_LIST& aItemsList, wxPoint& Center );
extern void MirrorX( PICKED_ITEMS_LIST& aItemsList, wxPoint& aMirrorPoint );
extern void MirrorY( PICKED_ITEMS_LIST& aItemsList, wxPoint& Center );
extern void DuplicateItemsInList( SCH_SCREEN*        screen,
                                  PICKED_ITEMS_LIST& aItemsList,
                                  const wxPoint      aMoveVector );

static void DrawMovingBlockOutlines( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                                     const wxPoint& aPosition, bool aErase );


int SCH_EDIT_FRAME::ReturnBlockCommand( int key )
{
    int cmd = BLOCK_IDLE;

    switch( key )
    {
    default:
        cmd = key & 0xFF;
        break;

    case 0:
        cmd = BLOCK_MOVE;
        break;

    case GR_KB_SHIFT:
        cmd = BLOCK_COPY;
        break;

    case GR_KB_ALT:
        cmd = BLOCK_ROTATE;
        break;

    case GR_KB_CTRL:
        cmd = BLOCK_DRAG;
        break;

    case GR_KB_SHIFTCTRL:
        cmd = BLOCK_DELETE;
        break;

    case MOUSE_MIDDLE:
        cmd = BLOCK_ZOOM;
        break;
    }

    return cmd;
}


void SCH_EDIT_FRAME::InitBlockPasteInfos()
{
    BLOCK_SELECTOR* block = &GetScreen()->m_BlockLocate;

    block->GetItems().CopyList( m_blockItems.GetItems() );
    m_canvas->SetMouseCaptureCallback( DrawMovingBlockOutlines );
}


void SCH_EDIT_FRAME::HandleBlockPlace( wxDC* DC )
{
    BLOCK_SELECTOR* block = &GetScreen()->m_BlockLocate;

    if( !m_canvas->IsMouseCaptured() )
    {
        DisplayError( this, wxT( "HandleBlockPLace() : m_mouseCaptureCallback = NULL" ) );
    }

    if( block->GetCount() == 0 )
    {
        wxString msg;
        msg.Printf( wxT( "HandleBlockPLace() error : no items to place (cmd %d, state %d)" ),
                    block->GetCommand(), block->GetState() );
        DisplayError( this, msg );
    }

    block->SetState( STATE_BLOCK_STOP );

    switch( block->GetCommand() )
    {
    case BLOCK_DRAG:        /* Drag */
    case BLOCK_MOVE:        /* Move */
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        SaveCopyInUndoList( block->GetItems(), UR_MOVED, block->GetMoveVector() );
        MoveItemsInList( block->GetItems(), block->GetMoveVector() );
        block->ClearItemsList();
        break;

    case BLOCK_COPY:                /* Copy */
    case BLOCK_PRESELECT_MOVE:      /* Move with preselection list*/
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        DuplicateItemsInList( GetScreen(), block->GetItems(), block->GetMoveVector() );

        SaveCopyInUndoList( block->GetItems(),
                            ( block->GetCommand() == BLOCK_PRESELECT_MOVE ) ? UR_CHANGED : UR_NEW );

        block->ClearItemsList();
        break;

    case BLOCK_PASTE:
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        PasteListOfItems( DC );
        block->ClearItemsList();
        break;

    default:        // others are handled by HandleBlockEnd()
       break;
    }

    OnModify();

    // clear dome flags and pointers
    GetScreen()->ClearDrawingState();
    GetScreen()->ClearBlockCommand();
    GetScreen()->SetCurItem( NULL );
    GetScreen()->TestDanglingEnds( m_canvas, DC );

    if( block->GetCount() )
    {
        DisplayError( this, wxT( "HandleBlockPLace() error: some items left in buffer" ) );
        block->ClearItemsList();
    }

    m_canvas->EndMouseCapture( GetToolId(), m_canvas->GetCurrentCursor(), wxEmptyString, false );
    m_canvas->Refresh();
}

/*
 * HandleBlockEnd is called when:
 * a block is defined
 * or a schematic iten should be dragged
 * When the block is defined, all items inside the block should be collected
 * When a schematic iten should be dragged, only this item should be collected
 *
 * In all cases, connected items are collected when a drag command is activated
 */
bool SCH_EDIT_FRAME::HandleBlockEnd( wxDC* DC )
{
    bool            nextcmd = false;
    bool            zoom_command = false;
    BLOCK_SELECTOR* block = &GetScreen()->m_BlockLocate;
    bool            currItemOnly = false;

    if ( block->GetCommand() == BLOCK_DRAG && GetScreen()->GetCurItem() != NULL )
        currItemOnly = true;

    if( block->GetCount() )
    {
        BLOCK_STATE_T   state   = block->GetState();
        BLOCK_COMMAND_T command = block->GetCommand();

        m_canvas->CallEndMouseCapture( DC );

        block->SetState( state );
        block->SetCommand( command );
        m_canvas->SetMouseCapture( DrawAndSizingBlockOutlines, AbortBlockCurrentCommand );
        GetScreen()->SetCrossHairPosition( block->GetEnd() );

        if( block->GetCommand() != BLOCK_ABORT )
            m_canvas->MoveCursorToCrossHair();
    }

    if( m_canvas->IsMouseCaptured() )
    {
        switch( block->GetCommand() )
        {
        case BLOCK_IDLE:
            DisplayError( this, wxT( "Error in HandleBlockPLace()" ) );
            break;

        case BLOCK_ROTATE:
            GetScreen()->UpdatePickList();
            DrawAndSizingBlockOutlines( m_canvas, DC, wxDefaultPosition, false );

            if( block->GetCount() )
            {
                // Compute the rotation center and put it on grid:
                wxPoint rotationPoint = block->Centre();
                rotationPoint = GetScreen()->GetNearestGridPosition( rotationPoint );
                GetScreen()->SetCrossHairPosition( rotationPoint );
                SaveCopyInUndoList( block->GetItems(), UR_ROTATED, rotationPoint );
                RotateListOfItems( block->GetItems(), rotationPoint );
                OnModify();
            }
            block->ClearItemsList();
            GetScreen()->TestDanglingEnds( m_canvas, DC );
            m_canvas->Refresh();
            break;

        case BLOCK_DRAG:    /* Drag */
            GetScreen()->BreakSegmentsOnJunctions();
            // fall through

        case BLOCK_MOVE:
        case BLOCK_COPY:
            if( currItemOnly )
            {
                ITEM_PICKER picker;
                picker.SetItem( GetScreen()->GetCurItem() );
                block->PushItem( picker );
            }
            else
                GetScreen()->UpdatePickList();
            // fall through

        case BLOCK_PRESELECT_MOVE: /* Move with preselection list*/
            if( block->GetCount() )
            {
                nextcmd = true;
                GetScreen()->SelectBlockItems();
                m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );
                m_canvas->SetMouseCaptureCallback( DrawMovingBlockOutlines );
                m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );
                block->SetState( STATE_BLOCK_MOVE );
            }
            else
            {
                m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );
                m_canvas->SetMouseCapture( NULL, NULL );
            }
            break;

        case BLOCK_DELETE: /* Delete */
            GetScreen()->UpdatePickList();
            DrawAndSizingBlockOutlines( m_canvas, DC, wxDefaultPosition, false );

            if( block->GetCount() )
            {
                DeleteItemsInList( m_canvas, block->GetItems() );
                OnModify();
            }
            block->ClearItemsList();
            GetScreen()->TestDanglingEnds( m_canvas, DC );
            m_canvas->Refresh();
            break;

        case BLOCK_SAVE:  /* Save */
            GetScreen()->UpdatePickList();
            DrawAndSizingBlockOutlines( m_canvas, DC, wxDefaultPosition, false );

            if( block->GetCount() )
            {
                wxPoint move_vector = -GetScreen()->m_BlockLocate.GetLastCursorPosition();
                copyBlockItems( block->GetItems() );
                MoveItemsInList( m_blockItems.GetItems(), move_vector );
             }

            block->ClearItemsList();
            break;

        case BLOCK_PASTE:
            block->SetState( STATE_BLOCK_MOVE );
            break;

        case BLOCK_ZOOM: /* Window Zoom */
            zoom_command = true;
            break;

        default:
            break;
        }
    }

    if( block->GetCommand() == BLOCK_ABORT )
    {
        GetScreen()->ClearDrawingState();
        m_canvas->Refresh();
    }

    if( ! nextcmd )
    {
        block->SetState( STATE_NO_BLOCK );
        block->SetCommand( BLOCK_IDLE );
        GetScreen()->SetCurItem( NULL );
        m_canvas->EndMouseCapture( GetToolId(), m_canvas->GetCurrentCursor(), wxEmptyString,
                                   false );
    }

    if( zoom_command )
        Window_Zoom( GetScreen()->m_BlockLocate );

    return nextcmd;
}


void SCH_EDIT_FRAME::HandleBlockEndByPopUp( int Command, wxDC* DC )
{
    bool blockCmdFinished = true;   /* set to false for block command which
                                     * have a next step
                                     * and true if the block command is finished here
                                     */
    BLOCK_SELECTOR* block = &GetScreen()->m_BlockLocate;

    // can convert only a block move command to an other command
    if( block->GetCommand() != BLOCK_MOVE )
        return;

    // Useless if the new command is block move because we are already in block move.
    if( Command == BLOCK_MOVE )
        return;

    block->SetCommand( (BLOCK_COMMAND_T) Command );
    block->SetMessageBlock( this );

    switch( block->GetCommand() )
    {
    case BLOCK_COPY:     /* move to copy */
        block->SetState( STATE_BLOCK_MOVE );
        blockCmdFinished = false;
        break;

    case BLOCK_DRAG:     /* move to Drag */
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        // Clear list of items to move, and rebuild it with items to drag:
        block->ClearItemsList();

        GetScreen()->BreakSegmentsOnJunctions();
        GetScreen()->UpdatePickList();

        if( block->GetCount() )
        {
            blockCmdFinished = false;
            GetScreen()->SelectBlockItems();

            if( m_canvas->IsMouseCaptured() )
                m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

            block->SetState( STATE_BLOCK_MOVE );
        }
        break;

    case BLOCK_DELETE:     /* move to Delete */
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        if( block->GetCount() )
        {
            DeleteItemsInList( m_canvas, block->GetItems() );
            OnModify();
        }

        GetScreen()->TestDanglingEnds( m_canvas, DC );
        m_canvas->Refresh();
        break;

    case BLOCK_SAVE:     /* Save list in paste buffer*/
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        if( block->GetCount() )
        {
            wxPoint move_vector = -GetScreen()->m_BlockLocate.GetLastCursorPosition();
            copyBlockItems( block->GetItems() );
            MoveItemsInList( m_blockItems.GetItems(), move_vector );
        }
        break;

    case BLOCK_ZOOM:     /* Window Zoom */
        m_canvas->CallEndMouseCapture( DC );
        m_canvas->SetCursor( (wxStockCursor) m_canvas->GetDefaultCursor() );
        Window_Zoom( GetScreen()->m_BlockLocate );
        break;


    case BLOCK_ROTATE:
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        if( block->GetCount() )
        {
            /* Compute the rotation center and put it on grid */
            wxPoint rotationPoint = block->Centre();
            rotationPoint = GetScreen()->GetNearestGridPosition( rotationPoint );
            GetScreen()->SetCrossHairPosition( rotationPoint );
            SaveCopyInUndoList( block->GetItems(), UR_ROTATED, rotationPoint );
            RotateListOfItems( block->GetItems(), rotationPoint );
            OnModify();
        }

        GetScreen()->TestDanglingEnds( m_canvas, DC );
        m_canvas->Refresh();
        break;

    case BLOCK_MIRROR_X:
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        if( block->GetCount() )
        {
            /* Compute the mirror center and put it on grid */
            wxPoint mirrorPoint = block->Centre();
            mirrorPoint = GetScreen()->GetNearestGridPosition( mirrorPoint );
            GetScreen()->SetCrossHairPosition( mirrorPoint );
            SaveCopyInUndoList( block->GetItems(), UR_MIRRORED_X, mirrorPoint );
            MirrorX( block->GetItems(), mirrorPoint );
            OnModify();
        }

        GetScreen()->TestDanglingEnds( m_canvas, DC );
        m_canvas->Refresh();
        break;

    case BLOCK_MIRROR_Y:
        if( m_canvas->IsMouseCaptured() )
            m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

        if( block->GetCount() )
        {
            /* Compute the mirror center and put it on grid */
            wxPoint mirrorPoint = block->Centre();
            mirrorPoint = GetScreen()->GetNearestGridPosition( mirrorPoint );
            GetScreen()->SetCrossHairPosition( mirrorPoint );
            SaveCopyInUndoList( block->GetItems(), UR_MIRRORED_Y, mirrorPoint );
            MirrorY( block->GetItems(), mirrorPoint );
            OnModify();
        }

        GetScreen()->TestDanglingEnds( m_canvas, DC );
        m_canvas->Refresh();
        break;

    default:
        break;
    }

    if( blockCmdFinished )
    {
        block->Clear();
        GetScreen()->SetCurItem( NULL );
        m_canvas->EndMouseCapture( GetToolId(), m_canvas->GetCurrentCursor(), wxEmptyString,
                                   false );
    }
}


/* Traces the outline of the search block structures
 * The entire block follows the cursor
 */
static void DrawMovingBlockOutlines( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                                     bool aErase )
{
    BASE_SCREEN*    screen = aPanel->GetScreen();
    BLOCK_SELECTOR* block = &screen->m_BlockLocate;
    SCH_ITEM*       schitem;

    /* Erase old block contents. */
    if( aErase )
    {
        block->Draw( aPanel, aDC, block->GetMoveVector(), g_XorMode, block->GetColor() );

        for( unsigned ii = 0; ii < block->GetCount(); ii++ )
        {
            schitem = (SCH_ITEM*) block->GetItem( ii );
            schitem->Draw( aPanel, aDC, block->GetMoveVector(), g_XorMode, g_GhostColor );
        }
    }

    /* Repaint new view. */
    block->SetMoveVector( screen->GetCrossHairPosition() - block->GetLastCursorPosition() );
    block->Draw( aPanel, aDC, block->GetMoveVector(), g_XorMode, block->GetColor() );

    for( unsigned ii = 0; ii < block->GetCount(); ii++ )
    {
        schitem = (SCH_ITEM*) block->GetItem( ii );
        schitem->Draw( aPanel, aDC, block->GetMoveVector(), g_XorMode, g_GhostColor );
    }
}


void SCH_EDIT_FRAME::copyBlockItems( PICKED_ITEMS_LIST& aItemsList )
{
    m_blockItems.ClearListAndDeleteItems();   // delete previous saved list, if exists

    for( unsigned ii = 0; ii < aItemsList.GetCount(); ii++ )
    {
        // Clear m_Flag member of selected items:
        aItemsList.GetPickedItem( ii )->ClearFlags();

        /* Make a copy of the original picked item. */
        SCH_ITEM* copy = DuplicateStruct( (SCH_ITEM*) aItemsList.GetPickedItem( ii ) );
        copy->SetParent( NULL );

        // In list the wrapper is owner of the schematic item, we can use the UR_DELETED
        // status for the picker because pickers with this status are owner of the picked item
        // (or TODO ?: create a new status like UR_DUPLICATE)
        ITEM_PICKER item( copy, UR_DELETED );

        m_blockItems.PushItem( item );
    }
}


void SCH_EDIT_FRAME::PasteListOfItems( wxDC* DC )
{
    SCH_ITEM* Struct;

    if( m_blockItems.GetCount() == 0 )
    {
        DisplayError( this, wxT( "No struct to paste" ) );
        return;
    }

    PICKED_ITEMS_LIST picklist;

    for( unsigned ii = 0; ii < m_blockItems.GetCount(); ii++ )
    {
        Struct = DuplicateStruct( (SCH_ITEM*) m_blockItems.GetItem( ii ) );

        // Creates data, and push it as new data in undo item list buffer
        ITEM_PICKER picker( Struct, UR_NEW );
        picklist.PushItem( picker );

        // Clear annotation and init new time stamp for the new components:
        if( Struct->Type() == SCH_COMPONENT_T )
        {
            ( (SCH_COMPONENT*) Struct )->SetTimeStamp( GetNewTimeStamp() );
            ( (SCH_COMPONENT*) Struct )->ClearAnnotation( NULL );
        }

        SetSchItemParent( Struct, GetScreen() );
        Struct->Draw( m_canvas, DC, wxPoint( 0, 0 ), GR_DEFAULT_DRAWMODE );
        GetScreen()->Append( Struct );
    }

    SaveCopyInUndoList( picklist, UR_NEW );

    MoveItemsInList( picklist, GetScreen()->m_BlockLocate.GetMoveVector() );

    // Clear flags for all items.
    GetScreen()->ClearDrawingState();

    OnModify();

    return;
}
