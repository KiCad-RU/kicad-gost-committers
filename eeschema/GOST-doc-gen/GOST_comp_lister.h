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
 * @file GOST_comp_lister.h
 */

#ifndef _GOST_COMP_LISTER_H_
#define _GOST_COMP_LISTER_H_

#include <netlist.h>

namespace GOST_DOC_GEN {
    class COMPONENT_DB;
}

class GOST_COMP_LISTER
{
private:
    GOST_DOC_GEN::COMPONENT_DB* m_componentDB;

public:
    GOST_COMP_LISTER();
    ~GOST_COMP_LISTER();

    void GetList();
};

#endif    // _GOST_COMP_LISTER_H_
