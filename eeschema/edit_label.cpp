/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file edit_label.cpp
 * @brief Label, global label and text creation and editing.
 */

#include "fctsys.h"
#include "gr_basic.h"
#include "base_struct.h"
#include "drawtxt.h"
#include "class_drawpanel.h"
#include "confirm.h"
#include "wxEeschemaStruct.h"
#include "kicad_device_context.h"

#include "general.h"
#include "protos.h"
#include "sch_text.h"
#include "eeschema_id.h"


static int       lastGlobalLabelShape = (int) NET_INPUT;
static int       lastTextOrientation = 0;
static bool      lastTextBold = false;
static bool      lastTextItalic = false;


void SCH_EDIT_FRAME::ChangeTextOrient( SCH_TEXT* aTextItem, wxDC* aDC )
{
    wxCHECK_RET( (aTextItem != NULL) && aTextItem->CanIncrementLabel(),
                 wxT( "Invalid schematic text item." )  );

    int orient = ( aTextItem->GetOrientation() + 1 ) & 3;

    // Save current text orientation in undo list if is not already in edit.
    if( aTextItem->GetFlags() == 0 )
        SaveCopyInUndoList( aTextItem, UR_CHANGED );

    DrawPanel->CrossHairOff( aDC );
    aTextItem->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), g_XorMode );
    aTextItem->SetOrientation( orient );
    OnModify();
    aTextItem->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), g_XorMode );
    DrawPanel->CrossHairOn( aDC );
}


SCH_TEXT* SCH_EDIT_FRAME::CreateNewText( wxDC* aDC, int aType )
{
    SCH_TEXT* textItem = NULL;

    m_itemToRepeat = NULL;

    switch( aType )
    {
    case LAYER_NOTES:
        textItem = new SCH_TEXT( GetScreen()->GetCrossHairPosition() );
        break;

    case LAYER_LOCLABEL:
        textItem = new SCH_LABEL( GetScreen()->GetCrossHairPosition() );
        break;

    case LAYER_HIERLABEL:
        textItem = new SCH_HIERLABEL( GetScreen()->GetCrossHairPosition() );
        textItem->m_Shape = lastGlobalLabelShape;
        break;

    case LAYER_GLOBLABEL:
        textItem = new SCH_GLOBALLABEL( GetScreen()->GetCrossHairPosition() );
        textItem->m_Shape = lastGlobalLabelShape;
        break;

    default:
        DisplayError( this, wxT( "SCH_EDIT_FRAME::CreateNewText() Internal error" ) );
        return NULL;
    }

    textItem->m_Bold = lastTextBold;
    textItem->m_Italic = lastTextItalic;
    textItem->SetOrientation( lastTextOrientation );
    textItem->m_Size.x = textItem->m_Size.y = g_DefaultTextLabelSize;
    textItem->SetFlags( IS_NEW | IS_MOVED );

    textItem->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), g_XorMode );
    EditSchematicText( textItem );

    if( textItem->m_Text.IsEmpty() )
    {
        SAFE_DELETE( textItem );
        return NULL;
    }

    lastTextBold = textItem->m_Bold;
    lastTextItalic = textItem->m_Italic;
    lastTextOrientation = textItem->GetOrientation();

    if( (aType == SCH_GLOBAL_LABEL_T) || (aType == SCH_HIERARCHICAL_LABEL_T) )
    {
        lastGlobalLabelShape = textItem->m_Shape;
    }

    textItem->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), GR_DEFAULT_DRAWMODE );
    MoveItem( (SCH_ITEM*) textItem, aDC );

    return textItem;
}


/*
 * OnConvertTextType is a command event handler to change a text type to an other one.
 * The new text, label, hierarchical label, or global label is created from the old text
 * The old text is deleted.
 * A tricky case is when the 'old" text is being edited (i.e. moving)
 * because we must create a new text, and prepare the undo/redo command data for this
 * change and the current move/edit command
 */
