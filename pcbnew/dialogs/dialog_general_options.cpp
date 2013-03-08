/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file dialog_general_options.cpp
 */

/* functions relatives to the dialogs opened from the main menu :
 *   Preferences/general
 *   Preferences/display
 */
#include <fctsys.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <pcbnew.h>
#include <wxPcbStruct.h>
#include <class_board_design_settings.h>
#include <pcbcommon.h>

#include <pcbnew_id.h>
#include <class_board.h>

#include <dialog_general_options.h>


DIALOG_GENERALOPTIONS::DIALOG_GENERALOPTIONS( PCB_EDIT_FRAME* parent ) :
    DIALOG_GENERALOPTIONS_BOARDEDITOR_BASE( parent )
{
    init();

    GetSizer()->SetSizeHints( this );
    Center();
}


void DIALOG_GENERALOPTIONS::init()
{
    SetFocus();
    m_sdbSizerOK->SetDefault();

    m_Board = GetParent()->GetBoard();

    /* Set display options */
    m_PolarDisplay->SetSelection( DisplayOpt.DisplayPolarCood ? 1 : 0 );
    m_UnitsSelection->SetSelection( g_UserUnit ? 1 : 0 );
    m_CursorShape->SetSelection( GetParent()->GetCursorShape() ? 1 : 0 );


    switch( g_RotationAngle )
    {
    case 450:
        m_RotationAngle->SetSelection( 0 );
        break;

    default:
        m_RotationAngle->SetSelection( 1 );
    }

    wxString timevalue;
    timevalue << GetParent()->GetAutoSaveInterval() / 60;
    m_SaveTime->SetValue( timevalue );
    m_MaxShowLinks->SetValue( g_MaxLinksShowed );

    m_DrcOn->SetValue( Drc_On );
    m_ShowModuleRatsnest->SetValue( g_Show_Module_Ratsnest );
    m_ShowGlobalRatsnest->SetValue( m_Board->IsElementVisible( RATSNEST_VISIBLE ) );
    m_TrackAutodel->SetValue( g_AutoDeleteOldTrack );
    m_Track_45_Only_Ctrl->SetValue( g_Track_45_Only_Allowed );
    m_Segments_45_Only_Ctrl->SetValue( Segments_45_Only );
    m_ZoomNoCenterOpt->SetValue( GetParent()->GetCanvas()->GetEnableZoomNoCenter() );
    m_MiddleButtonPANOpt->SetValue( GetParent()->GetCanvas()->GetEnableMiddleButtonPan() );
    m_OptMiddleButtonPanLimited->SetValue( GetParent()->GetCanvas()->GetMiddleButtonPanLimited() );
    m_OptMiddleButtonPanLimited->Enable( m_MiddleButtonPANOpt->GetValue() );
    m_AutoPANOpt->SetValue( GetParent()->GetCanvas()->GetEnableAutoPan() );
    m_Segments_45_Only_Ctrl->SetValue( Segments_45_Only );
    m_Track_DoubleSegm_Ctrl->SetValue( g_TwoSegmentTrackBuild );

    m_MagneticPadOptCtrl->SetSelection( g_MagneticPadOption );
    m_MagneticTrackOptCtrl->SetSelection( g_MagneticTrackOption );
}


void DIALOG_GENERALOPTIONS::OnCancelClick( wxCommandEvent& event )
{
    event.Skip();
}


void DIALOG_GENERALOPTIONS::OnOkClick( wxCommandEvent& event )
{
    EDA_UNITS_T ii;

    DisplayOpt.DisplayPolarCood = ( m_PolarDisplay->GetSelection() == 0 ) ? false : true;
    ii = g_UserUnit;
    g_UserUnit = ( m_UnitsSelection->GetSelection() == 0 )  ? INCHES : MILLIMETRES;

    if( ii != g_UserUnit )
        GetParent()->ReCreateAuxiliaryToolbar();

    GetParent()->SetCursorShape( m_CursorShape->GetSelection() );
    GetParent()->SetAutoSaveInterval( m_SaveTime->GetValue() * 60 );

    g_RotationAngle = 10 * wxAtoi( m_RotationAngle->GetStringSelection() );

    /* Updating the combobox to display the active layer. */
    g_MaxLinksShowed = m_MaxShowLinks->GetValue();
    Drc_On = m_DrcOn->GetValue();

    if( m_Board->IsElementVisible(RATSNEST_VISIBLE) != m_ShowGlobalRatsnest->GetValue() )
    {
        GetParent()->SetElementVisibility(RATSNEST_VISIBLE, m_ShowGlobalRatsnest->GetValue() );
        GetParent()->GetCanvas()->Refresh( );
    }

    g_Show_Module_Ratsnest = m_ShowModuleRatsnest->GetValue();
    g_AutoDeleteOldTrack   = m_TrackAutodel->GetValue();
    Segments_45_Only = m_Segments_45_Only_Ctrl->GetValue();
    g_Track_45_Only_Allowed    = m_Track_45_Only_Ctrl->GetValue();

    GetParent()->GetCanvas()->SetEnableZoomNoCenter( m_ZoomNoCenterOpt->GetValue() );
    GetParent()->GetCanvas()->SetEnableMiddleButtonPan( m_MiddleButtonPANOpt->GetValue() );
    GetParent()->GetCanvas()->SetMiddleButtonPanLimited( m_OptMiddleButtonPanLimited->GetValue() );

    GetParent()->GetCanvas()->SetEnableAutoPan( m_AutoPANOpt->GetValue() );
    g_TwoSegmentTrackBuild = m_Track_DoubleSegm_Ctrl->GetValue();
    g_MagneticPadOption   = m_MagneticPadOptCtrl->GetSelection();
    g_MagneticTrackOption = m_MagneticTrackOptCtrl->GetSelection();

    EndModal( wxID_OK );
}


