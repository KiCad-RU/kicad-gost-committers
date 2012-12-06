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
 * @file viewlib_frame.h
 */

#ifndef LIBVIEWFRM_H_
#define LIBVIEWFRM_H_


#include <wx/gdicmn.h>

#include <sch_base_frame.h>
#include <class_sch_screen.h>

class wxSashLayoutWindow;
class wxListBox;
class wxSemaphore;
class CMP_LIBRARY;


/**
 * Component library viewer main window.
 */
class LIB_VIEW_FRAME : public SCH_BASE_FRAME
{
private:
    wxComboBox*         SelpartBox;

    // List of libraries (for selection )
    wxSashLayoutWindow* m_LibListWindow;
    wxListBox*          m_LibList;          // The list of libs
    wxSize              m_LibListSize;      // size of the window

    // List of components in the selected library
    wxSashLayoutWindow* m_CmpListWindow;
    wxListBox*          m_CmpList;          // The list of components
    wxSize              m_CmpListSize;      // size of the window

    // Flags
    wxSemaphore*        m_Semaphore;        // != NULL if the frame must emulate a modal dialog
    wxString            m_configPath;       // subpath for configuration

protected:
    static wxString m_libraryName;
    static wxString m_entryName;
    static wxString m_exportToEeschemaCmpName;  // When the viewer is used to select a component
                                                // in schematic, the selected component is here
    static int      m_unit;
    static int      m_convert;

public:
    LIB_VIEW_FRAME( SCH_BASE_FRAME* aParent, CMP_LIBRARY* aLibrary = NULL,
                    wxSemaphore* aSemaphore = NULL,
                    long aStyle = KICAD_DEFAULT_DRAWFRAME_STYLE );

    ~LIB_VIEW_FRAME();

    /**
     * Function GetLibViewerFrameName (static)
     * @return the frame name used when creating the frame
     * used to get a reference to this frame, if exists
     */
    static const wxChar* GetLibViewerFrameName();

    /**
     * Function GetActiveLibraryViewer (static)
     * @return a reference to the current opened Library viewer
     * or NULL if no Library viewer currently opened
     */
    static LIB_VIEW_FRAME* GetActiveLibraryViewer();

    void OnSize( wxSizeEvent& event );

    /**
     * Function OnSashDrag
     * resizes the child windows when dragging a sash window border.
     */
    void OnSashDrag( wxSashEvent& event );

    /**
     * Function ReCreateListLib
     *
     * Creates or recreates the list of current loaded libraries.
     * This list is sorted, with the library cache always at end of the list
     */
    void ReCreateListLib();

    void ReCreateListCmp();
    void Process_Special_Functions( wxCommandEvent& event );
    void DisplayLibInfos();
    void RedrawActiveWindow( wxDC* DC, bool EraseBg );
    void OnCloseWindow( wxCloseEvent& Event );
    void ReCreateHToolbar();
    void ReCreateVToolbar();
    void OnLeftClick( wxDC* DC, const wxPoint& MousePos );
    double BestZoom();
    void ClickOnLibList( wxCommandEvent& event );
    void ClickOnCmpList( wxCommandEvent& event );
    void OnSetRelativeOffset( wxCommandEvent& event );

    void GeneralControl( wxDC* aDC, const wxPoint& aPosition, int aHotKey = 0 );

    /**
     * Function LoadSettings
     * loads the library viewer frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings will not get loaded.
     */
    void LoadSettings();

    /**
     * Function SaveSettings
     * save library viewer frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings will not get saved.
     */
    void SaveSettings();

    wxString& GetEntryName( void ) const { return m_entryName; }
    wxString& GetSelectedComponent( void ) const { return m_exportToEeschemaCmpName; }

    static int  GetUnit( void ) { return m_unit; }
    static int  GetConvert( void ) { return m_convert; }

private:
    /**
     * Function OnActivate
     * is called when the frame frame is activate to reload the libraries and component lists
     * that can be changed by the schematic editor or the library editor.
     */
    virtual void OnActivate( wxActivateEvent& event );

    void SelectCurrentLibrary();
    void SelectAndViewLibraryPart( int option );

    /**
     * Function ExportToSchematicLibraryPart
     * exports the current component to schematic and close the library browser.
     */
    void ExportToSchematicLibraryPart( wxCommandEvent& event );
    void ViewOneLibraryContent( CMP_LIBRARY* Lib, int Flag );
    bool OnRightClick( const wxPoint& MousePos, wxMenu* PopMenu );
    void DClickOnCmpList( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};

#endif  // LIBVIEWFRM_H_
