/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file basepcbframe.cpp
 */

#include <fctsys.h>
#include <wxstruct.h>
#include <pcbcommon.h>
#include <confirm.h>
#include <appl_wxstruct.h>
#include <dialog_helpers.h>
#include <kicad_device_context.h>
#include <wxBasePcbFrame.h>
#include <base_units.h>
#include <msgpanel.h>

#include <pcbnew.h>
#include <pcbnew_id.h>
#include <class_board.h>

#include <collectors.h>
#include <class_drawpanel.h>
#include <vector2d.h>


// Configuration entry names.
static const wxString UserGridSizeXEntry( wxT( "PcbUserGrid_X" ) );
static const wxString UserGridSizeYEntry( wxT( "PcbUserGrid_Y" ) );
static const wxString UserGridUnitsEntry( wxT( "PcbUserGrid_Unit" ) );
static const wxString DisplayPadFillEntry( wxT( "DiPadFi" ) );
static const wxString DisplayViaFillEntry( wxT( "DiViaFi" ) );
static const wxString DisplayPadNumberEntry( wxT( "DiPadNu" ) );
static const wxString DisplayModuleEdgeEntry( wxT( "DiModEd" ) );
static const wxString DisplayModuleTextEntry( wxT( "DiModTx" ) );
static const wxString FastGrid1Entry( wxT( "FastGrid1" ) );
static const wxString FastGrid2Entry( wxT( "FastGrid2" ) );


BEGIN_EVENT_TABLE( PCB_BASE_FRAME, EDA_DRAW_FRAME )
    EVT_MENU_RANGE( ID_POPUP_PCB_ITEM_SELECTION_START, ID_POPUP_PCB_ITEM_SELECTION_END,
                    PCB_BASE_FRAME::ProcessItemSelection )

    EVT_TOOL( ID_TB_OPTIONS_SHOW_POLAR_COORD, PCB_BASE_FRAME::OnTogglePolarCoords )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_PADS_SKETCH, PCB_BASE_FRAME::OnTogglePadDrawMode )

    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_POLAR_COORD, PCB_BASE_FRAME::OnUpdateCoordType )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_PADS_SKETCH, PCB_BASE_FRAME::OnUpdatePadDrawMode )
    EVT_UPDATE_UI( ID_ON_GRID_SELECT, PCB_BASE_FRAME::OnUpdateSelectGrid )
    EVT_UPDATE_UI( ID_ON_ZOOM_SELECT, PCB_BASE_FRAME::OnUpdateSelectZoom )

    EVT_UPDATE_UI_RANGE( ID_ZOOM_IN, ID_ZOOM_PAGE, PCB_BASE_FRAME::OnUpdateSelectZoom )
END_EVENT_TABLE()


PCB_BASE_FRAME::PCB_BASE_FRAME( wxWindow* aParent, ID_DRAWFRAME_TYPE aFrameType,
                                const wxString& aTitle,
                                const wxPoint& aPos, const wxSize& aSize,
                                long aStyle, const wxString & aFrameName) :
    EDA_DRAW_FRAME( aParent, aFrameType, aTitle, aPos, aSize, aStyle, aFrameName )
{
    m_Pcb                 = NULL;

    m_DisplayPadFill      = true;   // How to draw pads
    m_DisplayViaFill      = true;   // How to draw vias
    m_DisplayPadNum       = true;   // show pads number

    m_DisplayModEdge      = FILLED; // How to display module drawings (line/ filled / sketch)
    m_DisplayModText      = FILLED; // How to display module texts (line/ filled / sketch)
    m_DisplayPcbTrackFill = true;   // false = sketch , true = filled
    m_Draw3DFrame         = NULL;   // Display Window in 3D mode (OpenGL)

    m_UserGridSize        = wxRealPoint( 100.0, 100.0 );
    m_UserGridUnit        = INCHES;
    m_Collector           = new GENERAL_COLLECTOR();

    m_FastGrid1           = 0;
    m_FastGrid2           = 0;

    m_auxiliaryToolBar    = NULL;
}


