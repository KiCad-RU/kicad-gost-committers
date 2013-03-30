/**
 * @file dialog_copper_zones.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
  * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <wx/wx.h>
#include <fctsys.h>
#include <appl_wxstruct.h>
#include <confirm.h>
#include <PolyLine.h>
#include <pcbnew.h>
#include <wxPcbStruct.h>
#include <zones.h>
#include <base_units.h>

#include <class_zone.h>
#include <class_board.h>
#include <dialog_copper_zones_base.h>

#include <wx/imaglist.h>    // needed for wx/listctrl.h, in wxGTK 2.8.12
#include <wx/listctrl.h>



/**
 * Class DIALOG_COPPER_ZONE
 * is the derived class from dialog_copper_zone_frame created by wxFormBuilder
 */
class DIALOG_COPPER_ZONE : public DIALOG_COPPER_ZONE_BASE
{
public:
    DIALOG_COPPER_ZONE( PCB_BASE_FRAME* aParent, ZONE_SETTINGS* aSettings );

private:
    PCB_BASE_FRAME* m_Parent;
    wxConfig*       m_Config;               ///< Current config

    ZONE_EDIT_T     m_OnExitCode;           ///< exit code: ZONE_ABORT if no change,
                                            ///< ZONE_OK if new values accepted
                                            ///< ZONE_EXPORT_VALUES if values are exported to others zones

    ZONE_SETTINGS   m_settings;
    ZONE_SETTINGS*  m_ptr;

    bool            m_NetSortingByPadCount; ///< false = alphabetic sort.
                                            ///< true = pad count sort.

    long            m_NetFiltering;
    std::vector<int> m_LayerId;             ///< Handle the real layer number from layer
                                            ///< name position in m_LayerSelectionCtrl

    static wxString m_netNameShowFilter;    ///< the filter to show nets (default * "*").
                                            ///< static to keep this pattern for an entire pcbnew session

    /**
     * Function initDialog
     * fills in the dialog controls using the current settings.
     */
    void initDialog();

    void OnButtonOkClick( wxCommandEvent& event );
    void OnButtonCancelClick( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );
    void OnCornerSmoothingModeChoice( wxCommandEvent& event );

    /**
     * Function AcceptOptions
     * @param aPromptForErrors is true to prompt user on incorrect params.
     * @param aUseExportableSetupOnly is true to use exportable parametres only (used to export this setup to other zones).
     * @return bool - false if incorrect options, true if ok.
     */
    bool AcceptOptions( bool aPromptForErrors, bool aUseExportableSetupOnly = false );

    void OnNetSortingOptionSelected( wxCommandEvent& event );
    void ExportSetupToOtherCopperZones( wxCommandEvent& event );
    void OnPadsInZoneClick( wxCommandEvent& event );
    void OnRunFiltersButtonClick( wxCommandEvent& event );


    void buildAvailableListOfNets();

    /**
     * Function initListNetsParams
     * initializes m_NetSortingByPadCount and m_NetFiltering values
     * according to m_NetDisplayOption selection.
     */
    void initListNetsParams();

    /**
     * Function makeLayerBitmap
     * creates the colored rectangle bitmaps used in the layer selection widget.
     * @param aColor is the color to fill the rectangle with.
     */
    wxBitmap makeLayerBitmap( EDA_COLOR_T aColor );
};


#define LAYER_BITMAP_SIZE_X     20
#define LAYER_BITMAP_SIZE_Y     10

// Initialize static member variables
wxString DIALOG_COPPER_ZONE::m_netNameShowFilter( wxT( "*" ) );


ZONE_EDIT_T InvokeCopperZonesEditor( PCB_BASE_FRAME* aCaller, ZONE_SETTINGS* aSettings )
{
    DIALOG_COPPER_ZONE dlg( aCaller, aSettings );

    ZONE_EDIT_T result = ZONE_EDIT_T( dlg.ShowModal() );
    return result;
}


