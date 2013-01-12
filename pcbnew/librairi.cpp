/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2012 KiCad Developers, see change_log.txt for contributors.
 *
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
 * @file librairi.cpp
 * @brief Manage module (footprint) libraries.
 */

#include <fctsys.h>
#include <wx/ffile.h>
#include <appl_wxstruct.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <kicad_string.h>
#include <gestfich.h>
#include <wxPcbStruct.h>
#include <dialog_helpers.h>
#include <richio.h>
#include <filter_reader.h>
#include <pcbcommon.h>
#include <macros.h>

#include <class_board.h>
#include <class_module.h>

#include <pcbnew.h>
#include <module_editor_frame.h>
#include <wildcards_and_files_ext.h>
#include <kicad_plugin.h>
#include <legacy_plugin.h>


// unique, "file local" translations:

#define FMT_OK_OVERWRITE    _( "Library '%s' exists, OK to replace ?" )
#define FMT_CREATE_LIB      _( "Create New Library" )
#define FMT_OK_DELETE       _( "Ok to delete module '%s' in library '%s'" )
#define FMT_IMPORT_MODULE   _( "Import Footprint Module" )
#define FMT_FILE_NOT_FOUND  _( "File '%s' not found" )
#define FMT_NOT_MODULE      _( "Not a module file" )
#define FMT_MOD_NOT_FOUND   _( "Unable to find or load footprint '%s' from lib path '%s'" )
#define FMT_BAD_PATH        _( "Unable to find or load footprint from path '%s'" )
#define FMT_BAD_PATHS       _( "The footprint library '%s' could not be found in any of the search paths." )
#define FMT_LIB_READ_ONLY   _( "Library '%s' is read only, not writable" )

#define FMT_EXPORT_MODULE   _( "Export Module" )
#define FMT_SAVE_MODULE     _( "Save Module" )
#define FMT_MOD_REF         _( "Module Reference:" )
#define FMT_EXPORTED        _( "Module exported to file '%s'" )
#define FMT_MOD_DELETED     _( "Component '%s' deleted from library '%s'" )
#define FMT_MOD_CREATE      _( "Module Creation" )

#define FMT_NO_MODULES      _( "No modules to archive!" )
#define FMT_LIBRARY         _( "Library" )                                      // window title
#define FMT_MOD_EXISTS      _( "Footprint '%s' already exists in library '%s'" )
#define FMT_NO_REF_ABORTED  _( "No reference, aborted" )
#define FMT_SELECT_LIB      _( "Select Active Library:" )


static const wxString ModExportFileWildcard( _( "KiCad foot print export files (*.emp)|*.emp" ) );
static const wxString ModImportFileWildcard( _( "GPcb foot print files (*)|*" ) );


#define BACKUP_EXT                 wxT( "bak" )
#define FILETMP_EXT                wxT( "$$$" )
#define EXPORT_IMPORT_LASTPATH_KEY wxT( "import_last_path" )

const wxString        ModExportFileExtension( wxT( "emp" ) );


