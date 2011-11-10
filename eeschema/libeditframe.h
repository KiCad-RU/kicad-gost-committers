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
 * @file libeditframe.h
 * @brief Definition of class LIB_EDIT_FRAME
 */

#ifndef __LIBEDITFRM_H__
#define __LIBEDITFRM_H__

#include "wxstruct.h"
#include "class_sch_screen.h"

#include "lib_draw_item.h"
#include "lib_collectors.h"


class SCH_EDIT_FRAME;
class CMP_LIBRARY;
class LIB_COMPONENT;
class LIB_ALIAS;
class LIB_FIELD;
class DIALOG_LIB_EDIT_TEXT;


/**
 * The component library editor main window.
 */
class LIB_EDIT_FRAME : public EDA_DRAW_FRAME
{
    LIB_COMPONENT* m_tempCopyComponent;  ///< Temporary copy of current component during edit.
    LIB_COLLECTOR m_collectedItems;      // Used for hit testing.

    LIB_ITEM* locateItem( const wxPoint& aPosition, const KICAD_T aFilterList[] );

public:
    wxComboBox* m_SelpartBox;            // a Box to select a part to edit (if any)
    wxComboBox* m_SelAliasBox;           // a box to select the alias to edit (if any)

public:
    LIB_EDIT_FRAME( SCH_EDIT_FRAME* aParent, const wxString& title,
                    const wxPoint& pos, const wxSize& size,
                    long style = KICAD_DEFAULT_DRAWFRAME_STYLE );

    ~LIB_EDIT_FRAME();

    void ReCreateMenuBar();

    /**
     * Function EnsureActiveLibExists
     * must be called after the libraries are reloaded
     * (for instance after loading a schematic project)
     */
    static void EnsureActiveLibExists();

    /**
     * Function SetLanguage
     * is called on a language menu selection
     */
    void SetLanguage( wxCommandEvent& event );

    void InstallConfigFrame( wxCommandEvent& event );
    void InstallDimensionsDialog( wxCommandEvent& event );
    void OnColorConfig( wxCommandEvent& aEvent );
    void Process_Config( wxCommandEvent& event );

    /**
     * Function SycnronizePins
     * @return True if the edit pins per part or convert is false and the current
     *         component has multiple parts or body styles.  Otherwise false is
     *         returned.
     */
    bool SynchronizePins() const;

    /**
     * Function OnPlotCurrentComponent
     * plot the current component in SVG or PNG format.
     */
    void OnPlotCurrentComponent( wxCommandEvent& event );
    void Process_Special_Functions( wxCommandEvent& event );
    void OnSelectTool( wxCommandEvent& aEvent );

    /**
     * Routine to read one part.
     * The format is that of libraries, but it loads only 1 component.
     * Or 1 component if there are several.
     * If the first component is an alias, it will load the corresponding root.
     */
    void OnImportPart( wxCommandEvent& event );

    /**
     * Function OnExportPart
     * creates a new library and backup the current component in this library or export
     * the component of the current library.
     */
    void OnExportPart( wxCommandEvent& event );
    void OnSelectAlias( wxCommandEvent& event );
    void OnSelectPart( wxCommandEvent& event );

    /**
     * Function DeleteOnePart
     * is the command event handler to delete an entry from the current library.
     *
     * The deleted entry can be an alias or a component.  If the entry is an alias,
     * it is removed from the component and the list of alias is updated.  If the
     * entry is a component and the list of aliases is empty, the component and all
     * it drawable items are deleted.  Otherwise the first alias in the alias list
     * becomes the new component name and the other aliases become dependent on
     * renamed component.
     *
     * @note This only deletes the entry in memory.  The file does not change.
     */
    void DeleteOnePart( wxCommandEvent& event );

    /**
     * Function CreateNewLibraryPart
     * is the command event handler to create a new library component.
     *
     * If an old component is currently in edit, it is deleted.
     */
    void CreateNewLibraryPart( wxCommandEvent& event );

    void OnCreateNewPartFromExisting( wxCommandEvent& event );
    void OnEditComponentProperties( wxCommandEvent& event );
    void InstallFieldsEditorDialog(  wxCommandEvent& event );

    /**
     * Function LoadOneLibraryPart
     * loads a library component from the currently selected library.
     *
     * If a library is already selected, the user is prompted for the component name
     * to load.  If there is no current selected library, the user is prompted to select
     * a library name and then select component to load.
     */
    void LoadOneLibraryPart( wxCommandEvent& event );

    void OnViewEntryDoc( wxCommandEvent& event );
    void OnCheckComponent( wxCommandEvent& event );
    void OnSelectBodyStyle( wxCommandEvent& event );
    void OnEditPin( wxCommandEvent& event );
    void OnSelectItem( wxCommandEvent& aEvent );