PCB_BASE_FRAME::~PCB_BASE_FRAME()
{
    delete m_Collector;

    delete m_Pcb;       // is already NULL for FOOTPRINT_EDIT_FRAME
}


void PCB_BASE_FRAME::SetBoard( BOARD* aBoard )
{
    delete m_Pcb;
    m_Pcb = aBoard;
}


void PCB_BASE_FRAME::SetPageSettings( const PAGE_INFO& aPageSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetPageSettings( aPageSettings );

    if( GetScreen() )
        GetScreen()->InitDataPoints( aPageSettings.GetSizeIU() );
}


const PAGE_INFO& PCB_BASE_FRAME::GetPageSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetPageSettings();
}


const wxSize PCB_BASE_FRAME::GetPageSizeIU() const
{
    wxASSERT( m_Pcb );

    // this function is only needed because EDA_DRAW_FRAME is not compiled
    // with either -DPCBNEW or -DEESCHEMA, so the virtual is used to route
    // into an application specific source file.
    return m_Pcb->GetPageSettings().GetSizeIU();
}


const wxPoint& PCB_BASE_FRAME::GetOriginAxisPosition() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetOriginAxisPosition();
}


void PCB_BASE_FRAME::SetOriginAxisPosition( const wxPoint& aPosition )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetOriginAxisPosition( aPosition );
}


const TITLE_BLOCK& PCB_BASE_FRAME::GetTitleBlock() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetTitleBlock();
}


void PCB_BASE_FRAME::SetTitleBlock( const TITLE_BLOCK& aTitleBlock )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetTitleBlock( aTitleBlock );
}


BOARD_DESIGN_SETTINGS& PCB_BASE_FRAME::GetDesignSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetDesignSettings();
}


void PCB_BASE_FRAME::SetDesignSettings( const BOARD_DESIGN_SETTINGS& aSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetDesignSettings( aSettings );
}


const ZONE_SETTINGS& PCB_BASE_FRAME::GetZoneSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetZoneSettings();
}


void PCB_BASE_FRAME::SetZoneSettings( const ZONE_SETTINGS& aSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetZoneSettings( aSettings );
}


const PCB_PLOT_PARAMS& PCB_BASE_FRAME::GetPlotSettings() const
{
    wxASSERT( m_Pcb );
    return m_Pcb->GetPlotOptions();
}


void PCB_BASE_FRAME::SetPlotSettings( const PCB_PLOT_PARAMS& aSettings )
{
    wxASSERT( m_Pcb );
    m_Pcb->SetPlotOptions( aSettings );
}


EDA_RECT PCB_BASE_FRAME::GetBoardBoundingBox( bool aBoardEdgesOnly ) const
{
    wxASSERT( m_Pcb );

    EDA_RECT area = m_Pcb->ComputeBoundingBox( aBoardEdgesOnly );

    if( area.GetWidth() == 0 && area.GetHeight() == 0 )
    {
        wxSize pageSize = GetPageSizeIU();

        if( m_showBorderAndTitleBlock )
        {
            area.SetOrigin( 0, 0 );
            area.SetEnd( pageSize.x, pageSize.y );
        }
        else
        {
            area.SetOrigin( -pageSize.x / 2, -pageSize.y / 2 );
            area.SetEnd( pageSize.x / 2, pageSize.y / 2 );
        }
    }

    return area;
}


double PCB_BASE_FRAME::BestZoom()
{
    if( m_Pcb == NULL )
        return 1.0;

    EDA_RECT    ibbbox  = GetBoardBoundingBox();
    DSIZE       clientz = m_canvas->GetClientSize();
    DSIZE       boardz( ibbbox.GetWidth(), ibbbox.GetHeight() );

    double iu_per_du_X = clientz.x ? boardz.x / clientz.x : 1.0;
    double iu_per_du_Y = clientz.y ? boardz.y / clientz.y : 1.0;

    double bestzoom = std::max( iu_per_du_X, iu_per_du_Y );

    GetScreen()->SetScrollCenterPosition( ibbbox.Centre() );

    return bestzoom;
}


