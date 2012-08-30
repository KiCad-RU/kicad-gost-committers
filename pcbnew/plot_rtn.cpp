/**
 * @file plot_rtn.cpp
 * @brief Common plot routines.
 */

#include <fctsys.h>
#include <common.h>
#include <plot_common.h>
#include <base_struct.h>
#include <drawtxt.h>
#include <confirm.h>
#include <trigo.h>
#include <wxBasePcbFrame.h>
#include <pcbcommon.h>
#include <macros.h>

#include <class_board.h>
#include <class_module.h>
#include <class_track.h>
#include <class_edge_mod.h>
#include <class_pcb_text.h>
#include <class_zone.h>
#include <class_drawsegment.h>
#include <class_mire.h>
#include <class_dimension.h>

#include <pcbnew.h>
#include <pcbplot.h>

static void Plot_Edges_Modules( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts,
                BOARD* pcb, int aLayerMask, EDA_DRAW_MODE_T trace_mode );

static void PlotTextModule( PLOTTER* aPlotter, TEXTE_MODULE* pt_texte, EDA_DRAW_MODE_T trace_mode );


/* Creates the plot for silkscreen layers
 */
void PCB_BASE_FRAME::PlotSilkScreen( PLOTTER* aPlotter, int aLayerMask, EDA_DRAW_MODE_T trace_mode )
{
    bool          trace_val, trace_ref;
    TEXTE_MODULE* pt_texte;

    const PCB_PLOT_PARAMS&  plot_opts = GetPlotSettings();

    // Plot edge layer and graphic items

    for( EDA_ITEM*  item = m_Pcb->m_Drawings;  item;  item = item->Next() )
    {
        switch( item->Type() )
        {
        case PCB_LINE_T:
            PlotDrawSegment( aPlotter, plot_opts, (DRAWSEGMENT*) item, aLayerMask, trace_mode );
            break;

        case PCB_TEXT_T:
            PlotTextePcb( aPlotter, plot_opts, (TEXTE_PCB*) item, aLayerMask, trace_mode );
            break;

        case PCB_DIMENSION_T:
            PlotDimension( aPlotter, plot_opts, (DIMENSION*) item, aLayerMask, trace_mode );
            break;

        case PCB_TARGET_T:
            PlotPcbTarget( aPlotter, plot_opts, (PCB_TARGET*) item, aLayerMask, trace_mode );
            break;

        case PCB_MARKER_T:
            break;

        default:
            DisplayError( this, wxT( "PlotSilkScreen() error: unexpected Type()" ) );
            break;
        }
    }

    // Plot footprint outlines :
    Plot_Edges_Modules( aPlotter, plot_opts, m_Pcb, aLayerMask, trace_mode );

    // Plot pads (creates pads outlines, for pads on silkscreen layers)
    int layersmask_plotpads = aLayerMask;
    // Calculate the mask layers of allowed layers for pads

    if( !plot_opts.GetPlotPadsOnSilkLayer() )       // Do not plot pads on silk screen layers
        layersmask_plotpads &= ~(SILKSCREEN_LAYER_BACK | SILKSCREEN_LAYER_FRONT );

    if( layersmask_plotpads )
    {
        for( MODULE* Module = m_Pcb->m_Modules; Module; Module = Module->Next() )
        {
            for( D_PAD * pad = Module->m_Pads; pad != NULL; pad = pad->Next() )
            {
                // See if the pad is on this layer
                if( (pad->GetLayerMask() & layersmask_plotpads) == 0 )
                    continue;

                wxPoint shape_pos = pad->ReturnShapePos();

                switch( pad->GetShape() )
                {
                case PAD_CIRCLE:
                    aPlotter->FlashPadCircle( shape_pos, pad->GetSize().x, LINE );
                    break;

                case PAD_OVAL:
                    aPlotter->FlashPadOval( shape_pos, pad->GetSize(), 
		                            pad->GetOrientation(), LINE );
                    break;

                case PAD_TRAPEZOID:
                    {
                        wxPoint coord[4];
                        pad->BuildPadPolygon( coord, wxSize(0,0), 0 );
                        aPlotter->FlashPadTrapez( shape_pos, coord, 
			                          pad->GetOrientation(), LINE );
                    }
                    break;

                case PAD_RECT:
                default:
                    aPlotter->FlashPadRect( shape_pos, pad->GetSize(), 
		                            pad->GetOrientation(), LINE );
                    break;
                }
            }
        }
    }

    // Plot footprints fields (ref, value ...)
    for( MODULE* module = m_Pcb->m_Modules;  module;  module = module->Next() )
    {
        // see if we want to plot VALUE and REF fields
        trace_val = plot_opts.GetPlotValue();
        trace_ref = plot_opts.GetPlotReference();

        TEXTE_MODULE* text = module->m_Reference;
        unsigned      textLayer = text->GetLayer();

        if( textLayer >= 32 )
        {
            wxString errMsg;

            errMsg.Printf( _( "Your BOARD has a bad layer number of %u for \
module\n %s's \"reference\" text." ),
                           textLayer, GetChars( module->GetReference() ) );
            DisplayError( this, errMsg );
            return;
        }

        if( ( ( 1 << textLayer ) & aLayerMask ) == 0 )
            trace_ref = false;

        if( !text->IsVisible() && !plot_opts.GetPlotInvisibleText() )
            trace_ref = false;

        text = module->m_Value;
        textLayer = text->GetLayer();

        if( textLayer > 32 )
        {
            wxString errMsg;

            errMsg.Printf( _( "Your BOARD has a bad layer number of %u for \
module\n %s's \"value\" text." ),
                           textLayer, GetChars( module->GetReference() ) );
            DisplayError( this, errMsg );
            return;
        }

        if( ( (1 << textLayer) & aLayerMask ) == 0 )
            trace_val = false;

        if( !text->IsVisible() && !plot_opts.GetPlotInvisibleText() )
            trace_val = false;

        // Plot text fields, if allowed
        if( trace_ref )
            PlotTextModule( aPlotter, module->m_Reference, trace_mode );

        if( trace_val )
            PlotTextModule( aPlotter, module->m_Value, trace_mode );

        for( pt_texte = (TEXTE_MODULE*) module->m_Drawings.GetFirst();
             pt_texte != NULL;
             pt_texte = pt_texte->Next() )
        {
            if( pt_texte->Type() != PCB_MODULE_TEXT_T )
                continue;

            if( !plot_opts.GetPlotOtherText() )
                continue;

            if( !pt_texte->IsVisible() && !plot_opts.GetPlotInvisibleText() )
                continue;

            textLayer = pt_texte->GetLayer();

            if( textLayer >= 32 )
            {
                wxString errMsg;

                errMsg.Printf( _( "Your BOARD has a bad layer number of %u \
for module\n %s's \"module text\" text of %s." ),
                               textLayer, GetChars( module->GetReference() ),
                               GetChars( pt_texte->m_Text ) );
                DisplayError( this, errMsg );
                return;
            }

            if( !( ( 1 << textLayer ) & aLayerMask ) )
                continue;

            PlotTextModule( aPlotter, pt_texte, trace_mode );
        }
    }

    // Plot filled areas
    for( int ii = 0; ii < m_Pcb->GetAreaCount(); ii++ )
    {
        ZONE_CONTAINER* edge_zone = m_Pcb->GetArea( ii );

        if( ( ( 1 << edge_zone->GetLayer() ) & aLayerMask ) == 0 )
            continue;

        PlotFilledAreas( aPlotter, plot_opts, edge_zone, trace_mode );
    }

    // Plot segments used to fill zone areas (outdated, but here for old boards
    // compatibility):
    for( SEGZONE* seg = m_Pcb->m_Zone; seg != NULL; seg = seg->Next() )
    {
        if( ( ( 1 << seg->GetLayer() ) & aLayerMask ) == 0 )
            continue;

        aPlotter->ThickSegment( seg->m_Start, seg->m_End, seg->m_Width, trace_mode );
    }
}


