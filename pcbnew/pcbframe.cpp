/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2010 Jean-Pierre Charras, jean-pierre.charras@gpisa-lab.inpg.fr
 * Copyright (C) 2010 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file pcbframe.cpp
 * @brief PCB editor main window implementation.
 */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <wxPcbStruct.h>
#include <pcbcommon.h>      // enum PCB_VISIBLE
#include <collectors.h>
#include <build_version.h>
#include <macros.h>
#include <3d_viewer.h>
#include <msgpanel.h>

#include <pcbnew.h>
#include <protos.h>
#include <pcbnew_id.h>
#include <drc_stuff.h>
#include <layer_widget.h>
#include <dialog_design_rules.h>
#include <class_pcb_layer_widget.h>
#include <hotkeys.h>
#include <pcbnew_config.h>
#include <module_editor_frame.h>
#include <dialog_SVG_print.h>
#include <dialog_helpers.h>
#include <convert_from_iu.h>

#if defined(KICAD_SCRIPTING) || defined(KICAD_SCRIPTING_WXPYTHON)
#include <python_scripting.h>
#endif

// Keys used in read/write config
#define OPTKEY_DEFAULT_LINEWIDTH_VALUE  wxT( "PlotLineWidth_mm" )
#define PCB_SHOW_FULL_RATSNET_OPT       wxT( "PcbFullRatsnest" )
#define PCB_MAGNETIC_PADS_OPT           wxT( "PcbMagPadOpt" )
#define PCB_MAGNETIC_TRACKS_OPT         wxT( "PcbMagTrackOpt" )
#define SHOW_MICROWAVE_TOOLS            wxT( "ShowMicrowaveTools" )
#define SHOW_LAYER_MANAGER_TOOLS        wxT( "ShowLayerManagerTools" )


