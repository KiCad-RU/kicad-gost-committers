/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2013 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file component.h
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <wx/wx.h>

namespace GOST_DOC_GEN {

#define COMP_IN_CONST_PART      0
#define COMP_IN_VAR_PART        1
#define COMP_IN_VAR_PART_DIFF   2

enum ATTR_T
{
    ATTR_NAME,
    ATTR_TYPE,
    ATTR_TYPE1,
    ATTR_SUBTYPE,
    ATTR_VALUE,
    ATTR_PRECISION,
    ATTR_NOTE,
    ATTR_DESIGNATION,
    ATTR_MANUFACTURER,
    ATTR_QTY
};

typedef struct _TCOMPONENT_ATTRS
{
    int         variant;
    wxString    attrs[ATTR_QTY];
} TCOMPONENT_ATTRS, * pTCOMPONENT_ATTRS;

struct TKICAD_ATTR
{
    wxString    value_of_attr;
    bool        attr_changed;
};

WX_DEFINE_ARRAY( TCOMPONENT_ATTRS *, TCOMPONENT_ATTRS_ARRAY );

class COMPONENT : public wxObject
{
public:
    COMPONENT();
    virtual ~COMPONENT();

    wxString                m_RefDes;
    wxString                m_SortingRefDes;
    int                     m_Variants_State;
    bool                    ExcludeFromCompIndex;
    wxString                ComponentType;
    wxString                fmt;

    TCOMPONENT_ATTRS_ARRAY  m_comp_attr_variants;

    TKICAD_ATTR             m_KiCadAttrs[ATTR_QTY];

    virtual void    Update();
    virtual bool    ReadVariants();
    virtual int     WriteVariants();
    virtual int     GetVariant( int aVariant, TCOMPONENT_ATTRS* aVariant_attrs );

protected:
    virtual int     ReadVariantsInStr( wxString aStr, int aComp_attr, bool aInit );
    virtual void    FormKiCadAttribute( wxString aVariant_str, wxString aValue, wxString* aDest_string );
};

} // namespace GOST_DOC_GEN

#endif    // COMPONENT_H