void SCH_EDIT_FRAME::OnConvertTextType( wxCommandEvent& aEvent )
{
    SCH_SCREEN* screen = GetScreen();
    SCH_TEXT* text = (SCH_TEXT*) screen->GetCurItem();

    wxCHECK_RET( (text != NULL) && text->CanIncrementLabel(),
                 wxT( "Cannot convert text type." ) );

    KICAD_T type;

    switch( aEvent.GetId() )
    {
    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL:
        type = SCH_LABEL_T;
        break;

    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL:
        type = SCH_GLOBAL_LABEL_T;
        break;

    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL:
        type = SCH_HIERARCHICAL_LABEL_T;
        break;

    case ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT:
        type = SCH_TEXT_T;
        break;

    default:
        wxFAIL_MSG( wxString::Format( wxT( "Invalid text type command ID %d." ),
                                      aEvent.GetId() ) );
        return;
    }

    if( text->Type() == type )
        return;

    SCH_TEXT* newtext;

    switch( type )
    {
    case SCH_LABEL_T:
        newtext = new SCH_LABEL( text->m_Pos, text->m_Text );
        break;

    case SCH_GLOBAL_LABEL_T:
        newtext = new SCH_GLOBALLABEL( text->m_Pos, text->m_Text );
        break;

    case SCH_HIERARCHICAL_LABEL_T:
        newtext = new SCH_HIERLABEL( text->m_Pos, text->m_Text );
        break;

    case SCH_TEXT_T:
        newtext = new SCH_TEXT( text->m_Pos, text->m_Text );
        break;

    default:
        newtext = NULL;
        wxFAIL_MSG( wxString::Format( wxT( "Cannot convert text type to %d" ), type ) );
        return;
    }

    /* Copy the old text item settings to the new one.  Justifications are not copied because
     * they are not used in labels.  Justifications will be set to default value in the new
     * text item type.
     */
    newtext->SetFlags( text->GetFlags() );
    newtext->m_Shape = text->m_Shape;
    newtext->SetOrientation( text->GetOrientation() );
    newtext->m_Size = text->m_Size;
    newtext->m_Thickness = text->m_Thickness;
    newtext->m_Italic = text->m_Italic;
    newtext->m_Bold = text->m_Bold;

    /* Save the new text in undo list if the old text was not itself a "new created text"
     * In this case, the old text is already in undo list as a deleted item.
     * Of course if the old text was a "new created text" the new text will be
     * put in undo list later, at the end of the current command (if not aborted)
     */

    INSTALL_UNBUFFERED_DC( dc, DrawPanel );
    DrawPanel->CrossHairOff( &dc );   // Erase schematic cursor
    text->Draw( DrawPanel, &dc, wxPoint( 0, 0 ), g_XorMode );

    screen->RemoveFromDrawList( text );
    screen->AddToDrawList( newtext );
    GetScreen()->SetCurItem( newtext );
    m_itemToRepeat = NULL;
    OnModify();
    newtext->Draw( DrawPanel, &dc, wxPoint( 0, 0 ), GR_DEFAULT_DRAWMODE );
    DrawPanel->CrossHairOn( &dc );    // redraw schematic cursor

    if( text->IsNew() )
    {
        // if the previous text is new, no undo command to prepare here
        // just delete this previous text.
        delete text;
        return;
    }

    // previous text is not new and we replace text by new text.
    // So this is equivalent to delete text and add newtext
    // If text if being currently edited (i.e. moved)
    // we also save the initial copy of text, and prepare undo command for new text modifications.
    // we must save it as modified text (if currently beeing edited), then deleted text,
    // and replace text with newtext
    PICKED_ITEMS_LIST pickList;
    ITEM_PICKER picker( text, UR_CHANGED );

    if( text->GetFlags() )
    {
        // text is being edited, save initial text for undo command
        picker.SetLink( GetUndoItem() );
        pickList.PushItem( picker );

        // the owner of undoItem is no more "this", it is now "picker":
        SetUndoItem( NULL );

        // save current newtext copy for undo/abort current command
        SetUndoItem( newtext );
    }

    // Prepare undo command for delete old text
    picker.m_UndoRedoStatus = UR_DELETED;
    picker.SetLink( NULL );
    pickList.PushItem( picker );

    // Prepare undo command for new text
    picker.m_UndoRedoStatus = UR_NEW;
    picker.SetItem(newtext);
    pickList.PushItem( picker );

    SaveCopyInUndoList( pickList, UR_UNSPECIFIED );
}


/* Function to increment bus label members numbers,
 * i.e. when a text is ending with a number, adds
 * <RepeatDeltaLabel> to this number
 */
void IncrementLabelMember( wxString& name )
{
    int  ii, nn;
    long number = 0;

    ii = name.Len() - 1; nn = 0;

    if( !isdigit( name.GetChar( ii ) ) )
        return;

    while( (ii >= 0) && isdigit( name.GetChar( ii ) ) )
    {
        ii--; nn++;
    }

    ii++;   /* digits are starting at ii position */
    wxString litt_number = name.Right( nn );

    if( litt_number.ToLong( &number ) )
    {
        number += g_RepeatDeltaLabel;
        name.Remove( ii ); name << number;
    }
}