void PCB_BASE_FRAME::CursorGoto( const wxPoint& aPos, bool aWarp )
{
    // factored out of pcbnew/find.cpp

    PCB_SCREEN* screen = (PCB_SCREEN*)GetScreen();

    INSTALL_UNBUFFERED_DC( dc, m_canvas );

    // There may be need to reframe the drawing.
    if( !m_canvas->IsPointOnDisplay( aPos ) )
    {
        screen->SetCrossHairPosition( aPos );
        RedrawScreen( aPos, aWarp );
    }
    else
    {
        // Put cursor on item position
        m_canvas->CrossHairOff( &dc );
        screen->SetCrossHairPosition( aPos );

        if( aWarp )
            m_canvas->MoveCursorToCrossHair();
    }
    m_canvas->CrossHairOn( &dc );
    m_canvas->CrossHairOn( &dc );
}


// Virtual function
void PCB_BASE_FRAME::ReCreateMenuBar( void )
{
}


// Virtual functions: Do nothing for PCB_BASE_FRAME window
void PCB_BASE_FRAME::Show3D_Frame( wxCommandEvent& event )
{
}


// Note: virtual, overridden in PCB_EDIT_FRAME;
void PCB_BASE_FRAME::SwitchLayer( wxDC* DC, int layer )
{
    int preslayer = ((PCB_SCREEN*)GetScreen())->m_Active_Layer;

    // Check if the specified layer matches the present layer
    if( layer == preslayer )
        return;

    // Copper layers cannot be selected unconditionally; how many
    // of those layers are currently enabled needs to be checked.
    if( IsValidCopperLayerIndex( layer ) )
    {
        // If only one copper layer is enabled, the only such layer
        // that can be selected to is the "Copper" layer (so the
        // selection of any other copper layer is disregarded).
        if( m_Pcb->GetCopperLayerCount() < 2 )
        {
            if( layer != LAYER_N_BACK )
            {
                return;
            }
        }

        // If more than one copper layer is enabled, the "Copper"
        // and "Component" layers can be selected, but the total
        // number of copper layers determines which internal
        // layers are also capable of being selected.
        else
        {
            if( ( layer != LAYER_N_BACK ) && ( layer != LAYER_N_FRONT )
                && ( layer >= m_Pcb->GetCopperLayerCount() - 1 ) )
            {
                return;
            }
        }
    }

    // Is yet more checking required? E.g. when the layer to be selected
    // is a non-copper layer, or when switching between a copper layer
    // and a non-copper layer, or vice-versa?
    // ...

    GetScreen()->m_Active_Layer = layer;

    if( DisplayOpt.ContrastModeDisplay )
        m_canvas->Refresh();
}


void PCB_BASE_FRAME::OnTogglePolarCoords( wxCommandEvent& aEvent )
{
    SetStatusText( wxEmptyString );
    DisplayOpt.DisplayPolarCood = !DisplayOpt.DisplayPolarCood;
    UpdateStatusBar();
}


void PCB_BASE_FRAME::OnTogglePadDrawMode( wxCommandEvent& aEvent )
{
    m_DisplayPadFill = DisplayOpt.DisplayPadFill = !m_DisplayPadFill;
    m_canvas->Refresh();
}


void PCB_BASE_FRAME::OnUpdateCoordType( wxUpdateUIEvent& aEvent )
{
    aEvent.Check( DisplayOpt.DisplayPolarCood );
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_SHOW_POLAR_COORD,
                                        DisplayOpt.DisplayPolarCood ?
                                        _( "Display rectangular coordinates" ) :
                                        _( "Display polar coordinates" ) );
}


void PCB_BASE_FRAME::OnUpdatePadDrawMode( wxUpdateUIEvent& aEvent )
{
    aEvent.Check( !m_DisplayPadFill );
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_SHOW_PADS_SKETCH,
                                        m_DisplayPadFill ?
                                        _( "Show pads in outline mode" ) :
                                        _( "Show pads in fill mode" ) );
}


