/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
 * Copyright (C) 2014 KiCad Developers, see change_log.txt for contributors.
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
#include <dialog_choose_component.h>

#include <set>
#include <boost/foreach.hpp>
#include <wx/tokenzr.h>

#include <class_library.h>
#include <component_tree_search_container.h>
#include <sch_base_frame.h>

// Tree navigation helpers.
static wxTreeItemId GetPrevItem( const wxTreeCtrl& tree, const wxTreeItemId& item );
static wxTreeItemId GetNextItem( const wxTreeCtrl& tree, const wxTreeItemId& item );

DIALOG_CHOOSE_COMPONENT::DIALOG_CHOOSE_COMPONENT( wxWindow* aParent, const wxString& aTitle,
                                                  COMPONENT_TREE_SEARCH_CONTAINER* aContainer,
                                                  int aDeMorganConvert )
    : DIALOG_CHOOSE_COMPONENT_BASE( aParent, wxID_ANY, aTitle ),
      m_search_container( aContainer ),
      m_deMorganConvert( aDeMorganConvert >= 0 ? aDeMorganConvert : 0 ),
      m_external_browser_requested( false ),
      m_received_doubleclick_in_tree( false ),
      m_ready_to_render( false )
{
    m_search_container->SetTree( m_libraryComponentTree );
    m_searchBox->SetFocus();
    m_componentDetails->SetEditable( false );
    m_componentView
        ->Connect( wxEVT_PAINT,
                   wxPaintEventHandler( DIALOG_CHOOSE_COMPONENT::OnHandlePreviewRepaint ),
                   NULL, this );
    m_componentView
        ->Connect( wxEVT_LEFT_UP,
                   wxMouseEventHandler( DIALOG_CHOOSE_COMPONENT::OnStartComponentBrowser ),
                   NULL, this );

    m_ready_to_render = true;  // Only after setup, we accept drawing updates.
}


// After this dialog is done: return the alias that has been selected, or an
// empty string if there is none.
wxString DIALOG_CHOOSE_COMPONENT::GetSelectedAliasName( int* aUnit ) const
{
    LIB_ALIAS *alias = m_search_container->GetSelectedAlias( aUnit );

    if( alias )
        return alias->GetName();

    return wxEmptyString;
}


void DIALOG_CHOOSE_COMPONENT::OnSearchBoxChange( wxCommandEvent& aEvent )
{
    m_search_container->UpdateSearchTerm( m_searchBox->GetLineText(0) );
    updateSelection();
}


void DIALOG_CHOOSE_COMPONENT::OnSearchBoxEnter( wxCommandEvent& aEvent )
{
    EndModal( wxID_OK );   // We are done.
}


void DIALOG_CHOOSE_COMPONENT::selectIfValid( const wxTreeItemId& aTreeId )
{
    if( aTreeId.IsOk() && aTreeId != m_libraryComponentTree->GetRootItem() )
        m_libraryComponentTree->SelectItem( aTreeId );
}


void DIALOG_CHOOSE_COMPONENT::OnInterceptSearchBoxKey( wxKeyEvent& aKeyStroke )
{
    // Cursor up/down and partiallyi cursor are use to do tree navigation operations.
    // This is done by intercepting some navigational keystrokes that normally would go to
    // the text search box (which has the focus by default). That way, we are mostly keyboard
    // operable.
    // (If the tree has the focus, it can handle that by itself).
    const wxTreeItemId sel = m_libraryComponentTree->GetSelection();

    switch( aKeyStroke.GetKeyCode() )
    {
    case WXK_UP:
        selectIfValid( GetPrevItem( *m_libraryComponentTree, sel ) );
        break;

    case WXK_DOWN:
        selectIfValid( GetNextItem( *m_libraryComponentTree, sel ) );
        break;

        // The follwoing keys we can only hijack if they are not needed by the textbox itself.

    case WXK_LEFT:
        if( m_searchBox->GetInsertionPoint() == 0 )
            m_libraryComponentTree->Collapse( sel );
        else
            aKeyStroke.Skip();   // Use for original purpose: move cursor.
        break;

    case WXK_RIGHT:
        if( m_searchBox->GetInsertionPoint() >= (long) m_searchBox->GetLineText( 0 ).length() )
            m_libraryComponentTree->Expand( sel );
        else
            aKeyStroke.Skip();   // Use for original purpose: move cursor.
        break;

    default:
        aKeyStroke.Skip();  // Any other key: pass on to search box directly.
        break;
    }
}


void DIALOG_CHOOSE_COMPONENT::OnTreeSelect( wxTreeEvent& aEvent )
{
    updateSelection();
}


void DIALOG_CHOOSE_COMPONENT::OnDoubleClickTreeSelect( wxTreeEvent& aEvent )
{
    if( !updateSelection() )
        return;

    // Ok, got selection. We don't just end the modal dialog here, but
    // wait for the MouseUp event to occur. Otherwise something (broken?)
    // happens: the dialog will close and will deliver the 'MouseUp' event
    // to the eeschema canvas, that will immediately place the component.
    m_received_doubleclick_in_tree = true;
}


void DIALOG_CHOOSE_COMPONENT::OnTreeMouseUp( wxMouseEvent& aMouseEvent )
{
    if( m_received_doubleclick_in_tree )
        EndModal( wxID_OK );     // We are done (see OnDoubleClickTreeSelect)
    else
        aMouseEvent.Skip();      // Let upstream handle it.
}


