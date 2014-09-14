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
 * @file doc_common.cpp
 */


#include <component_db.h>
#include <component.h>
#include <common_funcs.h>
#include <dictionaries.h>
#include <common_doc_iface.h>
#include <doc_common.h>

namespace GOST_DOC_GEN {

DOC_COMMON::DOC_COMMON()
{
    m_current_row = 0;
    m_current_sheet = 0;
    m_specification_pos_field = 0;

#ifdef USE_PYTHON_GOSTDOCGEN
    // launch the Python interpreter
    Py_Initialize();

    ImportPyModule( wxT( "doc_common" ) );
#endif
}


DOC_COMMON::~DOC_COMMON()
{
#ifdef USE_PYTHON_GOSTDOCGEN
    Py_Finalize();
#endif
}


void DOC_COMMON::OO_PrintCompIndexDocRow( COMMON_DOC_IFACE* aDocIface,
                                          wxString          aRef_des,
                                          wxString          aName,
                                          int               aQty,
                                          wxString          aNote,
                                          int               aStyle,
                                          int               aReserve_strings,
                                          COMPONENT_DB*     aComponentDB )
{
    int             i, row_step;
    wxString        cell_address, fileName;
    wxArrayString   name_string_array, string_array;

    row_step = 1;

    if( ( i = aName.Find( wxT( "%" ) ) )!=wxNOT_FOUND )
        StringInsert( &aName, wxT( ' ' ), i );

    SplitString( aName, &name_string_array, NAME_LENGTH_MAX, 0 );
    SplitString( aNote, &string_array, NOTE_LENGTH_MAX, SPLIT_COMMA_ENA | SPLIT_DOT_ENA );

    row_step = std::max( row_step, (int) name_string_array.GetCount() );
    row_step = std::max( row_step, (int) string_array.GetCount() );
    aReserve_strings = std::max( aReserve_strings, row_step );

    if( ( ( m_current_sheet==0 )
          && (m_current_row + aReserve_strings - 1 > FIRST_SHEET_LAST_STR_I) )
        || ( ( m_current_sheet > 0 )
             && (m_current_row + aReserve_strings - 1 > SECOND_SHEET_LAST_STR_I) ) )
    {
        m_current_sheet++;
        m_current_row = 2;

        if( ( fileName = GetResourceFile( wxT( "templates/CompIndexMiddleSheet_template.odt" ) ) )
            == wxEmptyString
            || !aDocIface->AppendDocument( fileName ) )
            return;

        aDocIface->SelectTable( m_current_sheet );

        // fill 'sheet number' field
        aDocIface->PutCell( ADDR_MIDSHEET_NUMBER,
                            wxString::Format( wxT( "%d" ), m_current_sheet + 1 ),
                            0 );

        // fill 'designation' field
        aDocIface->PutCell( ADDR_MIDSHEET_DESIGNATION,
                            GetCompIndexDesignation( aComponentDB->m_designation ), 0 );
    }

    aDocIface->SelectTable( m_current_sheet );

    // ref des
    // for (i=0;i<(int)string_array.GetCount();i++)
    // {
    cell_address = ADDR_CMPIDX_COL_REFDES;
    cell_address += wxString::Format( wxT( "%d" ), m_current_row /* + i*/ );
    aDocIface->PutCell( cell_address, /*string_array[i]*/ aRef_des, 0 );
    // }

    // name
    for( i = 0; i<(int) name_string_array.GetCount(); i++ )
    {
        cell_address = ADDR_CMPIDX_COL_NAME;
        cell_address += wxString::Format( wxT( "%d" ), m_current_row + i );
        aDocIface->PutCell( cell_address, name_string_array[i], aStyle );
    }

    // qty
    if( aQty > 0 )
    {
        cell_address = ADDR_CMPIDX_COL_QTY;
        cell_address += wxString::Format( wxT( "%d" ), m_current_row );
        aDocIface->PutCell( cell_address, wxString::Format( wxT( "%d" ), aQty ), 0 );
    }

    // note
    for( i = 0; i < (int)string_array.GetCount(); i++ )
    {
        cell_address = ADDR_CMPIDX_COL_NOTE;
        cell_address += wxString::Format( wxT( "%d" ), m_current_row + i );
        aDocIface->PutCell( cell_address, string_array[i], 0 );
    }

    m_current_row += row_step;
}


void DOC_COMMON::ChangeWordForm( wxString* aStr, int aType )
{
    int     i;
    bool    title_found = false;

    for( i = 0; i < DICTIONARY_SIZE; i++ )
    {
        if( dictionary[i].singular_form == *aStr )
        {
            if( aType == WORDFORM_PLURAL )
                *aStr = dictionary[i].plural_form;
            else if( aType==WORDFORM_SINGULAR_GENITIVE )
                *aStr = dictionary[i].singular_genitive_form;
            else if( aType==WORDFORM_PLURAL_GENITIVE )
                *aStr = dictionary[i].plural_genitive_form;

            title_found = true;
            break;
        }
    }

    if( !title_found )
    {
        wxString lastChar = (*aStr)[aStr->Len() - 1];

        if( lastChar == wxT( "ь" ) )
        {
            (*aStr).RemoveLast();

            if( aType == WORDFORM_PLURAL )
                (*aStr) += wxT( "и" );
            else if( aType==WORDFORM_SINGULAR_GENITIVE )
                (*aStr) += wxT( "я" );
            else if( aType==WORDFORM_PLURAL_GENITIVE )
                (*aStr) += wxT( "ей" );
        }
        else
        {
            if( aType == WORDFORM_PLURAL )
                (*aStr) += wxT( "ы" );
            else if( aType==WORDFORM_SINGULAR_GENITIVE )
                (*aStr) += wxT( "а" );
            else if( aType==WORDFORM_PLURAL_GENITIVE )
                (*aStr) += wxT( "ов" );
        }
    }
}


void DOC_COMMON::PrintTitleGroup( COMMON_DOC_IFACE* aDocIface,
                                  COMPONENT_ARRAY*  aTitle_group_components,
                                  int               aPositions,
                                  wxArrayString*    aGroup_types,
                                  wxString          aBase_title,
                                  int               aVariant,
                                  COMPONENT_DB*     aComponentDB )
{
    wxString           note, ref_des_group, str;
    COMPONENT*         pComponent, * pBaseComponent;
    pTCOMPONENT_ATTRS  comp_attrs, baseComp_attrs;
    int                ref_des_base_i, qty, i, item_var_i, title_group_end, base_refdes_postfix,
                       next_refdes_postfix;

    OO_PrintCompIndexDocRow( aDocIface,
                             wxT( "" ), wxT( "" ), 0,
                             wxT( "" ), 0, 1, aComponentDB );

    if( aPositions > 1 )
    {
        ChangeWordForm( &aBase_title, WORDFORM_PLURAL );

        if( aBase_title==wxT( "Конденсаторы" ) || aBase_title==wxT( "Резисторы" ) )
        {
            int pos;

            for( i = 0; i < (int)aGroup_types->GetCount(); i++ )
            {
                pos = (*aGroup_types)[i].Find( wxT( "танталовый" ) );

                if( pos!=wxNOT_FOUND )
                {
                    (*aGroup_types)[i].Remove( pos, 10 );
                    StringInsert( &(*aGroup_types)[i], wxT( "танталовые" ), pos );
                }

                OO_PrintCompIndexDocRow( aDocIface,
                                         wxT( "" ),
                                         aBase_title + wxT( " " ) + (*aGroup_types)[i],
                                         0,
                                         wxT( "" ),
                                         TEXT_CENTERED,
                                         1,
                                         aComponentDB );
            }
        }
        else
        {
            OO_PrintCompIndexDocRow( aDocIface,
                                     wxT( "" ), aBase_title, 0,
                                     wxT( "" ), TEXT_CENTERED, 1,
                                     aComponentDB );
        }
    }

    title_group_end = (*aTitle_group_components).GetCount() - 1;
    ref_des_base_i  = 0;

    while( ref_des_base_i <= title_group_end )
    {
        base_refdes_postfix =
            RefDesPostfix( ( (COMPONENT*) (*aTitle_group_components)[ref_des_base_i] )->m_RefDes );
        qty = 1;
        pBaseComponent  = (COMPONENT*) (*aTitle_group_components)[ref_des_base_i];
        item_var_i      = ConvertVariantToItemVarI( pBaseComponent, aVariant );
        baseComp_attrs  = (pTCOMPONENT_ATTRS) pBaseComponent->m_comp_attr_variants[item_var_i];
        note            = baseComp_attrs->attrs[ATTR_NOTE];

        if( ref_des_base_i < title_group_end )
        {
            for( i = ref_des_base_i; i<title_group_end; i++ )
            {
                pComponent = (COMPONENT*) (*aTitle_group_components)[i + 1];
                next_refdes_postfix = RefDesPostfix( pComponent->m_RefDes );

                if( ( base_refdes_postfix!=next_refdes_postfix - 1 )
                    || ( !CompareComps( pBaseComponent, pComponent, aVariant )
                         && note != aComponentDB->m_notInstalledStr ) )
                {
                    break;
                }

                if( note == aComponentDB->m_notInstalledStr )
                {
                    comp_attrs = (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i];

                    if( comp_attrs->attrs[ATTR_NOTE] != aComponentDB->m_notInstalledStr )
                        break;
                }

                base_refdes_postfix = next_refdes_postfix;
                qty++;
            }
        }

        if( aPositions > 1 )
            str = wxT( "" );
        else
            str = aBase_title + wxT( "_" );

        str += baseComp_attrs->attrs[ATTR_TYPE];
        str += baseComp_attrs->attrs[ATTR_SUBTYPE];

        if( baseComp_attrs->attrs[ATTR_VALUE] != wxT( "" ) )
            str += wxT( "_" ) + baseComp_attrs->attrs[ATTR_VALUE];

        if( baseComp_attrs->attrs[ATTR_PRECISION] != wxT( "" ) )
            str += wxT( " ±" ) + baseComp_attrs->attrs[ATTR_PRECISION];

        if( baseComp_attrs->attrs[ATTR_DESIGNATION] != wxT( "" ) )
            str += wxT( "_" ) + baseComp_attrs->attrs[ATTR_DESIGNATION];

        if( note == aComponentDB->m_notInstalledStr )
        {
            str = wxT( "" );

            if( qty > 1 )
                note = wxT( "Не устанавливаются" );
        }
        else if( baseComp_attrs->attrs[ATTR_MANUFACTURER] != wxT( "" ) )
            note += wxT( " \"" ) + baseComp_attrs->attrs[ATTR_MANUFACTURER] + wxT( "\"" );

        note.Replace( wxT( " " ), wxT( "_" ) );

        ref_des_group = ( (COMPONENT*) (*aTitle_group_components)[ref_des_base_i] )->m_RefDes;

        if( qty > 1 )
        {
            if( qty==2 )
                ref_des_group += wxT( "," );
            else
                ref_des_group += wxT( "..." );

            ref_des_group +=
                ( (COMPONENT*) (*aTitle_group_components)[ref_des_base_i + qty - 1] )->m_RefDes;
        }

        OO_PrintCompIndexDocRow( aDocIface,
                                 ref_des_group,
                                 str,
                                 qty,
                                 note,
                                 0,
                                 1,
                                 aComponentDB );

        ref_des_base_i += qty;
    }
}


void DOC_COMMON::ProcessSingleVariant( COMMON_DOC_IFACE* aDocIface,
                                       COMPONENT_ARRAY*  aSingleVariantComponents,
                                       int               aVariant,
                                       COMPONENT_DB*     aComponentDB )
{
    COMPONENT_ARRAY    title_group_components;
    wxArrayString      group_titles;
    wxArrayString      group_types;
    wxString           str, base_title, base_ref_des_prefix;
    int                i, ref_des_group_start, ref_des_group_end, title_group_i, positions,
                       offset, item_var_i;
    COMPONENT*         pComponent, * pBaseComponent;
    pTCOMPONENT_ATTRS  comp_attrs;

    if( aSingleVariantComponents->GetCount()==0 )
        return;

    offset = 0;

    do
    {
        DefineRefDesPrefix( ( (COMPONENT*) (*aSingleVariantComponents)[offset] )->m_RefDes,
                            &base_ref_des_prefix );

        // define a regular ref des group
        ref_des_group_start = offset;
        ref_des_group_end = ref_des_group_start;

        for( i = ref_des_group_start; i < (int)aSingleVariantComponents->GetCount(); i++ )
        {
            DefineRefDesPrefix( ( (COMPONENT*) (*aSingleVariantComponents)[i] )->m_RefDes, &str );

            if( base_ref_des_prefix!=str )
                break;

            ref_des_group_end = i;
        }

        offset = ref_des_group_end + 1;

        // define title groups in the ref des group
        group_titles.Clear();

        for( i = ref_des_group_start; i<=ref_des_group_end; i++ )
        {
            pComponent = (COMPONENT*) (*aSingleVariantComponents)[i];
            str =
                ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[ConvertVariantToItemVarI(
                                                                           pComponent,
                                                                           aVariant )]
                )->attrs[ATTR_NAME];

            if( !DoesStringExist( &group_titles, str ) )
                group_titles.Add( str );
        }

        for( title_group_i = 0; title_group_i<(int) group_titles.GetCount(); title_group_i++ )
        {
            base_title = group_titles[title_group_i];
            // form title group component array
            title_group_components.Clear();

            for( i = ref_des_group_start; i<=ref_des_group_end; i++ )
            {
                pComponent  = (COMPONENT*) (*aSingleVariantComponents)[i];
                item_var_i  = ConvertVariantToItemVarI( pComponent, aVariant );
                str = ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i]
                      )->attrs[ATTR_NAME];

                if( str==base_title )
                    title_group_components.Add( pComponent );
            }

