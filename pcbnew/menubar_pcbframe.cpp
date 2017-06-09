/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012-2016 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2017 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file menubar_pcbframe.cpp
 * Pcbnew editor menu bar
 */


#include <menus_helpers.h>
#include <kiface_i.h>
#include <pgm_base.h>
#include <wxPcbStruct.h>

#include "help_common_strings.h"
#include "hotkeys.h"
#include "pcbnew.h"
#include "pcbnew_id.h"


// Build the place submenu
static void preparePlaceMenu( wxMenu* aParentMenu );

// Build the files menu. Because some commands are available only if
// Pcbnew is run outside a project (run alone), aIsOutsideProject is false
// when Pcbnew is run from Kicad manager, and true is run as stand alone app.
static void prepareFilesMenu( wxMenu* aParentMenu, bool aIsOutsideProject );

// Build the export submenu (inside files menu)
static void prepareExportMenu( wxMenu* aParentMenu );

// Build the tools menu
static void prepareToolsMenu( wxMenu* aParentMenu );

// Build the help menu
static void prepareHelpMenu( wxMenu* aParentMenu );

// Build the edit menu
static void prepareEditMenu( wxMenu* aParentMenu );

// Build the route menu
static void prepareRouteMenu( wxMenu* aParentMenu );

// Build the view menu
static void prepareViewMenu( wxMenu* aParentMenu );

// Build the dimensions menu
static void prepareDimensionsMenu( wxMenu* aParentMenu );

// Build the library management menu
static void prepareLibraryMenu( wxMenu* aParentMenu );

// Build the design rules menu
static void prepareDesignRulesMenu( wxMenu* aParentMenu );

// Build the preferences menu
static void preparePreferencesMenu( PCB_EDIT_FRAME* aFrame, wxMenu* aParentMenu );


void PCB_EDIT_FRAME::ReCreateMenuBar()
{
    wxString    text;
    wxMenuBar*  menuBar = GetMenuBar();

    if( !menuBar )
        menuBar = new wxMenuBar();

    // Delete all existing menus so they can be rebuilt.
    // This allows language changes of the menu text on the fly.
    menuBar->Freeze();

    while( menuBar->GetMenuCount() )
        delete menuBar->Remove( 0 );

    // Recreate all menus:

    // Create File Menu
    wxMenu* filesMenu = new wxMenu;
    prepareFilesMenu( filesMenu, Kiface().IsSingle() );

    //----- Edit menu -----------------------------------------------------------
    wxMenu* editMenu = new wxMenu;
    prepareEditMenu( editMenu );

    //----- View menu -----------------------------------------------------------
    wxMenu* viewMenu = new wxMenu;
    prepareViewMenu( viewMenu );

    //----- Place Menu ----------------------------------------------------------
    wxMenu* placeMenu = new wxMenu;
    preparePlaceMenu( placeMenu );

    //----------- Route Menu ----------------------------------------------------
    wxMenu* routeMenu = new wxMenu;
    prepareRouteMenu( routeMenu );

    //----- Preferences and configuration menu------------------------------------
    wxMenu* configmenu = new wxMenu;
    prepareLibraryMenu( configmenu );
    configmenu->AppendSeparator();

    preparePreferencesMenu( this, configmenu );

    // Update menu labels:
    configmenu->SetLabel( ID_MENU_PCB_SHOW_HIDE_LAYERS_MANAGER,
                          m_show_layer_manager_tools ?
                          _( "Hide La&yers Manager" ) : _("Show La&yers Manager" ) );
    configmenu->SetLabel( ID_MENU_PCB_SHOW_HIDE_MUWAVE_TOOLBAR,
                          m_show_microwave_tools ?
                          _( "Hide Microwa&ve Toolbar" ): _( "Show Microwa&ve Toolbar" ) );


    //--- dimensions submenu ------------------------------------------------------
    wxMenu* dimensionsMenu = new wxMenu;
    prepareDimensionsMenu( dimensionsMenu );

    //----- Tools menu ----------------------------------------------------------
    wxMenu* toolsMenu = new wxMenu;
    prepareToolsMenu( toolsMenu );

    //----- Design Rules menu -----------------------------------------------------
    wxMenu* designRulesMenu = new wxMenu;
    prepareDesignRulesMenu( designRulesMenu );

    //------ Help menu ----------------------------------------------------------------
    wxMenu* helpMenu = new wxMenu;
    prepareHelpMenu( helpMenu );

    // Append all menus to the menuBar
    menuBar->Append( filesMenu, _( "&File" ) );
    menuBar->Append( editMenu, _( "&Edit" ) );
    menuBar->Append( viewMenu, _( "&View" ) );
    menuBar->Append( placeMenu, _( "&Place" ) );
    menuBar->Append( routeMenu, _( "Ro&ute" ) );
    menuBar->Append( configmenu, _( "P&references" ) );
    menuBar->Append( dimensionsMenu, _( "D&imensions" ) );
    menuBar->Append( toolsMenu, _( "&Tools" ) );
    menuBar->Append( designRulesMenu, _( "&Design Rules" ) );
    menuBar->Append( helpMenu, _( "&Help" ) );

    menuBar->Thaw();

    // Associate the menu bar with the frame, if no previous menubar
    if( GetMenuBar() == NULL )
        SetMenuBar( menuBar );
    else
        menuBar->Refresh();

#if defined(KICAD_SCRIPTING) && defined(KICAD_SCRIPTING_ACTION_MENU)
    // Populate the Action Plugin sub-menu
    RebuildActionPluginMenus();
#endif

}

