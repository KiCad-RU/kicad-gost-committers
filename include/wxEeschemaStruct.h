/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file wxEeschemaStruct.h
 */

#ifndef  WX_EESCHEMA_STRUCT_H
#define  WX_EESCHEMA_STRUCT_H

#include "wxstruct.h"
#include "param_config.h"
#include "class_undoredo_container.h"
#include "template_fieldnames.h"
#include "block_commande.h"
#include "class_sch_screen.h"
#include "sch_collectors.h"


class LIB_EDIT_FRAME;
class LIB_VIEW_FRAME;
class DRAWSEGMENT;
class SCH_ITEM;
class SCH_NO_CONNECT;
class CMP_LIBRARY;
class LIB_COMPONENT;
class LIB_DRAW_ITEM;
class EDA_ITEM;
class SCH_BUS_ENTRY;
class SCH_GLOBALLABEL;
class SCH_TEXT;
class SCH_BITMAP;
class SCH_SHEET;
class SCH_SHEET_PATH;
class SCH_SHEET_PIN;
class SCH_COMPONENT;
class SCH_FIELD;
class LIB_PIN;
class SCH_JUNCTION;
class DIALOG_SCH_FIND;
class wxFindDialogEvent;
class wxFindReplaceData;


/* enum used in RotationMiroir() */
enum COMPONENT_ORIENTATION_T {
    CMP_NORMAL,                     // Normal orientation, no rotation or mirror
    CMP_ROTATE_CLOCKWISE,           // Rotate -90
    CMP_ROTATE_COUNTERCLOCKWISE,    // Rotate +90
    CMP_ORIENT_0,                   // No rotation and no mirror id CMP_NORMAL
    CMP_ORIENT_90,                  // Rotate 90, no mirror
    CMP_ORIENT_180,                 // Rotate 180, no mirror
    CMP_ORIENT_270,                 // Rotate -90, no mirror
    CMP_MIRROR_X = 0x100,           // Mirror around X axis
    CMP_MIRROR_Y = 0x200            // Mirror around Y axis
};


/** Schematic annotation order options. */
enum ANNOTATE_ORDER_T {
    SORT_BY_X_POSITION,     ///< Annotate by X position from left to right.
    SORT_BY_Y_POSITION,     ///< Annotate by Y position from top to bottom.
    UNSORTED,               ///< Annotate by position of component in the schematic sheet
                            ///< object list.
};


/** Schematic annotation type options. */
enum ANNOTATE_OPTION_T {
    INCREMENTAL_BY_REF,     ///< Annotate incrementally using the first free reference number.
    SHEET_NUMBER_X_100,     ///< Annotate using the first free reference number starting at
                            ///< the sheet number * 100.
    SHEET_NUMBER_X_1000,    ///< Annotate using the first free reference number starting at
                            ///< the sheet number * 1000.
};


/// Schematic search type used by the socket link with Pcbnew
enum SCH_SEARCH_T {
    FIND_COMPONENT_ONLY,    ///< Find a component in the schematic.
    FIND_PIN,               ///< Find a component pin in the schematic.
    FIND_REFERENCE,         ///< Find an item by it's reference designator.
    FIND_VALUE,             ///< Find an item by it's value field.
    FIND_FIELD              ///< Find a component field.
};


/**
 * Schematic editor (Eeschema) main window.
 */
class SCH_EDIT_FRAME : public EDA_DRAW_FRAME
{
public:
    wxComboBox*           m_SelPartBox;
    SCH_SHEET_PATH*       m_CurrentSheet;    ///< which sheet we are presently working on.
    int m_NetlistFormat;
    int m_AddSubPrefix;
    bool                  m_ShowAllPins;
    wxPoint               m_OldPos;
    LIB_EDIT_FRAME*       m_LibeditFrame;
    LIB_VIEW_FRAME*       m_ViewlibFrame;
    wxString              m_UserLibraryPath;
    wxArrayString         m_ComponentLibFiles;

private:
    wxString              m_DefaultSchematicFileName;
    int m_TextFieldSize;
    PARAM_CFG_ARRAY       m_projectFileParams;
    PARAM_CFG_ARRAY       m_configSettings;
    wxPageSetupDialogData m_pageSetupData;
    wxFindReplaceData*    m_findReplaceData;
    wxPoint               m_previewPosition;
    wxSize                m_previewSize;
    wxPoint               m_printDialogPosition;
    wxSize                m_printDialogSize;
    bool                  m_printMonochrome;     ///< Print monochrome instead of grey scale.
    bool                  m_printSheetReference;
    DIALOG_SCH_FIND*      m_dlgFindReplace;
    wxPoint               m_findDialogPosition;
    wxSize                m_findDialogSize;
    wxArrayString         m_findStringHistoryList;
    wxArrayString         m_replaceStringHistoryList;
    BLOCK_SELECTOR        m_blockItems;         ///< List of selected items.
    SCH_ITEM*             m_itemToRepeat;       ///< Last item to insert by the repeat command.
    int                   m_repeatLabelDelta;   ///< Repeat label number increment step.
    SCH_COLLECTOR         m_collectedItems;     ///< List of collected items.
    SCH_ITEM*             m_undoItem;           ///< Copy of the current item being edited.
    wxString              m_simulatorCommand;   ///< Command line used to call the circuit
                                                ///< simulator (gnucap, spice, ...)
    wxString              m_netListerCommand;   ///< Command line to call a custom net list
                                                ///< generator.

