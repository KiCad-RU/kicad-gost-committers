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
 * @file wxPcbStruct.h
 */

#ifndef  WXPCB_STRUCT_H
#define  WXPCB_STRUCT_H


#include "wxBasePcbFrame.h"
#include "param_config.h"
#include "class_layer_box_selector.h"
#include "class_macros_record.h"
#include "class_undoredo_container.h"


#ifndef PCB_INTERNAL_UNIT
#define PCB_INTERNAL_UNIT 10000
#endif


/*  Forward declarations of classes. */
class PCB_SCREEN;
class BOARD;
class TEXTE_PCB;
class MODULE;
class TRACK;
class SEGZONE;
class SEGVIA;
class D_PAD;
class TEXTE_MODULE;
class PCB_TARGET;
class DIMENSION;
class EDGE_MODULE;
class DRC;
class ZONE_CONTAINER;
class DRAWSEGMENT;
class GENERAL_COLLECTOR;
class GENERAL_COLLECTORS_GUIDE;
class PCB_LAYER_WIDGET;
class MARKER_PCB;
class BOARD_ITEM;


/**
 * class PCB_EDIT_FRAME
 * the main frame for Pcbnew
 *
 * See also class PCB_BASE_FRAME(): Basic class for Pcbnew and GerbView.
 */
class PCB_EDIT_FRAME : public PCB_BASE_FRAME
{
    friend class PCB_LAYER_WIDGET;

    void updateTraceWidthSelectBox();
    void updateViaSizeSelectBox();

    int             m_RecordingMacros;
    MACROS_RECORDED m_Macros[10];

protected:

    PCB_LAYER_WIDGET* m_Layers;

    DRC* m_drc;                              ///< the DRC controller, see drc.cpp

    PARAM_CFG_ARRAY   m_projectFileParams;   ///< List of Pcbnew project file settings.
    PARAM_CFG_ARRAY   m_configSettings;      ///< List of Pcbnew configuration settings.

    wxString          m_lastNetListRead;     ///< Last net list read with relative path.

    // we'll use lower case function names for private member functions.
    void createPopUpMenuForZones( ZONE_CONTAINER* edge_zone, wxMenu* aPopMenu );
    void createPopUpMenuForFootprints( MODULE* aModule, wxMenu* aPopMenu );
    void createPopUpMenuForFpTexts( TEXTE_MODULE* aText, wxMenu* aPopMenu );
    void createPopUpMenuForFpPads( D_PAD* aPad, wxMenu* aPopMenu );
    void createPopupMenuForTracks( TRACK* aTrack, wxMenu* aPopMenu );
    void createPopUpMenuForTexts( TEXTE_PCB* Text, wxMenu* menu );
    void createPopUpBlockMenu( wxMenu* menu );
    void createPopUpMenuForMarkers( MARKER_PCB* aMarker, wxMenu* aPopMenu );

    /**
     * Function setActiveLayer
     * will change the currently active layer to \a aLayer and also
     * update the PCB_LAYER_WIDGET.
     */
    void setActiveLayer( int aLayer, bool doLayerWidgetUpdate = true )
    {
        ( (PCB_SCREEN*) GetScreen() )->m_Active_Layer = aLayer;

        if( doLayerWidgetUpdate )
            syncLayerWidget();
    }

    /**
     * Function getActiveLayer
     * returns the active layer
     */
    int getActiveLayer()
    {
        return ( (PCB_SCREEN*) GetScreen() )->m_Active_Layer;
    }

    /**
     * Function syncLayerWidget
     * updates the currently "selected" layer within the PCB_LAYER_WIDGET.
     * The currently active layer is defined by the return value of getActiveLayer().
     * <p>
     * This function cannot be inline without including layer_widget.h in
     * here and we do not want to do that.
     * </p>
     */
    void syncLayerWidget();

    virtual void unitsChangeRefresh();

    /**
     * Function doAutoSave
     * performs auto save when the board has been modified and not saved within the
     * auto save interval.
     *
     * @return true if the auto save was successful.
     */
    virtual bool doAutoSave();

    /**
     * Function isautoSaveRequired
     * returns true if the board has been modified.
     */
    virtual bool isAutoSaveRequired() const;

public:
    LAYER_BOX_SELECTOR* m_SelLayerBox;  // a combo box to display and select active layer
    wxComboBox* m_SelTrackWidthBox;     // a combo box to display and select current track width
    wxComboBox* m_SelViaSizeBox;        // a combo box to display and select current via diameter

    bool m_show_microwave_tools;
    bool m_show_layer_manager_tools;


public:
    PCB_EDIT_FRAME( wxWindow* father, const wxString& title,
                    const wxPoint& pos, const wxSize& size,
                    long style = KICAD_DEFAULT_DRAWFRAME_STYLE );

    ~PCB_EDIT_FRAME();

    void OnQuit( wxCommandEvent& event );

    /**
     * Function ToPlotter
     * Open a dialog frame to create plot and drill files
     * relative to the current board
     */
    void ToPlotter( wxCommandEvent& event );

    /**
     * Function ToPrinter
     * Install the print dialog
     */
    void ToPrinter( wxCommandEvent& event );

    /**
     * Function SVG_Print
     * shows the print SVG file dialog.
     */
    void SVG_Print( wxCommandEvent& event );

