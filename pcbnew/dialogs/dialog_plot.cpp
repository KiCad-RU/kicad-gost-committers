/**
 * @file dialog_plot.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
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

#include <appl_wxstruct.h>
#include <plot_common.h>
#include <confirm.h>
#include <wxPcbStruct.h>
#include <pcbplot.h>
#include <base_units.h>
#include <class_board.h>
#include <plotcontroller.h>
#include <wx/ffile.h>
#include <dialog_plot.h>

/**
 * Class DIALOG_PLOT
 */

DIALOG_PLOT::DIALOG_PLOT( PCB_EDIT_FRAME* aParent ) :
    DIALOG_PLOT_BASE( aParent ), m_parent( aParent ),
    m_board( aParent->GetBoard() ),
    m_plotOpts( aParent->GetPlotSettings() )
{
    m_config = wxGetApp().GetSettings();
    m_brdSettings = m_board->GetDesignSettings();

    Init_Dialog();

    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
}


void DIALOG_PLOT::Init_Dialog()
{
    wxString    msg;
    wxFileName  fileName;

    m_config->Read( OPTKEY_PLOT_X_FINESCALE_ADJ, &m_XScaleAdjust );
    m_config->Read( OPTKEY_PLOT_Y_FINESCALE_ADJ, &m_YScaleAdjust );

    // m_PSWidthAdjust is stored in mm in user config
    double dtmp;
    m_config->Read( CONFIG_PS_FINEWIDTH_ADJ, &dtmp, 0 );
    m_PSWidthAdjust = KiROUND( dtmp * IU_PER_MM );

    // The reasonable width correction value must be in a range of
    // [-(MinTrackWidth-1), +(MinClearanceValue-1)] decimils.
    m_widthAdjustMinValue   = -(m_board->GetDesignSettings().m_TrackMinWidth - 1);
    m_widthAdjustMaxValue   = m_board->GetSmallestClearanceValue() - 1;

    switch( m_plotOpts.GetFormat() )
    {
    default:
    case PLOT_FORMAT_GERBER:
        m_plotFormatOpt->SetSelection( 0 );
        break;

    case PLOT_FORMAT_POST:
        m_plotFormatOpt->SetSelection( 1 );
        break;

    case PLOT_FORMAT_SVG:
        m_plotFormatOpt->SetSelection( 2 );
        break;

    case PLOT_FORMAT_DXF:
        m_plotFormatOpt->SetSelection( 3 );
        break;

    case PLOT_FORMAT_HPGL:
        m_plotFormatOpt->SetSelection( 4 );
        break;
    }

    msg = ReturnStringFromValue( g_UserUnit, m_brdSettings.m_SolderMaskMargin, true );
    m_SolderMaskMarginCurrValue->SetLabel( msg );
    msg = ReturnStringFromValue( g_UserUnit, m_brdSettings.m_SolderMaskMinWidth, true );
    m_SolderMaskMinWidthCurrValue->SetLabel( msg );


    // Set units and value for HPGL pen size (this param in in mils).
    AddUnitSymbol( *m_textPenSize, g_UserUnit );
    msg = ReturnStringFromValue( g_UserUnit,
                                 m_plotOpts.GetHPGLPenDiameter() * IU_PER_MILS );
    m_HPGLPenSizeOpt->AppendText( msg );

    // Set units and value for HPGL pen overlay (this param in in mils).
    AddUnitSymbol( *m_textPenOvr, g_UserUnit );
    msg = ReturnStringFromValue( g_UserUnit,
                                 m_plotOpts.GetHPGLPenOverlay() * IU_PER_MILS );
    m_HPGLPenOverlayOpt->AppendText( msg );

    AddUnitSymbol( *m_textDefaultPenSize, g_UserUnit );
    msg = ReturnStringFromValue( g_UserUnit, m_plotOpts.GetLineWidth() );
    m_linesWidth->AppendText( msg );

    // Set units for PS global width correction.
    AddUnitSymbol( *m_textPSFineAdjustWidth, g_UserUnit );

    m_useAuxOriginCheckBox->SetValue( m_plotOpts.GetUseAuxOrigin() );

    // Test for a reasonable scale value. Set to 1 if problem
    if( m_XScaleAdjust < PLOT_MIN_SCALE || m_YScaleAdjust < PLOT_MIN_SCALE
        || m_XScaleAdjust > PLOT_MAX_SCALE || m_YScaleAdjust > PLOT_MAX_SCALE )
        m_XScaleAdjust = m_YScaleAdjust = 1.0;

    msg.Printf( wxT( "%f" ), m_XScaleAdjust );
    m_fineAdjustXscaleOpt->AppendText( msg );

    msg.Printf( wxT( "%f" ), m_YScaleAdjust );
    m_fineAdjustYscaleOpt->AppendText( msg );

    // Test for a reasonable PS width correction value. Set to 0 if problem.
    if( m_PSWidthAdjust < m_widthAdjustMinValue || m_PSWidthAdjust > m_widthAdjustMaxValue )
        m_PSWidthAdjust = 0.;

    msg.Printf( wxT( "%f" ), To_User_Unit( g_UserUnit, m_PSWidthAdjust ) );
    m_PSFineAdjustWidthOpt->AppendText( msg );

    m_plotPSNegativeOpt->SetValue( m_plotOpts.GetNegative() );
    m_forcePSA4OutputOpt->SetValue( m_plotOpts.GetA4Output() );

    // List layers in same order than in setup layers dialog
    // (Front or Top to Back or Bottom)
    DECLARE_LAYERS_ORDER_LIST( layersOrder );
    int layerIndex, checkIndex, layer;

    for( layerIndex = 0; layerIndex < NB_LAYERS; layerIndex++ )
    {
        layer = layersOrder[layerIndex];

        wxASSERT( layer < NB_LAYERS );

        if( !m_board->IsLayerEnabled( layer ) )
            continue;

        m_layerList.push_back( layer );
        checkIndex = m_layerCheckListBox->Append( m_board->GetLayerName( layer ) );

        if( m_plotOpts.GetLayerSelection() & ( 1 << layer ) )
            m_layerCheckListBox->Check( checkIndex );
    }

    // Option for using proper Gerber extensions
    m_useGerberExtensions->SetValue( m_plotOpts.GetUseGerberExtensions() );

    // Option for excluding contents of "Edges Pcb" layer
    m_excludeEdgeLayerOpt->SetValue( m_plotOpts.GetExcludeEdgeLayer() );

    m_subtractMaskFromSilk->SetValue( m_plotOpts.GetSubtractMaskFromSilk() );

    // Option to plot page references:
    m_plotSheetRef->SetValue( m_plotOpts.GetPlotFrameRef() );

    // Option to allow pads on silkscreen layers
    m_plotPads_on_Silkscreen->SetValue( m_plotOpts.GetPlotPadsOnSilkLayer() );

    // Options to plot texts on footprints
    m_plotModuleValueOpt->SetValue( m_plotOpts.GetPlotValue() );
    m_plotModuleRefOpt->SetValue( m_plotOpts.GetPlotReference() );
    m_plotTextOther->SetValue( m_plotOpts.GetPlotOtherText() );
    m_plotInvisibleText->SetValue( m_plotOpts.GetPlotInvisibleText() );

    // Options to plot pads and vias holes
    m_drillShapeOpt->SetSelection( m_plotOpts.GetDrillMarksType() );

    // Scale option
    m_scaleOpt->SetSelection( m_plotOpts.GetScaleSelection() );

    // Plot mode
    m_plotModeOpt->SetSelection( m_plotOpts.GetMode() );

    // Plot mirror option
    m_plotMirrorOpt->SetValue( m_plotOpts.GetMirror() );

    // Put vias on mask layer
    m_plotNoViaOnMaskOpt->SetValue( m_plotOpts.GetPlotViaOnMaskLayer() );

    // Output directory
    m_outputDirectoryName->SetValue( m_plotOpts.GetOutputDirectory() );

    // Update options values:
    wxCommandEvent cmd_event;
    SetPlotFormat( cmd_event );
    OnSetScaleOpt( cmd_event );
}


