/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2012 KiCad Developers, see change_log.txt for contributors.
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
 * @file modview_frame.cpp
 */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <class_drawpanel.h>
#include <wxPcbStruct.h>
#include <3d_viewer.h>
#include <pcbcommon.h>
#include <msgpanel.h>

#include <class_board.h>
#include <class_module.h>

#include <pcbnew.h>
#include <pcbnew_id.h>
#include <modview_frame.h>
#include <footprint_info.h>

#include <hotkeys.h>
#include <wildcards_and_files_ext.h>


/**
 * Save previous component library viewer state.
 */
wxString FOOTPRINT_VIEWER_FRAME::m_libraryName;
wxString FOOTPRINT_VIEWER_FRAME::m_footprintName;


/// When the viewer is used to select a component in schematic, the selected component is here.
wxString FOOTPRINT_VIEWER_FRAME::m_selectedFootprintName;


BEGIN_EVENT_TABLE( FOOTPRINT_VIEWER_FRAME, EDA_DRAW_FRAME )
    /* Window events */
    EVT_CLOSE( FOOTPRINT_VIEWER_FRAME::OnCloseWindow )
    EVT_SIZE( FOOTPRINT_VIEWER_FRAME::OnSize )
    EVT_ACTIVATE( FOOTPRINT_VIEWER_FRAME::OnActivate )

    /* Sash drag events */
    EVT_SASH_DRAGGED( ID_MODVIEW_LIBWINDOW, FOOTPRINT_VIEWER_FRAME::OnSashDrag )
    EVT_SASH_DRAGGED( ID_MODVIEW_FOOTPRINT_WINDOW, FOOTPRINT_VIEWER_FRAME::OnSashDrag )

    /* Toolbar events */
    EVT_TOOL( ID_MODVIEW_SELECT_LIB,
              FOOTPRINT_VIEWER_FRAME::SelectCurrentLibrary )
    EVT_TOOL( ID_MODVIEW_SELECT_PART,
              FOOTPRINT_VIEWER_FRAME::SelectCurrentFootprint )
    EVT_TOOL( ID_MODVIEW_NEXT,
              FOOTPRINT_VIEWER_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_MODVIEW_PREVIOUS,
              FOOTPRINT_VIEWER_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_MODVIEW_FOOTPRINT_EXPORT_TO_BOARD,
              FOOTPRINT_VIEWER_FRAME::ExportSelectedFootprint )
    EVT_TOOL( ID_MODVIEW_SHOW_3D_VIEW, FOOTPRINT_VIEWER_FRAME::Show3D_Frame )

    /* listbox events */
    EVT_LISTBOX( ID_MODVIEW_LIB_LIST, FOOTPRINT_VIEWER_FRAME::ClickOnLibList )
    EVT_LISTBOX( ID_MODVIEW_FOOTPRINT_LIST, FOOTPRINT_VIEWER_FRAME::ClickOnFootprintList )
    EVT_LISTBOX_DCLICK( ID_MODVIEW_FOOTPRINT_LIST, FOOTPRINT_VIEWER_FRAME::DClickOnFootprintList )

    EVT_MENU( ID_SET_RELATIVE_OFFSET, FOOTPRINT_VIEWER_FRAME::OnSetRelativeOffset )
END_EVENT_TABLE()


/*
 * This emulates the zoom menu entries found in the other KiCad applications.
 * The library viewer does not have any menus so add an accelerator table to
 * the main frame.
 */
static wxAcceleratorEntry accels[] =
{
    wxAcceleratorEntry( wxACCEL_NORMAL, WXK_F1, ID_ZOOM_IN ),
    wxAcceleratorEntry( wxACCEL_NORMAL, WXK_F2, ID_ZOOM_OUT ),
    wxAcceleratorEntry( wxACCEL_NORMAL, WXK_F3, ID_ZOOM_REDRAW ),
    wxAcceleratorEntry( wxACCEL_NORMAL, WXK_F4, ID_POPUP_ZOOM_CENTER ),
    wxAcceleratorEntry( wxACCEL_NORMAL, WXK_HOME, ID_ZOOM_PAGE ),
    wxAcceleratorEntry( wxACCEL_NORMAL, WXK_SPACE, ID_SET_RELATIVE_OFFSET )
};

