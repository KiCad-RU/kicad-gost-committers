/* This program source code file is part of KiCad, a free EDA CAD application.
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
 * @file oo_python_uno_iface.h
 */

#ifndef _OO_PYTHON_UNO_IFACE_H_
#define _OO_PYTHON_UNO_IFACE_H_

#include <python_scripting.h>

#include <common_doc_iface.h>

namespace GOST_DOC_GEN {

class OO_PYTHON_UNO_IFACE : public COMMON_DOC_IFACE
{
public:
    OO_PYTHON_UNO_IFACE();
    ~OO_PYTHON_UNO_IFACE();

    bool Connect();
    bool LoadDocument( wxString aUrl );
    bool AppendDocument( wxString aUrl );
    void SelectTable( int aIndex );
    void PutCell( wxString aCellAddr,
                  wxString aStr,
                  int      aStyle );

private:
    PyObject* m_pyUNO_iface_inst;
};

} // namespace GOST_DOC_GEN

#endif    // _OO_PYTHON_UNO_IFACE_H_
