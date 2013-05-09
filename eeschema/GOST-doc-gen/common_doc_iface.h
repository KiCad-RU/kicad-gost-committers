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
 * @file common_doc_iface.h
 */

#ifndef _COMMON_DOC_IFACE_H_
#define _COMMON_DOC_IFACE_H_

#include <wx/wx.h>

namespace GOST_DOC_GEN {

#define TEXT_UNDERLINED    0x0001
#define TEXT_CENTERED      0x0002

class COMMON_DOC_IFACE
{
public:
    virtual ~COMMON_DOC_IFACE() {}

    virtual bool Connect() = 0;
    virtual void Disconnect() = 0;
    virtual bool LoadDocument( wxString aUrl ) = 0;
    virtual bool AppendDocument( wxString aUrl ) = 0;
    virtual void SelectTable( int aIndex ) = 0;
    virtual void PutCell( wxString aCellAddr,
                          wxString aStr,
                          int      aStyle ) = 0;
};

} // namespace GOST_DOC_GEN

#endif    // _COMMON_DOC_IFACE_H_