    static int            m_lastSheetPinType;      ///< Last sheet pin type.
    static wxSize         m_lastSheetPinTextSize;  ///< Last sheet pin text size.
    static wxPoint        m_lastSheetPinPosition;  ///< Last sheet pin position.

protected:
    TEMPLATES             m_TemplateFieldNames;

    /**
     * Function doAutoSave
     * saves the schematic files that have been modified and not yet saved.
     *
     * @return true if the auto save was successful otherwise false.
     */
    virtual bool doAutoSave();

    /**
     * Function autoSaveRequired
     * returns true if the schematic has been modified.
     */
    virtual bool isAutoSaveRequired() const;


public:
    SCH_EDIT_FRAME( wxWindow* father,
                    const wxString& title,
                    const wxPoint& pos, const wxSize& size,
                    long style = KICAD_DEFAULT_DRAWFRAME_STYLE );

    ~SCH_EDIT_FRAME();

    void OnCloseWindow( wxCloseEvent& Event );
    void Process_Special_Functions( wxCommandEvent& event );
    void OnColorConfig( wxCommandEvent& aEvent );
    void Process_Config( wxCommandEvent& event );
    void OnSelectTool( wxCommandEvent& aEvent );

    void GeneralControl( wxDC* aDC, const wxPoint& aPosition, int aHotKey = 0 );

    /**
     * Function GetProjectFileParameters
     * returns the project file parameter list for Eeschema.
     *
     *<p?
     * Populate the project file parameter array specific to Eeschema if it hasn't
     * already been populated and return a reference to the array to the caller.
     * Creating the parameter list at run time has the advantage of being able to
     * define local variables.  The old method of statically building the array at
     * compile time required global variable definitions.
     * </p>
     */
    PARAM_CFG_ARRAY& GetProjectFileParameters( void );

    /**
     * Function SaveProjectFile
     * saves changes to the project settings to the project (.pro) file.
     */
    void SaveProjectFile();

    /**
     * Function LoadProjectFile
     * loads the KiCad project file (*.pro) settings specific to Eeschema.
     *
     * @param aFileName The project file name to load.
     * @param aForceReread Force the project file to be reread if true.
     * @return True if the project file was loaded correctly.
     */
    bool LoadProjectFile( const wxString& aFileName, bool aForceReread );

    /**
     * Function GetDefaultFieldName
     * returns a default symbol field name for field \a aFieldNdx for all components.
     * These field names are not modifiable, but template field names are.
     * @param aFieldNdx The field number index
     */
    static wxString GetDefaultFieldName( int aFieldNdx );

    /**
     * Function AddTemplateFieldName
     * inserts or appends a wanted symbol field name into the field names
     * template.  Should be used for any symbol property editor.  If the name
     * already exists, it overwrites the same name.
     *
     * @param aFieldName is a full description of the wanted field, and it must not match
     *          any of the default field names.
     * @return int - the index within the config container at which aFieldName was
     *          added, or -1 if the name is illegal because it matches a default field name.
     */
    int AddTemplateFieldName( const TEMPLATE_FIELDNAME& aFieldName )
    {
        return m_TemplateFieldNames.AddTemplateFieldName( aFieldName );
    }


    /**
     * Function GetTemplateFieldName
     * returns a template field names list for read only access.
     */
    const TEMPLATE_FIELDNAMES& GetTemplateFieldNames()
    {
        return m_TemplateFieldNames.GetTemplateFieldNames();
    }


    /**
     * Function GetTemplates
     * returns the field names template for read only access.
     */
    const TEMPLATES& GetTemplates()
    {
        return m_TemplateFieldNames;
    }

    /**
     * Function DeleteAllTemplateFieldNames
     * removes all template field names.
     */
    void DeleteAllTemplateFieldNames()
    {
        m_TemplateFieldNames.DeleteAllTemplateFieldNames();
    }

    /**
     * Function GetConfigurationSettings
     * returns the Eeschema applications settings.
     * <p>
     * This replaces the old statically define list that had the project file settings and
     * the application settings mixed together.  This was confusing and caused some settings
     * to get saved and loaded incorrectly.  Currently, only the settings that are needed at
     * start up by the main window are defined here.  There are other locally used settings
     * scattered throughout the Eeschema source code.  If you need to define a configuration
     * setting that need to be loaded at run time, this is the place to define it.
     * </p>
     */
    PARAM_CFG_ARRAY& GetConfigurationSettings( void );

    void LoadSettings();
    void SaveSettings();

    void RedrawActiveWindow( wxDC* DC, bool EraseBg );

    void CreateScreens();
    void ReCreateHToolbar();
    void ReCreateVToolbar();
    void ReCreateOptToolbar();
    void ReCreateMenuBar();
    void OnHotKey( wxDC* aDC, int aHotKey, const wxPoint& aPosition, EDA_ITEM* aItem = NULL );

    /**
     * Function OnModify
     * Must be called after a schematic change
     * in order to set the "modify" flag of the current screen
     * and update the date in frame reference
     */
    void OnModify();

    SCH_SHEET_PATH* GetSheet();

