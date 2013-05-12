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
 * @file component.cpp
 */

#include <component.h>

namespace GOST_DOC_GEN {

COMPONENT::COMPONENT()
{
    m_Variants_State = COMP_IN_CONST_PART;

    for( int i = 0; i < ATTR_QTY; i++ )
    {
        m_KiCadAttrs[i].attr_changed  = false;
        m_KiCadAttrs[i].value_of_attr = wxT( "" );
    }

    ExcludeFromCompIndex = false;
    ComponentType = wxT( "" );
    fmt = wxT( "" );
    m_KiCadComponentPtr = NULL;
}


COMPONENT::~COMPONENT()
{
    int sz;

    sz = m_comp_attr_variants.GetCount();

    for( int n = 0; n < sz; n++ )
        delete (pTCOMPONENT_ATTRS) m_comp_attr_variants[n];
}


// returns -1 if tag is not find from given position of the string
// else returns tag start position
int FindVariantTagInStr( wxString   aStr,
                         int        aFrom_pos,
                         bool       aClose_tag,
                         wxString*  aTag,
                         int*       aTag_end_pos )
{
    int     i, str_len, tag_start_i;

    str_len = aStr.Len();

    if( str_len == 0 )
        return -1;

    i = aFrom_pos;

    while( aStr[i] != wxT( '<' )
           || (!aClose_tag && aStr[i + 1] == wxT( '/' ) )
           || (aClose_tag && aStr[i + 1] != wxT( '/' ) ) )
    {
        if( i >= str_len - 1 )
            return -1;

        i++;
    }

    tag_start_i = i++;

    while( aStr[i] != wxT( '>' ) )
    {
        if( i >= str_len - 1 )
            return -1;

        i++;
    }

    *aTag_end_pos = i;

    if( aClose_tag )
        *aTag = aStr.Mid( tag_start_i + 2, (*aTag_end_pos) - tag_start_i - 2 );
    else
        *aTag = aStr.Mid( tag_start_i + 1, (*aTag_end_pos) - tag_start_i - 1 );

    return tag_start_i;
}


// returns -1 if given string segment is not a variant
long DetermineVariant( wxString aStr )
{
    long res;
    int i, str_len;

    str_len = aStr.Len();

    if( str_len == 0 )
        return -1;

    for( i = 0; i < str_len; i++ )
        if( aStr[i] < wxT( '0' ) || aStr[i] > wxT( '9' ) )
            return -1;

    aStr.ToLong( &res, 10 );

    return res;
}


// returns -1 if given string has incorrect format
int COMPONENT::ReadVariantsInStr( wxString aStr, int aComp_attr, bool aInit )
{
    int pos, attr_variantsQty, str_len, open_variant, close_variant, tag_start_pos,
        open_tag_end_pos, close_tag_end_pos;
    int variant_ind;
    wxString tag;
    TCOMPONENT_ATTRS* component_attrs;

    str_len = aStr.Len();

    if( str_len == 0 )
        return 0;

    attr_variantsQty = 0;
    // if (aInit) m_Variants_Qty = 0;
    pos = 0;

    do
    {
        do
        {
            if( ( tag_start_pos =
                      FindVariantTagInStr( aStr, pos, false, &tag, &open_tag_end_pos ) )==-1 )
                return attr_variantsQty;

            if( open_tag_end_pos >= str_len - 1 )
                return attr_variantsQty;

            open_variant = DetermineVariant( tag );
            pos = open_tag_end_pos + 1;
        } while( open_variant==-1 );

        if( ( tag_start_pos =
                  FindVariantTagInStr( aStr, pos, true, &tag, &close_tag_end_pos ) )==-1 )
            return -1;

        close_variant = DetermineVariant( tag );

        if( open_variant!=close_variant )
            return -1;

        pos = close_tag_end_pos + 1;
        attr_variantsQty++;

        if( aInit )
        {
            component_attrs = new TCOMPONENT_ATTRS;
            component_attrs->attrs[ATTR_NAME] = aStr.Mid( open_tag_end_pos + 1,
                                                          tag_start_pos - open_tag_end_pos - 1 );
            for( int i = ATTR_TYPE; i < ATTR_QTY; i++ )
                component_attrs->attrs[i] = wxT( "" );

            component_attrs->variant        = open_variant;
            m_comp_attr_variants.Add( component_attrs );
        }
        else
        {
            variant_ind = GetVariant( open_variant, NULL );

            ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[variant_ind] )->attrs[aComp_attr] =
                aStr.Mid( open_tag_end_pos + 1, tag_start_pos - open_tag_end_pos - 1 );
        }
    } while( pos < str_len );