    void OnUpdateSelectTool( wxUpdateUIEvent& aEvent );
    void OnUpdateEditingPart( wxUpdateUIEvent& event );
    void OnUpdateNotEditingPart( wxUpdateUIEvent& event );
    void OnUpdateUndo( wxUpdateUIEvent& event );
    void OnUpdateRedo( wxUpdateUIEvent& event );
    void OnUpdateSaveCurrentLib( wxUpdateUIEvent& event );
    void OnUpdateViewDoc( wxUpdateUIEvent& event );
    void OnUpdatePinByPin( wxUpdateUIEvent& event );
    void OnUpdatePartNumber( wxUpdateUIEvent& event );
    void OnUpdateDeMorganNormal( wxUpdateUIEvent& event );
    void OnUpdateDeMorganConvert( wxUpdateUIEvent& event );
    void OnUpdateSelectAlias( wxUpdateUIEvent& event );

    void UpdateAliasSelectList();
    void UpdatePartSelectList();

    /**
     * Function DisplayLibInfos
     * updates the main window title bar with the current library name and read only status
     * of the library.
     */
    void DisplayLibInfos();

    void RedrawActiveWindow( wxDC* DC, bool EraseBg );
    void OnCloseWindow( wxCloseEvent& Event );
    void ReCreateHToolbar();
    void ReCreateVToolbar();
    void CreateOptionToolbar();
    void OnLeftClick( wxDC* DC, const wxPoint& MousePos );
    bool OnRightClick( const wxPoint& MousePos, wxMenu* PopMenu );
    double BestZoom();         // Returns the best zoom
    void OnLeftDClick( wxDC* DC, const wxPoint& MousePos );

    SCH_SCREEN* GetScreen() { return (SCH_SCREEN*) EDA_DRAW_FRAME::GetScreen(); }

    void OnHotKey( wxDC* aDC, int aHotKey, const wxPoint& aPosition, EDA_ITEM* aItem = NULL );

    void GeneralControl( wxDC* aDC, const wxPoint& aPosition, int aHotKey = 0 );

    /**
     * Function LoadSettings
     * loads the library editor frame specific configuration settings.
     *
     * Don't forget to call this method from any derived classes or the settings will not
     * get loaded.
     */
    void LoadSettings();

    /**
     * Function SaveSettings
     * saves the library editor frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the settings will
     * not get saved.
     */
    void SaveSettings();

    /**
     * Function CloseWindow
     * triggers the wxCloseEvent, which is handled by the function given
     * to EVT_CLOSE() macro:
     * <p>
     * EVT_CLOSE( LIB_EDIT_FRAME::OnCloseWindow )
     */
    void CloseWindow( wxCommandEvent& event )
    {
        // Generate a wxCloseEvent
        Close( false );
    }


    /**
     * Function OnModify
     * Must be called after a schematic change
     * in order to set the "modify" flag of the current screen
     */
    void OnModify()
    {
        GetScreen()->SetModify();
    }


    LIB_COMPONENT* GetComponent( void ) { return m_component; }

    CMP_LIBRARY* GetLibrary( void ) { return m_library; }

    wxString& GetAliasName( void ) { return m_aliasName; }

    int GetUnit( void ) { return m_unit; }

    void SetUnit( int unit )
    {
        wxASSERT( unit >= 1 );
        m_unit = unit;
    }


    int GetConvert( void ) { return m_convert; }

    void SetConvert( int convert )
    {
        wxASSERT( convert >= 0 );
        m_convert = convert;
    }


    LIB_ITEM* GetLastDrawItem( void ) { return m_lastDrawItem; }

    void SetLastDrawItem( LIB_ITEM* drawItem )
    {
        m_lastDrawItem = drawItem;
    }


    LIB_ITEM* GetDrawItem( void ) { return m_drawItem; }

    void SetDrawItem( LIB_ITEM* drawItem );

    bool GetShowDeMorgan( void ) { return m_showDeMorgan; }

    void SetShowDeMorgan( bool show ) { m_showDeMorgan = show; }

    FILL_T GetFillStyle( void ) { return m_drawFillStyle; }

    /**
     * Function TempCopyComponent
     * create a temporary copy of the current edited component
     * Used to prepare an Undo ant/or abort command before editing the component
     */
    void TempCopyComponent();

    /**
     * Function RestoreComponent
     * Restore the current edited component from its temporary copy.
     * Used to abort a command
     */
    void RestoreComponent();

    /**
     * Function GetTempCopyComponent
     * @return the temporary copy of the current component.
     */
    LIB_COMPONENT* GetTempCopyComponent() { return m_tempCopyComponent; }

    /**
     * Function ClearTempCopyComponent
     * delete temporary copy of the current component and clear pointer
     */
    void ClearTempCopyComponent();