    SCH_SCREEN* GetScreen() const;

    virtual wxString GetScreenDesc();

    void InstallConfigFrame( wxCommandEvent& event );

    void OnLeftClick( wxDC* aDC, const wxPoint& aPosition );
    void OnLeftDClick( wxDC* aDC, const wxPoint& aPosition );
    bool OnRightClick( const wxPoint& aPosition, wxMenu* PopMenu );
    void OnSelectOptionToolbar( wxCommandEvent& event );
    double BestZoom();

    /**
     * Function LocateAndShowItem
     * checks the schematic at \a aPosition in logical (drawing) units for a item
     * matching the types in \a aFilterList.
     * <p>
     * The search is first performed at the nearest grid position to \a aPosition.  If no
     * item if found on grid, then \a aPosition is tested for any items.  If the item found
     * can be cross probed, a message is send to Pcbnew and the selected item is highlighted
     * in PCB editor.
     * </p>
     * @param aPosition The wxPoint on the schematic to search.
     * @param aFilterList A list of #KICAD_T types to to filter.
     * @param aHotKeyCommandId A hot key command ID for performing additional tests when
     *                         multiple items are found at \a aPosition.
     * @return A SCH_ITEM pointer of the item found or NULL if no item found
     */
    SCH_ITEM* LocateAndShowItem( const wxPoint& aPosition,
                                 const KICAD_T aFilterList[] = SCH_COLLECTOR::AllItems,
                                 int aHotKeyCommandId = 0 );

    /**
     * Function LocateItem
     * checks for items at \a aPosition matching the types in \a aFilterList.
     * <p>
     * If multiple items are located at \a aPosition, a context menu is displayed to clarify
     * which item the user intended to select.  If the user aborts the context menu, NULL is
     * returned and the abort request flag will be set to true.  Make sure to clear this flag
     * before attempting to display any other context menus.
     * </p>
     *
     * @param aPosition The wxPoint location where to search.
     * @param aFilterList A list of #KICAD_T types to to filter.
     * @param aHotKeyCommandId A hot key command ID for performing additional tests when
     *                         multiple items are found at \a aPosition.
     * @return The SCH_ITEM pointer of the item found or NULL if no item found.
     */
    SCH_ITEM* LocateItem( const wxPoint& aPosition,
                          const KICAD_T aFilterList[] = SCH_COLLECTOR::AllItems,
                          int aHotKeyCommandId = 0 );

    /**
     * Function DeleteItemAtCrossHair
     * delete the item found under the cross hair.  If multiple items are found at the
     * cross hair position, a context menu is displayed to clarify which item to delete.
     * See LocateItem() for more information on locating multiple items.
     *
     * @param aDC The device context to update if and item is deleted.
     * @return True if an item was deleted.
     */
    bool DeleteItemAtCrossHair( wxDC* aDC );

    /**
     * Function FindComponentAndItem
     * finds a component in the schematic and an item in this component.
     * @param aReference The component reference designator to find.
     * @param aSearchHierarchy If false, search the current sheet only.  Otherwise,
     *                         the entire hierarchy
     * @param aSearchType A #SCH_SEARCH_T value used to determine what to search for.
     * @param aSearchText The text to search for, either in value, reference or elsewhere.
     * @param aWarpMouse If true, then move the mouse cursor to the item.
     */
    SCH_ITEM* FindComponentAndItem( const wxString& aReference,
                                    bool            aSearchHierarchy,
                                    SCH_SEARCH_T    aSearchType,
                                    const wxString& aSearchText,
                                    bool            aWarpMouse );

    /**
     * Function SendMessageToPcbnew
     * send a remote to Pcbnew via a socket connection.
     * @param objectToSync Item to be located on board (footprint, pad or text)
     * @param LibItem Component in library if objectToSync is a sub item of a component
     * <p>
     * Commands are
     * $PART: reference   put cursor on footprint anchor
     * $PIN: number $PART: reference put cursor on the footprint pad
     * </p>
     */
    void SendMessageToPCBNEW( EDA_ITEM* objectToSync, SCH_COMPONENT*  LibItem );

    /* netlist generation */
    void BuildNetListBase();

    /**
     * Function CreateNetlist
     * Create a netlist file:
     *  build netlist info
     *  test issues
     *  create file
     * @param aFormat = netlist format (NET_TYPE_PCBNEW ...)
     * @param aFullFileName = full netlist file name
     * @param aUse_netnames = bool. if true, use net names from labels in schematic
     *                              if false, use net numbers (net codes)
     *   bool aUse_netnames is used only for Spice netlist
     * @param aUsePrefix Prefix reference designator with an 'X' for spice output.
     * @return true if success.
     */
    bool CreateNetlist( int             aFormat,
                        const wxString& aFullFileName,
                        bool            aUse_netnames,
                        bool            aUsePrefix );

    /**
     * Function  WriteNetListFile
     * Create the netlist file. Netlist info must be existing
     * @param aFormat = netlist format (NET_TYPE_PCBNEW ...)
     * @param aFullFileName = full netlist file name
     * @param aUse_netnames = bool. if true, use net names from labels in schematic
     *                              if false, use net numbers (net codes)
     *   bool aUse_netnames is used only for Spice netlist
     * @return true if success.
     */
    bool WriteNetListFile( int             aFormat,
                           const wxString& aFullFileName,
                           bool            aUse_netnames,
                           bool            aUsePrefix );