void PCB_BASE_FRAME::OnUpdateSelectGrid( wxUpdateUIEvent& aEvent )
{
    // No need to update the grid select box if it doesn't exist or the grid setting change
    // was made using the select box.
    if( m_gridSelectBox == NULL || m_auxiliaryToolBar == NULL )
        return;

    int select = wxNOT_FOUND;

    for( size_t i = 0; i < GetScreen()->GetGridCount(); i++ )
    {
        if( GetScreen()->GetGridId() == GetScreen()->GetGrid( i ).m_Id )
        {
            select = (int) i;
            break;
        }
    }

    if( select != m_gridSelectBox->GetSelection() )
        m_gridSelectBox->SetSelection( select );
}


void PCB_BASE_FRAME::OnUpdateSelectZoom( wxUpdateUIEvent& aEvent )
{
    if( m_zoomSelectBox == NULL || m_auxiliaryToolBar == NULL )
        return;

    int current = 0;

    for( unsigned i = 0; i < GetScreen()->m_ZoomList.size(); i++ )
    {
        if( GetScreen()->GetZoom() == GetScreen()->m_ZoomList[i] )
        {
            current = i + 1;
            break;
        }
    }

    if( current != m_zoomSelectBox->GetSelection() )
        m_zoomSelectBox->SetSelection( current );
}


void PCB_BASE_FRAME::ProcessItemSelection( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();

    // index into the collector list:
    int itemNdx = id - ID_POPUP_PCB_ITEM_SELECTION_START;

    if( id >= ID_POPUP_PCB_ITEM_SELECTION_START && id <= ID_POPUP_PCB_ITEM_SELECTION_END )
    {
        BOARD_ITEM* item = (*m_Collector)[itemNdx];
        m_canvas->SetAbortRequest( false );

#if 0 && defined (DEBUG)
        item->Show( 0, std::cout );
#endif

        SetCurItem( item );
    }
}


void PCB_BASE_FRAME::SetCurItem( BOARD_ITEM* aItem, bool aDisplayInfo )
{
    GetScreen()->SetCurItem( aItem );

    if( aItem )
    {
        if( aDisplayInfo )
        {
            MSG_PANEL_ITEMS items;
            aItem->GetMsgPanelInfo( items );
            SetMsgPanel( items );
        }

#if 0 && defined(DEBUG)
    aItem->Show( 0, std::cout );
#endif

    }
    else
    {
        // we can use either of these two:

        MSG_PANEL_ITEMS items;
        m_Pcb->GetMsgPanelInfo( items );       // show the BOARD stuff
        SetMsgPanel( items );

#if 0 && defined(DEBUG)
        std::cout << "SetCurItem(NULL)\n";
#endif

    }
}


BOARD_ITEM* PCB_BASE_FRAME::GetCurItem()
{
    return GetScreen()->GetCurItem();
}


GENERAL_COLLECTORS_GUIDE PCB_BASE_FRAME::GetCollectorsGuide()
{
    GENERAL_COLLECTORS_GUIDE guide( m_Pcb->GetVisibleLayers(),
                                    ( (PCB_SCREEN*)GetScreen())->m_Active_Layer );

    // account for the globals
    guide.SetIgnoreMTextsMarkedNoShow( ! m_Pcb->IsElementVisible( MOD_TEXT_INVISIBLE ));
    guide.SetIgnoreMTextsOnCopper( ! m_Pcb->IsElementVisible( MOD_TEXT_BK_VISIBLE ));
    guide.SetIgnoreMTextsOnCmp( ! m_Pcb->IsElementVisible( MOD_TEXT_FR_VISIBLE ));
    guide.SetIgnoreModulesOnCu( ! m_Pcb->IsElementVisible( MOD_BK_VISIBLE ) );
    guide.SetIgnoreModulesOnCmp( ! m_Pcb->IsElementVisible( MOD_FR_VISIBLE ) );
    guide.SetIgnorePadsOnBack( ! m_Pcb->IsElementVisible( PAD_BK_VISIBLE ) );
    guide.SetIgnorePadsOnFront( ! m_Pcb->IsElementVisible( PAD_FR_VISIBLE ) );
    guide.SetIgnoreModulesVals( ! m_Pcb->IsElementVisible( MOD_VALUES_VISIBLE ) );
    guide.SetIgnoreModulesRefs( ! m_Pcb->IsElementVisible( MOD_REFERENCES_VISIBLE ) );

    return guide;
}

