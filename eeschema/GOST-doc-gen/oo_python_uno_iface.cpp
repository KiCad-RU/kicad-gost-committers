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
 * @file oo_python_uno_iface.cpp
 */

#include <macros.h>

#include <common_funcs.h>

#include <oo_python_uno_iface.h>

namespace GOST_DOC_GEN {

OO_PYTHON_UNO_IFACE::OO_PYTHON_UNO_IFACE()
{
    // launch the Python interpreter
    Py_Initialize();
}


OO_PYTHON_UNO_IFACE::~OO_PYTHON_UNO_IFACE()
{
    Py_Finalize();
}


bool OO_PYTHON_UNO_IFACE::Connect()
{
    wxString connection_str;

#if defined (__WXMSW__)
    connection_str = wxT( "C:\\Program Files (x86)\\OpenOffice.org 3\\program\\soffice.exe" );
#else
    connection_str = wxT( "soffice" );
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

    // remove filename from the full path
#if defined (__WXMSW__)
    path.Replace( wxT( "\\uno_iface.py" ), wxEmptyString );
#else
    path.Replace( wxT( "/uno_iface.py" ), wxEmptyString );
#endif

    path = wxT( "import sys\nsys.path.append('" ) + path + wxT( "')" );
    if( PyRun_SimpleString( TO_UTF8( path ) ) == -1 )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    PyObject* pyModuleString = PyString_FromString( (char*)"uno_iface" );
    PyObject* pyModule = PyImport_Import( pyModuleString );
    if( !pyModule )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    m_pyUNO_iface_inst = PyObject_GetAttrString( pyModule, (char*)"uno_iface_inst" );

    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyUNO_iface_inst,
                                                     PyString_FromString( (char*)"Connect" ),
                                                     NULL );
    if( !pyResult )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    return true;
}


bool OO_PYTHON_UNO_IFACE::LoadDocument( wxString aUrl )
{
    aUrl = AddUrlPrefix( aUrl );

    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyUNO_iface_inst,
                                                     PyString_FromString( (char*)"LoadDocument" ),
                                                     PyString_FromString( TO_UTF8( aUrl ) ),
                                                     NULL );
    if( !pyResult )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    return true;
}


bool OO_PYTHON_UNO_IFACE::AppendDocument( wxString aUrl )
{
    aUrl = AddUrlPrefix( aUrl );

    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyUNO_iface_inst,
                                                     PyString_FromString( (char*)"AppendDocument" ),
                                                     PyString_FromString( TO_UTF8( aUrl ) ),
                                                     NULL );

    if( !pyResult )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    return true;
}


void OO_PYTHON_UNO_IFACE::SelectTable( int aIndex )
{
    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyUNO_iface_inst,
                                                     PyString_FromString( (char*)"SelectTable" ),
                                                     PyInt_FromLong( aIndex ),
                                                     NULL );

    if( !pyResult )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }
}


void OO_PYTHON_UNO_IFACE::PutCell( wxString aCellAddr,
                                   wxString aStr,
                                   int      aStyle )
{
    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyUNO_iface_inst,
                                                     PyString_FromString( (char*)"PutCell" ),
                                                     PyString_FromString( TO_UTF8( aCellAddr ) ),
                                                     PyString_FromString( TO_UTF8( aStr ) ),
                                                     PyInt_FromLong( aStyle ),
                                                     NULL );

    if( !pyResult )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }
}

} // namespace GOST_DOC_GEN
