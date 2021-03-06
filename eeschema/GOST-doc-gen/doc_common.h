/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2013 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2013-2014 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file doc_common.h
 */

#ifndef DOC_COMMON_H
#define DOC_COMMON_H

#include <wx/wx.h>
#include <component_db.h>

#ifdef USE_PYTHON_GOSTDOCGEN
#include <python_scripting.h>
#endif

namespace GOST_DOC_GEN {

#define FIRST_SHEET_LAST_STR_I      26
#define SECOND_SHEET_LAST_STR_I     29

#define WORDFORM_PLURAL             1
#define WORDFORM_SINGULAR_GENITIVE  2
#define WORDFORM_PLURAL_GENITIVE    3

#define SPEC_NO_SORTING             0x0001
#define SPEC_GEN_POS_LIST           0x0002

#if 0
// cell address constants (old implementation)
#define NOTE_LENGTH_MAX             28

#define SPEC_NAME_LENGTH_MAX        34
#define SPEC_NOTE_LENGTH_MAX        11

#define ADDR_CMPIDX_COL_REFDES      wxT( "A" )
#define ADDR_CMPIDX_COL_NAME        wxT( "B" )
#define ADDR_CMPIDX_COL_QTY         wxT( "C" )
#define ADDR_CMPIDX_COL_NOTE        wxT( "D" )

#define ADDR_SPEC_COL_FORMAT        wxT( "A" )
#define ADDR_SPEC_COL_POSITION      wxT( "C" )
#define ADDR_SPEC_COL_DESIGNATION   wxT( "D" )
#define ADDR_SPEC_COL_NAME          wxT( "E" )
#define ADDR_SPEC_COL_QTY           wxT( "F" )
#define ADDR_SPEC_COL_NOTE          wxT( "G" )

#define ADDR_DESIGN_NAME            wxT( "B28" )
#define ADDR_DESIGNATION            wxT( "B27" )
#define ADDR_FIRST_USE              wxT( "B2" )
#define ADDR_DEVELOPER              wxT( "A28.2.1" )
#define ADDR_VERIFIER               wxT( "A28.2.2" )
#define ADDR_APPROVER               wxT( "A28.2.5" )
#define ADDR_COMPANY                wxT( "C28.1.3" )
#define ADDR_SHEET_QTY              wxT( "C28.5.2" )

#define ADDR_MIDSHEET_NUMBER        wxT( "C30.1.2" )
#define ADDR_MIDSHEET_DESIGNATION   wxT( "B30" )

#define ADDR_LASTSHEET_NUMBER       wxT( "C28.1.2" )
#define ADDR_LASTSHEET_DESIGNATION  wxT( "B28" )
#else
// cell address constants (new implementation)
#define NAME_LENGTH_MAX             46
#define NOTE_LENGTH_MAX             20

#define SPEC_NAME_LENGTH_MAX        28
#define SPEC_NOTE_LENGTH_MAX        10

#define ADDR_CMPIDX_COL_REFDES      wxT( "1:A" )
#define ADDR_CMPIDX_COL_NAME        wxT( "1:B" )
#define ADDR_CMPIDX_COL_QTY         wxT( "1:C" )
#define ADDR_CMPIDX_COL_NOTE        wxT( "1:D" )

#define ADDR_SPEC_COL_FORMAT        wxT( "1:A" )
#define ADDR_SPEC_COL_POSITION      wxT( "1:C" )
#define ADDR_SPEC_COL_DESIGNATION   wxT( "1:D" )
#define ADDR_SPEC_COL_NAME          wxT( "1:E" )
#define ADDR_SPEC_COL_QTY           wxT( "1:F" )
#define ADDR_SPEC_COL_NOTE          wxT( "1:G" )

#define ADDR_DESIGN_NAME            wxT( "3:E4" )
#define ADDR_DESIGNATION            wxT( "3:F1" )
#define ADDR_FIRST_USE              wxT( "0:B1" )
#define ADDR_DEVELOPER              wxT( "3:B4" )
#define ADDR_VERIFIER               wxT( "3:B5" )
#define ADDR_APPROVER               wxT( "3:B8" )
#define ADDR_COMPANY                wxT( "3:F6" )
#define ADDR_SHEET_QTY              wxT( "3:J5" )

#define ADDR_MIDSHEET_NUMBER        wxT( "3:G3" )
#define ADDR_MIDSHEET_DESIGNATION   wxT( "3:F1" )

#define ADDR_LASTSHEET_NUMBER       wxT( "3:G3" )
#define ADDR_LASTSHEET_DESIGNATION  wxT( "3:F1" )
#endif

class DOC_COMMON
{
public:
    DOC_COMMON();
    ~DOC_COMMON();