DIALOG_COPPER_ZONE::DIALOG_COPPER_ZONE( PCB_BASE_FRAME* aParent, ZONE_SETTINGS* aSettings ) :
    DIALOG_COPPER_ZONE_BASE( aParent )
{
    m_Parent = aParent;
    m_Config = wxGetApp().GetSettings();

    m_ptr = aSettings;
    m_settings = *aSettings;

    m_NetSortingByPadCount = true;      // false = alphabetic sort, true = pad count sort
    m_OnExitCode = ZONE_ABORT;

    SetReturnCode( ZONE_ABORT );        // Will be changed on buttons click

    // Fix static text widget minimum width to a suitable value so that
    // resizing the dialog is not necessary when changing the corner smoothing type.
    // Depends on the default text in the widget.
    m_cornerSmoothingTitle->SetMinSize( m_cornerSmoothingTitle->GetSize() );

    initDialog();

    GetSizer()->SetSizeHints( this );
    Center();
}


void DIALOG_COPPER_ZONE::initDialog()
{
    BOARD* board = m_Parent->GetBoard();

    wxString msg;

    if( m_settings.m_Zone_45_Only )
        m_OrientEdgesOpt->SetSelection( 1 );

    m_FillModeCtrl->SetSelection( m_settings.m_FillMode ? 1 : 0 );

    AddUnitSymbol( *m_ClearanceValueTitle, g_UserUnit );
    msg = ReturnStringFromValue( g_UserUnit, m_settings.m_ZoneClearance );
    m_ZoneClearanceCtrl->SetValue( msg );

    AddUnitSymbol( *m_MinThicknessValueTitle, g_UserUnit );
    msg = ReturnStringFromValue( g_UserUnit, m_settings.m_ZoneMinThickness );
    m_ZoneMinThicknessCtrl->SetValue( msg );

    switch( m_settings.GetPadConnection() )
    {
    case THT_THERMAL:               // Thermals only for THT pads
        m_PadInZoneOpt->SetSelection( 2 );
        break;

    case PAD_NOT_IN_ZONE:           // Pads are not covered
        m_PadInZoneOpt->SetSelection( 3 );
        break;

    default:
    case THERMAL_PAD:               // Use thermal relief for pads
        m_PadInZoneOpt->SetSelection( 1 );
        break;

    case PAD_IN_ZONE:               // pads are covered by copper
        m_PadInZoneOpt->SetSelection( 0 );
        break;
    }

    // Antipad and spokes are significant only for thermals
    if( m_settings.GetPadConnection() != THERMAL_PAD &&
        m_settings.GetPadConnection() != THT_THERMAL )
    {
        m_AntipadSizeValue->Enable( false );
        m_CopperWidthValue->Enable( false );
    }
    else
    {
        m_AntipadSizeValue->Enable( true );
        m_CopperWidthValue->Enable( true );
    }

    m_PriorityLevelCtrl->SetValue( m_settings.m_ZonePriority );

    AddUnitSymbol( *m_AntipadSizeText, g_UserUnit );
    AddUnitSymbol( *m_CopperBridgeWidthText, g_UserUnit );
    PutValueInLocalUnits( *m_AntipadSizeValue, m_settings.m_ThermalReliefGap );
    PutValueInLocalUnits( *m_CopperWidthValue, m_settings.m_ThermalReliefCopperBridge );

    m_cornerSmoothingChoice->SetSelection( m_settings.GetCornerSmoothingType() );

    PutValueInLocalUnits( *m_cornerSmoothingCtrl, m_settings.GetCornerRadius() );

    switch( m_settings.m_Zone_HatchingStyle )
    {
    case CPolyLine::NO_HATCH:
        m_OutlineAppearanceCtrl->SetSelection( 0 );
        break;

    case CPolyLine::DIAGONAL_EDGE:
        m_OutlineAppearanceCtrl->SetSelection( 1 );
        break;

    case CPolyLine::DIAGONAL_FULL:
        m_OutlineAppearanceCtrl->SetSelection( 2 );
        break;
    }

    m_ArcApproximationOpt->SetSelection(
        m_settings.m_ArcToSegmentsCount == ARC_APPROX_SEGMENTS_COUNT_HIGHT_DEF ? 1 : 0 );

    // Create one column in m_LayerSelectionCtrl
    wxListItem column0;
    column0.SetId( 0 );
    m_LayerSelectionCtrl->InsertColumn( 0, column0 );
    // Build copper layer list and append to layer widget
    int layerCount = board->GetCopperLayerCount();
    wxImageList* imageList = new wxImageList( LAYER_BITMAP_SIZE_X, LAYER_BITMAP_SIZE_Y );
    m_LayerSelectionCtrl->AssignImageList( imageList, wxIMAGE_LIST_SMALL );
    for( int ii = 0; ii < layerCount; ii++ )
    {
        int layerNumber = LAYER_N_BACK;

        if( layerCount <= 1 || ii < layerCount - 1 )
            layerNumber = ii;
        else if( ii == layerCount - 1 )
            layerNumber = LAYER_N_FRONT;

        m_LayerId.insert( m_LayerId.begin(), layerNumber );

        msg = board->GetLayerName( layerNumber ).Trim();
        EDA_COLOR_T layerColor = board->GetLayerColor( layerNumber );
        imageList->Add( makeLayerBitmap( layerColor ) );
        int itemIndex = m_LayerSelectionCtrl->InsertItem( 0, msg, ii );

        if( m_settings.m_CurrentZone_Layer == layerNumber )
            m_LayerSelectionCtrl->Select( itemIndex );
    }

    m_LayerSelectionCtrl->SetColumnWidth( 0, wxLIST_AUTOSIZE);

    wxString netNameDoNotShowFilter = wxT( "N-*" );
    if( m_Config )
    {
        int opt = m_Config->Read( ZONE_NET_SORT_OPTION_KEY, 1l );
        m_NetDisplayOption->SetSelection( opt );
        m_Config->Read( ZONE_NET_FILTER_STRING_KEY, netNameDoNotShowFilter );
    }
    else
        m_NetDisplayOption->SetSelection( 1 );

    m_ShowNetNameFilter->SetValue( m_netNameShowFilter );
    initListNetsParams();

    // Build list of nets:
    m_DoNotShowNetNameFilter->SetValue( netNameDoNotShowFilter );
    buildAvailableListOfNets();

    wxCommandEvent event;
    OnCornerSmoothingModeChoice( event );
}


