/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file wxstruct.h
 * @brief Base window classes and related definitions.
 */

#ifndef  WXSTRUCT_H_
#define  WXSTRUCT_H_


#include <vector>

#include <wx/socket.h>
#include <wx/log.h>
#include <wx/config.h>
#include <wx/wxhtml.h>
#include <wx/laywin.h>
#include <wx/aui/aui.h>
#include <wx/docview.h>

#include <colors.h>
#include <common.h>

#ifdef USE_WX_OVERLAY
#include <wx/overlay.h>
#endif

// C++ guarantees that operator delete checks its argument for null-ness
#ifndef SAFE_DELETE
#define SAFE_DELETE( p ) delete (p); (p) = NULL;
#endif

// Option for dialog boxes
#define DIALOG_STYLE wxDEFAULT_DIALOG_STYLE | wxFRAME_FLOAT_ON_PARENT | MAYBE_RESIZE_BORDER

#define KICAD_DEFAULT_DRAWFRAME_STYLE wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS


// Readability helper definitions for creating backup files.
#define CREATE_BACKUP_FILE    true
#define NO_BACKUP_FILE        false


class EDA_ITEM;
class EDA_RECT;
class EDA_DRAW_PANEL;
class EDA_MSG_PANEL;
class BASE_SCREEN;
class PARAM_CFG_BASE;
class PAGE_INFO;
class PLOTTER;
class TITLE_BLOCK;
class MSG_PANEL_ITEM;


enum id_librarytype {
    LIBRARY_TYPE_EESCHEMA,
    LIBRARY_TYPE_PCBNEW,
    LIBRARY_TYPE_DOC,
    LIBRARY_TYPE_SYMBOL
};

enum ID_DRAWFRAME_TYPE
{
    NOT_INIT_FRAME_TYPE = 0,
    SCHEMATIC_FRAME_TYPE,
    LIBEDITOR_FRAME_TYPE,
    VIEWER_FRAME_TYPE,
    PCB_FRAME_TYPE,
    MODULE_EDITOR_FRAME_TYPE,
    MODULE_VIEWER_FRAME_TYPE,
    FOOTPRINT_WIZARD_FRAME_TYPE,
    CVPCB_FRAME_TYPE,
    CVPCB_DISPLAY_FRAME_TYPE,
    GERBER_FRAME_TYPE,
    TEXT_EDITOR_FRAME_TYPE,
    DISPLAY3D_FRAME_TYPE,
    KICAD_MAIN_FRAME_TYPE
};


/// Custom trace mask to enable and disable auto save tracing.
extern const wxChar* traceAutoSave;


/**
 * Class EDA_BASE_FRAME
 * is the base frame for deriving all KiCad main window classes.  This class is not
 * intended to be used directly.
 */
class EDA_BASE_FRAME : public wxFrame
{
protected:
    ID_DRAWFRAME_TYPE m_Ident;  // Id Type (pcb, schematic, library..)
    wxPoint      m_FramePos;
    wxSize       m_FrameSize;

    wxAuiToolBar* m_mainToolBar; // Standard horizontal Toolbar
    bool         m_FrameIsActive;
    wxString     m_FrameName;    // name used for writing and reading setup
                                 // It is "SchematicFrame", "PcbFrame" ....
    wxString     m_AboutTitle;   // Name of program displayed in About.

    wxAuiManager m_auimgr;

    /// Flag to indicate if this frame supports auto save.
    bool         m_hasAutoSave;

    /// Flag to indicate the last auto save state.
    bool         m_autoSaveState;

    /// The auto save interval time in seconds.
    int          m_autoSaveInterval;

    /// The timer used to implement the auto save feature;
    wxTimer*     m_autoSaveTimer;

    /**
     * Function onAutoSaveTimer
     * handles the auto save timer event.
     */
    void onAutoSaveTimer( wxTimerEvent& aEvent );

    /**
     * Function autoSaveRequired
     * returns the auto save status of the application.  Override this function if
     * your derived frame supports automatic file saving.
     */
    virtual bool isAutoSaveRequired() const { return false; }

    /**
     * Function doAutoSave
     * should be overridden by the derived class to handle the auto save feature.
     *
     * @return true if the auto save was successful otherwise false.
     */
    virtual bool doAutoSave();

public:
    EDA_BASE_FRAME( wxWindow* aParent, ID_DRAWFRAME_TYPE aFrameType,
                    const wxString& aTitle,
                    const wxPoint& aPos, const wxSize& aSize,
                    long aStyle,
                    const wxString & aFrameName );

