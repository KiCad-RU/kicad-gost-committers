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
 * @file GOST_comp_manager.cpp
 */

#include <GOST_comp_manager.h>
#include <component_db.h>

using namespace GOST_DOC_GEN;

GOST_COMP_MANAGER::GOST_COMP_MANAGER( wxWindow* parent ) :
    FRAME_GOST_DOC_GEN_BASE( parent )
{
    m_schEditFrame = (SCH_EDIT_FRAME *)parent;
    m_onEditChangeComboBoxLock = false;
    m_onItemChangedCheckListCtrlLock = false;

    m_componentDB = new COMPONENT_DB();

    wxSize listClientSize = m_listCtrl->GetClientSize();
    m_listCtrl->InsertColumn( TABLE_REFDES_COL,
                              _( "Reference" ),
                              wxLIST_FORMAT_LEFT,
                              listClientSize.GetWidth() * 1/4 );
    m_listCtrl->InsertColumn( TABLE_NAME_COL,
                              _( "Name" ),
                              wxLIST_FORMAT_LEFT,
                              listClientSize.GetWidth() * 1/4 );
    m_listCtrl->InsertColumn( TABLE_NOTE_COL,
                              _( "Note" ),
                              wxLIST_FORMAT_LEFT,
                              listClientSize.GetWidth() * 1/4 );
    m_listCtrl->InsertColumn( TABLE_FORVARIANTS_COL,
                              _( "For variants" ),
                              wxLIST_FORMAT_LEFT,
                              listClientSize.GetWidth() * 1/4 );

    m_componentDB->LoadFromKiCad();

    FormVariantList();
    DisableComboBoxes();

    wxCommandEvent dummyEvent;
    OnRadioFullList( dummyEvent );
}


GOST_COMP_MANAGER::~GOST_COMP_MANAGER()
{
    delete m_componentDB;
    m_componentDB = NULL;
}


void GOST_COMP_MANAGER::DisableComboBoxes()
{
    m_combo_Name->Enable( false );
    m_combo_Type->Enable( false );
    m_combo_SubType->Enable( false );
    m_combo_Value->Enable( false );
    m_combo_Precision->Enable( false );
    m_combo_Note->Enable( false );
    m_combo_Designation->Enable( false );
    m_combo_Manufacturer->Enable( false);

    m_onEditChangeComboBoxLock = true;
    m_combo_Name->SetValue( wxEmptyString );
    m_combo_Type->SetValue( wxEmptyString );
    m_combo_SubType->SetValue( wxEmptyString );
    m_combo_Value->SetValue( wxEmptyString );
    m_combo_Precision->SetValue( wxEmptyString );
    m_combo_Note->SetValue( wxEmptyString );
    m_combo_Designation->SetValue( wxEmptyString );
    m_combo_Manufacturer->SetValue( wxEmptyString );
    m_onEditChangeComboBoxLock = false;
    m_staticBoxEditing->GetStaticBox()->SetLabel( _( "Editing" ) );
}


