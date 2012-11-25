/************************************************/
/* Module editor: Dialog box for editing module */
/*  properties and characteristics              */
/************************************************/

#include <fctsys.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <pcbnew.h>
#include <wxPcbStruct.h>
#include <module_editor_frame.h>
#include <trigo.h>
#include <3d_viewer.h>

#include <class_module.h>
#include <class_pad.h>
#include <class_edge_mod.h>

#include <dialog_edit_module_for_BoardEditor.h>


/*
 * Show module property dialog.
 */
void PCB_EDIT_FRAME::InstallModuleOptionsFrame( MODULE* Module, wxDC* DC )
{
    if( Module == NULL )
        return;

#ifndef __WXMAC__
    DIALOG_MODULE_BOARD_EDITOR* dialog = new DIALOG_MODULE_BOARD_EDITOR( this, Module, DC );
#else
    // avoid Avoid "writes" in the dialog, creates errors with WxOverlay and NSView & Modal
    // Raising an Exception - Fixes #764678
    DIALOG_MODULE_BOARD_EDITOR* dialog = new DIALOG_MODULE_BOARD_EDITOR( this, Module, NULL );
#endif
    
    int retvalue = dialog->ShowModal(); /* retvalue =
                                         *  -1 if abort,
                                         *  0 if exchange module,
                                         *  1 for normal edition
                                         *  and 2 for a goto editor command
                                         */
    dialog->Destroy();

#ifdef __WXMAC__
    // If something edited, push a refresh request
    if (retvalue == 0 || retvalue == 1)
        m_canvas->Refresh();
#endif

    if( retvalue == 2 )
    {
        FOOTPRINT_EDIT_FRAME * editorFrame =
                FOOTPRINT_EDIT_FRAME::GetActiveFootprintEditor();
        if( editorFrame == NULL )
            editorFrame = new FOOTPRINT_EDIT_FRAME( this );

        editorFrame->Load_Module_From_BOARD( Module );
        SetCurItem( NULL );

        editorFrame->Show( true );
        editorFrame->Iconize( false );
    }
}


/*
 * Move the footprint anchor position to the current cursor position.
 */
void FOOTPRINT_EDIT_FRAME::Place_Ancre( MODULE* aModule )
{
    wxPoint   moveVector;

    if( aModule == NULL )
        return;

    moveVector = aModule->m_Pos - GetScreen()->GetCrossHairPosition();

    aModule->m_Pos = GetScreen()->GetCrossHairPosition();

    /* Update the relative coordinates:
     * The coordinates are relative to the anchor point.
     * Calculate deltaX and deltaY from the anchor. */
    RotatePoint( &moveVector, -aModule->m_Orient );

    // Update the pad coordinates.
    for( D_PAD* pad = (D_PAD*) aModule->m_Pads;  pad;  pad = pad->Next() )
    {
        pad->SetPos0( pad->GetPos0() + moveVector );
    }

    // Update the draw element coordinates.
    for( EDA_ITEM* item = aModule->m_Drawings;  item;  item = item->Next() )
    {
        switch( item->Type() )
        {
        case PCB_MODULE_EDGE_T:
            #undef STRUCT
            #define STRUCT ( (EDGE_MODULE*) item )
            STRUCT->m_Start0 += moveVector;
            STRUCT->m_End0   += moveVector;
            break;

        case PCB_MODULE_TEXT_T:
            #undef STRUCT
            #define STRUCT ( (TEXTE_MODULE*) item )
            STRUCT->SetPos0( STRUCT->GetPos0() + moveVector );
            break;

        default:
            break;
        }
    }

    aModule->CalculateBoundingBox();
}


void FOOTPRINT_EDIT_FRAME::RemoveStruct( EDA_ITEM* Item )
{
    if( Item == NULL )
        return;

    switch( Item->Type() )
    {
    case PCB_PAD_T:
        DeletePad( (D_PAD*) Item, false );
        break;

    case PCB_MODULE_TEXT_T:
    {
        TEXTE_MODULE* text = (TEXTE_MODULE*) Item;

        if( text->GetType() == TEXT_is_REFERENCE )
        {
            DisplayError( this, _( "Text is REFERENCE!" ) );
            break;
        }

        if( text->GetType() == TEXT_is_VALUE )
        {
            DisplayError( this, _( "Text is VALUE!" ) );
            break;
        }

        DeleteTextModule( text );
    }
    break;

    case PCB_MODULE_EDGE_T:
        Delete_Edge_Module( (EDGE_MODULE*) Item );
        m_canvas->Refresh();
        break;

    case PCB_MODULE_T:
        break;

    default:
    {
        wxString Line;
        Line.Printf( wxT( " Remove: draw item type %d unknown." ), Item->Type() );
        DisplayError( this, Line );
    }
    break;
    }
}