    ~EDA_BASE_FRAME();

    /**
     * Function ProcessEvent
     * overrides the default process event handler to implement the auto save feature.
     *
     * @warning If you override this function in a derived class, make sure you call
     *          down to this or the auto save feature will be disabled.
     */
    virtual bool ProcessEvent( wxEvent& aEvent );

    void SetAutoSaveInterval( int aInterval ) { m_autoSaveInterval = aInterval; }

    int GetAutoSaveInterval() const { return m_autoSaveInterval; }

    wxString GetName() const { return m_FrameName; }

    bool IsActive() const { return m_FrameIsActive; }

    bool IsType( ID_DRAWFRAME_TYPE aType ) const { return m_Ident == aType; }

    void GetKicadHelp( wxCommandEvent& event );

    void GetKicadAbout( wxCommandEvent& event );

    /**
     * Function CopyVersionInfoToClipboard
     * copies the version information to the clipboard for bug reporting purposes.
     */
    void CopyVersionInfoToClipboard( wxCommandEvent& event );

    void PrintMsg( const wxString& text );

    /**
     * Append the copy version information to clipboard help menu entry to \a aMenu.
     *
     * @param aMenu - The menu to append.
     */
    void AddHelpVersionInfoMenuEntry( wxMenu* aMenu );

    /**
     * Load common frame parameters from configuration.
     *
     * The method is virtual so you can override it to load frame specific
     * parameters.  Don't forget to call the base method or your frames won't
     * remember their positions and sizes.
     */
    virtual void LoadSettings();

    /**
     * Save common frame parameters from configuration.
     *
     * The method is virtual so you can override it to save frame specific
     * parameters.  Don't forget to call the base method or your frames won't
     * remember their positions and sizes.
     */
    virtual void SaveSettings();

    /**
     * Function OnSelectPreferredEditor
     * Open a dialog to select the editor that will used in KiCad
     * to edit or display files (reports ... )
     * The full filename editor is saved in configuration (global params)
     */
    virtual void OnSelectPreferredEditor( wxCommandEvent& event );

    // Read/Save and Import/export hotkeys config

    /**
     * Function ReadHotkeyConfig
     * Read configuration data and fill the current hotkey list with hotkeys
     * @param aDescList = current hotkey list descr. to initialize.
     */
    int ReadHotkeyConfig( struct EDA_HOTKEY_CONFIG* aDescList );

    /**
     * Function WriteHotkeyConfig
     * Store the current hotkey list
     * It is stored using the standard wxConfig mechanism or a file.
     *
     * @param aDescList = pointer to the current hotkey list.
     * @param aFullFileName = a wxString pointer to a full file name.
     *  if NULL, use the standard wxConfig mechanism (default)
     * the output format is: shortcut  "key"  "function"
     * lines starting with # are comments
     */
    int WriteHotkeyConfig( struct EDA_HOTKEY_CONFIG* aDescList, wxString* aFullFileName = NULL);

    /**
     * Function ReadHotkeyConfigFile
     * Read an old configuration file (&ltfile&gt.key) and fill the current hotkey list
     * with hotkeys
     * @param aFilename = file name to read.
     * @param aDescList = current hotkey list descr. to initialize.
     */
    int ReadHotkeyConfigFile( const wxString& aFilename, struct EDA_HOTKEY_CONFIG* aDescList );

    /**
     * Function ImportHotkeyConfigFromFile
     * Prompt the user for an old hotkey file to read, and read it.
     * @param aDescList = current hotkey list descr. to initialize.
     */
    void ImportHotkeyConfigFromFile( struct EDA_HOTKEY_CONFIG* aDescList );

    /**
     * Function ExportHotkeyConfigToFile
     * Prompt the user for an old hotkey file to read, and read it.
     * @param aDescList = current hotkey list descr. to initialize.
     */
    void ExportHotkeyConfigToFile( struct EDA_HOTKEY_CONFIG* aDescList );

    /**
     * Function SetLanguage
     * called on a language menu selection
     * when using a derived function, do not forget to call this one
     */
    virtual void SetLanguage( wxCommandEvent& event );