MODULE* FOOTPRINT_EDIT_FRAME::Import_Module()
{
    // use the clipboard for this in the future?

    // Some day it might be useful save the last library type selected aong with the path.
    static int lastFilterIndex = 0;
    wxString   lastOpenedPathForLoading;
    wxConfig*  config = wxGetApp().GetSettings();

    if( config )
        config->Read( EXPORT_IMPORT_LASTPATH_KEY, &lastOpenedPathForLoading );

    wxString wildCard;

    wildCard << wxGetTranslation( KiCadFootprintLibFileWildcard ) << wxChar( '|' )
             << wxGetTranslation( ModExportFileWildcard ) << wxChar( '|' )
             << wxGetTranslation( ModImportFileWildcard ) << wxChar( '|' )
             << wxGetTranslation( GedaPcbFootprintLibFileWildcard );

    wxFileDialog dlg( this, FMT_IMPORT_MODULE,
                      lastOpenedPathForLoading, wxEmptyString,
                      wildCard, wxFD_OPEN | wxFD_FILE_MUST_EXIST );
    dlg.SetFilterIndex( lastFilterIndex );

    if( dlg.ShowModal() == wxID_CANCEL )
        return NULL;

    lastFilterIndex = dlg.GetFilterIndex();

    FILE* fp = wxFopen( dlg.GetPath(), wxT( "rt" ) );

    if( !fp )
    {
        wxString msg = wxString::Format( FMT_FILE_NOT_FOUND, GetChars( dlg.GetPath() ) );
        DisplayError( this, msg );
        return NULL;
    }

    if( config )    // Save file path
    {
        lastOpenedPathForLoading = wxPathOnly( dlg.GetPath() );
        config->Write( EXPORT_IMPORT_LASTPATH_KEY, lastOpenedPathForLoading );
    }

    wxString    moduleName;

    bool        isGeda   = false;
    bool        isLegacy = false;

    {
        FILE_LINE_READER         freader( fp, dlg.GetPath() );   // I own fp, and will close it.
        WHITESPACE_FILTER_READER reader( freader );              // skip blank lines

        reader.ReadLine();
        char* line = reader.Line();

        if( !strnicmp( line, "(module", 7 ) )
        {
            // isKicad = true;
        }
        else if( !strnicmp( line, FOOTPRINT_LIBRARY_HEADER, FOOTPRINT_LIBRARY_HEADER_CNT ) )
        {
            isLegacy = true;

            while( reader.ReadLine() )
            {
                if( !strnicmp( line, "$MODULE", 7 ) )
                {
                    moduleName = FROM_UTF8( StrPurge( line + sizeof( "$MODULE" ) -1 ) );
                    break;
                }
            }
        }
        else if( !strnicmp( line, "Element", 7 ) )
        {
            isGeda = true;
        }
        else
        {
            DisplayError( this, FMT_NOT_MODULE );
            return NULL;
        }

        // fp is closed here by ~FILE_LINE_READER()
    }

    MODULE*   module;

    if( isGeda )
    {
        try
        {
            wxFileName fn = dlg.GetPath();
            PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::GEDA_PCB ) );

            moduleName = fn.GetName();
            module = pi->FootprintLoad( fn.GetPath(), moduleName );

            if( !module )
            {
                wxString msg = wxString::Format(
                    FMT_MOD_NOT_FOUND, GetChars( moduleName ), GetChars( fn.GetPath() ) );

                DisplayError( this, msg );
                return NULL;
            }
        }
        catch( IO_ERROR ioe )
        {
            DisplayError( this, ioe.errorText );
            return NULL;
        }
    }
    else if( isLegacy )
    {
        try
        {
            PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::LEGACY ) );

            module = pi->FootprintLoad( dlg.GetPath(), moduleName );

            if( !module )
            {
                wxString msg = wxString::Format(
                    FMT_MOD_NOT_FOUND, GetChars( moduleName ), GetChars( dlg.GetPath() ) );

                DisplayError( this, msg );
                return NULL;
            }
        }
        catch( IO_ERROR ioe )
        {
            DisplayError( this, ioe.errorText );
            return NULL;
        }
    }
    else    //  if( isKicad )
    {
        try
        {
            // This technique was chosen to create an example of how reading
            // the s-expression format from clipboard could be done.

            wxString    fcontents;
            PCB_IO      pcb_io;
            wxFFile     f( dlg.GetPath() );

            if( !f.IsOpened() )
            {
                wxString msg = wxString::Format( FMT_BAD_PATH, GetChars( dlg.GetPath() ) );

                DisplayError( this, msg );
                return NULL;
            }

            f.ReadAll( &fcontents );

            module = dynamic_cast<MODULE*>( pcb_io.Parse( fcontents ) );

            if( !module )
            {
                wxString msg = wxString::Format( FMT_BAD_PATH, GetChars( dlg.GetPath() ) );

                DisplayError( this, msg );
                return NULL;
            }
        }
        catch( IO_ERROR ioe )
        {
            DisplayError( this, ioe.errorText );
            return NULL;
        }
    }

    // Insert footprint in list
    GetBoard()->Add( module );

    // Display info :
    SetMsgPanel( module );
    PlaceModule( module, NULL );
    GetBoard()->m_Status_Pcb = 0;
    GetBoard()->BuildListOfNets();

    return module;
}


