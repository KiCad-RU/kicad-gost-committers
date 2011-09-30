/********************/
/*  tool_cvpcb.cpp  */
/********************/

#include "fctsys.h"
#include "appl_wxstruct.h"
#include "common.h"

#include "bitmaps.h"
#include "cvpcb.h"
#include "cvpcb_mainframe.h"
#include "cvpcb_id.h"


void CVPCB_MAINFRAME::ReCreateHToolbar()
{
    wxConfig* config = wxGetApp().m_EDA_Config;

    if( m_HToolBar != NULL )
        return;

    m_HToolBar = new EDA_TOOLBAR( TOOLBAR_MAIN, this, ID_H_TOOLBAR, TRUE );

    m_HToolBar->AddTool( ID_CVPCB_READ_INPUT_NETLIST, wxEmptyString,
                         KiBitmap( open_document_xpm ),
                         _( "Open a net list file" ) );

    m_HToolBar->AddTool( wxID_SAVE, wxEmptyString, KiBitmap( save_xpm ),
                         _( "Save net list and footprint files" ) );

    m_HToolBar->AddSeparator();
    m_HToolBar->AddTool( ID_CVPCB_CREATE_CONFIGWINDOW, wxEmptyString,
                         KiBitmap( config_xpm ),
                         _( "Configuration" ) );

    m_HToolBar->AddSeparator();
    m_HToolBar->AddTool( ID_CVPCB_CREATE_SCREENCMP, wxEmptyString,
                         KiBitmap( show_footprint_xpm ),
                         _( "View selected footprint" ) );

    m_HToolBar->AddTool( ID_CVPCB_AUTO_ASSOCIE, wxEmptyString,
                         KiBitmap( auto_associe_xpm ),
                         _( "Perform automatic footprint association" ) );

    m_HToolBar->AddSeparator();
    m_HToolBar->AddTool( ID_CVPCB_GOTO_PREVIOUSNA, wxEmptyString,
                         KiBitmap( left_xpm ),
                         _( "Select previous free component" ) );

    m_HToolBar->AddTool( ID_CVPCB_GOTO_FIRSTNA, wxEmptyString,
                         KiBitmap( right_xpm ),
                         _( "Select next free component" ) );

    m_HToolBar->AddSeparator();
    m_HToolBar->AddTool( ID_CVPCB_DEL_ASSOCIATIONS, wxEmptyString,
                         KiBitmap( delete_association_xpm ),
                         _( "Delete all associations" ) );

    m_HToolBar->AddSeparator();
    m_HToolBar->AddTool( ID_CVPCB_CREATE_STUFF_FILE, wxEmptyString,
                         KiBitmap( export_footprint_names_xpm ),
                        _( "Create export file (component/footprint list, \
used by Eeschema to fill the footprint field of components)" ) );

    m_HToolBar->AddSeparator();
    m_HToolBar->AddTool( ID_PCB_DISPLAY_FOOTPRINT_DOC, wxEmptyString,
                         KiBitmap( datasheet_xpm ),
                         _( "Display footprints list documentation" ) );

    m_HToolBar->AddSeparator();
    m_HToolBar->AddSeparator();
    m_HToolBar->AddRadioTool( ID_CVPCB_FOOTPRINT_DISPLAY_FILTERED_LIST,
                              wxEmptyString,
                              KiBitmap( module_filtered_list_xpm ),
                              wxNullBitmap,
                              _( "Display the filtered footprint list for the current component" ) );

    m_HToolBar->AddRadioTool( ID_CVPCB_FOOTPRINT_DISPLAY_FULL_LIST,
                              wxEmptyString, KiBitmap( module_full_list_xpm ),
                              wxNullBitmap,
                              _( "Display the full footprint list (without filtering)" ) );

    if( config )
    {
        wxString key = wxT( FILTERFOOTPRINTKEY );
        int      opt = config->Read( key, (long) 1 );
        m_HToolBar->ToggleTool( ID_CVPCB_FOOTPRINT_DISPLAY_FILTERED_LIST, opt );
        m_HToolBar->ToggleTool( ID_CVPCB_FOOTPRINT_DISPLAY_FULL_LIST, !opt );
    }

    // after adding the buttons to the toolbar, must call Realize() to reflect the changes
    m_HToolBar->Realize();
}