void PCB_BASE_FRAME::SetToolID( int aId, int aCursor, const wxString& aToolMsg )
{
    bool redraw = false;

    EDA_DRAW_FRAME::SetToolID( aId, aCursor, aToolMsg );

    if( aId < 0 )
        return;

    // handle color changes for transitions in and out of ID_TRACK_BUTT
    if( ( GetToolId() == ID_TRACK_BUTT && aId != ID_TRACK_BUTT )
        || ( GetToolId() != ID_TRACK_BUTT && aId== ID_TRACK_BUTT ) )
    {
        if( DisplayOpt.ContrastModeDisplay )
            redraw = true;
    }

    // must do this after the tool has been set, otherwise pad::Draw() does
    // not show proper color when DisplayOpt.ContrastModeDisplay is true.
    if( redraw && m_canvas)
        m_canvas->Refresh();
}


/*
 * Update the status bar information.
 */
void PCB_BASE_FRAME::UpdateStatusBar()
{
    EDA_DRAW_FRAME::UpdateStatusBar();

    PCB_SCREEN* screen = GetScreen();

    if( !screen )
        return;

    int dx;
    int dy;
    double dXpos;
    double dYpos;
    wxString line;
    wxString locformatter;

    if( DisplayOpt.DisplayPolarCood )  // display polar coordinates
    {
        double       theta, ro;

        dx = screen->GetCrossHairPosition().x - screen->m_O_Curseur.x;
        dy = screen->GetCrossHairPosition().y - screen->m_O_Curseur.y;

        if( dx==0 && dy==0 )
            theta = 0.0;
        else
            theta = atan2( (double) -dy, (double) dx );

        theta = theta * 180.0 / M_PI;

        ro = sqrt( ( (double) dx * dx ) + ( (double) dy * dy ) );
        wxString formatter;
        switch( g_UserUnit )
        {
#if defined( USE_PCBNEW_NANOMETRE )
        case INCHES:
            formatter = wxT( "Ro %.6f Th %.1f" );
            break;

        case MILLIMETRES:
            formatter = wxT( "Ro %.6f Th %.1f" );
            break;
#else
        case INCHES:
            formatter = wxT( "Ro %.4f Th %.1f" );
            break;

        case MILLIMETRES:
            formatter = wxT( "Ro %.3f Th %.1f" );
            break;
#endif

        case UNSCALED_UNITS:
            formatter = wxT( "Ro %f Th %f" );
            break;
        }

        line.Printf( formatter, To_User_Unit( g_UserUnit, ro ), theta );

        SetStatusText( line, 3 );
    }

    // Display absolute coordinates:
    dXpos = To_User_Unit( g_UserUnit, screen->GetCrossHairPosition().x );
    dYpos = To_User_Unit( g_UserUnit, screen->GetCrossHairPosition().y );

    // The following sadly is an if Eeschema/if Pcbnew
    wxString absformatter;

    switch( g_UserUnit )
    {
#if defined( USE_PCBNEW_NANOMETRES )
    case INCHES:
        absformatter = wxT( "X %.6f  Y %.6f" );
        locformatter = wxT( "dx %.6f  dy %.6f  d %.6f" );
        break;

    case MILLIMETRES:
        absformatter = wxT( "X %.6f  Y %.6f" );
        locformatter = wxT( "dx %.6f  dy %.6f  d %.6f" );
        break;
#else
    case INCHES:
        absformatter = wxT( "X %.4f  Y %.4f" );
        locformatter = wxT( "dx %.4f  dy %.4f  d %.4f" );
        break;

    case MILLIMETRES:
        dXpos = RoundTo0( dXpos, 1000.0 );
        dYpos = RoundTo0( dYpos, 1000.0 );
        absformatter = wxT( "X %.3f  Y %.3f" );
        locformatter = wxT( "dx %.3f  dy %.3f  d %.3f" );
        break;
#endif

    case UNSCALED_UNITS:
        absformatter = wxT( "X %f  Y %f" );
        locformatter = wxT( "dx %f  dy %f  d %f" );
        break;
    }

    line.Printf( absformatter, dXpos, dYpos );
    SetStatusText( line, 2 );

    if( !DisplayOpt.DisplayPolarCood )  // display relative cartesian coordinates
    {
        // Display relative coordinates:
        dx = screen->GetCrossHairPosition().x - screen->m_O_Curseur.x;
        dy = screen->GetCrossHairPosition().y - screen->m_O_Curseur.y;
        dXpos = To_User_Unit( g_UserUnit, dx );
        dYpos = To_User_Unit( g_UserUnit, dy );

#ifndef USE_PCBNEW_NANOMETRES
        if ( g_UserUnit == MILLIMETRES )
        {
            dXpos = RoundTo0( dXpos, 1000.0 );
            dYpos = RoundTo0( dYpos, 1000.0 );
        }
#endif

        // We already decided the formatter above
        line.Printf( locformatter, dXpos, dYpos, sqrt( dXpos * dXpos + dYpos * dYpos ) );
        SetStatusText( line, 3 );
    }
}


