/**
 * @file pcbnew/menubar_modedit.cpp
 * @brief (Re)Create the main menubar for the module editor
 */
#include "fctsys.h"

#include "pcbnew.h"
#include "wxPcbStruct.h"
#include "module_editor_frame.h"

#include "protos.h"
#include "pcbnew_id.h"


void FOOTPRINT_EDIT_FRAME::ReCreateMenuBar()
{
    // Create and try to get the current menubar
    wxMenuBar*  menuBar = GetMenuBar();
    wxMenuItem* item;

    if( ! menuBar )
        menuBar = new wxMenuBar();

    // Delete all existing menus so they can be rebuilt.
    // This allows language changes of the menu text on the fly.
    menuBar->Freeze();

    while( menuBar->GetMenuCount() )
        delete menuBar->Remove(0);

    // Recreate all menus:

    // Menu File:
    wxMenu* fileMenu = new wxMenu;

    // New module
    item = new wxMenuItem( fileMenu,
                           ID_MODEDIT_NEW_MODULE,
                           _( "New Module" ),
                           _( "Create new module" ) );
    SET_BITMAP( KiBitmap( new_footprint_xpm ) );
    fileMenu->Append( item );

    // Open submenu
    wxMenu* openSubmenu = new wxMenu;

    // from File
    item = new wxMenuItem( openSubmenu,
                           ID_MODEDIT_IMPORT_PART,
                           _( "Load from File (Import)" ),
                           _( "Import a footprint from an existing file" ) );
    SET_BITMAP( KiBitmap( import_module_xpm ) );
    openSubmenu->Append( item );

    // from Library
    item = new wxMenuItem( openSubmenu,
                           ID_MODEDIT_LOAD_MODULE,
                           _( "Load from Library" ),
                           _( "Open a footprint module from a Library" ) );
    SET_BITMAP( KiBitmap( module_xpm ) );
    openSubmenu->Append( item );

    // from current Board
    item = new wxMenuItem( openSubmenu,
                           ID_MODEDIT_LOAD_MODULE_FROM_BOARD,
                           _( "Load from Current Board" ),
                           _( "Load a footprint module from the current loaded board" ) );
    SET_BITMAP( KiBitmap( load_module_board_xpm ) );
    openSubmenu->Append( item );

    /* Append openSubmenu to fileMenu */
    AddMenuItem( fileMenu, openSubmenu, -1,
                 _( "&Load Module" ),
                 _( "Load a footprint module" ),
                 KiBitmap( open_document_xpm ) );

    // Save module
    item = new wxMenuItem( fileMenu,
                           ID_MODEDIT_SAVE_LIBMODULE,
                           _( "&Save Module in Active Library" ),
                           _( "Save module in active library" ) );
    SET_BITMAP( KiBitmap( save_library_xpm ) );
    fileMenu->Append( item );

    // Save module in new lib
    item = new wxMenuItem( fileMenu,
                           ID_MODEDIT_CREATE_NEW_LIB_AND_SAVE_CURRENT_PART,
                           _( "&Save Module in a New Lib" ),
                           _( "Create new library and save current module" ) );
    SET_BITMAP( KiBitmap( new_library_xpm ) );
    fileMenu->Append( item );

    // Export module
    item = new wxMenuItem( fileMenu,
                           ID_MODEDIT_EXPORT_PART,
                           _( "&Export Module" ),
                           _( "Save the current loaded module to a file" ) );
    SET_BITMAP( KiBitmap( export_module_xpm ) );
    fileMenu->Append( item );

    // Separator
    fileMenu->AppendSeparator();

    // Print
    item = new wxMenuItem( fileMenu,
                           wxID_PRINT,
                           _( "&Print\tCtrl+P" ),
                           _( "Print the current module" ) );
    SET_BITMAP( KiBitmap( plot_xpm ) );
    fileMenu->Append( item );

    // Separator
    fileMenu->AppendSeparator();

    // Close editor
    item = new wxMenuItem( fileMenu,
                           wxID_EXIT,
                           _( "Close" ),
                           _( "Close the footprint editor" ) );
    SET_BITMAP( KiBitmap( exit_xpm ) );
    fileMenu->Append( item );

    // Menu Edit:
    wxMenu* editMenu = new wxMenu;

    // Undo
    item = new wxMenuItem( editMenu,
                           wxID_UNDO,
                           _( "Undo" ),
                           _( "Undo last edit" ) );
    SET_BITMAP( KiBitmap( undo_xpm ) );
    editMenu->Append( item );

    // Redo
    item = new wxMenuItem( editMenu,
                           wxID_REDO,
                           _( "Redo" ),
                           _( "Redo the last undo action" ) );
    SET_BITMAP( KiBitmap( redo_xpm ) );
    editMenu->Append( item );

    // Delete items
    item = new wxMenuItem( editMenu,
                           ID_MODEDIT_DELETE_TOOL,
                           _( "Delete" ),
                           _( "Delete objects with the eraser" ) );
    SET_BITMAP( KiBitmap( delete_body_xpm ) );
    editMenu->Append( item );

    // Separator
    editMenu->AppendSeparator();

    // Properties
    item = new wxMenuItem( editMenu,
                           ID_MODEDIT_EDIT_MODULE_PROPERTIES,
                           _( "Properties" ),
                           _( "Edit module properties" ) );
    SET_BITMAP( KiBitmap( module_options_xpm ) );
    editMenu->Append( item );

    // Dimensions submenu
    wxMenu* dimensions_Submenu = new wxMenu;

    // Sizes and Widths
    item = new wxMenuItem( dimensions_Submenu,
                           ID_PCB_DRAWINGS_WIDTHS_SETUP,
                           _( "Sizes and Widths" ),
                           _( "Adjust width for texts and drawings" ) );
    SET_BITMAP( KiBitmap( options_text_xpm ) );
    dimensions_Submenu->Append( item );

    // Pad settings
    item = new wxMenuItem( dimensions_Submenu,
                           ID_MODEDIT_PAD_SETTINGS,
                           _( "Pad Settings" ),
                           _( "Edit the settings for new pads" ) );
    SET_BITMAP( KiBitmap( options_pad_xpm ) );
    dimensions_Submenu->Append( item );

    // User grid size
    item = new wxMenuItem( dimensions_Submenu,
                           ID_PCB_USER_GRID_SETUP,
                           _( "User Grid Size" ),
                           _( "Adjust user grid" ) );
    SET_BITMAP( KiBitmap( grid_xpm ) );
    dimensions_Submenu->Append( item );

    // Append dimensions_Submenu to editMenu
    AddMenuItem( editMenu,
                 dimensions_Submenu, -1,
                 _( "&Dimensions" ),
                 _( "Edit dimensions preferences" ),
                 KiBitmap( add_dimension_xpm ) );

    // View menu
    wxMenu* viewMenu = new wxMenu;

    // Zoom In
    item = new wxMenuItem( viewMenu,
                           ID_ZOOM_IN,
                           _( "Zoom In" ),
                           _( "Zoom in on the module" ) );
    SET_BITMAP( KiBitmap( zoom_in_xpm ) );
    viewMenu->Append( item );

    // Zoom Out
    item = new wxMenuItem( viewMenu,
                           ID_ZOOM_OUT,
                           _( "Zoom Out" ),
                           _( "Zoom out on the module" ) );
    SET_BITMAP( KiBitmap( zoom_out_xpm ) );
    viewMenu->Append( item );

    // Fit on Screen
    item = new wxMenuItem( viewMenu,
                           ID_ZOOM_PAGE,
                           _( "Fit on Screen" ),
                           _( "Zoom and fit the module in the window" ) );
    SET_BITMAP( KiBitmap( zoom_fit_in_page_xpm ) );
    viewMenu->Append( item );

    // Separator
    viewMenu->AppendSeparator();

    // Redraw
    item = new wxMenuItem( viewMenu,
                           ID_ZOOM_REDRAW,
                           _( "Redraw" ),
                           _( "Redraw the window's viewport" ) );
    SET_BITMAP( KiBitmap( zoom_redraw_xpm ) );
    viewMenu->Append( item );

    // 3D view
    item = new wxMenuItem( viewMenu,
                           ID_MENU_PCB_SHOW_3D_FRAME,
                           _( "3D View" ),
                           _( "Show board in 3D viewer" ) );
    SET_BITMAP( KiBitmap( three_d_xpm ) );
    viewMenu->Append( item );

    // Menu Place:
    wxMenu* placeMenu = new wxMenu;

    // Pad
    item = new wxMenuItem( placeMenu,
                           ID_MODEDIT_PAD_TOOL,
                           _( "Pad" ),
                           _( "Add pad" ) );
    SET_BITMAP( KiBitmap( pad_xpm ) );
    placeMenu->Append( item );

    // Separator
    placeMenu->AppendSeparator();

    // Circle
    item = new wxMenuItem( placeMenu,
                           ID_MODEDIT_CIRCLE_TOOL,
                           _( "Circle" ),
                           _( "Add graphic circle" ) );
    SET_BITMAP( KiBitmap( add_circle_xpm ) );
    placeMenu->Append( item );

    // Line or Polygon
    item = new wxMenuItem( placeMenu,
                           ID_MODEDIT_LINE_TOOL,
                           _( "Line or Polygon" ),
                           _( "Add graphic line or polygon" ) );
    SET_BITMAP( KiBitmap( add_polygon_xpm ) );
    placeMenu->Append( item );

    // Arc
    item = new wxMenuItem( placeMenu,
                           ID_MODEDIT_ARC_TOOL,
                           _( "Arc" ),
                           _( "Add graphic arc" ) );
    SET_BITMAP( KiBitmap( add_arc_xpm ) );
    placeMenu->Append( item );

    // Text
    item = new wxMenuItem( placeMenu,
                           ID_MODEDIT_TEXT_TOOL,
                           _( "Text" ),
                           _( "Add graphic text" ) );
    SET_BITMAP( KiBitmap( add_text_xpm ) );
    placeMenu->Append( item );

    // Anchor
    placeMenu->AppendSeparator();
    item = new wxMenuItem( placeMenu,
                           ID_MODEDIT_ANCHOR_TOOL,
                           _( "Anchor" ),
                           _( "Place the footprint module reference anchor" ) );
    SET_BITMAP( KiBitmap( anchor_xpm ) );
    placeMenu->Append( item );

    // Menu Help:
    wxMenu* helpMenu = new wxMenu;

    // Version info
    AddHelpVersionInfoMenuEntry( helpMenu );

    // Contents
    AddMenuItem( helpMenu,
                 wxID_HELP,
                 _( "&Contents" ),
                 _( "Open the Pcbnew handbook" ),
                 KiBitmap( online_help_xpm ) );
    AddMenuItem( helpMenu,
                 wxID_INDEX,
                 _( "&Getting Started in KiCad" ),
                 _( "Open the \"Getting Started in KiCad\" guide for beginners" ),
                 KiBitmap( help_xpm ) );

    // About Pcbnew
    helpMenu->AppendSeparator();
    AddMenuItem( helpMenu, wxID_ABOUT,
                 _( "&About Pcbnew" ),
                 _( "About Pcbnew PCB designer" ),
                 KiBitmap( info_xpm ) );

    // Append menus to the menubar
    menuBar->Append( fileMenu,  _( "&File" ) );
    menuBar->Append( editMenu,  _( "&Edit" ) );
    menuBar->Append( viewMenu,  _( "&View" ) );
    menuBar->Append( placeMenu, _( "&Place" ) );
    menuBar->Append( helpMenu,  _( "&Help" ) );

    menuBar->Thaw();

    // Associate the menu bar with the frame, if no previous menubar
    if( GetMenuBar() == NULL )
        SetMenuBar( menuBar );
    else
        menuBar->Refresh();
}
