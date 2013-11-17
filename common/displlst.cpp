/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2013 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file displlst.cpp
 */

#include <fctsys.h>
#include <macros.h>
#include <wxstruct.h>
#include <kicad_string.h>
#include <dialog_helpers.h>

EDA_LIST_DIALOG::EDA_LIST_DIALOG( EDA_DRAW_FRAME* aParent, const wxString& aTitle,
                                  const wxArrayString& aItemHeaders,
                                  const std::vector<wxArrayString>& aItemList,
                                  const wxString& aRefText,
                                  void(*aCallBackFunction)(wxString& Text),
                                  bool aSortList ) :
    EDA_LIST_DIALOG_BASE( aParent, wxID_ANY, aTitle )
{
    m_sortList    = aSortList;
    m_callBackFct = aCallBackFunction;
    m_itemsListCp = &aItemList;

    for( unsigned i = 0; i < aItemHeaders.Count(); i++ )
    {
        wxListItem column;
        column.SetId( i );
        column.SetText( aItemHeaders.Item( i ) );
        m_listBox->InsertColumn( i, column );
    }

    InsertItems( aItemList, 0 );

    if( !aRefText.IsEmpty() )    // try to select the item matching aRefText
    {
        for( unsigned ii = 0; ii < aItemList.size(); ii++ )
        {
            if( aItemList[ii][0] == aRefText )
            {
                m_listBox->SetItemState( ii, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
                break;
            }
        }
    }

    // DIALOG_SHIM needs a unique hash_key because classname is not sufficient
    // because so many dialogs share this same class.
    m_hash_key = TO_UTF8( aTitle );

    for( unsigned i = 0; i < aItemHeaders.Count(); i++ )
        m_listBox->SetColumnWidth( i, wxLIST_AUTOSIZE );

    if( m_callBackFct == NULL )
    {
        m_messages->Show( false );
        m_staticTextMsg->Show( false );
    }

    m_filterBox->SetFocus();

    Layout();
}


EDA_LIST_DIALOG::~EDA_LIST_DIALOG()
{
}


void EDA_LIST_DIALOG::textChangeInFilterBox( wxCommandEvent& event )
{
    wxString filter;
    wxString itemName;

    filter = wxT( "*" ) + m_filterBox->GetLineText( 0 ).MakeLower() + wxT( "*" );

    m_listBox->DeleteAllItems();

    for( unsigned i = 0; i < m_itemsListCp->size(); i++ )
    {
        itemName = (*m_itemsListCp)[i].Item( 0 );

        if( itemName.MakeLower().Matches( filter ) )
        {
            Append( (*m_itemsListCp)[i] );
        }
    }

    if( m_sortList )
        sortList();
}


wxString EDA_LIST_DIALOG::GetTextSelection( int aColumn )
{
    wxCHECK_MSG( aColumn < m_listBox->GetColumnCount(), wxEmptyString,
                 wxT( "Invalid list control column." ) );

    wxListItem info;
    wxString   text;
    long       item = -1;
    item = m_listBox->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    info.m_mask = wxLIST_MASK_TEXT;
    info.m_itemId = item;
    info.m_col = aColumn;

    if( !m_listBox->GetItem( info ) )
        return wxEmptyString;

    return info.m_text;
}


void EDA_LIST_DIALOG::Append( const wxArrayString& itemList )
{
    long itemIndex = m_listBox->InsertItem( m_listBox->GetItemCount(), itemList[0] );

    m_listBox->SetItemData( itemIndex, (long) &(itemList[0]) );

    // Adding the next columns content
    for( unsigned i = 1; i < itemList.size(); i++ )
    {
        m_listBox->SetItem( itemIndex, i, itemList[i] );
    }
}


void EDA_LIST_DIALOG::InsertItems( const std::vector< wxArrayString >& itemList, int position )
{
    for( unsigned row = 0; row < itemList.size(); row++ )
    {
        wxASSERT( (int) itemList[row].GetCount() == m_listBox->GetColumnCount() );

        for( unsigned col = 0; col < itemList[row].GetCount(); col++ )
        {
            long itemIndex;

            if( col == 0 )
            {
                itemIndex = m_listBox->InsertItem( row+position, itemList[row].Item( col ) );
                m_listBox->SetItemData( itemIndex, (long) &itemList[row].Item( col ) );
            }
            else
            {
                m_listBox->SetItem( itemIndex, col, itemList[row].Item( col ) );
            }
        }
    }

    if( m_sortList )
        sortList();
}


void EDA_LIST_DIALOG::onCancelClick( wxCommandEvent& event )
{
    EndModal( wxID_CANCEL );
}


void EDA_LIST_DIALOG::onListItemSelected( wxListEvent& event )
{

    if( m_callBackFct )
    {
        m_messages->Clear();
        wxString text = GetTextSelection();
        m_callBackFct( text );
        m_messages->WriteText( text );
    }
}


void EDA_LIST_DIALOG::onListItemActivated( wxListEvent& event )
{
    EndModal( wxID_OK );
}


void EDA_LIST_DIALOG::onOkClick( wxCommandEvent& event )
{
    EndModal( wxID_OK );
}


void EDA_LIST_DIALOG::onClose( wxCloseEvent& event )
{
    EndModal( wxID_CANCEL );
}


/* Sort alphabetically, case insensitive.
 */
static int wxCALLBACK MyCompareFunction( long aItem1, long aItem2, long aSortData )
{
    wxString* component1Name = (wxString*) aItem1;
    wxString* component2Name = (wxString*) aItem2;

    return StrNumCmp( *component1Name, *component2Name, INT_MAX, true );
}


void EDA_LIST_DIALOG::sortList()
{
    m_listBox->SortItems( MyCompareFunction, 0 );
}
