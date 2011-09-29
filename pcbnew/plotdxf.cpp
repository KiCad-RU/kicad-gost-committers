/**
 * @file plotdxf.cpp
 * @brief Plot DXF.
 */

#include "fctsys.h"
#include "common.h"
#include "plot_common.h"
#include "confirm.h"
#include "trigo.h"
#include "wxBasePcbFrame.h"

#include "pcbnew.h"
#include "protos.h"
#include "pcbplot.h"


bool PCB_BASE_FRAME::ExportToDxfFile( const wxString& aFullFileName, int aLayer,
                                      GRTraceMode aTraceMode )
{
    Ki_PageDescr* currentsheet = GetScreen()->m_CurrentSheetDesc;

    FILE* output_file = wxFopen( aFullFileName, wxT( "wt" ) );

    if( output_file == NULL )
    {
        return false;
    }

    SetLocaleTo_C_standard();

    DXF_PLOTTER* plotter = new DXF_PLOTTER();
    plotter->set_paper_size( currentsheet );
    plotter->set_viewport( wxPoint( 0, 0 ), 1, 0 );
    plotter->set_creator( wxT( "PCBNEW-DXF" ) );
    plotter->set_filename( aFullFileName );
    plotter->start_plot( output_file );

    if( g_PcbPlotOptions.m_PlotFrameRef )
        PlotWorkSheet( plotter, GetScreen() );

    Plot_Layer( plotter, aLayer, aTraceMode );
    plotter->end_plot();
    delete plotter;
    SetLocaleTo_Default();

    return true;
}