    bool IsEditingDrawItem() { return m_drawItem && m_drawItem->InEditMode(); }

private:

    /**
     * Function OnActivate
     * is called when the frame is activated. Tests if the current library exists.
     * The library list can be changed by the schematic editor after reloading a new schematic
     * and the current m_library can point a non existent lib.
     */
    virtual void OnActivate( wxActivateEvent& event );

    // General:

    /**
     * Function SaveOnePartInMemory
     * updates the current component being edited in the active library.
     *
     * Any changes are updated in memory only and NOT to a file.  The old component is
     * deleted from the library and/or any aliases before the edited component is updated
     * in the library.
     */
    void SaveOnePartInMemory();

    /**
     * Function SelectActiveLibrary
     * sets the current active library to \a aLibrary.
     *
     * @param aLibrary A pointer to the CMP_LIBRARY object to select.  If NULL, then display
     *                 list of available libraries to select from.
     */
    void SelectActiveLibrary( CMP_LIBRARY* aLibrary = NULL );

    /**
     * Function SaveActiveLibrary
     * it the command event handler to save the changes to the current library.
     *
     * A backup file of the current library is saved with the .bak extension before the
     * changes made to the library are saved.
     */
    void SaveActiveLibrary( wxCommandEvent& event );

    /**
     * Function LoadComponentFromCurrentLib
     * loads a component from the current active library.
     * @param aLibEntry The component to load from \a aLibrary (can be an alias)
     * @return true if \a aLibEntry loaded correctly.
     */
    bool LoadComponentFromCurrentLib( LIB_ALIAS* aLibEntry );

    /**
     * Function LoadOneLibraryPartAux
     * loads a copy of \a aLibEntry from \a aLibrary into memory.
     *
     * @param aLibEntry A pointer to the LIB_ALIAS object to load.
     * @param aLibrary A pointer to the CMP_LIBRARY object to load \a aLibEntry from.
     * @return True if a copy of \a aLibEntry was successfully loaded from \a aLibrary.
     */
    bool LoadOneLibraryPartAux( LIB_ALIAS* aLibEntry, CMP_LIBRARY* aLibrary );

    /**
     * Function DisplayCmpDoc
     * displays the documentation of the selected component.
     */
    void DisplayCmpDoc();

    /**
     * Function OnRotateItem
     * rotates the current item.
     */
    void OnRotateItem( wxCommandEvent& aEvent );

    /**
     * Function deleteItem
     * deletes the currently selected draw item.
     * @param aDC The device context to draw upon when removing item.
     */
    void deleteItem( wxDC* aDC );

    // General editing
public:
    void SaveCopyInUndoList( EDA_ITEM* ItemToCopy, int flag_type_command = 0 );

private:
    void GetComponentFromUndoList( wxCommandEvent& event );
    void GetComponentFromRedoList( wxCommandEvent& event );

    // Editing pins
    void CreatePin( wxDC* DC );
    void StartMovePin( wxDC* DC );

    /**
     * Function CreateImagePins
     * adds copies of \a aPin for \a aUnit in components with multiple parts and
     * \a aConvert for components that have multiple body styles.
     *
     * @param aPin The pin to copy.
     * @param aUnit The unit to add a copy of \a aPin to.
     * @param aConvert The alternate body style to add a copy of \a aPin to.
     * @param aDeMorgan Flag to indicate if \a aPin should be created for the
     *                  alternate body style.
     */
    void CreateImagePins( LIB_PIN* aPin, int aUnit, int aConvert, bool aDeMorgan );

    /**
     * Function PlaceAnchor
     * places an  anchor reference coordinate for the current component.
     * <p>
     * All object coordinates are offset to the current cursor position.
     * </p>
     */
    void PlaceAnchor();

    // Editing graphic items
    LIB_ITEM* CreateGraphicItem( LIB_COMPONENT* LibEntry, wxDC* DC );
    void GraphicItemBeginDraw( wxDC* DC );
    void StartMoveDrawSymbol( wxDC* DC );
    void StartModifyDrawSymbol( wxDC* DC ); //<! Modify the item, adjust size etc.
    void EndDrawGraphicItem( wxDC* DC );

    /**
     * Function LoadOneSymbol
     * read a component symbol file (*.sym ) and add graphic items to the current component.
     * <p>
     * A symbol file *.sym has the same format as a library, and contains only
     * one symbol.
     * </p>
     */
    void LoadOneSymbol();

    /**
     * Function SaveOneSymbol
     * saves the current component to a symbol file.
     * <p>
     * The symbol file format is similar to the standard component library file format, but
     * there is only one symbol.  Invisible pins are not saved.
     */
    void SaveOneSymbol();

