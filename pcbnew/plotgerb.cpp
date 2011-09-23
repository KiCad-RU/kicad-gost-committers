/**
 * @file plotgerb.cpp
 * @brief Functions to plot a board in GERBER RS274X format.
 */

/* Creates the output files, one per board layer:
 * filenames are like xxxc.PHO and use the RS274X format
 * Units = inches
 * format 3.4, Leading zero omitted, Abs format
 * format 3.4 uses the native pcbnew units (1/10000 inch).
 */

#include "fctsys.h"
#include "common.h"
#include "plot_common.h"
#include "confirm.h"
#include "pcbplot.h"
#include "trigo.h"
#include "wxBasePcbFrame.h"
#include "layers_id_colors_and_visibility.h"

#include "pcbnew.h"
#include "protos.h"


/* Creates the output files, one per board layer:
 * filenames are like xxxc.PHO and use the RS274X format
 * Units = inches
 * format 3.4, Leading zero omitted, Abs format
 * format 3.4 uses the native pcbnew units (1/10000 inch).
 */
bool PCB_BASE_FRAME::Genere_GERBER( const wxString& FullFileName, int Layer,
                                    bool PlotOriginIsAuxAxis, GRTraceMode trace_mode )
{
    FILE* output_file = wxFopen( FullFileName, wxT( "wt" ) );

    if( output_file == NULL )
    {
        return false;
    }

    wxPoint offset;

    /* Calculate scaling from pcbnew units (in 0.1 mil or 0.0001 inch) to gerber units */
    double scale = g_PcbPlotOptions.m_PlotScale;

    if( PlotOriginIsAuxAxis )
    {
        offset = m_Auxiliary_Axis_Position;
    }
    else
    {
        offset.x = 0;
        offset.y = 0;
    }

    SetLocaleTo_C_standard();
    PLOTTER* plotter = new GERBER_PLOTTER();
    /* No mirror and scaling for gerbers! */
    plotter->set_viewport( offset, scale, 0 );
    plotter->set_default_line_width( g_PcbPlotOptions.m_PlotLineWidth );
    plotter->set_creator( wxT( "PCBNEW-RS274X" ) );
    plotter->set_filename( FullFileName );

    if( plotter->start_plot( output_file ) )
    {
        // Skip NPTH pads on copper layers
        // ( only if hole size == pad size ):
        if( (Layer >= LAYER_N_BACK) && (Layer <= LAYER_N_FRONT) )
            g_PcbPlotOptions.m_SkipNPTH_Pads = true;

        // Sheet refs on gerber CAN be useful... and they're always 1:1
        if( g_PcbPlotOptions.m_PlotFrameRef )
            PlotWorkSheet( plotter, GetScreen() );

        Plot_Layer( plotter, Layer, trace_mode );
        plotter->end_plot();

        g_PcbPlotOptions.m_SkipNPTH_Pads = false;
    }
    else    // error in start_plot( ): failed opening a temporary file
    {
        wxMessageBox( _("Error when creating %s file: unable to create a temporary file"));
    }

    delete plotter;
    SetLocaleTo_Default();

    return true;
}