            // define number of positions and types in the title group
            positions = 1;
            group_types.Clear();
            pBaseComponent = (COMPONENT*) title_group_components[0];

            for( i = 0; i < (int) title_group_components.GetCount(); i++ )
            {
                pComponent  = (COMPONENT*) title_group_components[i];
                item_var_i  = ConvertVariantToItemVarI( pComponent, aVariant );
                comp_attrs = (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i];
                str = comp_attrs->attrs[ATTR_TYPE];

                if( !DoesStringExist( &group_types, str )
                    && comp_attrs->attrs[ATTR_NOTE] != aComponentDB->m_notInstalledStr )
                {
                    group_types.Add( str );
                }

                if( !CompareComps( pBaseComponent, pComponent, aVariant ) )
                {
                    positions++;
                    pBaseComponent = pComponent;
                }
            }

            // print the title group
            PrintTitleGroup( aDocIface, &title_group_components,
                             positions, &group_types, base_title,
                             aVariant, aComponentDB );
        }
    } while( offset < (int) aSingleVariantComponents->GetCount() );
}




// #define FIRST_SHEET_LAST_STR_I       26
// #define SECOND_SHEET_LAST_STR_I      29

void DOC_COMMON::OO_AttachNewSpecificationSheet( COMMON_DOC_IFACE* aDocIface,
                                                 COMPONENT_DB*     aComponentDB )
{
    wxString fileName;

    m_current_sheet++;
    m_current_row = 2;

    if( ( fileName = GetResourceFile( wxT( "templates/SpecificationMiddleSheet_template.odt" ) ) )
        == wxEmptyString
        || !aDocIface->AppendDocument( fileName ) )
        return;

    aDocIface->SelectTable( m_current_sheet );

    // fill 'sheet number' field
    aDocIface->PutCell( ADDR_MIDSHEET_NUMBER,
                        wxString::Format( wxT( "%d" ), m_current_sheet + 1 ),
                        0 );
    // fill 'designation' field
    aDocIface->PutCell( ADDR_MIDSHEET_DESIGNATION, aComponentDB->m_designation, 0 );
}