    /**
     * Function GetFileFromHistory
     * fetches the file name from the file history list.
     * and removes the selected file, if this file does not exists
     * Note also the menu is updated, if wxFileHistory::UseMenu
     * was called at init time
     * @param cmdId The command ID associated with the \a aFileHistory object.
     * @param type Please document me!
     * @param aFileHistory The wxFileHistory in use. If null, the main application file
     *                     history is used
     * @return a wxString containing the selected filename
     */
    wxString GetFileFromHistory( int cmdId, const wxString& type,
                                 wxFileHistory* aFileHistory = NULL );

    /**
     * Function UpdateFileHistory
     * Updates the list of recently opened files.
     * Note also the menu is updated, if wxFileHistory::UseMenu
     * was called at init time
     * @param FullFileName The full file name including the path.
     * @param aFileHistory The wxFileHistory in use.
     * If NULL, the main application file history is used.
     */
    void UpdateFileHistory( const wxString& FullFileName, wxFileHistory * aFileHistory = NULL );

    /**
     * Function ReCreateMenuBar
     * Creates recreates the menu bar.
     * Needed when the language is changed
     */
    virtual void ReCreateMenuBar();

    /**
     * Function IsWritable
     * checks if \a aFileName can be written.
     * <p>
     * The function performs a number of tests on \a aFileName to verify that it
     * can be saved.  If \a aFileName defines a path with no file name, them the
     * path is tested for user write permission.  If \a aFileName defines a file
     * name that does not exist in the path, the path is tested for user write
     * permission.  If \a aFileName defines a file that already exits, the file
     * name is tested for user write permissions.
     * </p>
     *
     * @note The file name path must be set or an assertion will be raised on debug
     *       builds and return false on release builds.
     * @param aFileName The full path and/or file name of the file to test.
     * @return False if \a aFileName cannot be written.
     */
    bool IsWritable( const wxFileName& aFileName );

    /**
     * Function CheckForAutoSaveFile
     * checks if an auto save file exists for \a aFileName and takes the appropriate
     * action depending on the user input.
     * <p>
     * If an auto save file exists for \a aFileName, the user is prompted if they wish
     * to replace file \a aFileName with the auto saved file.  If the user chooses to
     * replace the file, the backup file of \a aFileName is removed, \a aFileName is
     * renamed to the backup file name, and the auto save file is renamed to \a aFileName.
     * If user chooses to keep the existing version of \a aFileName, the auto save file
     * is removed.
     * </p>
     * @param aFileName A wxFileName object containing the file name to check.
     * @param aBackupFileExtension A wxString object containing the backup file extension
     *                             used to create the backup file name.
     */
    void CheckForAutoSaveFile( const wxFileName& aFileName, const wxString& aBackupFileExtension );

    /**
     * Function SetModalMode
     * Disable or enable all other windows, to emulate a dialog behavior
     * Useful when the frame is used to show and selec items
     * (see FOOTPRINT_VIEWER_FRAME and LIB_VIEW_FRAME)
     *
     * @param aModal = true to disable all other opened windows (i.e.
     * this windows is in dialog mode
     *               = false to enable other windows
     * This function is analog to MakeModal( aModal ), deprecated since wxWidgets 2.9.4
     */
    void SetModalMode( bool aModal );
};


/**
 * Class EDA_DRAW_FRAME
 * is the base class for create windows for drawing purpose.  The Eeschema, Pcbnew and
 * GerbView main windows are just a few examples of classes derived from EDA_DRAW_FRAME.
 */
class EDA_DRAW_FRAME : public EDA_BASE_FRAME
{
    /// Let the #EDA_DRAW_PANEL object have access to the protected data since
    /// it is closely tied to the #EDA_DRAW_FRAME.
    friend class EDA_DRAW_PANEL;

    ///< Id of active button on the vertical toolbar.
    int m_toolId;

    BASE_SCREEN* m_currentScreen;           ///< current used SCREEN
    bool         m_snapToGrid;              ///< Indicates if cursor should be snapped to grid.

protected:
    EDA_HOTKEY_CONFIG* m_HotkeysZoomAndGridList;
    int         m_LastGridSizeId;
    bool        m_DrawGrid;                 // hide/Show grid
    EDA_COLOR_T m_GridColor;                // Grid color

    /// The area to draw on.
    EDA_DRAW_PANEL* m_canvas;

    /// Tool ID of previously active draw tool bar button.
    int m_lastDrawToolId;