    /**
     * Function DeleteAnnotation
     * clears the current component annotation.
     * @param aCurrentSheetOnly Clear the entire schematic annotation if true.  Otherwise
     *                          only clear the annotation for the current sheet.
     */
    void DeleteAnnotation( bool aCurrentSheetOnly );

    /**
     * Function AnnotateComponents
     *
     * annotates the components in the schematic that are not currently annotated.
     *
     * @param aAnnotateSchematic Annotate the entire schematic if true.  Otherwise annotate
     *                           the current sheet only.
     * @param aSortOption Define the annotation order.  See #ANNOTATE_ORDER_T.
     * @param aAlgoOption Define the annotation style.  See #ANNOTATE_OPTION_T.
     * @param aResetAnnotation Clear any previous annotation if true.  Otherwise, keep the
     *                         existing component annotation.
     * @param aRepairTimestamps Test for and repair any duplicate time stamps if true.
     *                          Otherwise, keep the existing time stamps.  This option
     *                          could change previous annotation because time stamps are
     *                          used to handle annotation in complex hierarchies.
     *
     * When the sheet number is used in annotation, each sheet annotation starts from sheet
     * number * 100.  In other words the first sheet uses 100 to 199, the second sheet uses
     * 200 to 299, and so on.
     */
    void AnnotateComponents( bool aAnnotateSchematic, ANNOTATE_ORDER_T aSortOption,
                             ANNOTATE_OPTION_T aAlgoOption, bool aResetAnnotation,
                             bool aRepairTimestamps );

    /**
     * Function CheckAnnotate
     * checks for annotation errors.
     *
     * <p>
     * The following list of items are checked:
     * <ul>
     * <li> Components that are not annotated.
     * <li> Duplicate component references.
     * <li> Multiple part per package components where the part\n
     *      number is greater number of parts in the package.
     * <li> Multiple part per package components where the reference\n
     *      designator is different between parts.
     * </ul>
     * </p>
     *
     * @return Number of annotation errors found.
     * @param aMessageList A wxArrayString to store error messages.
     * @param aOneSheetOnly Check the current sheet only if true.  Otherwise check
     *                      the entire schematic.
     */
    int CheckAnnotate( wxArrayString* aMessageList, bool aOneSheetOnly );

    // Functions used for hierarchy handling
    /**
     * Function DisplayCurrentSheet
     * draws the current sheet on the display.
     */
    void DisplayCurrentSheet();

    /**
     * Function GetUniqueFilenameForCurrentSheet
     * @return a filename that can be used in plot and print functions
     * for the current screen and sheet path.
     * This filename is unique and must be used instead of the screen filename
     * (or screen filename) when one must creates file for each sheet in the
     * hierarchy.  because in complex hierarchies a sheet and a SCH_SCREEN is
     * used more than once
     * Name is &ltroot sheet filename&gt-&ltsheet path&gt and has no extension.
     * However if filename is too long name is &ltsheet filename&gt-&ltsheet number&gt
     */
    wxString GetUniqueFilenameForCurrentSheet();

    /**
     * Function SetSheetNumberAndCount
     * Set the m_ScreenNumber and m_NumberOfScreen members for screens
     * must be called after a delete or add sheet command, and when entering
     * a sheet
     */
    void SetSheetNumberAndCount();

    /**
     * Show the print dialog
     */
    void OnPrint( wxCommandEvent& event );

    wxPageSetupDialogData& GetPageSetupData() { return m_pageSetupData; }

    void SetPreviewPosition( const wxPoint& aPoint ) { m_previewPosition = aPoint; }
    void SetPreviewSize( const wxSize& aSize ) { m_previewSize = aSize; }
    const wxPoint& GetPreviewPosition() { return m_previewPosition; }
    const wxSize& GetPreviewSize() { return m_previewSize; }

    void SetPrintDialogPosition( const wxPoint& aPoint )
    {
        m_printDialogPosition = aPoint;
    }


    void SetPrintDialogSize( const wxSize& aSize ) { m_printDialogSize = aSize; }
    const wxPoint& GetPrintDialogPosition() { return m_printDialogPosition; }
    const wxSize& GetPrintDialogSize() { return m_printDialogSize; }

    bool GetPrintMonochrome() { return m_printMonochrome; }
    void SetPrintMonochrome( bool aMonochrome ) { m_printMonochrome = aMonochrome; }
    bool GetPrintSheetReference() { return m_printSheetReference; }
    void SetPrintSheetReference( bool aShow ) { m_printSheetReference = aShow; }
    void SVG_Print( wxCommandEvent& event );

    // Plot functions:
    void ToPlot_PS( wxCommandEvent& event );
    void ToPlot_HPGL( wxCommandEvent& event );
    void ToPlot_DXF( wxCommandEvent& event );
    void ToPostProcess( wxCommandEvent& event );

    // read and save files
    void Save_File( wxCommandEvent& event );

    /**
     * Function OnSaveProject
     * is the command event handler to save the entire project and create a component library
     * archive.
     *
     * The component library archive name is &ltroot_name&gt-cache.lib
     */
    void OnSaveProject( wxCommandEvent& aEvent );

