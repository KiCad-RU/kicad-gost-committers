/**
 * @file ^pcbnew/dialogs/dialog_pcbnew_config_libs_and_paths.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2012 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2004-2012 KiCad Developers, see change_log.txt for contributors.
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



/* Handle the Pcbnew library config (library list, and default lib path)
 */

#include <fctsys.h>
#include <wx/tokenzr.h>
#include <appl_wxstruct.h>
#include <confirm.h>
#include <gestfich.h>
#include <pcbnew.h>
#include <macros.h>
#include <wxPcbStruct.h>
#include <pcbcommon.h>
#include <wildcards_and_files_ext.h>

#include <dialog_pcbnew_config_libs_and_paths.h>
#include <wildcards_and_files_ext.h>


void PCB_EDIT_FRAME::InstallConfigFrame( )
{
    DIALOG_PCBNEW_CONFIG_LIBS dialog( this );
    dialog.ShowModal();
}


DIALOG_PCBNEW_CONFIG_LIBS::DIALOG_PCBNEW_CONFIG_LIBS( PCB_EDIT_FRAME* parent ):
    DIALOG_PCBNEW_CONFIG_LIBS_FBP(parent)
{
    m_Config = wxGetApp().GetCommonSettings();

    Init( );

    wxString title = _( "from " ) + wxGetApp().GetCurrentOptionFile();
    SetTitle( title );

    m_sdbSizer1OK->SetDefault();

    if( GetSizer() )
        GetSizer()->SetSizeHints( this );
}