void DOC_COMMON::OO_PrintSpecificationDocRow( COMMON_DOC_IFACE* aDocIface,
                                              wxString          aFormat,
                                              int               aPos,
                                              wxString          aDesignation,
                                              wxString          aName,
                                              wxString          aQty,
                                              wxString          aNote,
                                              int               aStyle,
                                              int               aReserve_strings,
                                              COMPONENT_DB*     aComponentDB )
{
    int             i, note_i, row_step;
    wxString        cell_address;
    wxArrayString   name_string_array, note_string_array;

    row_step = 1;

    if( ( i = aName.Find( wxT( "%" ) ) )!=wxNOT_FOUND )
        StringInsert( &aName, wxT( ' ' ), i );

    SplitString( aName, &name_string_array, SPEC_NAME_LENGTH_MAX, 0 );
    SplitString( aNote, &note_string_array, SPEC_NOTE_LENGTH_MAX, SPLIT_COMMA_ENA | SPLIT_DOT_ENA );

    row_step    = std::max( row_step, (int) name_string_array.GetCount() );
    row_step    = std::max( row_step, (int) note_string_array.GetCount() );
    aReserve_strings = std::max( aReserve_strings, row_step );

    if( ( ( m_current_sheet==0 )
          && ( m_current_row + aReserve_strings - 1 > FIRST_SHEET_LAST_STR_I ) )
        || ( ( m_current_sheet > 0 )
             && ( m_current_row + aReserve_strings - 1 > SECOND_SHEET_LAST_STR_I ) ) )
        OO_AttachNewSpecificationSheet( aDocIface, aComponentDB );

    aDocIface->SelectTable( m_current_sheet );

    // format field
    cell_address = ADDR_SPEC_COL_FORMAT;
    cell_address += wxString::Format( wxT( "%d" ), m_current_row );
    aDocIface->PutCell( cell_address, aFormat, 0 );

    // position field
    if( aPos > 0 )
    {
        cell_address = ADDR_SPEC_COL_POSITION;
        cell_address += wxString::Format( wxT( "%d" ), m_current_row );
        // 'position' field
        aDocIface->PutCell( cell_address, wxString::Format( wxT( "%d" ), aPos ), 0 );
    }

    // designation field
    cell_address = ADDR_SPEC_COL_DESIGNATION;
    cell_address += wxString::Format( wxT( "%d" ), m_current_row );
    aDocIface->PutCell( cell_address, aDesignation, 0 );

    // name field
    for( i = 0; i<(int) name_string_array.GetCount(); i++ )
    {
        cell_address = ADDR_SPEC_COL_NAME;
        cell_address += wxString::Format( wxT( "%d" ), m_current_row + i );
        aDocIface->PutCell( cell_address, name_string_array[i], aStyle );
    }

    // qty field
    if( aQty != wxT( "" ) )
    {
        cell_address = ADDR_SPEC_COL_QTY;
        cell_address += wxString::Format( wxT( "%d" ), m_current_row );
        aDocIface->PutCell( cell_address, aQty, 0 );
    }

    // note field
    note_i = 0;

    for( i = 0; i<(int) note_string_array.GetCount(); i++ )
    {
        cell_address = ADDR_SPEC_COL_NOTE;
        cell_address += wxString::Format( wxT( "%d" ), m_current_row + note_i );
        aDocIface->PutCell( cell_address, note_string_array[i], 0 );

        if( ( ( m_current_sheet==0 )
              && ( m_current_row + note_i >= FIRST_SHEET_LAST_STR_I ) )
            || ( ( m_current_sheet > 0 )
                 && ( m_current_row + note_i >= SECOND_SHEET_LAST_STR_I ) ) )
        {
            OO_AttachNewSpecificationSheet( aDocIface, aComponentDB );
            aDocIface->SelectTable( m_current_sheet );

            row_step = note_string_array.GetCount() - note_i;
            note_i   = 0;
        }
        else
            note_i++;
    }

    m_current_row += row_step;
}