void DIALOG_PLOT::OnQuit( wxCommandEvent& event )
{
    Close( true );    // true is to force the frame to close
}


void DIALOG_PLOT::OnClose( wxCloseEvent& event )
{
    applyPlotSettings();
    EndModal( 0 );
}

// A helper function to show a popup menu, when the dialog is right clicked.
void DIALOG_PLOT::OnRightClick( wxMouseEvent& event )
{
    PopupMenu( m_popMenu );
}

// Select or deselect groups of layers in the layers list:
#include <layers_id_colors_and_visibility.h>
void DIALOG_PLOT::OnPopUpLayers( wxCommandEvent& event )
{
    unsigned int    i;

    switch( event.GetId() )
    {
        case ID_LAYER_FAB: // Select layers usually neede d to build a board
            for( i = 0; i < m_layerList.size(); i++ )
            {
                long layermask = 1 << m_layerList[ i ];
                if( ( layermask &
                    ( ALL_CU_LAYERS | SOLDERPASTE_LAYER_BACK | SOLDERPASTE_LAYER_FRONT |
                      SOLDERMASK_LAYER_BACK | SOLDERMASK_LAYER_FRONT |
                      SILKSCREEN_LAYER_BACK | SILKSCREEN_LAYER_FRONT ) )
                    != 0 )
                    m_layerCheckListBox->Check( i, true );
                else
                    m_layerCheckListBox->Check( i, false );

            }
            break;

        case ID_SELECT_COPPER_LAYERS:
            for( i = 0; i < m_layerList.size(); i++ )
            {
                if( m_layerList[i] <= LAST_COPPER_LAYER )
                    m_layerCheckListBox->Check( i, true );

            }
            break;

        case ID_DESELECT_COPPER_LAYERS:
            for( i = 0; i < m_layerList.size(); i++ )
            {
                if( m_layerList[i] <= LAST_COPPER_LAYER )
                    m_layerCheckListBox->Check( i, false );

            }
            break;

        case ID_SELECT_ALL_LAYERS:
            for( i = 0; i < m_layerList.size(); i++ )
                m_layerCheckListBox->Check( i, true );
            break;

        case ID_DESELECT_ALL_LAYERS:
            for( i = 0; i < m_layerList.size(); i++ )
                m_layerCheckListBox->Check( i, false );
            break;

        default:
            break;
    }
}