#define ACCEL_TABLE_CNT ( sizeof( accels ) / sizeof( wxAcceleratorEntry ) )

#define EXTRA_BORDER_SIZE 2

#define FOOTPRINT_VIEWER_FRAME_NAME wxT( "ModViewFrame" )

FOOTPRINT_VIEWER_FRAME::FOOTPRINT_VIEWER_FRAME( PCB_BASE_FRAME* parent,
                                                wxSemaphore* semaphore, long style ) :
    PCB_BASE_FRAME( parent, MODULE_VIEWER_FRAME_TYPE, _( "Footprint Library Browser" ),
                    wxDefaultPosition, wxDefaultSize, style, GetFootprintViewerFrameName() )
{
    wxAcceleratorTable table( ACCEL_TABLE_CNT, accels );

    m_FrameName = GetFootprintViewerFrameName();
    m_configPath = wxT( "FootprintViewer" );
    m_showAxis = true;         // true to draw axis.

    // Give an icon
    wxIcon  icon;
    icon.CopyFromBitmap( KiBitmap( modview_icon_xpm ) );
    SetIcon( icon );

    m_HotkeysZoomAndGridList = g_Module_Viewer_Hokeys_Descr;
    m_FootprintList = NULL;
    m_LibList = NULL;
    m_LibListWindow = NULL;
    m_FootprintListWindow = NULL;
    m_Semaphore     = semaphore;
    m_selectedFootprintName.Empty();

    if( m_Semaphore )
        SetModalMode(true);

    SetBoard( new BOARD() );
    // Ensure all layers and items are visible:
    GetBoard()->SetVisibleAlls();
    SetScreen( new PCB_SCREEN(GetPageSizeIU()) );
    GetScreen()->m_Center = true;      // Center coordinate origins on screen.
    LoadSettings();

    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );
    GetScreen()->SetGrid( ID_POPUP_GRID_LEVEL_1000 + m_LastGridSizeId  );

    ReCreateHToolbar();
    ReCreateVToolbar();

    wxSize  size = GetClientSize();
    size.y -= m_MsgFrameHeight + 2;

    m_LibListSize.y = -1;

    wxPoint win_pos( 0, 0 );

     // Creates the libraries window display
    m_LibListWindow =
        new wxSashLayoutWindow( this, ID_MODVIEW_LIBWINDOW, win_pos,
                                wxDefaultSize, wxCLIP_CHILDREN | wxSW_3D,
                                wxT( "LibWindow" ) );
    m_LibListWindow->SetOrientation( wxLAYOUT_VERTICAL );
    m_LibListWindow->SetAlignment( wxLAYOUT_LEFT );
    m_LibListWindow->SetSashVisible( wxSASH_RIGHT, true );
    m_LibListWindow->SetExtraBorderSize( EXTRA_BORDER_SIZE );
    m_LibList = new wxListBox( m_LibListWindow, ID_MODVIEW_LIB_LIST,
                               wxPoint( 0, 0 ), wxDefaultSize,
                               0, NULL, wxLB_HSCROLL );

    // Creates the component window display
    m_FootprintListSize.y = size.y;
    win_pos.x = m_LibListSize.x;
    m_FootprintListWindow = new wxSashLayoutWindow( this, ID_MODVIEW_FOOTPRINT_WINDOW,
                                              win_pos, wxDefaultSize,
                                              wxCLIP_CHILDREN | wxSW_3D,
                                              wxT( "CmpWindow" ) );
    m_FootprintListWindow->SetOrientation( wxLAYOUT_VERTICAL );

    m_FootprintListWindow->SetSashVisible( wxSASH_RIGHT, true );
    m_FootprintListWindow->SetExtraBorderSize( EXTRA_BORDER_SIZE );
    m_FootprintList = new wxListBox( m_FootprintListWindow, ID_MODVIEW_FOOTPRINT_LIST,
                               wxPoint( 0, 0 ), wxDefaultSize,
                               0, NULL, wxLB_HSCROLL );

    ReCreateLibraryList();

    DisplayLibInfos();

    // If a footprint was previsiously loaded, reload it
    if( !m_libraryName.IsEmpty() && !m_footprintName.IsEmpty() )
        GetModuleLibrary( m_libraryName + wxT(".") + LegacyFootprintLibPathExtension,
                      m_footprintName, false );


    if( m_canvas )
        m_canvas->SetAcceleratorTable( table );

    m_auimgr.SetManagedWindow( this );


    EDA_PANEINFO horiz;
    horiz.HorizontalToolbarPane();

    EDA_PANEINFO vert;
    vert.VerticalToolbarPane();

    EDA_PANEINFO info;
    info.InfoToolbarPane();

    EDA_PANEINFO mesg;
    mesg.MessageToolbarPane();


    // Manage main toolbal
    m_auimgr.AddPane( m_mainToolBar,
                      wxAuiPaneInfo( horiz ).Name( wxT ("m_mainToolBar" ) ).Top().Row( 0 ) );

    wxSize minsize( 60, -1 );

    // Manage the left window (list of libraries)
    if( m_LibListWindow )
        m_auimgr.AddPane( m_LibListWindow, wxAuiPaneInfo( info ).Name( wxT( "m_LibList" ) ).
                          Left().Row( 0 ));

    // Manage the list of components)
    m_auimgr.AddPane( m_FootprintListWindow,
                      wxAuiPaneInfo( info ).Name( wxT( "m_FootprintList" ) ).
                      Left().Row( 1 ) );

    // Manage the draw panel
    m_auimgr.AddPane( m_canvas,
                      wxAuiPaneInfo().Name( wxT( "DrawFrame" ) ).Centre() );

    // Manage the message panel
    m_auimgr.AddPane( m_messagePanel,
                      wxAuiPaneInfo( mesg ).Name( wxT( "MsgPanel" ) ).Bottom().Layer(10) );

    /* Now the minimum windows are fixed, set library list
     * and component list of the previous values from last viewlib use
     */
    if( m_LibListWindow )
    {
        wxAuiPaneInfo& pane = m_auimgr.GetPane(m_LibListWindow);
        pane.MinSize( wxSize(m_LibListSize.x, -1));
    }
    wxAuiPaneInfo& pane = m_auimgr.GetPane(m_FootprintListWindow);
    pane.MinSize(wxSize(m_FootprintListSize.x, -1));

    m_auimgr.Update();

    // Now Drawpanel is sized, we can use BestZoom to show the component (if any)
