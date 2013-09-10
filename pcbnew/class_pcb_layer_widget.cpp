/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2010 Jean-Pierre Charras, jean-pierre.charras@gpisa-lab.inpg.fr
 * Copyright (C) 2010-2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
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


/******************************************************/
/* class_pcb_layer_widget.cpp - Pcbnew layers manager */
/******************************************************/

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <class_drawpanel.h>
#include <class_drawpanel_gal.h>
#include <view/view.h>
#include <painter.h>

#include <confirm.h>
#include <wxPcbStruct.h>
#include <pcbstruct.h>      // enum PCB_VISIBLE
#include <layer_widget.h>
#include <macros.h>
#include <pcbcommon.h>

#include <class_board.h>
#include <class_pcb_layer_widget.h>

#include <pcbnew.h>
#include <collectors.h>
#include <pcbnew_id.h>


/// This is a read only template that is copied and modified before adding to LAYER_WIDGET
const LAYER_WIDGET::ROW PCB_LAYER_WIDGET::s_render_rows[] = {

#define RR  LAYER_WIDGET::ROW   // Render Row abreviation to reduce source width

         // text                id                      color       tooltip
    RR( _( "Through Via" ),     VIA_THROUGH_VISIBLE,    WHITE,      _( "Show through vias" ) ),
    RR( _( "Bl/Buried Via" ),   VIA_BBLIND_VISIBLE,     WHITE,      _( "Show blind or buried vias" )  ),
    RR( _( "Micro Via" ),       VIA_MICROVIA_VISIBLE,   WHITE,      _( "Show micro vias") ),
    RR( _( "Non Plated" ),      NON_PLATED_VISIBLE,     WHITE,      _( "Show non plated holes") ),
    RR( _( "Ratsnest" ),        RATSNEST_VISIBLE,       WHITE,      _( "Show unconnected nets as a ratsnest") ),

    RR( _( "Pads Front" ),      PAD_FR_VISIBLE,         WHITE,      _( "Show footprint pads on board's front" ) ),
    RR( _( "Pads Back" ),       PAD_BK_VISIBLE,         WHITE,      _( "Show footprint pads on board's back" ) ),

    RR( _( "Text Front" ),      MOD_TEXT_FR_VISIBLE,    WHITE,      _( "Show footprint text on board's back" ) ),
    RR( _( "Text Back" ),       MOD_TEXT_BK_VISIBLE,    WHITE,      _( "Show footprint text on board's back" ) ),
    RR( _( "Hidden Text" ),     MOD_TEXT_INVISIBLE,     WHITE,      _( "Show footprint text marked as invisible" ) ),

    RR( _( "Anchors" ),         ANCHOR_VISIBLE,         WHITE,      _( "Show footprint and text origins as a cross" ) ),
    RR( _( "Grid" ),            GRID_VISIBLE,           WHITE,      _( "Show the (x,y) grid dots" ) ),
    RR( _( "No-Connects" ),     NO_CONNECTS_VISIBLE,    UNSPECIFIED_COLOR,         _( "Show a marker on pads which have no net connected" ) ),
    RR( _( "Modules Front" ),   MOD_FR_VISIBLE,         UNSPECIFIED_COLOR,         _( "Show footprints that are on board's front") ),
    RR( _( "Modules Back" ),    MOD_BK_VISIBLE,         UNSPECIFIED_COLOR,         _( "Show footprints that are on board's back") ),
    RR( _( "Values" ),          MOD_VALUES_VISIBLE,     UNSPECIFIED_COLOR,         _( "Show footprint's values") ),
    RR( _( "References" ),      MOD_REFERENCES_VISIBLE, UNSPECIFIED_COLOR,         _( "Show footprint's references") ),
};