    /**
     * Function LoadOneEEProject
     * load an entire project into the schematic editor.
     *
     * This function loads  schematic root file and it's subhierarchies, the project
     * configuration, and the component libraries which are not already loaded.
     *
     * @param aFileName The full path an file name to load.
     * @param aIsNew True indicates that this is a new project and the default project
     *               template is loaded.
     * @return True if the project loaded properly.
     */
    bool LoadOneEEProject( const wxString& aFileName, bool aIsNew );

    /**
     * Function LoadOneEEFile
     * loads the schematic (.sch) file \a aFullFileName into \a aScreen.
     *
     * @param aScreen Pointer to the associated SCH_SCREEN object in which to load
     *                \a aFullFileName.
     * @param aFullFileName A reference to a wxString object containing the absolute path
     *                      and file name to load.
     * @return True if \a aFullFileName has been loaded (at least partially.)
     */
    bool LoadOneEEFile( SCH_SCREEN* aScreen, const wxString& aFullFileName );

    bool ReadInputStuffFile();

    /**
     * Function ProcessStuffFile
     * gets footprint info from each line in the Stuff File by Ref Desg
     *
     * Read a "stuff" file created by CvPcb.
     * That file has lines like:
     * comp = "C1" module = "CP6"
     * comp = "C2" module = "C1"
     * comp = "C3" module = "C1"
     * "comp =" gives the component reference
     * "module =" gives the footprint name
     *
     * @param aFilename The file to read from.
     * @param aSetFieldsAttributeToVisible = true to set the footprint field flag to visible
     * @return bool - true if success, else true.
     */
    bool ProcessStuffFile( FILE* aFilename, bool  aSetFieldsAttributeToVisible );

    /**
     * Function SaveEEFile
     * saves \a aScreen to a schematic file.
     *
     * @param aScreen A pointer to the SCH_SCREEN object to save.  A NULL pointer saves
     *                the current screen.
     * @param aSaveType Controls how the file is to be saved.
     * @param aCreateBackupFile Creates a back of the file associated with \a aScreen
     *                          if true.  Helper definitions #CREATE_BACKUP_FILE and
     *                          #NO_BACKUP_FILE are defined for improved code readability.
     * @return True if the file has been saved.
     */
    bool SaveEEFile( SCH_SCREEN* aScreen,
                     int         aSaveType,
                     bool        aCreateBackupFile = CREATE_BACKUP_FILE );

    // General search:

private:
    /**
     * Function OnMoveItem
     * handles the #ID_SCH_MOVE_ITEM event used to move schematic itams.
     */
    void OnMoveItem( wxCommandEvent& aEvent );

    /**
     * Function OnRotate
     * handles the #ID_SCH_ROTATE_CLOCKWISE and #ID_SCH_ROTATE_COUNTERCLOCKWISE events
     * used to rotate schematic itams and blocks.
     */
    void OnRotate( wxCommandEvent& aEvent );

    /**
     * Function OnEditItem
     * handles the #ID_SCH_EDIT_ITEM event used to edit schematic itams.
     */
    void OnEditItem( wxCommandEvent& aEvent );

    /**
     * Function OnDragItem
     * handles the #ID_SCH_DRAG_ITEM event used to drag schematic itams.
     */
    void OnDragItem( wxCommandEvent& aEvent );

    /**
     * Function OnOrient
     * handles the #ID_SCH_MIRROR_X, #ID_SCH_MIRROR_Y, and #ID_SCH_ORIENT_NORMAL events
     * used to orient schematic itams and blocks.
     */
    void OnOrient( wxCommandEvent& aEvent );

    void OnExit( wxCommandEvent& event );
    void OnAnnotate( wxCommandEvent& event );
    void OnErc( wxCommandEvent& event );
    void OnCreateNetlist( wxCommandEvent& event );
    void OnCreateBillOfMaterials( wxCommandEvent& event );
    void OnFindItems( wxCommandEvent& event );
    void OnFindDialogClose( wxFindDialogEvent& event );
    void OnFindDrcMarker( wxFindDialogEvent& event );
    void OnFindCompnentInLib( wxFindDialogEvent& event );

    /**
     * Function OnFindSchematicItem
     * finds an item in the schematic matching the search criteria in \a aEvent.
     *
     * @param aEvent - Find dialog event containing the find parameters.
     */
    void OnFindSchematicItem( wxFindDialogEvent& aEvent );

    /**
     * Function OnReplace
     * performs a search and replace of text in an item in the schematic matching the
     * search and replace criteria in \a aEvent.
     *
     * @param aEvent - Find dialog event containing the search and replace parameters.
     */
    void OnFindReplace( wxFindDialogEvent& aEvent );

    void OnLoadFile( wxCommandEvent& event );
    void OnLoadStuffFile( wxCommandEvent& event );
    void OnNewProject( wxCommandEvent& event );
    void OnLoadProject( wxCommandEvent& event );
    void OnOpenPcbnew( wxCommandEvent& event );
    void OnOpenCvpcb( wxCommandEvent& event );
    void OnOpenLibraryViewer( wxCommandEvent& event );
    void OnOpenLibraryEditor( wxCommandEvent& event );
    void OnSetOptions( wxCommandEvent& event );
    void OnCancelCurrentCommand( wxCommandEvent& aEvent );