void DOC_COMMON::SortStringArray( wxArrayString* aString_array )
{
    int         i, j, item_qty = aString_array->GetCount();
    wxString    tmp;

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour strings
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            if( (*aString_array)[j] > (*aString_array)[j + 1] )
            {
                // if the strings are placed out of order then the strings are swapped
                tmp = (*aString_array)[j]; (*aString_array)[j] = (*aString_array)[j + 1];
                (*aString_array)[j + 1] = tmp;
            }
        }
    }
}


double DOC_COMMON::ExtractAbsoluteValue( wxString aValue )
{
    int         i, start, exp;
    double      mant;
    wxString    str;

    if( ( start = FindOneOf( aValue, wxT( "0123456789" ) ) )==wxNOT_FOUND )
        return -1.0;

    aValue.Remove( 0, start );

    start = -1;

    for( i = 0; i<(int) aValue.Len(); i++ )
        if( (aValue[i] < wxT( '0' )
            || aValue[i] > wxT( '9' ) )
            && aValue[i] != wxT( ',' )
            && aValue[i] != wxT( '.' ) )
        {
            start = i;
            break;
        }

    // the smallest exponenta is for pF.
    // It can fail for Ohms, however it does not matter for sorting.
    exp    = -12;
    aValue += wxT( ' ' );

    if( start != -1 )
    {
        for( i = 0; i < UNITS_MATRIX_SIZE; i++ )
        {
            str = meas_units_matrix[i].unit;
            str += wxT( ' ' );

            if( FindFrom( aValue, str, start )!=wxNOT_FOUND )
            {
                exp = meas_units_matrix[i].exp;
                break;
            }
        }

        aValue = aValue.Left( start );
    }

    aValue.Replace( wxT( "," ), wxT( "." ) );

    LOCALE_IO toggle;

    if( start == -1 )
        aValue.ToDouble( &mant );
    else
        aValue.Left( start ).ToDouble( &mant );

    return mant * pow( 10.0, exp );
}


// returns true if comp_pos1 value is greater than value of comp_pos2
bool DOC_COMMON::CompareTwoCompPositionsValues( TCOMPONENT_ATTRS* aComp_pos1,
                                                TCOMPONENT_ATTRS* aComp_pos2 )
{
    double absvalue1, absvalue2;

    if( ( absvalue1 = ExtractAbsoluteValue( aComp_pos1->attrs[ATTR_VALUE] ) )==-1.0 )
        return false;

    if( ( absvalue2 = ExtractAbsoluteValue( aComp_pos2->attrs[ATTR_VALUE] ) )==-1.0 )
        return false;

    return absvalue1 > absvalue2;
}


// returns true if component1 value is greater than value of component2
bool DOC_COMMON::CompareTwoComponentsValues( COMPONENT* aComponent1,
                                             COMPONENT* aComponent2,
                                             int variant )
{
    int comp1_var_i = ConvertVariantToItemVarI( aComponent1, variant );
    int comp2_var_i = ConvertVariantToItemVarI( aComponent2, variant );

    return CompareTwoCompPositionsValues(
        (pTCOMPONENT_ATTRS) aComponent1->m_comp_attr_variants[comp1_var_i],
        (pTCOMPONENT_ATTRS) aComponent2->m_comp_attr_variants[comp2_var_i] );
}