void FOOTPRINT_EDIT_FRAME::Export_Module( MODULE* aModule )
{
    wxFileName  fn;
    wxConfig*   config = wxGetApp().GetSettings();

    if( aModule == NULL )
        return;

    fn.SetName( aModule->m_LibRef );

    wxString    wildcard = wxGetTranslation( KiCadFootprintLibFileWildcard );

    fn.SetExt( KiCadFootprintFileExtension );

    if( config )
    {
        wxString    path;
        config->Read( EXPORT_IMPORT_LASTPATH_KEY, &path );
        fn.SetPath( path );
    }

    wxFileDialog dlg( this, FMT_EXPORT_MODULE, fn.GetPath(), fn.GetFullName(),
                        wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    fn = dlg.GetPath();

    if( config )  // Save file path
    {
        config->Write( EXPORT_IMPORT_LASTPATH_KEY, fn.GetPath() );
    }

    try
    {
        // Export as *.kicad_pcb format, using a strategy which is specifically chosen
        // as an example on how it could also be used to send it to the system clipboard.

        PCB_IO  pcb_io( CTL_FOR_LIBRARY );

        /*  This module should *already* be "normalized" in a way such that
            orientation is zero, etc., since it came from module editor.

            module->SetTimeStamp( 0 );
            module->SetParent( 0 );
            module->SetOrientation( 0 );
        */

        pcb_io.Format( aModule );

        FILE* fp = wxFopen( dlg.GetPath(), wxT( "wt" ) );
        fprintf( fp, "%s", pcb_io.GetStringOutput( false ).c_str() );
        fclose( fp );
    }
    catch( IO_ERROR ioe )
    {
        DisplayError( this, ioe.errorText );
        return;
    }

    wxString msg = wxString::Format( FMT_EXPORTED, GetChars( dlg.GetPath() ) );
    DisplayInfoMessage( this, msg );
}


bool FOOTPRINT_EDIT_FRAME::SaveCurrentModule( const wxString* aLibPath )
{
    wxString            libPath = aLibPath ? *aLibPath : getLibPath();

    IO_MGR::PCB_FILE_T  piType = IO_MGR::GuessPluginTypeFromLibPath( libPath );

    try
    {
        PLUGIN::RELEASER  pi( IO_MGR::PluginFind( piType ) );

        pi->FootprintSave( libPath, GetBoard()->m_Modules );
    }
    catch( IO_ERROR ioe )
    {
        DisplayError( this, ioe.errorText );
        return false;
    }
    return true;
}


wxString FOOTPRINT_EDIT_FRAME::CreateNewLibrary()
{
    wxFileName  fn;
    wxConfig*   config = wxGetApp().GetSettings();

    if( config )
    {
        wxString    path;
        config->Read( EXPORT_IMPORT_LASTPATH_KEY, &path );
        fn.SetPath( path );
    }

    wxString    wildcard;

    wildcard << wxGetTranslation( LegacyFootprintLibPathWildcard ) << wxChar('|')
             << wxGetTranslation( KiCadFootprintLibPathWildcard );

    // prompt user for libPath and PLUGIN (library) type
    wxFileDialog dlg( this, FMT_CREATE_LIB, fn.GetPath(), wxEmptyString,
                      wildcard,
                      wxFD_SAVE
                      // | wxFD_OVERWRITE_PROMPT overwrite is tested below
                      // after file extension has been added.
                      );

    if( dlg.ShowModal() == wxID_CANCEL )
        return wxEmptyString;

    fn = dlg.GetPath();

    if( config )  // Save file path without filename, save user typing.
    {
        config->Write( EXPORT_IMPORT_LASTPATH_KEY, fn.GetPath() );
    }

    // wildcard's filter index has legacy in position 0.
    IO_MGR::PCB_FILE_T  piType = ( dlg.GetFilterIndex() == 0 ) ? IO_MGR::LEGACY : IO_MGR::KICAD;

    // wxFileDialog does not supply nor enforce the file extension, add it here.
    if( piType == IO_MGR::LEGACY )
    {
        fn.SetExt( LegacyFootprintLibPathExtension );
    }
    else
    {
        fn.SetExt( KiCadFootprintLibPathExtension );
    }

    wxString libPath = fn.GetFullPath();

    try
    {
        PLUGIN::RELEASER  pi( IO_MGR::PluginFind( piType ) );

        bool    writable = false;
        bool    exists   = false;

        try
        {
            writable = pi->IsFootprintLibWritable( libPath );
            exists   = true;    // no exception was thrown, lib must exist.
        }
        catch( IO_ERROR )
        {
            // ignore, original values of 'writable' and 'exists' are accurate.
        }

        if( exists )
        {
            if( !writable )
            {
                wxString msg = wxString::Format( FMT_LIB_READ_ONLY, GetChars( libPath ) );
                DisplayError( this, msg );
                return wxEmptyString;
            }
            else
            {
                wxString msg = wxString::Format( FMT_OK_OVERWRITE, GetChars( libPath ) );

                if( !IsOK( this, msg ) )
                    return wxEmptyString;

                pi->FootprintLibDelete( libPath );
            }
        }

        pi->FootprintLibCreate( libPath );
    }
    catch( IO_ERROR ioe )
    {
        DisplayError( this, ioe.errorText );
        return wxEmptyString;
    }

    return libPath;
}


bool FOOTPRINT_EDIT_FRAME::DeleteModuleFromCurrentLibrary()
{
    wxString    libPath = getLibPath();
    wxString    footprintName = Select_1_Module_From_List( this, libPath,
                        wxEmptyString, wxEmptyString );

    if( !footprintName )
        return false;

    // Confirmation
    wxString msg = wxString::Format( FMT_OK_DELETE, footprintName.GetData(), libPath.GetData() );

    if( !IsOK( this, msg ) )
        return false;

    IO_MGR::PCB_FILE_T pluginType = IO_MGR::GuessPluginTypeFromLibPath( libPath );

    try
    {
        PLUGIN::RELEASER  pi( IO_MGR::PluginFind( pluginType ) );

        pi->FootprintDelete( libPath, footprintName );
    }
    catch( IO_ERROR ioe )
    {
        DisplayError( NULL, ioe.errorText );
        return false;
    }

    msg.Printf( FMT_MOD_DELETED, footprintName.GetData(), libPath.GetData() );

    SetStatusText( msg );

    return true;
}


/* Save modules in a library:
 * param aNewModulesOnly:
 *              true : save modules not already existing in this lib
 *              false: save all modules
 */
void PCB_EDIT_FRAME::ArchiveModulesOnBoard( const wxString& aLibName, bool aNewModulesOnly )
{
    wxString fileName = aLibName;
    wxString path;

    if( GetBoard()->m_Modules == NULL )
    {
        DisplayInfoMessage( this, FMT_NO_MODULES );
        return;
    }

    path = wxGetApp().ReturnLastVisitedLibraryPath();

    if( !aLibName )
    {
        wxFileDialog dlg( this, FMT_LIBRARY, path,
                          wxEmptyString,
                          wxGetTranslation( LegacyFootprintLibPathWildcard ),
                          wxFD_SAVE );

        if( dlg.ShowModal() == wxID_CANCEL )
            return;

        fileName = dlg.GetPath();
    }

    wxFileName fn( fileName );
    wxGetApp().SaveLastVisitedLibraryPath( fn.GetPath() );
    bool       lib_exists = wxFileExists( fileName );

    if( !aNewModulesOnly && lib_exists )
    {
        wxString msg = wxString::Format( FMT_OK_OVERWRITE, GetChars( fileName ) );

        if( !IsOK( this, msg ) )
            return;
    }

    m_canvas->SetAbortRequest( false );

    try
    {
        PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::LEGACY ) );

        // Delete old library if we're replacing it entirely.
        if( lib_exists && !aNewModulesOnly )
        {
            pi->FootprintLibDelete( fileName );
            lib_exists = false;
        }

        if( !lib_exists )
        {
            pi->FootprintLibCreate( fileName );
        }

        if( !aNewModulesOnly )
        {
            for( MODULE* m = GetBoard()->m_Modules;  m;  m = m->Next() )
            {
                pi->FootprintSave( fileName, m );
            }
        }
        else
        {
            for( MODULE* m = GetBoard()->m_Modules;  m;  m = m->Next() )
            {
                if( !Save_Module_In_Library( fileName, m, false, false ) )
                    break;

                // Check for request to stop backup (ESCAPE key actuated)
                if( m_canvas->GetAbortRequest() )
                    break;
            }
        }
    }
    catch( IO_ERROR ioe )
    {
        DisplayError( this, ioe.errorText );
        return;
    }
}


