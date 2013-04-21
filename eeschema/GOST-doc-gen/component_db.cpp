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
 * @file component_db.cpp
 */


#include <component_db.h>
#include <oo_component_index.hxx>
#include <oo_specification.hxx>
#include <math.h>
#include <sch_sheet.h>
#include <template_fieldnames.h>

namespace GOST_DOC_GEN {

COMPONENT_DB::COMPONENT_DB()
{
    DesignName = wxT( "" );
    Designation = wxT( "" );
    SpecFirstUse = wxT( "" );
    PCBDesignation = wxT( "" );
    CircuitDrawingFmt   = wxT( "" );
    AssemblyDrawingFmt  = wxT( "" );
}


COMPONENT_DB::~COMPONENT_DB()
{
    int sz;

    sz = m_AllComponents.GetCount();

    for( int n = 0; n < sz; n++ )
    {
        delete (COMPONENT*) m_AllComponents[n];
    }
}


void COMPONENT_DB::ZeroInserting( wxString* aStr )
{
    int   i, pos, str_len, dig_subset_len, dig_subset_ofs;
    char* temp_str;

    dig_subset_ofs = 0;

    while( true )
    {
        str_len = (*aStr).Len();

        if( dig_subset_ofs >= str_len )
            break;

        temp_str = strdup( *(aStr) );
        pos = strcspn( temp_str + dig_subset_ofs, "0123456789" ) + dig_subset_ofs;
        dig_subset_len = 0;
        i = pos;

        while( i < str_len
               && temp_str[i] >= wxT( '0' )
               && temp_str[i]<=wxT( '9' ) )
        {
            dig_subset_len++;
            i++;
        }

        for( i = 0; i < ZEROS_LENGTH_INS - dig_subset_len; i++ )
            StringInsert( aStr, wxT( '0' ), pos );

        dig_subset_ofs = pos + ZEROS_LENGTH_INS;
        free( temp_str );
    }
}


void COMPONENT_DB::SortComponents()
{
    int         item, item_qty, i, j;
    wxString    str;
    COMPONENT*  tmp;

    item_qty = m_AllComponents.GetCount();

    for( item = 0; item < item_qty; item++ )
    {
        str = m_AllComponents[item]->m_RefDes;
        ZeroInserting( &str );
        m_AllComponents[item]->m_SortingRefDes = str;
    }

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour items
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            if( m_AllComponents[j]->m_SortingRefDes > m_AllComponents[j + 1]->m_SortingRefDes )
            {
                // if the items are placed out of order then the items are swapped
                tmp = m_AllComponents[j]; m_AllComponents[j] = m_AllComponents[j + 1];
                m_AllComponents[j + 1] = tmp;
            }
        }
    }
}


void COMPONENT_DB::OO_GenerateComponentIndex()
{
    OO_CreateNewCompIndexDoc( this );
}


void COMPONENT_DB::OO_GenerateSpecification()
{
    OO_CreateNewSpecificationDoc( this );
}


// returns FALSE if given variant is not found
bool COMPONENT_DB::FindVariant( int aVariant )
{
    size_t i;

    for( i = 0; i < VariantsIndexes.GetCount(); i++ )
    {
        if( VariantsIndexes[i] == aVariant )
            return TRUE;
    }

    return FALSE;
}


void COMPONENT_DB::ReadVariants()
{
    size_t      item, var_i;
    int         variant;
    COMPONENT*  pComponent;

    for( item = 0; item < m_AllComponents.GetCount(); item++ )
    {
        pComponent = m_AllComponents[item];
        pComponent->ReadVariants();

        if( pComponent->m_Variants_State!=COMP_IN_CONST_PART )
        {
            for( var_i = 0; var_i<pComponent->m_comp_attr_variants.GetCount(); var_i++ )
            {
                variant = ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[var_i] )->variant;

                if( variant!=-1 && !FindVariant( variant ) )
                    VariantsIndexes.Add( variant );
            }
        }
    }

    SortCByteArray( &VariantsIndexes );
}


void COMPONENT_DB::AddNewVariant( int aVariant )
{
    VariantsIndexes.Add( aVariant );
    SortCByteArray( &VariantsIndexes );
}


void COMPONENT_DB::LoadFromKiCad()
{
    SCH_SHEET_LIST  sheetList;
    wxString        str;
    COMPONENT*      pComp;

    sheetList.GetComponents( m_cmplist, false );

    unsigned int index = 0;

    while( index < m_cmplist.GetCount() )
    {
        SCH_COMPONENT* component = m_cmplist[index].GetComponent();

        str = m_cmplist[index].GetRef();
        ZeroInserting( &str );

        pComp = new COMPONENT();
        pComp->m_RefDes = m_cmplist[index].GetRef();
        pComp->m_SortingRefDes = str;

        if( component->GetFieldCount() > FIELD1 )
            pComp->m_KiCadAttrs[ATTR_NAME].value_of_attr = component->GetField( FIELD1 )->GetText();

        pComp->m_KiCadAttrs[ATTR_TYPE].value_of_attr  = component->GetLibName();
        pComp->m_KiCadAttrs[ATTR_VALUE].value_of_attr = component->GetField( VALUE )->GetText();

        m_AllComponents.Add( pComp );
        index++;
    }

    SortComponents();

    ReadVariants();
}


