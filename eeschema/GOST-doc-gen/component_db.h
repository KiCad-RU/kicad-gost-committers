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
 * @file component_db.h
 */

#ifndef COMPONENTSDB_H
#define COMPONENTSDB_H

#include <wx/wx.h>
#include <wx/dynarray.h>
#include <common_funcs.h>

#include <component.h>
#include <netlist.h>

namespace GOST_DOC_GEN {

#define ZEROS_LENGTH_INS 5

// part_type of ExtractPartOfComponentsDB()
#define PARTTYPE_VAR                0x001
#define PARTTYPE_A_SET              0x002
#define PARTTYPE_SPECIFICATION      0x004
#define PARTTYPE_DOCUMENTATION      0x008
#define PARTTYPE_STANDARD_DETAILS   0x010
#define PARTTYPE_DETAILS            0x020
#define PARTTYPE_ASSEMBLY_UNITS     0x040
#define PARTTYPE_GOODS              0x080
#define PARTTYPE_ALL_DETAILS        0x100

WX_DEFINE_ARRAY( COMPONENT *, COMPONENT_ARRAY );
WX_DEFINE_ARRAY( COMPONENT_ARRAY *, COMPONENT_2D_ARRAY );
WX_DEFINE_ARRAY( wxArrayString *, STRING_2D_ARRAY );

class COMPONENT_DB
{
// Construction
public:
    COMPONENT_DB();
    COMPONENT_ARRAY m_AllComponents;

    wxString        m_designName;
    wxString        m_designation;
    wxString        m_specFirstUse;
    wxString        m_PCBDesignation;
    wxString        m_circuitDrawingFmt;
    wxString        m_assemblyDrawingFmt;
    wxString        m_developerField;
    wxString        m_verifierField;
    wxString        m_approverField;

    INT_ARRAY       m_variantIndexes;

    virtual         ~COMPONENT_DB();
    virtual void    LoadFromKiCad();
    virtual void    SortComponents();
    virtual void    AddNewVariant( int aVariant );
    virtual void    FormRefDes( COMPONENT_ARRAY*    aTitle_group_components,
                                int                 aStart_item,
                                int                 aEnd_item,
                                wxString*           aResult );
    virtual int     FindSetType( STRING_2D_ARRAY* aAll_set_types_array, wxString aIn_setrefdes );
    virtual void    FindSets( COMPONENT_ARRAY*  aComponents,
                              STRING_2D_ARRAY*  aResult,
                              int               aPart_type,
                              int               aVariant,
                              bool              aAppend );
    virtual void    ExtractPartOfComponentsDB( COMPONENT_ARRAY* aResult,
                                               int              aPart_type,
                                               int              aVariant,
                                               wxString         aSet_prefix );
    virtual void    GenerateComponentIndexDoc();
    virtual void    GenerateSpecificationDoc();

protected:
    virtual void    ZeroInserting( wxString* aStr );
    virtual bool    FindVariant( int aVariant );
    virtual void    ReadVariants();

private:
    SCH_REFERENCE_LIST m_cmplist;              // a flat list of components in the full hierarchy
};

} // namespace GOST_DOC_GEN

#endif    // COMPONENTSDB_H