bool PCB_BASE_FRAME::Save_Module_In_Library( const wxString& aLibPath,
                                             MODULE*         aModule,
                                             bool            aOverwrite,
                                             bool            aDisplayDialog )
{
    if( aModule == NULL )
        return false;

    SetMsgPanel( aModule );

    // Ask what to use as the footprint name in the library
    wxString footprintName = aModule->GetLibRef();

    if( aDisplayDialog )
    {
        wxTextEntryDialog dlg( this, _( "Name:" ), FMT_SAVE_MODULE, footprintName );

        if( dlg.ShowModal() != wxID_OK )
            return false;                   // canceled by user

        footprintName = dlg.GetValue();
        footprintName.Trim( true );
        footprintName.Trim( false );

        if( footprintName.IsEmpty() )
            return false;

        if( ! MODULE::IsLibNameValid( footprintName ) )
        {
            wxString msg;
            msg.Printf( _("Error:\none of invalid chars <%s> found\nin <%s>" ),
                        MODULE::ReturnStringLibNameInvalidChars( true ),
                        GetChars( footprintName ) );

            DisplayError( NULL, msg );
                return false;
        }

        aModule->SetLibRef( footprintName );
    }

    // Ensure this footprint has a libname
    if( footprintName.IsEmpty() )
    {
        footprintName = wxT("noname");
        aModule->SetLibRef( footprintName );
    }

    IO_MGR::PCB_FILE_T  pluginType = IO_MGR::GuessPluginTypeFromLibPath( aLibPath );

    MODULE*  module_exists = NULL;

    try
    {
        PLUGIN::RELEASER pi( IO_MGR::PluginFind( pluginType ) );

        module_exists = pi->FootprintLoad( aLibPath, footprintName );

        if( module_exists )
        {
            delete module_exists;

            // an existing footprint is found in current lib
            if( aDisplayDialog )
            {
                wxString msg = wxString::Format( FMT_MOD_EXISTS,
                    footprintName.GetData(), aLibPath.GetData() );

                SetStatusText( msg );
            }

            if( !aOverwrite )
            {
                // Do not save the given footprint: an old one exists
                return true;
            }
        }

        // this always overwrites any existing footprint, but should yell on its
        // own if the library or footprint is not writable.
        pi->FootprintSave( aLibPath, aModule );
    }
    catch( IO_ERROR ioe )
    {
        DisplayError( this, ioe.errorText );
        return false;
    }

    if( aDisplayDialog )
    {
        wxString fmt = module_exists ?
            _( "Component [%s] replaced in <%s>" ) :
            _( "Component [%s] added in  <%s>" );

        wxString msg = wxString::Format( fmt, footprintName.GetData(), aLibPath.GetData() );
        SetStatusText( msg );
    }

    return true;
}