void DOC_COMMON::SortComponentsArrayByValue( COMPONENT_ARRAY* aComponent_array, int aVariant )
{
    int         i, j, item_qty;
    COMPONENT*  tmp;

    item_qty = aComponent_array->GetCount();

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour components
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            if( CompareTwoComponentsValues( (COMPONENT*) (*aComponent_array)[j],
                                            (COMPONENT*) (*aComponent_array)[j + 1],
                                            aVariant ) )
            {
                // if the components are placed out of order then they are swapped
                tmp = (COMPONENT*) (*aComponent_array)[j];
                (*aComponent_array)[j]     = (*aComponent_array)[j + 1];
                (*aComponent_array)[j + 1] = (COMPONENT*) tmp;
            }
        }
    }
}


void DOC_COMMON::SortComponentsArrayByDesignation( COMPONENT_ARRAY* aComponent_array,
                                                   int aVariant )
{
    int         comp1_var_i, comp2_var_i;
    int         i, j, item_qty;
    COMPONENT*  tmp, * pComponent1, * pComponent2;
    wxString    str1, str2;

    item_qty = aComponent_array->GetCount();

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour components
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            pComponent1 = (COMPONENT*) (*aComponent_array)[j];
            pComponent2 = (COMPONENT*) (*aComponent_array)[j + 1];
            comp1_var_i = ConvertVariantToItemVarI( pComponent1, aVariant );
            comp2_var_i = ConvertVariantToItemVarI( pComponent2, aVariant );

            str1 =
                ( (pTCOMPONENT_ATTRS) pComponent1->m_comp_attr_variants[comp1_var_i]
                )->attrs[ATTR_DESIGNATION];

            str2 =
                ( (pTCOMPONENT_ATTRS) pComponent2->m_comp_attr_variants[comp2_var_i]
                )->attrs[ATTR_DESIGNATION];

            if( str1 > str2 )
            {
                // if the components are placed out of order then they are swapped
                tmp = (COMPONENT*) (*aComponent_array)[j];
                (*aComponent_array)[j]      = (*aComponent_array)[j + 1];
                (*aComponent_array)[j + 1]  = (COMPONENT*) tmp;
            }
        }
    }
}


void DOC_COMMON::SortComponentsArrayBySubType( COMPONENT_ARRAY* aComponent_array, int aVariant )
{
    int         comp1_var_i, comp2_var_i;
    int         i, j, item_qty;
    COMPONENT*  tmp, * pComponent1, * pComponent2;
    wxString    str1, str2;

    item_qty = aComponent_array->GetCount();

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour components
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            pComponent1 = (COMPONENT*) (*aComponent_array)[j];
            pComponent2 = (COMPONENT*) (*aComponent_array)[j + 1];
            comp1_var_i = ConvertVariantToItemVarI( pComponent1, aVariant );
            comp2_var_i = ConvertVariantToItemVarI( pComponent2, aVariant );

            str1 =
                ( (pTCOMPONENT_ATTRS) pComponent1->m_comp_attr_variants[comp1_var_i]
                )->attrs[ATTR_SUBTYPE];
            str2 =
                ( (pTCOMPONENT_ATTRS) pComponent2->m_comp_attr_variants[comp2_var_i]
                )->attrs[ATTR_SUBTYPE];

            if( str1 > str2 )
            {
                // if the components are placed out of order then they are swapped
                tmp = (COMPONENT*) (*aComponent_array)[j];
                (*aComponent_array)[j]      = (*aComponent_array)[j + 1];
                (*aComponent_array)[j + 1]  = (COMPONENT*) tmp;
            }
        }
    }
}


void DOC_COMMON::SortCompPositionsArrayByDesignation( TCOMPONENT_ATTRS_ARRAY* aComp_position_array )
{
    int i, j, item_qty;
    TCOMPONENT_ATTRS* tmp, * pComp_pos1, * pComp_pos2;

    item_qty = aComp_position_array->GetCount();

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour comp positions
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            pComp_pos1  = (pTCOMPONENT_ATTRS) (*aComp_position_array)[j];
            pComp_pos2  = (pTCOMPONENT_ATTRS) (*aComp_position_array)[j + 1];

            if( pComp_pos1->attrs[ATTR_DESIGNATION] > pComp_pos2->attrs[ATTR_DESIGNATION] )
            {
                // if the comp positions are placed out of order then they are swapped
                tmp = (pTCOMPONENT_ATTRS) (*aComp_position_array)[j];
                (*aComp_position_array)[j]      = (*aComp_position_array)[j + 1];
                (*aComp_position_array)[j + 1]  = (pTCOMPONENT_ATTRS) tmp;
            }
        }
    }
}


void DOC_COMMON::SortCompPositionsArrayBySubType( TCOMPONENT_ATTRS_ARRAY* aComp_position_array )
{
    int i, j, item_qty;
    TCOMPONENT_ATTRS* tmp, * pComp_pos1, * pComp_pos2;

    item_qty = aComp_position_array->GetCount();

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour comp positions
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            pComp_pos1  = (pTCOMPONENT_ATTRS) (*aComp_position_array)[j];
            pComp_pos2  = (pTCOMPONENT_ATTRS) (*aComp_position_array)[j + 1];

            if( pComp_pos1->attrs[ATTR_SUBTYPE] > pComp_pos2->attrs[ATTR_SUBTYPE] )
            {
                // if the comp positions are placed out of order then they are swapped
                tmp = (pTCOMPONENT_ATTRS) (*aComp_position_array)[j];
                (*aComp_position_array)[j]      = (*aComp_position_array)[j + 1];
                (*aComp_position_array)[j + 1]  = (pTCOMPONENT_ATTRS) tmp;
            }
        }
    }
}