void DIALOG_COPPER_ZONE::OnButtonCancelClick( wxCommandEvent& event )
{
    Close( true );
}

void DIALOG_COPPER_ZONE::OnButtonOkClick( wxCommandEvent& event )
{
    m_netNameShowFilter = m_ShowNetNameFilter->GetValue();

    if( AcceptOptions( true ) )
    {
        *m_ptr = m_settings;
        EndModal( ZONE_OK );
    }
}


// called on system close button
void DIALOG_COPPER_ZONE::OnClose( wxCloseEvent& event )
{
    if( m_OnExitCode != ZONE_ABORT )
        *m_ptr = m_settings;

    EndModal( m_OnExitCode );
}


bool DIALOG_COPPER_ZONE::AcceptOptions( bool aPromptForErrors, bool aUseExportableSetupOnly )
{
    switch( m_PadInZoneOpt->GetSelection() )
    {
    case 3:
        // Pads are not covered
        m_settings.SetPadConnection( PAD_NOT_IN_ZONE );
        break;

    case 2:
        // Use thermal relief for THT pads
        m_settings.SetPadConnection( THT_THERMAL );
        break;

    case 1:
        // Use thermal relief for pads
        m_settings.SetPadConnection( THERMAL_PAD );
        break;

    case 0:
        // pads are covered by copper
        m_settings.SetPadConnection( PAD_IN_ZONE );
        break;
    }

    switch( m_OutlineAppearanceCtrl->GetSelection() )
    {
    case 0:
        m_settings.m_Zone_HatchingStyle = CPolyLine::NO_HATCH;
        break;

    case 1:
        m_settings.m_Zone_HatchingStyle = CPolyLine::DIAGONAL_EDGE;
        break;

    case 2:
        m_settings.m_Zone_HatchingStyle = CPolyLine::DIAGONAL_FULL;
        break;
    }

    m_settings.m_ArcToSegmentsCount = m_ArcApproximationOpt->GetSelection() == 1 ?
                                           ARC_APPROX_SEGMENTS_COUNT_HIGHT_DEF :
                                           ARC_APPROX_SEGMENTS_COUNT_LOW_DEF;

    if( m_Config )
    {
        m_Config->Write( ZONE_NET_OUTLINES_HATCH_OPTION_KEY,
                         (long) m_settings.m_Zone_HatchingStyle );
        wxString filter = m_DoNotShowNetNameFilter->GetValue();
        m_Config->Write( ZONE_NET_FILTER_STRING_KEY, filter );
    }

    m_netNameShowFilter = m_ShowNetNameFilter->GetValue();
    m_settings.m_FillMode = (m_FillModeCtrl->GetSelection() == 0) ? 0 : 1;

    wxString txtvalue = m_ZoneClearanceCtrl->GetValue();
    m_settings.m_ZoneClearance = ReturnValueFromString( g_UserUnit, txtvalue );

    // Test if this is a reasonable value for this parameter
    // A too large value can hang Pcbnew
    #define CLEARANCE_MAX_VALUE ZONE_CLEARANCE_MAX_VALUE_MIL*IU_PER_MILS
    if( m_settings.m_ZoneClearance > CLEARANCE_MAX_VALUE )
    {
        wxString msg;
        msg.Printf( _( "Clearance must be smaller than %f\" / %f mm." ),
            ZONE_CLEARANCE_MAX_VALUE_MIL / 1000.0, ZONE_CLEARANCE_MAX_VALUE_MIL * 0.0254 );
        DisplayError( this, msg );
        return false;
    }

    txtvalue = m_ZoneMinThicknessCtrl->GetValue();
    m_settings.m_ZoneMinThickness = ReturnValueFromString( g_UserUnit, txtvalue );

    if( m_settings.m_ZoneMinThickness < (ZONE_THICKNESS_MIN_VALUE_MIL*IU_PER_MILS) )
    {
        wxString msg;
        msg.Printf( _( "Minimum width must be larger than %f\" / %f mm." ),
            ZONE_THICKNESS_MIN_VALUE_MIL / 1000.0, ZONE_THICKNESS_MIN_VALUE_MIL * 0.0254 );
        DisplayError( this, msg );
        return false;
    }

    m_settings.SetCornerSmoothingType( m_cornerSmoothingChoice->GetSelection() );
    txtvalue = m_cornerSmoothingCtrl->GetValue();
    m_settings.SetCornerRadius( ReturnValueFromString( g_UserUnit, txtvalue ) );

    m_settings.m_ZonePriority = m_PriorityLevelCtrl->GetValue();

    if( m_OrientEdgesOpt->GetSelection() == 0 )
        m_settings.m_Zone_45_Only = false;
    else
        m_settings.m_Zone_45_Only = true;

    m_settings.m_ThermalReliefGap = ReturnValueFromTextCtrl( *m_AntipadSizeValue );
    m_settings.m_ThermalReliefCopperBridge = ReturnValueFromTextCtrl( *m_CopperWidthValue );

    if( m_Config )
    {
        ConfigBaseWriteDouble( m_Config, ZONE_CLEARANCE_WIDTH_STRING_KEY,
                               (double) m_settings.m_ZoneClearance / IU_PER_MILS );

        ConfigBaseWriteDouble( m_Config, ZONE_MIN_THICKNESS_WIDTH_STRING_KEY,
            (double) m_settings.m_ZoneMinThickness / IU_PER_MILS );

        ConfigBaseWriteDouble( m_Config, ZONE_THERMAL_RELIEF_GAP_STRING_KEY,
            (double) m_settings.m_ThermalReliefGap / IU_PER_MILS );

        ConfigBaseWriteDouble( m_Config, ZONE_THERMAL_RELIEF_COPPER_WIDTH_STRING_KEY,
            (double) m_settings.m_ThermalReliefCopperBridge / IU_PER_MILS );
    }

    if( m_settings.m_ThermalReliefCopperBridge <= m_settings.m_ZoneMinThickness )
    {
        DisplayError( this,
                     _( "Thermal relief spoke width is smaller than the minimum width." ) );
        return false;
    }

    // If we use only exportable to others zones parameters, exit here:
    if( aUseExportableSetupOnly )
        return true;

    // Get the layer selection for this zone
    int ii = m_LayerSelectionCtrl->GetFirstSelected();

    if( ii < 0 && aPromptForErrors )
    {
        DisplayError( this, _( "No layer selected." ) );
        return false;
    }

    m_settings.m_CurrentZone_Layer = m_LayerId[ii];

    // Get the net name selection for this zone
    ii = m_ListNetNameSelection->GetSelection();

    if( ii < 0 && aPromptForErrors )
    {
        DisplayError( this, _( "No net selected." ) );
        return false;
    }

    if( ii == 0 )   // the not connected option was selected: this is not a good practice: warn:
    {
        if( !IsOK( this, _(
                      "You have chosen the \"not connected\" option. This will create insulated copper islands. Are you sure ?" ) )
            )
            return false;
    }

    wxString net_name = m_ListNetNameSelection->GetString( ii );

    m_settings.m_NetcodeSelection = 0;

    // Search net_code for this net, if a net was selected
    if( m_ListNetNameSelection->GetSelection() > 0 )
    {
        NETINFO_ITEM* net = m_Parent->GetBoard()->FindNet( net_name );
        if( net )
            m_settings.m_NetcodeSelection = net->GetNet();
    }

    return true;
}

