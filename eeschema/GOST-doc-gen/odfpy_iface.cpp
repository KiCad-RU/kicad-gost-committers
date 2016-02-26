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

#include <odfpy_iface.h>

namespace GOST_DOC_GEN {

ODFPY_IFACE::ODFPY_IFACE()
{
    // launch the Python interpreter
    Py_Initialize();

    PyRun_SimpleString( "import sys\n"
                        "sys.path.append('/home/a-lunev/git/bzr/GOST-doc-gen/Debug/eeschema')\n"
                      );

    PyObject* pyModuleString = PyString_FromString( (char*)"odfpy_iface" );
    PyObject* pyModule = PyImport_Import( pyModuleString );
    m_pyOdfpy_iface_inst = PyObject_GetAttrString( pyModule, (char*)"odfpy_iface_inst" );
}


ODFPY_IFACE::~ODFPY_IFACE()
{
    Py_Finalize();
}


bool ODFPY_IFACE::Connect()
{
    return true;
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