    /// The shape of the KiCad cursor.  The default value (0) is the normal cross
    /// hair cursor.  Set to non-zero value to draw the full screen cursor.
    /// @note This is not the system mouse cursor.
    int m_cursorShape;

    /// True shows the X and Y axis indicators.
    bool m_showAxis;

    /// True shows the grid axis indicators.
    bool m_showGridAxis;

    /// True shows the origin axis used to indicate the coordinate offset for
    /// drill, gerber, and component position files.
    bool m_showOriginAxis;

    /// True shows the drawing border and title block.
    bool m_showBorderAndTitleBlock;

    /// Choice box to choose the grid size.
    wxComboBox* m_gridSelectBox;

    /// Choice box to choose the zoom value.
    wxComboBox* m_zoomSelectBox;

    /// The tool bar that contains the buttons for quick access to the application draw
    /// tools.  It typically is located on the right side of the main window.
    wxAuiToolBar* m_drawToolBar;

    /// The options tool bar typcially located on the left edge of the main window.
    wxAuiToolBar* m_optionsToolBar;

    /// Panel used to display information at the bottom of the main window.
    EDA_MSG_PANEL* m_messagePanel;

    int            m_MsgFrameHeight;

#ifdef USE_WX_OVERLAY
    // MAC Uses overlay to workaround the wxINVERT and wxXOR miss
    wxOverlay m_overlay;
#endif

protected:

    void SetScreen( BASE_SCREEN* aScreen )  { m_currentScreen = aScreen; }

    /**
     * Function unitsChangeRefresh
     * is called when when the units setting has changed to allow for any derived classes
     * to handle refreshing and controls that have units based measurements in them.  The
     * default version only updates the status bar.  Don't forget to call the default
     * in your derived class or the status bar will not get updated properly.
     */
    virtual void unitsChangeRefresh();

public:
    EDA_DRAW_FRAME( wxWindow* aParent,
                    ID_DRAWFRAME_TYPE aFrameType,
                    const wxString& aTitle,
                    const wxPoint& aPos, const wxSize& aSize,
                    long aStyle,
                    const wxString & aFrameName );

    ~EDA_DRAW_FRAME();

    virtual void SetPageSettings( const PAGE_INFO& aPageSettings ) = 0;
    virtual const PAGE_INFO& GetPageSettings() const = 0;

    /**
     * Function GetPageSizeIU
     * works off of GetPageSettings() to return the size of the paper page in
     * the internal units of this particular view.
     */
    virtual const wxSize GetPageSizeIU() const = 0;

    virtual const wxPoint& GetOriginAxisPosition() const = 0;
    virtual void SetOriginAxisPosition( const wxPoint& aPosition ) = 0;

    virtual const TITLE_BLOCK& GetTitleBlock() const = 0;
    virtual void SetTitleBlock( const TITLE_BLOCK& aTitleBlock ) = 0;

    int GetCursorShape() const { return m_cursorShape; }

    void SetCursorShape( int aCursorShape ) { m_cursorShape = aCursorShape; }

    bool GetShowBorderAndTitleBlock() const { return m_showBorderAndTitleBlock; }

    void SetShowBorderAndTitleBlock( bool aShow ) { m_showBorderAndTitleBlock = aShow; }

    EDA_DRAW_PANEL* GetCanvas() { return m_canvas; }

    virtual wxString GetScreenDesc();

    /**
     * Function GetScreen
     * returns a pointer to a BASE_SCREEN or one of its
     * derivatives.  It is overloaded by derived classes to return
     * SCH_SCREEN or PCB_SCREEN.
     */
    virtual BASE_SCREEN* GetScreen() const  { return m_currentScreen; }

    void OnMenuOpen( wxMenuEvent& event );
    void  OnMouseEvent( wxMouseEvent& event );

    /** function SkipNextLeftButtonReleaseEvent
     * after calling this function, if the left mouse button
     * is down, the next left mouse button release event will be ignored.
     * It is is usefull for instance when closing a dialog on a mouse click,
     * to skip the next mouse left button release event
     * by the parent window, because the mouse button
     * clicked on the dialog is often released in the parent frame,
     * and therefore creates a left button released mouse event
     * which can be unwanted in some cases
     */
    void SkipNextLeftButtonReleaseEvent();

    virtual void OnHotKey( wxDC* aDC, int aHotKey, const wxPoint& aPosition,
                           EDA_ITEM* aItem = NULL );