    // User interface update command event handlers.
    void OnUpdateSave( wxUpdateUIEvent& aEvent );
    void OnUpdateLayerPair( wxUpdateUIEvent& aEvent );
    void OnUpdateLayerSelectBox( wxUpdateUIEvent& aEvent );
    void OnUpdateDrcEnable( wxUpdateUIEvent& aEvent );
    void OnUpdateShowBoardRatsnest( wxUpdateUIEvent& aEvent );
    void OnUpdateShowModuleRatsnest( wxUpdateUIEvent& aEvent );
    void OnUpdateAutoDeleteTrack( wxUpdateUIEvent& aEvent );
    void OnUpdateViaDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateTraceDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateHighContrastDisplayMode( wxUpdateUIEvent& aEvent );
    void OnUpdateShowLayerManager( wxUpdateUIEvent& aEvent );
    void OnUpdateShowMicrowaveToolbar( wxUpdateUIEvent& aEvent );
    void OnUpdateVerticalToolbar( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectViaSize( wxUpdateUIEvent& aEvent );
    void OnUpdateZoneDisplayStyle( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectTrackWidth( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectAutoTrackWidth( wxUpdateUIEvent& aEvent );

    /**
     * Function RecordMacros.
     * records sequence of hotkeys and cursor positions to a macro.
     * @param aDC = current device context
     * @param aNumber The current number macros.
     */
    void RecordMacros( wxDC* aDC, int aNumber );

    /**
     * Function CallMacros
     * play hotkeys and cursor position from a recorded macro.
     * @param aDC = current device context
     * @param aPosition The current cursor position in logical (drawing) units.
     * @param aNumber The current number macros.
     */
    void CallMacros( wxDC* aDC, const wxPoint& aPosition, int aNumber );

    void SaveMacros();

    void ReadMacros();

    /**
     * Function PrintPage , virtual
     * used to print a page
     * Print the page pointed by the current screen, set by the calling print function
     * @param aDC = wxDC given by the calling print function
     * @param aPrintMaskLayer = a 32 bits mask: bit n = 1 -> layer n is printed
     * @param aPrintMirrorMode = true to plot mirrored
     * @param aData = a pointer on an auxiliary data (NULL if not used)
     */
    virtual void PrintPage( wxDC* aDC, int aPrintMaskLayer, bool aPrintMirrorMode,
                            void * aData = NULL );

    void GetKicadAbout( wxCommandEvent& event );

    /**
     * Function IsGridVisible() , virtual
     * @return true if the grid must be shown
     */
    virtual bool IsGridVisible();

    /**
     * Function SetGridVisibility() , virtual
     * It may be overloaded by derived classes
     * if you want to store/retrieve the grid visibility in configuration.
     * @param aVisible = true if the grid must be shown
     */
    virtual void SetGridVisibility( bool aVisible );

    /**
     * Function GetGridColor() , virtual
     * @return the color of the grid
     */
    virtual int GetGridColor();

    /**
     * Function SetGridColor() , virtual
     * @param aColor = the new color of the grid
     */
    virtual void SetGridColor(int aColor);

    // Configurations:
    void InstallConfigFrame();
    void Process_Config( wxCommandEvent& event );

    /**
     * Function GetProjectFileParameters
     * returns the project file parameter list for Pcbnew.
     * <p>
     * Populate the project file parameter array specific to Pcbnew if it hasn't
     * already been populated and return a reference to the array to the caller.
     * Creating the parameter list at run time has the advantage of being able
     * to define local variables.  The old method of statically building the array
     * at compile time requiring global variable definitions by design.
     * </p>
     */
    PARAM_CFG_ARRAY& GetProjectFileParameters();

    void SaveProjectSettings();

    /**
     * Load the project file configuration settings.
     *
     * @param aProjectFileName = The project filename.
     *  if not found use kicad.pro and initialize default values
     * @return always returns true.
     */
    bool LoadProjectSettings( const wxString& aProjectFileName );

    /**
     * Function GetConfigurationSettings
     * returns the Pcbnew applications settings list.
     *
     * This replaces the old statically define list that had the project
     * file settings and the application settings mixed together.  This
     * was confusing and caused some settings to get saved and loaded
     * incorrectly.  Currently, only the settings that are needed at start
     * up by the main window are defined here.  There are other locally used
     * settings are scattered throughout the Pcbnew source code.  If you need
     * to define a configuration setting that need to be loaded at run time,
     * this is the place to define it.
     *
     * @todo: Define the configuration variables as member variables instead of
     *        global variables or move them to the object class where they are
     *        used.
     * @return - Reference to the list of applications settings.
     */
    PARAM_CFG_ARRAY& GetConfigurationSettings();

    /**
     * Function LoadSettings
     * loads applications settings specific to Pcbnew.
     *
     * This overrides the base class PCB_BASE_FRAME::LoadSettings() to
     * handle settings specific common to the PCB layout application.  It
     * calls down to the base class to load settings common to all PCB type
     * drawing frames.  Please put your application settings for Pcbnew here
     * to avoid having application settings loaded all over the place.
     */
    virtual void LoadSettings();

    /**
     * Function SaveSettings
     * saves applications settings common to Pcbnew.
     *
     * This overrides the base class PCB_BASE_FRAME::SaveSettings() to
     * save settings specific to the PCB layout application main window.  It
     * calls down to the base class to save settings common to all PCB type
     * drawing frames.  Please put your application settings for Pcbnew here
     * to avoid having application settings saved all over the place.
     */
    virtual void SaveSettings();

    /**
     * Get the last net list read with the net list dialog box.
     *
     * @return - Absolute path and file name of the last net list file successfully read.
     */
    wxString GetLastNetListRead();

    /**
     * Set the last net list successfully read by the net list dialog box.
     *
     * Note: the file path is converted to a path relative to the project file path.  If
     *       the path cannot be made relative, than m_lastNetListRead is set to and empty
     *       string.  This could happen when the net list file is on a different drive than
     *       the project file.  The advantage of relative paths is that is more likely to
     *       work when opening the same project from both Windows and Linux.
     *
     * @param aNetListFile - The last net list file with full path successfully read.
     */
    void SetLastNetListRead( const wxString& aNetListFile );

    /**
     * Function Test_Duplicate_Missing_And_Extra_Footprints
     * Build a list of duplicate, missing and extra footprints
     * from the current board and a netlist netlist :
     * Shows 3 lists:
     *  1 - duplicate footprints on board
     *  2 - missing footprints (found in netlist but not on board)
     *  3 - footprints not in netlist but on board
     * @param aNetlistFullFilename = the full filename netlist
     */
    void Test_Duplicate_Missing_And_Extra_Footprints( const wxString& aNetlistFullFilename );

    /**
     * Function OnHotKey.
     *  ** Commands are case insensitive **
     *  Some commands are relatives to the item under the mouse cursor
     * @param aDC = current device context
     * @param aHotkeyCode = hotkey code (ascii or wxWidget code for special keys)
     * @param aPosition The cursor position in logical (drawing) units.
     * @param aItem = NULL or pointer on a EDA_ITEM under the mouse cursor
     */
    void OnHotKey( wxDC* aDC, int aHotkeyCode, const wxPoint& aPosition, EDA_ITEM* aItem = NULL );

    /**
     * Function OnHotkeyDeleteItem
     * Delete the item found under the mouse cursor
     *  Depending on the current active tool::
     *      Tool track
     *          if a track is in progress: Delete the last segment
     *          else delete the entire track
     *      Tool module (footprint):
     *          Delete the module.
     * @param aDC = current device context
     * @return true if an item was deleted
     */
    bool OnHotkeyDeleteItem( wxDC* aDC );

    /**
     * Function OnHotkeyPlaceItem
     * Place the item (footprint, track, text .. ) found under the mouse cursor
     * An item can be placed only if there is this item currently edited
     * Only a footprint, a pad or a track can be placed
     * @param aDC = current device context
     * @return true if an item was placed
     */
    bool OnHotkeyPlaceItem( wxDC* aDC );

    bool OnHotkeyEditItem( int aIdCommand );

    /**
     * Function OnHotkeyMoveItem
     * Moves or drag the item (footprint, track, text .. ) found under the mouse cursor
     * Only a footprint or a track can be dragged
     * @param aIdCommand = the hotkey command id
     * @return true if an item was moved
     */
    bool OnHotkeyMoveItem( int aIdCommand );

    /**
     * Function OnHotkeyRotateItem
     * Rotate the item (text or footprint) found under the mouse cursor
     * @note This command can be used with an item currently in edit.
     *       Only some items can be rotated (footprints and texts).
     * @param aIdCommand = the hotkey command id
     * @return true if an item was moved
     */
    bool OnHotkeyRotateItem( int aIdCommand );

    void OnCloseWindow( wxCloseEvent& Event );
    void Process_Special_Functions( wxCommandEvent& event );
    void Tracks_and_Vias_Size_Event( wxCommandEvent& event );
    void OnSelectTool( wxCommandEvent& aEvent );

    void ProcessMuWaveFunctions( wxCommandEvent& event );
    void MuWaveCommand( wxDC* DC, const wxPoint& MousePos );

    void RedrawActiveWindow( wxDC* DC, bool EraseBg );
    void ReCreateHToolbar();
    void ReCreateAuxiliaryToolbar();
    void ReCreateVToolbar();
    void ReCreateMicrowaveVToolbar();
    void ReCreateOptToolbar();
    void ReCreateMenuBar();
    LAYER_BOX_SELECTOR* ReCreateLayerBox( EDA_TOOLBAR* parent );

    /**
     * Function OnModify
     * must be called after a board change to set the modified flag.
     * <p>
     * Reloads the 3D view if required and calls the base PCB_BASE_FRAME::OnModify function
     * to update auxiliary information.
     * </p>
     */
    virtual void OnModify();

    /**
     * Function IsElementVisible
     * tests whether a given element category is visible. Keep this as an
     * inline function.
     * @param aElement is from the enum by the same name
     * @return bool - true if the element is visible.
     * @see enum PCB_VISIBLE
     */
    bool IsElementVisible( int aElement );

    /**
     * Function SetElementVisibility
     * changes the visibility of an element category
     * @param aElement is from the enum by the same name
     * @param aNewState = The new visibility state of the element category
     * @see enum PCB_VISIBLE
     */
    void SetElementVisibility( int aElement, bool aNewState );

    /**
     * Function SetVisibleAlls
     * Set the status of all visible element categories and layers to VISIBLE
     */
    void SetVisibleAlls();

    /**
     * Function ReFillLayerWidget
     * changes out all the layers in m_Layers and may be called upon
     * loading a new BOARD.
     */
    void ReFillLayerWidget();

    /**
     * Function Show3D_Frame
     * displays the 3D view of current printed circuit board.
     */
    void Show3D_Frame( wxCommandEvent& event );
    void GeneralControl( wxDC* aDC, const wxPoint& aPosition, int aHotKey = 0 );

    /**
     * Function ShowDesignRulesEditor
     * displays the Design Rules Editor.
     */
    void ShowDesignRulesEditor( wxCommandEvent& event );

    /* toolbars update UI functions: */

    void PrepareLayerIndicator();

    /* mouse functions events: */
    void OnLeftClick( wxDC* DC, const wxPoint& MousePos );
    void OnLeftDClick( wxDC* DC, const wxPoint& MousePos );

    /**
     * Function OnRightClick
     * populates a popup menu with the choices appropriate for the current context.
     * The caller will add the ZOOM menu choices afterward.
     * @param aMousePos The current mouse position
     * @param aPopMenu The menu to add to.
     */
    bool OnRightClick( const wxPoint& aMousePos, wxMenu* aPopMenu );

    void OnSelectOptionToolbar( wxCommandEvent& event );
    void ToolOnRightClick( wxCommandEvent& event );

    /**
     * Function SaveCopyInUndoList.
     * Creates a new entry in undo list of commands.
     * add a picker to handle aItemToCopy
     * @param aItemToCopy = the board item modified by the command to undo
     * @param aTypeCommand = command type (see enum UNDO_REDO_T)
     * @param aTransformPoint = the reference point of the transformation, for
     *                          commands like move
     */
    virtual void SaveCopyInUndoList( BOARD_ITEM* aItemToCopy,
                                     UNDO_REDO_T aTypeCommand,
                                     const wxPoint& aTransformPoint = wxPoint( 0, 0 ) );

    /**
     * Function SaveCopyInUndoList (overloaded).
     * Creates a new entry in undo list of commands.
     * add a list of pickers to handle a list of items
     * @param aItemsList = the list of items modified by the command to undo
     * @param aTypeCommand = command type (see enum UNDO_REDO_T)
     * @param aTransformPoint = the reference point of the transformation, for
     *                          commands like move
     */
    virtual void SaveCopyInUndoList( PICKED_ITEMS_LIST& aItemsList,
                                     UNDO_REDO_T aTypeCommand,
                                     const wxPoint& aTransformPoint = wxPoint( 0, 0 ) );

    /**
     * Function PutDataInPreviousState
     * Used in undo or redo command.
     * Put data pointed by List in the previous state, i.e. the state memorized by List
     * @param aList = a PICKED_ITEMS_LIST pointer to the list of items to undo/redo
     * @param aRedoCommand = a bool: true for redo, false for undo
     * @param aRebuildRatsnet = a bool: true to rebuild ratsnest (normal use), false
     * to just retrieve last state (used in abort commands that do not need to
     * rebuild ratsnest)
     */
    void PutDataInPreviousState( PICKED_ITEMS_LIST* aList,
                                 bool               aRedoCommand,
                                 bool               aRebuildRatsnet = true );

    /**
     * Function GetBoardFromRedoList
     *  Redo the last edition:
     *  - Save the current board in Undo list
     *  - Get an old version of the board from Redo list
     *  @return none
     */
    void GetBoardFromRedoList( wxCommandEvent& event );

    /**
     * Function GetBoardFromUndoList
     *  Undo the last edition:
     *  - Save the current board in Redo list
     *  - Get an old version of the board from Undo list
     *  @return none
     */
    void GetBoardFromUndoList( wxCommandEvent& event );

    /* Block operations: */

    /**
     * Function ReturnBlockCommand
     * Returns the block command internat code (BLOCK_MOVE, BLOCK_COPY...)
     * corresponding to the keys pressed (ALT, SHIFT, SHIFT ALT ..) when
     * block command is started by dragging the mouse.
     * @param aKey = the key modifiers (Alt, Shift ...)
     * @return the block command id (BLOCK_MOVE, BLOCK_COPY...)
     */
    virtual int ReturnBlockCommand( int aKey );

    /**
     * Function HandleBlockPlace()
     * Called after HandleBlockEnd, when a block command needs to be
     * executed after the block is moved to its new place
     * (bloc move, drag, copy .. )
     * Parameters must be initialized in GetScreen()->m_BlockLocate
     */
    virtual void HandleBlockPlace( wxDC* DC );

    /**
     * Function HandleBlockEnd()
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
     * Function Block_SelectItems
     * Uses  GetScreen()->m_BlockLocate
     * select items within the selected block.
     * selected items are put in the pick list
     */
    void Block_SelectItems();

    /**
     * Function Block_Delete
     * deletes all items within the selected block.
     */
    void Block_Delete();

    /**
     * Function Block_Rotate
     * Rotate all items within the selected block.
     * The rotation center is the center of the block
     */
    void Block_Rotate();

    /**
     * Function Block_Flip
     * Flip items within the selected block.
     * The flip center is the center of the block
     */
    void Block_Flip();

    /**
     * Function Block_Move
     * move all items within the selected block.
     * New location is determined by the current offset from the selected
     * block's original location.
     */
    void Block_Move();

    /**
     * Function Block_Mirror_X
     * mirrors all items within the currently selected block in the X axis.
     */
    void Block_Mirror_X();

    /**
     * Function Block_Duplicate
     * Duplicate all items within the selected block.
     * New location is determined by the current offset from the selected
     * block's original location.
     */
    void Block_Duplicate();

    void Process_Settings( wxCommandEvent& event );
    void OnConfigurePcbOptions( wxCommandEvent& aEvent );
    void InstallDisplayOptionsDialog( wxCommandEvent& aEvent );
    void InstallPcbGlobalDeleteFrame( const wxPoint& pos );
    bool InstallDialogNonCopperZonesEditor( ZONE_CONTAINER* aZone );
    void InstallDialogLayerSetup();

    void GenModulesPosition( wxCommandEvent& event );
    void GenModuleReport( wxCommandEvent& event );
    void InstallDrillFrame( wxCommandEvent& event );
    void ToPostProcess( wxCommandEvent& event );

    void OnFileHistory( wxCommandEvent& event );

    /**
     * Function Files_io
     * is the command event handler for read and write file commands.
     */
    void Files_io( wxCommandEvent& event );

    /**
     * Function LoadOnePcbFile
     * loads a KiCad board (.brd) from \a aFileName.
     *
     *  @param aFileName - File name including path. If empty, a file dialog will
     *                     be displayed.
     *  @param aAppend - Append board file aFileName to the currently loaded file if true.
     *                   Default = false.
     *  @param aForceFileDialog - Display the file open dialog even if aFullFileName is
     *                            valid if true; Default = false.
     *
     *  @return False if file load fails or is canceled by the user, otherwise true.
     */
    bool LoadOnePcbFile( const wxString& aFileName, bool aAppend = false,
                         bool aForceFileDialog = false );

    /**
     * Function ReadPcbFile
     * reads a board file  &ltfile&gt.brd
     * @param aReader The line reader object to read from.
     * @param Append if 0: a previously loaded board is deleted before loading
     *               the file else all items of the board file are added to the
     *               existing board
     */
    int ReadPcbFile( LINE_READER* aReader, bool Append );

    /**
     * Function SavePcbFile
     * writes the board data structures to \a a aFileName
     *
     * @param aFileName The file name to write or wxEmptyString to prompt user for
     *                  file name.
     * @param aCreateBackupFile Creates a back of \a aFileName if true.  Helper
     *                          definitions #CREATE_BACKUP_FILE and #NO_BACKUP_FILE
     *                          are defined for improved code readability.
     * @return True if file was saved successfully.
     */
    bool SavePcbFile( const wxString& aFileName, bool aCreateBackupFile = CREATE_BACKUP_FILE );

    int SavePcbFormatAscii( FILE* File );
    bool WriteGeneralDescrPcb( FILE* File );

    // BOARD handling

    /**
     * Function Clear_Pcb
     * delete all and reinitialize the current board
     * @param aQuery = true to prompt user for confirmation, false to initialize silently
     */
    bool Clear_Pcb( bool aQuery );

    // Drc control

    /* function GetDrcController
     * @return the DRC controller
     */
    DRC* GetDrcController() { return m_drc; }

    /**
     * Function RecreateBOMFileFromBoard
     * Recreates a .cmp file from the current loaded board
     * this is the same as created by CvPcb.
     * can be used if this file is lost
     */
    void RecreateCmpFileFromBoard( wxCommandEvent& aEvent );

    /**
     * Function RecreateBOMFileFromBoard
     * Creates a BOM file from the current loaded board
     */
    void RecreateBOMFileFromBoard( wxCommandEvent& aEvent );

    /**
     * Function ExportToGenCAD
     * creates a file in  GenCAD 1.4 format from the current board.
     */
    void ExportToGenCAD( wxCommandEvent& event );

    /**
     * Function OnExportVRML
     * will export the current BOARD to a VRML file.
     */
    void OnExportVRML( wxCommandEvent& event );

    /**
     * Function ExportVRML_File
     * Creates the file(s) exporting current BOARD to a VRML file.
     * @param aFullFileName = the full filename of the file to create
     * @param aScale = the general scaling factor. 1.0 to export in inches
     * @param aExport3DFiles = true to copy 3D shapes in the subir a3D_Subdir
     * @param a3D_Subdir = sub directory where 3D shapes files are copied
     * used only when aExport3DFiles == true
     * @return true if Ok.
     */
    bool ExportVRML_File( const wxString & aFullFileName, double aScale,
                          bool aExport3DFiles, const wxString & a3D_Subdir );

    /**
     * Function ExporttoSPECCTRA
     * will export the current BOARD to a specctra dsn file.  See
     * See http://www.autotraxeda.com/docs/SPECCTRA/SPECCTRA.pdf for the
     * specification.
     */
    void ExportToSpecctra( wxCommandEvent& event );

    /**
     * Function ImportSpecctraSession
     * will import a specctra *.ses file and use it to relocate MODULEs and
     * to replace all vias and tracks in an existing and loaded BOARD.
     * See http://www.autotraxeda.com/docs/SPECCTRA/SPECCTRA.pdf for the
     * specification.
     */
    void ImportSpecctraSession( wxCommandEvent& event );

    /**
     * Function ImportSpecctraDesign
     * will import a specctra *.dsn file and use it to replace an entire BOARD.
     * The new board will not have any graphics, only components, tracks and
     * vias.
     * See http://www.autotraxeda.com/docs/SPECCTRA/SPECCTRA.pdf for the
     * specification.
     */
    void ImportSpecctraDesign( wxCommandEvent& event );

    /**
     * Function Access_to_External_Tool
     * Run an external tool (like freeroute )
     */
    void Access_to_External_Tool( wxCommandEvent& event );

    MODULE* ListAndSelectModuleName();

    /**
     * Function ListNetsAndSelect
     * called by a command event
     * displays the sorted list of nets in a dialog frame
     * If a net is selected, it is highlighted
     */
    void ListNetsAndSelect( wxCommandEvent& event );

    void Swap_Layers( wxCommandEvent& event );

    // Handling texts on the board
    void Rotate_Texte_Pcb( TEXTE_PCB* TextePcb, wxDC* DC );
    TEXTE_PCB* Create_Texte_Pcb( wxDC* DC );
    void Delete_Texte_Pcb( TEXTE_PCB* TextePcb, wxDC* DC );
    void StartMoveTextePcb( TEXTE_PCB* TextePcb, wxDC* DC );
    void Place_Texte_Pcb( TEXTE_PCB* TextePcb, wxDC* DC );
    void InstallTextPCBOptionsFrame( TEXTE_PCB* TextPCB, wxDC* DC );

    // Graphic Segments type DRAWSEGMENT
    void Start_Move_DrawItem( DRAWSEGMENT* drawitem, wxDC* DC );
    void Place_DrawItem( DRAWSEGMENT* drawitem, wxDC* DC );
    void InstallGraphicItemPropertiesDialog( DRAWSEGMENT* aItem, wxDC* aDC );

    // Footprint edition (see also PCB_BASE_FRAME)
    void InstallModuleOptionsFrame( MODULE* Module, wxDC* DC );
    void StartMove_Module( MODULE* module, wxDC* DC );

    /**
     * Function Delete Module
     * Remove a footprint from m_Modules linked list and put it in undelete buffer
     * The ratsnest and pad list are recalculated
     * @param aModule = footprint to delete
     * @param aDC = currentDevice Context. if NULL: do not redraw new ratsnest
     * @param aAskBeforeDeleting : if true: ask for confirmation before deleting
     */
    bool Delete_Module( MODULE* aModule, wxDC* aDC, bool aAskBeforeDeleting );

    /**
     * Function Change_Side_Module
     * Flip a footprint (switch layer from component or component to copper)
     * The mirroring is made from X axis
     * if a footprint is not on copper or component layer it is not flipped
     * (it could be on an adhesive layer, not supported at this time)
     * @param Module the footprint to flip
     * @param  DC Current Device Context. if NULL, no redraw
     */
    void Change_Side_Module( MODULE* Module, wxDC* DC );

    void InstallExchangeModuleFrame( MODULE* ExchangeModuleModule );

    /**
     * Function Exchange_Module
     * Replaces OldModule by NewModule, using OldModule settings:
     * position, orientation, pad netnames ...)
     * OldModule is deleted or put in undo list.
     * @param aOldModule = footprint to replace
     * @param aNewModule = footprint to put
     * @param aUndoPickList = the undo list used to save  OldModule. If null,
     *                        OldModule is deleted
     */
    void Exchange_Module( MODULE* aOldModule, MODULE* aNewModule,
                          PICKED_ITEMS_LIST* aUndoPickList );

    // loading modules: see PCB_BASE_FRAME

    // Board handling
    void RemoveStruct( BOARD_ITEM* Item, wxDC* DC );

    /**
     * Function OnEditItemRequest
     * Install the corresponding dialog editor for the given item
     * @param aDC = the current device context
     * @param aItem = a pointer to the BOARD_ITEM to edit
     */
    void OnEditItemRequest( wxDC* aDC, BOARD_ITEM* aItem );

    /**
     * Locate track or pad and highlight the corresponding net.
     * @return The Netcode, or -1 if no net located.
     */
    int SelectHighLight( wxDC* DC );

    /**
     * Function HighLight.
     * highlights the net at the current cursor position.
     */
    void HighLight( wxDC* DC );

    // Track and via edition:
    void Via_Edit_Control( wxCommandEvent& event );

    /**
     * Function IsMicroViaAcceptable
     * return true if a microvia can be placed on the board.
     * <p>
     * A microvia is a small via restricted to 2 near neighbor layers
     * because its is hole is made by laser which can penetrate only one layer
     * It is mainly used to connect BGA to the first inner layer
     * And it is allowed from an external layer to the first inner layer
     * </p>
     */
    bool IsMicroViaAcceptable( void );

    /**
     * Function Other_Layer_Route
     * operates in one of two ways.  If argument track is NULL, then swap the
     * active layer between m_Route_Layer_TOP and m_Route_Layer_BOTTOM.  If a
     * track is in progress (track is not NULL), and if DRC allows it, place
     * a via on the end of the current track, and then swap the current active
     * layer and start a new segment on the new layer.
     * @param track A TRACK* to append the via to or NULL.
     * @param DC A device context to draw on.
     * @return bool - true if the operation was successful, else false such as
     *                the case where DRC would not allow a via.
     */
    bool Other_Layer_Route( TRACK* track, wxDC* DC );
    void HighlightUnconnectedPads( wxDC* DC );

    /**
     * Function DisplayNetStatus
     * shows the status of the net at the current mouse position or the
     * PCB status if no segment selected.
     */
    void DisplayNetStatus( wxDC* DC );

    /**
     * Function Delete_Segment
     * removes 1 segment of track.
     * 2 cases:
     * If There is evidence of new track: delete new segment.
     * Otherwise, delete segment under the cursor.
     */
    TRACK* Delete_Segment( wxDC* DC, TRACK* Track );

    void Delete_Track( wxDC* DC, TRACK* Track );
    void Delete_net( wxDC* DC, TRACK* Track );

    /**
     * Function Remove_One_Track
     * removes 1 track/
     * The leading segment is removed and all adjacent segments
     * until a pad or a junction point of more than 2 segments is found
     */
    void Remove_One_Track( wxDC* DC, TRACK* pt_segm );

    /**
     * Function Reset_All_Tracks_And_Vias_To_Netclass_Values
     * Reset all tracks width and/or vias diameters and drill
     * to their default Netclass value
     * @param aTrack : bool true to modify tracks
     * @param aVia : bool true to modify vias
     */
    bool Reset_All_Tracks_And_Vias_To_Netclass_Values( bool aTrack, bool aVia );

    /**
     * Function Change_Net_Tracks_And_Vias_Sizes
     * Reset all tracks width and vias diameters and drill
     * to their default Netclass value or current values
     * @param aNetcode : the netcode of the net to edit
     * @param aUseNetclassValue : bool. True to use netclass values, false to
     *                            use current values
     */
    bool Change_Net_Tracks_And_Vias_Sizes( int  aNetcode, bool aUseNetclassValue );

    /**
     * Function Edit_Track_Width
     * Modify a full track width (using DRC control).
     * a full track is the set of track segments between 2 ends: pads or a
     * point that has more than 2 segments ends connected
     * @param  aDC = the curred device context (can be NULL)
     * @param aTrackSegment = a segment or via on the track to change
     */
    void Edit_Track_Width( wxDC* aDC, TRACK* aTrackSegment );

    /**
     * Function Edit_TrackSegm_Width
     *  Modify one track segment width or one via diameter (using DRC control).
     * @param  aDC = the current device context (can be NULL)
     * @param aTrackItem = the track segment or via to modify
     */
    void Edit_TrackSegm_Width( wxDC* aDC, TRACK* aTrackItem );

    /**
     * Function Begin_Route
     * Starts a new track and/or establish of a new track point.
     *
     * For a new track:
     * - Search the netname of the new track from the starting point
     * if it is on a pad or an existing track
     * - Highlight all this net
     * If a track is in progress:
     * - Call DRC
     * - If DRC is OK: finish the track segment and starts a new one.
     * @param aTrack = the current track segment, or NULL to start a new track
     * @param aDC = the current device context
     * @return a pointer to the new track segment or null if not created (DRC error)
     */
    TRACK* Begin_Route( TRACK* aTrack, wxDC* aDC );

    /**
     * Function End_Route
     * Terminates a track currently being created
     * @param aTrack = the current track segment in progress
     * @param aDC = the current device context
     * @return true if the track was created, false if not (due to a DRC error)
     */
    bool End_Route( TRACK* aTrack, wxDC* aDC );

    void Attribut_Segment( TRACK* track, wxDC* DC, bool Flag_On );
    void Attribut_Track( TRACK* track, wxDC* DC, bool Flag_On );
    void Attribut_net( wxDC* DC, int net_code, bool Flag_On );

    /**
     * Function StartMoveOneNodeOrSegment
     * initializes the parameters to move one  via or/and a terminal point of a track segment
     * The terminal point of other connected segments (if any) are moved too.
     */
    void StartMoveOneNodeOrSegment( TRACK* aTrack, wxDC* aDC, int aCommand );

    bool PlaceDraggedOrMovedTrackSegment( TRACK* Track, wxDC* DC );

    /**
     * @todo This function is broken, because it merge segments having different
     *       widths or without any connectivity test.
     * 2 collinear segments can be merged only if no other segment or via is
     * connected to the common point and if they have the same width. See
     * cleanup.cpp for merge functions and consider MarkTrace() to locate segments
     * that can be merged
     */
    bool MergeCollinearTracks( TRACK* track, wxDC* DC, int end );

    void Start_DragTrackSegmentAndKeepSlope( TRACK* track, wxDC* DC );
    void SwitchLayer( wxDC* DC, int layer );

    /**
     * Function Add45DegreeSegment
     * adds a track segment between 2 tracks segments if these 2 segments
     * make a 90 deg angle, in order to have 45 deg track segments
     * Its only works on horizontal or vertical segments.
     *
     * @param aDC The wxDC device context to draw on.
     * @return A bool value true if ok or false if not.
     */
    bool Add45DegreeSegment( wxDC* aDC );

    /**
     * Function EraseRedundantTrack
     * Called after creating a track
     * Remove (if exists) the old track that have the same starting and the
     * same ending point as the new created track
     * (this is the redunding track)
     * @param aDC = the current device context (can be NULL)
     * @param aNewTrack = the new created track (a pointer to a segment of the
     *                    track list)
     * @param aNewTrackSegmentsCount = number of segments in this new track
     * @param aItemsListPicker = the list picker to use for an undo command
     *                           (can be NULL)
     */
    int EraseRedundantTrack( wxDC*              aDC,
                             TRACK*             aNewTrack,
                             int                aNewTrackSegmentsCount,
                             PICKED_ITEMS_LIST* aItemsListPicker );

    /**
     * Function SetTrackSegmentWidth
     *  Modify one track segment width or one via diameter (using DRC control).
     *  Basic routine used by other routines when editing tracks or vias
     * @param aTrackItem = the track segment or via to modify
     * @param aItemsListPicker = the list picker to use for an undo command
     *                           (can be NULL)
     * @param aUseNetclassValue = true to use NetClass value, false to use
     *                            current designSettings value
     * @return  true if done, false if no not change (because DRC error)
     */
    bool SetTrackSegmentWidth( TRACK*             aTrackItem,
                               PICKED_ITEMS_LIST* aItemsListPicker,
                               bool               aUseNetclassValue );


    // zone handling

    /**
     * Function Delete_OldZone_Fill (obsolete)
     * Used for compatibility with old boards
     * Remove the zone filling which include the segment aZone, or the zone
     * which have the given time stamp.
     * For old boards, a zone is a group of SEGZONE segments which have the same TimeStamp
     * @param aZone = zone segment within the zone to delete. Can be NULL
     * @param aTimestamp = Timestamp for the zone to delete, used if aZone ==
     *                     NULL
     */
    void Delete_OldZone_Fill( SEGZONE* aZone, long aTimestamp = 0 );

    /**
     * Function Delete_LastCreatedCorner
     * Used only while creating a new zone outline
     * Remove and delete the current outline segment in progress
     * @return 0 if no corner in list, or corner number
     */
    int Delete_LastCreatedCorner( wxDC* DC );

    /**
     * Function Begin_Zone
     * either initializes the first segment of a new zone, or adds an
     * intermediate segment.
     * A new zone can be:
     * created from scratch: the user will be prompted to define parameters (layer, clearence ...)
     * created from a similar zone (s_CurrentZone is used): parameters are copied from
     * s_CurrentZone
     * created as a cutout (an hole) inside s_CurrentZone
     */
    int Begin_Zone( wxDC* DC );

    /**
     * Function End_Zone
     * terminates (if no DRC error ) the zone edge creation process
     * @param DC = current Device Context
     * @return true if Ok, false if DRC error
     */
    bool End_Zone( wxDC* DC );

    /**
     * Function Fill_Zone
     *  Calculate the zone filling for the outline zone_container
     *  The zone outline is a frontier, and can be complex (with holes)
     *  The filling starts from starting points like pads, tracks.
     * If exists the old filling is removed
     * @param aZone = zone to fill
     * @return error level (0 = no error)
     */
    int Fill_Zone( ZONE_CONTAINER* aZone );

    /**
     * Function Fill_All_Zones
     *  Fill all zones on the board
     * The old fillings are removed
     * @param aActiveWindow = the current active window, if a progress bar is shown
     *                      = NULL to do not display a progress bar
     * @param aVerbose = true to show error messages
     */
    int Fill_All_Zones( wxWindow * aActiveWindow, bool aVerbose = true );


    /**
     * Function Add_Zone_Cutout
     * Add a cutout zone to a given zone outline
     * @param DC = current Device Context
     * @param zone_container = parent zone outline
     */
    void Add_Zone_Cutout( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Add_Similar_Zone
     * Add a zone to a given zone outline.
     * if the zones are overlapping they will be merged
     * @param DC = current Device Context
     * @param zone_container = parent zone outline
     */
    void Add_Similar_Zone( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Edit_Zone_Params
     * Edit params (layer, clearance, ...) for a zone outline
     */
    void Edit_Zone_Params( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Start_Move_Zone_Corner
     * Prepares a move corner in a zone outline,
     * called from a move corner command (IsNewCorner = false),
     * or a create new cornet command (IsNewCorner = true )
     */
    void Start_Move_Zone_Corner( wxDC*           DC,
                                 ZONE_CONTAINER* zone_container,
                                 int             corner_id,
                                 bool            IsNewCorner );

    /**
     * Function Start_Move_Zone_Corner
     * Prepares a drag edge in an existing zone outline,
     */
    void Start_Move_Zone_Drag_Outline_Edge( wxDC*            DC,
                                            ZONE_CONTAINER* zone_container,
                                            int             corner_id );

    /**
     * Function End_Move_Zone_Corner_Or_Outlines
     * Terminates a move corner in a zone outline, or a move zone outlines
     * @param DC = current Device Context (can be NULL)
     * @param zone_container: the given zone
     */
    void End_Move_Zone_Corner_Or_Outlines( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function End_Move_Zone_Corner_Or_Outlines
     * Remove the currently selected corner in a zone outline
     * the .m_CornerSelection is used as corner selection
     */
    void Remove_Zone_Corner( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Delete_Zone
     * Remove the zone which include the segment aZone, or the zone which have
     * the given time stamp.  A zone is a group of segments which have the
     * same TimeStamp
     * @param DC = current Device Context (can be NULL)
     * @param zone_container = zone to modify
     *  the member .m_CornerSelection is used to find the outline to remove.
     * if the outline is the main outline, all the zone is removed
     * otherwise, the hole is deleted
     */
    void Delete_Zone_Contour( wxDC* DC, ZONE_CONTAINER* zone_container );

    /**
     * Function Start_Move_Zone_Outlines
     * Initialize parameters to move an existing zone outlines.
     * @param DC = current Device Context (can be NULL)
     * @param zone_container: the given zone to move
     */
    void Start_Move_Zone_Outlines( wxDC* DC, ZONE_CONTAINER* zone_container );

    // Target handling
    PCB_TARGET* CreateTarget( wxDC* DC );
    void DeleteTarget( PCB_TARGET* aTarget, wxDC* DC );
    void BeginMoveTarget( PCB_TARGET* aTarget, wxDC* DC );
    void PlaceTarget( PCB_TARGET* aTarget, wxDC* DC );
    void ShowTargetOptionsDialog( PCB_TARGET* aTarget, wxDC* DC );

    // Graphic segments type DRAWSEGMENT handling:
    DRAWSEGMENT* Begin_DrawSegment( DRAWSEGMENT* Segment, int shape, wxDC* DC );
    void End_Edge( DRAWSEGMENT* Segment, wxDC* DC );
    void Delete_Segment_Edge( DRAWSEGMENT* Segment, wxDC* DC );
    void Delete_Drawings_All_Layer( int aLayer );

    // Dimension handling:
    void ShowDimensionPropertyDialog( DIMENSION* aDimension, wxDC* aDC );
    DIMENSION* EditDimension( DIMENSION* aDimension, wxDC* aDC );
    void DeleteDimension( DIMENSION* aDimension, wxDC* aDC );
    void BeginMoveDimensionText( DIMENSION* aItem, wxDC* DC );
    void PlaceDimensionText( DIMENSION* aItem, wxDC* DC );


    // netlist  handling:
    void InstallNetlistFrame( wxDC* DC );

    /**
     * Function ReadPcbNetlist
     * Update footprints (load missing footprints and delete on demand extra
     * footprints)
     * Update connectivity info, references, values and "TIME STAMP"
     * @param aNetlistFullFilename = netlist file name (*.net)
     * @param aCmpFullFileName = cmp/footprint link file name (*.cmp).
      *                         if not found, only the netlist will be used
     * @param aMessageWindow = a reference to a wxTextCtrl where to display messages.
     *                  can be NULL
     * @param aChangeFootprint if true, footprints that have changed in netlist will be changed
     * @param aDeleteBadTracks if true, erroneous tracks will be deleted
     * @param aDeleteExtraFootprints if true, remove unlocked footprints that are not in netlist
     * @param aSelect_By_Timestamp if true, use timestamp instead of reference to identify
     *                             footprints from components (use after reannotation of the
     *                             schematic)
     * @return true if Ok
     */
    bool ReadPcbNetlist( const wxString&  aNetlistFullFilename,
                         const wxString&  aCmpFullFileName,
                         wxTextCtrl*      aMessageWindow,
                         bool             aChangeFootprint,
                         bool             aDeleteBadTracks,
                         bool             aDeleteExtraFootprints,
                         bool             aSelect_By_Timestamp );

    /**
     * Function RemoveMisConnectedTracks
     * finds all track segments which are mis-connected (to more than one net).
     * When such a bad segment is found, mark it as needing to be removed.
     * and remove all tracks having at least one flagged segment.
     * @param aDC = the current device context (can be NULL)
     * @return true if any change is made
     */
    bool RemoveMisConnectedTracks( wxDC* aDC );


    // Autoplacement:
    void AutoPlace( wxCommandEvent& event );

    /**
     * Function OnOrientFootprints
     * install the dialog box for the common Orient Footprints
     */
    void OnOrientFootprints( wxCommandEvent& event );

    /**
     * Function ReOrientModules
     * Set the orientation of footprints
     * @param ModuleMask = mask (wildcard allowed) selection
     * @param Orient = new orientation
     * @param include_fixe = true to orient locked footprints
     * @return true if some footprints modified, false if no change
     */
    bool ReOrientModules( const wxString& ModuleMask, int Orient,
                                  bool include_fixe );
    void LockModule( MODULE* aModule, bool aLocked );
    void AutoMoveModulesOnPcb( bool PlaceModulesHorsPcb );

    /**
     * Function AutoPlaceModule
     * automatically places footprints within the confines of the PCB edges.
     * The components with the FIXED status are not moved.  If the menu is
     * calling the placement of 1 module, it will be replaced.
     */
    void AutoPlaceModule( MODULE* Module, int place_mode, wxDC* DC );

    /**
     * Function GetOptimalModulePlacement
     * searches for the optimal position of the \a aModule.
     *
     * @param aModule A pointer to the MODULE object to get the optimal placement.
     * @param aDC The device context to draw on.
     * @return 1 if placement impossible or 0 if OK.
     */
    int GetOptimalModulePlacement( MODULE* aModule, wxDC* aDC );

    void GenModuleOnBoard( MODULE* Module );

    /**
     * Function Compute_Ratsnest_PlaceModule
     * displays the module's ratsnest during displacement, and assess the "cost"
     * of the position.
     *
     * The cost is the longest ratsnest distance with penalty for connections
     * approaching 45 degrees.
     */
    float Compute_Ratsnest_PlaceModule( wxDC* DC );

    /**
     * Function GenPlaceBoard
     * generates board board (component side copper + rating):
     * Allocate the memory needed to represent in "bitmap" on the grid
     * Current:
     * - The size of clearance area of component (the board)
     * - The bitmap PENALTIES
     * And initialize the cells of the board has
     * - Hole in the cells occupied by a segment EDGE
     * - CELL_is_ZONE for cell internal contour EDGE (if closed)
     *
     * Placement surface (board) gives the cells internal to the contour
     * PCB, and among the latter the free cells and cells already occupied
     *
     * The bitmap PENALTIES give cells occupied by the modules,
     * Plus a surface penalty related to the number of pads of the module
     *
     * Bitmap of the penalty is set to 0
     * Occupation cell is a 0 leaves
     */
    int GenPlaceBoard();

    void DrawInfoPlace( wxDC* DC );

    // Autorouting:
    int Solve( wxDC* DC, int two_sides );
    void Reset_Noroutable( wxDC* DC );
    void Autoroute( wxDC* DC, int mode );
    void ReadAutoroutedTracks( wxDC* DC );
    void GlobalRoute( wxDC* DC );

    /**
     * Function Show_1_Ratsnest
     * draw ratsnest.
     *
     * The net edge pad with mouse or module locates the mouse.
     * Delete the ratsnest if no module or pad is selected.
     */
    void Show_1_Ratsnest( EDA_ITEM* item, wxDC* DC );

    void Clean_Pcb( wxDC* DC );

    void InstallFindFrame( const wxPoint& pos, wxDC* DC );

    /**
     * Function SendMessageToEESCHEMA
     * sends a message to the schematic editor so that it may move its cursor
     * to a part with the same reference as the objectToSync
     * @param objectToSync The object whose reference is used to synchronize Eeschema.
     */
    void SendMessageToEESCHEMA( BOARD_ITEM* objectToSync );

    /**
     * Function Edit_Gap
     * edits the GAP module if it has changed the position and/or size of the pads that
     * form the gap get a new value.
     */
    void Edit_Gap( wxDC* DC, MODULE* Module );

    /**
     * Function Create_MuWaveBasicShape
     * create a footprint with pad_count pads for micro wave applications.
     * This footprint has pad_count pads:
     *  PAD_SMD, rectangular, H size = V size = current track width.
     */
    MODULE* Create_MuWaveBasicShape( const wxString& name, int pad_count );

    /**
     * Create_MuWaveComponent
     * creates a module "GAP" or "STUB" used in micro wave designs.
     *  This module has 2 pads:
     *  PAD_SMD, rectangular, H size = V size = current track width.
     *  the "gap" is isolation created between this 2 pads
     */
    MODULE* Create_MuWaveComponent( int shape_type );

    MODULE* Create_MuWavePolygonShape();

    void Begin_Self( wxDC* DC );

    /**
     * Function Genre_Self
     * creates a self-shaped coil for microwave applications.
     * - Length Mself.lng
     * - Extremities Mself.m_Start and Mself.m_End
     *
     * We must determine:
     * Mself.nbrin = number of segments perpendicular to the direction
     * (The coil nbrin will demicercles + 1 + 2 1 / 4 circle)
     * Mself.lbrin = length of a strand
     * Mself.radius = radius of rounded parts of the coil
     * Mself.delta = segments extremities connection between him and the coil even
     *
     * The equations are
     * Mself.m_Size.x = 2 * Mself.radius + Mself.lbrin
     * Mself.m_Size.y * Mself.delta = 2 + 2 * Mself.nbrin * Mself.radius
     * Mself.lng = 2 * Mself.delta / / connections to the coil
     + (Mself.nbrin-2) * Mself.lbrin / / length of the strands except 1st and last
     + (Mself.nbrin 1) * (PI * Mself.radius) / / length of rounded
     * Mself.lbrin + / 2 - Melf.radius * 2) / / length of 1st and last bit
     *
     * The constraints are:
     * Nbrin >= 2
     * Mself.radius < Mself.m_Size.x
     * Mself.m_Size.y = Mself.radius * 4 + 2 * Mself.raccord
     * Mself.lbrin> Mself.radius * 2
     *
     * The calculation is conducted in the following way:
     * Initially:
     * Nbrin = 2
     * Radius = 4 * m_Size.x (arbitrarily fixed value)
     * Then:
     * Increasing the number of segments to the desired length
     * (Radius decreases if necessary)
     *
     */
    MODULE* Genere_Self( wxDC* DC );

    /**
     * Function SetLanguage
     * called on a language menu selection
     */
    virtual void SetLanguage( wxCommandEvent& event );

    /**
     * Function UpdateTitle
     * sets the main window title bar text.
     * <p>
     * If file name defined by PCB_SCREEN::m_FileName is not set, the title is set to the
     * application name appended with no file.  Otherwise, the title is set to the full path
     * and file name and read only is appended to the title if the user does not have write
     * access to the file.
     * </p>
     */
    void UpdateTitle();

    DECLARE_EVENT_TABLE()
};


#endif  /* WXPCB_STRUCT_H */