void COMPONENT_DB::FormRefDes( COMPONENT_ARRAY* aTitle_group_components,
                               int              aStart_item,
                               int              aEnd_item,
                               wxString*        aResult )
{
    wxString    subgroup, base_refdes_prefix, next_refdes_prefix;
    int         base_refdes_postfix, next_refdes_postfix, cur_pos, qty, i;

    cur_pos = aStart_item;

    if( cur_pos == aEnd_item )
    {
        *aResult = ( (COMPONENT*) (*aTitle_group_components)[cur_pos] )->m_RefDes;
        return;
    }

    *aResult = wxT( "" );

    while( cur_pos <= aEnd_item )
    {
        base_refdes_postfix = RefDesPostfix(
             ( (COMPONENT*) (*aTitle_group_components)[cur_pos] )->m_RefDes );
        DefineRefDesPrefix( ( (COMPONENT*) (*aTitle_group_components)[cur_pos] )->m_RefDes,
                            &base_refdes_prefix );
        qty = 1;

        if( cur_pos < aEnd_item )
        {
            for( i = cur_pos; i < aEnd_item; i++ )
            {
                next_refdes_postfix =
                    RefDesPostfix( ( (COMPONENT*) (*aTitle_group_components)[i + 1] )->m_RefDes );
                DefineRefDesPrefix( ( (COMPONENT*) (*aTitle_group_components)[i + 1] )->m_RefDes,
                                    &next_refdes_prefix );

                if( (base_refdes_postfix!=next_refdes_postfix - 1)
                    || (base_refdes_prefix!=next_refdes_prefix) )
                    break;

                base_refdes_postfix = next_refdes_postfix;
                qty++;
            }
        }

        if( qty==1 )
            (*aResult) += ( (COMPONENT*) (*aTitle_group_components)[cur_pos] )->m_RefDes + wxT( "," );
        else if( qty==2 )
            (*aResult) += ( (COMPONENT*) (*aTitle_group_components)[cur_pos] )->m_RefDes +
                         wxT( "," ) +
                         ( (COMPONENT*) (*aTitle_group_components)[cur_pos + 1] )->m_RefDes + 
                         wxT( "," );
        else
            (*aResult) += ( (COMPONENT*) (*aTitle_group_components)[cur_pos] )->m_RefDes +
                         wxT( "..." ) +
                         ( (COMPONENT*) (*aTitle_group_components)[cur_pos + qty - 1] )->m_RefDes +
                         wxT( "," );

        cur_pos += qty;
    }

    // delete last comma
    (*aResult).Remove( (*aResult).Len() - 1 );
}


// returns index of the set type which given in_setrefdes belongs to.
// returns -1 if in_setrefdes belongs nothing set type
int COMPONENT_DB::FindSetType( STRING_2D_ARRAY* aAll_set_types_array, wxString aIn_setrefdes )
{
    size_t          i;
    wxArrayString*  set_type_array;
    wxString        str, in_setrefdes_prefix;

    DefineRefDesPrefix( aIn_setrefdes, &in_setrefdes_prefix );

    for( i = 0; i < aAll_set_types_array->GetCount(); i++ )
    {
        set_type_array = (wxArrayString*) (*aAll_set_types_array)[i];
        DefineRefDesPrefix( (*set_type_array)[0], &str );

        if( str==in_setrefdes_prefix )
            return i;
    }

    return -1;
}


void COMPONENT_DB::FindSets( COMPONENT_ARRAY*   aComponents,
                             STRING_2D_ARRAY*   aResult,
                             int                aPart_type,
                             int                aVariant,
                             bool               aAppend )
{
    COMPONENT*      pComponent;
    wxArrayString   letter_digit_sets;
    size_t          i;
    int             set_type_i, item_var_i;
    bool            analysis_ena;

    if( !aAppend )
        aResult->Clear();

    for( i = 0; i < aComponents->GetCount(); i++ )
    {
        pComponent = (COMPONENT*) (*aComponents)[i];

        analysis_ena = TRUE;

        if( aPart_type & PARTTYPE_VAR )
        {
            item_var_i = pComponent->GetVariant( aVariant, NULL );

            if( pComponent->m_Variants_State==COMP_IN_CONST_PART || item_var_i==-1
                || ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i] )->attrs[ATTR_NAME]==
                     wxT( "" )
                || ( PARTTYPE_SPECIFICATION
                     && ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i] )->attrs[ATTR_NOTE]
                     ==wxT( "Не устанавливается" ) ) )
                analysis_ena = FALSE;
        }
        else
        {
            if( pComponent->m_Variants_State!=COMP_IN_CONST_PART
                || ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[0] )->attrs[ATTR_NAME]==wxT( "" ) )
                analysis_ena = FALSE;

            if( PARTTYPE_SPECIFICATION
                && ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[0] )->attrs[ATTR_NOTE]==
                wxT( "Не устанавливается" ) )
                analysis_ena = FALSE;
        }

        if( analysis_ena )
        {
            ExtractLetterDigitSets( pComponent->m_RefDes, &letter_digit_sets );

            if( letter_digit_sets.GetCount() > 1 )
            {
                set_type_i = FindSetType( aResult, letter_digit_sets[0] );

                if( set_type_i==-1 )
                {
                    aResult->Add( new wxArrayString );
                    set_type_i = aResult->GetCount() - 1;
                }

                if( !DoesStringExist( (wxArrayString*) (*aResult)[set_type_i], letter_digit_sets[0] ) )
                    ( (wxArrayString*) (*aResult)[set_type_i] )->Add( letter_digit_sets[0] );
            }
        }
    }
}