    /**
     * Function AddMenuZoomAndGrid (virtual)
     * Add standard zoom commands and submenu zoom and grid selection to a popup menu
     * uses zoom hotkeys info base to add hotkeys info to menu commands
     * @param aMasterMenu = the menu to populate.
     */
    virtual void AddMenuZoomAndGrid( wxMenu* aMasterMenu );

    void EraseMsgBox();
    void Process_PageSettings( wxCommandEvent& event );

    /**
     * Function SetLanguage
     * called on a language menu selection
     * when using a derived function, do not forget to call this one
     */
    virtual void SetLanguage( wxCommandEvent& event );

    virtual void ReCreateHToolbar() = 0;
    virtual void ReCreateVToolbar() = 0;
    virtual void ReCreateMenuBar();
    virtual void ReCreateAuxiliaryToolbar();

    /**
     * Function SetToolID
     * sets the tool command ID to \a aId and sets the cursor to \a aCursor.  The
     * command ID must be greater or equal ::ID_NO_TOOL_SELECTED.  If the command
     * ID is less than ::ID_NO_TOOL_SELECTED, the tool command ID is set to
     * ::ID_NO_TOOL_SELECTED.  On debug builds, an assertion will be raised when
     * \a aId is invalid.
     * @param aId New tool command ID if greater than or equal to ::ID_NO_TOOL_SELECTED.
                  If less than zero, the current tool command ID is retained.
     * @param aCursor Sets the cursor shape if greater than or equal to zero.
     * @param aToolMsg The tool message to set in the status bar.
     */
    virtual void SetToolID( int aId, int aCursor, const wxString& aToolMsg );

    int GetToolId() const { return m_toolId; }

    /* These 4 functions provide a basic way to show/hide grid
     * and /get/set grid color.
     * These parameters are saved in KiCad config for each main frame
     */
    /**
     * Function IsGridVisible() , virtual
     * @return true if the grid must be shown
     */
    virtual bool IsGridVisible() const
    {
        return m_DrawGrid;
    }

    /**
     * Function SetGridVisibility() , virtual
     * It may be overloaded by derived classes
     * @param aVisible = true if the grid must be shown
     */
    virtual void SetGridVisibility( bool aVisible )
    {
        m_DrawGrid = aVisible;
    }

    /**
     * Function GetGridColor() , virtual
     * @return the color of the grid
     */
    virtual EDA_COLOR_T GetGridColor() const
    {
        return m_GridColor;
    }

    /**
     * Function SetGridColor() , virtual
     * @param aColor = the new color of the grid
     */
    virtual void SetGridColor( EDA_COLOR_T aColor )
    {
        m_GridColor = aColor;
    }

    /**
     * Function GetGridPosition
     * returns the nearest grid position to \a aPosition if a screen is defined and snap to
     * grid is enabled.  Otherwise, the original positions is returned.
     * @see m_snapToGrid and m_BaseScreen members.
     * @param aPosition The position to test.
     * @return The wxPoint of the appropriate cursor position.
     */
    wxPoint GetGridPosition( const wxPoint& aPosition ) const;

    /**
     * Command event handler for selecting grid sizes.
     *
     * All commands that set the grid size should eventually end up here.
     * This is where the application setting is saved.  If you override
     * this method, make sure you call down to the base class.
     *
     * @param event - Command event passed by selecting grid size from the
     *                grid size combobox on the toolbar.
     */
    virtual void OnSelectGrid( wxCommandEvent& event );

    /**
     * Functions OnSelectZoom
     * sets the zoom factor when selected by the zoom list box in the main tool bar.
     * @note List position 0 is fit to page
     *       List position >= 1 = zoom (1 to zoom max)
     *       Last list position is custom zoom not in zoom list.
     */
    virtual void OnSelectZoom( wxCommandEvent& event );

    // Command event handlers shared by all applications derived from EDA_DRAW_FRAME.
    void OnToggleGridState( wxCommandEvent& aEvent );
    void OnSelectUnits( wxCommandEvent& aEvent );
    void OnToggleCrossHairStyle( wxCommandEvent& aEvent );

    // Update user interface event handlers shared by all applications derived from
    // EDA_DRAW_FRAME.
    void OnUpdateUndo( wxUpdateUIEvent& aEvent );
    void OnUpdateRedo( wxUpdateUIEvent& aEvent );
    void OnUpdateGrid( wxUpdateUIEvent& aEvent );
    void OnUpdateUnits( wxUpdateUIEvent& aEvent );
    void OnUpdateCrossHairStyle( wxUpdateUIEvent& aEvent );