void DIALOG_COPPER_ZONE::OnCornerSmoothingModeChoice( wxCommandEvent& event )
{
    int selection = m_cornerSmoothingChoice->GetSelection();

    switch( selection )
    {
    case ZONE_SETTINGS::SMOOTHING_NONE:
        m_cornerSmoothingTitle->Enable( false );
        m_cornerSmoothingCtrl->Enable( false );
        break;
    case ZONE_SETTINGS::SMOOTHING_CHAMFER:
        m_cornerSmoothingTitle->Enable( true );
        m_cornerSmoothingCtrl->Enable( true );
        m_cornerSmoothingTitle->SetLabel( _( "Chamfer distance" ) );
        AddUnitSymbol( *m_cornerSmoothingTitle, g_UserUnit );
        break;
    case ZONE_SETTINGS::SMOOTHING_FILLET:
        m_cornerSmoothingTitle->Enable( true );
        m_cornerSmoothingCtrl->Enable( true );
        m_cornerSmoothingTitle->SetLabel( _( "Fillet radius" ) );
        AddUnitSymbol( *m_cornerSmoothingTitle, g_UserUnit );
        break;
    }
}


void DIALOG_COPPER_ZONE::OnNetSortingOptionSelected( wxCommandEvent& event )
{
    initListNetsParams();
    buildAvailableListOfNets();

    m_netNameShowFilter = m_ShowNetNameFilter->GetValue();
    if( m_Config )
    {
        m_Config->Write( ZONE_NET_SORT_OPTION_KEY, (long) m_NetDisplayOption->GetSelection() );
        wxString Filter = m_DoNotShowNetNameFilter->GetValue();
        m_Config->Write( ZONE_NET_FILTER_STRING_KEY, Filter );
    }
}


