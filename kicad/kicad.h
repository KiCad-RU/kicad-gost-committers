/**
 * @file kicad/kicad.h
 * @brief KICAD_MANAGER_FRAME is the KiCad main frame.
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN (www.cern.ch)
 * Copyright (C) 2013 KiCad Developers, see CHANGELOG.txt for contributors.
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

#ifndef KICAD_H
#define KICAD_H

#include <vector>

#include <wx/treectrl.h>
#include <wx/dragimag.h>
#include <wx/filename.h>
#include <wx/process.h>

#include <id.h>
#include <wxstruct.h>
#include <appl_wxstruct.h>

// With a recent wxWidget, we can use the wxFileSystemWatcherEvent
// to monitor files add/remove/rename in tree project
#if wxCHECK_VERSION( 2, 9, 4  )
#define KICAD_USE_FILES_WATCHER
#endif

extern const wxString g_KicadPrjFilenameExtension;

class RIGHT_KM_FRAME;
class TREEPROJECTFILES;
class TREE_PROJECT_FRAME;

// Enum to identify the type of files handled by Kicad manager
//
// When changing this enum  please verify (and perhaps update)
// TREE_PROJECT_FRAME::GetFileExt(),
// TREE_PROJECT_FRAME::GetFileExt()
// s_AllowedExtensionsToList[]

enum TreeFileType {
    TREE_PROJECT = 1,
    TREE_SCHEMA,        // Schematic file (.sch)
    TREE_LEGACY_PCB,    // board file (.brd) legacy format
    TREE_SEXP_PCB,      // board file (.kicad_brd) new s expression format
    TREE_GERBER,        // Gerber  file (.pho, .g*)
    TREE_PDF,           // PDF file (.pdf)
    TREE_TXT,           // ascii text file (.txt)
    TREE_NET,           // netlist file (.net)
    TREE_UNKNOWN,
    TREE_DIRECTORY,
    TREE_CMP_LINK,      // cmp/footprint link file (.cmp)
    TREE_REPORT,        // report file (.rpt)
    TREE_FP_PLACE,      // fooprints position (place) file (.pos)
    TREE_DRILL,         // Excellon drill file (.drl)
    TREE_SVG,           // SVG file (.svg)
    TREE_MAX
};


/**
 * Command IDs for KiCad.
 *
 * Please add IDs that are unique to Kicad  here and not in the global id.h
 * file.  This will prevent the entire project from being rebuilt when adding
 * new commands to KiCad.
 */

enum id_kicad_frm {
    ID_LEFT_FRAME = ID_END_LIST,
    ID_PROJECT_TREE,
    ID_PROJECT_TXTEDIT,
    ID_PROJECT_TREE_REFRESH,
    ID_PROJECT_NEWDIR,
    ID_PROJECT_DELETE,
    ID_PROJECT_RENAME,
    ID_PROJECT_OPEN_FILE_WITH_TEXT_EDITOR,

    ID_TO_EDITOR,
    ID_TO_EESCHEMA,
    ID_TO_GERBVIEW,
    ID_TO_BITMAP_CONVERTER,
    ID_TO_PCB_CALCULATOR,
    ID_BROWSE_AN_SELECT_FILE,
    ID_SELECT_PREFERED_EDITOR,
    ID_SELECT_PREFERED_PDF_BROWSER_NAME,
    ID_SELECT_PREFERED_PDF_BROWSER,
    ID_SELECT_DEFAULT_PDF_BROWSER,
    ID_SAVE_AND_ZIP_FILES,
    ID_READ_ZIP_ARCHIVE,
    ID_INIT_WATCHED_PATHS
};


/* class KICAD_MANAGER_FRAME
 * This is the main KiCad frame
 */
class KICAD_MANAGER_FRAME : public EDA_BASE_FRAME
{
public:
    TREE_PROJECT_FRAME* m_LeftWin;
    RIGHT_KM_FRAME*     m_RightWin;
    wxAuiToolBar*       m_VToolBar;  // Vertical toolbar (not used)
    wxString            m_BoardFileName;
    wxString            m_SchematicRootFileName;
    wxFileName          m_ProjectFileName;

private:
    int m_LeftWin_Width;

public: KICAD_MANAGER_FRAME( wxWindow* parent, const wxString& title,
                             const wxPoint& pos, const wxSize& size );

    ~KICAD_MANAGER_FRAME();

    void OnCloseWindow( wxCloseEvent& Event );
    void OnSize( wxSizeEvent& event );
    void OnSashDrag( wxSashEvent& event );

