/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file dialog_freeroute_exchange.cpp
 * Dialog to access to FreeRoute, the web bases free router, export/import files
 * to/from FreeRoute
 */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <confirm.h>
#include <gestfich.h>
#include <pcbnew.h>
#include <wxPcbStruct.h>
#include <macros.h>

#include <../common/dialogs/dialog_display_info_HTML_base.h>

#include <dialog_freeroute_exchange.h>

#ifdef __WINDOWS__
#include <wx/msw/registry.h>
#endif


#define FREEROUTE_URL_KEY wxT( "freeroute_url" )
#define FREEROUTE_RUN_KEY wxT( "freeroute_command" )


void PCB_EDIT_FRAME::Access_to_External_Tool( wxCommandEvent& event )
{
    DIALOG_FREEROUTE dialog( this );
    dialog.ShowModal();
}



DIALOG_FREEROUTE::DIALOG_FREEROUTE( PCB_EDIT_FRAME* parent ):
    DIALOG_FREEROUTE_BASE( parent )
{
    m_Parent = parent;
    MyInit();

    m_sdbSizer1OK->SetDefault();
    GetSizer()->SetSizeHints( this );
    Centre();
}



/* Specific data initialization
 */

void DIALOG_FREEROUTE::MyInit()
{
    SetFocus();
    m_FreeRouteSetupChanged = false;

    wxString msg;
    wxGetApp().GetSettings()->Read( FREEROUTE_URL_KEY, &msg );

    if( msg.IsEmpty() )
        m_FreerouteURLName->SetValue( wxT( "http://www.freerouting.net/" ) );
    else
        m_FreerouteURLName->SetValue( msg );
}

const char * s_FreeRouteHelpInfo =
#include <dialog_freeroute_exchange_help_html.h>
;
void DIALOG_FREEROUTE::OnHelpButtonClick( wxCommandEvent& event )
{
    DIALOG_DISPLAY_HTML_TEXT_BASE help_Dlg( this, wxID_ANY,
        _("Freeroute Help"),wxDefaultPosition, wxSize( 650,550 ) );

    wxString msg = FROM_UTF8(s_FreeRouteHelpInfo);
    help_Dlg.m_htmlWindow->AppendToPage( msg );
    help_Dlg.ShowModal();
}

/*  wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CREATE_EXPORT_DSN_FILE
 */
void DIALOG_FREEROUTE::OnExportButtonClick( wxCommandEvent& event )
{
    m_Parent->ExportToSpecctra( event );
}


/*  wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT_FREEROUTE_DSN_FILE
 */
void DIALOG_FREEROUTE::OnImportButtonClick( wxCommandEvent& event )
{
    m_Parent->ImportSpecctraSession(  event );

    /* Connectivity inf must be rebuild.
     * because for large board it can take some time, this is made only on demand
     */
    if( IsOK( this, _("Do you want to rebuild connectivity data ?" ) ) )
        m_Parent->Compile_Ratsnest( NULL, true );
}


/* wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RUN_FREEROUTE
 */
void DIALOG_FREEROUTE::OnLaunchButtonClick( wxCommandEvent& event )
{
    wxString url;
    wxString command;
    wxFileName fileName( FindKicadFile( wxT( "freeroute.jnlp" ) ), wxPATH_UNIX );

    if( fileName.FileExists() )
    {
        wxString javaWebStartCommand = wxT( "javaws" );

        // Find the Java web start application on Windows.
#ifdef __WINDOWS__
        // If you thought the registry was brain dead before, now you have to deal with
        // accessing it in either 64 or 32 bit mode depending on the build version of
        // Windows and the build version of KiCad.

        // This key works for 32 bit Java on 32 bit Windows and 64 bit Java on 64 bit Windows.
        wxRegKey key( wxRegKey::HKLM, wxT( "SOFTWARE\\JavaSoft\\Java Web Start" ),
                      wxIsPlatform64Bit() ? wxRegKey::WOW64ViewMode_64 :
                      wxRegKey::WOW64ViewMode_Default );

        // It's possible that 32 bit Java is installed on 64 bit Windows.
        if( !key.Exists() && wxIsPlatform64Bit() )
            key.SetName( wxRegKey::HKLM, wxT( "SOFTWARE\\Wow6432Node\\JavaSoft\\Java Web Start" ) );

        if( !key.Exists() )
        {
            ::wxMessageBox( _( "It appears that the Java run time environment is not "
                               "installed on this computer.  Java is required to use "
                               "FreeRoute." ),
                            _( "Pcbnew Error" ), wxOK | wxICON_ERROR );
            return;
        }

        key.Open( wxRegKey::Read );

        // Get the current version of java installed to determine the executable path.
        wxString value;
        key.QueryValue( wxT( "CurrentVersion" ), value );
        key.SetName( key.GetName() + wxT( "\\" ) + value );
        key.QueryValue( wxT( "Home" ), value );
        javaWebStartCommand = value + wxFileName::GetPathSeparator() + javaWebStartCommand;
#endif

        // Wrap FullFileName in double quotes in case it has C:\Program Files in it.
        // The space is interpreted as an argument separator.
        command << javaWebStartCommand << wxChar( ' ' ) << wxChar( '"' )
                << fileName.GetFullPath() << wxChar( '"' );
        ProcessExecute( command );
        return;
    }

    url = m_FreerouteURLName->GetValue() + wxT( "/java/freeroute.jnlp" );

    wxLaunchDefaultBrowser( url );
}


/* wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */
void DIALOG_FREEROUTE::OnVisitButtonClick( wxCommandEvent& event )
{
    wxString command = m_FreerouteURLName->GetValue();

    wxLaunchDefaultBrowser( command );
}


/*  wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */
void DIALOG_FREEROUTE::OnCancelButtonClick( wxCommandEvent& event )
{
    EndModal(wxID_CANCEL);
}


void DIALOG_FREEROUTE::OnOKButtonClick( wxCommandEvent& event )
{
    if( m_FreeRouteSetupChanged )  // Save new config
    {
        wxGetApp().GetSettings()->Write( FREEROUTE_URL_KEY,
                                         m_FreerouteURLName->GetValue() );
    }

    EndModal(wxID_OK);
}


/* wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXT_EDIT_FR_URL
 */
void DIALOG_FREEROUTE::OnTextEditFrUrlUpdated( wxCommandEvent& event )
{
    m_FreeRouteSetupChanged = true;
}
