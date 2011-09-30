/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2010 Jean-Pierre Charras, jean-pierre.charras@gpisa-lab.inpg.fr
 * Copyright (C) 2010 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2010 KiCad Developers, see change_log.txt for contributors.
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
 * @file class_gerbview_layer_widget.cpp
 * @brief  GerbView layers manager.
 */

#include "fctsys.h"
#include "common.h"
#include "class_drawpanel.h"
#include "pcbstruct.h"
#include "macros.h"
#include "class_layer_box_selector.h"

#include "gerbview.h"
#include "class_GERBER.h"
#include "layer_widget.h"
#include "class_gerbview_layer_widget.h"


/*
 * Class GERBER_LAYER_WIDGET
 * is here to implement the abtract functions of LAYER_WIDGET so they
 * may be tied into the GERBVIEW_FRAME's data and so we can add a popup
 * menu which is specific to Pcbnew's needs.
 */


GERBER_LAYER_WIDGET::GERBER_LAYER_WIDGET( GERBVIEW_FRAME* aParent, wxWindow* aFocusOwner,
                                          int aPointSize ) :
    LAYER_WIDGET( aParent, aFocusOwner, aPointSize ),
    myframe( aParent )
{
    ReFillRender();

    // Update default tabs labels for GerbView
    SetLayersManagerTabsText( );

    //-----<Popup menu>-------------------------------------------------
    // handle the popup menu over the layer window.
    m_LayerScrolledWindow->Connect( wxEVT_RIGHT_DOWN,
        wxMouseEventHandler( GERBER_LAYER_WIDGET::onRightDownLayers ), NULL, this );

    // since Popupmenu() calls this->ProcessEvent() we must call this->Connect()
    // and not m_LayerScrolledWindow->Connect()
    Connect( ID_SHOW_ALL_COPPERS, ID_SHOW_NO_COPPERS, wxEVT_COMMAND_MENU_SELECTED,
        wxCommandEventHandler( GERBER_LAYER_WIDGET::onPopupSelection ), NULL, this );

    // install the right click handler into each control at end of ReFill()
    // using installRightLayerClickHandler
}

/**
 * Function SetLayersManagerTabsText
 * Update the layer manager tabs labels
 * Useful when changing Language or to set labels to a non default value
 */
void GERBER_LAYER_WIDGET::SetLayersManagerTabsText( )
{
    m_notebook->SetPageText(0, _("Layer") );
    m_notebook->SetPageText(1, _("Render") );
}

/**
 * Function ReFillRender
 * Rebuild Render for instance after the config is read
 */
void GERBER_LAYER_WIDGET::ReFillRender()
{
    BOARD*  board = myframe->GetBoard();
    ClearRenderRows();

    // Fixed "Rendering" tab rows within the LAYER_WIDGET, only the initial color
    // is changed before appending to the LAYER_WIDGET.  This is an automatic variable
    // not a static variable, change the color & state after copying from code to renderRows
    // on the stack.
    LAYER_WIDGET::ROW renderRows[2] = {

#define RR  LAYER_WIDGET::ROW   // Render Row abreviation to reduce source width

             // text                id                      color       tooltip                 checked
        RR( _( "Grid" ),            GERBER_GRID_VISIBLE,    WHITE,      _( "Show the (x,y) grid dots" ) ),
        RR( _( "DCodes" ),          DCODES_VISIBLE,         WHITE,      _( "Show DCodes identification" ) ),
    };

    for( unsigned row=0;  row<DIM(renderRows);  ++row )
    {
        if( renderRows[row].color != -1 )       // does this row show a color?
        {
            // this window frame must have an established BOARD, i.e. after SetBoard()
            renderRows[row].color = board->GetVisibleElementColor( renderRows[row].id );
        }
        renderRows[row].state = board->IsElementVisible( renderRows[row].id );
    }

    AppendRenderRows( renderRows, DIM(renderRows) );
}

void GERBER_LAYER_WIDGET::installRightLayerClickHandler()
{
    int rowCount = GetLayerRowCount();
    for( int row=0;  row<rowCount;  ++row )
    {
        for( int col=0; col<LYR_COLUMN_COUNT;  ++col )
        {
            wxWindow* w = getLayerComp( row, col );

            w->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(
                GERBER_LAYER_WIDGET::onRightDownLayers ), NULL, this );
        }
    }
}


void GERBER_LAYER_WIDGET::onRightDownLayers( wxMouseEvent& event )
{
    wxMenu          menu;

    // menu text is capitalized:
    // http://library.gnome.org/devel/hig-book/2.20/design-text-labels.html.en#layout-capitalization
    menu.Append( new wxMenuItem( &menu, ID_SHOW_ALL_COPPERS,
        _("Show All Layers") ) );

    menu.Append( new wxMenuItem( &menu, ID_SHOW_NO_COPPERS,
        _( "Hide All Layers" ) ) );

    PopupMenu( &menu );

    passOnFocus();
}

