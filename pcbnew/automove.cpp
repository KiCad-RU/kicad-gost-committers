/****************************************************************/
/* Routines for automatic displacement and rotation of modules. */
/****************************************************************/

#include <algorithm>

#include "fctsys.h"
#include "class_drawpanel.h"
#include "confirm.h"
#include "kicad_string.h"
#include "pcbnew.h"
#include "wxPcbStruct.h"
#include "kicad_device_context.h"

#include "autorout.h"
#include "cell.h"
#include "pcbnew_id.h"
#include "class_board.h"
#include "class_module.h"


typedef enum {
    FIXE_MODULE,
    FREE_MODULE,
    FIXE_ALL_MODULES,
    FREE_ALL_MODULES
} SelectFixeFct;


static bool sortModulesbySize( MODULE* ref, MODULE* compare );


wxString ModulesMaskSelection = wxT( "*" );


/* Called on events (popup menus) relative to automove and autoplace footprints
 */
void PCB_EDIT_FRAME::AutoPlace( wxCommandEvent& event )
{
    int        id = event.GetId();
    bool       on_state;

    if( m_HToolBar == NULL )
        return;

    INSTALL_UNBUFFERED_DC( dc, DrawPanel );

    switch( id )
    {
    case ID_TOOLBARH_PCB_MODE_MODULE:
        on_state = m_HToolBar->GetToolState( ID_TOOLBARH_PCB_MODE_MODULE );

        if( on_state )
        {
            m_HToolBar->ToggleTool( ID_TOOLBARH_PCB_MODE_TRACKS, false );
            m_HTOOL_current_state = ID_TOOLBARH_PCB_MODE_MODULE;
        }
        else
        {
            m_HTOOL_current_state = 0;
        }

        return;

    case ID_TOOLBARH_PCB_MODE_TRACKS:
        on_state = m_HToolBar->GetToolState( ID_TOOLBARH_PCB_MODE_TRACKS );

        if( on_state )
        {
            m_HToolBar->ToggleTool( ID_TOOLBARH_PCB_MODE_MODULE, false );
            m_HTOOL_current_state = ID_TOOLBARH_PCB_MODE_TRACKS;
        }
        else
        {
            m_HTOOL_current_state = 0;
        }

        return;


    case ID_POPUP_PCB_AUTOROUTE_SELECT_LAYERS:
        return;

    case ID_POPUP_PCB_AUTOPLACE_FIXE_MODULE:
        LockModule( (MODULE*) GetScreen()->GetCurItem(), true );
        return;

    case ID_POPUP_PCB_AUTOPLACE_FREE_MODULE:
        LockModule( (MODULE*) GetScreen()->GetCurItem(), false );
        return;

    case ID_POPUP_PCB_AUTOPLACE_FREE_ALL_MODULES:
        LockModule( NULL, false );
        return;

    case ID_POPUP_PCB_AUTOPLACE_FIXE_ALL_MODULES:
        LockModule( NULL, true );
        return;

    case ID_POPUP_CANCEL_CURRENT_COMMAND:
        if( DrawPanel->IsMouseCaptured() )
        {
            DrawPanel->m_endMouseCaptureCallback( DrawPanel, &dc );
        }

        break;

    default:   // Abort a current command (if any)
        DrawPanel->EndMouseCapture( ID_NO_TOOL_SELECTED, DrawPanel->GetDefaultCursor() );
        break;
    }

    /* Erase ratsnest if needed */
    if( GetBoard()->IsElementVisible(RATSNEST_VISIBLE) )
        DrawGeneralRatsnest( &dc );

    GetBoard()->m_Status_Pcb |= DO_NOT_SHOW_GENERAL_RASTNEST;

    switch( id )
    {
    case ID_POPUP_PCB_AUTOPLACE_CURRENT_MODULE:
        AutoPlaceModule( (MODULE*) GetScreen()->GetCurItem(), PLACE_1_MODULE, &dc );
        break;

    case ID_POPUP_PCB_AUTOPLACE_ALL_MODULES:
        AutoPlaceModule( NULL, PLACE_ALL, &dc );
        break;

    case ID_POPUP_PCB_AUTOPLACE_NEW_MODULES:
        AutoPlaceModule( NULL, PLACE_OUT_OF_BOARD, &dc );
        break;

    case ID_POPUP_PCB_AUTOPLACE_NEXT_MODULE:
        AutoPlaceModule( NULL, PLACE_INCREMENTAL, &dc );
        break;

    case ID_POPUP_PCB_AUTOMOVE_ALL_MODULES:
        AutoMoveModulesOnPcb( false );
        break;

    case ID_POPUP_PCB_AUTOMOVE_NEW_MODULES:
        AutoMoveModulesOnPcb( true );
        break;

    case ID_POPUP_PCB_AUTOROUTE_ALL_MODULES:
        Autoroute( &dc, ROUTE_ALL );
        break;

    case ID_POPUP_PCB_AUTOROUTE_MODULE:
        Autoroute( &dc, ROUTE_MODULE );
        break;

    case ID_POPUP_PCB_AUTOROUTE_PAD:
        Autoroute( &dc, ROUTE_PAD );
        break;

    case ID_POPUP_PCB_AUTOROUTE_NET:
        Autoroute( &dc, ROUTE_NET );
        break;

    case ID_POPUP_PCB_AUTOROUTE_RESET_UNROUTED:
        Reset_Noroutable( &dc );
        break;

    default:
        wxMessageBox( wxT( "AutoPlace command error" ) );
        break;
    }

    GetBoard()->m_Status_Pcb &= ~DO_NOT_SHOW_GENERAL_RASTNEST;
    Compile_Ratsnest( &dc, true );
}