PCB_LAYER_WIDGET::PCB_LAYER_WIDGET( PCB_EDIT_FRAME* aParent, wxWindow* aFocusOwner, int aPointSize ) :
    LAYER_WIDGET( aParent, aFocusOwner, aPointSize ),
    myframe( aParent )
{
    m_alwaysShowActiveCopperLayer = false;
    ReFillRender();

    // Update default tabs labels for GerbView
    SetLayersManagerTabsText();

    //-----<Popup menu>-------------------------------------------------
    // handle the popup menu over the layer window.
    m_LayerScrolledWindow->Connect( wxEVT_RIGHT_DOWN,
        wxMouseEventHandler( PCB_LAYER_WIDGET::onRightDownLayers ), NULL, this );

    // since Popupmenu() calls this->ProcessEvent() we must call this->Connect()
    // and not m_LayerScrolledWindow->Connect()
    Connect( ID_SHOW_ALL_COPPERS, ID_ALWAYS_SHOW_NO_COPPERS_BUT_ACTIVE,
        wxEVT_COMMAND_MENU_SELECTED,
        wxCommandEventHandler( PCB_LAYER_WIDGET::onPopupSelection ), NULL, this );

    // install the right click handler into each control at end of ReFill()
    // using installRightLayerClickHandler
}


void PCB_LAYER_WIDGET::installRightLayerClickHandler()
{
    int rowCount = GetLayerRowCount();
    for( int row=0; row < rowCount; ++row )
    {
        for( int col=0; col<LYR_COLUMN_COUNT; ++col )
        {
            wxWindow* w = getLayerComp( row, col );

            w->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler(
                PCB_LAYER_WIDGET::onRightDownLayers ), NULL, this );
        }
    }
}


void PCB_LAYER_WIDGET::onRightDownLayers( wxMouseEvent& event )
{
    wxMenu          menu;

    // menu text is capitalized:
    // http://library.gnome.org/devel/hig-book/2.20/design-text-labels.html.en#layout-capitalization
    menu.Append( new wxMenuItem( &menu, ID_SHOW_ALL_COPPERS,
                                 _( "Show All Copper Layers" ) ) );
    menu.Append( new wxMenuItem( &menu, ID_SHOW_NO_COPPERS_BUT_ACTIVE,
                                 _( "Hide All Copper Layers But Active" ) ) );
    menu.Append( new wxMenuItem( &menu, ID_ALWAYS_SHOW_NO_COPPERS_BUT_ACTIVE,
                                 _( "Always Hide All Copper Layers But Active" ) ) );
    menu.Append( new wxMenuItem( &menu, ID_SHOW_NO_COPPERS,
                                 _( "Hide All Copper Layers" ) ) );

    PopupMenu( &menu );

    passOnFocus();
}


void PCB_LAYER_WIDGET::onPopupSelection( wxCommandEvent& event )
{
    int     rowCount;
    int     menuId = event.GetId();
    bool    visible;
    bool    force_active_layer_visible;

    visible = menuId == ID_SHOW_ALL_COPPERS;
    m_alwaysShowActiveCopperLayer = ( menuId == ID_ALWAYS_SHOW_NO_COPPERS_BUT_ACTIVE );
    force_active_layer_visible = ( menuId == ID_SHOW_NO_COPPERS_BUT_ACTIVE ||
                                   menuId == ID_ALWAYS_SHOW_NO_COPPERS_BUT_ACTIVE );

    switch( menuId )
    {
    case ID_SHOW_ALL_COPPERS:
    case ID_ALWAYS_SHOW_NO_COPPERS_BUT_ACTIVE:
    case ID_SHOW_NO_COPPERS_BUT_ACTIVE:
    case ID_SHOW_NO_COPPERS:
        // Search the last copper layer row index:
        int lastCu = -1;
        rowCount = GetLayerRowCount();
        for( int row = rowCount-1; row>=0; --row )
        {
            wxCheckBox* cb = (wxCheckBox*) getLayerComp( row, 3 );
            LAYER_NUM layer = getDecodedId( cb->GetId() );
            if( IsCopperLayer( layer ) )
            {
                lastCu = row;
                break;
            }
        }

        // Enable/disable the copper layers visibility:
        for( int row=0;  row<rowCount;  ++row )
        {
            wxCheckBox* cb = (wxCheckBox*) getLayerComp( row, 3 );
            LAYER_NUM layer = getDecodedId( cb->GetId() );

            if( IsCopperLayer( layer ) )
            {
                bool loc_visible = visible;
                if( force_active_layer_visible && (layer == myframe->getActiveLayer() ) )
                    loc_visible = true;

                cb->SetValue( loc_visible );

                bool isLastCopperLayer = (row==lastCu);
                OnLayerVisible( layer, loc_visible, isLastCopperLayer );

                if( isLastCopperLayer )
                    break;
            }
        }
        break;
    }
}


