/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2013 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file rpc_doc_iface.cpp
 */

#include <macros.h>

#include <common_funcs.h>

#include <rpc_doc_iface.h>

namespace GOST_DOC_GEN {


RPC_DOC_IFACE::RPC_DOC_IFACE()
{
    // put a string terminator
    m_buffer[RECV_MSG_SIZE] = 0;

    m_sock = new wxSocketClient();
}


RPC_DOC_IFACE::~RPC_DOC_IFACE()
{
    m_sock->Destroy();
}


bool RPC_DOC_IFACE::Connect()
{
    wxString connection_str;
    wxString pythonExecutable;

#if defined (__WXMSW__)
    wxString windowsOOInstallationPath = FindWindowsOOInstallationPath();
    connection_str = windowsOOInstallationPath + wxT( "soffice.exe" );
    pythonExecutable = windowsOOInstallationPath + wxT( "Python.exe" );
    if ( wxFileExists( pythonExecutable ) )
    {
        wxMessageBox( wxT( "Unable to find OpenOffice or LibreOffice embedded Python" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }
#else
    connection_str = wxT( "soffice" );
    pythonExecutable = wxT( "python" );
#endif

#if defined(__WXMSW__)
    connection_str += wxT(
         " -accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
                         );

    if( !wxExecute( connection_str, wxEXEC_ASYNC ) )
        // for some reason this check does not work in Linux, however it works in Windows
        wxMessageBox( wxT( "Unable to launch the process: " ) + connection_str
                      + wxT( ".\nPlease make sure that OpenOffice / LibreOffice is installed." ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        // continue anyway in order to allow a user to run soffice from command shell
#else
    // TODO: determine OpenOffice / LibreOffice version at runtime
    // old OpenOffice command line format
    connection_str += wxT(
         " -invisible -accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
                         );
    wxExecute( connection_str, wxEXEC_ASYNC );

    // new LibreOffice command line format
    connection_str += wxT(
        " --invisible --accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
                         );
    wxExecute( connection_str, wxEXEC_ASYNC );
#endif

    wxString path = GetResourceFile( wxT( "uno_iface.py" ) );
    if( path == wxEmptyString )
        return false;


    wxExecute( pythonExecutable + wxT( " " ) + path, wxEXEC_ASYNC );

    wxIPV4address addr;
    addr.Hostname( HOSTNAME );
    addr.Service( RPC_DOC_IFACE_PORT_SERVICE_NUMBER );

    bool connected = false;
    for( int retry = 0; retry < 10; retry++ )
    {
        m_sock->Connect( addr );

        if( m_sock->Ok() && m_sock->IsConnected() )
        {
            connected = true;
            break;
        }

        wxMilliSleep( 500 );
    }

    if( !connected )
    {
        wxMessageBox( wxT( "RPC_DOC_IFACE: Unable to connect to RPC document server" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    m_sock->SetFlags( wxSOCKET_WAITALL );

    wxString connect( wxT( "Connect" ) );

    m_sock->Write( TO_UTF8( connect ), strlen( TO_UTF8( connect ) ) );
    // wait for command completion
    m_sock->Read( m_buffer, RECV_MSG_SIZE );

    if( strcmp( m_buffer, "OK______" ) )
    {
        wxMessageBox( wxT( "RPC_DOC_IFACE: Unable to connect to office" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );

        Disconnect();

        return false;
    }

    return true;
}


void RPC_DOC_IFACE::Disconnect()
{
    const char* EXIT = "Exit";

    m_sock->Write( EXIT, strlen( EXIT ) );
    // wait for command completion
    m_sock->Read( m_buffer, RECV_MSG_SIZE );

    if( strcmp( m_buffer, "BYE_____" ) )
    {
        wxMessageBox( wxT( "RPC_DOC_IFACE: Unable to disconnect from RPC document server" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }

    m_sock->Close();
}


bool RPC_DOC_IFACE::LoadDocument( wxString aUrl )
{
    aUrl = AddUrlPrefix( aUrl );

    wxString loadDocument( wxT( "LoadDocument {" ) + aUrl + wxT( "}" ) );

    m_sock->Write( TO_UTF8( loadDocument ), strlen( TO_UTF8( loadDocument ) ) );
    // wait for command completion
    m_sock->Read( m_buffer, RECV_MSG_SIZE );

    if( strcmp( m_buffer, "OK______" ) )
    {
        wxMessageBox( wxT( "RPC_DOC_IFACE: Unable to load document" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    return true;
}


bool RPC_DOC_IFACE::AppendDocument( wxString aUrl )
{
    aUrl = AddUrlPrefix( aUrl );

    wxString appendDocument( wxT( "AppendDocument {" ) + aUrl + wxT( "}" ) );

    m_sock->Write( TO_UTF8( appendDocument ), strlen( TO_UTF8( appendDocument ) ) );
    // wait for command completion
    m_sock->Read( m_buffer, RECV_MSG_SIZE );

    if( strcmp( m_buffer, "OK______" ) )
    {
        wxMessageBox( wxT( "RPC_DOC_IFACE: Unable to append document" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    return true;
}


void RPC_DOC_IFACE::SelectTable( int aIndex )
{
    wxString selectTable = wxString::Format( wxT( "SelectTable {%d}" ), aIndex );

    m_sock->Write( TO_UTF8( selectTable ), strlen( TO_UTF8( selectTable ) ) );
    // wait for command completion
    m_sock->Read( m_buffer, RECV_MSG_SIZE );

    if( strcmp( m_buffer, "OK______" ) )
    {
        wxMessageBox( wxT( "RPC_DOC_IFACE: Unable to select table" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }
}


void RPC_DOC_IFACE::PutCell( wxString aCellAddr,
                             wxString aStr,
                             int      aStyle )
{
    wxString putCell = wxString::Format( wxT( "PutCell {%s} {%s} {%d}" ),
                                         aCellAddr, aStr, aStyle );

    m_sock->Write( TO_UTF8( putCell ), strlen( TO_UTF8( putCell ) ) );
    // wait for command completion
    m_sock->Read( m_buffer, RECV_MSG_SIZE );

    if( strcmp( m_buffer, "OK______" ) )
    {
        wxMessageBox( wxT( "RPC_DOC_IFACE: Unable to put cell" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }
}

} // namespace GOST_DOC_GEN