BEGIN_EVENT_TABLE( PCB_EDIT_FRAME, PCB_BASE_FRAME )
    EVT_SOCKET( ID_EDA_SOCKET_EVENT_SERV, PCB_EDIT_FRAME::OnSockRequestServer )
    EVT_SOCKET( ID_EDA_SOCKET_EVENT, PCB_EDIT_FRAME::OnSockRequest )

    EVT_COMBOBOX( ID_ON_ZOOM_SELECT, PCB_EDIT_FRAME::OnSelectZoom )
    EVT_COMBOBOX( ID_ON_GRID_SELECT, PCB_EDIT_FRAME::OnSelectGrid )

    EVT_CLOSE( PCB_EDIT_FRAME::OnCloseWindow )
    EVT_SIZE( PCB_EDIT_FRAME::OnSize )

    EVT_TOOL( ID_LOAD_FILE, PCB_EDIT_FRAME::Files_io )
    EVT_TOOL( ID_MENU_READ_BOARD_BACKUP_FILE, PCB_EDIT_FRAME::Files_io )
    EVT_TOOL( ID_MENU_RECOVER_BOARD_AUTOSAVE, PCB_EDIT_FRAME::Files_io )
    EVT_TOOL( ID_NEW_BOARD, PCB_EDIT_FRAME::Files_io )
    EVT_TOOL( ID_SAVE_BOARD, PCB_EDIT_FRAME::Files_io )
    EVT_TOOL( ID_OPEN_MODULE_EDITOR, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_OPEN_MODULE_VIEWER, PCB_EDIT_FRAME::Process_Special_Functions )

    // Menu Files:
    EVT_MENU( ID_MAIN_MENUBAR, PCB_EDIT_FRAME::Process_Special_Functions )

    EVT_MENU( ID_APPEND_FILE, PCB_EDIT_FRAME::Files_io )
    EVT_MENU( ID_SAVE_BOARD_AS, PCB_EDIT_FRAME::Files_io )
    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, PCB_EDIT_FRAME::OnFileHistory )

    EVT_MENU( ID_GEN_PLOT, PCB_EDIT_FRAME::ToPlotter )

    EVT_MENU( ID_GEN_EXPORT_SPECCTRA, PCB_EDIT_FRAME::ExportToSpecctra )
    EVT_MENU( ID_GEN_EXPORT_FILE_GENCADFORMAT, PCB_EDIT_FRAME::ExportToGenCAD )
    EVT_MENU( ID_GEN_EXPORT_FILE_MODULE_REPORT, PCB_EDIT_FRAME::GenFootprintsReport )
    EVT_MENU( ID_GEN_EXPORT_FILE_VRML, PCB_EDIT_FRAME::OnExportVRML )

    EVT_MENU( ID_GEN_IMPORT_SPECCTRA_SESSION,PCB_EDIT_FRAME::ImportSpecctraSession )
    EVT_MENU( ID_GEN_IMPORT_SPECCTRA_DESIGN, PCB_EDIT_FRAME::ImportSpecctraDesign )

    EVT_MENU( ID_MENU_ARCHIVE_NEW_MODULES, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_MENU( ID_MENU_ARCHIVE_ALL_MODULES, PCB_EDIT_FRAME::Process_Special_Functions )

    EVT_MENU( wxID_EXIT, PCB_EDIT_FRAME::OnQuit )

    // menu Config
    EVT_MENU( ID_PCB_DRAWINGS_WIDTHS_SETUP, PCB_EDIT_FRAME::OnConfigurePcbOptions )
    EVT_MENU( ID_CONFIG_REQ, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_PCB_LIB_TABLE_EDIT, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_CONFIG_SAVE, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_CONFIG_READ, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU_RANGE( ID_PREFERENCES_HOTKEY_START, ID_PREFERENCES_HOTKEY_END,
                    PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_MENU_PCB_SHOW_HIDE_LAYERS_MANAGER_DIALOG, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( wxID_PREFERENCES, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_PCB_LAYERS_SETUP, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_PCB_MASK_CLEARANCE, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_PCB_PAD_SETUP, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_CONFIG_SAVE, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_CONFIG_READ, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_PREFRENCES_MACROS_SAVE, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_PREFRENCES_MACROS_READ, PCB_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_PCB_DISPLAY_OPTIONS_SETUP, PCB_EDIT_FRAME::InstallDisplayOptionsDialog )
    EVT_MENU( ID_PCB_USER_GRID_SETUP, PCB_EDIT_FRAME::Process_Special_Functions )

    EVT_MENU_RANGE( ID_LANGUAGE_CHOICE, ID_LANGUAGE_CHOICE_END, PCB_EDIT_FRAME::SetLanguage )

    // menu Postprocess
    EVT_MENU( ID_PCB_GEN_POS_MODULES_FILE, PCB_EDIT_FRAME::GenFootprintsPositionFile )
    EVT_MENU( ID_PCB_GEN_DRILL_FILE, PCB_EDIT_FRAME::InstallDrillFrame )
    EVT_MENU( ID_PCB_GEN_CMP_FILE, PCB_EDIT_FRAME::RecreateCmpFileFromBoard )
    EVT_MENU( ID_PCB_GEN_BOM_FILE_FROM_BOARD, PCB_EDIT_FRAME::RecreateBOMFileFromBoard )

    // menu Miscellaneous
    EVT_MENU( ID_MENU_LIST_NETS, PCB_EDIT_FRAME::ListNetsAndSelect )
    EVT_MENU( ID_PCB_GLOBAL_DELETE, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_MENU( ID_MENU_PCB_CLEAN, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_MENU( ID_MENU_PCB_SWAP_LAYERS, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_MENU( ID_MENU_PCB_RESET_TEXTMODULE_FIELDS_SIZES,
              PCB_EDIT_FRAME::OnResetModuleTextSizes )

    // Menu Help
    EVT_MENU( wxID_HELP, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( wxID_INDEX, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( wxID_ABOUT, EDA_BASE_FRAME::GetKicadAbout )

    // Menu 3D Frame
    EVT_MENU( ID_MENU_PCB_SHOW_3D_FRAME, PCB_EDIT_FRAME::Show3D_Frame )

    // Menu Get Design Rules Editor
    EVT_MENU( ID_MENU_PCB_SHOW_DESIGN_RULES_DIALOG, PCB_EDIT_FRAME::ShowDesignRulesEditor )

    // Horizontal toolbar
    EVT_TOOL( ID_TO_LIBRARY, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_SHEET_SET, EDA_DRAW_FRAME::Process_PageSettings )
    EVT_TOOL( wxID_CUT, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_COPY, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_PASTE, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_UNDO, PCB_EDIT_FRAME::GetBoardFromUndoList )
    EVT_TOOL( wxID_REDO, PCB_EDIT_FRAME::GetBoardFromRedoList )
    EVT_TOOL( wxID_PRINT, PCB_EDIT_FRAME::ToPrinter )
    EVT_TOOL( ID_GEN_PLOT_SVG, PCB_EDIT_FRAME::SVG_Print )
    EVT_TOOL( ID_GEN_PLOT, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_FIND_ITEMS, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_GET_NETLIST, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_DRC_CONTROL, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_AUX_TOOLBAR_PCB_SELECT_LAYER_PAIR, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_AUX_TOOLBAR_PCB_SELECT_AUTO_WIDTH, PCB_EDIT_FRAME::Tracks_and_Vias_Size_Event )
    EVT_COMBOBOX( ID_TOOLBARH_PCB_SELECT_LAYER, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_COMBOBOX( ID_AUX_TOOLBAR_PCB_TRACK_WIDTH, PCB_EDIT_FRAME::Tracks_and_Vias_Size_Event )
    EVT_COMBOBOX( ID_AUX_TOOLBAR_PCB_VIA_SIZE, PCB_EDIT_FRAME::Tracks_and_Vias_Size_Event )
    EVT_TOOL( ID_TOOLBARH_PCB_MODE_MODULE, PCB_EDIT_FRAME::OnSelectAutoPlaceMode )
    EVT_TOOL( ID_TOOLBARH_PCB_MODE_TRACKS, PCB_EDIT_FRAME::OnSelectAutoPlaceMode )
    EVT_TOOL( ID_TOOLBARH_PCB_FREEROUTE_ACCESS, PCB_EDIT_FRAME::Access_to_External_Tool )
#ifdef KICAD_SCRIPTING_WXPYTHON
    EVT_TOOL( ID_TOOLBARH_PCB_SCRIPTING_CONSOLE, PCB_EDIT_FRAME::ScriptingConsoleEnableDisable )
#endif
    // Option toolbar
    EVT_TOOL( ID_TB_OPTIONS_DRC_OFF,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_RATSNEST,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_MODULE_RATSNEST,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_AUTO_DEL_TRACK,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_VIAS_SKETCH,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_TRACKS_SKETCH,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_HIGH_CONTRAST_MODE,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_EXTRA_VERTICAL_TOOLBAR_MICROWAVE,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )

    EVT_TOOL_RANGE( ID_TB_OPTIONS_SHOW_ZONES, ID_TB_OPTIONS_SHOW_ZONES_OUTLINES_ONLY,
                    PCB_EDIT_FRAME::OnSelectOptionToolbar )

    EVT_TOOL( ID_TB_OPTIONS_SHOW_MANAGE_LAYERS_VERTICAL_TOOLBAR,
              PCB_EDIT_FRAME::OnSelectOptionToolbar )

    // Vertical main toolbar:
    EVT_TOOL( ID_NO_TOOL_SELECTED, PCB_EDIT_FRAME::OnSelectTool )
    EVT_TOOL_RANGE( ID_PCB_HIGHLIGHT_BUTT, ID_PCB_PLACE_GRID_COORD_BUTT,
                    PCB_EDIT_FRAME::OnSelectTool )

    EVT_TOOL_RANGE( ID_PCB_MUWAVE_START_CMD, ID_PCB_MUWAVE_END_CMD,
                    PCB_EDIT_FRAME::ProcessMuWaveFunctions )

    EVT_MENU_RANGE( ID_POPUP_PCB_AUTOPLACE_START_RANGE, ID_POPUP_PCB_AUTOPLACE_END_RANGE,
                    PCB_EDIT_FRAME::AutoPlace )

    EVT_MENU( ID_POPUP_PCB_REORIENT_ALL_MODULES, PCB_EDIT_FRAME::OnOrientFootprints )

    EVT_MENU_RANGE( ID_POPUP_PCB_START_RANGE, ID_POPUP_PCB_END_RANGE,
                    PCB_EDIT_FRAME::Process_Special_Functions )

    // Tracks and vias sizes general options
    EVT_MENU_RANGE( ID_POPUP_PCB_SELECT_WIDTH_START_RANGE,
                    ID_POPUP_PCB_SELECT_WIDTH_END_RANGE,
                    PCB_EDIT_FRAME::Tracks_and_Vias_Size_Event )

    // popup menus
    EVT_MENU( ID_POPUP_PCB_DELETE_TRACKSEG, PCB_EDIT_FRAME::Process_Special_Functions )
    EVT_MENU_RANGE( ID_POPUP_GENERAL_START_RANGE, ID_POPUP_GENERAL_END_RANGE,
                    PCB_EDIT_FRAME::Process_Special_Functions )

    // User interface update event handlers.
    EVT_UPDATE_UI( ID_SAVE_BOARD, PCB_EDIT_FRAME::OnUpdateSave )
    EVT_UPDATE_UI( ID_AUX_TOOLBAR_PCB_SELECT_LAYER_PAIR, PCB_EDIT_FRAME::OnUpdateLayerPair )
    EVT_UPDATE_UI( ID_TOOLBARH_PCB_SELECT_LAYER, PCB_EDIT_FRAME::OnUpdateLayerSelectBox )
    EVT_UPDATE_UI( ID_TB_OPTIONS_DRC_OFF, PCB_EDIT_FRAME::OnUpdateDrcEnable )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_RATSNEST, PCB_EDIT_FRAME::OnUpdateShowBoardRatsnest )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_MODULE_RATSNEST, PCB_EDIT_FRAME::OnUpdateShowModuleRatsnest )
    EVT_UPDATE_UI( ID_TB_OPTIONS_AUTO_DEL_TRACK, PCB_EDIT_FRAME::OnUpdateAutoDeleteTrack )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_VIAS_SKETCH, PCB_EDIT_FRAME::OnUpdateViaDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_TRACKS_SKETCH, PCB_EDIT_FRAME::OnUpdateTraceDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_HIGH_CONTRAST_MODE,
                   PCB_EDIT_FRAME::OnUpdateHighContrastDisplayMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_MANAGE_LAYERS_VERTICAL_TOOLBAR,
                   PCB_EDIT_FRAME::OnUpdateShowLayerManager )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_EXTRA_VERTICAL_TOOLBAR_MICROWAVE,
                   PCB_EDIT_FRAME::OnUpdateShowMicrowaveToolbar )
    EVT_UPDATE_UI( ID_NO_TOOL_SELECTED, PCB_EDIT_FRAME::OnUpdateVerticalToolbar )
    EVT_UPDATE_UI( ID_AUX_TOOLBAR_PCB_TRACK_WIDTH, PCB_EDIT_FRAME::OnUpdateSelectTrackWidth )
    EVT_UPDATE_UI( ID_AUX_TOOLBAR_PCB_SELECT_AUTO_WIDTH,
                   PCB_EDIT_FRAME::OnUpdateSelectAutoTrackWidth )
    EVT_UPDATE_UI( ID_POPUP_PCB_SELECT_AUTO_WIDTH, PCB_EDIT_FRAME::OnUpdateSelectAutoTrackWidth )
    EVT_UPDATE_UI( ID_AUX_TOOLBAR_PCB_VIA_SIZE, PCB_EDIT_FRAME::OnUpdateSelectViaSize )
    EVT_UPDATE_UI( ID_TOOLBARH_PCB_MODE_MODULE, PCB_EDIT_FRAME::OnUpdateAutoPlaceModulesMode )
    EVT_UPDATE_UI( ID_TOOLBARH_PCB_MODE_TRACKS, PCB_EDIT_FRAME::OnUpdateAutoPlaceTracksMode )
    EVT_UPDATE_UI_RANGE( ID_POPUP_PCB_SELECT_WIDTH1, ID_POPUP_PCB_SELECT_WIDTH8,
                         PCB_EDIT_FRAME::OnUpdateSelectTrackWidth )
    EVT_UPDATE_UI_RANGE( ID_POPUP_PCB_SELECT_VIASIZE1, ID_POPUP_PCB_SELECT_VIASIZE8,
                         PCB_EDIT_FRAME::OnUpdateSelectViaSize )
    EVT_UPDATE_UI_RANGE( ID_PCB_HIGHLIGHT_BUTT, ID_PCB_PLACE_GRID_COORD_BUTT,
                         PCB_EDIT_FRAME::OnUpdateVerticalToolbar )
    EVT_UPDATE_UI_RANGE( ID_TB_OPTIONS_SHOW_ZONES, ID_TB_OPTIONS_SHOW_ZONES_OUTLINES_ONLY,
                         PCB_EDIT_FRAME::OnUpdateZoneDisplayStyle )
END_EVENT_TABLE()


///////****************************///////////:

#define PCB_EDIT_FRAME_NAME wxT( "PcbFrame" )

PCB_EDIT_FRAME::PCB_EDIT_FRAME( wxWindow* parent, const wxString& title,
                                const wxPoint& pos, const wxSize& size,
                                long style ) :
    PCB_BASE_FRAME( parent, PCB_FRAME_TYPE, title, pos, size,
                    style, PCB_EDIT_FRAME_NAME )
{
    m_FrameName = PCB_EDIT_FRAME_NAME;
    m_showBorderAndTitleBlock = true;   // true to display sheet references
    m_showAxis = false;                 // true to display X and Y axis
    m_showOriginAxis = true;
    m_showGridAxis = true;
    m_SelTrackWidthBox = NULL;
    m_SelViaSizeBox = NULL;
    m_SelLayerBox = NULL;
    m_show_microwave_tools = false;
    m_show_layer_manager_tools = true;
    m_HotkeysZoomAndGridList = g_Board_Editor_Hokeys_Descr;
    m_hasAutoSave = true;
    m_RecordingMacros = -1;
    m_microWaveToolBar = NULL;
    m_useCmpFileForFpNames = true;
#ifdef KICAD_SCRIPTING_WXPYTHON
    m_pythonPanel = NULL;
#endif
    for ( int i = 0; i < 10; i++ )
        m_Macros[i].m_Record.clear();

    SetBoard( new BOARD() );

    // Create the PCB_LAYER_WIDGET *after* SetBoard():

    wxFont font = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
    int pointSize = font.GetPointSize();
    int screenHeight = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );

    // printf( "pointSize:%d  80%%:%d\n", pointSize, (pointSize*8)/10 );

    if( screenHeight <= 900 )
        pointSize = (pointSize * 8) / 10;

    m_Layers = new PCB_LAYER_WIDGET( this, m_canvas, pointSize );

    m_drc = new DRC( this );        // these 2 objects point to each other

    wxIcon  icon;
    icon.CopyFromBitmap( KiBitmap( icon_pcbnew_xpm ) );
    SetIcon( icon );

    SetScreen( new PCB_SCREEN( GetPageSettings().GetSizeIU() ) );

    // PCB drawings start in the upper left corner.
    GetScreen()->m_Center = false;

    // LoadSettings() *after* creating m_LayersManager, because LoadSettings()
    // initialize parameters in m_LayersManager
    LoadSettings();

    // Be sure options are updated
    m_DisplayPcbTrackFill = DisplayOpt.DisplayPcbTrackFill;
    m_DisplayPadFill = DisplayOpt.DisplayPadFill;
    m_DisplayViaFill = DisplayOpt.DisplayViaFill;
    m_DisplayPadNum  = DisplayOpt.DisplayPadNum;

    m_DisplayModEdge = DisplayOpt.DisplayModEdge;
    m_DisplayModText = DisplayOpt.DisplayModText;

    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    GetScreen()->AddGrid( m_UserGridSize, m_UserGridUnit, ID_POPUP_GRID_USER );
    GetScreen()->SetGrid( ID_POPUP_GRID_LEVEL_1000 + m_LastGridSizeId  );

    if( m_canvas )
        m_canvas->SetEnableBlockCommands( true );

    ReCreateMenuBar();
    ReCreateHToolbar();
    ReCreateAuxiliaryToolbar();
    ReCreateVToolbar();
    ReCreateOptToolbar();

    ReCreateMicrowaveVToolbar();

    m_auimgr.SetManagedWindow( this );

    EDA_PANEINFO horiz;
    horiz.HorizontalToolbarPane();

    EDA_PANEINFO vert;
    vert.VerticalToolbarPane();

    EDA_PANEINFO mesg;
    mesg.MessageToolbarPane();

    // Create a wxAuiPaneInfo for the Layers Manager, not derived from the template.
    // LAYER_WIDGET is floatable, but initially docked at far right
    EDA_PANEINFO   lyrs;
    lyrs.LayersToolbarPane();
    lyrs.MinSize( m_Layers->GetBestSize() );    // updated in ReFillLayerWidget
    lyrs.BestSize( m_Layers->GetBestSize() );
    lyrs.Caption( _( "Visibles" ) );


    if( m_mainToolBar )    // The main horizontal toolbar
    {
        m_auimgr.AddPane( m_mainToolBar,
                          wxAuiPaneInfo( horiz ).Name( wxT( "m_mainToolBar" ) ).Top().Row( 0 ) );
    }

    if( m_auxiliaryToolBar )    // the auxiliary horizontal toolbar, that shows track and via sizes, zoom ...)
    {
        m_auimgr.AddPane( m_auxiliaryToolBar,
                          wxAuiPaneInfo( horiz ).Name( wxT( "m_auxiliaryToolBar" ) ).Top().Row( 1 ) );
    }

    if( m_microWaveToolBar )    // The auxiliary vertical right toolbar (currently microwave tools)
        m_auimgr.AddPane( m_microWaveToolBar,
                          wxAuiPaneInfo( vert ).Name( wxT( "m_microWaveToolBar" ) ).Right().Layer( 1 ).Position(1).Hide() );

    if( m_drawToolBar )    // The main right vertical toolbar
        m_auimgr.AddPane( m_drawToolBar,
                          wxAuiPaneInfo( vert ).Name( wxT( "m_VToolBar" ) ).Right().Layer( 2 ) );

    // Add the layer manager ( most right side of pcbframe )
    m_auimgr.AddPane( m_Layers, lyrs.Name( wxT( "m_LayersManagerToolBar" ) ).Right().Layer( 3 ) );

    if( m_optionsToolBar )    // The left vertical toolbar (fast acces display options of Pcbnew)
    {
        m_auimgr.AddPane( m_optionsToolBar,
                          wxAuiPaneInfo( vert ).Name( wxT( "m_optionsToolBar" ) ).Left().Layer(1) );

        m_auimgr.GetPane( wxT( "m_LayersManagerToolBar" ) ).Show( m_show_layer_manager_tools );
        m_auimgr.GetPane( wxT( "m_microWaveToolBar" ) ).Show( m_show_microwave_tools );
    }

    if( m_canvas )
        m_auimgr.AddPane( m_canvas,
                          wxAuiPaneInfo().Name( wxT( "DrawFrame" ) ).CentrePane() );

    if( m_messagePanel )
        m_auimgr.AddPane( m_messagePanel,
                          wxAuiPaneInfo( mesg ).Name( wxT( "MsgPanel" ) ).Bottom().Layer(10) );


#ifdef KICAD_SCRIPTING_WXPYTHON
    // Add the scripting panel
    EDA_PANEINFO  pythonAuiInfo;
    pythonAuiInfo.ScriptingToolbarPane();
    pythonAuiInfo.Caption( wxT( "Python Scripting" ) );
    pythonAuiInfo.MinSize( wxSize( 200, 100 ) );
    pythonAuiInfo.BestSize( wxSize( GetClientSize().x/2, 200 ) );
    pythonAuiInfo.Hide();

    m_pythonPanel = CreatePythonShellWindow( this );
    m_auimgr.AddPane( m_pythonPanel,
                          pythonAuiInfo.Name( wxT( "PythonPanel" ) ).Bottom().Layer(9) );

    m_pythonPanelHidden = true;
#endif

    ReFillLayerWidget();        // this is near end because contents establish size

    m_Layers->ReFillRender();   // Update colors in Render after the config is read

    syncLayerWidgetLayer();

    m_auimgr.Update();
}


PCB_EDIT_FRAME::~PCB_EDIT_FRAME()
{
    m_RecordingMacros = -1;

    for( int i = 0; i < 10; i++ )
        m_Macros[i].m_Record.clear();

    delete m_drc;
}


bool PCB_EDIT_FRAME::isAutoSaveRequired() const
{
    return GetScreen()->IsSave();
}


void PCB_EDIT_FRAME::ReFillLayerWidget()
{
    m_Layers->ReFill();

    wxAuiPaneInfo& lyrs = m_auimgr.GetPane( m_Layers );

    wxSize bestz = m_Layers->GetBestSize();

    lyrs.MinSize( bestz );
    lyrs.BestSize( bestz );
    lyrs.FloatingSize( bestz );

    if( lyrs.IsDocked() )
        m_auimgr.Update();
    else
        m_Layers->SetSize( bestz );
}


void PCB_EDIT_FRAME::OnQuit( wxCommandEvent& event )
{
    Close( true );
}


void PCB_EDIT_FRAME::OnCloseWindow( wxCloseEvent& Event )
{
    m_canvas->SetAbortRequest( true );

    if( GetScreen()->IsModify() )
    {
        wxString msg;
        msg.Printf( _("Save the changes in\n<%s>\nbefore closing?"),
                    GetChars( GetBoard()->GetFileName() ) );

        int ii = DisplayExitDialog( this, msg );
        switch( ii )
        {
        case wxID_CANCEL:
            Event.Veto();
            return;

        case wxID_NO:
            break;

        case wxID_OK:
        case wxID_YES:
            SavePcbFile( GetBoard()->GetFileName() );
            break;
        }
    }

    // Delete the auto save file if it exists.
    wxFileName fn = GetBoard()->GetFileName();

    // Auto save file name is the normal file name prefixed with a '$'.
    fn.SetName( wxT( "$" ) + fn.GetName() );

    // Remove the auto save file on a normal close of Pcbnew.
    if( fn.FileExists() && !wxRemoveFile( fn.GetFullPath() ) )
    {
        wxString msg;

        msg.Printf( _( "The auto save file <%s> could not be removed!" ),
                    GetChars( fn.GetFullPath() ) );

        wxMessageBox( msg, wxGetApp().GetAppName(), wxOK | wxICON_ERROR, this );
    }

    SaveSettings();

    // do not show the window because ScreenPcb will be deleted and we do not
    // want any paint event
    Show( false );
    Destroy();
}


void PCB_EDIT_FRAME::Show3D_Frame( wxCommandEvent& event )
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

    m_Draw3DFrame = new EDA_3D_FRAME( this, _( "3D Viewer" ) );
    m_Draw3DFrame->SetDefaultFileName( GetBoard()->GetFileName() );
    m_Draw3DFrame->Show( true );
}


void PCB_EDIT_FRAME::ShowDesignRulesEditor( wxCommandEvent& event )
{
    DIALOG_DESIGN_RULES dR_editor( this );
    int returncode = dR_editor.ShowModal();

    if( returncode == wxID_OK )     // New rules, or others changes.
    {
        ReCreateLayerBox( NULL );
        updateTraceWidthSelectBox();
        updateViaSizeSelectBox();
        OnModify();
    }
}


void PCB_EDIT_FRAME::LoadSettings()
{
    wxConfig* config = wxGetApp().GetSettings();

    if( config == NULL )
        return;

    // The configuration setting that used to be mixed in with the project file settings.
    wxGetApp().ReadCurrentSetupValues( GetConfigurationSettings() );

    PCB_BASE_FRAME::LoadSettings();

    double dtmp;
    config->Read( OPTKEY_DEFAULT_LINEWIDTH_VALUE, &dtmp, 0.1 ); // stored in mm
    if( dtmp < 0.01 )
        dtmp = 0.01;
    if( dtmp > 5.0 )
        dtmp = 5.0;
    g_DrawDefaultLineThickness = Millimeter2iu( dtmp );
    long tmp;
    config->Read( PCB_SHOW_FULL_RATSNET_OPT, &tmp );
    GetBoard()->SetElementVisibility(RATSNEST_VISIBLE, tmp);

    config->Read( PCB_MAGNETIC_PADS_OPT, &g_MagneticPadOption );
    config->Read( PCB_MAGNETIC_TRACKS_OPT, &g_MagneticTrackOption );
    config->Read( SHOW_MICROWAVE_TOOLS, &m_show_microwave_tools );
    config->Read( SHOW_LAYER_MANAGER_TOOLS, &m_show_layer_manager_tools );

    // WxWidgets 2.9.1 seems call setlocale( LC_NUMERIC, "" )
    // when reading doubles in config,
    // but forget to back to current locale. So we call SetLocaleTo_Default
    SetLocaleTo_Default( );
}


void PCB_EDIT_FRAME::SaveSettings()
{
    wxConfig* config = wxGetApp().GetSettings();

    if( config == NULL )
        return;

    // The configuration setting that used to be mixed in with the project file settings.
    wxGetApp().SaveCurrentSetupValues( GetConfigurationSettings() );

    PCB_BASE_FRAME::SaveSettings();

    // This value is stored in mm )
    config->Write( OPTKEY_DEFAULT_LINEWIDTH_VALUE,
                   MM_PER_IU * g_DrawDefaultLineThickness );
    long tmp = GetBoard()->IsElementVisible(RATSNEST_VISIBLE);
    config->Write( PCB_SHOW_FULL_RATSNET_OPT, tmp );
    config->Write( PCB_MAGNETIC_PADS_OPT, (long) g_MagneticPadOption );
    config->Write( PCB_MAGNETIC_TRACKS_OPT, (long) g_MagneticTrackOption );
    config->Write( SHOW_MICROWAVE_TOOLS, (long) m_show_microwave_tools );
    config->Write( SHOW_LAYER_MANAGER_TOOLS, (long)m_show_layer_manager_tools );
}


bool PCB_EDIT_FRAME::IsGridVisible() const
{
    return IsElementVisible( GRID_VISIBLE );
}


void PCB_EDIT_FRAME::SetGridVisibility(bool aVisible)
{
    SetElementVisibility( GRID_VISIBLE, aVisible );
}


EDA_COLOR_T PCB_EDIT_FRAME::GetGridColor() const
{
    return GetBoard()->GetVisibleElementColor( GRID_VISIBLE );
}


void PCB_EDIT_FRAME::SetGridColor(EDA_COLOR_T aColor)
{
    GetBoard()->SetVisibleElementColor( GRID_VISIBLE, aColor );
}


bool PCB_EDIT_FRAME::IsMicroViaAcceptable( void )
{
    int copperlayercnt = GetBoard()->GetCopperLayerCount( );
    int currLayer = getActiveLayer();

    if( !GetDesignSettings().m_MicroViasAllowed )
        return false;   // Obvious..

    if( copperlayercnt < 4 )
        return false;   // Only on multilayer boards..

    if( ( currLayer == LAYER_N_BACK )
       || ( currLayer == LAYER_N_FRONT )
       || ( currLayer == copperlayercnt - 2 )
       || ( currLayer == LAYER_N_2 ) )
        return true;

    return false;
}


void PCB_EDIT_FRAME::syncLayerWidgetLayer()
{
    m_Layers->SelectLayer( getActiveLayer() );
}


void PCB_EDIT_FRAME::syncRenderStates()
{
    m_Layers->SyncRenderStates();
}


void PCB_EDIT_FRAME::syncLayerVisibilities()
{
    m_Layers->SyncLayerVisibilities();
}


void PCB_EDIT_FRAME::unitsChangeRefresh()
{
    PCB_BASE_FRAME::unitsChangeRefresh();    // Update the grid size select box.

    updateTraceWidthSelectBox();
    updateViaSizeSelectBox();
}


bool PCB_EDIT_FRAME::IsElementVisible( int aElement ) const
{
    return GetBoard()->IsElementVisible( aElement );
}


void PCB_EDIT_FRAME::SetElementVisibility( int aElement, bool aNewState )
{
    GetBoard()->SetElementVisibility( aElement, aNewState );
    m_Layers->SetRenderState( aElement, aNewState );
}


void PCB_EDIT_FRAME::SetVisibleAlls()
{
    GetBoard()->SetVisibleAlls();

    for( int ii = 0; ii < PCB_VISIBLE( END_PCB_VISIBLE_LIST ); ii++ )
        m_Layers->SetRenderState( ii, true );
}


void PCB_EDIT_FRAME::SetLanguage( wxCommandEvent& event )
{
    EDA_DRAW_FRAME::SetLanguage( event );
    m_Layers->SetLayersManagerTabsText();
    wxAuiPaneInfo& pane_info = m_auimgr.GetPane( m_Layers );
    pane_info.Caption( _( "Visibles" ) );
    m_auimgr.Update();
    ReFillLayerWidget();

    FOOTPRINT_EDIT_FRAME * moduleEditFrame = FOOTPRINT_EDIT_FRAME::GetActiveFootprintEditor();
    if( moduleEditFrame )
        moduleEditFrame->EDA_DRAW_FRAME::SetLanguage( event );
}


wxString PCB_EDIT_FRAME::GetLastNetListRead()
{
    wxFileName absoluteFileName = m_lastNetListRead;
    wxFileName pcbFileName = GetBoard()->GetFileName();

    if( !absoluteFileName.MakeAbsolute( pcbFileName.GetPath() ) || !absoluteFileName.FileExists() )
    {
        absoluteFileName.Clear();
        m_lastNetListRead = wxEmptyString;
    }

    return absoluteFileName.GetFullPath();
}


void PCB_EDIT_FRAME::SetLastNetListRead( const wxString& aLastNetListRead )
{
    wxFileName relativeFileName = aLastNetListRead;
    wxFileName pcbFileName = GetBoard()->GetFileName();

    if( relativeFileName.MakeRelativeTo( pcbFileName.GetPath() )
        && relativeFileName.GetFullPath() != aLastNetListRead )
    {
        m_lastNetListRead = relativeFileName.GetFullPath();
    }
}


void PCB_EDIT_FRAME::OnModify( )
{
    PCB_BASE_FRAME::OnModify();

    if( m_Draw3DFrame )
        m_Draw3DFrame->ReloadRequest();
}


void PCB_EDIT_FRAME::SVG_Print( wxCommandEvent& event )
{
    DIALOG_SVG_PRINT frame( this );

    frame.ShowModal();
}


void PCB_EDIT_FRAME::UpdateTitle()
{
    wxString title;
    wxFileName fileName = GetBoard()->GetFileName();

    title.Printf( wxT( "Pcbnew %s " ), GetChars( GetBuildVersion() ) );

    if( fileName.IsOk() && fileName.FileExists() )
    {
        title << fileName.GetFullPath();

        if( !fileName.IsFileWritable() )
            title << _( " [Read Only]" );
    }
    else
    {
        title << _( " [new file]" ) << wxT(" ") << fileName.GetFullPath();
    }

    SetTitle( title );
}

#ifdef KICAD_SCRIPTING_WXPYTHON
void PCB_EDIT_FRAME::ScriptingConsoleEnableDisable( wxCommandEvent& aEvent )
{
    if ( m_pythonPanelHidden )
    {
        m_auimgr.GetPane( m_pythonPanel ).Show();
        m_pythonPanelHidden = false;
    }
    else
    {
        m_auimgr.GetPane( m_pythonPanel ).Hide();
        m_pythonPanelHidden = true;
    }

    m_auimgr.Update();

}
#endif

void PCB_EDIT_FRAME::OnSelectAutoPlaceMode( wxCommandEvent& aEvent )
{
    // Automatic placement of modules and tracks is a mutually exclusive operation so
    // clear the other tool if one of the two is selected.
    // Be careful: this event function is called both by the
    // ID_TOOLBARH_PCB_MODE_MODULE and the ID_TOOLBARH_PCB_MODE_TRACKS tool
    // Therefore we should avoid a race condition when deselecting one of these tools
    // inside this function (seems happen on some Linux/wxWidgets versions)
    // when the other tool is selected

    switch( aEvent.GetId() )
    {
        case ID_TOOLBARH_PCB_MODE_MODULE:
            if( aEvent.IsChecked() &&
                m_mainToolBar->GetToolToggled( ID_TOOLBARH_PCB_MODE_TRACKS ) )
                m_mainToolBar->ToggleTool( ID_TOOLBARH_PCB_MODE_TRACKS, false );
            break;

        case ID_TOOLBARH_PCB_MODE_TRACKS:
            if( aEvent.IsChecked() &&
                m_mainToolBar->GetToolToggled( ID_TOOLBARH_PCB_MODE_MODULE ) )
                m_mainToolBar->ToggleTool( ID_TOOLBARH_PCB_MODE_MODULE, false );
            break;
        }
}