void PCB_BASE_FRAME::unitsChangeRefresh()
{
    EDA_DRAW_FRAME::unitsChangeRefresh();    // Update the status bar.

    updateGridSelectBox();
}


void PCB_BASE_FRAME::LoadSettings()
{
    wxASSERT( wxGetApp().GetSettings() != NULL );

    wxConfig* cfg = wxGetApp().GetSettings();

    EDA_DRAW_FRAME::LoadSettings();

    // Ensure grid id is an existent grid id:
    if( (m_LastGridSizeId <= 0) ||
        (m_LastGridSizeId > (ID_POPUP_GRID_USER - ID_POPUP_GRID_LEVEL_1000)) )
        m_LastGridSizeId = ID_POPUP_GRID_LEVEL_500 - ID_POPUP_GRID_LEVEL_1000;

    cfg->Read( m_FrameName + UserGridSizeXEntry, &m_UserGridSize.x, 0.01 );
    cfg->Read( m_FrameName + UserGridSizeYEntry, &m_UserGridSize.y, 0.01 );

    long itmp;
    cfg->Read( m_FrameName + UserGridUnitsEntry, &itmp, ( long )INCHES );
    m_UserGridUnit = (EDA_UNITS_T) itmp;
    cfg->Read( m_FrameName + DisplayPadFillEntry, &m_DisplayPadFill, true );
    cfg->Read( m_FrameName + DisplayViaFillEntry, &m_DisplayViaFill, true );
    cfg->Read( m_FrameName + DisplayPadNumberEntry, &m_DisplayPadNum, true );
    cfg->Read( m_FrameName + DisplayModuleEdgeEntry, &m_DisplayModEdge, ( long )FILLED );

    cfg->Read( m_FrameName + FastGrid1Entry, &itmp, ( long )0);
    m_FastGrid1 = itmp;
    cfg->Read( m_FrameName + FastGrid2Entry, &itmp, ( long )0);
    m_FastGrid2 = itmp;

    if( m_DisplayModEdge < LINE || m_DisplayModEdge > SKETCH )
        m_DisplayModEdge = FILLED;

    cfg->Read( m_FrameName + DisplayModuleTextEntry, &m_DisplayModText, ( long )FILLED );

    if( m_DisplayModText < LINE || m_DisplayModText > SKETCH )
        m_DisplayModText = FILLED;

    // WxWidgets 2.9.1 seems call setlocale( LC_NUMERIC, "" )
    // when reading doubles in config,
    // but forget to back to current locale. So we call SetLocaleTo_Default
    SetLocaleTo_Default( );
}