void DOC_COMMON::SortCompPositionsArrayByValue( TCOMPONENT_ATTRS_ARRAY* aComp_position_array )
{
    int i, j, item_qty;
    TCOMPONENT_ATTRS* tmp;

    item_qty = aComp_position_array->GetCount();

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour comp positions
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            if( CompareTwoCompPositionsValues( (pTCOMPONENT_ATTRS) (*aComp_position_array)[j],
                                               (pTCOMPONENT_ATTRS) (*aComp_position_array)[j + 1] ) )
            {
                // if the comp positions are placed out of order then they are swapped
                tmp = (pTCOMPONENT_ATTRS) (*aComp_position_array)[j];
                (*aComp_position_array)[j]      = (*aComp_position_array)[j + 1];
                (*aComp_position_array)[j + 1]  = (pTCOMPONENT_ATTRS) tmp;
            }
        }
    }
}


// return -1 if no given position found
int DOC_COMMON::GetSpecificationPos( wxArrayString* aSpecification_positions,
                                     TCOMPONENT_ATTRS* aComponentAttrs )
{
    wxString    str;
    int         i;

    str = aComponentAttrs->attrs[ATTR_NAME]
        + aComponentAttrs->attrs[ATTR_TYPE]
        + aComponentAttrs->attrs[ATTR_SUBTYPE]
        + aComponentAttrs->attrs[ATTR_VALUE]
        + aComponentAttrs->attrs[ATTR_PRECISION]
        + aComponentAttrs->attrs[ATTR_NOTE]
        + aComponentAttrs->attrs[ATTR_DESIGNATION]
        + aComponentAttrs->attrs[ATTR_MANUFACTURER];

    for( i = 0; i<(int) aSpecification_positions->GetCount(); i++ )
    {
        if( (*aSpecification_positions)[i]==str )
            return i + m_specification_pos_field;
    }

    return -1;
}


void DOC_COMMON::Specification_PrintTypeGroup( COMMON_DOC_IFACE* aDocIface,
                                               COMPONENT_ARRAY*  aType_group_components,
                                               int               aPositions,
                                               wxString          aBase_type,
                                               wxString          aBase_title,
                                               int               aVariant,
                                               COMPONENT_DB*     aComponentDB,
                                               wxArrayString*    aSpecification_positions )
{
    wxString            note, manufacturer, ref_des_group, str, qty_str;
    COMPONENT*          pComponent;
    pTCOMPONENT_ATTRS   componentAttrs;
    int     ref_des_base, qty, i, item_var_i, type_group_end, spec_pos;

    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

    if( aPositions > 1 )
    {
        ChangeWordForm( &aBase_title, WORDFORM_PLURAL );

        int pos;
        pos = aBase_type.Find( wxT( "танталовый" ) );

        if( pos!=wxNOT_FOUND )
        {
            aBase_type.Remove( pos, 10 );
            StringInsert( &aBase_type, wxT( "танталовые" ), pos );
        }

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                     wxT( "" ), aBase_title + wxT( " " ) + aBase_type, wxT( "" ),
                                     wxT( "" ), 0, 1,
                                     aComponentDB );
    }

    qty = 1;
    pComponent = (COMPONENT*) (*aType_group_components)[0];
    ref_des_base = 0;
    type_group_end = (*aType_group_components).GetCount() - 1;

    for( i = 0; i<=type_group_end + 1; i++ )
    {
        if( (i<=type_group_end)
            && CompareComps( pComponent, (COMPONENT*) (*aType_group_components)[i], aVariant ) )
        {
            if( i!=0 )
                qty++;
        }
        else
        {
            if( aPositions > 1 )
                str = wxT( "" );
            else
                str = aBase_title + wxT( "_" );

            str.Replace( wxT( " " ), wxT( "_" ) );    // allow to split a title

            item_var_i      = ConvertVariantToItemVarI( pComponent, aVariant );
            componentAttrs  = (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i];
            str += componentAttrs->attrs[ATTR_TYPE];
            str += componentAttrs->attrs[ATTR_SUBTYPE];

            if( componentAttrs->attrs[ATTR_VALUE] != wxT( "" ) )
                str += wxT( "_" ) + componentAttrs->attrs[ATTR_VALUE];

            if( componentAttrs->attrs[ATTR_PRECISION] != wxT( "" ) )
                str += wxT( " ±" ) + componentAttrs->attrs[ATTR_PRECISION];

            manufacturer = componentAttrs->attrs[ATTR_MANUFACTURER];

            if( manufacturer != wxT( "" ) )
            {
                str += wxT( "_@Фирма_\"" );
                manufacturer.Replace( wxT( " " ), wxT( "_" ) );
                str += manufacturer;
                str += wxT( "\"" );
            }

            if( componentAttrs->attrs[ATTR_NOTE] != wxT( "" ) )
            {
                note = componentAttrs->attrs[ATTR_NOTE];
                note.Replace( wxT( " " ), wxT( "_" ) );
                str += wxT( "@" ) + note;    // go to the next string
            }

            if( pComponent->ExcludeFromCompIndex )
                ref_des_group = wxT( "" );
            else
                aComponentDB->FormRefDes( aType_group_components,
                                          ref_des_base,
                                          ref_des_base + qty - 1,
                                          &ref_des_group );

            qty_str = wxString::Format( wxT( "%d" ), qty );

            if( pComponent->ComponentType==wxT( "Documentation" ) )
            {
                spec_pos    = -1;
                qty_str = wxEmptyString;
            }
            else
                spec_pos = GetSpecificationPos( aSpecification_positions, componentAttrs );

            if( pComponent->ComponentType==wxT( "AssemblyUnit" ) )
                OO_PrintSpecificationDocRow( aDocIface,
                                             pComponent->fmt,
                                             spec_pos,
                                             componentAttrs->attrs[ATTR_DESIGNATION],
                                             componentAttrs->attrs[ATTR_NAME],
                                             qty_str,
                                             componentAttrs->attrs[ATTR_NOTE],
                                             0,
                                             1,
                                             aComponentDB );
            else if( pComponent->ComponentType==wxT( "Goods" ) )
                OO_PrintSpecificationDocRow( aDocIface,
                                             wxT( "" ), spec_pos,
                                             componentAttrs->attrs[ATTR_DESIGNATION],
                                             str, pComponent->fmt,
                                             componentAttrs->attrs[ATTR_NOTE],
                                             0, 1, aComponentDB );
            else
                OO_PrintSpecificationDocRow( aDocIface,
                                             pComponent->fmt,
                                             spec_pos,
                                             componentAttrs->attrs[ATTR_DESIGNATION],
                                             str,
                                             qty_str,
                                             ref_des_group,
                                             0,
                                             1,
                                             aComponentDB );

            qty = 1;
            ref_des_base = i;

            if( i<=type_group_end )
                pComponent = (COMPONENT*) (*aType_group_components)[i];
        }
    }
}


