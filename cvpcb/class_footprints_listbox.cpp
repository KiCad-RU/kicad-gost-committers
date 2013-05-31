/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file class_footprints_listbox.cpp
 * class to display the list of available footprints
 */

#include <fctsys.h>
#include <wxstruct.h>
#include <macros.h>
#include <appl_wxstruct.h>
#include <wildcards_and_files_ext.h>

#include <cvpcb.h>
#include <cvpcb_mainframe.h>
#include <cvstruct.h>


/***************************************/
/* ListBox handling the footprint list */
/***************************************/

FOOTPRINTS_LISTBOX::FOOTPRINTS_LISTBOX( CVPCB_MAINFRAME* parent,
                                        wxWindowID id, const wxPoint& loc,
                                        const wxSize& size,
                                        int nbitems, wxString choice[] ) :
    ITEMS_LISTBOX_BASE( parent, id, loc, size )
{
    m_UseFootprintFullList = true;
    m_ActiveFootprintList  = NULL;
    SetActiveFootprintList( true );
}


FOOTPRINTS_LISTBOX::~FOOTPRINTS_LISTBOX()
{
}


/*
 * Return number of items
 */
int FOOTPRINTS_LISTBOX::GetCount()
{
    return m_ActiveFootprintList->Count();
}


/*
 * Change an item text
 */
void FOOTPRINTS_LISTBOX::SetString( unsigned linecount, const wxString& text )
{
    if( linecount >= m_ActiveFootprintList->Count() )
        linecount = m_ActiveFootprintList->Count() - 1;
    if( linecount >= 0 )
        (*m_ActiveFootprintList)[linecount] = text;
}


wxString FOOTPRINTS_LISTBOX::GetSelectedFootprint()
{
    wxString footprintName;
    int      ii = GetFirstSelected();

    if( ii >= 0 )
    {
        wxString msg = (*m_ActiveFootprintList)[ii];
        msg.Trim( true );
        msg.Trim( false );
        footprintName = msg.AfterFirst( wxChar( ' ' ) );
    }

    return footprintName;
}


void FOOTPRINTS_LISTBOX::AppendLine( const wxString& text )
{
    m_ActiveFootprintList->Add( text );
    SetItemCount( m_ActiveFootprintList->Count() );
}


/*
 * Overlaid function: MUST be provided in wxLC_VIRTUAL mode
 * because real data is not handled by ITEMS_LISTBOX_BASE
 */
wxString FOOTPRINTS_LISTBOX::OnGetItemText( long item, long column ) const
{
    return m_ActiveFootprintList->Item( item );
}


/*
 * Enable or disable an item
 */
void FOOTPRINTS_LISTBOX::SetSelection( unsigned index, bool State )
{
    if( (int) index >= GetCount() )
        index = GetCount() - 1;

    if( (index >= 0)  && (GetCount() > 0) )
    {
#ifndef __WXMAC__
        Select( index, State );
#endif
        EnsureVisible( index );
#ifdef __WXMAC__
        Refresh();
#endif
    }
}


void FOOTPRINTS_LISTBOX::SetFootprintFullList( FOOTPRINT_LIST& list )
{
    wxString msg;
    int      oldSelection = GetSelection();

    m_FullFootprintList.Clear();

    for( unsigned ii = 0; ii < list.GetCount(); ii++ )
    {
        FOOTPRINT_INFO & footprint = list.GetItem(ii);
        msg.Printf( wxT( "%3zu %s" ), m_FullFootprintList.GetCount() + 1,
                   GetChars(footprint.m_Module) );
        m_FullFootprintList.Add( msg );
    }

    SetActiveFootprintList( true );

    if(  GetCount() == 0 || oldSelection < 0 || oldSelection >= GetCount() )
        SetSelection( 0, true );
    Refresh();
}