MODULE* PCB_BASE_FRAME::Create_1_Module( const wxString& aModuleName )
{
    MODULE*  module;
    wxString moduleName;
    wxPoint  newpos;

    moduleName = aModuleName;

    // Ask for the new module reference
    if( moduleName.IsEmpty() )
    {
        wxTextEntryDialog dlg( this, FMT_MOD_REF, FMT_MOD_CREATE, moduleName );

        if( dlg.ShowModal() != wxID_OK )
            return NULL;    //Aborted by user

        moduleName = dlg.GetValue();
    }

    moduleName.Trim( true );
    moduleName.Trim( false );

    if( moduleName.IsEmpty( ) )
    {
        DisplayInfoMessage( this, FMT_NO_REF_ABORTED );
        return NULL;
    }

    // Creates the new module and add it to the head of the linked list of modules
    module = new MODULE( GetBoard() );

    GetBoard()->Add( module );

    // Update parameters: position, timestamp ...
    newpos = GetScreen()->GetCrossHairPosition();
    module->SetPosition( newpos );
    module->SetLastEditTime();

    // Update its name in lib
    module->m_LibRef = moduleName;

    // Update reference:
    module->m_Reference->m_Text = moduleName;
    module->m_Reference->SetThickness( GetDesignSettings().m_ModuleTextWidth );
    module->m_Reference->SetSize( GetDesignSettings().m_ModuleTextSize );

    // Set the value field to a default value
    module->m_Value->m_Text = wxT( "VAL**" );
    module->m_Value->SetThickness( GetDesignSettings().m_ModuleTextWidth );
    module->m_Value->SetSize( GetDesignSettings().m_ModuleTextSize );
    module->SetPosition( wxPoint( 0, 0 ) );

    SetMsgPanel( module );
    return module;
}


void FOOTPRINT_EDIT_FRAME::Select_Active_Library()
{
    if( g_LibraryNames.GetCount() == 0 )
        return;

    EDA_LIST_DIALOG dlg( this, FMT_SELECT_LIB, g_LibraryNames, getLibNickName() );

    if( dlg.ShowModal() != wxID_OK )
        return;

    wxFileName fileName = wxFileName( wxEmptyString, dlg.GetTextSelection(),
                                      LegacyFootprintLibPathExtension );

    fileName = wxGetApp().FindLibraryPath( fileName );

    if( fileName.IsOk() && fileName.FileExists() )
    {
        setLibNickName( fileName.GetName() );
        setLibPath( fileName.GetFullPath() );
    }
    else
    {
        wxString msg = wxString::Format( FMT_BAD_PATHS, GetChars( dlg.GetTextSelection() ) );

        DisplayError( this, msg );

        setLibNickName( wxEmptyString );
        setLibPath( wxEmptyString );
    }

    updateTitle();
}