    /**
     * Function OnLoadProject
     * loads an exiting or creates a new project (.pro) file.
     */
    void OnLoadProject( wxCommandEvent& event );

    /**
     * Function OnSaveProject
     * is the command event hendler to Save the project (.pro) file containing the top level
     * configuration parameters.
     */
    void OnSaveProject( wxCommandEvent& event );

    void OnArchiveFiles( wxCommandEvent& event );
    void OnUnarchiveFiles( wxCommandEvent& event );
    void OnRunPcbNew( wxCommandEvent& event );
    void OnRunCvpcb( wxCommandEvent& event );
    void OnRunEeschema( wxCommandEvent& event );
    void OnRunGerbview( wxCommandEvent& event );
    void OnRunBitmapConverter( wxCommandEvent& event );
    void OnRunPcbCalculator( wxCommandEvent& event );

    void OnOpenTextEditor( wxCommandEvent& event );
    void OnOpenFileInTextEditor( wxCommandEvent& event );
    void OnOpenFileInEditor( wxCommandEvent& event );

    void OnFileHistory( wxCommandEvent& event );
    void OnExit( wxCommandEvent& event );
    void Process_Preferences( wxCommandEvent& event );
    void ReCreateMenuBar();
    void RecreateBaseHToolbar();

    /**
     * Function PrintMsg
     * displays \a aText in the text panel.
     *
     * @param aText The text to display.
     */
    void PrintMsg( const wxString& aText );

    void ClearMsg();
    void SetLanguage( wxCommandEvent& event );
    void OnRefresh( wxCommandEvent& event );
    void OnSelectDefaultPdfBrowser( wxCommandEvent& event );
    void OnSelectPreferredPdfBrowser( wxCommandEvent& event );

    void OnUpdateDefaultPdfBrowser( wxUpdateUIEvent& event );
    void OnUpdatePreferredPdfBrowser( wxUpdateUIEvent& event );

    void CreateNewProject( const wxString aPrjFullFileName, bool aTemplateSelector );

    /**
     * Function LoadSettings
     * loads the KiCad main frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings will not get loaded.
     */
    void LoadSettings();

    /**
     * Function SaveSettings
     * saves the KiCad main frame specific configuration settings.
     *
     * Don't forget to call this base method from any derived classes or the
     * settings will not get saved.
     */
    void SaveSettings();

    /**
     * Function Execute
     * opens another KiCad application and logs a message.
     * @param frame = owner frame.
     * @param execFile = name of the executable file.
     * @param param = parameters to be passed to the executable.
     */
    void Execute( wxWindow* frame, const wxString& execFile,
                  const wxString& param = wxEmptyString );

    class PROCESS_TERMINATE_EVENT_HANDLER : public wxProcess
    {
    private:
        wxString appName;

    public:
        PROCESS_TERMINATE_EVENT_HANDLER( const wxString& appName ) :
            appName(appName)
        {
        }

        void OnTerminate( int pid, int status );
    };

#ifdef KICAD_USE_FILES_WATCHER
    /**
     * Called by sending a event with id = ID_INIT_WATCHED_PATHS
     * rebuild the list of wahtched paths
     */
    void OnChangeWatchedPaths(wxCommandEvent& aEvent );
#endif

    DECLARE_EVENT_TABLE()
};


/** class RIGHT_KM_FRAME
 */
class RIGHT_KM_FRAME : public wxSashLayoutWindow
{
public:
    wxTextCtrl*          m_MessagesBox;
private:
    KICAD_MANAGER_FRAME* m_Parent;          // a wxTextCtrl to displays messages frm KiCad
    int m_ButtonsPanelHeight;
    wxPanel*             m_ButtPanel;
    int     m_ButtonSeparation;             // button distance in pixels
    wxPoint m_ButtonsListPosition;          /* position of the left bottom corner
                                             *  of the first bitmap button
                                             */
    wxPoint m_ButtonLastPosition;           // position of the last button in the window
    int     m_bitmapButtons_maxHeigth;      // height of bigger bitmap buttons
                                            // Used to calculate the height of the panel.

public: RIGHT_KM_FRAME( KICAD_MANAGER_FRAME* parent );
    ~RIGHT_KM_FRAME() { };
    void            OnSize( wxSizeEvent& event );

private:

    /**
     * Function CreateCommandToolbar
     * creates the main tool bar buttons (fast launch buttons)
     */
    void            CreateCommandToolbar( void );

    wxBitmapButton* AddBitmapButton( wxWindowID aId, const wxBitmap& aBitmap );

    DECLARE_EVENT_TABLE()
};

#endif
