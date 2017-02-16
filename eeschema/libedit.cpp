/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2017 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2017 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file libedit.cpp
 * @brief Eeschema component library editor.
 */

#include <fctsys.h>
#include <kiway.h>
#include <gr_basic.h>
#include <macros.h>
#include <pgm_base.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <gestfich.h>
#include <class_sch_screen.h>

#include <eeschema_id.h>
#include <general.h>
#include <libeditframe.h>
#include <class_library.h>
#include <template_fieldnames.h>
#include <wildcards_and_files_ext.h>

#include <dialog_choose_component.h>
#include <component_tree_search_container.h>

#include <dialogs/dialog_lib_new_component.h>


void LIB_EDIT_FRAME::DisplayLibInfos()
{
    PART_LIB* lib = GetCurLib();
    wxString title = wxString::Format( L"Part Library Editor \u2014 %s%s",
            lib ? lib->GetFullFileName() : _( "no library selected" ),
            lib && lib->IsReadOnly() ? _( " [Read Only] ") : wxString( wxEmptyString ) );

    SetTitle( title );
}


void LIB_EDIT_FRAME::SelectActiveLibrary( PART_LIB* aLibrary )
{
    if( !aLibrary )
        aLibrary = SelectLibraryFromList();

    if( aLibrary )
    {
        SetCurLib( aLibrary );
    }

    DisplayLibInfos();
}


bool LIB_EDIT_FRAME::LoadComponentAndSelectLib( LIB_ALIAS* aLibEntry, PART_LIB* aLibrary )
{
    if( GetScreen()->IsModify()
        && !IsOK( this, _( "The current component is not saved.\n\nDiscard current changes?" ) ) )
        return false;

    SelectActiveLibrary( aLibrary );
    return LoadComponentFromCurrentLib( aLibEntry );
}


bool LIB_EDIT_FRAME::LoadComponentFromCurrentLib( LIB_ALIAS* aLibEntry )
{
    if( !LoadOneLibraryPartAux( aLibEntry, GetCurLib() ) )
        return false;

    m_editPinsPerPartOrConvert = GetCurPart()->UnitsLocked() ? true : false;

    GetScreen()->ClearUndoRedoList();
    Zoom_Automatique( false );
    SetShowDeMorgan( GetCurPart()->HasConversion() );

    return true;
}


void LIB_EDIT_FRAME::LoadOneLibraryPart( wxCommandEvent& event )
{
    wxString   cmp_name;
    LIB_ALIAS* libEntry = NULL;

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    if( GetScreen()->IsModify()
        && !IsOK( this, _( "The current component is not saved.\n\nDiscard current changes?" ) ) )
        return;

    PART_LIB* lib = GetCurLib();

    // No current lib, ask user for the library to use.
    if( !lib )
    {
        SelectActiveLibrary();
        lib = GetCurLib();

        if( !lib )
            return;
    }

    // Get the name of the current part to preselect it
    LIB_PART* current_part = GetCurPart();
    wxString part_name = current_part ? current_part->GetName() : wxString( wxEmptyString );

    wxArrayString dummyHistoryList;
    int dummyLastUnit;
    SCHLIB_FILTER filter;
    filter.LoadFrom( lib->GetName() );
    cmp_name = SelectComponentFromLibrary( &filter, dummyHistoryList, dummyLastUnit,
                                          true, NULL, NULL, part_name );

    if( cmp_name.IsEmpty() )
        return;

    GetScreen()->ClrModify();
    m_lastDrawItem = m_drawItem = NULL;

    // Delete previous library component, if any
    SetCurPart( NULL );
    m_aliasName.Empty();

    // Load the new library component
    libEntry = lib->FindAlias( cmp_name );
    PART_LIB* searchLib = lib;

    if( !libEntry )
    {
        // Not found in the active library: search inside the full list
        // (can happen when using Viewlib to load a component)
        libEntry = Prj().SchLibs()->FindLibraryAlias( LIB_ID( wxEmptyString, cmp_name ) );

        if( libEntry )
        {
            searchLib = libEntry->GetLib();

            // The entry to load is not in the active lib
            // Ask for a new active lib
            wxString msg = _( "The selected component is not in the active library." );
            msg += "\n\n";
            msg += _( "Do you want to change the active library?" );

            if( IsOK( this, msg ) )
                SelectActiveLibrary( searchLib );
        }
    }

    if( !libEntry )
    {
        wxString msg = wxString::Format( _( "Part name '%s' not found in library '%s'" ),
                                         GetChars( cmp_name ),
                                         GetChars( searchLib->GetName() )  );
        DisplayError( this, msg );
        return;
    }

    PART_LIB* old = SetCurLib( searchLib );

    LoadComponentFromCurrentLib( libEntry );

    SetCurLib( old );

    DisplayLibInfos();
}


