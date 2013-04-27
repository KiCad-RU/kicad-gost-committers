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
 * @file GOST_comp_lister.cpp
 */

#include <GOST_comp_lister.h>
#include <component_db.h>

using namespace GOST_DOC_GEN;

GOST_COMP_LISTER::GOST_COMP_LISTER()
{
    m_componentDB = new COMPONENT_DB();
}


GOST_COMP_LISTER::~GOST_COMP_LISTER()
{
    delete m_componentDB;
}


void GOST_COMP_LISTER::GetList()
{
    m_componentDB->LoadFromKiCad();

    wxExecute( wxT(
        "soffice -invisible -accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
                  ), wxEXEC_ASYNC );

    m_componentDB->OO_GenerateComponentIndex();
    m_componentDB->OO_GenerateSpecification();
}