void DOC_COMMON::Specification_ProcessSingleVariant( COMMON_DOC_IFACE* aDocIface,
                                                     wxArrayPtrVoid*   aSingleVariantComponents,
                                                     int               aVariant,
                                                     COMPONENT_DB*     aComponentDB,
                                                     wxArrayString*    aSpecification_positions,
                                                     int               aMode )
{
    wxArrayPtrVoid      title_group_components, type_group_components;
    wxArrayString       group_titles, group_types;
    wxString            str, base_title, base_type, base_ref_des_prefix;
    int                 i, title_group_i, type_group_i, positions, item_var_i;
    COMPONENT*          pComponent, * pBaseComponent;
    pTCOMPONENT_ATTRS   componentAttrs;

    if( aSingleVariantComponents->GetCount() == 0 )
        return;

    // define title groups in the ref des group
    group_titles.Clear();

    for( i = 0; i < (int)(*aSingleVariantComponents).GetCount(); i++ )
    {
        if( aMode & SPEC_GEN_POS_LIST )
            str = ( (pTCOMPONENT_ATTRS) (*aSingleVariantComponents)[i] )->attrs[ATTR_NAME];
        else
        {
            pComponent = (COMPONENT*) (*aSingleVariantComponents)[i];
            str =
                ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[ConvertVariantToItemVarI(
                                                                           pComponent,
                                                                           aVariant )]
                )->attrs[ATTR_NAME];
        }

        if( !DoesStringExist( &group_titles, str ) )
            group_titles.Add( str );
    }

    if( !(aMode & SPEC_NO_SORTING) )
        SortStringArray( &group_titles );

    for( title_group_i = 0; title_group_i < (int)group_titles.GetCount(); title_group_i++ )
    {
        base_title = group_titles[title_group_i];
        // form title group compononents array
        title_group_components.Clear();

        for( i = 0; i<(int) (*aSingleVariantComponents).GetCount(); i++ )
        {
            if( aMode & SPEC_GEN_POS_LIST )
            {
                componentAttrs = (pTCOMPONENT_ATTRS) (*aSingleVariantComponents)[i];

                if( componentAttrs->attrs[ATTR_NAME]==base_title )
                    title_group_components.Add( componentAttrs );
            }
            else
            {
                pComponent  = (COMPONENT*) (*aSingleVariantComponents)[i];
                item_var_i  = ConvertVariantToItemVarI( pComponent, aVariant );
                str = ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i]
                      )->attrs[ATTR_NAME];

                if( str==base_title )
                    title_group_components.Add( pComponent );
            }
        }

        // define type groups in the title group
        group_types.Clear();

        for( i = 0; i < (int)title_group_components.GetCount(); i++ )
        {
            if( aMode & SPEC_GEN_POS_LIST )
                str = ( (pTCOMPONENT_ATTRS) title_group_components[i] )->attrs[ATTR_TYPE];
            else
            {
                pComponent  = (COMPONENT*) title_group_components[i];
                item_var_i  = ConvertVariantToItemVarI( pComponent, aVariant );
                str = ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i]
                      )->attrs[ATTR_TYPE];
            }

            if( !DoesStringExist( &group_types, str ) )
                group_types.Add( str );
        }

        if( !(aMode & SPEC_NO_SORTING) )
            SortStringArray( &group_types );

        for( type_group_i = 0; type_group_i < (int)group_types.GetCount(); type_group_i++ )
        {
            base_type = group_types[type_group_i];
            // form type group compononents array
            type_group_components.Clear();

            for( i = 0; i<(int) title_group_components.GetCount(); i++ )
            {
                if( aMode & SPEC_GEN_POS_LIST )
                {
                    componentAttrs = (pTCOMPONENT_ATTRS) title_group_components[i];

                    if( componentAttrs->attrs[ATTR_TYPE]==base_type )
                        type_group_components.Add( componentAttrs );
                }
                else
                {
                    pComponent  = (COMPONENT*) title_group_components[i];
                    item_var_i  = ConvertVariantToItemVarI( pComponent, aVariant );
                    str =
                        ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i]
                        )->attrs[ATTR_TYPE];

                    if( str==base_type )
                        type_group_components.Add( pComponent );
                }
            }

            if( !(aMode & SPEC_NO_SORTING) )
            {
                if( aMode & SPEC_GEN_POS_LIST )
                {
                    SortCompPositionsArrayByDesignation(
                         (TCOMPONENT_ATTRS_ARRAY*) &type_group_components );
                    SortCompPositionsArrayBySubType(
                         (TCOMPONENT_ATTRS_ARRAY*) &type_group_components );
                    SortCompPositionsArrayByValue(
                         (TCOMPONENT_ATTRS_ARRAY*) &type_group_components );
                }
                else
                {
                    SortComponentsArrayByDesignation( (COMPONENT_ARRAY*) &type_group_components,
                                                      aVariant );
                    SortComponentsArrayBySubType( (COMPONENT_ARRAY*) &type_group_components,
                                                  aVariant );
                    SortComponentsArrayByValue( (COMPONENT_ARRAY*) &type_group_components, aVariant );
                }
            }

            if( aMode & SPEC_GEN_POS_LIST )
            {
                // add type group to all list of specification positions
                for( i = 0; i<(int) type_group_components.GetCount(); i++ )
                {
                    componentAttrs = (pTCOMPONENT_ATTRS) type_group_components[i];
                    str = componentAttrs->attrs[ATTR_NAME]
                        + componentAttrs->attrs[ATTR_TYPE]
                        + componentAttrs->attrs[ATTR_SUBTYPE]
                        + componentAttrs->attrs[ATTR_VALUE]
                        + componentAttrs->attrs[ATTR_PRECISION]
                        + componentAttrs->attrs[ATTR_NOTE]
                        + componentAttrs->attrs[ATTR_DESIGNATION]
                        + componentAttrs->attrs[ATTR_MANUFACTURER];

                    aSpecification_positions->Add( str );
                }
            }
            else
            {
                // define number of positions in the type group
                positions = 1;
                pBaseComponent = (COMPONENT*) type_group_components[0];

                for( i = 0; i<(int) type_group_components.GetCount(); i++ )
                {
                    pComponent = (COMPONENT*) type_group_components[i];

                    if( !CompareComps( pBaseComponent, pComponent, aVariant ) )
                    {
                        positions++;
                        pBaseComponent = pComponent;
                    }
                }


                // print the type group
                Specification_PrintTypeGroup( aDocIface,
                                              (COMPONENT_ARRAY*) &type_group_components, positions,
                                              base_type, base_title,
                                              aVariant, aComponentDB,
                                              aSpecification_positions );
            }
        }
    }
}