#ifdef USE_WX_GRAPHICS_CONTEXT
    GetScreen()->SetZoom( BestZoom() );
#else
    Zoom_Automatique( false );
#endif

    Show( true );
}


FOOTPRINT_VIEWER_FRAME::~FOOTPRINT_VIEWER_FRAME()
{
    if( m_Draw3DFrame )
        m_Draw3DFrame->Destroy();
}

/* return the frame name used when creating the frame
 * used to get a reference to this frame, if exists
 */
const wxChar* FOOTPRINT_VIEWER_FRAME::GetFootprintViewerFrameName()
{
    return FOOTPRINT_VIEWER_FRAME_NAME;
}

/* return a reference to the current opened Footprint viewer
 * or NULL if no Footprint viewer currently opened
 */
FOOTPRINT_VIEWER_FRAME* FOOTPRINT_VIEWER_FRAME::GetActiveFootprintViewer()
{
    return (FOOTPRINT_VIEWER_FRAME*)
            wxWindow::FindWindowByName(GetFootprintViewerFrameName());
}


void FOOTPRINT_VIEWER_FRAME::OnCloseWindow( wxCloseEvent& Event )
{
    SaveSettings();

    if( m_Semaphore )
    {
        m_Semaphore->Post();
        SetModalMode(false);
        // This window will be destroyed by the calling function,
        // to avoid side effects
    }
    else
        Destroy();
}