void GERBER_LAYER_WIDGET::onPopupSelection( wxCommandEvent& event )
{
    int     rowCount;
    int     menuId = event.GetId();
    bool    visible = (menuId == ID_SHOW_ALL_COPPERS) ? true : false;;
    int     visibleLayers = 0;

    switch( menuId )
    {
    case ID_SHOW_ALL_COPPERS:
    case ID_SHOW_NO_COPPERS:
        rowCount = GetLayerRowCount();
        for( int row=0; row < rowCount; ++row )
        {
            wxCheckBox* cb = (wxCheckBox*) getLayerComp( row, 3 );
            cb->SetValue( visible );
            if( visible )
                visibleLayers |= (1 << row);
            else
                visibleLayers &= ~(1 << row);
        }

        myframe->GetBoard()->SetVisibleLayers( visibleLayers );
        myframe->DrawPanel->Refresh();
        break;
    }
}



void GERBER_LAYER_WIDGET::ReFill()
{
    BOARD*  brd = myframe->GetBoard();
    int     layer;
    ClearLayerRows();
    for( layer = 0; layer < LAYER_COUNT; layer++ )
    {
        wxString msg;
        msg.Printf( _("Layer %d"), layer+1 );
        AppendLayerRow( LAYER_WIDGET::ROW( msg, layer,
                        brd->GetLayerColor( layer ), wxEmptyString, true ) );
    }

    installRightLayerClickHandler();
}

//-----<LAYER_WIDGET callbacks>-------------------------------------------

void GERBER_LAYER_WIDGET::OnLayerColorChange( int aLayer, int aColor )
{
    myframe->GetBoard()->SetLayerColor( aLayer, aColor );
    myframe->m_SelLayerBox->ResyncBitmapOnly();
    myframe->DrawPanel->Refresh();
}

bool GERBER_LAYER_WIDGET::OnLayerSelect( int aLayer )
{
    // the layer change from the GERBER_LAYER_WIDGET can be denied by returning
    // false from this function.
    int layer = myframe->getActiveLayer( );
    myframe->setActiveLayer( aLayer, false );
    myframe->syncLayerBox();
    if( layer != myframe->getActiveLayer( ) )
        myframe->DrawPanel->Refresh();

    return true;
}

void GERBER_LAYER_WIDGET::OnLayerVisible( int aLayer, bool isVisible, bool isFinal )
{
    BOARD* brd = myframe->GetBoard();
    int visibleLayers = brd->GetVisibleLayers();

    if( isVisible )
        visibleLayers |= (1 << aLayer);
    else
        visibleLayers &= ~(1 << aLayer);

    brd->SetVisibleLayers( visibleLayers );

    if( isFinal )
        myframe->DrawPanel->Refresh();
}

void GERBER_LAYER_WIDGET::OnRenderColorChange( int aId, int aColor )
{
    myframe->GetBoard()->SetVisibleElementColor( aId, aColor );
    myframe->DrawPanel->Refresh();
}

void GERBER_LAYER_WIDGET::OnRenderEnable( int aId, bool isEnabled )
{
    BOARD*  brd = myframe->GetBoard();
        brd->SetElementVisibility( aId, isEnabled );

    myframe->DrawPanel->Refresh();
}

//-----</LAYER_WIDGET callbacks>------------------------------------------

/*
 * Virtual Function useAlternateBitmap
 * return true if bitmaps shown in Render layer list
 * must be alternate bitmaps, or false to use "normal" bitmaps
 */
bool GERBER_LAYER_WIDGET::useAlternateBitmap(int aRow)
{
    bool inUse = false;
    GERBER_IMAGE* gerber = g_GERBER_List[aRow];

    if( gerber != NULL && gerber->m_InUse )
        inUse = true;

    return inUse;
}

/**
 * Function UpdateLayerIcons
 * Update the layer manager icons (layers only)
 * Useful when loading a file or clearing a layer because they change
 */
void GERBER_LAYER_WIDGET::UpdateLayerIcons()
{
    int row_count = GetLayerRowCount();
    for( int row = 0; row < row_count ; row++ )
    {
        wxStaticBitmap* bm = (wxStaticBitmap*) getLayerComp( row, 0 );
        if( bm == NULL)
            continue;

        if( row == m_CurrentRow )
            bm->SetBitmap( useAlternateBitmap(row) ? *m_RightArrowAlternateBitmap : *m_RightArrowBitmap );
        else
            bm->SetBitmap( useAlternateBitmap(row) ? *m_BlankAlternateBitmap : *m_BlankBitmap );
    }
}