void PCB_BASE_FRAME::SaveSettings()
{
    wxASSERT( wxGetApp().GetSettings() != NULL );

    wxConfig* cfg = wxGetApp().GetSettings();

    EDA_DRAW_FRAME::SaveSettings();
    cfg->Write( m_FrameName + UserGridSizeXEntry, m_UserGridSize.x );
    cfg->Write( m_FrameName + UserGridSizeYEntry, m_UserGridSize.y );
    cfg->Write( m_FrameName + UserGridUnitsEntry, ( long )m_UserGridUnit );
    cfg->Write( m_FrameName + DisplayPadFillEntry, m_DisplayPadFill );
    cfg->Write( m_FrameName + DisplayViaFillEntry, m_DisplayViaFill );
    cfg->Write( m_FrameName + DisplayPadNumberEntry, m_DisplayPadNum );
    cfg->Write( m_FrameName + DisplayModuleEdgeEntry, ( long )m_DisplayModEdge );
    cfg->Write( m_FrameName + DisplayModuleTextEntry, ( long )m_DisplayModText );
    cfg->Write( m_FrameName + FastGrid1Entry, ( long )m_FastGrid1 );
    cfg->Write( m_FrameName + FastGrid2Entry, ( long )m_FastGrid2 );
}


void PCB_BASE_FRAME::OnModify()
{
    GetScreen()->SetModify();
    GetScreen()->SetSave();

    wxASSERT( m_Pcb );
    m_Pcb->GetTitleBlock().SetDate();
}


void PCB_BASE_FRAME::updateGridSelectBox()
{
    UpdateStatusBar();
    DisplayUnitsMsg();

    if( m_gridSelectBox == NULL )
        return;

    // Update grid values with the current units setting.
    m_gridSelectBox->Clear();

    wxString msg;
    wxString format = _( "Grid:");

    switch( g_UserUnit )
    {
    case INCHES:    // the grid size is displayed in mils
    case MILLIMETRES:
        format += wxT( " %.6f" );
        break;

    case UNSCALED_UNITS:
        format += wxT( " %f" );
        break;
    }

    for( size_t i = 0; i < GetScreen()->GetGridCount(); i++ )
    {
        GRID_TYPE& grid = GetScreen()->GetGrid( i );
        double value = To_User_Unit( g_UserUnit, grid.m_Size.x );
        if( g_UserUnit == INCHES )
            value *= 1000;

        if( grid.m_Id != ID_POPUP_GRID_USER )
        {
            msg.Printf( format.GetData(), value );
            StripTrailingZeros( msg );
        }
        else
            msg = _( "User Grid" );

        m_gridSelectBox->Append( msg, (void*) &grid.m_Id );

        if( ( m_LastGridSizeId + ID_POPUP_GRID_LEVEL_1000 ) == GetScreen()->GetGrid( i ).m_Id )
            m_gridSelectBox->SetSelection( i );
    }
}

void PCB_BASE_FRAME::updateZoomSelectBox()
{
    if( m_zoomSelectBox == NULL )
        return;

    wxString msg;

    m_zoomSelectBox->Clear();
    m_zoomSelectBox->Append( _( "Auto" ) );
    m_zoomSelectBox->SetSelection( 0 );

    for( unsigned i = 0;  i < GetScreen()->m_ZoomList.size();  ++i )
    {
        msg = _( "Zoom " );

        wxString value = wxString::Format( wxT( "%g" ),

                                // @todo could do scaling here and show a "percentage"
                                GetScreen()->m_ZoomList[i]
                                );

        msg += value;

        m_zoomSelectBox->Append( msg );

        if( GetScreen()->GetZoom() == GetScreen()->m_ZoomList[i] )
            m_zoomSelectBox->SetSelection( i + 1 );
    }
}