// Build the design rules menu
void prepareDesignRulesMenu( wxMenu* aParentMenu )
{
    AddMenuItem( aParentMenu, ID_MENU_PCB_SHOW_DESIGN_RULES_DIALOG,
                 _( "&Design Rules" ),
                 _( "Open design rules editor" ), KiBitmap( config_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_LAYERS_SETUP,
                 _( "&Layers Setup" ),  _( "Enable and set layer properties" ),
                 KiBitmap( copper_layers_setup_xpm ) );
}


// Build the preferences menu
void preparePreferencesMenu( PCB_EDIT_FRAME* aFrame, wxMenu* aParentMenu )
{
    AddMenuItem( aParentMenu, ID_MENU_PCB_SHOW_HIDE_LAYERS_MANAGER,
                 _( "Hide La&yers Manager" ),
                 HELP_SHOW_HIDE_LAYERMANAGER,
                 KiBitmap( layers_manager_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_PCB_SHOW_HIDE_MUWAVE_TOOLBAR,
                 _( "Hide Microwa&ve Toolbar" ),
                 HELP_SHOW_HIDE_MICROWAVE_TOOLS,
                 KiBitmap( mw_toolbar_xpm ) );
#ifdef __WXMAC__
    aParentMenu->Append( wxID_PREFERENCES );
#else
    AddMenuItem( aParentMenu, wxID_PREFERENCES,
                 _( "&General Settings" ), _( "Select general options for Pcbnew" ),
                 KiBitmap( preference_xpm ) );
#endif

    AddMenuItem( aParentMenu, ID_PCB_DISPLAY_OPTIONS_SETUP,
                 _( "&Display and Hide" ),
                 _( "Select how items (pads, tracks, texts) are displayed" ),
                 KiBitmap( display_options_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_INTERACTIVE_ROUTER_SETTINGS,
                 _( "&Interactive Routing" ),
                 _( "Configure interactive router" ),
                 KiBitmap( add_tracks_xpm ) ); // fixme: icon

    // Language submenu
    aParentMenu->AppendSeparator();
    Pgm().AddMenuLanguageList( aParentMenu );

    // Icons options submenu
    aFrame->AddMenuIconsOptions( aParentMenu );

    // Hotkey submenu
    AddHotkeyConfigMenu( aParentMenu );

    aParentMenu->AppendSeparator();

    wxMenu* macrosMenu = new wxMenu();
 
    AddMenuItem( macrosMenu, ID_PREFRENCES_MACROS_SAVE,
                 _( "&Save macros" ),
                 _( "Save macros to file" ),
                 KiBitmap( save_setup_xpm ) );
 
    AddMenuItem( macrosMenu, ID_PREFRENCES_MACROS_READ,
                 _( "&Read macros" ),
                 _( "Read macros from file" ),
                 KiBitmap( read_setup_xpm ) );
 
    AddMenuItem( aParentMenu, macrosMenu,
                 -1, _( "Ma&cros" ),
                 _( "Macros save/read operations" ),
                 KiBitmap( macros_record_xpm ) );
 
    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_CONFIG_SAVE,
                 _( "&Save Preferences" ),
                 _( "Save application preferences" ),
                 KiBitmap( save_setup_xpm ) );

    AddMenuItem( aParentMenu, ID_CONFIG_READ,
                 _( "Load Prefe&rences" ),
                 _( "Load application preferences" ),
                 KiBitmap( read_setup_xpm ) );
}


// Build the route menu
void prepareRouteMenu( wxMenu* aParentMenu )
{
    AddMenuItem( aParentMenu, ID_TRACK_BUTT,
                 _( "&Single Track" ),
                 _( "Interactively route single track" ),
                 KiBitmap( add_tracks_xpm ) );

    AddMenuItem( aParentMenu, ID_DIFF_PAIR_BUTT,
                 _( "&Differential Pair" ),
                 _( "Interactively route differential pair" ),
                 KiBitmap( ps_diff_pair_xpm ) );

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_TUNE_SINGLE_TRACK_LEN_BUTT,
                 _( "&Tune Track Length" ),
                 _( "Tune length of single track" ),
                 KiBitmap( ps_tune_length_xpm ) );

    AddMenuItem( aParentMenu, ID_TUNE_DIFF_PAIR_LEN_BUTT,
                 _( "Tune Differential Pair &Length" ),
                 _( "Tune length of differential pair" ),
                 KiBitmap( ps_diff_pair_tune_length_xpm ) );

    AddMenuItem( aParentMenu, ID_TUNE_DIFF_PAIR_SKEW_BUTT,
                 _( "Tune Differential Pair &Skew/Phase" ),
                 _( "Tune skew/phase of a differential pair" ),
                 KiBitmap( ps_diff_pair_tune_phase_xpm ) );
}


// Build the library management menu
void prepareLibraryMenu( wxMenu* aParentMenu )
{
    AddMenuItem( aParentMenu, ID_PCB_LIB_WIZARD,
                _( "&Footprint Libraries Wizard" ), _( "Add footprint libraries using wizard" ),
                KiBitmap( wizard_add_fplib_small_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_LIB_TABLE_EDIT,
                _( "Footprint Li&braries Manager" ), _( "Configure footprint libraries" ),
                KiBitmap( library_table_xpm ) );

    // Path configuration edit dialog.
    AddMenuItem( aParentMenu,
                 ID_PREFERENCES_CONFIGURE_PATHS,
                 _( "Configure Pa&ths" ),
                 _( "Edit path configuration environment variables" ),
                 KiBitmap( path_xpm ) );

#ifdef BUILD_GITHUB_PLUGIN
    AddMenuItem( aParentMenu, ID_PCB_3DSHAPELIB_WIZARD,
                 _( "&3D Shape Downloader" ),
                 _( "Download from Github 3D shape libraries using wizard" ),
                 KiBitmap( import3d_xpm ) );
#endif
}


// Build the place submenu
void preparePlaceMenu( wxMenu* aParentMenu )
{
    wxString text;

    text = AddHotkeyName( _( "&Footprint" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_ADD_MODULE );
    AddMenuItem( aParentMenu, ID_PCB_MODULE_BUTT, text,
                 _( "Add footprints" ), KiBitmap( module_xpm ) );

    text = AddHotkeyName( _( "&Track" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_ADD_NEW_TRACK, IS_ACCELERATOR );
    AddMenuItem( aParentMenu, ID_TRACK_BUTT, text,
                 _( "Add tracks and vias" ), KiBitmap( add_tracks_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_ZONES_BUTT,
                 _( "&Zone" ), _( "Add filled zones" ), KiBitmap( add_zone_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_KEEPOUT_AREA_BUTT,
                 _( "&Keepout Area" ), _( "Add keepout areas" ), KiBitmap( add_keepout_area_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_ADD_TEXT_BUTT,
                 _( "Te&xt" ), _( "Add text on copper layers or graphic text" ),
                 KiBitmap( text_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_ARC_BUTT,
                 _( "&Arc" ), _( "Add graphic arc" ),KiBitmap( add_arc_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_CIRCLE_BUTT,
                 _( "&Circle" ), _( "Add graphic circle" ),
                 KiBitmap( add_circle_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_ADD_LINE_BUTT,
                 _( "&Line or Polygon" ),
                 _( "Add graphic line or polygon" ),
                 KiBitmap( add_dashed_line_xpm ) );

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_PCB_DIMENSION_BUTT,
                 _( "&Dimension" ), _( "Add dimension" ),
                 KiBitmap( add_dimension_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_MIRE_BUTT,
                 _( "La&yer alignment target" ), _( "Add layer alignment target" ),
                 KiBitmap( add_mires_xpm ) );

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_PCB_PLACE_OFFSET_COORD_BUTT,
                 _( "Drill and &Place Offset" ),
                 _( "Place origin point for drill and place files" ),
                 KiBitmap( pcb_offset_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_PLACE_GRID_COORD_BUTT,
                 _( "&Grid Origin" ),
                 _( "Set grid origin point" ),
                 KiBitmap( grid_select_axis_xpm ) );
}


// Build the tools menu
void prepareToolsMenu( wxMenu* aParentMenu )
{
    AddMenuItem( aParentMenu,
                 ID_UPDATE_PCB_FROM_SCH,
                 _( "Update PCB from Schematic" ),
                 _( "Update PCB design with current schematic (forward annotation)" ),
                 KiBitmap( import_brd_file_xpm ) );

    aParentMenu->AppendSeparator( );

    AddMenuItem( aParentMenu, ID_GET_NETLIST,
                 _( "Load &Netlist" ),
                 _( "Read netlist and update board connectivity" ),
                 KiBitmap( netlist_xpm ) );

    AddMenuItem( aParentMenu, ID_AUX_TOOLBAR_PCB_SELECT_LAYER_PAIR,
                 _( "Set &Layer Pair" ), _( "Change active layer pair" ),
                 KiBitmap( select_layer_pair_xpm ) );

    AddMenuItem( aParentMenu, ID_DRC_CONTROL,
                 _( "&Design Rules Check" ),
                 _( "Perform design rules check" ), KiBitmap( erc_xpm ) );

    AddMenuItem( aParentMenu, ID_TOOLBARH_PCB_FREEROUTE_ACCESS,
                 _( "&FreeRoute" ),
                 _( "Fast access to the FreeROUTE external advanced router" ),
                 KiBitmap( web_support_xpm ) );

#if defined(KICAD_SCRIPTING_WXPYTHON)
    AddMenuItem( aParentMenu, ID_TOOLBARH_PCB_SCRIPTING_CONSOLE,
                 _( "&Scripting Console" ),
                 _( "Show/Hide the Python scripting console" ),
                 KiBitmap( py_script_xpm ) );
#endif

#if defined(KICAD_SCRIPTING) && defined(KICAD_SCRIPTING_ACTION_MENU)
    aParentMenu->AppendSeparator( );

    wxMenu* submenuActionPluginsMenu = new wxMenu();

    AddMenuItem( aParentMenu, submenuActionPluginsMenu, ID_TOOLBARH_PCB_ACTION_PLUGIN,
                 _( "&External Plugins" ),
                 _( "Execute or reload python action plugins" ),
                 KiBitmap( hammer_xpm ) );

    AddMenuItem( submenuActionPluginsMenu, ID_TOOLBARH_PCB_ACTION_PLUGIN_REFRESH,
                 _( "&Refresh Plugins" ),
                 _( "Reload all python plugins and refresh plugin menus" ),
                 KiBitmap( reload_xpm ) );

    submenuActionPluginsMenu->AppendSeparator();
#endif
}


// Build the help menu
void prepareHelpMenu( wxMenu* aParentMenu )
{

    AddMenuItem( aParentMenu, wxID_HELP,
                 _( "Pcbnew &Manual" ),
                 _( "Open Pcbnew Manual" ),
                 KiBitmap( online_help_xpm ) );

    AddMenuItem( aParentMenu, wxID_INDEX,
                 _( "&Getting Started in KiCad" ),
                 _( "Open \"Getting Started in KiCad\" guide for beginners" ),
                 KiBitmap( help_xpm ) );

    AddMenuItem( aParentMenu, ID_PREFERENCES_HOTKEY_SHOW_CURRENT_LIST,
                 _( "&List Hotkeys" ),
                 _( "Display current hotkeys list and corresponding commands" ),
                 KiBitmap( hotkeys_xpm ) );

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_HELP_GET_INVOLVED,
                 _( "Get &Involved" ),
                 _( "Contribute to KiCad - open web browser" ),
                 KiBitmap( info_xpm ) );

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, wxID_ABOUT,
                 _( "&About KiCad" ),
                 _( "Display KiCad About dialog" ),
                 KiBitmap( about_xpm ) );
}


// Build the edit menu
void prepareEditMenu( wxMenu* aParentMenu )
{
    wxString text;

    text  = AddHotkeyName( _( "&Undo" ), g_Pcbnew_Editor_Hokeys_Descr, HK_UNDO );
    AddMenuItem( aParentMenu, wxID_UNDO, text, HELP_UNDO, KiBitmap( undo_xpm ) );

    text  = AddHotkeyName( _( "&Redo" ), g_Pcbnew_Editor_Hokeys_Descr, HK_REDO );
    AddMenuItem( aParentMenu, wxID_REDO, text, HELP_REDO, KiBitmap( redo_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_DELETE_ITEM_BUTT,
                 _( "&Delete" ), _( "Delete items" ),
                 KiBitmap( delete_xpm ) );

    aParentMenu->AppendSeparator();

    text = AddHotkeyName( _( "&Find" ), g_Pcbnew_Editor_Hokeys_Descr, HK_FIND_ITEM );
    AddMenuItem( aParentMenu, ID_FIND_ITEMS, text, HELP_FIND , KiBitmap( find_xpm ) );

    aParentMenu->AppendSeparator();
    AddMenuItem( aParentMenu, ID_PCB_EDIT_ALL_VIAS_AND_TRACK_SIZE,
                     _( "Edit All Tracks and Vias" ), KiBitmap( width_track_via_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_PCB_RESET_TEXTMODULE_FIELDS_SIZES,
                 _( "Set Footp&rint Field Sizes" ),
                 _( "Set text size and width of footprint fields" ),
                 KiBitmap( reset_text_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_PCB_SWAP_LAYERS,
                 _( "&Swap Layers" ),
                 _( "Swap tracks on copper layers or drawings on other layers" ),
                 KiBitmap( swap_layer_xpm ) );

    aParentMenu->AppendSeparator();
    AddMenuItem( aParentMenu, ID_PCB_GLOBAL_DELETE,
                 _( "&Global Deletions" ),
                 _( "Delete tracks, footprints and texts on board" ),
                 KiBitmap( general_deletions_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_PCB_CLEAN,
                 _( "&Cleanup Tracks and Vias" ),
                 _( "Clean stubs, vias, delete break points or unconnected tracks" ),
                 KiBitmap( delete_xpm ) );
}


// Build the view menu
void prepareViewMenu( wxMenu* aParentMenu )
{
    wxString text;
    /* Important Note for ZOOM IN and ZOOM OUT commands from menubar:
     * we cannot add hotkey info here, because the hotkey HK_ZOOM_IN and HK_ZOOM_OUT
     * events(default = WXK_F1 and WXK_F2) are *NOT* equivalent to this menu command:
     * zoom in and out from hotkeys are equivalent to the pop up menu zoom
     * From here, zooming is made around the screen center
     * From hotkeys, zooming is made around the mouse cursor position
     * (obviously not possible from the toolbar or menubar command)
     *
     * in other words HK_ZOOM_IN and HK_ZOOM_OUT *are NOT* accelerators
     * for Zoom in and Zoom out sub menus
     */
    text = AddHotkeyName( _( "Zoom &In" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_ZOOM_IN, IS_ACCELERATOR );
    AddMenuItem( aParentMenu, ID_ZOOM_IN, text, HELP_ZOOM_IN, KiBitmap( zoom_in_xpm ) );

    text = AddHotkeyName( _( "Zoom &Out" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_ZOOM_OUT, IS_ACCELERATOR );
    AddMenuItem( aParentMenu, ID_ZOOM_OUT, text, HELP_ZOOM_OUT, KiBitmap( zoom_out_xpm ) );

    text = AddHotkeyName( _( "&Fit on Screen" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_ZOOM_AUTO  );
    AddMenuItem( aParentMenu, ID_ZOOM_PAGE, text, HELP_ZOOM_FIT,
                 KiBitmap( zoom_fit_in_page_xpm ) );

    text = AddHotkeyName( _( "&Redraw" ), g_Pcbnew_Editor_Hokeys_Descr, HK_ZOOM_REDRAW );
    AddMenuItem( aParentMenu, ID_ZOOM_REDRAW, text,
                 HELP_ZOOM_REDRAW, KiBitmap( zoom_redraw_xpm ) );

    aParentMenu->AppendSeparator();

    text = AddHotkeyName( _( "&3D Viewer" ), g_Pcbnew_Editor_Hokeys_Descr, HK_3D_VIEWER );

    AddMenuItem( aParentMenu, ID_MENU_PCB_SHOW_3D_FRAME, text, _( "Show board in 3D viewer" ),
                 KiBitmap( three_d_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_LIST_NETS,
                 _( "&List Nets" ), _( "View list of nets with names and IDs" ),
                 KiBitmap( list_nets_xpm ) );

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_MENU_PCB_FLIP_VIEW,
                 _( "&Flip Board View" ),
                 _( "Flip (mirror) the board view" ),
                 KiBitmap( flip_board_xpm ), wxITEM_CHECK );


    aParentMenu->AppendSeparator();

    text = AddHotkeyName( _( "Legacy Canva&s" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_CANVAS_LEGACY );

    aParentMenu->Append(
        new wxMenuItem( aParentMenu, ID_MENU_CANVAS_LEGACY,
                        text, _( "Switch canvas implementation to Legacy" ),
                        wxITEM_RADIO ) );

    text = AddHotkeyName( _( "Open&GL Canvas" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_CANVAS_OPENGL );

    aParentMenu->Append(
        new wxMenuItem( aParentMenu, ID_MENU_CANVAS_OPENGL,
                        text, _( "Switch canvas implementation to OpenGL" ),
                        wxITEM_RADIO ) );

    text = AddHotkeyName( _( "&Cairo Canvas" ), g_Pcbnew_Editor_Hokeys_Descr,
                          HK_CANVAS_CAIRO );

    aParentMenu->Append(
        new wxMenuItem( aParentMenu, ID_MENU_CANVAS_CAIRO,
                        text, _( "Switch canvas implementation to Cairo" ),
                        wxITEM_RADIO ) );
}


// Build the dimensions menu
void prepareDimensionsMenu( wxMenu* aParentMenu )
{
    AddMenuItem( aParentMenu, ID_PCB_USER_GRID_SETUP,
                 _( "User Defined G&rid" ),_( "Adjust custom user-defined grid dimensions" ),
                 KiBitmap( grid_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_DRAWINGS_WIDTHS_SETUP,
                 _( "Te&xts and Drawings" ),
                 _( "Adjust dimensions for texts and drawings" ),
                 KiBitmap( text_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_PAD_SETUP,
                 _( "&Pads Size" ),  _( "Adjust default pad characteristics" ),
                 KiBitmap( pad_dimensions_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_MASK_CLEARANCE,
                 _( "Pads to &Mask Clearance" ),
                 _( "Adjust global clearance between pads and solder resist mask" ),
                 KiBitmap( pads_mask_layers_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_DIFF_PAIR_DIMENSIONS,
                 _( "&Differential Pairs" ),
                 _( "Define global gap/width for differential pairs." ),
                 KiBitmap( ps_diff_pair_xpm ) );

    aParentMenu->AppendSeparator();
    AddMenuItem( aParentMenu, ID_CONFIG_SAVE,
                 _( "&Save Preferences" ), _( "Save dimension preferences" ),
                 KiBitmap( save_xpm ) );
}


// Build the files menu.
void prepareFilesMenu( wxMenu* aParentMenu, bool aIsOutsideProject )
{
    wxString text;

    // Some commands are available only if Pcbnew is run outside a project (run alone).
    // aIsOutsideProject is false when Pcbnew is run from Kicad manager.
    if( aIsOutsideProject )
    {
        AddMenuItem( aParentMenu, ID_NEW_BOARD,
                _( "&New Board" ),
                _( "Create new board" ),
                KiBitmap( new_generic_xpm ) );

        text = AddHotkeyName( _( "&Open Board" ), g_Pcbnew_Editor_Hokeys_Descr, HK_LOAD_BOARD );
        AddMenuItem( aParentMenu, ID_LOAD_FILE, text,
                _( "Load existing board" ),
                KiBitmap( open_brd_file_xpm ) );
    }

    wxFileHistory&  fhist = Kiface().GetFileHistory();

    // Load Recent submenu
    static wxMenu* openRecentMenu;

    // Add this menu to list menu managed by m_fileHistory
    // (the file history will be updated when adding/removing files in history
    if( openRecentMenu )
        fhist.RemoveMenu( openRecentMenu );

    openRecentMenu = new wxMenu();

    fhist.UseMenu( openRecentMenu );
    fhist.AddFilesToMenu();

    if( aIsOutsideProject )
    {
        AddMenuItem( aParentMenu, openRecentMenu,
                     -1, _( "Open &Recent" ),
                     _( "Open recently opened board" ),
                     KiBitmap( open_project_xpm ) );

        AddMenuItem( aParentMenu, ID_APPEND_FILE,
                     _( "&Append Board" ),
                     _( "Append another board to currently loaded board" ),
                     KiBitmap( import_xpm ) );

        AddMenuItem( aParentMenu, ID_IMPORT_NON_KICAD_BOARD,
                _( "Import Non-Kicad Board File" ),
                _( "Import board file from other applications" ),
                KiBitmap( import_brd_file_xpm ) );

        aParentMenu->AppendSeparator();
    }

    text = AddHotkeyName( _( "&Save" ), g_Pcbnew_Editor_Hokeys_Descr, HK_SAVE_BOARD );
    AddMenuItem( aParentMenu, ID_SAVE_BOARD, text,
                 _( "Save current board" ),
                 KiBitmap( save_xpm ) );

    // Save as menu:
    // under a project mgr we do not want to modify the board filename
    // to keep consistency with the project mgr which expects files names same as prj name
    // for main files
    // when not under a project mgr, we are free to change filenames, cwd ...
    if( Kiface().IsSingle() )      // not when under a project mgr (pcbnew is run as stand alone)
    {
        text = AddHotkeyName( _( "Sa&ve As" ), g_Pcbnew_Editor_Hokeys_Descr, HK_SAVE_BOARD_AS );
        AddMenuItem( aParentMenu, ID_SAVE_BOARD_AS, text,
                     _( "Save current board with new name" ),
                     KiBitmap( save_as_xpm ) );
    }
    // under a project mgr, we can save a copy of the board,
    // but do not change the current board file name
    else
    {
        text = AddHotkeyName( _( "Sa&ve Copy As" ), g_Pcbnew_Editor_Hokeys_Descr, HK_SAVE_BOARD_AS );
        AddMenuItem( aParentMenu, ID_COPY_BOARD_AS, text,
                     _( "Save copy of the current board" ),
                     KiBitmap( save_as_xpm ) );
    }

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_MENU_READ_BOARD_BACKUP_FILE,
                 _( "Revert to Las&t Backup" ),
                 _( "Clear board and get previous backup version of board" ),
                 KiBitmap( undo_xpm ) );

    AddMenuItem( aParentMenu, ID_MENU_RECOVER_BOARD_AUTOSAVE,
            _( "Resc&ue" ),
            _( "Clear board and get last rescue file automatically saved by Pcbnew" ),
            KiBitmap( rescue_xpm ) );
    aParentMenu->AppendSeparator();

    //----- Fabrication Outputs submenu -----------------------------------------
    wxMenu* fabricationOutputsMenu = new wxMenu;
    AddMenuItem( fabricationOutputsMenu, ID_PCB_GEN_POS_MODULES_FILE,
                 _( "Footprint &Position (.pos) File" ),
                 _( "Generate footprint position file for pick and place" ),
                 KiBitmap( post_compo_xpm ) );

    AddMenuItem( fabricationOutputsMenu, ID_PCB_GEN_DRILL_FILE,
                 _( "&Drill (.drl) File" ),
                 _( "Generate excellon2 drill file" ),
                 KiBitmap( post_drill_xpm ) );

    AddMenuItem( fabricationOutputsMenu, ID_GEN_EXPORT_FILE_MODULE_REPORT,
                 _( "&Footprint (.rpt) Report.." ),
                 _( "Create report of all footprints from current board" ),
                 KiBitmap( tools_xpm ) );

    AddMenuItem( fabricationOutputsMenu, ID_PCB_GEN_D356_FILE,
            _( "IPC-D-356 Netlist File" ),
            _( "Generate IPC-D-356 netlist file" ),
            KiBitmap( netlist_xpm ) );

    AddMenuItem( fabricationOutputsMenu, ID_PCB_GEN_BOM_FILE_FROM_BOARD,
                 _( "&BOM File" ),
                 _( "Create bill of materials from current schematic" ),
                 KiBitmap( bom_xpm ) );

    AddMenuItem( aParentMenu, fabricationOutputsMenu,
                 -1, _( "&Fabrication Outputs" ),
                 _( "Generate files for fabrication" ),
                 KiBitmap( fabrication_xpm ) );

    //----- Import submenu ------------------------------------------------------
    wxMenu* submenuImport = new wxMenu();

    AddMenuItem( submenuImport, ID_GEN_IMPORT_SPECCTRA_SESSION,
                 _( "&Specctra Session" ),
                 _( "Import routed \"Specctra Session\" (*.ses) file" ),
                 KiBitmap( import_xpm ) );

    AddMenuItem( submenuImport, ID_GEN_IMPORT_DXF_FILE,
                 _( "&DXF File" ),
                 _( "Import 2D Drawing DXF file to Pcbnew on Drawings layer" ),
                 KiBitmap( import_xpm ) );

    AddMenuItem( aParentMenu, submenuImport,
                 ID_GEN_IMPORT_FILE, _( "&Import" ),
                 _( "Import files" ), KiBitmap( import_xpm ) );


    //----- Export submenu ------------------------------------------------------
    wxMenu* submenuexport = new wxMenu();
    prepareExportMenu( submenuexport );

    AddMenuItem( aParentMenu, submenuexport,
                 ID_GEN_EXPORT_FILE, _( "E&xport" ),
                 _( "Export board" ), KiBitmap( export_xpm ) );

    aParentMenu->AppendSeparator();

    AddMenuItem( aParentMenu, ID_SHEET_SET,
                 _( "Page S&ettings" ),
                 _( "Settings for sheet size and frame references" ),
                 KiBitmap( sheetset_xpm ) );

    AddMenuItem( aParentMenu, wxID_PRINT,
                 _( "&Print" ), _( "Print board" ),
                 KiBitmap( print_button_xpm ) );

    AddMenuItem( aParentMenu, ID_GEN_PLOT_SVG,
                 _( "Export SV&G" ),
                 _( "Export board file in Scalable Vector Graphics format" ),
                 KiBitmap( plot_svg_xpm ) );

    AddMenuItem( aParentMenu, ID_GEN_PLOT,
                 _( "P&lot" ),
                 _( "Plot board in HPGL, PostScript or Gerber RS-274X format)" ),
                 KiBitmap( plot_xpm ) );

    aParentMenu->AppendSeparator();

    //----- archive submenu -----------------------------------------------------
    wxMenu* submenuarchive = new wxMenu();

    AddMenuItem( submenuarchive, ID_MENU_ARCHIVE_MODULES_IN_LIBRARY,
                 _( "&Archive Footprints in Project Library." ),
                 _( "Archive footprints in existing library in footprint Lib table"
                    "(does not remove other footprints in this library)" ),
                 KiBitmap( library_update_xpm ) );

    AddMenuItem( submenuarchive, ID_MENU_CREATE_LIBRARY_AND_ARCHIVE_MODULES,
                 _( "&Create Library and Archive Footprints." ),
                 _( "Archive all footprints in new library\n"
                    "(if the library already exists it will be deleted)" ),
                 KiBitmap( library_xpm ) );

    AddMenuItem( aParentMenu, submenuarchive,
                 ID_MENU_ARCHIVE_MODULES,
                 _( "Arc&hive Footprints" ),
                 _( "Archive or add footprints in library file" ),
                 KiBitmap( library_xpm ) );

    aParentMenu->AppendSeparator();
    AddMenuItem( aParentMenu, wxID_EXIT, _( "&Close" ), _( "Close Pcbnew" ), KiBitmap( exit_xpm ) );
}


// Build the import/export submenu (inside files menu)
void prepareExportMenu( wxMenu* aParentMenu )
{
    AddMenuItem( aParentMenu, ID_GEN_EXPORT_SPECCTRA,
                 _( "&Specctra DSN" ),
                 _( "Export current board to \"Specctra DSN\" file" ),
                 KiBitmap( export_dsn_xpm ) );

    AddMenuItem( aParentMenu, ID_GEN_EXPORT_FILE_GENCADFORMAT,
                 _( "&GenCAD" ), _( "Export GenCAD format" ),
                 KiBitmap( export_xpm ) );

    AddMenuItem( aParentMenu, ID_GEN_EXPORT_FILE_VRML,
                 _( "&VRML" ),
                 _( "Export VRML board representation" ),
                 KiBitmap( three_d_xpm ) );

    AddMenuItem( aParentMenu, ID_GEN_EXPORT_FILE_IDF3,
                 _( "I&DFv3" ), _( "IDFv3 board and component export" ),
                 KiBitmap( export_idf_xpm ) );

    AddMenuItem( aParentMenu, ID_GEN_EXPORT_FILE_STEP,
                 _( "S&TEP" ), _( "STEP export" ),
                 KiBitmap( export_idf_xpm ) );

    AddMenuItem( aParentMenu, ID_PCB_GEN_CMP_FILE,
                 _( "&Component (.cmp) File" ),
                 _( "Export component file (*.cmp) for Eeschema footprint field back-annotation" ),
                 KiBitmap( create_cmp_file_xpm ) );
}