    /**
     * Function GeneralControl
     * performs application specific control using \a aDC at \a aPosition in logical units.
     * <p>
     * Override this function for application specific control.  This function gets
     * called on every mouse and key event.
     *</p>
     * @param aDC A device context.
     * @param aPosition The current cursor position in logical (drawing) units.
     * @param aHotKey A key event used for application specific control if not zero.
     */
    virtual void GeneralControl( wxDC* aDC, const wxPoint& aPosition, int aHotKey = 0 ) { }

    /**
     * Function OnSize
     * recalculates the size of toolbars and display panel when the frame size changes.
     */
    virtual void OnSize( wxSizeEvent& event );

    void OnEraseBackground( wxEraseEvent& SizeEvent );

    virtual void OnZoom( wxCommandEvent& event );
    void OnGrid( int grid_type );

    /**
     * Function RedrawScreen
     * redraws the entire screen area by updating the scroll bars and mouse pointer in
     * order to have \a aCenterPoint at the center of the screen.
     * @param aCenterPoint The position in logical units to center the scroll bars.
     * @param aWarpPointer Moves the mouse cursor to \a aCenterPoint if true.
     */
    void RedrawScreen( const wxPoint& aCenterPoint, bool aWarpPointer );

    /**
     * Function RedrawScreen2
     * puts the crosshair back to the screen position it had before zooming
     * @param posBefore screen position of the crosshair before zooming
     */
    void RedrawScreen2( const wxPoint& posBefore );

    /**
     * Function Zoom_Automatique
     * redraws the screen with best zoom level and the best centering
     * that shows all the page or the board
     */
    void Zoom_Automatique( bool aWarpPointer );

    /* Set the zoom level to show the area Rect */
    void Window_Zoom( EDA_RECT& Rect );

    /** Return the zoom level which displays the full page on screen */
    virtual double BestZoom() = 0;

    /**
     * Function GetZoom
     * @return The current zoom level.
     */
    double GetZoom();

    void TraceWorkSheet( wxDC* aDC, BASE_SCREEN* aScreen, int aLineWidth,
                         double aScale, const wxString &aFilename );

    /**
     * Function TraceWorkSheet is a core function for drawing of the page layout with
     * the frame and the basic inscriptions.
     * @param aDC The device context.
     * @param aSz The size of the page layout.
     * @param aLT The left top margin of the page layout.
     * @param aRB The right bottom margin of the page layout.
     * @param aType The paper size type (for basic inscriptions).
     * @param aFlNm The file name (for basic inscriptions).
     * @param aTb The block of titles (for basic inscriptions).
     * @param aNScr The number of screens (for basic inscriptions).
     * @param aScr The screen number (for basic inscriptions).
     * @param aLnW The line width for drawing.
     * @param aScalar Scalar to convert from mils to internal units.
     * @param aClr1 The color for drawing.
     * @param aClr2 The colr for inscriptions.
     */
    void TraceWorkSheet( wxDC* aDC, wxSize& aSz, wxPoint& aLT, wxPoint& aRB,
                         wxString& aType, wxString& aFlNm, TITLE_BLOCK& aTb,
                         int aNScr, int aScr, int aLnW, double aScalar,
                         EDA_COLOR_T aClr1 = RED, EDA_COLOR_T aClr2 = RED );

    /**
     * Function GetXYSheetReferences
     * returns the X,Y sheet references where the point position is located
     * @param aPosition = position to identify by YX ref
     * @return a wxString containing the message locator like A3 or B6
     *         (or ?? if out of page limits)
     */
    const wxString GetXYSheetReferences( const wxPoint& aPosition ) const;

    void DisplayToolMsg( const wxString& msg );
    virtual void RedrawActiveWindow( wxDC* DC, bool EraseBg ) = 0;
    virtual void OnLeftClick( wxDC* DC, const wxPoint& MousePos ) = 0;
    virtual void OnLeftDClick( wxDC* DC, const wxPoint& MousePos );
    virtual bool OnRightClick( const wxPoint& MousePos, wxMenu* PopMenu ) = 0;
    virtual void ToolOnRightClick( wxCommandEvent& event );
    void AdjustScrollBars( const wxPoint& aCenterPosition );