static void PlotTextModule( PLOTTER* aPlotter, TEXTE_MODULE* pt_texte, EDA_DRAW_MODE_T trace_mode )
{
    wxSize  size;
    wxPoint pos;
    int     orient, thickness;

    // calculate some text parameters :
    size = pt_texte->m_Size;
    pos  = pt_texte->m_Pos;

    orient = pt_texte->GetDrawRotation();

    thickness = pt_texte->m_Thickness;

    if( trace_mode == LINE )
        thickness = -1;

    if( pt_texte->m_Mirror )
        NEGATE( size.x );  // Text is mirrored

    // Non bold texts thickness is clamped at 1/6 char size by the low level draw function.
    // but in Pcbnew we do not manage bold texts and thickness up to 1/4 char size
    // (like bold text) and we manage the thickness.
    // So we set bold flag to true
    bool allow_bold = pt_texte->m_Bold || thickness;

    aPlotter->Text( pos, BLACK,
                   pt_texte->m_Text,
                   orient, size,
                   pt_texte->m_HJustify, pt_texte->m_VJustify,
                   thickness, pt_texte->m_Italic, allow_bold );
}


void PlotDimension( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts, DIMENSION* aDim, int aLayerMask,
                    EDA_DRAW_MODE_T trace_mode )
{
    if( (GetLayerMask( aDim->GetLayer() ) & aLayerMask) == 0 )
        return;

    DRAWSEGMENT draw;

    draw.SetWidth( (trace_mode==LINE) ? -1 : aDim->GetWidth() );
    draw.SetLayer( aDim->GetLayer() );

    PlotTextePcb( aPlotter, aPlotOpts, &aDim->m_Text, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( aDim->m_crossBarOx, aDim->m_crossBarOy ));
    draw.SetEnd(   wxPoint( aDim->m_crossBarFx, aDim->m_crossBarFy ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( aDim->m_featureLineGOx, aDim->m_featureLineGOy ));
    draw.SetEnd(   wxPoint( aDim->m_featureLineGFx, aDim->m_featureLineGFy ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( aDim->m_featureLineDOx, aDim->m_featureLineDOy ));
    draw.SetEnd(   wxPoint( aDim->m_featureLineDFx, aDim->m_featureLineDFy ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( aDim->m_arrowD1Ox, aDim->m_arrowD1Oy ));
    draw.SetEnd(   wxPoint( aDim->m_arrowD1Fx, aDim->m_arrowD1Fy ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( aDim->m_arrowD2Ox, aDim->m_arrowD2Oy ));
    draw.SetEnd(   wxPoint( aDim->m_arrowD2Fx, aDim->m_arrowD2Fy ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( aDim->m_arrowG1Ox, aDim->m_arrowG1Oy ));
    draw.SetEnd(   wxPoint( aDim->m_arrowG1Fx, aDim->m_arrowG1Fy ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( aDim->m_arrowG2Ox, aDim->m_arrowG2Oy ));
    draw.SetEnd(   wxPoint( aDim->m_arrowG2Fx, aDim->m_arrowG2Fy ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );
}


void PlotPcbTarget( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts, PCB_TARGET* aMire, int aLayerMask,
                    EDA_DRAW_MODE_T trace_mode )
{
    int     dx1, dx2, dy1, dy2, radius;

    if( (GetLayerMask( aMire->GetLayer() ) & aLayerMask) == 0 )
        return;

    DRAWSEGMENT  draw;

    draw.SetShape( S_CIRCLE );
    draw.SetWidth( ( trace_mode == LINE ) ? -1 : aMire->GetWidth() );
    draw.SetLayer( aMire->GetLayer() );

    draw.SetStart( aMire->GetPosition() );
    draw.SetEnd( wxPoint( draw.GetStart().x + ( aMire->GetSize() / 4 ), draw.GetStart().y ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetShape( S_SEGMENT );

    radius = aMire->GetSize() / 2;
    dx1    = radius;
    dy1    = 0;
    dx2    = 0;
    dy2    = radius;

    if( aMire->GetShape() )    // Shape X
    {
        dx1 = dy1 = ( radius * 7 ) / 5;
        dx2 = dx1;
        dy2 = -dy1;
    }

    wxPoint mirePos( aMire->GetPosition() );

    draw.SetStart( wxPoint( mirePos.x - dx1, mirePos.y - dy1 ));
    draw.SetEnd(   wxPoint( mirePos.x + dx1, mirePos.y + dy1 ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );

    draw.SetStart( wxPoint( mirePos.x - dx2, mirePos.y - dy2 ));
    draw.SetEnd(   wxPoint( mirePos.x + dx2, mirePos.y + dy2 ));
    PlotDrawSegment( aPlotter, aPlotOpts, &draw, aLayerMask, trace_mode );
}


// Plot footprints graphic items (outlines)
void Plot_Edges_Modules( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts, BOARD* aPcb, int aLayerMask, EDA_DRAW_MODE_T trace_mode )
{
    for( MODULE* module = aPcb->m_Modules;  module;  module = module->Next() )
    {
        for( EDGE_MODULE* edge = (EDGE_MODULE*) module->m_Drawings.GetFirst();
             edge;
             edge = edge->Next() )
        {
            if( edge->Type() != PCB_MODULE_EDGE_T )
                continue;

            if( ( GetLayerMask( edge->GetLayer() ) & aLayerMask ) == 0 )
                continue;

            Plot_1_EdgeModule( aPlotter, aPlotOpts, edge, trace_mode, aLayerMask );
        }
    }
}


//* Plot a graphic item (outline) relative to a footprint
void Plot_1_EdgeModule( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts,
        EDGE_MODULE* aEdge, EDA_DRAW_MODE_T trace_mode, int masque_layer )
{
    int     type_trace;         // Type of item to plot.
    int     thickness;          // Segment thickness.
    int     radius;             // Circle radius.

    if( aEdge->Type() != PCB_MODULE_EDGE_T )
        return;

    type_trace = aEdge->GetShape();
    thickness  = aEdge->GetWidth();

    wxPoint pos( aEdge->GetStart() );
    wxPoint end( aEdge->GetEnd() );

    switch( type_trace )
    {
    case S_SEGMENT:
        aPlotter->ThickSegment( pos, end, thickness, trace_mode );
        break;

    case S_CIRCLE:
        radius = (int) hypot( (double) ( end.x - pos.x ),
                              (double) ( end.y - pos.y ) );
        aPlotter->ThickCircle( pos, radius * 2, thickness, trace_mode );
        break;

    case S_ARC:
        {
            radius = (int) hypot( (double) ( end.x - pos.x ),
                                  (double) ( end.y - pos.y ) );

            double startAngle  = ArcTangente( end.y - pos.y, end.x - pos.x );

            double endAngle = startAngle + aEdge->GetAngle();

            if ( ( aPlotOpts.GetFormat() == PLOT_FORMAT_DXF ) &&
               ( masque_layer & ( SILKSCREEN_LAYER_BACK | DRAW_LAYER | COMMENT_LAYER ) ) )
                aPlotter->ThickArc( pos, -startAngle, -endAngle, radius,
                                thickness, trace_mode );
            else
                aPlotter->ThickArc( pos, -endAngle, -startAngle, radius,
                                thickness, trace_mode );
        }
        break;

    case S_POLYGON:
        {
            const std::vector<wxPoint>& polyPoints = aEdge->GetPolyPoints();

            if( polyPoints.size() <= 1 )  // Malformed polygon
                break;

            // We must compute true coordinates from m_PolyList
            // which are relative to module position, orientation 0
            MODULE* module = aEdge->GetParentModule();

            std::vector< wxPoint > cornerList;

            cornerList.reserve( polyPoints.size() );

            for( unsigned ii = 0; ii < polyPoints.size(); ii++ )
            {
                wxPoint corner = polyPoints[ii];

                if( module )
                {
                    RotatePoint( &corner, module->GetOrientation() );
                    corner += module->GetPosition();
                }

                cornerList.push_back( corner );
            }

            aPlotter->PlotPoly( cornerList, FILLED_SHAPE, thickness );
        }
        break;
    }
}


// Plot a PCB Text, i;e. a text found on a copper or technical layer
void PlotTextePcb( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts, TEXTE_PCB* pt_texte, int aLayerMask,
                   EDA_DRAW_MODE_T trace_mode )
{
    int     orient, thickness;
    wxPoint pos;
    wxSize  size;

    if( pt_texte->m_Text.IsEmpty() )
        return;

    if( ( GetLayerMask( pt_texte->GetLayer() ) & aLayerMask ) == 0 )
        return;

    size = pt_texte->m_Size;
    pos  = pt_texte->m_Pos;
    orient    = pt_texte->m_Orient;
    thickness = ( trace_mode==LINE ) ? -1 : pt_texte->m_Thickness;

    if( pt_texte->m_Mirror )
        size.x = -size.x;

    // Non bold texts thickness is clamped at 1/6 char size by the low level draw function.
    // but in Pcbnew we do not manage bold texts and thickness up to 1/4 char size
    // (like bold text) and we manage the thickness.
    // So we set bold flag to true
    bool allow_bold = pt_texte->m_Bold || thickness;

    if( pt_texte->m_MultilineAllowed )
    {
        wxArrayString* list = wxStringSplit( pt_texte->m_Text, '\n' );
        wxPoint        offset;

        offset.y = pt_texte->GetInterline();

        RotatePoint( &offset, orient );

        for( unsigned i = 0; i < list->Count(); i++ )
        {
            wxString txt = list->Item( i );
            aPlotter->Text( pos, BLACK, txt, orient, size,
                           pt_texte->m_HJustify, pt_texte->m_VJustify,
                           thickness, pt_texte->m_Italic, allow_bold );
            pos += offset;
        }

        delete list;
    }
    else
    {
        aPlotter->Text( pos, BLACK, pt_texte->m_Text, orient, size,
                       pt_texte->m_HJustify, pt_texte->m_VJustify,
                       thickness, pt_texte->m_Italic, allow_bold );
    }
}


/* Plot areas (given by .m_FilledPolysList member) in a zone
 */
void PlotFilledAreas( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts, ZONE_CONTAINER* aZone, EDA_DRAW_MODE_T trace_mode )
{
    std::vector<CPolyPt> polysList = aZone->GetFilledPolysList();
    unsigned imax = polysList.size();

    if( imax == 0 )  // Nothing to draw
        return;

    // We need a buffer to store corners coordinates:
    static std::vector< wxPoint > cornerList;
    cornerList.clear();

    /* Plot all filled areas: filled areas have a filled area and a thick
     * outline we must plot the filled area itself ( as a filled polygon
     * OR a set of segments ) and plot the thick outline itself
     *
     * in non filled mode the outline is plotted, but not the filling items
     */
    for( unsigned ic = 0; ic < imax; ic++ )
    {
        CPolyPt* corner = &polysList[ic];
        cornerList.push_back( wxPoint( corner->x, corner->y) );

        if( corner->end_contour )   // Plot the current filled area outline
        {
            // First, close the outline
            if( cornerList[0] != cornerList[cornerList.size() - 1] )
            {
                cornerList.push_back( cornerList[0] );
            }

            // Plot the current filled area and its outline
            if( trace_mode == FILLED )
            {
                // Plot the current filled area polygon
                if( aZone->m_FillMode == 0 )    // We are using solid polygons
                {                               // (if != 0: using segments )
                    aPlotter->PlotPoly( cornerList, FILLED_SHAPE );
                }
                else                            // We are using areas filled by
                {                               // segments: plot them )
                    for( unsigned iseg = 0; iseg < aZone->m_FillSegmList.size(); iseg++ )
                    {
                        wxPoint start = aZone->m_FillSegmList[iseg].m_Start;
                        wxPoint end   = aZone->m_FillSegmList[iseg].m_End;
                        aPlotter->ThickSegment( start, end,
                                                aZone->m_ZoneMinThickness,
                                                trace_mode );
                    }
                }

                // Plot the current filled area outline
                if( aZone->m_ZoneMinThickness > 0 )
                    aPlotter->PlotPoly( cornerList, NO_FILL, aZone->m_ZoneMinThickness );
            }
            else
            {
                if( aZone->m_ZoneMinThickness > 0 )
                {
                    for( unsigned jj = 1; jj<cornerList.size(); jj++ )
                        aPlotter->ThickSegment( cornerList[jj -1], cornerList[jj],
                                                ( trace_mode == LINE ) ? -1 : aZone->m_ZoneMinThickness,
                                                trace_mode );
                }

                aPlotter->SetCurrentLineWidth( -1 );
            }

            cornerList.clear();
        }
    }
}


/* Plot items type DRAWSEGMENT on layers allowed by aLayerMask
 */
void PlotDrawSegment( PLOTTER* aPlotter, const PCB_PLOT_PARAMS& aPlotOpts, DRAWSEGMENT* aSeg, int aLayerMask,
                      EDA_DRAW_MODE_T trace_mode )
{
    int     thickness;
    int     radius = 0, StAngle = 0, EndAngle = 0;

    if( (GetLayerMask( aSeg->GetLayer() ) & aLayerMask) == 0 )
        return;

    if( trace_mode == LINE )
        thickness = aPlotOpts.GetLineWidth();
    else
        thickness = aSeg->GetWidth();

    wxPoint start( aSeg->GetStart() );
    wxPoint end(   aSeg->GetEnd() );

    aPlotter->SetCurrentLineWidth( thickness );

    switch( aSeg->GetShape() )
    {
    case S_CIRCLE:
        radius = (int) hypot( (double) ( end.x - start.x ),
                              (double) ( end.y - start.y ) );
        aPlotter->ThickCircle( start, radius * 2, thickness, trace_mode );
        break;

    case S_ARC:
        radius = (int) hypot( (double) ( end.x - start.x ),
                              (double) ( end.y - start.y ) );
        StAngle  = ArcTangente( end.y - start.y, end.x - start.x );
        EndAngle = StAngle + aSeg->GetAngle();
        aPlotter->ThickArc( start, -EndAngle, -StAngle, radius, thickness, trace_mode );
        break;

    case S_CURVE:
        {
            const std::vector<wxPoint>& bezierPoints = aSeg->GetBezierPoints();

            for( unsigned i = 1; i < bezierPoints.size(); i++ )
                aPlotter->ThickSegment( bezierPoints[i - 1],
                                        bezierPoints[i],
                                        thickness,
                                        trace_mode );
        }
        break;

    default:
        aPlotter->ThickSegment( start, end, thickness, trace_mode );
    }
}


void PCB_BASE_FRAME::Plot_Layer( PLOTTER* aPlotter, int Layer, EDA_DRAW_MODE_T trace_mode )
{
    const PCB_PLOT_PARAMS& plot_opts = GetPlotSettings();

    // Specify that the contents of the "Edges Pcb" layer are to be plotted
    // in addition to the contents of the currently specified layer.
    int layer_mask = GetLayerMask( Layer );

    if( !plot_opts.GetExcludeEdgeLayer() )
        layer_mask |= EDGE_LAYER;

    switch( Layer )
    {
    case FIRST_COPPER_LAYER:
    case LAYER_N_2:
    case LAYER_N_3:
    case LAYER_N_4:
    case LAYER_N_5:
    case LAYER_N_6:
    case LAYER_N_7:
    case LAYER_N_8:
    case LAYER_N_9:
    case LAYER_N_10:
    case LAYER_N_11:
    case LAYER_N_12:
    case LAYER_N_13:
    case LAYER_N_14:
    case LAYER_N_15:
    case LAST_COPPER_LAYER:
        // The last true make it skip NPTH pad plotting
        Plot_Standard_Layer( aPlotter, layer_mask, true, trace_mode, true );

        // Adding drill marks, if required and if the plotter is able to plot them:
        if( plot_opts.GetDrillMarksType() != PCB_PLOT_PARAMS::NO_DRILL_SHAPE )
        {
            if( aPlotter->GetPlotterType() == PLOT_FORMAT_POST )
                PlotDrillMark( aPlotter, trace_mode,
                               plot_opts.GetDrillMarksType() ==
                               PCB_PLOT_PARAMS::SMALL_DRILL_SHAPE );
        }

        break;

    case SOLDERMASK_N_BACK:
    case SOLDERMASK_N_FRONT:
        Plot_Standard_Layer( aPlotter, layer_mask,
                             plot_opts.GetPlotViaOnMaskLayer(), trace_mode );
        break;

    case SOLDERPASTE_N_BACK:
    case SOLDERPASTE_N_FRONT:
        Plot_Standard_Layer( aPlotter, layer_mask, false, trace_mode );
        break;

    case SILKSCREEN_N_FRONT:
    case SILKSCREEN_N_BACK:
        PlotSilkScreen( aPlotter, layer_mask, trace_mode );

        // Gerber: Subtract soldermask from silkscreen if enabled
        if( aPlotter->GetPlotterType() == PLOT_FORMAT_GERBER
            && plot_opts.GetSubtractMaskFromSilk() )
        {
            if( Layer == SILKSCREEN_N_FRONT )
            {
                layer_mask = GetLayerMask( SOLDERMASK_N_FRONT );
            }
            else
            {
                layer_mask = GetLayerMask( SOLDERMASK_N_BACK );
            }

            // Set layer polarity to negative
            aPlotter->SetLayerPolarity( false );
            Plot_Standard_Layer( aPlotter, layer_mask,
                                 plot_opts.GetPlotViaOnMaskLayer(),
                                 trace_mode );
        }

        break;

    default:
        PlotSilkScreen( aPlotter, layer_mask, trace_mode );
        break;
    }
}


/* Plot a copper layer or mask.
 * Silk screen layers are not plotted here.
 */
void PCB_BASE_FRAME::Plot_Standard_Layer( PLOTTER*        aPlotter,
                                          int             aLayerMask,
                                          bool            aPlotVia,
                                          EDA_DRAW_MODE_T aPlotMode,
                                          bool            aSkipNPTH_Pads  )
{
    wxPoint  pos;
    wxSize   size;
    wxString msg;

    const PCB_PLOT_PARAMS& plot_opts = GetPlotSettings();

    // Plot pcb draw items.
    for( BOARD_ITEM* item = m_Pcb->m_Drawings; item; item = item->Next() )
    {
        switch( item->Type() )
        {
        case PCB_LINE_T:
            PlotDrawSegment( aPlotter, plot_opts, (DRAWSEGMENT*) item, aLayerMask, aPlotMode );
            break;

        case PCB_TEXT_T:
            PlotTextePcb( aPlotter, plot_opts, (TEXTE_PCB*) item, aLayerMask, aPlotMode );
            break;

        case PCB_DIMENSION_T:
            PlotDimension( aPlotter, plot_opts, (DIMENSION*) item, aLayerMask, aPlotMode );
            break;

        case PCB_TARGET_T:
            PlotPcbTarget( aPlotter, plot_opts, (PCB_TARGET*) item, aLayerMask, aPlotMode );
            break;

        case PCB_MARKER_T:
            break;

        default:
            wxMessageBox( wxT( "Plot_Standard_Layer() error : Unexpected Draw Type" ) );
            break;
        }
    }

    // Draw footprint shapes without pads (pads will plotted later)
    for( MODULE* module = m_Pcb->m_Modules;  module;  module = module->Next() )
    {
        for( BOARD_ITEM* item = module->m_Drawings; item; item = item->Next() )
        {
            switch( item->Type() )
            {
            case PCB_MODULE_EDGE_T:
                if( aLayerMask & GetLayerMask(  item->GetLayer() ) )
                    Plot_1_EdgeModule( aPlotter, plot_opts, (EDGE_MODULE*) item, aPlotMode, aLayerMask );

                break;

            default:
                break;
            }
        }
    }

    // Plot footprint pads
    for( MODULE* module = m_Pcb->m_Modules;  module;  module = module->Next() )
    {
        for( D_PAD* pad = module->m_Pads;  pad;  pad = pad->Next() )
        {
            if( (pad->GetLayerMask() & aLayerMask) == 0 )
                continue;

            wxPoint shape_pos = pad->ReturnShapePos();

            pos = shape_pos;

            wxSize margin;
            double width_adj = 0;

            if( aLayerMask & ALL_CU_LAYERS )
            {
                width_adj =  aPlotter->GetPlotWidthAdj();
            }

            switch( aLayerMask &
                   ( SOLDERMASK_LAYER_BACK | SOLDERMASK_LAYER_FRONT |
                     SOLDERPASTE_LAYER_BACK | SOLDERPASTE_LAYER_FRONT ) )
            {
            case SOLDERMASK_LAYER_FRONT:
            case SOLDERMASK_LAYER_BACK:
                margin.x = margin.y = pad->GetSolderMaskMargin();
                break;

            case SOLDERPASTE_LAYER_FRONT:
            case SOLDERPASTE_LAYER_BACK:
                margin = pad->GetSolderPasteMargin();
                break;

            default:
                break;
            }

            size.x = pad->GetSize().x + ( 2 * margin.x ) + width_adj;
            size.y = pad->GetSize().y + ( 2 * margin.y ) + width_adj;

            // Don't draw a null size item :
            if( size.x <= 0 || size.y <= 0 )
                continue;

            switch( pad->GetShape() )
            {
            case PAD_CIRCLE:
                if( aSkipNPTH_Pads &&
                    (pad->GetSize() == pad->GetDrillSize()) &&
                    (pad->GetAttribute() == PAD_HOLE_NOT_PLATED) )
                    break;

                aPlotter->FlashPadCircle( pos, size.x, aPlotMode );
                break;

            case PAD_OVAL:
                if( aSkipNPTH_Pads &&
                    (pad->GetSize() == pad->GetDrillSize()) &&
                    (pad->GetAttribute() == PAD_HOLE_NOT_PLATED) )
                    break;

                aPlotter->FlashPadOval( pos, size, pad->GetOrientation(), aPlotMode );
                break;

            case PAD_TRAPEZOID:
            {
                wxPoint coord[4];
                pad->BuildPadPolygon( coord, margin, 0 );
                aPlotter->FlashPadTrapez( pos, coord, pad->GetOrientation(), aPlotMode );
            }
            break;

            case PAD_RECT:
            default:
                aPlotter->FlashPadRect( pos, size, pad->GetOrientation(), aPlotMode );
                break;
            }
        }
    }

    // Plot vias :
    if( aPlotVia )
    {
        for( TRACK* track = m_Pcb->m_Track; track; track = track->Next() )
        {
            if( track->Type() != PCB_VIA_T )
                continue;

            SEGVIA* Via = (SEGVIA*) track;

            // vias are not plotted if not on selected layer, but if layer
            // is SOLDERMASK_LAYER_BACK or SOLDERMASK_LAYER_FRONT,vias are drawn,
            // if they are on an external copper layer
            int via_mask_layer = Via->ReturnMaskLayer();

            if( via_mask_layer & LAYER_BACK )
                via_mask_layer |= SOLDERMASK_LAYER_BACK;

            if( via_mask_layer & LAYER_FRONT )
                via_mask_layer |= SOLDERMASK_LAYER_FRONT;

            if( ( via_mask_layer & aLayerMask ) == 0 )
                continue;

            int via_margin = 0;
            double width_adj = 0;

            // If the current layer is a solder mask, use the global mask
            // clearance for vias
            if( ( aLayerMask & ( SOLDERMASK_LAYER_BACK | SOLDERMASK_LAYER_FRONT ) ) )
                via_margin = GetBoard()->GetDesignSettings().m_SolderMaskMargin;

            if( aLayerMask & ALL_CU_LAYERS )
            {
                width_adj =  aPlotter->GetPlotWidthAdj();
            }

            pos    = Via->m_Start;
            size.x = size.y = Via->m_Width + 2 * via_margin + width_adj;

            // Don't draw a null size item :
            if( size.x <= 0 )
                continue;

            aPlotter->FlashPadCircle( pos, size.x, aPlotMode );
        }
    }

    // Plot tracks (not vias) :
    for( TRACK* track = m_Pcb->m_Track; track; track = track->Next() )
    {
        wxPoint end;

        if( track->Type() == PCB_VIA_T )
            continue;

        if( (GetLayerMask( track->GetLayer() ) & aLayerMask) == 0 )
            continue;

        size.x = size.y = track->m_Width + aPlotter->GetPlotWidthAdj();
        pos    = track->m_Start;
        end    = track->m_End;

        aPlotter->ThickSegment( pos, end, size.x, aPlotMode );
    }

    // Plot zones (outdated, for old boards compatibility):
    for( TRACK* track = m_Pcb->m_Zone; track; track = track->Next() )
    {
        wxPoint end;

        if( (GetLayerMask( track->GetLayer() ) & aLayerMask) == 0 )
            continue;

        size.x = size.y = track->m_Width + aPlotter->GetPlotWidthAdj();
        pos    = track->m_Start;
        end    = track->m_End;

        aPlotter->ThickSegment( pos, end, size.x, aPlotMode );
    }

    // Plot filled ares
    for( int ii = 0; ii < m_Pcb->GetAreaCount(); ii++ )
    {
        ZONE_CONTAINER* edge_zone = m_Pcb->GetArea( ii );

        if( ( ( 1 << edge_zone->GetLayer() ) & aLayerMask ) == 0 )
            continue;

        PlotFilledAreas( aPlotter, plot_opts, edge_zone, aPlotMode );
    }
}


/**
 * Function PlotDrillMark
 * Draw a drill mark for pads and vias.
 * Must be called after all drawings, because it
 * redraw the drill mark on a pad or via, as a negative (i.e. white) shape in
 * FILLED plot mode
 * @param aPlotter = the PLOTTER
 * @param aTraceMode = the mode of plot (FILLED, SKETCH)
 * @param aSmallDrillShape = true to plot a small drill shape, false to plot
 * the actual drill shape
 */
void PCB_BASE_FRAME::PlotDrillMark( PLOTTER*        aPlotter,
                                    EDA_DRAW_MODE_T aTraceMode,
                                    bool            aSmallDrillShape )
{
    wxPoint   pos;
    wxSize    diam;
    MODULE*   Module;
    D_PAD*    pad;
    TRACK*    pts;

    const PCB_PLOT_PARAMS& plot_opts = GetPlotSettings();

    if( aTraceMode == FILLED )
    {
        aPlotter->SetColor( WHITE );
    }

    for( pts = m_Pcb->m_Track; pts != NULL; pts = pts->Next() )
    {
        if( pts->Type() != PCB_VIA_T )
            continue;

        pos = pts->m_Start;

        // It is quite possible that the real drill value is less then small drill value.
        if( plot_opts.GetDrillMarksType() == PCB_PLOT_PARAMS::SMALL_DRILL_SHAPE )
            diam.x = diam.y = MIN( SMALL_DRILL, pts->GetDrillValue() );
        else
            diam.x = diam.y = pts->GetDrillValue();

        diam.x -= aPlotter->GetPlotWidthAdj();
        diam.x = Clamp( 1, diam.x, pts->m_Width - 1 );
        aPlotter->FlashPadCircle( pos, diam.x, aTraceMode );
    }

    for( Module = m_Pcb->m_Modules; Module != NULL; Module = Module->Next() )
    {
        for( pad = Module->m_Pads; pad != NULL; pad = pad->Next() )
        {
            if( pad->GetDrillSize().x == 0 )
                continue;

            // Output hole shapes:
            pos = pad->GetPosition();

            if( pad->GetDrillShape() == PAD_OVAL )
            {
                diam = pad->GetDrillSize();
                diam.x -= aPlotter->GetPlotWidthAdj();
                diam.x = Clamp( 1, diam.x, pad->GetSize().x - 1 );
                diam.y -= aPlotter->GetPlotWidthAdj();
                diam.y = Clamp( 1, diam.y, pad->GetSize().y - 1 );
                aPlotter->FlashPadOval( pos, diam, pad->GetOrientation(), aTraceMode );
            }
            else
            {
                // It is quite possible that the real pad drill value is less then small drill value.
                diam.x = aSmallDrillShape ? MIN( SMALL_DRILL, pad->GetDrillSize().x ) : pad->GetDrillSize().x;
                diam.x -= aPlotter->GetPlotWidthAdj();
                diam.x = Clamp( 1, diam.x, pad->GetSize().x - 1 );
                aPlotter->FlashPadCircle( pos, diam.x, aTraceMode );
            }
        }
    }

    if( aTraceMode == FILLED )
    {
        aPlotter->SetColor( BLACK );
    }
}