void FOOTPRINTS_LISTBOX::SetFootprintFilteredList( COMPONENT*      aComponent,
                                                   FOOTPRINT_LIST& list )
{
    wxString msg;
    unsigned jj;
    int      OldSelection = GetSelection();
    bool     hasItem = false;

    m_FilteredFootprintList.Clear();

    for( unsigned ii = 0; ii < list.GetCount(); ii++ )
    {
        FOOTPRINT_INFO& footprint = list.GetItem(ii);
        // Search for matching footprints
        // The search is case insensitive
        wxString module = footprint.m_Module.Upper();
        wxString candidate;

        for( jj = 0; jj < aComponent->GetFootprintFilters().GetCount(); jj++ )
        {
            candidate = aComponent->GetFootprintFilters()[jj].Upper();

            if( !module.Matches( candidate ) )
                continue;

            msg.Printf( wxT( "%3zu %s" ), m_FilteredFootprintList.GetCount() + 1,
                        footprint.m_Module.GetData() );
            m_FilteredFootprintList.Add( msg );
            hasItem = true;
        }
    }

    if( hasItem )
        SetActiveFootprintList( false );
    else
        SetActiveFootprintList( true );

    if( ( GetCount() == 0 ) || ( OldSelection >= GetCount() ) )
        SetSelection( 0, true );

    Refresh();
}


void FOOTPRINTS_LISTBOX::SetFootprintFilteredByPinCount( COMPONENT*      aComponent,
                                                         FOOTPRINT_LIST& list )
{
    wxString msg;
    int      oldSelection = GetSelection();
    bool     hasItem = false;

    m_FilteredFootprintList.Clear();

    for( unsigned ii = 0; ii < list.GetCount(); ii++ )
    {
        FOOTPRINT_INFO& footprint = list.GetItem(ii);

        if( aComponent->GetNetCount() == footprint.m_padCount )
        {
            msg.Printf( wxT( "%3zu %s" ), m_FilteredFootprintList.GetCount() + 1,
                        footprint.m_Module.GetData() );
            m_FilteredFootprintList.Add( msg );
            hasItem = true;
        }
    }

    if( hasItem )
        SetActiveFootprintList( false );
    else
        SetActiveFootprintList( true );

    if( ( GetCount() == 0 ) || ( oldSelection >= GetCount() ) )
        SetSelection( 0, true );

    Refresh();
}

void FOOTPRINTS_LISTBOX::SetFootprintFilteredByLibraryList( FOOTPRINT_LIST& list,
                                                            wxString SelectedLibrary ) {
    wxString msg;
    int      oldSelection = GetSelection();
    bool     hasItem = false;

    wxFileName filename = SelectedLibrary;
    filename.SetExt( LegacyFootprintLibPathExtension );
    wxString FullLibraryName = wxGetApp().FindLibraryPath( filename );

    m_FilteredFootprintList.Clear();

    for( unsigned ii = 0; ii < list.GetCount(); ii++ )
    {
        FOOTPRINT_INFO& footprint = list.GetItem(ii);
        wxString LibName = footprint.m_libPath;
        if( LibName.Matches( FullLibraryName ) )
        {
            msg.Printf( wxT( "%3d %s" ), m_FilteredFootprintList.GetCount() + 1,
                     footprint.m_Module.GetData() );
            m_FilteredFootprintList.Add( msg );
            hasItem = true;
        }
    }

    if( hasItem )
        SetActiveFootprintList( false );
    else
        SetActiveFootprintList( true );

    if( ( GetCount() == 0 ) || ( oldSelection >= GetCount() ) )
        SetSelection( 0, true );

    Refresh();
}

/** Set the footprint list. We can have 2 footprint list:
 *  The full footprint list
 *  The filtered footprint list (if the current selected component has a
 * filter for footprints)
 *  @param FullList true = full footprint list, false = filtered footprint list
 *  @param Redraw = true to redraw the window
 */