    /**
     * Function OnActivate (virtual)
     * is called when activating the frame.
     * In derived classes with a overriding OnActivate function,
     * do not forget to call this EDA_DRAW_FRAME::OnActivate( event ) basic function.
     */
    virtual void OnActivate( wxActivateEvent& event );

    /**
     * Function UpdateStatusBar
     * updates the status bar information.
     *
     * The base method updates the absolute and relative coordinates and the
     * zoom information.  If you override this virtual method, make sure to call
     * this subclassed method.  The status bar can draw itself.  This is not
     * a drawing function per se, but rather updates lines of text held by
     * the components within the status bar which is owned by the wxFrame.
     * <p>
     * On a MAC, be careful about calling this function when there is an
     * existing wxDC in existence on a sibling window.
     */
    virtual void UpdateStatusBar();

    /**
     * Function DisplayUnitsMsg
     * displays current unit pane on the status bar.
     */
    void DisplayUnitsMsg();

    /* Handlers for block commands */
    virtual void InitBlockPasteInfos();

    /**
     * Function HandleBlockBegin
     * initializes the block command including the command type, initial position,
     * and other variables.
     */
    virtual bool HandleBlockBegin( wxDC* aDC, int aKey, const wxPoint& aPosition );

    /**
     * Function ReturnBlockCommand
     * Returns the block command code (BLOCK_MOVE, BLOCK_COPY...) corresponding to the
     * keys pressed (ALT, SHIFT, SHIFT ALT ..) when block command is started by dragging
     * the mouse.
     *
     * @param aKey = the key modifiers (Alt, Shift ...)
     * @return the block command id (BLOCK_MOVE, BLOCK_COPY...)
     */
    virtual int ReturnBlockCommand( int aKey );

    /**
     * Function HandleBlockPlace( )
     * Called after HandleBlockEnd, when a block command needs to be
     * executed after the block is moved to its new place
     * (bloc move, drag, copy .. )
     * Parameters must be initialized in GetScreen()->m_BlockLocate
     */
    virtual void HandleBlockPlace( wxDC* DC );

    /**
     * Function HandleBlockEnd( )
     * Handle the "end"  of a block command,
     * i.e. is called at the end of the definition of the area of a block.
     * depending on the current block command, this command is executed
     * or parameters are initialized to prepare a call to HandleBlockPlace
     * in GetScreen()->m_BlockLocate
     * @return false if no item selected, or command finished,
     * true if some items found and HandleBlockPlace must be called later
     */
    virtual bool HandleBlockEnd( wxDC* DC );

    /**
     * Function CopyToClipboard
     * copies the current page or the current block to the clipboard.
     */
    void CopyToClipboard( wxCommandEvent& event );

    /* interprocess communication */
    void OnSockRequest( wxSocketEvent& evt );
    void OnSockRequestServer( wxSocketEvent& evt );

    /**
     * Function LoadSettings
     * loads the draw frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings common to the draw frame will not get loaded.
     */
    virtual void LoadSettings();

    /**
     * Funxtion SaveSettings
     * saves the draw frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings common to the draw frame will not get saved.
     */
    virtual void SaveSettings();

    /**
     * Append a message to the message panel.
     *
     * This helper method checks to make sure the message panel exists in
     * the frame and appends a message to it using the message panel
     * AppendMessage() method.
     *
     * @param textUpper - The message upper text.
     * @param textLower - The message lower text.
     * @param color - A color ID from the KiCad color list (see colors.h).
     * @param pad - Number of spaces to pad between messages (default = 4).
     */
    void AppendMsgPanel( const wxString& textUpper, const wxString& textLower,
                         EDA_COLOR_T color, int pad = 6 );

    /**
     * Clear all messages from the message panel.
     */
    void ClearMsgPanel( void );

    /**
     * Function SetMsgPanel
     * clears the message panel and populates it with the contents of \a aList.
     *
     * @param aList is the list of #MSG_PANEL_ITEM objects to fill the message panel.
     */
    void SetMsgPanel( const std::vector< MSG_PANEL_ITEM >& aList );

    void SetMsgPanel( EDA_ITEM* aItem );

    /**
     * Function PrintPage
     * used to print a page
     * Print the page pointed by current screen, set by the calling print function
     * @param aDC = wxDC given by the calling print function
     * @param aPrintMask = not used here
     * @param aPrintMirrorMode = not used here (Set when printing in mirror mode)
     * @param aData = a pointer on an auxiliary data (not always used, NULL if not used)
     */
    virtual void PrintPage( wxDC* aDC, int aPrintMask, bool aPrintMirrorMode, void* aData = NULL );