    void OnSelectItem( wxCommandEvent& aEvent );

    /**
     * Function OnCopySchematicItemRequest
     * is the command event handler for duplicating the item at the current location.
     */
    void OnCopySchematicItemRequest( wxCommandEvent& event );

    /* User interface update event handlers. */
    void OnUpdateBlockSelected( wxUpdateUIEvent& event );
    void OnUpdatePaste( wxUpdateUIEvent& event );
    void OnUpdateHiddenPins( wxUpdateUIEvent& event );
    void OnUpdateBusOrientation( wxUpdateUIEvent& event );
    void OnUpdateSelectTool( wxUpdateUIEvent& aEvent );

    /**
     * Function SetLanguage
     * called on a language menu selection
     */
    void SetLanguage( wxCommandEvent& event );

    // Bus Entry
    SCH_BUS_ENTRY* CreateBusEntry( wxDC* DC, int entry_type );
    void SetBusEntryShape( wxDC* DC, SCH_BUS_ENTRY* BusEntry, int entry_type );
    int GetBusEntryShape( SCH_BUS_ENTRY* BusEntry );

    /**
     * Function AddNoConnect
     * add a no connect item to the current schematic sheet at \a aPosition.
     * @param aDC The device context to draw the no connect to.
     * @param aPosition The position in logical (drawing) units to add the no connect.
     * @return The no connect item added.
     */
    SCH_NO_CONNECT* AddNoConnect( wxDC* aDC, const wxPoint& aPosition );

    /**
     * Function AddJunction
     * adds a new junction at \a aPosition.
     */
    SCH_JUNCTION* AddJunction( wxDC* aDC, const wxPoint& aPosition, bool aPutInUndoList = false );

    /**
     * Function MoveItem
     * start moving \a aItem using the mouse.
     *
     * @param aItem A pointer to an SCH_ITEM to move.
     * @param aDC The device context to draw \a aItem.
     */
    void MoveItem( SCH_ITEM* aItem, wxDC* aDC );

    // Text, label, glabel
    SCH_TEXT* CreateNewText( wxDC* aDC, int aType );
    void EditSchematicText( SCH_TEXT* TextStruct );
    void ChangeTextOrient( SCH_TEXT* aTextItem, wxDC* aDC );

    /**
     * Function OnCovertTextType
     * is a command event handler to change a text type to an other one.  The new text,
     * label, hierarchical label, or global label is created from the old text and the
     * old text is deleted.
     */
    void OnConvertTextType( wxCommandEvent& aEvent );

    /**
     * Function BeginSegment
     * creates a new segment ( WIRE, BUS ) or terminates the current segment in progress.
     *
     * If the end of the current segment is on an other segment, place a junction if needed
     * and terminates the command.  If the end of the current segment is on a pin, terminate
     * the command.  In all other cases starts a new segment.
     */
    void BeginSegment( wxDC* DC, int type );

    /**
     * Function EndSegment
     * called to terminate a bus, wire, or line creation
     */
    void EndSegment( wxDC* DC );

    /**
     * Function DeleteCurrentSegment
     * erases the last segment at the current mouse position.
     */
    void DeleteCurrentSegment( wxDC* DC );
    void DeleteConnection( bool DeleteFullConnection );

    // graphic lines
    void Edge( DRAWSEGMENT* Segment, wxDC* DC );
    void SetNewWidth( DRAWSEGMENT* DrawSegm, wxDC* DC );
    void Layer( DRAWSEGMENT* Segment, wxDC* DC );
    DRAWSEGMENT* Begin_Edge( DRAWSEGMENT* Segment, wxDC* DC );

    // Images:
    SCH_BITMAP* CreateNewImage( wxDC* aDC );
    void MoveImage( SCH_BITMAP* aItem, wxDC* aDC );
    void RotateImage( SCH_BITMAP* aItem );
    /**
     * Function MirrorImage
     * Mirror a bitmap
     * @param aItem = the SCH_BITMAP item to mirror
     * @param Is_X_axis = true to mirror relative to Horizontal axis
     *                      false to mirror relative to vertical axis
     */
    void MirrorImage( SCH_BITMAP* aItem, bool Is_X_axis );
    void EditImage( SCH_BITMAP* aItem );

    // Hierarchical Sheet & PinSheet
    void InstallHierarchyFrame( wxDC* DC, wxPoint& pos );
    SCH_SHEET* CreateSheet( wxDC* DC );
    void ReSizeSheet( SCH_SHEET* Sheet, wxDC* DC );

    /**
     * Use the component viewer to select component to import into schematic.
     */
    wxString SelectFromLibBrowser( void );

public:
    /**
     * Function EditSheet
     * is used to edit an existing sheet or add a new sheet to the schematic.
     * <p>
     * When \a aSheet is a new sheet:
     * <ul>
     * <li>and the file name already exists in the schematic hierarchy, the screen associated
     * with the sheet found in the hierarchy is associated with \a aSheet.</li>
     * <li>and the file name already exists on the system, then \a aSheet is loaded with the
     * existing file.</li>
     * <li>and the file name does not exist in the schematic hierarchy or on the file system,
     * then a new screen is created and associated with \a aSheet.</li>
     * </ul> </p> <p>
     * When \a aSheet is an existing sheet:
     * <ul>
     * <li>and the file name already exists in the schematic hierarchy, the current associated
     * screen is replace by the one found in the hierarchy.</li>
     * <li>and the file name already exists on the system, the current associated screen file
     * name is changed and the file is loaded.</li>
     * <li>and the file name does not exist in the schematic hierarchy or on the file system,
     * the current associated screen file name is changed and saved to disk.</li>
     * </ul> </p>
     */
    bool EditSheet( SCH_SHEET* aSheet, wxDC* aDC );

