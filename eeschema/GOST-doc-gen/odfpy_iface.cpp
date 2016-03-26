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
 * @file odfpy_iface.cpp
 */

#include <macros.h>

#include <common_funcs.h>

#include <odfpy_iface.h>

namespace GOST_DOC_GEN {

ODFPY_IFACE::ODFPY_IFACE()
{
    // launch the Python interpreter
    Py_Initialize();
}


ODFPY_IFACE::~ODFPY_IFACE()
{
    Py_Finalize();
}


bool ODFPY_IFACE::Connect()
{
    wxString path = GetResourceFile( wxT( "odfpy_iface.py" ) );

    if( path == wxEmptyString )
        return false;

    // remove filename from the full path
#if defined (__WXMSW__)
    path.Replace( wxT( "\\odfpy_iface.py" ), wxEmptyString );
#else
    path.Replace( wxT( "/odfpy_iface.py" ), wxEmptyString );
#endif

    path = wxT( "import sys\nsys.path.append('" ) + path + wxT( "')" );

    if( PyRun_SimpleString( TO_UTF8( path ) ) == -1 )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    PyObject* pyModuleString = PyString_FromString( (char*)"odfpy_iface" );
    PyObject* pyModule = PyImport_Import( pyModuleString );

    if( !pyModule )
    {
        wxMessageBox( PyErrStringWithTraceback(),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        return false;
    }

    m_pyOdfpy_iface_inst = PyObject_GetAttrString( pyModule, (char*)"odfpy_iface_inst" );

    return true;
}


void ODFPY_IFACE::Disconnect()
{
}


bool ODFPY_IFACE::LoadDocument( wxString aUrl )
{
    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyOdfpy_iface_inst,
                                                     PyString_FromString( (char*)"LoadDocument" ),
                                                     PyString_FromString( aUrl ),
                                                     NULL );

    return ( pyResult != NULL );
}


bool ODFPY_IFACE::AppendDocument( wxString aUrl )
{
    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyOdfpy_iface_inst,
                                                     PyString_FromString( (char*)"AppendDocument" ),
                                                     PyString_FromString( aUrl ),
                                                     NULL );

    return ( pyResult != NULL );
}


void ODFPY_IFACE::SelectTable( int aIndex )
{
    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyOdfpy_iface_inst,
                                                     PyString_FromString( (char*)"SelectTable" ),
                                                     PyInt_FromLong( aIndex ),
                                                     NULL );

    if( pyResult == NULL )
    {
        wxMessageBox( wxT( "ODFPY_IFACE: Unable to select table" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }
}


void ODFPY_IFACE::PutCell( wxString aCellAddr,
                           wxString aStr,
                           int      aStyle )
{
    PyObject* pyResult = PyObject_CallMethodObjArgs( m_pyOdfpy_iface_inst,
                                                     PyString_FromString( (char*)"PutCell" ),
                                                     PyString_FromString( aCellAddr ),
                                                     PyString_FromString( aStr ),
                                                     PyInt_FromLong( aStyle ),
                                                     NULL );

    if( pyResult == NULL )
    {
        wxMessageBox( wxT( "ODFPY_IFACE: Unable to put cell" ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }
    Py_Finalize();

}

} // namespace GOST_DOC_GEN