void DIALOG_PCBNEW_CONFIG_LIBS::Init()
{
    SetFocus();

    m_LibListChanged = false;
    m_LibPathChanged = false;
    m_UserLibDirBufferImg = g_UserLibDirBuffer;  // Save the original lib path

    m_ListLibr->InsertItems( g_LibraryNames, 0 );

    // Display current modules doc file:
    m_Config->Read( wxT( "module_doc_file" ), g_DocModulesFileName );
    m_TextHelpModulesFileName->SetValue( g_DocModulesFileName );

    // Load user libs paths:
    wxStringTokenizer Token( m_UserLibDirBufferImg, wxT( ";\n\r" ) );

    while( Token.HasMoreTokens() )
    {
        wxString path = Token.GetNextToken();

        if( wxFileName::DirExists( path ) )
            m_listUserPaths->Append(path);
    }

    // Display actual libraries paths:
    wxPathList libpaths = wxGetApp().GetLibraryPathList();
    for( unsigned ii = 0; ii < libpaths.GetCount(); ii++ )
    {
        m_DefaultLibraryPathslistBox->Append( libpaths[ii]);
    }

    // select the first path after the current path project
    if ( libpaths.GetCount() > 1 )
        m_DefaultLibraryPathslistBox->Select( 1 );
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnCancelClick( wxCommandEvent& event )
{
    // Recreate the user lib path
    if ( m_LibPathChanged )
    {
        for ( unsigned ii = 0; ii < m_ListLibr->GetCount(); ii++ )
            wxGetApp().RemoveLibraryPath( m_listUserPaths->GetString(ii) );

        wxGetApp().InsertLibraryPath( g_UserLibDirBuffer, 1);
    }

    EndModal( wxID_CANCEL );
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnOkClick( wxCommandEvent& event )
{
     m_Config->Write( wxT( "module_doc_file" ), m_TextHelpModulesFileName->GetValue() );

    // Recreate the user lib path
    if ( m_LibPathChanged )
    {
        g_UserLibDirBuffer.Empty();

        for ( unsigned ii = 0; ii < m_listUserPaths->GetCount(); ii ++ )
        {
            if ( ii > 0 )
                g_UserLibDirBuffer << wxT(";");

            g_UserLibDirBuffer << m_listUserPaths->GetString(ii);
        }
    }

    // Set new active library list if the lib list of if default path list was modified
    if( m_LibListChanged || m_LibPathChanged )
    {
        // Recreate lib list
        g_LibraryNames.Clear();

        for ( unsigned ii = 0; ii < m_ListLibr->GetCount(); ii ++ )
            g_LibraryNames.Add( m_ListLibr->GetString(ii) );
    }

    GetParent()->SaveProjectSettings();
    EndModal( wxID_OK );
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnCloseWindow( wxCloseEvent& event )
{
    EndModal( wxID_CANCEL );
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnButtonUpClick( wxCommandEvent& event )
{
    wxArrayInt selections;

    m_ListLibr->GetSelections(selections);
    if ( selections.GetCount() <= 0 )   // No selection.
        return;

    if( selections[0] == 0 )            // The first lib is selected. cannot move up it
        return;

    wxArrayString libnames = m_ListLibr->GetStrings();

    for( size_t ii = 0; ii < selections.GetCount(); ii++ )
    {
        int jj = selections[ii];
        EXCHG( libnames[jj],  libnames[jj-1]);
    }

    m_ListLibr->Set(libnames);

    // Reselect previously selected names
    for( size_t ii = 0; ii < selections.GetCount(); ii++ )
    {
        int jj = selections[ii];
        m_ListLibr->SetSelection(jj-1);
    }

    m_LibListChanged = true;
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnButtonDownClick( wxCommandEvent& event )
{
    wxArrayInt selections;

    m_ListLibr->GetSelections(selections);

    if ( selections.GetCount() <= 0 )   // No selection.
        return;

    // The last lib is selected. cannot move down it
    if( selections.Last() == (int)(m_ListLibr->GetCount()-1) )
        return;

    wxArrayString libnames = m_ListLibr->GetStrings();

    for( int ii = selections.GetCount()-1; ii >= 0; ii-- )
    {
        int jj = selections[ii];
        EXCHG( libnames[jj], libnames[jj+1] );
    }

    m_ListLibr->Set(libnames);

    // Reselect previously selected names
    for( size_t ii = 0; ii < selections.GetCount(); ii++ )
    {
        int jj = selections[ii];
        m_ListLibr->SetSelection(jj+1);
    }

    m_LibListChanged = true;
}


/* Remove a library to the library list.
 * The real list (g_LibName_List) is not changed, so the change can be canceled
 */
void DIALOG_PCBNEW_CONFIG_LIBS::OnRemoveLibClick( wxCommandEvent& event )
{
    wxArrayInt selections;

    m_ListLibr->GetSelections( selections );

    for( int ii = selections.GetCount()-1; ii >= 0; ii-- )
    {
        m_ListLibr->Delete( selections[ii] );
        m_LibListChanged = true;
    }
}


/* Insert or add a library to the library list:
 *   The new library is put in list before (insert button) the selection,
 *   or added (add button) to end of list
 * The real list (g_LibName_List) is not changed, so the change can be canceled
 */
void DIALOG_PCBNEW_CONFIG_LIBS::OnAddOrInsertLibClick( wxCommandEvent& event )
{
    int        ii = 0;
    wxString   libfilename;
    wxFileName fn;

    wxArrayInt selections;
    m_ListLibr->GetSelections(selections);

    ii = selections.GetCount();

    if( ii > 0 )
        ii = selections[0];
    else
        ii = 0;

    wxString libpath;
    libpath = m_DefaultLibraryPathslistBox->GetStringSelection();
    if ( libpath.IsEmpty() )
        libpath = wxGetApp().ReturnLastVisitedLibraryPath();

    wxFileDialog FilesDialog( this, _( "Footprint library files:" ), libpath,
                              wxEmptyString,
                              wxGetTranslation( LegacyFootprintLibPathWildcard ),
                              wxFD_DEFAULT_STYLE | wxFD_MULTIPLE );

    if( FilesDialog.ShowModal() != wxID_OK )
        return;

    wxArrayString Filenames;
    FilesDialog.GetPaths( Filenames );

    for( unsigned jj = 0; jj < Filenames.GetCount(); jj++ )
    {
        fn = Filenames[jj];

        if ( jj == 0 )
            wxGetApp().SaveLastVisitedLibraryPath( fn.GetPath() );

        /* If the library path is already in the library search paths
         * list, just add the library name to the list.  Otherwise, add
         * the library name with the full or relative path.
         * the relative path, when possible is preferable,
         * because it preserve use of default libraries paths, when the
         * path is a sub path of these default paths
         */
        libfilename = wxGetApp().ReturnFilenameWithRelativePathInLibPath( fn.GetFullPath() );

        // Remove extension:
        fn = libfilename;
        fn.SetExt( wxEmptyString );
        libfilename = fn.GetFullPath();

        //Add or insert new library name, if not already in list
        if( m_ListLibr->FindString( libfilename, fn.IsCaseSensitive() ) == wxNOT_FOUND )
        {
            m_LibListChanged = true;

            if( event.GetId() == ID_ADD_LIB )
                m_ListLibr->Append( libfilename );
            else
                m_ListLibr->Insert( libfilename, ii++ );
        }
        else
        {
            wxString msg = wxT( "<" ) + libfilename + wxT( "> : " ) +
                           _( "Library already in use" );
            DisplayError( this, msg );
        }
    }
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnAddOrInsertPath( wxCommandEvent& event )
{
    wxString path = wxGetApp().ReturnLastVisitedLibraryPath();

    bool     select = EDA_DirectorySelector( _( "Default Path for Libraries" ),
                                             path, wxDD_DEFAULT_STYLE,
                                             this, wxDefaultPosition );

    if( !select )
        return;

    if( ! wxFileName::DirExists( path ) )    // Should not occurs
        return;

    // Add or insert path if not already in list
    if( m_listUserPaths->FindString( path ) == wxNOT_FOUND )
    {
        int ipos = m_listUserPaths->GetCount();

        if ( event.GetId() == wxID_INSERT_PATH )
        {
            if ( ipos  )
                ipos--;

            int jj = m_listUserPaths->GetSelection();

            if ( jj >= 0 )
                ipos = jj;
        }

        // Ask the user if this is a relative path
        int diag = wxMessageBox( _( "Use a relative path?" ),
                                 _( "Path type" ),
                                 wxYES_NO | wxICON_QUESTION, this );

        if( diag == wxYES )
        {   // Make it relative
            wxFileName fn = path;
            fn.MakeRelativeTo( wxT( "." ) );
            path = fn.GetPathWithSep() + fn.GetFullName();
        }

        m_listUserPaths->Insert(path, ipos);
        m_LibPathChanged = true;
        wxGetApp().InsertLibraryPath( path, ipos+1 );

        // Display actual libraries paths:
        wxPathList libpaths = wxGetApp().GetLibraryPathList();
        m_DefaultLibraryPathslistBox->Clear();

        for( unsigned ii = 0; ii < libpaths.GetCount(); ii++ )
        {
            m_DefaultLibraryPathslistBox->Append( libpaths[ii]);
        }
    }
    else
    {
        DisplayError( this, _( "Path already in use" ) );
    }

    wxGetApp().SaveLastVisitedLibraryPath( path );
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnRemoveUserPath( wxCommandEvent& event )
{
    int ii = m_listUserPaths->GetSelection();

    if ( ii < 0 )
        ii = m_listUserPaths->GetCount()-1;

    if ( ii >= 0 )
    {
        wxGetApp().RemoveLibraryPath( m_listUserPaths->GetStringSelection() );
        m_listUserPaths->Delete( ii );
        m_LibPathChanged = true;
    }

    // Display actual libraries paths:
    wxPathList libpaths = wxGetApp().GetLibraryPathList();
    m_DefaultLibraryPathslistBox->Clear();

    for( unsigned ii = 0; ii < libpaths.GetCount(); ii++ )
    {
        m_DefaultLibraryPathslistBox->Append( libpaths[ii] );
    }
}


void DIALOG_PCBNEW_CONFIG_LIBS::OnBrowseModDocFile( wxCommandEvent& event )
{
    wxString FullFileName;
    wxString docpath, filename;

    docpath = wxGetApp().ReturnLastVisitedLibraryPath(wxT( "doc" ));

    wxFileDialog FilesDialog( this, _( "Footprint document file:" ), docpath,
                              wxEmptyString, PdfFileWildcard, wxFD_DEFAULT_STYLE  );

    if( FilesDialog.ShowModal() != wxID_OK )
        return;

    FullFileName = FilesDialog.GetPath( );
    /* If the path is already in the library search paths
     * list, just add the library name to the list.  Otherwise, add
     * the library name with the full or relative path.
     * the relative path, when possible is preferable,
     * because it preserve use of default libraries paths, when the path is
     * a sub path of these default paths
     */
    wxFileName fn = FullFileName;
    wxGetApp().SaveLastVisitedLibraryPath( fn.GetPath() );

    filename = wxGetApp().ReturnFilenameWithRelativePathInLibPath(FullFileName);
    m_TextHelpModulesFileName->SetValue( filename );
}