// returns false if no component presents. else true
bool DOC_COMMON::Form_a_set( COMMON_DOC_IFACE* aDocIface,
                             COMPONENT_DB*     aComponentDB,
                             int               aPart_type,
                             int               aVariant,
                             wxArrayString*    aSpecification_positions )
{
    int             i, set_type_i;
    COMPONENT*      pComponent;
    STRING_2D_ARRAY setsByTypes;
    wxArrayString*  pSetsOfType;
    wxString        str, refdes_prefix_reserved;
    COMPONENT_ARRAY singleVariantComponents;
    bool            result = false;

    if( !(aPart_type & PARTTYPE_VAR) )
        aVariant = -1;

    aComponentDB->FindSets( &aComponentDB->m_AllComponents,
                            &setsByTypes,
                            aPart_type,
                            aVariant,
                            false );

    for( set_type_i = 0; set_type_i<(int) setsByTypes.GetCount(); set_type_i++ )
    {
        pSetsOfType = (wxArrayString*) setsByTypes[set_type_i];
        refdes_prefix_reserved = (*pSetsOfType)[0];
        aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents,
                                                 aPart_type,
                                                 aVariant,
                                                 refdes_prefix_reserved );

        if( singleVariantComponents.GetCount() > 0 )
            result = true;

        // delete prefix of components refdes
        for( i = 0; i<(int) singleVariantComponents.GetCount(); i++ )
        {
            pComponent = (COMPONENT*) singleVariantComponents[i];
            pComponent->m_RefDes.Remove( 0, refdes_prefix_reserved.Len() );
        }

        if( pSetsOfType->GetCount() )
        {
            FormRefDesFromStringArray( pSetsOfType, &str );

            if( aPart_type & PARTTYPE_SPECIFICATION )
            {
                OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                             wxT( "" ), wxT( "" ), wxT( "" ),
                                             wxT( "" ), 0, 5, aComponentDB );
                OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                             wxT( "" ), wxT( "Абонентский комплект" ),
                                             wxT( "" ), str, TEXT_UNDERLINED | TEXT_CENTERED, 1,
                                             aComponentDB );

                Specification_ProcessSingleVariant( aDocIface,
                                                    (wxArrayPtrVoid*) &singleVariantComponents,
                                                    aVariant, aComponentDB,
                                                    aSpecification_positions, 0 );
            }
            else
            {
                OO_PrintCompIndexDocRow( aDocIface, wxT( "" ),
                                         wxT( "" ), 0, wxT( "" ), 0, 5, aComponentDB );
                OO_PrintCompIndexDocRow( aDocIface, str,
                                         wxT( "Абонентский комплект" ),
                                         pSetsOfType->GetCount(), wxT( "" ),
                                         TEXT_UNDERLINED | TEXT_CENTERED, 1,
                                         aComponentDB );

                ProcessSingleVariant( aDocIface,
                                      &singleVariantComponents,
                                      aVariant,
                                      aComponentDB );
            }
        }

        // restore refdes!!!
        for( i = 0; i<(int) singleVariantComponents.GetCount(); i++ )
        {
            StringInsert( &( (COMPONENT*) singleVariantComponents[i] )->m_RefDes,
                          refdes_prefix_reserved,
                          0 );
        }
    }

    return result;
}

wxString DOC_COMMON::GetCompIndexDesignation( wxString designation )
{
    int idx;

    idx = designation.Find( wxT( 'Э' ), true );
    if ( idx != wxNOT_FOUND && idx > 3)
        designation.insert( idx, 1, wxT( 'П' ) );
    else
        designation += wxT( "ПЭ3" );

    return designation;
}

} // namespace GOST_DOC_GEN