void PCB_LAYER_WIDGET::SetLayersManagerTabsText()
{
    m_notebook->SetPageText( 0, _( "Layer" ) );
    m_notebook->SetPageText( 1, _( "Render" ) );
}


void PCB_LAYER_WIDGET::ReFillRender()
{
    BOARD*  board = myframe->GetBoard();
    ClearRenderRows();

    // Add "Render" tab rows to LAYER_WIDGET, after setting color and checkbox state.
    // Because s_render_rows is created static, we must explicitely call
    // wxGetTranslation for texts which are internationalized (tool tips
    // and item names)
    for( unsigned row=0;  row<DIM(s_render_rows);  ++row )
    {
        LAYER_WIDGET::ROW renderRow = s_render_rows[row];
        renderRow.tooltip = wxGetTranslation( s_render_rows[row].tooltip);
        renderRow.rowName = wxGetTranslation( s_render_rows[row].rowName);

        if( renderRow.color != -1 )       // does this row show a color?
        {
            // this window frame must have an established BOARD, i.e. after SetBoard()
            renderRow.color = board->GetVisibleElementColor( renderRow.id );
        }
        renderRow.state = board->IsElementVisible( renderRow.id );

        AppendRenderRow( renderRow );
    }
}


void PCB_LAYER_WIDGET::SyncRenderStates()
{
    BOARD*  board = myframe->GetBoard();

    for( unsigned row=0;  row<DIM(s_render_rows);  ++row )
    {
        int rowId = s_render_rows[row].id;

        // this does not fire a UI event
        SetRenderState( rowId, board->IsElementVisible( rowId ) );
    }
}


void PCB_LAYER_WIDGET::SyncLayerVisibilities()
{
    BOARD*  board = myframe->GetBoard();
    int     count = GetLayerRowCount();

    for( int row=0;  row<count;  ++row )
    {
        // this utilizes more implementation knowledge than ideal, eventually
        // add member ROW getRow() or similar to base LAYER_WIDGET.

        wxWindow* w = getLayerComp( row, 0 );

        LAYER_NUM layerId = getDecodedId( w->GetId() );

        // this does not fire a UI event
        SetLayerVisible( layerId, board->IsLayerVisible( layerId ) );
    }
}


void PCB_LAYER_WIDGET::ReFill()
{
    BOARD*  brd = myframe->GetBoard();
    int     enabledLayers = brd->GetEnabledLayers();

    ClearLayerRows();

    // show all coppers first, with front on top, back on bottom, then technical layers
    for( LAYER_NUM layer = LAYER_N_FRONT; layer >= FIRST_LAYER; --layer )
    {
        if( enabledLayers & GetLayerMask( layer ) )
        {
            const wxChar *dsc;
            switch( layer )
            {
            case LAYER_N_FRONT:
                dsc = _("Front copper layer");
                break;

            case LAYER_N_BACK:
                dsc = _("Back copper layer");
                break;

            default:
                dsc = _("Inner copper layer");
                break;
            }

            AppendLayerRow( LAYER_WIDGET::ROW(
                brd->GetLayerName( layer ), layer, brd->GetLayerColor( layer ),
                dsc, true ) );
        }
    }


    // technical layers are shown in this order:
    // Because they are static, wxGetTranslation must be explicitely
    // called for tooltips.
    static const struct {
        LAYER_NUM   layerId;
        wxString    tooltip;
    } techLayerSeq[] = {
        { ADHESIVE_N_FRONT,     _( "Adhesive on board's front" )    },
        { ADHESIVE_N_BACK,      _( "Adhesive on board's back" )     },
        { SOLDERPASTE_N_FRONT,  _( "Solder paste on board's front" )},
        { SOLDERPASTE_N_BACK,   _( "Solder paste on board's back" ) },
        { SILKSCREEN_N_FRONT,   _( "Silkscreen on board's front" )  },
        { SILKSCREEN_N_BACK,    _( "Silkscreen on board's back" )   },
        { SOLDERMASK_N_FRONT,   _( "Solder mask on board's front" ) },
        { SOLDERMASK_N_BACK,    _( "Solder mask on board's back" )  },
        { DRAW_N,               _( "Explanatory drawings" )         },
        { COMMENT_N,            _( "Explanatory comments" )         },
        { ECO1_N,               _( "User defined meaning" )         },
        { ECO2_N,               _( "User defined meaning" )         },
        { EDGE_N,               _( "Board's perimeter definition" ) },
    };

    for( unsigned i=0;  i<DIM(techLayerSeq);  ++i )
    {
        LAYER_NUM layer = techLayerSeq[i].layerId;

        if( !(enabledLayers & GetLayerMask( layer )) )
            continue;

        AppendLayerRow( LAYER_WIDGET::ROW(
            brd->GetLayerName( layer ), layer, brd->GetLayerColor( layer ),
            wxGetTranslation( techLayerSeq[i].tooltip ), true ) );
    }

    installRightLayerClickHandler();
}