void DIALOG_COPPER_ZONE::ExportSetupToOtherCopperZones( wxCommandEvent& event )
{
    if( !AcceptOptions( true, true ) )
        return;

    // Export settings ( but layer and netcode ) to others copper zones
    BOARD* pcb = m_Parent->GetBoard();
    for( int ii = 0; ii < pcb->GetAreaCount(); ii++ )
    {
        ZONE_CONTAINER* zone = pcb->GetArea( ii );

        // Cannot export settings from a copper zone
        // to a zone keepout:
        if( zone->GetIsKeepout() )
            continue;
        m_settings.ExportSetting( *zone, false );  // false = partial export
        m_Parent->OnModify();
    }

    m_OnExitCode = ZONE_EXPORT_VALUES;     // values are exported to others zones
}


void DIALOG_COPPER_ZONE::OnPadsInZoneClick( wxCommandEvent& event )
{
    switch( m_PadInZoneOpt->GetSelection() )
    {
    default:
        m_AntipadSizeValue->Enable( false );
        m_CopperWidthValue->Enable( false );
        break;

    case 2:
    case 1:
        m_AntipadSizeValue->Enable( true );
        m_CopperWidthValue->Enable( true );
        break;
    }
}


void DIALOG_COPPER_ZONE::initListNetsParams()
{
    switch( m_NetDisplayOption->GetSelection() )
    {
    case 0:
        m_NetSortingByPadCount = false;
        m_NetFiltering = false;
        break;

    case 1:
        m_NetSortingByPadCount = true;
        m_NetFiltering = false;
        break;

    case 2:
        m_NetSortingByPadCount = false;
        m_NetFiltering = true;
        break;

    case 3:
        m_NetSortingByPadCount = true;
        m_NetFiltering = true;
        break;
    }
}