bool LIB_EDIT_FRAME::LoadOneLibraryPartAux( LIB_ALIAS* aEntry, PART_LIB* aLibrary )
{
    wxString msg, rootName;

    if( !aEntry || !aLibrary )
        return false;

    if( aEntry->GetName().IsEmpty() )
    {
        wxLogWarning( "Entry in library <%s> has empty name field.",
                      GetChars( aLibrary->GetName() ) );
        return false;
    }

    wxString cmpName = m_aliasName = aEntry->GetName();

    LIB_PART* lib_part = aEntry->GetPart();

    wxASSERT( lib_part );

    wxLogDebug( "\"<%s>\" is alias of \"<%s>\"",
                GetChars( cmpName ),
                GetChars( lib_part->GetName() ) );

    LIB_PART* part = new LIB_PART( *lib_part );      // clone it and own it.
    SetCurPart( part );
    m_aliasName = aEntry->GetName();

    m_unit = 1;
    m_convert = 1;

    m_showDeMorgan = false;

    if( part->HasConversion() )
        m_showDeMorgan = true;

    GetScreen()->ClrModify();
    DisplayLibInfos();
    UpdateAliasSelectList();
    UpdatePartSelectList();

    // Display the document information based on the entry selected just in
    // case the entry is an alias.
    DisplayCmpDoc();

    return true;
}


void LIB_EDIT_FRAME::RedrawComponent( wxDC* aDC, wxPoint aOffset  )
{
    LIB_PART* part = GetCurPart();

    if( part )
    {
        // display reference like in schematic (a reference U is shown U? or U?A)
        // although it is stored without ? and part id.
        // So temporary change the reference by a schematic like reference
        LIB_FIELD*  field = part->GetField( REFERENCE );
        wxString    fieldText = field->GetText();
        wxString    fieldfullText = field->GetFullText( m_unit );

        field->EDA_TEXT::SetText( fieldfullText );  // change the field text string only
        part->Draw( m_canvas, aDC, aOffset, m_unit, m_convert, GR_DEFAULT_DRAWMODE,
                    UNSPECIFIED_COLOR, DefaultTransform,
                    true, true,false, NULL, GetShowElectricalType() );
        field->EDA_TEXT::SetText( fieldText );      // restore the field text string
    }
}

void LIB_EDIT_FRAME::RedrawActiveWindow( wxDC* DC, bool EraseBg )
{
    if( GetScreen() == NULL )
        return;

    m_canvas->DrawBackGround( DC );

    RedrawComponent( DC, wxPoint( 0, 0 ) );

#ifdef USE_WX_OVERLAY
    if( IsShown() )
    {
        m_overlay.Reset();
        wxDCOverlay overlaydc( m_overlay, (wxWindowDC*)DC );
        overlaydc.Clear();
    }
#endif

    if( m_canvas->IsMouseCaptured() )
        m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

    m_canvas->DrawCrossHair( DC );

    DisplayLibInfos();
    UpdateStatusBar();
}


void LIB_EDIT_FRAME::OnSaveActiveLibrary( wxCommandEvent& event )
{
    bool newFile = false;
    if( event.GetId() == ID_LIBEDIT_SAVE_CURRENT_LIB_AS )
        newFile = true;

    this->SaveActiveLibrary( newFile );
}


