#include "common.h"
#include "pcbnew.h"
#include "wxPcbStruct.h"
#include "class_board_design_settings.h"
#include "colors_selection.h"
#include "layers_id_colors_and_visibility.h"
#include "bitmaps.h"
#include "hotkeys.h"
#include "help_common_strings.h"

#include "class_board.h"

#include <wx/ownerdrw.h>
#include <wx/menuitem.h>
#include <wx/bmpcbox.h>
#include <wx/wx.h>

#include "class_layer_box_selector.h"

/* class to display a layer list.
 *
 */

LAYER_BOX_SELECTOR::LAYER_BOX_SELECTOR( EDA_TOOLBAR* parent, wxWindowID id,
                                        const wxPoint& pos, const wxSize& size,
                                        int n, const wxString choices[] ) :
    wxBitmapComboBox( parent, id, wxEmptyString, pos, size, n, choices, wxCB_READONLY )
{
    m_layerorder   = true;
    m_layerhotkeys = true;
    m_hotkeys      = NULL;

    if( choices != NULL )
        ResyncBitmapOnly();
}


LAYER_BOX_SELECTOR::LAYER_BOX_SELECTOR( EDA_TOOLBAR* parent, wxWindowID id,
                                        const wxPoint& pos, const wxSize& size,
                                        const wxArrayString& choices ) :
    wxBitmapComboBox( parent, id, wxEmptyString, pos, size, choices, wxCB_READONLY )
{
    m_layerorder   = true;
    m_layerhotkeys = true;
    m_hotkeys      = NULL;

    if( !choices.IsEmpty() )
        ResyncBitmapOnly();
}


bool LAYER_BOX_SELECTOR::SetLayersOrdered( bool value )
{
    m_layerorder = value;
    return m_layerorder;
}


bool LAYER_BOX_SELECTOR::SetLayersHotkeys( bool value )
{
    m_layerhotkeys = value;
    return m_layerhotkeys;
}


// Get Current Item #
int LAYER_BOX_SELECTOR::GetChoice()
{
    return GetSelection();
}


// Get Current Layer
int LAYER_BOX_SELECTOR::GetLayerSelection()
{
    return (long) GetClientData( GetSelection() );
}


// Set Layer #
int LAYER_BOX_SELECTOR::SetLayerSelection( int layer )
{
    int elements = GetCount();

    for( int i = 0; i < elements; i++ )
    {
        if( GetClientData( i ) == (void*) layer )
        {
            if( GetSelection() != i )   // Element (i) is not selected
            {
                SetSelection( i );
                return i;
            }
            else
                return i;               //If element already selected; do nothing
        }
    }

    // Not Found
    SetSelection( -1 );
    return -1;
}


// Reload the Layers
void LAYER_BOX_SELECTOR::Resync()
{
    PCB_BASE_FRAME* pcbFrame = (PCB_BASE_FRAME*) GetParent()->GetParent();
    BOARD* board = pcbFrame->GetBoard();

    wxASSERT( board != NULL );

    Clear();

    static DECLARE_LAYERS_ORDER_LIST( layertranscode );
    static DECLARE_LAYERS_HOTKEY( layerhk );

    for( int i = 0; i < LAYER_COUNT; i++ )
    {
        wxBitmap   layerbmp( 14, 14 );
        wxMemoryDC bmpDC;
        wxBrush    brush;
        wxString   layername;
        int        layerid = i;

        if( m_layerorder )
            layerid = layertranscode[i];

        if( !board->IsLayerEnabled( layerid ) )
            continue;

        // Prepare Bitmap
        bmpDC.SelectObject( layerbmp );
        brush.SetColour( MakeColour( board->GetLayerColor( layerid ) ) );
        brush.SetStyle( wxSOLID );

        bmpDC.SetBrush( brush );
        bmpDC.DrawRectangle( 0, 0, layerbmp.GetWidth(), layerbmp.GetHeight() );
        bmpDC.SetBrush( *wxTRANSPARENT_BRUSH );
        bmpDC.SetPen( *wxBLACK_PEN );
        bmpDC.DrawRectangle( 0, 0, layerbmp.GetWidth(), layerbmp.GetHeight() );

        layername = board->GetLayerName( layerid );

        if( m_layerhotkeys && m_hotkeys != NULL )
            layername = AddHotkeyName( layername, m_hotkeys, layerhk[layerid], IS_COMMENT );

        Append( layername, layerbmp, (void*) layerid );
    }
}

void LAYER_BOX_SELECTOR::ResyncBitmapOnly()
{
    PCB_BASE_FRAME* pcbFrame = (PCB_BASE_FRAME*) GetParent()->GetParent();
    BOARD* board = pcbFrame->GetBoard();

    int elements = GetCount();
    for( int i = 0; i < elements; i++ )
    {
        wxBitmap   layerbmp( 14, 14 );
        wxMemoryDC bmpDC;
        wxBrush    brush;
        wxString   layername;
        int        layerid = i;

        // Prepare Bitmap
        bmpDC.SelectObject( layerbmp );
        brush.SetColour( MakeColour( board->GetLayerColor( layerid ) ) );
        brush.SetStyle( wxSOLID );

        bmpDC.SetBrush( brush );
        bmpDC.DrawRectangle( 0, 0, layerbmp.GetWidth(), layerbmp.GetHeight() );
        bmpDC.SetBrush( *wxTRANSPARENT_BRUSH );
        bmpDC.SetPen( *wxBLACK_PEN );
        bmpDC.DrawRectangle( 0, 0, layerbmp.GetWidth(), layerbmp.GetHeight() );

        SetItemBitmap(i, layerbmp);
    }
}
