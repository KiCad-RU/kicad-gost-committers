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

#include <wxEeschemaStruct.h>
#include <general.h>
#include <sch_sheet.h>
#include <template_fieldnames.h>

#include <component_db.h>
#include <common_doc_iface.h>

#if defined(USE_RPC_DOC_SERVER)
#include <rpc_doc_iface.h>
#elif defined(USE_OPENOFFICE_SDK)
#include <oo_iface.hxx>
#else
#include <oo_python_uno_iface.h>
#endif

#include <doc_component_index.h>
#include <doc_specification.h>

namespace GOST_DOC_GEN {

COMPONENT_DB::COMPONENT_DB()
{
    m_designName         = wxT( "" );
    m_designation        = wxT( "" );
    m_specFirstUse       = wxT( "" );
    m_PCBDesignation     = wxT( "" );
    m_circuitDrawingFmt  = wxT( "" );
    m_assemblyDrawingFmt = wxT( "" );
    m_developerField     = wxT( "" );
    m_verifierField      = wxT( "" );
    m_approverField      = wxT( "" );
    m_dbgEna             = false;
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
    wxString temp_str;

    dig_subset_ofs = 0;

    while( true )
    {
        str_len = (*aStr).Len();

        if( dig_subset_ofs >= str_len )
            break;

        temp_str = *(aStr);
        pos = FindOneOf( temp_str.Mid( dig_subset_ofs ), wxT( "0123456789" ) );
        if( pos == wxNOT_FOUND ) pos = temp_str.Len() - dig_subset_ofs;
        pos += dig_subset_ofs;
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


void COMPONENT_DB::GenerateComponentIndexDoc()
{
#if defined(USE_RPC_DOC_SERVER)
    COMMON_DOC_IFACE* docIface = new RPC_DOC_IFACE( m_dbgEna );
#elif defined(USE_OPENOFFICE_SDK)
    COMMON_DOC_IFACE* docIface = new OO_IFACE();
#else
    COMMON_DOC_IFACE* docIface = new OO_PYTHON_UNO_IFACE();
#endif

    CreateNewCompIndexDoc( this, docIface );

    delete docIface;
}


void COMPONENT_DB::GenerateSpecificationDoc()
{
#if defined(USE_RPC_DOC_SERVER)
    COMMON_DOC_IFACE* docIface = new RPC_DOC_IFACE( m_dbgEna );
#elif defined(USE_OPENOFFICE_SDK)
    COMMON_DOC_IFACE* docIface = new OO_IFACE();
#else
    COMMON_DOC_IFACE* docIface = new OO_PYTHON_UNO_IFACE();
#endif

    CreateNewSpecificationDoc( this, docIface );

    delete docIface;
}


void COMPONENT_DB::SetDbgMode( bool aDbgEna )
{
    m_dbgEna = aDbgEna;
}


// returns false if given variant is not found
bool COMPONENT_DB::FindVariant( int aVariant )
{
    size_t i;

    for( i = 0; i < m_variantIndexes.GetCount(); i++ )
    {
        if( m_variantIndexes[i] == aVariant )
            return true;
    }

    return false;
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
            for( var_i = 0; var_i < pComponent->m_comp_attr_variants.GetCount(); var_i++ )
            {
                variant = ( (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[var_i] )->variant;

                if( variant!=-1 && !FindVariant( variant ) )
                    m_variantIndexes.Add( variant );
            }
        }
    }

    SortCByteArray( &m_variantIndexes );
}


// returns true if some changes were done
bool COMPONENT_DB::WriteVariants()
{
    size_t item;
    bool some_changes_done = false;

    for ( item = 0; item < m_AllComponents.GetCount(); item++ )
    {
        if ( ( (COMPONENT *)m_AllComponents[item] )->WriteVariants() )
            some_changes_done = true;
    }

    return some_changes_done;
}


void COMPONENT_DB::AddNewVariant( int aVariant )
{
    m_variantIndexes.Add( aVariant );
    SortCByteArray( &m_variantIndexes );
}


void COMPONENT_DB::LoadFromKiCad()
{
    SCH_SHEET_LIST  sheetList;
    wxString        str;
    COMPONENT*      pComp;
    SCH_FIELD*      pSch_field;

    sheetList.GetComponents( m_cmplist, false );

    unsigned int index = 0;

    TITLE_BLOCK tb   = g_RootSheet->GetScreen()->GetTitleBlock();
    m_designName     = tb.GetTitle();
    m_designation    = tb.GetComment1();
    m_developerField = tb.GetComment2();
    m_verifierField  = tb.GetComment3();
    m_approverField  = tb.GetComment4();

    while( index < m_cmplist.GetCount() )
    {
        SCH_COMPONENT* component = m_cmplist[index].GetComponent();

        str = m_cmplist[index].GetRef();
        ZeroInserting( &str );

        pComp = new COMPONENT();
        pComp->m_KiCadComponentPtr = component;
        pComp->m_RefDes = m_cmplist[index].GetRef();
        pComp->m_SortingRefDes = str;

        pComp->m_KiCadAttrs[ATTR_TYPE].value_of_attr  = component->GetLibName();

        if( component->GetField( VALUE )->GetText() == wxT( "~" ) )
            pComp->m_KiCadAttrs[ATTR_VALUE].value_of_attr = wxEmptyString;
        else
            pComp->m_KiCadAttrs[ATTR_VALUE].value_of_attr = component->GetField( VALUE )->GetText();

        if ( ( pSch_field = component->FindField( wxT( "Title" ) ) ) )
            pComp->m_KiCadAttrs[ATTR_NAME].value_of_attr = pSch_field->GetText();

        if ( ( pSch_field = component->FindField( wxT( "Type" ) ) ) )
            pComp->m_KiCadAttrs[ATTR_TYPE1].value_of_attr = pSch_field->GetText();

        if ( ( pSch_field = component->FindField( wxT( "SType" ) ) ) )
            pComp->m_KiCadAttrs[ATTR_SUBTYPE].value_of_attr = pSch_field->GetText();

        if ( ( pSch_field = component->FindField( wxT( "Precision" ) ) ) )
            pComp->m_KiCadAttrs[ATTR_PRECISION].value_of_attr = pSch_field->GetText();

        if ( ( pSch_field = component->FindField( wxT( "Note" ) ) ) )
            pComp->m_KiCadAttrs[ATTR_NOTE].value_of_attr = pSch_field->GetText();

        if ( ( pSch_field = component->FindField( wxT( "Designation" ) ) ) )
            pComp->m_KiCadAttrs[ATTR_DESIGNATION].value_of_attr = pSch_field->GetText();

        if ( ( pSch_field = component->FindField( wxT( "Manufacturer" ) ) ) )
            pComp->m_KiCadAttrs[ATTR_MANUFACTURER].value_of_attr = pSch_field->GetText();

        m_AllComponents.Add( pComp );
        index++;
    }

    SortComponents();

    ReadVariants();
}


void COMPONENT_DB::WriteAttributeBackToKiCad( COMPONENT* aComp,
                                              int aAttrIndex,
                                              wxString aAttrName )
{
    SCH_FIELD*  pSch_field;

    if( aComp->m_KiCadAttrs[aAttrIndex].attr_changed )
    {
        if ( ( pSch_field = aComp->m_KiCadComponentPtr->FindField( aAttrName ) ) )
            pSch_field->SetText( aComp->m_KiCadAttrs[aAttrIndex].value_of_attr );
        else
        {
            // the field does not exist then create it
            SCH_FIELD field( wxPoint( 0, 0 ),
                             -1, // field id is not relavant for user defined fields
                             aComp->m_KiCadComponentPtr,
                             aAttrName );

            field.SetText( aComp->m_KiCadAttrs[aAttrIndex].value_of_attr );
            aComp->m_KiCadComponentPtr->AddField( field );
        }
    }
}


bool COMPONENT_DB::WriteBackToKiCad()
{
    size_t      item;
    COMPONENT*  pComp;

    bool modified = WriteVariants();

    for( item = 0; item < m_AllComponents.GetCount(); item++ )
    {
        pComp = m_AllComponents[item];

        // Save Value attribute
        if( pComp->m_KiCadAttrs[ATTR_VALUE].attr_changed )
            pComp->m_KiCadComponentPtr->GetField( VALUE )->SetText(
                pComp->m_KiCadAttrs[ATTR_VALUE].value_of_attr );

        // Save the other attributes
        WriteAttributeBackToKiCad( pComp, ATTR_NAME,         wxT( "Title" ) );
        WriteAttributeBackToKiCad( pComp, ATTR_TYPE1,        wxT( "Type" ) );
        WriteAttributeBackToKiCad( pComp, ATTR_SUBTYPE,      wxT( "SType" ) );
        WriteAttributeBackToKiCad( pComp, ATTR_PRECISION,    wxT( "Precision" ) );
        WriteAttributeBackToKiCad( pComp, ATTR_NOTE,         wxT( "Note" ) );
        WriteAttributeBackToKiCad( pComp, ATTR_DESIGNATION,  wxT( "Designation" ) );
        WriteAttributeBackToKiCad( pComp, ATTR_MANUFACTURER, wxT( "Manufacturer" ) );
    }

    return modified;
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
    COMPONENT*         pComponent;
    pTCOMPONENT_ATTRS  comp_attrs;
    wxArrayString      letter_digit_sets;
    size_t             i;
    int                set_type_i, item_var_i;
    bool               analysis_ena;

    if( !aAppend )
        aResult->Clear();

    for( i = 0; i < aComponents->GetCount(); i++ )
    {
        pComponent = (COMPONENT*) (*aComponents)[i];

        analysis_ena = true;

        if( aPart_type & PARTTYPE_VAR )
        {
            item_var_i = pComponent->GetVariant( aVariant, NULL );

            if( pComponent->m_Variants_State==COMP_IN_CONST_PART || item_var_i==-1 )
                analysis_ena = false;
            else
            {
                comp_attrs = (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[item_var_i];

                if( comp_attrs->attrs[ATTR_NAME]==wxT( "" )
                    || ( ( aPart_type & PARTTYPE_SPECIFICATION )
                         && ( comp_attrs->attrs[ATTR_NOTE]==wxT( "Не устанавливается" )
                              || comp_attrs->attrs[ATTR_NOTE]==wxT( "Not installed" )
                            ) ) )
                {
                    analysis_ena = false;
                }
            }
        }
        else
        {
            if( pComponent->m_Variants_State!=COMP_IN_CONST_PART )
                analysis_ena = false;
            else
            {
                comp_attrs = (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[0];

                if( comp_attrs->attrs[ATTR_NAME]==wxT( "" ) )
                    analysis_ena = false;

                if( ( aPart_type & PARTTYPE_SPECIFICATION )
                    && ( comp_attrs->attrs[ATTR_NOTE]==wxT( "Не устанавливается" )
                         || comp_attrs->attrs[ATTR_NOTE]==wxT( "Not installed" ) ) )
                {
                    analysis_ena = false;
                }
            }
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
    size_t             i;
    int                item_var_i;
    COMPONENT*         pComponent;
    pTCOMPONENT_ATTRS  comp_attrs;
    wxArrayString      letter_digit_sets;
    bool               add_ena;

    aResult->Clear();

    for( i = 0; i < m_AllComponents.GetCount(); i++ )
    {
        pComponent = (COMPONENT*) m_AllComponents[i];
        ExtractLetterDigitSets( pComponent->m_RefDes, &letter_digit_sets );

        add_ena = true;

        if( aPart_type & PARTTYPE_VAR )
        {
            item_var_i = pComponent->GetVariant( aVariant, NULL );

            if( pComponent->m_Variants_State==COMP_IN_CONST_PART || item_var_i==-1 )
                add_ena = false;
            else
            {
                comp_attrs = (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i];

                if( comp_attrs->attrs[ATTR_NAME]==wxT( "" )
                    || ( (aPart_type & PARTTYPE_SPECIFICATION)
                         && ( comp_attrs->attrs[ATTR_NOTE]==wxT( "Не устанавливается" )
                              || comp_attrs->attrs[ATTR_NOTE]==wxT( "Not installed" )
                            ) ) )
                {
                    add_ena = false;
                }
            }
        }
        else
        {
            if( pComponent->m_Variants_State!=COMP_IN_CONST_PART )
                add_ena = false;
            else
            {
                comp_attrs = (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[0];

                if( comp_attrs->attrs[ATTR_NAME]==wxT( "" ) )
                    add_ena = false;

                if( (aPart_type & PARTTYPE_SPECIFICATION)
                    && ( comp_attrs->attrs[ATTR_NOTE]==wxT( "Не устанавливается" )
                         || comp_attrs->attrs[ATTR_NOTE]==wxT( "Not installed" ) ) )
                {
                    add_ena = false;
                }
            }
        }

        if( aPart_type & PARTTYPE_SPECIFICATION )
        {
            if( ( (aPart_type & PARTTYPE_DOCUMENTATION)
                  && ( pComponent->ComponentType!=wxT( "Documentation" ) ) )
                || ( ( !(aPart_type & PARTTYPE_DOCUMENTATION) )
                     && ( pComponent->ComponentType==wxT( "Documentation" ) ) ) )
                add_ena = false;

            if( ( (aPart_type & PARTTYPE_STANDARD_DETAILS)
                  && ( pComponent->ComponentType!=wxT( "StandardDetail" ) ) )
                || ( ( !(aPart_type & PARTTYPE_STANDARD_DETAILS) )
                     && ( pComponent->ComponentType==wxT( "StandardDetail" ) ) ) )
                add_ena = false;

            if( ( (aPart_type & PARTTYPE_DETAILS) && ( pComponent->ComponentType!=wxT( "Detail" ) ) )
                || ( ( !(aPart_type & PARTTYPE_DETAILS) )
                     && ( pComponent->ComponentType==wxT( "Detail" ) ) ) )
                add_ena = false;

            if( ( (aPart_type & PARTTYPE_ASSEMBLY_UNITS)
                  && ( pComponent->ComponentType!=wxT( "AssemblyUnit" ) ) )
                || ( ( !(aPart_type & PARTTYPE_ASSEMBLY_UNITS) )
                     && ( pComponent->ComponentType==wxT( "AssemblyUnit" ) ) ) )
                add_ena = false;

            if( ( (aPart_type & PARTTYPE_GOODS) && ( pComponent->ComponentType!=wxT( "Goods" ) ) )
                || ( ( !(aPart_type & PARTTYPE_GOODS) )
                     && ( pComponent->ComponentType==wxT( "Goods" ) ) ) )
                add_ena = false;
        }
        else if( pComponent->ExcludeFromCompIndex )
            add_ena = false;

        if( aPart_type & PARTTYPE_A_SET )
        {
            if( letter_digit_sets.GetCount() <= 1 || letter_digit_sets[0] != aSet_prefix )
                add_ena = false;
        }
        else
        {
            if( letter_digit_sets.GetCount() != 1 )
                add_ena = false;
        }

        if( add_ena )
            aResult->Add( pComponent );
    }
}

} // namespace GOST_DOC_GEN