void FOOTPRINT_VIEWER_FRAME::OnSashDrag( wxSashEvent& event )
{
    if( event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE )
        return;

    m_LibListSize.y = GetClientSize().y - m_MsgFrameHeight;
    m_FootprintListSize.y = m_LibListSize.y;

    switch( event.GetId() )
    {
    case ID_MODVIEW_LIBWINDOW:
        if( m_LibListWindow )
        {
            wxAuiPaneInfo& pane = m_auimgr.GetPane( m_LibListWindow );
            m_LibListSize.x = event.GetDragRect().width;
            pane.MinSize( m_LibListSize );
            m_auimgr.Update();
        }
        break;

    case ID_MODVIEW_FOOTPRINT_WINDOW:
    {
        wxAuiPaneInfo& pane = m_auimgr.GetPane( m_FootprintListWindow );
        m_FootprintListSize.x = event.GetDragRect().width;
        pane.MinSize( m_FootprintListSize );
        m_auimgr.Update();
    }
        break;
    }
}


void FOOTPRINT_VIEWER_FRAME::OnSize( wxSizeEvent& SizeEv )
{
    if( m_auimgr.GetManagedWindow() )
        m_auimgr.Update();

    SizeEv.Skip();
}


void FOOTPRINT_VIEWER_FRAME::OnSetRelativeOffset( wxCommandEvent& event )
{
    GetScreen()->m_O_Curseur = GetScreen()->GetCrossHairPosition();
    UpdateStatusBar();
}


void FOOTPRINT_VIEWER_FRAME::ReCreateLibraryList()
{
    if( m_LibList == NULL )
        return;

    m_LibList->Clear();
    for( unsigned ii = 0; ii < g_LibraryNames.GetCount(); ii++ )
    {
        m_LibList->Append( g_LibraryNames[ii] );
    }

    // Search for a previous selection:
    int index =  m_LibList->FindString( m_libraryName );

    if( index != wxNOT_FOUND )
    {
        m_LibList->SetSelection( index, true );
    }
    else
    {
        /* If not found, clear current library selection because it can be
         * deleted after a config change. */
        m_libraryName = wxEmptyString;
        m_footprintName = wxEmptyString;
    }

    ReCreateFootprintList();
    ReCreateHToolbar();
    DisplayLibInfos();
    m_canvas->Refresh();
}


void FOOTPRINT_VIEWER_FRAME::ReCreateFootprintList()
{
    if( m_FootprintList == NULL )
        return;

    m_FootprintList->Clear();

    if( m_libraryName.IsEmpty() )
    {
        m_footprintName = wxEmptyString;
        return;
    }

    wxArrayString libsList;
    libsList.Add( m_libraryName );
    FOOTPRINT_LIST fp_info_list;
    fp_info_list.ReadFootprintFiles( libsList );

    wxArrayString  fpList;
    BOOST_FOREACH( FOOTPRINT_INFO& footprint, fp_info_list.m_List )
    {
        fpList.Add(( footprint.m_Module ) );
    }
    m_FootprintList->Append( fpList );

    int index = m_FootprintList->FindString( m_footprintName );

    if( index == wxNOT_FOUND )
        m_footprintName = wxEmptyString;
    else
        m_FootprintList->SetSelection( index, true );
}


void FOOTPRINT_VIEWER_FRAME::ClickOnLibList( wxCommandEvent& event )
{
    int ii = m_LibList->GetSelection();

    if( ii < 0 )
        return;

    wxString name = m_LibList->GetString( ii );

    if( m_libraryName == name )
        return;

    m_libraryName = name;
    ReCreateFootprintList();
    m_canvas->Refresh();
    DisplayLibInfos();
    ReCreateHToolbar();
}