void DIALOG_PLOT::CreateDrillFile( wxCommandEvent& event )
{
    m_parent->InstallDrillFrame( event );
}


void DIALOG_PLOT::OnSetScaleOpt( wxCommandEvent& event )
{
    /* Disable sheet reference for scale != 1:1 */
    bool scale1 = ( m_scaleOpt->GetSelection() == 1 );

    m_plotSheetRef->Enable( scale1 );

    if( !scale1 )
        m_plotSheetRef->SetValue( false );
}


void DIALOG_PLOT::OnOutputDirectoryBrowseClicked( wxCommandEvent& event )
{
    // Build the absolute path of current output plot directory
    // to preselect it when opening the dialog.
    wxFileName  fn( m_outputDirectoryName->GetValue() );
    wxString    path;

    if( fn.IsRelative() )
        path = wxGetCwd() + fn.GetPathSeparator() + m_outputDirectoryName->GetValue();
    else
        path = m_outputDirectoryName->GetValue();

    wxDirDialog dirDialog( this, _( "Select Output Directory" ), path );

    if( dirDialog.ShowModal() == wxID_CANCEL )
        return;

    wxFileName      dirName = wxFileName::DirName( dirDialog.GetPath() );

    wxMessageDialog dialog( this, _( "Use a relative path? " ),
                            _( "Plot Output Directory" ),
                            wxYES_NO | wxICON_QUESTION | wxYES_DEFAULT );

    if( dialog.ShowModal() == wxID_YES )
    {
        wxString boardFilePath = ( (wxFileName) m_parent->GetBoard()->GetFileName() ).GetPath();

        if( !dirName.MakeRelativeTo( boardFilePath ) )
            wxMessageBox( _(
                             "Cannot make path relative (target volume different from board file volume)!" ),
                          _( "Plot Output Directory" ), wxOK | wxICON_ERROR );
    }

    m_outputDirectoryName->SetValue( dirName.GetFullPath() );
}