/* Must be called on a click on the left toolbar (options toolbar
 * Update variables according to tools states
 */
void PCB_EDIT_FRAME::OnSelectOptionToolbar( wxCommandEvent& event )
{
    int id = event.GetId();
    bool state = event.IsChecked();

    switch( id )
    {
    case ID_TB_OPTIONS_DRC_OFF:
        Drc_On = !state;

        if( GetToolId() == ID_TRACK_BUTT )
        {
            if( Drc_On )
                m_canvas->SetCursor( wxCURSOR_PENCIL );
            else
                m_canvas->SetCursor( wxCURSOR_QUESTION_ARROW );
        }
        break;

    case ID_TB_OPTIONS_SHOW_RATSNEST:
        SetElementVisibility( RATSNEST_VISIBLE, state );

        if( state && (GetBoard()->m_Status_Pcb & LISTE_RATSNEST_ITEM_OK) == 0 )
            Compile_Ratsnest( NULL, true );

        m_canvas->Refresh();
        break;

    case ID_TB_OPTIONS_SHOW_MODULE_RATSNEST:
        g_Show_Module_Ratsnest = state; // TODO: use the visibility list
        break;

    case ID_TB_OPTIONS_AUTO_DEL_TRACK:
        g_AutoDeleteOldTrack = state;
        break;

    case ID_TB_OPTIONS_SHOW_ZONES:
        DisplayOpt.DisplayZonesMode = 0;
        m_canvas->Refresh();
        break;

    case ID_TB_OPTIONS_SHOW_ZONES_DISABLE:
        DisplayOpt.DisplayZonesMode = 1;
        m_canvas->Refresh();
        break;

    case ID_TB_OPTIONS_SHOW_ZONES_OUTLINES_ONLY:
        DisplayOpt.DisplayZonesMode = 2;
        m_canvas->Refresh();
        break;

    case ID_TB_OPTIONS_SHOW_VIAS_SKETCH:
        m_DisplayViaFill = DisplayOpt.DisplayViaFill = !state;
        m_canvas->Refresh();
        break;

    case ID_TB_OPTIONS_SHOW_TRACKS_SKETCH:
        m_DisplayPcbTrackFill = DisplayOpt.DisplayPcbTrackFill = !state;
        m_canvas->Refresh();
        break;

    case ID_TB_OPTIONS_SHOW_HIGH_CONTRAST_MODE:
        DisplayOpt.ContrastModeDisplay = state;
        m_canvas->Refresh();
        break;

    case ID_TB_OPTIONS_SHOW_EXTRA_VERTICAL_TOOLBAR_MICROWAVE:
        m_show_microwave_tools = state;
        m_auimgr.GetPane( wxT( "m_microWaveToolBar" ) ).Show( m_show_microwave_tools );
        m_auimgr.Update();
        break;

    case ID_TB_OPTIONS_SHOW_MANAGE_LAYERS_VERTICAL_TOOLBAR:
        // show auxiliary Vertical layers and visibility manager toolbar
        m_show_layer_manager_tools = state;
        m_auimgr.GetPane( wxT( "m_LayersManagerToolBar" ) ).Show( m_show_layer_manager_tools );
        m_auimgr.Update();

        GetMenuBar()->SetLabel( ID_MENU_PCB_SHOW_HIDE_LAYERS_MANAGER_DIALOG,
                                m_show_layer_manager_tools ?
                                _( "Hide &Layers Manager" ) : _( "Show &Layers Manager" ) );
        break;

    default:
        DisplayError( this,
                      wxT( "PCB_EDIT_FRAME::OnSelectOptionToolbar error \n (event not handled!)" ) );
        break;
    }
}