bool LIB_EDIT_FRAME::SaveActiveLibrary( bool newFile )
{
    wxFileName fn;
    wxString   msg;

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    PART_LIB* lib = GetCurLib();

    // Just in case the library hasn't been cached yet.
    lib->GetCount();

    if( !lib )
    {
        DisplayError( this, _( "No library specified." ) );
        return false;
    }

    wxString oldFileName = lib->GetFullFileName();

    if( GetScreen()->IsModify() )
    {
        if( IsOK( this, _( "Include last component changes?" ) ) )
        {
            lib->EnableBuffering();

            try
            {
                SaveOnePart( lib, false );
            }
            catch( ... )
            {
                lib->EnableBuffering( false );
                msg.Printf( _( "Unexpected error occured saving part to '%s' symbol library." ),
                            lib->GetName() );
                DisplayError( this, msg );
                return false;
            }

            lib->EnableBuffering( false );
        }
    }

    if( newFile )
    {
        PROJECT&        prj = Prj();
        SEARCH_STACK*   search = prj.SchSearchS();

        // Get a new name for the library
        wxString default_path = prj.GetRString( PROJECT::SCH_LIB_PATH );

        if( !default_path )
            default_path = search->LastVisitedPath();

        wxFileDialog dlg( this, _( "Part Library Name:" ), default_path,
                          wxEmptyString, SchematicLibraryFileWildcard,
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

        if( dlg.ShowModal() == wxID_CANCEL )
            return false;

        fn = dlg.GetPath();

        // The GTK file chooser doesn't return the file extension added to
        // file name so add it here.
        if( fn.GetExt().IsEmpty() )
            fn.SetExt( SchematicLibraryFileExtension );

        prj.SetRString( PROJECT::SCH_LIB_PATH, fn.GetPath() );
    }
    else
    {
        fn = wxFileName( lib->GetFullFileName() );

        msg.Printf( _( "Modify library file '%s' ?" ), GetChars( fn.GetFullPath() ) );

        if( !IsOK( this, msg ) )
            return false;
    }

    // Verify the user has write privileges before attempting to save the library file.
    if( !IsWritable( fn ) )
        return false;

    ClearMsgPanel();

    wxFileName libFileName = fn;
    wxFileName backupFileName = fn;

    // Rename the old .lib file to .bak.
    if( libFileName.FileExists() )
    {
        backupFileName.SetExt( "bak" );

        if( backupFileName.FileExists() )
            wxRemoveFile( backupFileName.GetFullPath() );

        if( !wxRenameFile( libFileName.GetFullPath(), backupFileName.GetFullPath() ) )
        {
            libFileName.MakeAbsolute();
            msg = _( "Failed to rename old component library file " ) + backupFileName.GetFullPath();
            DisplayError( this, msg );
        }
    }

    wxFileName docFileName = libFileName;

    docFileName.SetExt( DOC_EXT );

    // Rename .doc file to .bck.
    if( docFileName.FileExists() )
    {
        backupFileName.SetExt( "bck" );

        if( backupFileName.FileExists() )
            wxRemoveFile( backupFileName.GetFullPath() );

        if( !wxRenameFile( docFileName.GetFullPath(), backupFileName.GetFullPath() ) )
        {
            msg = _( "Failed to save old library document file " ) + backupFileName.GetFullPath();
            DisplayError( this, msg );
        }
    }

    try
    {
        lib->SetFileName( fn.GetFullPath() );
        lib->Save();
    }
    catch( ... /* IO_ERROR ioe */ )
    {
        lib->SetFileName( oldFileName );
        msg.Printf( _( "Failed to create symbol library file '%s'" ),
                    GetChars( docFileName.GetFullPath() ) );
        DisplayError( this, msg );
        return false;
    }

    lib->SetFileName( oldFileName );
    msg.Printf( _( "Library file '%s' saved" ), GetChars( fn.GetFullPath() ) );
    fn.SetExt( DOC_EXT );
    wxString msg1;
    msg1.Printf( _( "Documentation file '%s' saved" ), GetChars( fn.GetFullPath() ) );
    AppendMsgPanel( msg, msg1, BLUE );
    UpdateAliasSelectList();
    UpdatePartSelectList();

    return true;
}


void LIB_EDIT_FRAME::DisplayCmpDoc()
{
    LIB_ALIAS*      alias;
    PART_LIB*    lib = GetCurLib();
    LIB_PART*       part = GetCurPart();

    ClearMsgPanel();

    if( !lib || !part )
        return;

    wxString msg = part->GetName();

    AppendMsgPanel( _( "Name" ), msg, BLUE, 8 );

    if( m_aliasName == part->GetName() )
        msg = _( "None" );
    else
        msg = m_aliasName;

    alias = part->GetAlias( m_aliasName );

    wxCHECK_RET( alias != NULL, "Alias not found in component." );

    AppendMsgPanel( _( "Alias" ), msg, RED, 8 );

#if defined(KICAD_GOST)
    msg.Printf( wxT( "%d" ) , m_unit );
#else
    static wxChar UnitLetter[] = wxT( "?ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
    msg = UnitLetter[m_unit];
#endif

    AppendMsgPanel( _( "Unit" ), msg, BROWN, 8 );

    if( m_convert > 1 )
        msg = _( "Convert" );
    else
        msg = _( "Normal" );

    AppendMsgPanel( _( "Body" ), msg, GREEN, 8 );

    if( part->IsPower() )
        msg = _( "Power Symbol" );
    else
        msg = _( "Part" );

    AppendMsgPanel( _( "Type" ), msg, MAGENTA, 8 );
    AppendMsgPanel( _( "Description" ), alias->GetDescription(), CYAN, 8 );
    AppendMsgPanel( _( "Key words" ), alias->GetKeyWords(), DARKDARKGRAY );
    AppendMsgPanel( _( "Datasheet" ), alias->GetDocFileName(), DARKDARKGRAY );
}


void LIB_EDIT_FRAME::DeleteOnePart( wxCommandEvent& event )
{
    wxString      cmp_name;
    LIB_ALIAS*    libEntry;
    wxArrayString nameList;
    wxString      msg;

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    m_lastDrawItem = NULL;
    m_drawItem = NULL;

    LIB_PART *part = GetCurPart();
    PART_LIB* lib = GetCurLib();

    if( !lib )
    {
        SelectActiveLibrary();

        lib = GetCurLib();
        if( !lib )
        {
            DisplayError( this, _( "Please select a component library." ) );
            return;
        }
    }

    COMPONENT_TREE_SEARCH_CONTAINER search_container( Prj().SchLibs() );

    wxString name = part ? part->GetName() : wxString( wxEmptyString );
    search_container.SetPreselectNode( name, /* aUnit */ 0 );
    search_container.ShowUnits( false );
    search_container.AddLibrary( *lib );

    wxString dialogTitle;
    dialogTitle.Printf( _( "Delete Component (%u items loaded)" ), search_container.GetComponentsCount() );

    DIALOG_CHOOSE_COMPONENT dlg( this, dialogTitle, &search_container, m_convert );

    if( dlg.ShowModal() == wxID_CANCEL )
    {
        return;
    }

    libEntry = dlg.GetSelectedAlias( NULL );

    if( !libEntry )
    {
        msg.Printf( _( "Entry '%s' not found in library '%s'." ),
                    GetChars( libEntry->GetName() ),
                    GetChars( lib->GetName() ) );
        DisplayError( this, msg );
        return;
    }

    msg.Printf( _( "Delete component '%s' from library '%s' ?" ),
                GetChars( libEntry->GetName() ),
                GetChars( lib->GetName() ) );

    if( !IsOK( this, msg ) )
        return;

    part = GetCurPart();

    if( !part || !part->HasAlias( libEntry->GetName() ) )
    {
        lib->RemoveAlias( libEntry );
        m_canvas->Refresh();
        return;
    }

    // If deleting the current entry or removing one of the aliases for
    // the current entry, sync the changes in the current entry as well.

    if( GetScreen()->IsModify() && !IsOK( this, _(
        "The component being deleted has been modified."
        " All changes will be lost. Discard changes?" ) ) )
    {
        return;
    }

    LIB_ALIAS* nextEntry = lib->RemoveAlias( libEntry );

    if( nextEntry != NULL )
    {
        if( LoadOneLibraryPartAux( nextEntry, lib ) )
            Zoom_Automatique( false );
    }
    else
    {
        SetCurPart( NULL );     // delete CurPart
        m_aliasName.Empty();
    }

    m_canvas->Refresh();
}



void LIB_EDIT_FRAME::CreateNewLibraryPart( wxCommandEvent& event )
{
    wxString name;

    if( GetCurPart() && GetScreen()->IsModify() && !IsOK( this, _(
        "All changes to the current component will be lost!\n\n"
        "Clear the current component from the screen?" ) ) )
    {
        return;
    }

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    m_drawItem = NULL;

    DIALOG_LIB_NEW_COMPONENT dlg( this );

    dlg.SetMinSize( dlg.GetSize() );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    if( dlg.GetName().IsEmpty() )
    {
        wxMessageBox( _( "This new component has no name and cannot be created. Aborted" ) );
        return;
    }

    name = dlg.GetName();
    name.Replace( " ", "_" );

    PART_LIB* lib = GetCurLib();

    // Test if there a component with this name already.
    if( lib && lib->FindAlias( name ) )
    {
        wxString msg = wxString::Format( _(
            "Part '%s' already exists in library '%s'" ),
            GetChars( name ),
            GetChars( lib->GetName() )
            );
        DisplayError( this, msg );
        return;
    }

    LIB_PART* new_part = new LIB_PART( name );

    SetCurPart( new_part );
    m_aliasName = new_part->GetName();

    new_part->GetReferenceField().SetText( dlg.GetReference() );
    new_part->SetUnitCount( dlg.GetUnitCount() );

    // Initialize new_part->m_TextInside member:
    // if 0, pin text is outside the body (on the pin)
    // if > 0, pin text is inside the body
    new_part->SetConversion( dlg.GetAlternateBodyStyle() );
    SetShowDeMorgan( dlg.GetAlternateBodyStyle() );

    if( dlg.GetPinNameInside() )
    {
        new_part->SetPinNameOffset( dlg.GetPinTextPosition() );

        if( new_part->GetPinNameOffset() == 0 )
            new_part->SetPinNameOffset( 1 );
    }
    else
    {
        new_part->SetPinNameOffset( 0 );
    }

    ( dlg.GetPowerSymbol() ) ? new_part->SetPower() : new_part->SetNormal();
    new_part->SetShowPinNumbers( dlg.GetShowPinNumber() );
    new_part->SetShowPinNames( dlg.GetShowPinName() );
    new_part->LockUnits( dlg.GetLockItems() );

    if( dlg.GetUnitCount() < 2 )
        new_part->LockUnits( false );

    m_unit = 1;
    m_convert  = 1;

    DisplayLibInfos();
    DisplayCmpDoc();
    UpdateAliasSelectList();
    UpdatePartSelectList();

    m_editPinsPerPartOrConvert = new_part->UnitsLocked() ? true : false;
    m_lastDrawItem = NULL;

    GetScreen()->ClearUndoRedoList();
    OnModify();

    m_canvas->Refresh();
    m_mainToolBar->Refresh();
}


bool LIB_EDIT_FRAME::SaveOnePart( PART_LIB* aLib, bool aPromptUser )
{
    wxString    msg;
    LIB_PART*   part = GetCurPart();
    LIB_PART*   old_part = NULL;

    GetScreen()->ClrModify();

    if( !wxFileName::FileExists( aLib->GetFullFileName() ) )
    {
        aLib->Create();
    }
    else
    {
        old_part = aLib->FindPart( part->GetName() );

        if( old_part && aPromptUser )
        {
            msg.Printf( _( "Part '%s' already exists. Change it?" ),
                        GetChars( part->GetName() ) );

            if( !IsOK( this, msg ) )
                return false;
        }
    }

    m_drawItem = m_lastDrawItem = NULL;

    if( old_part )
        aLib->ReplacePart( old_part, part );
    else
        aLib->AddPart( part );

    msg.Printf( _( "Part '%s' saved in library '%s'" ),
                GetChars( part->GetName() ),
                GetChars( aLib->GetName() ) );

    SetStatusText( msg );

    return true;
}