PlotFormat DIALOG_PLOT::GetPlotFormat()
{
    // plot format id's are ordered like displayed in m_plotFormatOpt
    static const PlotFormat plotFmt[] =
    {
        PLOT_FORMAT_GERBER,
        PLOT_FORMAT_POST,
        PLOT_FORMAT_SVG,
        PLOT_FORMAT_DXF,
        PLOT_FORMAT_HPGL,
        PLOT_FORMAT_PDF
    };

    return plotFmt[ m_plotFormatOpt->GetSelection() ];
}

// Enable or disable widgets according to the plot format selected
// and clear also some optional values
void DIALOG_PLOT::SetPlotFormat( wxCommandEvent& event )
{
    switch( GetPlotFormat() )
    {
    case PLOT_FORMAT_PDF:
    case PLOT_FORMAT_SVG:
        m_drillShapeOpt->Enable( true );
        m_plotModeOpt->Enable( false );
        m_plotModeOpt->SetSelection( 1 );
        m_plotMirrorOpt->Enable( true );
        m_useAuxOriginCheckBox->Enable( false );
        m_useAuxOriginCheckBox->SetValue( false );
        m_linesWidth->Enable( true );
        m_HPGLPenSizeOpt->Enable( false );
        m_HPGLPenOverlayOpt->Enable( false );
        m_excludeEdgeLayerOpt->Enable( true );
        m_subtractMaskFromSilk->Enable( false );
        m_subtractMaskFromSilk->SetValue( false );
        m_useGerberExtensions->Enable( false );
        m_useGerberExtensions->SetValue( false );
        m_scaleOpt->Enable( false );
        m_scaleOpt->SetSelection( 1 );
        m_fineAdjustXscaleOpt->Enable( false );
        m_fineAdjustYscaleOpt->Enable( false );
        m_PSFineAdjustWidthOpt->Enable( false );
        m_plotPSNegativeOpt->Enable( true );
        m_forcePSA4OutputOpt->Enable( false );
        m_forcePSA4OutputOpt->SetValue( false );

        m_PlotOptionsSizer->Hide( m_GerberOptionsSizer );
        m_PlotOptionsSizer->Hide( m_HPGLOptionsSizer );
        m_PlotOptionsSizer->Hide( m_PSOptionsSizer );
        break;

    case PLOT_FORMAT_POST:
        m_drillShapeOpt->Enable( true );
        m_plotModeOpt->Enable( true );
        m_plotMirrorOpt->Enable( true );
        m_useAuxOriginCheckBox->Enable( false );
        m_useAuxOriginCheckBox->SetValue( false );
        m_linesWidth->Enable( true );
        m_HPGLPenSizeOpt->Enable( false );
        m_HPGLPenOverlayOpt->Enable( false );
        m_excludeEdgeLayerOpt->Enable( true );
        m_subtractMaskFromSilk->Enable( false );
        m_subtractMaskFromSilk->SetValue( false );
        m_useGerberExtensions->Enable( false );
        m_useGerberExtensions->SetValue( false );
        m_scaleOpt->Enable( true );
        m_fineAdjustXscaleOpt->Enable( true );
        m_fineAdjustYscaleOpt->Enable( true );
        m_PSFineAdjustWidthOpt->Enable( true );
        m_plotPSNegativeOpt->Enable( true );
        m_forcePSA4OutputOpt->Enable( true );

        m_PlotOptionsSizer->Hide( m_GerberOptionsSizer );
        m_PlotOptionsSizer->Hide( m_HPGLOptionsSizer );
        m_PlotOptionsSizer->Show( m_PSOptionsSizer );
        break;

    case PLOT_FORMAT_GERBER:
        m_drillShapeOpt->Enable( false );
        m_drillShapeOpt->SetSelection( 0 );
        m_plotModeOpt->Enable( false );
        m_plotModeOpt->SetSelection( 1 );
        m_plotMirrorOpt->Enable( false );
        m_plotMirrorOpt->SetValue( false );
        m_useAuxOriginCheckBox->Enable( true );
        m_linesWidth->Enable( true );
        m_HPGLPenSizeOpt->Enable( false );
        m_HPGLPenOverlayOpt->Enable( false );
        m_excludeEdgeLayerOpt->Enable( true );
        m_subtractMaskFromSilk->Enable( true );
        m_useGerberExtensions->Enable( true );
        m_scaleOpt->Enable( false );
        m_scaleOpt->SetSelection( 1 );
        m_fineAdjustXscaleOpt->Enable( false );
        m_fineAdjustYscaleOpt->Enable( false );
        m_PSFineAdjustWidthOpt->Enable( false );
        m_plotPSNegativeOpt->Enable( false );
        m_plotPSNegativeOpt->SetValue( false );
        m_forcePSA4OutputOpt->Enable( false );
        m_forcePSA4OutputOpt->SetValue( false );

        m_PlotOptionsSizer->Show( m_GerberOptionsSizer );
        m_PlotOptionsSizer->Hide( m_HPGLOptionsSizer );
        m_PlotOptionsSizer->Hide( m_PSOptionsSizer );
        break;

    case PLOT_FORMAT_HPGL:
        m_drillShapeOpt->Enable( true );
        m_plotModeOpt->Enable( true );
        m_plotMirrorOpt->Enable( true );
        m_useAuxOriginCheckBox->Enable( false );
        m_useAuxOriginCheckBox->SetValue( false );
        m_linesWidth->Enable( false );
        m_HPGLPenSizeOpt->Enable( true );
        m_HPGLPenOverlayOpt->Enable( true );
        m_excludeEdgeLayerOpt->Enable( true );
        m_subtractMaskFromSilk->Enable( false );
        m_subtractMaskFromSilk->SetValue( false );
        m_useGerberExtensions->Enable( false );
        m_useGerberExtensions->SetValue( false );
        m_scaleOpt->Enable( true );
        m_fineAdjustXscaleOpt->Enable( false );
        m_fineAdjustYscaleOpt->Enable( false );
        m_PSFineAdjustWidthOpt->Enable( false );
        m_plotPSNegativeOpt->SetValue( false );
        m_plotPSNegativeOpt->Enable( false );
        m_forcePSA4OutputOpt->Enable( true );

        m_PlotOptionsSizer->Hide( m_GerberOptionsSizer );
        m_PlotOptionsSizer->Show( m_HPGLOptionsSizer );
        m_PlotOptionsSizer->Hide( m_PSOptionsSizer );
        break;

    case PLOT_FORMAT_DXF:
        m_drillShapeOpt->Enable( true );
        m_plotModeOpt->Enable( true );
        m_plotMirrorOpt->Enable( false );
        m_plotMirrorOpt->SetValue( false );
        m_useAuxOriginCheckBox->Enable( true );
        m_linesWidth->Enable( false );
        m_HPGLPenSizeOpt->Enable( false );
        m_HPGLPenOverlayOpt->Enable( false );
        m_excludeEdgeLayerOpt->Enable( true );
        m_subtractMaskFromSilk->Enable( false );
        m_subtractMaskFromSilk->SetValue( false );
        m_useGerberExtensions->Enable( false );
        m_useGerberExtensions->SetValue( false );
        m_scaleOpt->Enable( false );
        m_scaleOpt->SetSelection( 1 );
        m_fineAdjustXscaleOpt->Enable( false );
        m_fineAdjustYscaleOpt->Enable( false );
        m_PSFineAdjustWidthOpt->Enable( false );
        m_plotPSNegativeOpt->Enable( false );
        m_plotPSNegativeOpt->SetValue( false );
        m_forcePSA4OutputOpt->Enable( false );
        m_forcePSA4OutputOpt->SetValue( false );

        m_PlotOptionsSizer->Show( m_GerberOptionsSizer );
        m_PlotOptionsSizer->Hide( m_HPGLOptionsSizer );
        m_PlotOptionsSizer->Hide( m_PSOptionsSizer );
        break;

    default:
        wxASSERT( false );
    }

    /* Update the interlock between scale and frame reference
     * (scaling would mess up the frame border...) */
    OnSetScaleOpt( event );

    Layout();
    m_MainSizer->SetSizeHints( this );
}