void DIALOG_COPPER_ZONE::OnRunFiltersButtonClick( wxCommandEvent& event )
{
    m_netNameShowFilter = m_ShowNetNameFilter->GetValue();

    // Ensure filtered option for nets
    if( m_NetDisplayOption->GetSelection() == 0 )
        m_NetDisplayOption->SetSelection( 2 );
    else if( m_NetDisplayOption->GetSelection() == 1 )
        m_NetDisplayOption->SetSelection( 3 );
    initListNetsParams();
    buildAvailableListOfNets();
}


void DIALOG_COPPER_ZONE::buildAvailableListOfNets()
{
    wxArrayString   listNetName;

    m_Parent->GetBoard()->ReturnSortedNetnamesList( listNetName, m_NetSortingByPadCount );

    if( m_NetFiltering )
    {
        wxString doNotShowFilter = m_DoNotShowNetNameFilter->GetValue();
        wxString ShowFilter = m_ShowNetNameFilter->GetValue();

        for( unsigned ii = 0; ii < listNetName.GetCount(); ii++ )
        {
            if( listNetName[ii].Matches( doNotShowFilter ) )
            {
                listNetName.RemoveAt( ii );
                ii--;
            }
            else if( !listNetName[ii].Matches( ShowFilter ) )
            {
                listNetName.RemoveAt( ii );
                ii--;
            }
        }
    }

    listNetName.Insert( wxT( "<no net>" ), 0 );

    // Ensure currently selected net for the zone is visible, regardless of filters
    int selectedNetListNdx = -1;
    int net_select = m_settings.m_NetcodeSelection;

    if( net_select > 0 )
    {
        NETINFO_ITEM* equipot = m_Parent->GetBoard()->FindNet( net_select );
        if( equipot )
        {
            selectedNetListNdx = listNetName.Index( equipot->GetNetname() );

            if( wxNOT_FOUND == selectedNetListNdx )
            {
                // the currently selected net must *always* be visible.
                listNetName.Insert( equipot->GetNetname(), 0 );
                selectedNetListNdx = 0;
            }
        }
    }
    else if( net_select == 0 )
        selectedNetListNdx = 0;     // SetSelection() on "<no net>"
    else
    {
        // selectedNetListNdx remains -1, no net selected.
    }

    m_ListNetNameSelection->Clear();
    m_ListNetNameSelection->InsertItems( listNetName, 0 );
    m_ListNetNameSelection->SetSelection( 0 );

    if( selectedNetListNdx >= 0 )
    {
        m_ListNetNameSelection->SetSelection( selectedNetListNdx );
        m_ListNetNameSelection->EnsureVisible( selectedNetListNdx );
    }
}


wxBitmap DIALOG_COPPER_ZONE::makeLayerBitmap( EDA_COLOR_T aColor )
{
    wxBitmap    bitmap( LAYER_BITMAP_SIZE_X, LAYER_BITMAP_SIZE_Y );
    wxBrush     brush;
    wxMemoryDC  iconDC;

    iconDC.SelectObject( bitmap );
    brush.SetColour( MakeColour( aColor ) );
    brush.SetStyle( wxSOLID );
    iconDC.SetBrush( brush );
    iconDC.DrawRectangle( 0, 0, LAYER_BITMAP_SIZE_X, LAYER_BITMAP_SIZE_Y );

    return bitmap;
}