    return attr_variantsQty;
}


void COMPONENT::Update()
{
    size_t i;

    if( m_comp_attr_variants.GetCount()==1
        && ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->variant==-1 )
        m_Variants_State = COMP_IN_CONST_PART;
    else if( m_comp_attr_variants.GetCount()==1 )
        m_Variants_State = COMP_IN_VAR_PART;
    else
    {
        m_Variants_State = COMP_IN_VAR_PART;

        for( i = 1; i<m_comp_attr_variants.GetCount(); i++ )
        {
            if( ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_NAME]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_NAME]
                || ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_TYPE]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_TYPE]
                || ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_SUBTYPE]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_SUBTYPE]
                || ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_VALUE]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_VALUE]
                || ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_PRECISION]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_PRECISION]
                || ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_NOTE]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_NOTE]
                || ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_DESIGNATION]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_DESIGNATION]
                || ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[ATTR_MANUFACTURER]!=
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_MANUFACTURER] )
            {
                m_Variants_State = COMP_IN_VAR_PART_DIFF;
                break;
            }
        }
    }
}


// returns false if KiCad attributes have incorrect format
bool COMPONENT::ReadVariants()
{
    wxString            str, Type;
    TCOMPONENT_ATTRS*    component_attrs;
    int err;
    size_t              i;

    if( m_KiCadAttrs[ATTR_TYPE1].value_of_attr == wxT( "" ) )
        Type = m_KiCadAttrs[ATTR_TYPE].value_of_attr;
    else
        Type = m_KiCadAttrs[ATTR_TYPE1].value_of_attr;

    err = ReadVariantsInStr( m_KiCadAttrs[ATTR_NAME].value_of_attr, ATTR_NAME, true );

    if( err==-1 )
        return false;
    else if( err==0 )
    {
        component_attrs = new TCOMPONENT_ATTRS;

        for( int attr = 0; attr < ATTR_QTY; attr++ )
            component_attrs->attrs[attr] = m_KiCadAttrs[attr].value_of_attr;

        component_attrs->attrs[ATTR_TYPE] = Type;
        component_attrs->variant        = -1;
        m_comp_attr_variants.Add( component_attrs );
        m_Variants_State = COMP_IN_CONST_PART;
        return true;
    }

    m_Variants_State = COMP_IN_VAR_PART;

    err = ReadVariantsInStr( Type, ATTR_TYPE, false );

    if( err==-1 )
        return false;
    else if( err==0 )
        for( i = 0; i<m_comp_attr_variants.GetCount(); i++ )
            ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_TYPE] = Type;


    for( int attr = ATTR_SUBTYPE; attr <= ATTR_MANUFACTURER; attr++ )
    {
        err = ReadVariantsInStr( m_KiCadAttrs[attr].value_of_attr, attr, false );

        if( err==-1 )
            return false;
        else if( err==0 )
            for( i = 0; i<m_comp_attr_variants.GetCount(); i++ )
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[attr] =
                    m_KiCadAttrs[attr].value_of_attr;
    }

    Update();

    return true;
}


// returns -1 if given variant is not found
int COMPONENT::GetVariant( int aVariant, TCOMPONENT_ATTRS* aVariant_attrs )
{
    size_t i;

    if( m_Variants_State==COMP_IN_CONST_PART )
        return -1;

    for( i = 0; i < m_comp_attr_variants.GetCount(); i++ )
    {
        if( ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->variant == aVariant )
        {
            if( aVariant_attrs )
            {
                for( int attr = 0; attr < ATTR_QTY; attr++ )
                    aVariant_attrs->attrs[attr] =
                        ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[attr];

                aVariant_attrs->variant =
                    ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->variant;
            }

            return i;
        }
    }

    return -1;
}


void COMPONENT::FormKiCadAttribute( wxString aVariant_str, wxString aValue, wxString* aDest_string )
{
    (*aDest_string)  += wxT( "<" );
    (*aDest_string)  += aVariant_str;
    (*aDest_string)  += wxT( ">" );
    (*aDest_string)  += aValue;
    (*aDest_string)  += wxT( "</" );
    (*aDest_string)  += aVariant_str;
    (*aDest_string)  += wxT( ">" );
}