void COMPONENT_DB::ExtractPartOfComponentsDB( COMPONENT_ARRAY*  aResult,
                                              int               aPart_type,
                                              int               aVariant,
                                              wxString          aSet_prefix )
{
    size_t          i;
    int             item_var_i;
    COMPONENT*      pComponent;
    wxArrayString   letter_digit_sets;
    bool            add_ena;

    aResult->Clear();

    for( i = 0; i < m_AllComponents.GetCount(); i++ )
    {
        pComponent = (COMPONENT*) m_AllComponents[i];
        ExtractLetterDigitSets( pComponent->m_RefDes, &letter_digit_sets );

        add_ena = TRUE;

        if( aPart_type & PARTTYPE_VAR )
        {
            item_var_i = pComponent->GetVariant( aVariant, NULL );

            if( pComponent->m_Variants_State==COMP_IN_CONST_PART || item_var_i==-1
                || ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i] )->attrs[ATTR_NAME]==
                     wxT( "" )
                || ( (aPart_type & PARTTYPE_SPECIFICATION)
                     && ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i] )->attrs[ATTR_NOTE]
                     ==wxT( "Не устанавливается" ) ) )
                add_ena = FALSE;
        }
        else
        {
            if( pComponent->m_Variants_State!=COMP_IN_CONST_PART
                || ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[0] )->attrs[ATTR_NAME]==wxT( "" ) )
                add_ena = FALSE;

            if( (aPart_type & PARTTYPE_SPECIFICATION)
                && ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[0] )->attrs[ATTR_NOTE]==
                wxT( "Не устанавливается" ) )
                add_ena = FALSE;
        }

        if( aPart_type & PARTTYPE_SPECIFICATION )
        {
            if( ( (aPart_type & PARTTYPE_DOCUMENTATION)
                  && ( pComponent->ComponentType!=wxT( "Documentation" ) ) )
                || ( ( !(aPart_type & PARTTYPE_DOCUMENTATION) )
                     && ( pComponent->ComponentType==wxT( "Documentation" ) ) ) )
                add_ena = FALSE;

            if( ( (aPart_type & PARTTYPE_STANDARD_DETAILS)
                  && ( pComponent->ComponentType!=wxT( "StandardDetail" ) ) )
                || ( ( !(aPart_type & PARTTYPE_STANDARD_DETAILS) )
                     && ( pComponent->ComponentType==wxT( "StandardDetail" ) ) ) )
                add_ena = FALSE;

            if( ( (aPart_type & PARTTYPE_DETAILS) && ( pComponent->ComponentType!=wxT( "Detail" ) ) )
                || ( ( !(aPart_type & PARTTYPE_DETAILS) )
                     && ( pComponent->ComponentType==wxT( "Detail" ) ) ) )
                add_ena = FALSE;

            if( ( (aPart_type & PARTTYPE_ASSEMBLY_UNITS)
                  && ( pComponent->ComponentType!=wxT( "AssemblyUnit" ) ) )
                || ( ( !(aPart_type & PARTTYPE_ASSEMBLY_UNITS) )
                     && ( pComponent->ComponentType==wxT( "AssemblyUnit" ) ) ) )
                add_ena = FALSE;

            if( ( (aPart_type & PARTTYPE_GOODS) && ( pComponent->ComponentType!=wxT( "Goods" ) ) )
                || ( ( !(aPart_type & PARTTYPE_GOODS) )
                     && ( pComponent->ComponentType==wxT( "Goods" ) ) ) )
                add_ena = FALSE;
        }
        else if( pComponent->ExcludeCompsIndex )
            add_ena = FALSE;

        if( aPart_type & PARTTYPE_A_SET )
        {
            if( letter_digit_sets.GetCount() <= 1 || letter_digit_sets[0] != aSet_prefix )
                add_ena = FALSE;
        }
        else
        {
            if( letter_digit_sets.GetCount() != 1 )
                add_ena = FALSE;
        }

        if( add_ena )
            aResult->Add( pComponent );
    }
}

} // namespace GOST_DOC_GEN