void FOOTPRINTS_LISTBOX::SetActiveFootprintList( bool FullList, bool Redraw )
{
    bool old_selection = m_UseFootprintFullList;

#ifdef __WINDOWS__

    /* Workaround for a curious bug in wxWidgets:
     * if we switch from a long list of footprints to a short list (a
     * filtered footprint list), and if the selected item is near the end
     * of the long list,  the new list is not displayed from the top of
     * the list box
     */
    if( m_ActiveFootprintList )
    {
        bool new_selection;

        if( FullList )
            new_selection = true;
        else
            new_selection = false;

        if( new_selection != old_selection )
            SetSelection( 0, true );
    }
#endif
    if( FullList )
    {
        m_UseFootprintFullList = true;
        m_ActiveFootprintList  = &m_FullFootprintList;
        SetItemCount( m_FullFootprintList.GetCount() );
    }
    else
    {
        m_UseFootprintFullList = false;
        m_ActiveFootprintList  = &m_FilteredFootprintList;
        SetItemCount( m_FilteredFootprintList.GetCount() );
    }

    if( Redraw )
    {
        if( !m_UseFootprintFullList || ( m_UseFootprintFullList != old_selection ) )
        {
            Refresh();
        }
    }

    GetParent()->DisplayStatus();
}


/**************************************/
/* Event table for the footprint list */
/**************************************/

BEGIN_EVENT_TABLE( FOOTPRINTS_LISTBOX, ITEMS_LISTBOX_BASE )
    EVT_SIZE( ITEMS_LISTBOX_BASE::OnSize )
    EVT_CHAR( FOOTPRINTS_LISTBOX::OnChar )
END_EVENT_TABLE()


void FOOTPRINTS_LISTBOX::OnLeftClick( wxListEvent& event )
{
    FOOTPRINT_INFO* Module;
    wxString   footprintName = GetSelectedFootprint();

    Module = GetParent()->m_footprints.GetModuleInfo( footprintName );
    wxASSERT(Module);
    if( GetParent()->m_DisplayFootprintFrame )
    {
        // Refresh current selected footprint view:
        GetParent()->CreateScreenCmp();
    }

    if( Module )
    {
        wxString msg;
        msg = Module->m_Doc;
        GetParent()->SetStatusText( msg, 0 );

        msg  = wxT( "KeyW: " );
        msg += Module->m_KeyWord;
        GetParent()->SetStatusText( msg, 1 );
    }
}


void FOOTPRINTS_LISTBOX::OnLeftDClick( wxListEvent& event )
{
    wxString footprintName = GetSelectedFootprint();

    GetParent()->SetNewPkg( footprintName );
}


void FOOTPRINTS_LISTBOX::OnChar( wxKeyEvent& event )
{
    int key = event.GetKeyCode();

    switch( key )
    {
        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            GetParent()->m_ListCmp->SetFocus();
            return;

        case WXK_HOME:
        case WXK_END:
        case WXK_UP:
        case WXK_DOWN:
        case WXK_PAGEUP:
        case WXK_PAGEDOWN:
        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            event.Skip();
            return;

        default:
            break;
    }

    // Search for an item name starting by the key code:
    key = toupper(key);

    for( unsigned ii = 0; ii < m_ActiveFootprintList->GetCount(); ii++ )
    {
        wxString text = m_ActiveFootprintList->Item(ii);

        /* search for the start char of the footprint name.
         * we must skip the line number
         */
        text.Trim(false);      // Remove leading spaces in line
        unsigned jj = 0;

        for( ; jj < text.Len(); jj++ )
        {
            // skip line number
            if( text[jj] == ' ' )
                break;
        }

        for( ; jj < text.Len(); jj++ )
        {   // skip blanks
            if( text[jj] != ' ' )
                break;
        }

        int start_char = toupper( text[jj] );

        if( key == start_char )
        {
            Focus( ii );
            SetSelection( ii, true );   // Ensure visible
            break;
        }
    }
}