//-----<LAYER_WIDGET callbacks>-------------------------------------------

void PCB_LAYER_WIDGET::OnLayerColorChange( LAYER_NUM aLayer, EDA_COLOR_T aColor )
{
    myframe->GetBoard()->SetLayerColor( aLayer, aColor );
    myframe->ReCreateLayerBox( NULL );
    myframe->RefreshCanvas();
}


bool PCB_LAYER_WIDGET::OnLayerSelect( LAYER_NUM aLayer )
{
    // the layer change from the PCB_LAYER_WIDGET can be denied by returning
    // false from this function.
    myframe->setActiveLayer( aLayer, false );

    if( m_alwaysShowActiveCopperLayer )
        OnLayerSelected();
    else if( DisplayOpt.ContrastModeDisplay )
        myframe->RefreshCanvas();

    return true;
}


bool  PCB_LAYER_WIDGET::OnLayerSelected()
{
    if( !m_alwaysShowActiveCopperLayer )
        return false;

    // postprocess after an active layer selection
    // ensure active layer visible
    wxCommandEvent event;
    event.SetId( ID_ALWAYS_SHOW_NO_COPPERS_BUT_ACTIVE );
    onPopupSelection( event );

    return true;
}


void PCB_LAYER_WIDGET::OnLayerVisible( LAYER_NUM aLayer, bool isVisible, bool isFinal )
{
    BOARD* brd = myframe->GetBoard();

    LAYER_MSK visibleLayers = brd->GetVisibleLayers();

    if( isVisible )
        visibleLayers |= GetLayerMask( aLayer );
    else
        visibleLayers &= ~GetLayerMask( aLayer );

    brd->SetVisibleLayers( visibleLayers );

    EDA_DRAW_PANEL_GAL* galCanvas = myframe->GetGalCanvas();
    if( galCanvas )
    {
        KiGfx::VIEW* view = galCanvas->GetView();
        view->SetLayerVisible( aLayer, isVisible );
    }

    if( isFinal )
        myframe->RefreshCanvas();
}

void PCB_LAYER_WIDGET::OnRenderColorChange( int aId, EDA_COLOR_T aColor )
{
    myframe->GetBoard()->SetVisibleElementColor( aId, aColor );
    myframe->RefreshCanvas();
}

void PCB_LAYER_WIDGET::OnRenderEnable( int aId, bool isEnabled )
{
    BOARD*  brd = myframe->GetBoard();
    brd->SetElementVisibility( aId, isEnabled );

    EDA_DRAW_PANEL_GAL *galCanvas = myframe->GetGalCanvas();
    if( galCanvas )
    {
        KiGfx::VIEW* view = galCanvas->GetView();
        view->SetLayerVisible( ITEM_GAL_LAYER( aId ), isEnabled );
    }

    if( myframe->IsGalCanvasActive() )
        galCanvas->Refresh();
    else
        myframe->RefreshCanvas();
}

//-----</LAYER_WIDGET callbacks>------------------------------------------
