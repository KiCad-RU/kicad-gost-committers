/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2013 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file schframe.cpp
 */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <gestfich.h>
#include <confirm.h>
#include <base_units.h>
#include <msgpanel.h>

#include <general.h>
#include <eeschema_id.h>
#include <netlist.h>
#include <lib_pin.h>
#include <class_library.h>
#include <wxEeschemaStruct.h>
#include <sch_component.h>

#include <dialog_helpers.h>
#include <dialog_netlist.h>
#include <libeditframe.h>
#include <viewlib_frame.h>
#include <hotkeys.h>
#include <eeschema_config.h>
#include <sch_sheet.h>

#include <dialogs/annotate_dialog.h>
#include <dialogs/dialog_build_BOM.h>
#include <dialogs/dialog_erc.h>
#include <dialogs/dialog_print_using_printer.h>
#include <dialogs/dialog_schematic_find.h>

#include <wx/display.h>
#include <build_version.h>
#include <wildcards_and_files_ext.h>



BEGIN_EVENT_TABLE( SCH_EDIT_FRAME, EDA_DRAW_FRAME )
    EVT_SOCKET( ID_EDA_SOCKET_EVENT_SERV, EDA_DRAW_FRAME::OnSockRequestServer )
    EVT_SOCKET( ID_EDA_SOCKET_EVENT, EDA_DRAW_FRAME::OnSockRequest )

    EVT_CLOSE( SCH_EDIT_FRAME::OnCloseWindow )
    EVT_SIZE( SCH_EDIT_FRAME::OnSize )

    EVT_MENU( ID_NEW_PROJECT, SCH_EDIT_FRAME::OnNewProject )
    EVT_MENU( ID_LOAD_PROJECT, SCH_EDIT_FRAME::OnLoadProject )

    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, SCH_EDIT_FRAME::OnLoadFile )

    EVT_MENU( ID_APPEND_PROJECT, SCH_EDIT_FRAME::OnAppendProject )

    EVT_TOOL( ID_NEW_PROJECT, SCH_EDIT_FRAME::OnNewProject )
    EVT_TOOL( ID_LOAD_PROJECT, SCH_EDIT_FRAME::OnLoadProject )

    EVT_MENU( ID_SAVE_PROJECT, SCH_EDIT_FRAME::OnSaveProject )
    EVT_MENU( ID_UPDATE_ONE_SHEET, SCH_EDIT_FRAME::Save_File )
    EVT_MENU( ID_SAVE_ONE_SHEET_UNDER_NEW_NAME, SCH_EDIT_FRAME::Save_File )
    EVT_MENU( ID_GEN_PLOT_SCHEMATIC, SCH_EDIT_FRAME::PlotSchematic )
    EVT_MENU( ID_GEN_COPY_SHEET_TO_CLIPBOARD, EDA_DRAW_FRAME::CopyToClipboard )
    EVT_MENU( wxID_EXIT, SCH_EDIT_FRAME::OnExit )

    EVT_MENU( ID_POPUP_SCH_COPY_ITEM, SCH_EDIT_FRAME::OnCopySchematicItemRequest )

    EVT_MENU( ID_CONFIG_REQ, SCH_EDIT_FRAME::InstallConfigFrame )
    EVT_MENU( ID_CONFIG_SAVE, SCH_EDIT_FRAME::Process_Config )
    EVT_MENU( ID_CONFIG_READ, SCH_EDIT_FRAME::Process_Config )
    EVT_MENU_RANGE( ID_PREFERENCES_HOTKEY_START, ID_PREFERENCES_HOTKEY_END,
                    SCH_EDIT_FRAME::Process_Config )

    EVT_MENU( ID_COLORS_SETUP, SCH_EDIT_FRAME::OnColorConfig )
    EVT_TOOL( wxID_PREFERENCES, SCH_EDIT_FRAME::OnSetOptions )

    EVT_MENU_RANGE( ID_LANGUAGE_CHOICE, ID_LANGUAGE_CHOICE_END, SCH_EDIT_FRAME::SetLanguage )

    EVT_TOOL( ID_TO_LIBRARY, SCH_EDIT_FRAME::OnOpenLibraryEditor )
    EVT_TOOL( ID_POPUP_SCH_CALL_LIBEDIT_AND_LOAD_CMP, SCH_EDIT_FRAME::OnOpenLibraryEditor )
    EVT_TOOL( ID_TO_LIBVIEW, SCH_EDIT_FRAME::OnOpenLibraryViewer )

    EVT_TOOL( ID_TO_PCB, SCH_EDIT_FRAME::OnOpenPcbnew )
    EVT_TOOL( ID_TO_CVPCB, SCH_EDIT_FRAME::OnOpenCvpcb )

    EVT_TOOL( ID_SHEET_SET, EDA_DRAW_FRAME::Process_PageSettings )
    EVT_TOOL( ID_HIERARCHY, SCH_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_CUT, SCH_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_COPY, SCH_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_PASTE, SCH_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_UNDO, SCH_EDIT_FRAME::GetSchematicFromUndoList )
    EVT_TOOL( wxID_REDO, SCH_EDIT_FRAME::GetSchematicFromRedoList )
    EVT_TOOL( ID_GET_ANNOTATE, SCH_EDIT_FRAME::OnAnnotate )
    EVT_TOOL( wxID_PRINT, SCH_EDIT_FRAME::OnPrint )
    EVT_TOOL( ID_GET_ERC, SCH_EDIT_FRAME::OnErc )
    EVT_TOOL( ID_GET_NETLIST, SCH_EDIT_FRAME::OnCreateNetlist )
    EVT_TOOL( ID_GET_TOOLS, SCH_EDIT_FRAME::OnCreateBillOfMaterials )
    EVT_TOOL( ID_FIND_ITEMS, SCH_EDIT_FRAME::OnFindItems )
    EVT_TOOL( wxID_REPLACE, SCH_EDIT_FRAME::OnFindItems )
    EVT_TOOL( ID_BACKANNO_ITEMS, SCH_EDIT_FRAME::OnLoadCmpToFootprintLinkFile )
    EVT_TOOL( ID_SCH_MOVE_ITEM, SCH_EDIT_FRAME::OnMoveItem )
    EVT_MENU( wxID_HELP, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( wxID_INDEX, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( wxID_ABOUT, EDA_BASE_FRAME::GetKicadAbout )

    // Tools and buttons for vertical toolbar.
    EVT_TOOL( ID_NO_TOOL_SELECTED, SCH_EDIT_FRAME::OnSelectTool )
    EVT_TOOL_RANGE( ID_SCHEMATIC_VERTICAL_TOOLBAR_START, ID_SCHEMATIC_VERTICAL_TOOLBAR_END,
                    SCH_EDIT_FRAME::OnSelectTool )

    EVT_MENU( ID_CANCEL_CURRENT_COMMAND, SCH_EDIT_FRAME::OnCancelCurrentCommand )
    EVT_MENU( ID_SCH_DRAG_ITEM, SCH_EDIT_FRAME::OnDragItem )
    EVT_MENU_RANGE( ID_SCH_ROTATE_CLOCKWISE, ID_SCH_ROTATE_COUNTERCLOCKWISE,
                    SCH_EDIT_FRAME::OnRotate )
    EVT_MENU_RANGE( ID_SCH_EDIT_ITEM, ID_SCH_EDIT_COMPONENT_FOOTPRINT,
                    SCH_EDIT_FRAME::OnEditItem )
    EVT_MENU_RANGE( ID_SCH_MIRROR_X, ID_SCH_ORIENT_NORMAL, SCH_EDIT_FRAME::OnOrient )
    EVT_MENU_RANGE( ID_POPUP_START_RANGE, ID_POPUP_END_RANGE,
                    SCH_EDIT_FRAME::Process_Special_Functions )

    // Tools and buttons options toolbar
    EVT_TOOL( ID_TB_OPTIONS_HIDDEN_PINS, SCH_EDIT_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_BUS_WIRES_ORIENT, SCH_EDIT_FRAME::OnSelectOptionToolbar )

    EVT_MENU_RANGE( ID_POPUP_GENERAL_START_RANGE, ID_POPUP_GENERAL_END_RANGE,
                    SCH_EDIT_FRAME::Process_Special_Functions )
    EVT_MENU_RANGE( ID_POPUP_SCH_SELECT_UNIT1, ID_POPUP_SCH_SELECT_UNIT26,
                    SCH_EDIT_FRAME::OnSelectUnit )
    EVT_MENU_RANGE( ID_POPUP_SCH_CHANGE_TYPE_TEXT, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT,
                    SCH_EDIT_FRAME::OnConvertTextType )

    // Multple item selection context menu commands.
    EVT_MENU_RANGE( ID_SELECT_ITEM_START, ID_SELECT_ITEM_END, SCH_EDIT_FRAME::OnSelectItem )

    /* Handle user interface update events. */
    EVT_UPDATE_UI( wxID_CUT, SCH_EDIT_FRAME::OnUpdateBlockSelected )
    EVT_UPDATE_UI( wxID_COPY, SCH_EDIT_FRAME::OnUpdateBlockSelected )
    EVT_UPDATE_UI( wxID_PASTE, SCH_EDIT_FRAME::OnUpdatePaste )
    EVT_UPDATE_UI( ID_TB_OPTIONS_HIDDEN_PINS, SCH_EDIT_FRAME::OnUpdateHiddenPins )
    EVT_UPDATE_UI( ID_TB_OPTIONS_BUS_WIRES_ORIENT, SCH_EDIT_FRAME::OnUpdateBusOrientation )
    EVT_UPDATE_UI( ID_NO_TOOL_SELECTED, SCH_EDIT_FRAME::OnUpdateSelectTool )
    EVT_UPDATE_UI_RANGE( ID_SCHEMATIC_VERTICAL_TOOLBAR_START, ID_SCHEMATIC_VERTICAL_TOOLBAR_END,
                         SCH_EDIT_FRAME::OnUpdateSelectTool )

    /* Search dialog events. */
    EVT_FIND_CLOSE( wxID_ANY, SCH_EDIT_FRAME::OnFindDialogClose )
    EVT_FIND_DRC_MARKER( wxID_ANY, SCH_EDIT_FRAME::OnFindDrcMarker )
    EVT_FIND( wxID_ANY, SCH_EDIT_FRAME::OnFindSchematicItem )
    EVT_FIND_REPLACE( wxID_ANY, SCH_EDIT_FRAME::OnFindReplace )
    EVT_FIND_REPLACE_ALL( wxID_ANY, SCH_EDIT_FRAME::OnFindReplace )

END_EVENT_TABLE()

#define SCH_EDIT_FRAME_NAME wxT( "SchematicFrame" )

SCH_EDIT_FRAME::SCH_EDIT_FRAME( wxWindow* aParent, const wxString& aTitle,
                    const wxPoint& aPosition, const wxSize& aSize,
                    long aStyle ) :
    SCH_BASE_FRAME( aParent, SCHEMATIC_FRAME_TYPE, aTitle, aPosition, aSize,
                    aStyle, SCH_EDIT_FRAME_NAME )
{
    m_FrameName = SCH_EDIT_FRAME_NAME;
    m_showAxis = false;                 // true to show axis
    m_showBorderAndTitleBlock = true;   // true to show sheet references
    m_CurrentSheet = new SCH_SHEET_PATH();
    m_TextFieldSize = DEFAULT_SIZE_TEXT;
    m_DefaultSchematicFileName = NAMELESS_PROJECT;
    m_DefaultSchematicFileName += wxT( ".sch" );
    m_showAllPins = false;
    m_previewPosition = wxDefaultPosition;
    m_previewSize = wxDefaultSize;
    m_printMonochrome = true;
    m_printSheetReference = true;
    m_HotkeysZoomAndGridList = s_Schematic_Hokeys_Descr;
    m_dlgFindReplace = NULL;
    m_findReplaceData = new wxFindReplaceData( wxFR_DOWN );
    m_undoItem = NULL;
    m_hasAutoSave = true;
    SetForceHVLines( true );
    SetDefaultLabelSize( DEFAULT_SIZE_TEXT );

    CreateScreens();

    // Give an icon
    wxIcon icon;
    icon.CopyFromBitmap( KiBitmap( icon_eeschema_xpm ) );
    SetIcon( icon );

    m_itemToRepeat = NULL;

    /* Get config */
    LoadSettings();

    // Initialize grid id to the default value (50 mils):
    m_LastGridSizeId = ID_POPUP_GRID_LEVEL_50 - ID_POPUP_GRID_LEVEL_1000;
    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    if( m_canvas )
        m_canvas->SetEnableBlockCommands( true );

    ReCreateMenuBar();
    ReCreateHToolbar();
    ReCreateVToolbar();
    ReCreateOptToolbar();

    // Initialize common print setup dialog settings.
    m_pageSetupData.GetPrintData().SetPrintMode( wxPRINT_MODE_PRINTER );
    m_pageSetupData.GetPrintData().SetQuality( wxPRINT_QUALITY_MEDIUM );
    m_pageSetupData.GetPrintData().SetBin( wxPRINTBIN_AUTO );
    m_pageSetupData.GetPrintData().SetNoCopies( 1 );

    m_auimgr.SetManagedWindow( this );

    EDA_PANEINFO horiz;
    horiz.HorizontalToolbarPane();

    EDA_PANEINFO vert;
    vert.VerticalToolbarPane();

    EDA_PANEINFO mesg;
    mesg.MessageToolbarPane();



    if( m_mainToolBar )
        m_auimgr.AddPane( m_mainToolBar,
                          wxAuiPaneInfo( horiz ).Name( wxT( "m_mainToolBar" ) ).Top().Row( 0 ) );

    if( m_drawToolBar )
        m_auimgr.AddPane( m_drawToolBar, wxAuiPaneInfo( vert ).Name( wxT( "m_drawToolBar" ) ).Right() );

    if( m_optionsToolBar )
        m_auimgr.AddPane( m_optionsToolBar,
                          wxAuiPaneInfo( vert ).Name( wxT( "m_optionsToolBar" ) ).Left() );

    if( m_canvas )
        m_auimgr.AddPane( m_canvas, wxAuiPaneInfo().Name( wxT( "DrawFrame" ) ).CentrePane() );

    if( m_messagePanel )
        m_auimgr.AddPane( m_messagePanel, wxAuiPaneInfo( mesg ).Name( wxT( "MsgPanel" ) ).Bottom().
                          Layer(10) );

    m_auimgr.Update();

    // Now Drawpanel is sized, we can use BestZoom to show the component (if any)
    GetScreen()->SetZoom( BestZoom() );
}


SCH_EDIT_FRAME::~SCH_EDIT_FRAME()
{
    SetScreen( NULL );
    SAFE_DELETE( m_CurrentSheet );     // a SCH_SHEET_PATH, on the heap.
    SAFE_DELETE( m_undoItem );
    SAFE_DELETE( g_RootSheet );
    SAFE_DELETE( m_findReplaceData );
    CMP_LIBRARY::RemoveAllLibraries();
}


void SCH_EDIT_FRAME::SetSheetNumberAndCount()
{
    SCH_SCREEN* screen = GetScreen();
    SCH_SCREENS s_list;

    /* Set the sheet count, and the sheet number (1 for root sheet)
     */
    int            sheet_count = g_RootSheet->CountSheets();
    int            SheetNumber = 1;
    wxString       current_sheetpath = m_CurrentSheet->Path();
    SCH_SHEET_LIST SheetList;

    // Examine all sheets path to find the current sheets path,
    // and count them from root to the current sheet path:
    SCH_SHEET_PATH* sheet;

    for( sheet = SheetList.GetFirst(); sheet != NULL; sheet = SheetList.GetNext() )
    {
        wxString sheetpath = sheet->Path();

        if( sheetpath == current_sheetpath )    // Current sheet path found
            break;

        SheetNumber++;                          /* Not found, increment sheet
                                                 * number before this current
                                                 * path */
    }

    for( screen = s_list.GetFirst(); screen != NULL; screen = s_list.GetNext() )
    {
        screen->m_NumberOfScreens = sheet_count;
    }

    GetScreen()->m_ScreenNumber = SheetNumber;
}


SCH_SCREEN* SCH_EDIT_FRAME::GetScreen() const
{
    return m_CurrentSheet->LastScreen();
}


wxString SCH_EDIT_FRAME::GetScreenDesc()
{
    wxString s = m_CurrentSheet->PathHumanReadable();

    return s;
}


void SCH_EDIT_FRAME::CreateScreens()
{
    if( g_RootSheet == NULL )
    {
        g_RootSheet = new SCH_SHEET();
    }

    if( g_RootSheet->GetScreen() == NULL )
    {
        g_RootSheet->SetScreen( new SCH_SCREEN() );
        SetScreen( g_RootSheet->GetScreen() );
    }

    g_RootSheet->GetScreen()->SetFileName( m_DefaultSchematicFileName );

    TITLE_BLOCK tb = g_RootSheet->GetScreen()->GetTitleBlock();
    tb.SetDate();
    g_RootSheet->GetScreen()->SetTitleBlock( tb );

    m_CurrentSheet->Clear();
    m_CurrentSheet->Push( g_RootSheet );

    if( GetScreen() == NULL )
        SetScreen( new SCH_SCREEN() );

    GetScreen()->SetZoom( 32.0 );
    GetScreen()->m_UndoRedoCountMax = 10;
}


SCH_SHEET_PATH& SCH_EDIT_FRAME::GetCurrentSheet()
{
    wxASSERT_MSG( m_CurrentSheet != NULL, wxT( "SCH_EDIT_FRAME m_CurrentSheet member is NULL." ) );

    return *m_CurrentSheet;
}


void SCH_EDIT_FRAME::SetCurrentSheet( const SCH_SHEET_PATH& aSheet )
{
    *m_CurrentSheet = aSheet;
}


void SCH_EDIT_FRAME::SetUndoItem( const SCH_ITEM* aItem )
{
    // if aItem != NULL, delete a previous m_undoItem, if exists
    // if aItme = NULL, just clear m_undoItem,
    // because when calling SetUndoItem( NULL ), we only clear m_undoItem,
    // because the owner of m_undoItem is no more me.
    if( aItem && m_undoItem )
    {
        delete m_undoItem;
    }

    m_undoItem = NULL;

    if( aItem )
        m_undoItem = (SCH_ITEM*) aItem->Clone();

}


void SCH_EDIT_FRAME::SaveUndoItemInUndoList( SCH_ITEM* aItem )
{
    wxCHECK_RET( aItem != NULL,
                 wxT( "Cannot swap undo item structures.  Bad programmer!." ) );
    wxCHECK_RET( m_undoItem != NULL,
                 wxT( "Cannot swap undo item structures.  Bad programmer!." ) );
    wxCHECK_RET( aItem->Type() == m_undoItem->Type(),
                 wxT( "Cannot swap undo item structures.  Bad programmer!." ) );

    aItem->SwapData( m_undoItem );
    SaveCopyInUndoList( aItem, UR_CHANGED );
    aItem->SwapData( m_undoItem );
}


void SCH_EDIT_FRAME::OnCloseWindow( wxCloseEvent& aEvent )
{
    LIB_EDIT_FRAME * libeditFrame = LIB_EDIT_FRAME::GetActiveLibraryEditor();;
    if( libeditFrame && !libeditFrame->Close() )   // Can close component editor?
        return;

    LIB_VIEW_FRAME * viewlibFrame = LIB_VIEW_FRAME::GetActiveLibraryViewer();;
    if( viewlibFrame && !viewlibFrame->Close() )   // Can close component viewer?
        return;

    SCH_SHEET_LIST SheetList;

    if( SheetList.IsModified() )
    {
        wxString msg;
        msg.Printf( _("Save the changes in\n<%s>\nbefore closing?"),
                    GetChars( g_RootSheet->GetScreen()->GetFileName() ) );

        int ii = DisplayExitDialog( this, msg );

        switch( ii )
        {
        case wxID_CANCEL:
            aEvent.Veto();
            return;

        case wxID_NO:
            break;

        case wxID_OK:
        case wxID_YES:
            wxCommandEvent tmp( ID_SAVE_PROJECT );
            OnSaveProject( tmp );
            break;
        }
    }

    // Close the find dialog and perserve it's setting if it is displayed.
    if( m_dlgFindReplace )
    {
        m_findDialogPosition = m_dlgFindReplace->GetPosition();
        m_findDialogSize = m_dlgFindReplace->GetSize();
        m_findStringHistoryList = m_dlgFindReplace->GetFindEntries();
        m_replaceStringHistoryList = m_dlgFindReplace->GetReplaceEntries();
        m_dlgFindReplace->Destroy();
        m_dlgFindReplace = NULL;
    }

    SCH_SCREENS screens;
    wxFileName fn;

    for( SCH_SCREEN* screen = screens.GetFirst(); screen != NULL; screen = screens.GetNext() )
    {
        fn = screen->GetFileName();

        // Auto save file name is the normal file name prepended with $.
        fn.SetName( wxT( "$" ) + fn.GetName() );

        if( fn.FileExists() && fn.IsFileWritable() )
            wxRemoveFile( fn.GetFullPath() );
    }

    SheetList.ClearModifyStatus();

    if( !g_RootSheet->GetScreen()->GetFileName().IsEmpty()
       && (g_RootSheet->GetScreen()->GetDrawItems() != NULL) )
        UpdateFileHistory( g_RootSheet->GetScreen()->GetFileName() );

    g_RootSheet->GetScreen()->Clear();

    /* all sub sheets are deleted, only the main sheet is usable */
    m_CurrentSheet->Clear();
    SaveSettings();
    Destroy();
}


double SCH_EDIT_FRAME::BestZoom()
{
    int    dx, dy;
    wxSize size;

    dx = GetScreen()->GetPageSettings().GetWidthIU();
    dy = GetScreen()->GetPageSettings().GetHeightIU();

    size = m_canvas->GetClientSize();

    // Reserve no margin because best zoom shows the full page
    // and margins are already included in function that draws the sheet refernces
    double margin_scale_factor = 1.0;
    double zx =(double) dx / ( margin_scale_factor * (double)size.x );
    double zy = (double) dy / ( margin_scale_factor * (double)size.y );

    double bestzoom = std::max( zx, zy );

    GetScreen()->SetScrollCenterPosition( wxPoint( dx / 2, dy / 2 ) );

    return bestzoom;
}


wxString SCH_EDIT_FRAME::GetUniqueFilenameForCurrentSheet()
{
    wxFileName fn = GetScreen()->GetFileName();

#ifndef KICAD_GOST
    wxString filename = fn.GetName();
    if( ( filename.Len() + m_CurrentSheet->PathHumanReadable().Len() ) < 50 )
#else
    fn.ClearExt();
    wxString filename = fn.GetFullPath();
    if( ( filename.Len() + m_CurrentSheet->PathHumanReadable().Len() ) < 80 )
#endif

    {
        filename += m_CurrentSheet->PathHumanReadable();
        filename.Replace( wxT( "/" ), wxT( "-" ) );
        filename.RemoveLast();
#if defined(KICAD_GOST)
#ifndef __WINDOWS__
        wxString newfn;
        if( filename.StartsWith( wxT( "-" ), &newfn ) )
            filename = newfn;
#endif
#endif
    }
    else
    {
        filename << wxT( "-" ) << GetScreen()->m_ScreenNumber;
    }

    return filename;
}


void SCH_EDIT_FRAME::OnModify()
{
    GetScreen()->SetModify();
    GetScreen()->SetSave();

    if( m_dlgFindReplace == NULL )
        m_foundItems.SetForceSearch();

    SCH_SCREENS s_list;

    // Set the date for each sheet
    // There are 2 possibilities:
    // >> change only the current sheet
    // >> change all sheets.
    // I believe all sheets in a project must have the same date
    s_list.SetDate();
}


/*****************************************************************************
* Enable or disable menu entry and toolbar buttons according to current
* conditions.
*****************************************************************************/

void SCH_EDIT_FRAME::OnUpdateBlockSelected( wxUpdateUIEvent& event )
{
    bool enable = ( GetScreen() && GetScreen()->m_BlockLocate.GetCommand() == BLOCK_MOVE );

    event.Enable( enable );
}


void SCH_EDIT_FRAME::OnUpdatePaste( wxUpdateUIEvent& event )
{
    event.Enable( m_blockItems.GetCount() > 0 );
}


void SCH_EDIT_FRAME::OnUpdateBusOrientation( wxUpdateUIEvent& aEvent )
{
    wxString tool_tip = GetForceHVLines() ?
                        _( "Draw wires and buses in any direction" ) :
                        _( "Draw horizontal and vertical wires and buses only" );

    aEvent.Check( GetForceHVLines() );
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_BUS_WIRES_ORIENT, tool_tip );
}


void SCH_EDIT_FRAME::OnUpdateHiddenPins( wxUpdateUIEvent& aEvent )
{
    wxString tool_tip = m_showAllPins ? _( "Do not show hidden pins" ) :
                        _( "Show hidden pins" );

    aEvent.Check( m_showAllPins );
    m_optionsToolBar->SetToolShortHelp( ID_TB_OPTIONS_HIDDEN_PINS, tool_tip );
}


void SCH_EDIT_FRAME::OnAnnotate( wxCommandEvent& event )
{
    DIALOG_ANNOTATE* dlg = new DIALOG_ANNOTATE( this );

    dlg->ShowModal();
    dlg->Destroy();
}


void SCH_EDIT_FRAME::OnErc( wxCommandEvent& event )
{
    DIALOG_ERC* dlg = new DIALOG_ERC( this );

    dlg->Show( true );
//    dlg->Destroy();
}


void SCH_EDIT_FRAME::OnCreateNetlist( wxCommandEvent& event )
{
    int i;

    do
    {
        NETLIST_DIALOG* dlg = new NETLIST_DIALOG( this );
        i = dlg->ShowModal();
        dlg->Destroy();
    } while( i == NET_PLUGIN_CHANGE );

    // If a plugin is removed or added, rebuild and reopen the new dialog
}


void SCH_EDIT_FRAME::OnCreateBillOfMaterials( wxCommandEvent& )
{
    DIALOG_BUILD_BOM* dlg = new DIALOG_BUILD_BOM( this );

    dlg->ShowModal();
    dlg->Destroy();
}


void SCH_EDIT_FRAME::OnFindItems( wxCommandEvent& aEvent )
{
    wxCHECK_RET( m_findReplaceData != NULL,
                 wxT( "Forgot to create find/replace data.  Bad Programmer!" ) );

    if( m_dlgFindReplace )
    {
        delete m_dlgFindReplace;
        m_dlgFindReplace = NULL;
    }

    // Verify the find dialog is not drawn off the visible display area in case the
    // display configuration has changed since the last time the dialog position was
    // saved.
    wxRect displayRect = wxDisplay().GetGeometry();
    wxRect dialogRect = wxRect( m_findDialogPosition, m_findDialogSize );

    wxPoint position = m_findDialogPosition;

    if( !displayRect.Contains( dialogRect ) )
    {
        position = wxDefaultPosition;
    }

    int style = 0;

    if( aEvent.GetId() == wxID_REPLACE )
        style = wxFR_REPLACEDIALOG;

    m_dlgFindReplace = new DIALOG_SCH_FIND( this, m_findReplaceData, position, m_findDialogSize,
                                            style );

    m_dlgFindReplace->SetFindEntries( m_findStringHistoryList );
    m_dlgFindReplace->SetReplaceEntries( m_replaceStringHistoryList );
    m_dlgFindReplace->Show( true );
}


void SCH_EDIT_FRAME::OnFindDialogClose( wxFindDialogEvent& event )
{
    // If the user dismissed the dialog with the mouse, this will send the cursor back
    // to the last item found.
    OnFindSchematicItem( event );

    if( m_dlgFindReplace )
    {
        m_findDialogPosition = m_dlgFindReplace->GetPosition();
        m_findDialogSize = m_dlgFindReplace->GetSize();
        m_findStringHistoryList = m_dlgFindReplace->GetFindEntries();
        m_replaceStringHistoryList = m_dlgFindReplace->GetReplaceEntries();
        m_dlgFindReplace->Destroy();
        m_dlgFindReplace = NULL;
    }
}


void SCH_EDIT_FRAME::OnLoadFile( wxCommandEvent& event )
{
    wxString fn;

    fn = GetFileFromHistory( event.GetId(), _( "Schematic" ) );

    if( fn != wxEmptyString )
        LoadOneEEProject( fn, false );
}


void SCH_EDIT_FRAME::OnLoadCmpToFootprintLinkFile( wxCommandEvent& event )
{
    LoadCmpToFootprintLinkFile();
    m_canvas->Refresh();
}


void SCH_EDIT_FRAME::OnNewProject( wxCommandEvent& event )
{
    LoadOneEEProject( wxEmptyString, true );
}


void SCH_EDIT_FRAME::OnLoadProject( wxCommandEvent& event )
{
    LoadOneEEProject( wxEmptyString, false );
}


void SCH_EDIT_FRAME::OnOpenPcbnew( wxCommandEvent& event )
{
    wxFileName fn = g_RootSheet->GetScreen()->GetFileName();

    if( fn.IsOk() )
    {
        fn.SetExt( PcbFileExtension );

        wxString filename = QuoteFullPath( fn );

        ExecuteFile( this, PCBNEW_EXE, filename );
    }
    else
    {
        ExecuteFile( this, PCBNEW_EXE );
    }
}


void SCH_EDIT_FRAME::OnOpenCvpcb( wxCommandEvent& event )
{
    wxFileName fn = g_RootSheet->GetScreen()->GetFileName();

    fn.SetExt( NetlistFileExtension );

    if( fn.IsOk() && fn.FileExists() )
    {
        ExecuteFile( this, CVPCB_EXE, QuoteFullPath( fn ) );
    }
    else
    {
        ExecuteFile( this, CVPCB_EXE );
    }
}

void SCH_EDIT_FRAME::OnOpenLibraryEditor( wxCommandEvent& event )
{
    SCH_COMPONENT* component = NULL;
    if( event.GetId() == ID_POPUP_SCH_CALL_LIBEDIT_AND_LOAD_CMP )
    {
        SCH_ITEM* item = GetScreen()->GetCurItem();

        if( (item == NULL) || (item->GetFlags() != 0) || ( item->Type() != SCH_COMPONENT_T ) )
        {
            wxMessageBox( _("Error: not a component or no component" ) );
            return;
        }

        component = (SCH_COMPONENT*) item;
    }

    LIB_EDIT_FRAME * libeditFrame = LIB_EDIT_FRAME::GetActiveLibraryEditor();;
    if( libeditFrame )
    {
        if( libeditFrame->IsIconized() )
             libeditFrame->Iconize( false );

        libeditFrame->Raise();
    }
    else
    {
        libeditFrame = new LIB_EDIT_FRAME( this,
                                           wxT( "Library Editor" ),
                                           wxPoint( -1, -1 ),
                                           wxSize( 600, 400 ) );
    }

    if( component )
    {
        LIB_ALIAS* entry = CMP_LIBRARY::FindLibraryEntry( component->GetLibName() );

        if( entry == NULL )     // Should not occur
            return;

        CMP_LIBRARY* library = entry->GetLibrary();
        libeditFrame->LoadComponentAndSelectLib( entry, library );
    }
}


void SCH_EDIT_FRAME::OnExit( wxCommandEvent& event )
{
    Close( true );
}


void SCH_EDIT_FRAME::SetLanguage( wxCommandEvent& event )
{
    EDA_BASE_FRAME::SetLanguage( event );

    LIB_EDIT_FRAME * libeditFrame = LIB_EDIT_FRAME::GetActiveLibraryEditor();;
    if( libeditFrame )
        libeditFrame->EDA_BASE_FRAME::SetLanguage( event );
}


void SCH_EDIT_FRAME::OnPrint( wxCommandEvent& event )
{
    wxFileName fn;
    DIALOG_PRINT_USING_PRINTER dlg( this );

    dlg.ShowModal();

    fn = g_RootSheet->GetScreen()->GetFileName();

    wxString default_name = NAMELESS_PROJECT;
    default_name += wxT( ".sch" );

    if( fn.GetFullName() != default_name )
    {
        fn.SetExt( ProjectFileExtension );
        wxGetApp().WriteProjectConfig( fn.GetFullPath(), GROUP, GetProjectFileParameters() );
    }
}

void SCH_EDIT_FRAME::PrintPage( wxDC* aDC, int aPrintMask, bool aPrintMirrorMode, void* aData )
{
    GetScreen()->Draw( m_canvas, aDC, GR_DEFAULT_DRAWMODE );
    TraceWorkSheet( aDC, GetScreen(), GetDefaultLineThickness(), IU_PER_MILS,
                    GetScreen()->GetFileName() );
}


void SCH_EDIT_FRAME::OnSelectItem( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();
    int index = id - ID_SELECT_ITEM_START;

    if( (id >= ID_SELECT_ITEM_START && id <= ID_SELECT_ITEM_END)
        && (index >= 0 && index < m_collectedItems.GetCount()) )
    {
        SCH_ITEM* item = m_collectedItems[index];
        m_canvas->SetAbortRequest( false );
        GetScreen()->SetCurItem( item );
    }
}


bool SCH_EDIT_FRAME::isAutoSaveRequired() const
{
    SCH_SHEET_LIST SheetList;

    return SheetList.IsAutoSaveRequired();
}


void SCH_EDIT_FRAME::addCurrentItemToList( wxDC* aDC )
{
    SCH_SCREEN* screen = GetScreen();
    SCH_ITEM* item = screen->GetCurItem();

    wxCHECK_RET( item != NULL, wxT( "Cannot add current item to list." ) );

    m_canvas->SetAutoPanRequest( false );

    SCH_ITEM* undoItem = item;

    if( item->Type() == SCH_SHEET_PIN_T )
    {
        SCH_SHEET* sheet = (SCH_SHEET*) item->GetParent();

        wxCHECK_RET( (sheet != NULL) && (sheet->Type() == SCH_SHEET_T),
                     wxT( "Cannot place sheet pin in invalid schematic sheet object." ) );

        undoItem = sheet;
    }

    else if( item->Type() == SCH_FIELD_T )
    {
        SCH_COMPONENT* cmp = (SCH_COMPONENT*) item->GetParent();

        wxCHECK_RET( (cmp != NULL) && (cmp->Type() == SCH_COMPONENT_T),
                     wxT( "Cannot place field in invalid schematic component object." ) );

        undoItem = cmp;
    }

    if( item->IsNew() )
    {
        if( item->Type() == SCH_SHEET_T )
        {
            // Fix the size and position of the new sheet using the last values set by
            // the m_mouseCaptureCallback function.
            m_canvas->SetMouseCapture( NULL, NULL );

            if( !EditSheet( (SCH_SHEET*)item, aDC ) )
            {
                screen->SetCurItem( NULL );
                item->Draw( m_canvas, aDC, wxPoint( 0, 0 ), g_XorMode );
                delete item;
                return;
            }

            SetSheetNumberAndCount();
        }

        if( undoItem == item )
        {
            if( !screen->CheckIfOnDrawList( item ) )  // don't want a loop!
                screen->Append( item );

            SetRepeatItem( item );

            SaveCopyInUndoList( undoItem, UR_NEW );
        }
        else
        {
            // Here, item is not a basic schematic item, but an item inside
            // a parent basic schematic item,
            // currently: sheet pin or component field.
            // currently, only a sheet pin can be found as new item,
            // because new component fields have a specific handling, and do not appears here
            SaveCopyInUndoList( undoItem, UR_CHANGED );

            if( item->Type() == SCH_SHEET_PIN_T )
                ( (SCH_SHEET*)undoItem )->AddPin( (SCH_SHEET_PIN*) item );
            else
                wxLogMessage(wxT( "addCurrentItemToList: expected type = SCH_SHEET_PIN_T, actual type = %d" ),
                            item->Type() );
        }
    }
    else
    {
        SaveUndoItemInUndoList( undoItem );
    }

    item->ClearFlags();
    screen->SetModify();
    screen->SetCurItem( NULL );
    m_canvas->SetMouseCapture( NULL, NULL );
    m_canvas->EndMouseCapture();

    if( item->IsConnectable() )
        screen->TestDanglingEnds();

    if( aDC )
    {
        EDA_CROSS_HAIR_MANAGER( m_canvas, aDC );  // Erase schematic cursor
        undoItem->Draw( m_canvas, aDC, wxPoint( 0, 0 ), GR_DEFAULT_DRAWMODE );
    }
}

/* sets the main window title bar text.
 * If file name defined by SCH_SCREEN::m_FileName is not set, the title is set to the
 * application name appended with no file.
 * Otherwise, the title is set to the hierarchical sheet path and the full file name,
 * and read only is appended to the title if the user does not have write
 * access to the file.
 */
void SCH_EDIT_FRAME::UpdateTitle()
{
    wxString title;

    if( GetScreen()->GetFileName() == m_DefaultSchematicFileName )
    {
        title.Printf( wxT( "Eeschema %s [%s]" ), GetChars( GetBuildVersion() ),
                            GetChars( GetScreen()->GetFileName() ) );
    }
    else
    {
        wxFileName fn( GetScreen()->GetFileName() );

        // Often the /path/to/filedir is blank because of the FullFileName argument
        // passed to LoadOneEEFile() which omits the path on non-root schematics.
        // Making the path absolute solves this problem.
        fn.MakeAbsolute();
        title.Printf( wxT( "[ %s %s] (%s)" ),
                      GetChars( fn.GetName() ),
                      GetChars( m_CurrentSheet->PathHumanReadable() ),
                      GetChars( fn.GetPath() ) );

        if( fn.FileExists() )
        {
            if( !fn.IsFileWritable() )
                title << _( " [Read Only]" );
        }
        else
            title << _( " [no file]" );

    }

    SetTitle( title );
}