    wxPoint GetLastSheetPinPosition() const { return m_lastSheetPinPosition; }

private:
    void StartMoveSheet( SCH_SHEET* sheet, wxDC* DC );

    /**
     * Function CreateSheetPin
     * creates a new SCH_SHEET_PIN object and add it to \a aSheet at the current cursor position.
     * @param aSheet The sheet to add the new sheet pin to.
     * @param aDC The device context to draw on.
     * @return The new sheet pin object created or NULL if the task was aborted by the user.
     */
    SCH_SHEET_PIN* CreateSheetPin( SCH_SHEET* aSheet, wxDC* aDC );

    /**
     * Function EditSheetPin
     * displays the dialog for editing the parameters of \a aSheetPin.
     * @param aSheetPin The sheet pin item to edit.
     * @param aDC The device context to draw on.
     * @return The user response from the edit dialog.
     */
    int EditSheetPin( SCH_SHEET_PIN* aSheetPin, wxDC* aDC );

    /**
     * Function ImportSheetPin
     * automatically creates a sheet pin from the hierarchical labels in the schematic
     * referenced by \a aSheet.
     * @param aSheet The sheet to import the new sheet pin to.
     * @param aDC The device context to draw on.
     * @return The new sheet pin object importd or NULL if the task was aborted by the user.
     */
    SCH_SHEET_PIN* ImportSheetPin( SCH_SHEET* aSheet, wxDC* aDC );

public:
    /**
     * Function DeleteItem
     * removes \a aItem from the current screen and saves it in the undo list.
     * @param aItem The item to remove from the current screen.
     */
    void DeleteItem( SCH_ITEM* aItem );

    int GetLabelIncrement() const { return m_repeatLabelDelta; }

private:

    // Component
    SCH_COMPONENT* Load_Component( wxDC*           DC,
                                   const wxString& libname,
                                   wxArrayString&  List,
                                   bool            UseLibBrowser );

    /**
     * Function EditComponent
     * displays the edit component dialog to edit the parameters of \a aComponent.
     *
     * @param aComponent is a pointer to the SCH_COMPONENT object to be edited.
     */
    void EditComponent( SCH_COMPONENT* aComponent );

public:
    void OrientComponent( COMPONENT_ORIENTATION_T aOrientation = CMP_NORMAL );

private:
    void OnSelectUnit( wxCommandEvent& aEvent );
    void ConvertPart( SCH_COMPONENT* DrawComponent, wxDC* DC );
    void SetInitCmp( SCH_COMPONENT* DrawComponent, wxDC* DC );

    void EditComponentFieldText( SCH_FIELD* aField, wxDC* aDC );
    void RotateField( SCH_FIELD* aField, wxDC* aDC );

    /**
     * Function PastListOfItems
     * pastes a list of items from the block stack.
     */
    void PasteListOfItems( wxDC* DC );

    /* Undo - redo */
public:

    /**
     * Function SaveCopyInUndoList.
     * Create a copy of the current schematic item, and put it in the undo list.
     *
     *  flag_type_command =
     *      UR_CHANGED
     *      UR_NEW
     *      UR_DELETED
     *      UR_WIRE_IMAGE
     *      UR_MOVED
     *
     * If it is a delete command, items are put on list with the .Flags member
     * set to UR_DELETED.  When it will be really deleted, the GetDrawItems() and the
     * sub-hierarchy will be deleted.  If it is only a copy, the GetDrawItems() and the
     * sub-hierarchy must NOT be deleted.
     *
     * @note
     * Edit wires and buses is a bit complex.
     * because when a new wire is added, a lot of modifications in wire list is made
     * (wire concatenation): modified items, deleted items and new items
     * so flag_type_command is UR_WIRE_IMAGE: the struct ItemToCopy is a list of
     * wires saved in Undo List (for Undo or Redo commands, saved wires will be
     * exchanged with current wire list
     * @param aItemToCopy = the schematic item modified by the command to undo
     * @param aTypeCommand = command type (see enum UNDO_REDO_T)
     * @param aTransformPoint = the reference point of the transformation,
     *                          for commands like move
     */
    void SaveCopyInUndoList( SCH_ITEM* aItemToCopy,
                             UNDO_REDO_T aTypeCommand,
                             const wxPoint& aTransformPoint = wxPoint( 0, 0 ) );

    /**
     * Function SaveCopyInUndoList (overloaded).
     * Creates a new entry in undo list of commands.
     * add a list of pickers to handle a list of items
     * @param aItemsList = the list of items modified by the command to undo
     * @param aTypeCommand = command type (see enum UNDO_REDO_T)
     * @param aTransformPoint = the reference point of the transformation,
     *                          for commands like move
     */
    void SaveCopyInUndoList( PICKED_ITEMS_LIST& aItemsList,
                             UNDO_REDO_T aTypeCommand,
                             const wxPoint& aTransformPoint = wxPoint( 0, 0 ) );

private:

    /**
     * Function PutDataInPreviousState
     * is used in undo or redo command to put data pointed by List in the previous state, i.e.
     * the state stored in \a aList
     * @param aList a PICKED_ITEMS_LIST pointer to the list of items to undo/redo
     * @param aRedoCommand  a bool: true for redo, false for undo
     */
    void PutDataInPreviousState( PICKED_ITEMS_LIST* aList, bool aRedoCommand );

    /**
     * Function GetSchematicFromRedoList
     *  Redo the last edition:
     *  - Save the current schematic in Undo list
     *  - Get an old version of the schematic from Redo list
     *  @return none
     */
    void GetSchematicFromRedoList( wxCommandEvent& event );

    /**
     * Function GetSchematicFromUndoList
     * performs an undo the last edition:
     *  - Save the current schematic in Redo list
     *  - Get an old version of the schematic from Undo list
     */
    void GetSchematicFromUndoList( wxCommandEvent& event );

    /**
     * Function copyBlockItems
     * copies the list of block item.
     * @sa m_blockItems
     * @param aItemsList List to copy the block select items into.
     */
    void copyBlockItems( PICKED_ITEMS_LIST& aItemsList );

    /**
     * Function addJunctionMenuEntries
     * adds the context menu items to \a aMenu for \a aJunction.
     * @param aMenu The menu to add the items to.
     * @param aJunction The SCH_JUNCTION object selected.
     */
    void addJunctionMenuEntries( wxMenu* aMenu, SCH_JUNCTION* aJunction );

public:
    void Key( wxDC* DC, int hotkey, EDA_ITEM* DrawStruct );

    /**
     * Function InitBlockPasteInfos
     * initializes the parameters used by the block paste command.
     */
    void InitBlockPasteInfos();

    /* Function HandleBlockEndByPopUp
     * performs an end block command from context menu.
     *
     * This can be called only after HandleBlockEnd and the current command is block
     * move.  Execute a command other than block move from the current block move
     * selected items list.  Due to (minor) problems in undo/redo or/and display block,
     * a mirror/rotate command is immediately executed and multiple block commands are
     * not allowed (multiple commands are tricky to undo/redo in one time)
     */
    void HandleBlockEndByPopUp( int Command, wxDC* DC );

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
     * Function HandleBlockPlace
     * Called after HandleBlockEnd, when a block command needs to be
     * executed after the block is moved to its new place
     * (bloc move, drag, copy .. )
     * Parameters must be initialized in GetScreen()->m_BlockLocate
     */
    virtual void HandleBlockPlace( wxDC* DC );

    /**
     * Function HandleBlockEnd
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
     * Function RepeatDrawItem
     * repeats the last item placement if the last item was a bus, bus entry,
     * label, or component.
     *
     * Labels that end with a number will be incremented.
     */
    void RepeatDrawItem( wxDC* DC );

    void SetRepeatItem( SCH_ITEM* aItem ) { m_itemToRepeat = aItem; }

    /**
     * Function SetUndoItem
     * clones \a aItem which can be used to restore the state of the item being edited
     * when the user cancels the editing in progress.
     *
     * @param aItem The item to make a clone of for undoing the last change.  Set to
     *              NULL to free the current undo item.
     */
    void SetUndoItem( const SCH_ITEM* aItem );

    SCH_ITEM* GetUndoItem() const { return m_undoItem; }

    /**
     * Function SaveUndoItemInUndoList
     * swaps the cloned item in member variable m_undoItem with \a aItem and saves it to
     * the undo list then swap the data back.  This swaps the internal structure of the
     * item with the cloned item.  It does not swap the actual item pointers themselves.
     *
     * @param aItem The item to swap with the current undo item.
     */
    void SaveUndoItemInUndoList( SCH_ITEM* aItem );

    /**
     * Function LoadLibraries
     *
     * Clear all libraries currently loaded and load all of the project libraries.
     */
    void LoadLibraries( void );

    /**
     * Function CreateArchiveLibrary
     * creates a library \a aFileName that contains all components used in the current schematic.
     *
     * @param aFileName The full path and file name of the archive library.
     * @return True if \a aFileName was written successfully.
     */
    bool CreateArchiveLibrary( const wxString& aFileName );

    /**
     * Function PrintPage
     * plots or prints the current sheet to the clipboard.
     * @param aDC = wxDC given by the calling print function
     * @param aPrintMask = not used here
     * @param aPrintMirrorMode = not used here (Set when printing in mirror mode)
     * @param aData = a pointer on an auxiliary data (not always used, NULL if not used)
     */
    virtual void PrintPage( wxDC* aDC, int aPrintMask,
                            bool aPrintMirrorMode, void* aData = NULL );

    void SetSimulatorCommand( const wxString& aCommand ) { m_simulatorCommand = aCommand; }

    wxString GetSimulatorCommand() const { return m_simulatorCommand; }

    void SetNetListerCommand( const wxString& aCommand ) { m_netListerCommand = aCommand; }

    wxString GetNetListerCommand() const { return m_netListerCommand; }

    DECLARE_EVENT_TABLE()
};


#endif  // WX_EESCHEMA_STRUCT_H