    int m_current_row;
    int m_current_sheet;
    int m_specification_pos_field;

    void OO_PrintCompIndexDocRow( COMMON_DOC_IFACE* aDocIface,
                                  wxString          aRef_des,
                                  wxString          aName,
                                  int               aQty,
                                  wxString          aNote,
                                  int               aStyle,
                                  int               aReserve_strings,
                                  COMPONENT_DB*     aComponentDB );

    void ProcessSingleVariant( COMMON_DOC_IFACE* aDocIface,
                               COMPONENT_ARRAY*  aSingleVariantComponents,
                               int               aVariant,
                               COMPONENT_DB*     aComponentDB );

    void OO_PrintSpecificationDocRow( COMMON_DOC_IFACE* aDocIface,
                                      wxString          aFormat,
                                      int               aPos,
                                      wxString          aDesignation,
                                      wxString          aName,
                                      wxString          aQty,
                                      wxString          aNote,
                                      int               aStyle,
                                      int               aReserve_strings,
                                      COMPONENT_DB*     aComponentDB );

    void Specification_ProcessSingleVariant( COMMON_DOC_IFACE* aDocIface,
                                             wxArrayPtrVoid*   aSingleVariantComponents,
                                             int               aVariant,
                                             COMPONENT_DB*     aComponentDB,
                                             wxArrayString*    aSpecification_positions,
                                             int               aMode );

    bool Form_a_set( COMMON_DOC_IFACE* aDocIface,
                     COMPONENT_DB*     aComponentDB,
                     int               aPart_type,
                     int               aVariant,
                     wxArrayString*    aSpecification_positions );

    wxString ChangeSuffixOfDesignation( wxString aDesignation,
                                        wxString aSuffix=wxEmptyString,
                                        bool     aInsert=false );

private:
    void ChangeWordForm( wxString* aStr, int aType );

    void PrintTitleGroup( COMMON_DOC_IFACE* aDocIface,
                          COMPONENT_ARRAY*  aTitle_group_components,
                          int               aPositions,
                          wxArrayString*    aGroup_types,
                          wxString          aBase_title,
                          int               aVariant,
                          COMPONENT_DB*     aComponentDB );

    void OO_AttachNewSpecificationSheet( COMMON_DOC_IFACE* aDocIface,
                                         COMPONENT_DB*     aComponentDB );

    void SortStringArray( wxArrayString* aString_array );
    double ExtractAbsoluteValue( wxString aValue );

    bool CompareTwoCompPositionsValues( TCOMPONENT_ATTRS* aComp_pos1,
                                        TCOMPONENT_ATTRS* aComp_pos2 );

    bool CompareTwoComponentsValues( COMPONENT* aComponent1,
                                     COMPONENT* aComponent2,
                                     int variant );

    void SortComponentsArrayByValue( COMPONENT_ARRAY* aComponent_array, int aVariant );
    void SortComponentsArrayByDesignation( COMPONENT_ARRAY* aComponent_array, int aVariant );
    void SortComponentsArrayBySubType( COMPONENT_ARRAY* aComponent_array, int aVariant );
    void SortCompPositionsArrayByDesignation( TCOMPONENT_ATTRS_ARRAY* aComp_position_array );
    void SortCompPositionsArrayBySubType( TCOMPONENT_ATTRS_ARRAY* aComp_position_array );
    void SortCompPositionsArrayByValue( TCOMPONENT_ATTRS_ARRAY* aComp_position_array );

    int  GetSpecificationPos( wxArrayString* aSpecification_positions,
                              TCOMPONENT_ATTRS* aComponentAttrs );

    void Specification_PrintTypeGroup( COMMON_DOC_IFACE* aDocIface,
                                       COMPONENT_ARRAY*  aType_group_components,
                                       int               aPositions,
                                       wxString          aBase_type,
                                       wxString          aBase_title,
                                       int               aVariant,
                                       COMPONENT_DB*     aComponentDB,
                                       wxArrayString*    aSpecification_positions );
};

} // namespace GOST_DOC_GEN

#endif    // DOC_COMMON_H