/* Function to move components in a rectangular area format 4 / 3,
 * starting from the mouse cursor
 * The components with the FIXED status set are not moved
 */
void PCB_EDIT_FRAME::AutoMoveModulesOnPcb( bool PlaceModulesHorsPcb )
{
    std::vector <MODULE*> moduleList;
    wxPoint  start, current;
    int      Ymax_size, Xsize_allowed;
    int      pas_grille = (int) GetScreen()->GetGridSize().x;
    bool     edgesExists;
    double   surface;

    if( GetBoard()->m_Modules == NULL )
    {
        DisplayError( this, _( "No modules found!" ) );
        return;
    }

    /* Confirmation */
    if( !IsOK( this, _( "Move modules?" ) ) )
        return;

    edgesExists = GetBoard()->ComputeBoundingBox( true );

    if( PlaceModulesHorsPcb && !edgesExists )
    {
        DisplayError( this,
                      _( "Could not automatically place modules. No board outlines detected." ) );
        return;
    }

    // Build sorted footprints list (sort by decreasing size )
    MODULE* Module = GetBoard()->m_Modules;

    for( ; Module != NULL; Module = Module->Next() )
    {
        Module->CalculateBoundingBox();
        moduleList.push_back(Module);
    }

    sort( moduleList.begin(), moduleList.end(), sortModulesbySize );

    /* to move modules outside the board, the cursor is placed below
     * the current board, to avoid placing components in board area.
     */
    if( PlaceModulesHorsPcb && edgesExists )
    {
        if( GetScreen()->GetCrossHairPosition().y < (GetBoard()->m_BoundaryBox.GetBottom() + 2000) )
        {
            wxPoint pos = GetScreen()->GetCrossHairPosition();
            pos.y = GetBoard()->m_BoundaryBox.GetBottom() + 2000;
            GetScreen()->SetCrossHairPosition( pos );
        }
    }

    /* calculate the area needed by footprints */
    surface = 0.0;

    for( unsigned ii = 0; ii < moduleList.size(); ii++ )
    {
        Module = moduleList[ii];

        if( PlaceModulesHorsPcb && edgesExists )
        {
            if( GetBoard()->m_BoundaryBox.Contains( Module->m_Pos ) )
                continue;
        }

        surface += Module->m_Surface;
    }

    Xsize_allowed = (int) ( sqrt( surface ) * 4.0 / 3.0 );

    start     = current = GetScreen()->GetCrossHairPosition();
    Ymax_size = 0;

    for( unsigned ii = 0; ii < moduleList.size(); ii++ )
    {
        Module = moduleList[ii];

        if( Module->IsLocked() )
            continue;

        if( PlaceModulesHorsPcb && edgesExists )
        {
            if( GetBoard()->m_BoundaryBox.Contains( Module->m_Pos ) )
                continue;
        }

        if( current.x > (Xsize_allowed + start.x) )
        {
            current.x  = start.x;
            current.y += Ymax_size + pas_grille;
            Ymax_size  = 0;
        }

        GetScreen()->SetCrossHairPosition( current + Module->m_Pos -
                                           Module->m_BoundaryBox.GetPosition() );

        Ymax_size = MAX( Ymax_size, Module->m_BoundaryBox.GetHeight() );

        PlaceModule( Module, NULL, true );

        current.x += Module->m_BoundaryBox.GetWidth() + pas_grille;
    }

    DrawPanel->Refresh();
}


/* Set or reset (true or false) Lock attribute of aModule or all modules if aModule == NULL
 */
void PCB_EDIT_FRAME::LockModule( MODULE* aModule, bool aLocked )
{
    if( aModule )
    {
        aModule->SetLocked( aLocked );

        aModule->DisplayInfo( this );
        OnModify();
    }
    else
    {
        aModule = GetBoard()->m_Modules;

        for( ; aModule != NULL; aModule = aModule->Next() )
        {
            if( WildCompareString( ModulesMaskSelection, aModule->m_Reference->m_Text ) )
            {
                aModule->SetLocked( aLocked );
                OnModify();
            }
        }
    }
}


static bool sortModulesbySize( MODULE* ref, MODULE* compare )
{
    return compare->m_Surface < ref->m_Surface;
}