    /**
     * Function CoordinateToString
     * is a helper to convert the \a integer coordinate \a aValue to a string in inches or mm
     * according to the current user units setting.
     * @param aValue The coordinate to convert.
     * @param aConvertToMils Convert inch values to mils if true.  This setting has no effect if
     *                       the current user unit is millimeters.
     * @return The converted string for display in user interface elements.
     */
    wxString CoordinateToString( int aValue, bool aConvertToMils = false ) const;

    /**
     * Function LengthDoubleToString
     * is a helper to convert the \a double value \a aValue to a string in inches or mm
     * according to the current user units setting.
     * @param aValue The coordinate to convert.
     * @param aConvertToMils Convert inch values to mils if true.  This setting has no effect if
     *                       the current user unit is millimeters.
     * @return The converted string for display in user interface elements.
     */
    wxString LengthDoubleToString( double aValue, bool aConvertToMils = false ) const;

    DECLARE_EVENT_TABLE()
};


/**
 * Specialization of the wxAuiPaneInfo class for KiCad panels.
 *
 * Documentation for wxAui is poor at this time. The following notes spring from errors made in
 * previous KiCad implementations:
 *
 * wxAuiPaneInfo.ToolbarPane() and .Defaults() are used to clear and then prepare the objects so
 * only use them once at the beginning of configuration..
 *
 * Panels are organized in layers, from 0 (close to the center) and increasing outward. Note
 * that for ToolbarPanes, layer 0 considered a special default value, and ToolbarPanes on
 * layer 0 are pushed to layer 10 automatically. Use Layer 1 for the inner layer as a work-
 * around.
 *
 * Each panel has rows, starting at 0. Each row has positions starting at 0. Each item in a panel
 * can have it's row and position set.
 *
 * Eventually panels will be movable. Each initialization function sets up the panel for this,
 * then after a //==// break has additional calls to anchor toolbars in a way that matches
 * present functionality.
 */

class EDA_PANEINFO : public wxAuiPaneInfo
{

public:

    /**
     * Function HorizontalToolbarPane
     * Change *this to a horizontal toolbar for KiCad.
     */
    EDA_PANEINFO& HorizontalToolbarPane()
    {
        ToolbarPane();
        CloseButton( false );
        LeftDockable( false );
        RightDockable( false );
        //====================  Remove calls below here for movable toolbars //
        Gripper( false );
        DockFixed( true );
        Movable( false );
        Resizable( true );
        return *this;
    }

    /**
     * Function VerticalToolbarPane
     * Change *this to a vertical toolbar for KiCad.
     */
    EDA_PANEINFO& VerticalToolbarPane()
    {
        ToolbarPane();
        CloseButton( false );
        TopDockable( false );
        BottomDockable( false );
        //====================  Remove calls below here for movable toolbars //
        Gripper( false );
        DockFixed( true );
        Movable( false );
        Resizable( true );
        return *this;
    }

    /**
     * Function MessageToolbarPane
     * Change *this to a message pane for KiCad.
     *
     */
    EDA_PANEINFO& MessageToolbarPane()
    {
        Gripper( false );
        DockFixed( true );
        Movable( false );
        Floatable( false );
        CloseButton( false );
        CaptionVisible( false );
        return *this;
    }

    /**
     * Function LayersToolbarPane
     * Change *this to a layers toolbar for KiCad.
     */
    EDA_PANEINFO& LayersToolbarPane()
    {
        CloseButton( false );
        return *this;
    }

    /**
     * Function InfoToolbarPane
     * Change *this to a information panel for for KiCad.
     *
     * Info panes are used for vertical display of information next to the center pane.
     * Used in CvPcb and the library viewer primarily.
     */
    EDA_PANEINFO& InfoToolbarPane()
    {
        Gripper( false );
        CloseButton( false );
        CaptionVisible( false );
        return *this;
    }

    /**
     * Function ScriptingToolbarPane
     * Change *this to a scripting toolbar for KiCad.
     */
    EDA_PANEINFO& ScriptingToolbarPane()
    {
        CloseButton( false );
        Floatable( true );
        Resizable( true );
        return *this;
    }


};

#endif  // WXSTRUCT_H_