void FOOTPRINT_VIEWER_FRAME::ClickOnFootprintList( wxCommandEvent& event )
{
    int ii = m_FootprintList->GetSelection();

    if( ii < 0 )
        return;

    wxString name = m_FootprintList->GetString( ii );

    if( m_footprintName.CmpNoCase( name ) != 0 )
    {
        m_footprintName = name;
        SetCurItem( NULL );
        // Delete the current footprint
        GetBoard()->m_Modules.DeleteAll();
        GetModuleLibrary( m_libraryName + wxT(".") + LegacyFootprintLibPathExtension,
                          m_footprintName, true );
        DisplayLibInfos();
        Zoom_Automatique( false );
        m_canvas->Refresh();
        Update3D_Frame();
    }
}

void FOOTPRINT_VIEWER_FRAME::DClickOnFootprintList( wxCommandEvent& event )
{
    if( m_Semaphore )
    {
        ExportSelectedFootprint( event );
        // Prevent the double click from being as a single mouse button release
        // event in the parent window which would cause the part to be parked
        // rather than staying in mode mode.
        // Remember the mouse button will be released in the parent window
        // thus creating a mouse button release event which should be ingnored.
        ((PCB_BASE_FRAME*)GetParent())->SkipNextLeftButtonReleaseEvent();
    }
}

void FOOTPRINT_VIEWER_FRAME::ExportSelectedFootprint( wxCommandEvent& event )
{
    int ii = m_FootprintList->GetSelection();

    if( ii >= 0 )
        m_selectedFootprintName = m_FootprintList->GetString( ii );
    else
        m_selectedFootprintName.Empty();

    Close( true );
}


#define LIBLIST_WIDTH_KEY wxT( "Liblist_width" )
#define CMPLIST_WIDTH_KEY wxT( "Cmplist_width" )


void FOOTPRINT_VIEWER_FRAME::LoadSettings( )
{
    wxConfig* cfg ;

    EDA_DRAW_FRAME::LoadSettings();

    wxConfigPathChanger cpc( wxGetApp().GetSettings(), m_configPath );
    cfg = wxGetApp().GetSettings();

    m_LibListSize.x = 150; // default width of libs list
    m_FootprintListSize.x = 150; // default width of component list

    cfg->Read( LIBLIST_WIDTH_KEY, &m_LibListSize.x );
    cfg->Read( CMPLIST_WIDTH_KEY, &m_FootprintListSize.x );

    // Set parameters to a reasonable value.
    if ( m_LibListSize.x > m_FrameSize.x/2 )
        m_LibListSize.x = m_FrameSize.x/2;

    if ( m_FootprintListSize.x > m_FrameSize.x/2 )
        m_FootprintListSize.x = m_FrameSize.x/2;
}


void FOOTPRINT_VIEWER_FRAME::SaveSettings()
{
    wxConfig* cfg;

    EDA_DRAW_FRAME::SaveSettings();

    wxConfigPathChanger cpc( wxGetApp().GetSettings(), m_configPath );
    cfg = wxGetApp().GetSettings();

    if ( m_LibListSize.x )
        cfg->Write( LIBLIST_WIDTH_KEY, m_LibListSize.x );

    cfg->Write( CMPLIST_WIDTH_KEY, m_FootprintListSize.x );
}


void FOOTPRINT_VIEWER_FRAME::OnActivate( wxActivateEvent& event )
{
    EDA_DRAW_FRAME::OnActivate( event );

    // Ensure we do not have old selection:
    if( ! m_FrameIsActive )
        return;

    m_selectedFootprintName.Empty();

    // Ensure we have the right library list:
    if( g_LibraryNames.GetCount() == m_LibList->GetCount() )
    {
        unsigned ii;
        for( ii = 0; ii < g_LibraryNames.GetCount(); ii++ )
        {
            if( m_LibList->GetString(ii) != g_LibraryNames[ii] )
                break;
        }
        if( ii == g_LibraryNames.GetCount() )
            return;
    }

    // If we are here, the library list has changed, rebuild it
    ReCreateLibraryList();
    DisplayLibInfos();
}