void DIALOG_CHOOSE_COMPONENT::OnStartComponentBrowser( wxMouseEvent& aEvent )
{
    m_external_browser_requested = true;
    EndModal( wxID_OK );   // We are done.
}


bool DIALOG_CHOOSE_COMPONENT::updateSelection()
{
    int unit = 0;
    LIB_ALIAS* selection = m_search_container->GetSelectedAlias( &unit );

    renderPreview( selection ? selection->GetComponent() : NULL, unit );

    m_componentDetails->Clear();

    if( selection == NULL )
        return false;

    m_componentDetails->Freeze();
    wxFont font_normal = m_componentDetails->GetFont();
    wxFont font_bold = m_componentDetails->GetFont();
    font_bold.SetWeight( wxFONTWEIGHT_BOLD );

    wxTextAttr headline_attribute;
    headline_attribute.SetFont(font_bold);
    wxTextAttr text_attribute;
    text_attribute.SetFont(font_normal);

    const wxString description = selection->GetDescription();

    if( !description.empty() )
    {
        m_componentDetails->SetDefaultStyle( headline_attribute );
        m_componentDetails->AppendText( _("Description\n") );
        m_componentDetails->SetDefaultStyle( text_attribute );
        m_componentDetails->AppendText( description );
        m_componentDetails->AppendText( wxT("\n\n") );
    }

    const wxString keywords = selection->GetKeyWords();

    if( !keywords.empty() )
    {
        m_componentDetails->SetDefaultStyle( headline_attribute );
        m_componentDetails->AppendText( _("Keywords\n") );
        m_componentDetails->SetDefaultStyle( text_attribute );
        m_componentDetails->AppendText( keywords );
    }

    m_componentDetails->SetInsertionPoint( 0 );  // scroll up.
    m_componentDetails->Thaw();

    return true;
}


void DIALOG_CHOOSE_COMPONENT::OnHandlePreviewRepaint( wxPaintEvent& aRepaintEvent )
{
    int unit = 0;
    LIB_ALIAS* selection = m_search_container->GetSelectedAlias( &unit );

    renderPreview( selection ? selection->GetComponent() : NULL, unit );
}


// Render the preview in our m_componentView. If this gets more complicated, we should
// probably have a derived class from wxPanel; but this keeps things local.
void DIALOG_CHOOSE_COMPONENT::renderPreview( LIB_COMPONENT* aComponent, int aUnit )
{
    if( !m_ready_to_render )
        return;

    wxPaintDC dc( m_componentView );
    dc.SetBackground( *wxWHITE_BRUSH );
    dc.Clear();

    if( aComponent == NULL )
        return;

    if( aUnit <= 0 )
        aUnit = 1;

    const wxSize dc_size = dc.GetSize();
    dc.SetDeviceOrigin( dc_size.x / 2, dc_size.y / 2 );

    // Find joint bounding box for everything we are about to draw.
    EDA_RECT bBox;

    BOOST_FOREACH( LIB_ITEM& item, aComponent->GetDrawItemList() )
    {
        if( ( item.GetUnit() && item.GetUnit() != aUnit )
             || ( item.GetConvert() && item.GetConvert() != m_deMorganConvert ) )
            continue;
        bBox.Merge( item.GetBoundingBox() );
    }

    const double xscale = (double) dc_size.x / bBox.GetWidth();
    const double yscale = (double) dc_size.y / bBox.GetHeight();
    const double scale  = std::min( xscale, yscale ) * 0.85;

    dc.SetUserScale( scale, scale );

    wxPoint offset =  bBox.Centre();
    NEGATE( offset.x );
    NEGATE( offset.y );

    GRResetPenAndBrush( &dc );

    BOOST_FOREACH( LIB_ITEM& item, aComponent->GetDrawItemList() )
    {
        if( ( item.GetUnit() && item.GetUnit() != aUnit )
             || ( item.GetConvert() && item.GetConvert() != m_deMorganConvert ) )
            continue;
        item.Draw( NULL, &dc, offset, UNSPECIFIED_COLOR, GR_COPY,
                   NULL, DefaultTransform );
    }
}


static wxTreeItemId GetPrevItem( const wxTreeCtrl& tree, const wxTreeItemId& item )
{
    wxTreeItemId prevItem = tree.GetPrevSibling( item );

    if( !prevItem.IsOk() )
    {
        prevItem = tree.GetItemParent( item );
    }
    else if( tree.IsExpanded( prevItem ) )
    {
        prevItem = tree.GetLastChild( prevItem );
    }

    return prevItem;
}


static wxTreeItemId GetNextItem( const wxTreeCtrl& tree, const wxTreeItemId& item )
{
    wxTreeItemId nextItem;

    if( tree.IsExpanded( item ) )
    {
        wxTreeItemIdValue dummy;
        nextItem = tree.GetFirstChild( item, dummy );
    }
    else
    {
        // Walk up levels until we find one that has a next sibling.
        for ( wxTreeItemId walk = item; walk.IsOk(); walk = tree.GetItemParent( walk ) )
        {
            nextItem = tree.GetNextSibling( walk );
            if( nextItem.IsOk() )
                break;
        }
    }

    return nextItem;
}