// returns true if some changes were done
bool COMPONENT::WriteVariants()
{
    TCOMPONENT_ATTRS component_attrs;
    wxString         variant_str;
    size_t           i;
    int              attr;
    bool             same_attrs[ATTR_QTY] = { false };
    bool             some_changes_done = false;

    if( m_Variants_State==COMP_IN_CONST_PART )
    {
        for( attr = 0; attr < ATTR_QTY; attr++ )
            component_attrs.attrs[attr] =
                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[attr];
    }
    else
    {
        for( attr = 0; attr < ATTR_QTY; attr++)
            same_attrs[attr] = true;

        for( i = 0; i < m_comp_attr_variants.GetCount(); i++ )
        {
            for( attr = ATTR_TYPE; attr < ATTR_QTY; attr++ )
                if( ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[attr]!=
                    ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[attr] )
                    same_attrs[attr] = false;
        }

        for( attr = 0; attr < ATTR_QTY; attr++ )
            component_attrs.attrs[attr] = wxT( "" );

        for( i = 0; i<m_comp_attr_variants.GetCount(); i++ )
        {
            variant_str =
                wxString::Format( wxT( "%02d" ),
                                  ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->variant );

            FormKiCadAttribute( variant_str,
                                ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_NAME],
                                &component_attrs.attrs[ATTR_NAME] );

            if( !same_attrs[ATTR_TYPE] )
                FormKiCadAttribute( variant_str,
                                    ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[ATTR_TYPE],
                                    &component_attrs.attrs[ATTR_TYPE] );

            for( attr = ATTR_SUBTYPE; attr < ATTR_QTY; attr++ )
                if( !same_attrs[attr] )
                    FormKiCadAttribute( variant_str,
                                        ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[i] )->attrs[attr],
                                        &component_attrs.attrs[attr] );
        }

        for( attr = ATTR_TYPE; attr < ATTR_QTY; attr++ )
            if( same_attrs[attr] )
                component_attrs.attrs[attr] =
                    ( (pTCOMPONENT_ATTRS) m_comp_attr_variants[0] )->attrs[attr];
    }

    if( m_KiCadAttrs[ATTR_NAME].value_of_attr!=component_attrs.attrs[ATTR_NAME] )
    {
        m_KiCadAttrs[ATTR_NAME].value_of_attr = component_attrs.attrs[ATTR_NAME];
        m_KiCadAttrs[ATTR_NAME].attr_changed  = true;
        some_changes_done = true;
    }

    if( m_KiCadAttrs[ATTR_TYPE].value_of_attr==component_attrs.attrs[ATTR_TYPE]
        && m_KiCadAttrs[ATTR_TYPE1].value_of_attr != wxT( "" ) )
    {
        m_KiCadAttrs[ATTR_TYPE1].value_of_attr = wxT( "" );
        m_KiCadAttrs[ATTR_TYPE1].attr_changed  = true;
        some_changes_done = true;
    }
    else if( m_KiCadAttrs[ATTR_TYPE].value_of_attr!=component_attrs.attrs[ATTR_TYPE]
             && m_KiCadAttrs[ATTR_TYPE1].value_of_attr!=component_attrs.attrs[ATTR_TYPE] )
    {
        m_KiCadAttrs[ATTR_TYPE1].value_of_attr = component_attrs.attrs[ATTR_TYPE];
        m_KiCadAttrs[ATTR_TYPE1].attr_changed  = true;
        some_changes_done = true;
    }

    for( attr = ATTR_SUBTYPE; attr < ATTR_QTY; attr++ )
        if( m_KiCadAttrs[attr].value_of_attr!=component_attrs.attrs[attr] )
        {
            m_KiCadAttrs[attr].value_of_attr = component_attrs.attrs[attr];
            m_KiCadAttrs[attr].attr_changed = true;
            some_changes_done = true;
        }

    return some_changes_done;
}


int ConvertVariantToItemVarI( COMPONENT* aComponent, int aVariant )
{
    if( aVariant==-1 )
        return 0;
    else
        return aComponent->GetVariant( aVariant, NULL );
}


// returns true if both components have identical attributes except ref des
bool CompareComps( COMPONENT* aComp1, COMPONENT* aComp2, int aVariant )
{
    int comp1_var_i = ConvertVariantToItemVarI( aComp1, aVariant );
    int comp2_var_i = ConvertVariantToItemVarI( aComp2, aVariant );

    for( int i = 0; i < ATTR_QTY; i++ )
    {
        // skip comparison of ATTR_TYPE1
        if( i==ATTR_TYPE1)
            continue;

        if( ( (pTCOMPONENT_ATTRS) aComp1->m_comp_attr_variants[comp1_var_i] )->attrs[i]!=
            ( (pTCOMPONENT_ATTRS) aComp2->m_comp_attr_variants[comp2_var_i] )->attrs[i] )
            return false;
    }

    return true;
}

} // namespace GOST_DOC_GEN
