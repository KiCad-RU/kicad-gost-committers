/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2012 KiCad Developers, see CHANGELOG.TXT for contributors.
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
 * @file sch_component.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <sch_component.h>

namespace PCAD2KICAD {

SCH_COMPONENT::SCH_COMPONENT()
{
    m_objType   = wxT( "?" );
    m_positionX = 0;
    m_positionY = 0;
    m_rotation  = 0;
    m_mirror    = 0;
    m_partNum   = 0;
    m_width     = 0;
    m_isVisible = 0;
}


SCH_COMPONENT::~SCH_COMPONENT()
{
}

void SCH_COMPONENT::SetPosOffset( int aX_offs, int aY_offs )
{
    m_positionX += aX_offs;
    m_positionY += aY_offs;
}

void SCH_COMPONENT::WriteToFile( wxFile* aFile, char aFileType )
{
}

} // namespace PCAD2KICAD
