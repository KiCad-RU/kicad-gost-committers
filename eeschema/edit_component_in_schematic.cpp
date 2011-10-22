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
 * @file edit_component_in_schematic.cpp
 * @brief Schematic component editing code.
 */

#include "fctsys.h"
#include "gr_basic.h"
#include "class_drawpanel.h"
#include "confirm.h"
#include "wxEeschemaStruct.h"

#include "general.h"
#include "protos.h"
#include "class_library.h"
#include "sch_component.h"


void SCH_EDIT_FRAME::EditComponentFieldText( SCH_FIELD* aField, wxDC* aDC )
{
    wxCHECK_RET( aField != NULL && aField->Type() == SCH_FIELD_T,
                 wxT( "Cannot edit invalid schematic field." ) );

    int            fieldNdx;
    SCH_COMPONENT* component = (SCH_COMPONENT*) aField->GetParent();

    wxCHECK_RET( component != NULL && component->Type() == SCH_COMPONENT_T,
                 wxT( "Invalid schematic field parent item." ) );

    LIB_COMPONENT* entry = CMP_LIBRARY::FindLibraryComponent( component->GetLibName() );

    wxCHECK_RET( entry != NULL, wxT( "Library entry for component <" ) +
                 component->GetLibName() + wxT( "> could not be found." ) );

    fieldNdx = aField->GetId();

    if( fieldNdx == VALUE && entry->IsPower() )
    {
        wxString msg;
        msg.Printf( _( "%s is a power component and it's value cannot be modified!\n\nYou must \
create a new power component with the new value." ), GetChars( entry->GetName() ) );
        DisplayInfoMessage( this, msg );
        return;
    }

    // Save old component in undo list if not already in edit, or moving.
    if( aField->GetFlags() == 0 )
        SaveCopyInUndoList( component, UR_CHANGED );

    // Don't use GetText() here.  If the field is the reference designator and it's parent
    // component has multiple parts, we don't want the part suffix added to the field.
    wxString newtext = aField->m_Text;
    DrawPanel->m_IgnoreMouseEvents = true;

    wxString title;
    title.Printf( _( "Edit %s Field" ), GetChars( aField->m_Name ) );

    wxTextEntryDialog dlg( this, wxEmptyString , title, newtext );
    int response = dlg.ShowModal();

    DrawPanel->MoveCursorToCrossHair();
    DrawPanel->m_IgnoreMouseEvents = false;
    newtext = dlg.GetValue( );
    newtext.Trim( true );
    newtext.Trim( false );

    if ( response != wxID_OK || newtext == aField->GetText() )
        return;  // canceled by user

    aField->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), g_XorMode );

    if( !newtext.IsEmpty() )
    {
        if( aField->m_Text.IsEmpty() )  // Means the field was not already in use
        {
            aField->m_Pos = component->GetPosition();
            aField->m_Size.x = aField->m_Size.y = m_TextFieldSize;
        }

        if( fieldNdx == REFERENCE )
        {
            // Test is reference is acceptable:
            if( SCH_COMPONENT::IsReferenceStringValid( newtext ) )
            {
                component->SetRef( GetSheet(), newtext );
                aField->m_Text = newtext;
            }
            else
            {
                DisplayError( this, _( "Illegal reference string!  No change" ) );
            }
        }
        else
        {
            aField->m_Text = newtext;
        }
    }
    else
    {
        if( fieldNdx == REFERENCE )
        {
            DisplayError( this, _( "The reference field cannot be empty!  No change" ) );
        }
        else if( fieldNdx == VALUE )
        {
            DisplayError( this, _( "The value field cannot be empty!  No change" ) );
        }
        else
        {
            aField->m_Text = wxT( "~" );
        }
    }

    aField->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), g_XorMode );
    component->DisplayInfo( this );
    OnModify();
}


void SCH_EDIT_FRAME::RotateField( SCH_FIELD* aField, wxDC* aDC )
{
    wxCHECK_RET( aField != NULL && aField->Type() == SCH_FIELD_T && !aField->GetText().IsEmpty(),
                 wxT( "Cannot rotate invalid schematic field." ) );

    SCH_COMPONENT* component = (SCH_COMPONENT*) aField->GetParent();

    // Save old component in undo list if not already in edit, or moving.
    if( aField->GetFlags() == 0 )
        SaveCopyInUndoList( component, UR_CHANGED );

    aField->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), g_XorMode );

    if( aField->m_Orient == TEXT_ORIENT_HORIZ )
        aField->m_Orient = TEXT_ORIENT_VERT;
    else
        aField->m_Orient = TEXT_ORIENT_HORIZ;

    aField->Draw( DrawPanel, aDC, wxPoint( 0, 0 ), g_XorMode );

    OnModify();
}