void GOST_COMP_MANAGER::FormCheckListBoxContent( int aDBitem )
{
    TCOMPONENT_ATTRS* pVariant_attrs;
    wxString str;
    size_t i;
    int variant_ind;
    bool first_string_printed = false;
    bool first_string_preprinted = false;
    COMPONENT *pComponent;

    m_onItemChangedCheckListCtrlLock = true;

    pComponent = (COMPONENT *)m_componentDB->m_AllComponents[aDBitem];

    if( m_radio_FullList->GetValue() )
    {
        for( i = 0; i < m_componentDB->m_variantIndexes.GetCount(); i++ )
        {
            str = wxString::Format( wxT( "%02d" ), m_componentDB->m_variantIndexes[i] );

            variant_ind = pComponent->GetVariant( m_componentDB->m_variantIndexes[i], NULL );
            if( variant_ind != -1 &&
                ( pComponent->m_Variants_State==COMP_IN_VAR_PART_DIFF ||
                ( !first_string_printed && pComponent->m_Variants_State==COMP_IN_VAR_PART ) ) )
            {
                pVariant_attrs = (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[variant_ind];
                str += wxT( ": " ) + pVariant_attrs->attrs[ATTR_NAME] + wxT( " " );
                str += pVariant_attrs->attrs[ATTR_TYPE];
                str += pVariant_attrs->attrs[ATTR_SUBTYPE];
                str += wxT( " " ) + pVariant_attrs->attrs[ATTR_VALUE];
                str += wxT( " " ) + pVariant_attrs->attrs[ATTR_PRECISION];
                str += wxT( " " ) + pVariant_attrs->attrs[ATTR_DESIGNATION];
                str += wxT( " " ) + pVariant_attrs->attrs[ATTR_MANUFACTURER];

                if( pVariant_attrs->attrs[ATTR_NOTE] != wxT( "" ) )
                    str += _( " note: " ) + pVariant_attrs->attrs[ATTR_NOTE];

                first_string_preprinted = true;
            }

            if( variant_ind != -1
                && pComponent->m_Variants_State==COMP_IN_VAR_PART && first_string_printed )
            {
                str += wxT( " ---//---" );
            }

            if( first_string_preprinted )
                first_string_printed = true;

            m_checkListCtrl->SetString( i, str );

            m_checkListCtrl->Check( i, false );
            if( pComponent->m_Variants_State != COMP_IN_CONST_PART && variant_ind != -1 )
                m_checkListCtrl->Check( i, true );
        }
    }

    m_onItemChangedCheckListCtrlLock = false;
}


void GOST_COMP_MANAGER::OnClickListCtrl( wxListEvent& event )
{
    long item;
    int variant;
    wxString str;

    COMPONENT* pComponent;
    TCOMPONENT_ATTRS* pVariant_attrs;

    item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    // if no component is selected then do nothing
    if( item == -1 )
        return;

    item = m_listCtrl->GetItemData( item );

    pComponent = (COMPONENT *)m_componentDB->m_AllComponents[item];

    if( m_radio_FullList->GetValue() && pComponent->m_Variants_State==COMP_IN_VAR_PART_DIFF )
    {
        DisableComboBoxes();
    }
    else
    {
        m_combo_Name->Enable( true );
        m_combo_Type->Enable( true );
        m_combo_SubType->Enable( true );
        m_combo_Value->Enable( true );
        m_combo_Precision->Enable( true );
        m_combo_Note->Enable( true );
        m_combo_Designation->Enable( true );
        m_combo_Manufacturer->Enable( true );
    }

    if( m_radio_FullList->GetValue() )
    {
        m_checkListCtrl->Show( true );
        FormCheckListBoxContent( item );
        variant = 0;
    }
    else if( m_radio_ConstPart->GetValue() )
    {
        variant = 0;
    }
    else
    {
        variant = pComponent->GetVariant( m_componentDB->m_variantIndexes[m_combo_Variant->GetCurrentSelection()], NULL );
    }

    if(variant >= 0)
    {
        m_onEditChangeComboBoxLock = true;
        pVariant_attrs = (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[variant];
        m_combo_Name->SetValue( pVariant_attrs->attrs[ATTR_NAME] );
        m_combo_Type->SetValue( pVariant_attrs->attrs[ATTR_TYPE] );
        m_combo_SubType->SetValue( pVariant_attrs->attrs[ATTR_SUBTYPE] );
        m_combo_Value->SetValue( pVariant_attrs->attrs[ATTR_VALUE] );
        m_combo_Precision->SetValue( pVariant_attrs->attrs[ATTR_PRECISION] );
        m_combo_Designation->SetValue( pVariant_attrs->attrs[ATTR_DESIGNATION] );
        m_combo_Manufacturer->SetValue( pVariant_attrs->attrs[ATTR_MANUFACTURER] );
        m_combo_Note->SetValue( pVariant_attrs->attrs[ATTR_NOTE] );
        m_onEditChangeComboBoxLock = false;
        str = _( "Editing the component with Reference " );
        str += pComponent->m_RefDes;
        m_staticBoxEditing->GetStaticBox()->SetLabel( str );
    }
}


void GOST_COMP_MANAGER::OnItemChangedCheckListCtrl( wxCommandEvent& event )
{
    TCOMPONENT_ATTRS* pComp_attrs;
    TCOMPONENT_ATTRS* pMain_comp_attrs;
    int sel_variant_ind, comp_variant_ind, variant, sel_DBitem, sel_list_item;
    COMPONENT *pComponent;

    if( m_onItemChangedCheckListCtrlLock )
        return;

    sel_variant_ind = event.GetSelection();
    variant = m_componentDB->m_variantIndexes[sel_variant_ind];

    sel_list_item = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    while( sel_list_item >= 0 )
    {
        sel_DBitem = m_listCtrl->GetItemData( sel_list_item );

        pComponent = (COMPONENT *)m_componentDB->m_AllComponents[sel_DBitem];

        comp_variant_ind = pComponent->GetVariant( variant, NULL );

        if( ( ( comp_variant_ind != -1 ) && ( !m_checkListCtrl->IsChecked( sel_variant_ind ) ) )
            || ( ( comp_variant_ind == -1 ) && m_checkListCtrl->IsChecked( sel_variant_ind ) ) )
        {
            pMain_comp_attrs = (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[0];

            if( m_checkListCtrl->IsChecked( sel_variant_ind ) )
            {
                if( pComponent->m_Variants_State==COMP_IN_CONST_PART )
                {
                    pComponent->m_Variants_State = COMP_IN_VAR_PART;
                    pMain_comp_attrs->variant = variant;
                }
                else
                {
                    pComp_attrs = new TCOMPONENT_ATTRS;
                    pComp_attrs->attrs[ATTR_NAME]         = pMain_comp_attrs->attrs[ATTR_NAME];
                    pComp_attrs->attrs[ATTR_TYPE]         = pMain_comp_attrs->attrs[ATTR_TYPE];
                    pComp_attrs->attrs[ATTR_SUBTYPE]      = pMain_comp_attrs->attrs[ATTR_SUBTYPE];
                    pComp_attrs->attrs[ATTR_VALUE]        = pMain_comp_attrs->attrs[ATTR_VALUE];
                    pComp_attrs->attrs[ATTR_PRECISION]    = pMain_comp_attrs->attrs[ATTR_PRECISION];
                    pComp_attrs->attrs[ATTR_NOTE]         = pMain_comp_attrs->attrs[ATTR_NOTE];
                    pComp_attrs->attrs[ATTR_DESIGNATION]  =
                        pMain_comp_attrs->attrs[ATTR_DESIGNATION];
                    pComp_attrs->attrs[ATTR_MANUFACTURER] =
                        pMain_comp_attrs->attrs[ATTR_MANUFACTURER];
                    pComp_attrs->variant = variant;
                    pComponent->m_comp_attr_variants.Add( pComp_attrs );
                }
            }
            else
            {
                if( pComponent->m_comp_attr_variants.GetCount() == 1 )
                {
                    pComponent->m_Variants_State = COMP_IN_CONST_PART;
                    pMain_comp_attrs->variant = -1;
                }
                else
                {
                    delete pComponent->m_comp_attr_variants[comp_variant_ind];
                    pComponent->m_comp_attr_variants.RemoveAt( comp_variant_ind, 1 );
                    pComponent->Update();
                }
            }

            FormFullListString( sel_DBitem, sel_list_item, false );
            wxListEvent dummyEvent;
            OnClickListCtrl( dummyEvent );
            //FormCheckListBoxContent( sel_DBitem );
        }

        sel_list_item = m_listCtrl->GetNextItem( sel_list_item,
                                                 wxLIST_NEXT_ALL,
                                                 wxLIST_STATE_SELECTED );
    }
}


void GOST_COMP_MANAGER::FormFullListString( int aItem, int aList_str_ind, bool aNew_string )
{
    TCOMPONENT_ATTRS* pMain_comp_attrs;
    wxString str, variant_str;
    size_t variant;
    int actual_item_i, icon;

    COMPONENT *pComponent = m_componentDB->m_AllComponents[aItem];
    pMain_comp_attrs = (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[0];
    str = pMain_comp_attrs->attrs[ATTR_NAME] + wxT( " " );

    str += pMain_comp_attrs->attrs[ATTR_TYPE];

    if( pMain_comp_attrs->attrs[ATTR_SUBTYPE] == wxT( "" )
        && pMain_comp_attrs->attrs[ATTR_VALUE] != wxT( "" ) )
    {
        str += wxT( ' ' );
    }
    else
        str += pMain_comp_attrs->attrs[ATTR_SUBTYPE];

    str += pMain_comp_attrs->attrs[ATTR_VALUE];
    if( pMain_comp_attrs->attrs[ATTR_PRECISION] != wxT( "" ) )
    {
        str += wxT( " ±" ) + pMain_comp_attrs->attrs[ATTR_PRECISION];
    }

    str += pMain_comp_attrs->attrs[ATTR_DESIGNATION];

    if( pComponent->m_Variants_State == COMP_IN_VAR_PART_DIFF )
        str = wxT( "{{{" ) + str + wxT( "}}}" );

    icon = 1;
    if( pComponent->m_Variants_State == COMP_IN_CONST_PART )
    {
        if( pMain_comp_attrs->attrs[ATTR_NAME] == wxT( "" ) )
        {
            icon = 0;
        }
    }
    else
    {
        for( variant = 0; variant < pComponent->m_comp_attr_variants.GetCount(); variant++ )
            if( ( (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[variant])->attrs[ATTR_NAME] ==
                wxT( "" ) )
            {
                icon = 0;
            }
    }

    if( aNew_string )
        actual_item_i = m_listCtrl->InsertItem( aList_str_ind, pComponent->m_RefDes );
    else
        actual_item_i = aList_str_ind;

    m_listCtrl->SetItemData( actual_item_i, aItem );
    m_listCtrl->SetItemImage( actual_item_i, icon );
    m_listCtrl->SetItem( actual_item_i, TABLE_NAME_COL, str );

    str = wxT( "" );
    if( pMain_comp_attrs->attrs[ATTR_MANUFACTURER] != wxT( "" ) )
    {
        str = wxT( "\"" );
        str += pMain_comp_attrs->attrs[ATTR_MANUFACTURER];
        str += wxT( "\" " );
    }

    str += pMain_comp_attrs->attrs[ATTR_NOTE];
    m_listCtrl->SetItem( actual_item_i, TABLE_NOTE_COL, str );

    str = wxT( "" );
    if( pComponent->m_Variants_State != COMP_IN_CONST_PART )
    {
        for( variant = 0; variant < pComponent->m_comp_attr_variants.GetCount(); variant++ )
        {
            if( variant > 0 )
                str += wxT( ", " );

            variant_str = wxString::Format( wxT( "%02d" ),
                ( (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[variant] )->variant );

            str += variant_str;
        }
    }

    m_listCtrl->SetItem( actual_item_i, TABLE_FORVARIANTS_COL, str );
}


void GOST_COMP_MANAGER::OnRadioFullList( wxCommandEvent& event )
{
    size_t DBitem;

    DisableComboBoxes();
    m_listCtrl->DeleteAllItems();
    m_combo_Variant->Enable( false );

    for( DBitem = 0; DBitem < m_componentDB->m_AllComponents.GetCount(); DBitem++ )
    {
        FormFullListString( DBitem, DBitem, true );
    }
}


void GOST_COMP_MANAGER::FormConstPartString( int aItem, int aList_str_ind, bool aNew_string )
{
    TCOMPONENT_ATTRS* pMain_comp_attrs;
    wxString str;
    int actual_item_i, icon;
    COMPONENT* pComponent = (COMPONENT *)m_componentDB->m_AllComponents[aItem];
    pMain_comp_attrs = (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[0];

    str = pMain_comp_attrs->attrs[ATTR_NAME] + wxT( " " );
    str += pMain_comp_attrs->attrs[ATTR_TYPE];

    if( pMain_comp_attrs->attrs[ATTR_SUBTYPE] == wxT( "" )
        && pMain_comp_attrs->attrs[ATTR_VALUE] != wxT( "" ) )
        str += wxT( ' ' );
    else
        str += pMain_comp_attrs->attrs[ATTR_SUBTYPE];

    str += pMain_comp_attrs->attrs[ATTR_VALUE];

    if( pMain_comp_attrs->attrs[ATTR_PRECISION] != wxT( "" ) )
        str += wxT( " ±" ) + pMain_comp_attrs->attrs[ATTR_PRECISION];

    str += pMain_comp_attrs->attrs[ATTR_DESIGNATION];

    if( pMain_comp_attrs->attrs[ATTR_NAME] == wxT( "" ) )
        icon = 0;
    else
        icon = 1;

    if( aNew_string )
        actual_item_i = m_listCtrl->InsertItem( aList_str_ind, pComponent->m_RefDes );
    else
        actual_item_i = aList_str_ind;

    m_listCtrl->SetItemData( actual_item_i, aItem );
    m_listCtrl->SetItemImage( actual_item_i, icon );
    m_listCtrl->SetItem( actual_item_i, TABLE_NAME_COL, str );

    str = wxT( "" );
    if( pMain_comp_attrs->attrs[ATTR_MANUFACTURER] != wxT( "" ) )
    {
        str = wxT( "\"" );
        str += pMain_comp_attrs->attrs[ATTR_MANUFACTURER];
        str += wxT( "\" " );
    }

    str += pMain_comp_attrs->attrs[ATTR_NOTE];
    m_listCtrl->SetItem( actual_item_i, TABLE_NOTE_COL, str );
}


void GOST_COMP_MANAGER::OnRadioConstPart( wxCommandEvent& event )
{
    size_t DBitem;
    int list_str_ind;

    DisableComboBoxes();
    m_listCtrl->DeleteAllItems();
    m_combo_Variant->Enable( false );
    m_checkListCtrl->Show( false );

    list_str_ind = 0;
    for( DBitem = 0; DBitem < m_componentDB->m_AllComponents.GetCount(); DBitem++ )
    {
        if( ( (COMPONENT *)m_componentDB->m_AllComponents[DBitem] )->m_Variants_State ==
            COMP_IN_CONST_PART
            && ( (COMPONENT *)m_componentDB->m_AllComponents[DBitem]
               )->m_comp_attr_variants.GetCount() > 0 )
        {
            FormConstPartString( DBitem, list_str_ind++, true );
        }
    }
}


void GOST_COMP_MANAGER::FormVarPartString( int aItem, int aList_str_ind, bool aNew_string )
{
    TCOMPONENT_ATTRS variant_attrs;
    wxString str;
    int actual_item_i, icon, item_var_i;
    COMPONENT *pComponent = (COMPONENT *)m_componentDB->m_AllComponents[aItem];

    if( ( item_var_i = pComponent->GetVariant(
            m_componentDB->m_variantIndexes[m_combo_Variant->GetCurrentSelection()],
            &variant_attrs ) ) != -1 )
    {
        if( ( (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[item_var_i]
            )->attrs[ATTR_NAME] == wxT( "" ) )
            icon = 0;
        else
            icon = 1;

        if( aNew_string )
            actual_item_i = m_listCtrl->InsertItem( aList_str_ind, pComponent->m_RefDes );
        else
            actual_item_i = aList_str_ind;

        m_listCtrl->SetItemData( actual_item_i, aItem );
        m_listCtrl->SetItemImage( actual_item_i, icon );

        str = variant_attrs.attrs[ATTR_NAME] + wxT( " " ) + variant_attrs.attrs[ATTR_TYPE];
        if( variant_attrs.attrs[ATTR_SUBTYPE] == wxT( "" )
            && variant_attrs.attrs[ATTR_VALUE] != wxT( "" ) )
            str += wxT( ' ' );
        else
            str += variant_attrs.attrs[ATTR_SUBTYPE];

        str += variant_attrs.attrs[ATTR_VALUE];
        if( variant_attrs.attrs[ATTR_PRECISION] != wxT( "" ) )
            str += wxT( " ±" ) + variant_attrs.attrs[ATTR_PRECISION];

        str += variant_attrs.attrs[ATTR_DESIGNATION];

        m_listCtrl->SetItem( actual_item_i, TABLE_NAME_COL, str );

        str = wxT( "" );
        if( variant_attrs.attrs[ATTR_MANUFACTURER] != wxT( "" ) )
        {
            str = wxT( "\"" );
            str += variant_attrs.attrs[ATTR_MANUFACTURER];
            str += wxT( "\" " );
        }
        str += variant_attrs.attrs[ATTR_NOTE];
        m_listCtrl->SetItem( actual_item_i, TABLE_NOTE_COL, str );
    }
}


void GOST_COMP_MANAGER::OnRadioVarPart( wxCommandEvent& event )
{
    size_t DBitem;
    int list_str_ind;

    DisableComboBoxes();
    m_listCtrl->DeleteAllItems();
    m_combo_Variant->Enable( true );
    m_checkListCtrl->Show( false );

    list_str_ind = 0;
    for( DBitem = 0; DBitem < m_componentDB->m_AllComponents.GetCount(); DBitem++ )
    {
        if( ( (COMPONENT *)m_componentDB->m_AllComponents[DBitem]
            )->m_Variants_State != COMP_IN_CONST_PART
            && ( (COMPONENT *)m_componentDB->m_AllComponents[DBitem]
               )->m_comp_attr_variants.GetCount() > 0 )
        {
            FormVarPartString( DBitem, list_str_ind++, true );
        }
    }
}


void GOST_COMP_MANAGER::OnSelChangeComboVariant( wxCommandEvent& event )
{
    if ( m_radio_VarPart->GetValue() )
    {
        wxCommandEvent dummyEvent;
        OnRadioVarPart( dummyEvent );
    }
}


void GOST_COMP_MANAGER::FormVariantList()
{
    wxString variant_str;
    size_t i;

    m_onItemChangedCheckListCtrlLock = true;

    m_combo_Variant->Clear();
    m_checkListCtrl->Clear();

    for ( i=0; i < m_componentDB->m_variantIndexes.GetCount(); i++ )
    {
        variant_str = wxString::Format( wxT( "%02d" ), m_componentDB->m_variantIndexes[i] );

        m_combo_Variant->Append( variant_str );
        m_checkListCtrl->Insert( variant_str, i );
    }
    m_combo_Variant->Select( 0 );

    m_onItemChangedCheckListCtrlLock = false;

    if ( m_radio_FullList->GetValue() )
    {
        wxCommandEvent dummyEvent;
        OnRadioFullList( dummyEvent );
    }
}


#define EDITATTR_ERR_NO_ERROR               0
#define EDITATTR_ERR_N_A_IN_VAR_PART_MODE   1
#define EDITATTR_ERR_FATAL_DIFFERENT_PARAMS 2
#define EDITATTR_ERR_WARN_DIFFERENT_PARAMS  3

void GOST_COMP_MANAGER::EditAttribute( int aComp_attr, wxString aStr )
{
    size_t i;
    int item, list_str_ind, variant, variant_ind, selectedCount, error;
    COMPONENT *pComponent, *pBaseComponent;

    selectedCount = m_listCtrl->GetSelectedItemCount();
    if( selectedCount > 1 )
    {
        list_str_ind = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
        item = m_listCtrl->GetItemData( list_str_ind );
        pBaseComponent = (COMPONENT *)m_componentDB->m_AllComponents[item];

        error = EDITATTR_ERR_NO_ERROR;
        if( m_radio_VarPart->GetValue() )
            error = EDITATTR_ERR_N_A_IN_VAR_PART_MODE;
        else if( pBaseComponent->m_Variants_State==COMP_IN_VAR_PART_DIFF )
            error = EDITATTR_ERR_FATAL_DIFFERENT_PARAMS;
        else
        {
            for( i = 0; i < (size_t)selectedCount - 1; i++ )
            {
                list_str_ind = m_listCtrl->GetNextItem( list_str_ind,
                                                        wxLIST_NEXT_ALL,
                                                        wxLIST_STATE_SELECTED );

                item = m_listCtrl->GetItemData( list_str_ind );
                pComponent = (COMPONENT *)m_componentDB->m_AllComponents[item];
                if( pComponent->m_Variants_State==COMP_IN_VAR_PART_DIFF )
                {
                    error = EDITATTR_ERR_FATAL_DIFFERENT_PARAMS;
                    break;
                }

                if( !CompareComps( pBaseComponent, pComponent, -1 ) )
                {
                    error = EDITATTR_ERR_WARN_DIFFERENT_PARAMS;
                    break;
                }
            }
        }

        if ( error==EDITATTR_ERR_N_A_IN_VAR_PART_MODE
             || error==EDITATTR_ERR_FATAL_DIFFERENT_PARAMS )
        {
            wxString err_str;

            switch( error )
            {
                case EDITATTR_ERR_N_A_IN_VAR_PART_MODE:
                    err_str = _( "Group operation is not available in the 'variable part' mode" );
                    break;
                case EDITATTR_ERR_FATAL_DIFFERENT_PARAMS:
                    err_str = _( "Group operation is not available because one of the selected "
                                   "components has different parameters along its variants" );
                    break;
            }
            wxMessageBox( err_str, wxT( "" ), wxOK );
            return;
        }

        if ( error==EDITATTR_ERR_WARN_DIFFERENT_PARAMS )
        {
            int res = wxMessageBox( _( "The selected components have different parameters. "
                                         "Continue?" ), wxT( "" ), wxYES_NO );
            if ( res==wxNO )
                return;
        }
    }

    list_str_ind = m_listCtrl->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    while( list_str_ind >= 0 )
    {
        item = m_listCtrl->GetItemData( list_str_ind );

        pComponent = (COMPONENT *)m_componentDB->m_AllComponents[item];

        if( pComponent->m_Variants_State==COMP_IN_CONST_PART || m_radio_FullList->GetValue() )
            variant_ind = 0;
        else
        {
            variant = m_componentDB->m_variantIndexes[m_combo_Variant->GetCurrentSelection()];
            variant_ind = pComponent->GetVariant( variant, NULL );
        }

        // update attributes for variants
        if( pComponent->m_comp_attr_variants.GetCount() > 1 && m_radio_FullList->GetValue() )
        {
            for ( i = 0; i < pComponent->m_comp_attr_variants.GetCount(); i++ )
            ( (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[i] )->attrs[aComp_attr] = aStr;
        }
        else
            ( (pTCOMPONENT_ATTRS)pComponent->m_comp_attr_variants[variant_ind]
            )->attrs[aComp_attr] = aStr;

        pComponent->Update();

        if( m_radio_FullList->GetValue() )
            FormFullListString( item, list_str_ind, false );
        else if( m_radio_ConstPart->GetValue() )
            FormConstPartString( item, list_str_ind, false );
        else
            FormVarPartString( item, list_str_ind, false );

        list_str_ind = m_listCtrl->GetNextItem( list_str_ind,
                                                wxLIST_NEXT_ALL,
                                                wxLIST_STATE_SELECTED );
    }
}


void GOST_COMP_MANAGER::OnEditChangeComboName( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute( ATTR_NAME, m_combo_Name->GetValue() );
}


void GOST_COMP_MANAGER::OnEditChangeComboType( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute( ATTR_TYPE, m_combo_Type->GetValue() );
}


void GOST_COMP_MANAGER::OnEditChangeComboSubtype( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute( ATTR_SUBTYPE, m_combo_SubType->GetValue() );
}


void GOST_COMP_MANAGER::OnEditChangeComboValue( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute(ATTR_VALUE, m_combo_Value->GetValue() );
}


void GOST_COMP_MANAGER::OnEditChangeComboPrecision( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute( ATTR_PRECISION, m_combo_Precision->GetValue() );
}


void GOST_COMP_MANAGER::OnEditChangeComboNote( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute( ATTR_NOTE, m_combo_Note->GetValue() );
}


void GOST_COMP_MANAGER::OnEditChangeComboDesignation( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute( ATTR_DESIGNATION, m_combo_Designation->GetValue() );
}


void GOST_COMP_MANAGER::OnEditChangeComboManufacturer( wxCommandEvent& event )
{
    if( m_onEditChangeComboBoxLock )
        return;

    EditAttribute( ATTR_MANUFACTURER, m_combo_Manufacturer->GetValue() );
}


/*void GOST_COMP_MANAGER::UpdateListCtrlContent()
{
    wxCommandEvent dummyEvent;

    if( m_radio_FullList->GetValue() )
        OnRadioFullList( dummyEvent );
    else if( m_radio_ConstPart->GetValue() )
        OnRadioConstPart( dummyEvent );
    else
        OnRadioVarPart( dummyEvent );
}*/


void GOST_COMP_MANAGER::OnFileGenerateComponentIndex( wxCommandEvent& event )
{
    m_componentDB->GenerateComponentIndexDoc();
}


void GOST_COMP_MANAGER::OnFileGenerateSpecification( wxCommandEvent& event )
{
    m_componentDB->GenerateSpecificationDoc();
}


void GOST_COMP_MANAGER::OnSettingsAddaNewVariant( wxCommandEvent& event )
{
    long variant;

    wxTextEntryDialog variantDlg( this, _( "Enter a variant" ),
                                  _( "Add a new variant" ), wxT( "" ),
                                  wxOK | wxCANCEL | wxCENTRE );

    if ( variantDlg.ShowModal() == wxID_OK )
    {
        variantDlg.GetValue().ToLong( &variant );
        m_componentDB->AddNewVariant( variant );
        FormVariantList();
    }
}


void GOST_COMP_MANAGER::OnActivate( wxActivateEvent& event )
{
    if( event.GetActive() )
    {
        // focus acquired
        m_componentDB = new COMPONENT_DB();

        m_componentDB->LoadFromKiCad();

        FormVariantList();
        DisableComboBoxes();

        wxCommandEvent dummyEvent;
        OnRadioFullList( dummyEvent );
    }
    else
    {
        // focus lost
        if( m_componentDB->WriteBackToKiCad() )
            // let eeschema know to save changes on closing it
            m_schEditFrame->GetScreen()->SetModify();

        delete m_componentDB;
        m_componentDB = NULL;
    }

    event.Skip();
}


void GOST_COMP_MANAGER::OnCloseWindow( wxCloseEvent& event )
{
    if( m_componentDB->WriteBackToKiCad() )
        // let eeschema know to save changes on closing it
        m_schEditFrame->GetScreen()->SetModify();

    Destroy();
}


void GOST_COMP_MANAGER::OnSize(wxSizeEvent& event)
{
    FRAME_GOST_DOC_GEN_BASE::OnSize( event );

    if (m_listCtrl)
    {
        wxSize listClientSize = m_listCtrl->GetClientSize();
        m_listCtrl->SetColumnWidth( TABLE_REFDES_COL, listClientSize.GetWidth() * 1/4 );
        m_listCtrl->SetColumnWidth( TABLE_NAME_COL, listClientSize.GetWidth() * 1/4 );
        m_listCtrl->SetColumnWidth( TABLE_NOTE_COL, listClientSize.GetWidth() * 1/4 );
        m_listCtrl->SetColumnWidth( TABLE_FORVARIANTS_COL, listClientSize.GetWidth() * 1/4 );
    }
}