    void EditGraphicSymbol( wxDC* DC, LIB_ITEM* DrawItem );
    void EditSymbolText( wxDC* DC, LIB_ITEM* DrawItem );
    LIB_ITEM* LocateItemUsingCursor( const wxPoint& aPosition,
                                     const KICAD_T aFilterList[] = LIB_COLLECTOR::AllItems );
    void EditField( wxDC* DC, LIB_FIELD* Field );

public:
    /**
     * Function LoadComponentAndSelectLib
     * selects the current active library.
     *
     * @param aLibrary The CMP_LIBRARY to select
     * @param aLibEntry The component to load from aLibrary (can be an alias).
     * @return true if \a aLibEntry was loaded from \a aLibrary.
     */
    bool LoadComponentAndSelectLib( LIB_ALIAS* aLibEntry, CMP_LIBRARY* aLibrary );

    /* Block commands: */

    /**
     * Function ReturnBlockCommand
     * returns the block command (BLOCK_MOVE, BLOCK_COPY...) corresponding to
     * the \a aKey (ALT, SHIFT ALT ..)
     */
    virtual int ReturnBlockCommand( int aKey );

    /**
     * Function HandleBlockPlace
     * handles the block place command.
     */
    virtual void HandleBlockPlace( wxDC* DC );

    /**
     * Function HandleBlockEnd
     * performs a block end command.
     * @return If command finished (zoom, delete ...) false is returned otherwise true
     *         is returned indicating more processing is required.
     */
    virtual bool HandleBlockEnd( wxDC* DC );

    void PlacePin( wxDC* DC );
    void GlobalSetPins( wxDC* DC, LIB_PIN* MasterPin, int id );

    // Automatic placement of pins
    void RepeatPinItem( wxDC* DC, LIB_PIN* Pin );

protected:
    wxString m_ConfigPath;
    wxString m_LastLibImportPath;
    wxString m_LastLibExportPath;

    /** Convert of the item currently being drawn. */
    bool m_drawSpecificConvert;

    /**
     * Specify which component parts the current draw item applies to.
     *
     * If true, the item being drawn or edited applies only to the selected
     * part.  Otherwise it applies to all parts in the component.
     */
    bool m_drawSpecificUnit;

    /**
     * Set to true to not synchronize pins at the same position when editing
     * components with multiple parts or multiple body styles.  Setting this
     * to false allows editing each pin per part or body style individually.
     * This requires the user to open each part or body style to make changes
     * to the pin at the same location.
     */
    bool m_editPinsPerPartOrConvert;

    /** The current draw or edit graphic item fill style. */
    static FILL_T m_drawFillStyle;

    /** Default line width for drawing or editing graphic items. */
    static int m_drawLineWidth;

    /** The current active library. NULL if no active library is selected. */
    static CMP_LIBRARY* m_library;
    /** The current component being edited.  NULL if no component is selected. */
    static LIB_COMPONENT* m_component;

    static LIB_ITEM* m_lastDrawItem;
    static LIB_ITEM* m_drawItem;
    static wxString m_aliasName;

    // The unit number to edit and show
    static int m_unit;

    // Show the normal shape ( m_convert <= 1 ) or the converted shape
    // ( m_convert > 1 )
    static int m_convert;

    // true to force DeMorgan/normal tools selection enabled.
    // They are enabled when the loaded component has
    // Graphic items for converted shape
    // But under some circumstances (New component created)
    // these tools must left enable
    static bool m_showDeMorgan;

    /// The current text size setting.
    static int m_textSize;

    /// Current text orientation setting.
    static int m_textOrientation;

    static wxSize m_clientSize;

    friend class DIALOG_LIB_EDIT_TEXT;

    /**
     * Function CreatePNGorJPEGFile
     * creates an image (screenshot) of the current component in PNG or JPEG format.
     * @param aFileName = the full filename
     * @param aFmt_jpeg = true to use JPEG file format, false to use PNG file format
     */
    void CreatePNGorJPEGFile( const wxString& aFileName, bool aFmt_jpeg );

    /**
     * Virtual function PrintPage
     * used to print a page
     * @param aDC = wxDC given by the calling print function
     * @param aPrintMask = not used here
     * @param aPrintMirrorMode = not used here (Set when printing in mirror mode)
     * @param aData = a pointer on an auxiliary data (not always used, NULL if not used)
     */
    virtual void PrintPage( wxDC* aDC, int aPrintMask,
                            bool aPrintMirrorMode, void* aData = NULL );

    /**
     * Function SVG_Print_Component
     * Creates the SVG print file for the current edited component.
     * @param aFullFileName = the full filename of the file
     */
    void SVG_Print_Component( const wxString& aFullFileName );


    DECLARE_EVENT_TABLE()
};

#endif  /* __LIBEDITFRM_H__ */