void FOOTPRINT_VIEWER_FRAME::GeneralControl( wxDC* aDC, const wxPoint& aPosition, int aHotKey )
{
    wxRealPoint gridSize;
    wxPoint     oldpos;
    PCB_SCREEN* screen = GetScreen();
    wxPoint     pos = aPosition;

    wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );
    cmd.SetEventObject( this );

    pos = screen->GetNearestGridPosition( pos );
    oldpos = screen->GetCrossHairPosition();
    gridSize = screen->GetGridSize();

    switch( aHotKey )
    {
    case WXK_F1:
        cmd.SetId( ID_POPUP_ZOOM_IN );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case WXK_F2:
        cmd.SetId( ID_POPUP_ZOOM_OUT );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case WXK_F3:
        cmd.SetId( ID_ZOOM_REDRAW );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case WXK_F4:
        cmd.SetId( ID_POPUP_ZOOM_CENTER );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case WXK_HOME:
        cmd.SetId( ID_ZOOM_PAGE );
        GetEventHandler()->ProcessEvent( cmd );
        break;

    case ' ':
        screen->m_O_Curseur = screen->GetCrossHairPosition();
        break;

    case WXK_NUMPAD8:       /* cursor moved up */
    case WXK_UP:
        pos.y -= KiROUND( gridSize.y );
        m_canvas->MoveCursor( pos );
        break;

    case WXK_NUMPAD2:       /* cursor moved down */
    case WXK_DOWN:
        pos.y += KiROUND( gridSize.y );
        m_canvas->MoveCursor( pos );
        break;

    case WXK_NUMPAD4:       /*  cursor moved left */
    case WXK_LEFT:
        pos.x -= KiROUND( gridSize.x );
        m_canvas->MoveCursor( pos );
        break;

    case WXK_NUMPAD6:      /*  cursor moved right */
    case WXK_RIGHT:
        pos.x += KiROUND( gridSize.x );
        m_canvas->MoveCursor( pos );
        break;
    }

    screen->SetCrossHairPosition( pos );

    if( oldpos != screen->GetCrossHairPosition() )
    {
        pos = screen->GetCrossHairPosition();
        screen->SetCrossHairPosition( oldpos );
        m_canvas->CrossHairOff( aDC );
        screen->SetCrossHairPosition( pos );
        m_canvas->CrossHairOn( aDC );

        if( m_canvas->IsMouseCaptured() )
        {
            m_canvas->CallMouseCapture( aDC, aPosition, 0 );
        }
    }

    UpdateStatusBar();    /* Display new cursor coordinates */
}


void FOOTPRINT_VIEWER_FRAME::Show3D_Frame( wxCommandEvent& event )
{
    if( m_Draw3DFrame )
    {
        // Raising the window does not show the window on Windows if iconized.
        // This should work on any platform.
        if( m_Draw3DFrame->IsIconized() )
             m_Draw3DFrame->Iconize( false );

        m_Draw3DFrame->Raise();

        // Raising the window does not set the focus on Linux.  This should work on any platform.
        if( wxWindow::FindFocus() != m_Draw3DFrame )
            m_Draw3DFrame->SetFocus();

        return;
    }

    m_Draw3DFrame = new EDA_3D_FRAME( this, wxEmptyString );
    Update3D_Frame( false );
    m_Draw3DFrame->Show( true );
}

/**
 * Function Update3D_Frame
 * must be called after a footprint selection
 * Updates the 3D view and 3D frame title.
 */
void FOOTPRINT_VIEWER_FRAME::Update3D_Frame( bool aForceReloadFootprint )
{
    if( m_Draw3DFrame == NULL )
        return;

    wxString frm3Dtitle;
    frm3Dtitle.Printf( _( "ModView: 3D Viewer [%s]" ), GetChars( m_footprintName ) );
    m_Draw3DFrame->SetTitle( frm3Dtitle );

    if( aForceReloadFootprint )
    {
        m_Draw3DFrame->ReloadRequest();
        // Force 3D screen refresh immediately
        if( GetBoard()->m_Modules )
            m_Draw3DFrame->NewDisplay();
    }
}