// A helper function to "clip" aValue between aMin and aMax
// and write result in * aResult
// return false if clipped, true if aValue is just copied into * aResult
static bool setDouble( double* aResult, double aValue, double aMin, double aMax )
{
    if( aValue < aMin )
    {
        *aResult = aMin;
        return false;
    }
    else if( aValue > aMax )
    {
        *aResult = aMax;
        return false;
    }

    *aResult = aValue;
    return true;
}
static bool setInt( int* aResult, int aValue, int aMin, int aMax )
{
    if( aValue < aMin )
    {
        *aResult = aMin;
        return false;
    }
    else if( aValue > aMax )
    {
        *aResult = aMax;
        return false;
    }

    *aResult = aValue;
    return true;
}


void DIALOG_PLOT::applyPlotSettings()
{
    PCB_PLOT_PARAMS tempOptions;

    tempOptions.SetExcludeEdgeLayer( m_excludeEdgeLayerOpt->GetValue() );
    tempOptions.SetSubtractMaskFromSilk( m_subtractMaskFromSilk->GetValue() );
    tempOptions.SetPlotFrameRef( m_plotSheetRef->GetValue() );
    tempOptions.SetPlotPadsOnSilkLayer( m_plotPads_on_Silkscreen->GetValue() );
    tempOptions.SetUseAuxOrigin( m_useAuxOriginCheckBox->GetValue() );
    tempOptions.SetPlotValue( m_plotModuleValueOpt->GetValue() );
    tempOptions.SetPlotReference( m_plotModuleRefOpt->GetValue() );
    tempOptions.SetPlotOtherText( m_plotTextOther->GetValue() );
    tempOptions.SetPlotInvisibleText( m_plotInvisibleText->GetValue() );
    tempOptions.SetScaleSelection( m_scaleOpt->GetSelection() );
    tempOptions.SetDrillMarksType( static_cast<PCB_PLOT_PARAMS::DrillMarksType>
                                   ( m_drillShapeOpt->GetSelection() ) );
    tempOptions.SetMirror( m_plotMirrorOpt->GetValue() );
    tempOptions.SetMode( static_cast<EDA_DRAW_MODE_T>( m_plotModeOpt->GetSelection() ) );
    tempOptions.SetPlotViaOnMaskLayer( m_plotNoViaOnMaskOpt->GetValue() );

    // Update settings from text fields. Rewrite values back to the fields,
    // since the values may have been constrained by the setters.

    // read HPLG pen size (this param is stored in mils)
    wxString    msg = m_HPGLPenSizeOpt->GetValue();
    int         tmp = ReturnValueFromString( g_UserUnit, msg ) / IU_PER_MILS;

    if( !tempOptions.SetHPGLPenDiameter( tmp ) )
    {
        msg = ReturnStringFromValue( g_UserUnit, tempOptions.GetHPGLPenDiameter() * IU_PER_MILS );
        m_HPGLPenSizeOpt->SetValue( msg );
        msg.Printf( _( "HPGL pen size constrained!\n" ) );
        m_messagesBox->AppendText( msg );
    }

    // Read HPGL pen overlay (this param is stored in mils)
    msg = m_HPGLPenOverlayOpt->GetValue();
    tmp = ReturnValueFromString( g_UserUnit, msg ) / IU_PER_MILS;

    if( !tempOptions.SetHPGLPenOverlay( tmp ) )
    {
        msg = ReturnStringFromValue( g_UserUnit,
                                     tempOptions.GetHPGLPenOverlay() * IU_PER_MILS );
        m_HPGLPenOverlayOpt->SetValue( msg );
        msg.Printf( _( "HPGL pen overlay constrained!\n" ) );
        m_messagesBox->AppendText( msg );
    }

    // Default linewidth
    msg = m_linesWidth->GetValue();
    tmp = ReturnValueFromString( g_UserUnit, msg );

    if( !tempOptions.SetLineWidth( tmp ) )
    {
        msg = ReturnStringFromValue( g_UserUnit, tempOptions.GetLineWidth() );
        m_linesWidth->SetValue( msg );
        msg.Printf( _( "Default linewidth constrained!\n" ) );
        m_messagesBox->AppendText( msg );
    }

    // X scale
    double tmpDouble;
    msg = m_fineAdjustXscaleOpt->GetValue();
    msg.ToDouble( &tmpDouble );

    if( !setDouble( &m_XScaleAdjust, tmpDouble, PLOT_MIN_SCALE, PLOT_MAX_SCALE ) )
    {
        msg.Printf( wxT( "%f" ), m_XScaleAdjust );
        m_fineAdjustXscaleOpt->SetValue( msg );
        msg.Printf( _( "X scale constrained!\n" ) );
        m_messagesBox->AppendText( msg );
    }

   ConfigBaseWriteDouble( m_config, OPTKEY_PLOT_X_FINESCALE_ADJ, m_XScaleAdjust );

    // Y scale
    msg = m_fineAdjustYscaleOpt->GetValue();
    msg.ToDouble( &tmpDouble );

    if( !setDouble( &m_YScaleAdjust, tmpDouble, PLOT_MIN_SCALE, PLOT_MAX_SCALE ) )
    {
        msg.Printf( wxT( "%f" ), m_YScaleAdjust );
        m_fineAdjustYscaleOpt->SetValue( msg );
        msg.Printf( _( "Y scale constrained!\n" ) );
        m_messagesBox->AppendText( msg );
    }

    ConfigBaseWriteDouble( m_config, OPTKEY_PLOT_Y_FINESCALE_ADJ, m_YScaleAdjust );

    // PS Width correction
    msg = m_PSFineAdjustWidthOpt->GetValue();
    int itmp = ReturnValueFromString( g_UserUnit, msg );

    if( !setInt( &m_PSWidthAdjust, itmp, m_widthAdjustMinValue, m_widthAdjustMaxValue ) )
    {
        msg = ReturnStringFromValue( g_UserUnit, m_PSWidthAdjust );
        m_PSFineAdjustWidthOpt->SetValue( msg );
        msg.Printf( _( "Width correction constrained!\n"
                       "The reasonable width correction value must be in a range of\n"
                       " [%+f; %+f] (%s) for current design rules!\n" ),
                    To_User_Unit( g_UserUnit, m_widthAdjustMinValue ),
                    To_User_Unit( g_UserUnit, m_widthAdjustMaxValue ),
                    ( g_UserUnit == INCHES ) ? wxT( "\"" ) : wxT( "mm" ) );
        m_messagesBox->AppendText( msg );
    }

    // Store m_PSWidthAdjust in mm in user config
    ConfigBaseWriteDouble( m_config, CONFIG_PS_FINEWIDTH_ADJ,
                           (double)m_PSWidthAdjust / IU_PER_MM );

    tempOptions.SetUseGerberExtensions( m_useGerberExtensions->GetValue() );

    tempOptions.SetFormat( GetPlotFormat() );

    long            selectedLayers = 0;
    unsigned int    i;

    for( i = 0; i < m_layerList.size(); i++ )
    {
        if( m_layerCheckListBox->IsChecked( i ) )
            selectedLayers |= (1 << m_layerList[i]);
    }

    tempOptions.SetLayerSelection( selectedLayers );
    tempOptions.SetNegative( m_plotPSNegativeOpt->GetValue() );
    tempOptions.SetA4Output( m_forcePSA4OutputOpt->GetValue() );

    // Set output directory and replace backslashes with forward ones
    wxString dirStr;
    dirStr = m_outputDirectoryName->GetValue();
    dirStr.Replace( wxT( "\\" ), wxT( "/" ) );
    tempOptions.SetOutputDirectory( dirStr );

    if( m_plotOpts != tempOptions )
    {
        m_parent->SetPlotSettings( tempOptions );
        m_plotOpts = tempOptions;
        m_parent->OnModify();
    }
}
