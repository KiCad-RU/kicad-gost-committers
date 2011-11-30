/**
 * @file plot_rtn.cpp
 * @brief Common plot routines.
 */

#include "fctsys.h"
#include "common.h"
#include "plot_common.h"
#include "base_struct.h"
#include "drawtxt.h"
#include "confirm.h"
#include "trigo.h"
#include "wxBasePcbFrame.h"
#include "pcbcommon.h"
#include "macros.h"

#include "class_board.h"
#include "class_module.h"
#include "class_track.h"
#include "class_edge_mod.h"
#include "class_pcb_text.h"
#include "class_zone.h"
#include "class_drawsegment.h"
#include "class_mire.h"
#include "class_dimension.h"

#include "pcbnew.h"
#include "pcbplot.h"

static void Plot_Edges_Modules( PLOTTER* plotter, BOARD* pcb, int aLayerMask,
                                GRTraceMode trace_mode );
static void PlotTextModule( PLOTTER* plotter, TEXTE_MODULE* pt_texte,
                            GRTraceMode trace_mode );


/* Creates the plot for silkscreen layers
 */
void PCB_BASE_FRAME::PlotSilkScreen( PLOTTER* plotter, int aLayerMask, GRTraceMode trace_mode )
{
    bool          trace_val, trace_ref;
    TEXTE_MODULE* pt_texte;
    EDA_ITEM*     PtStruct;

    /* Plot edge layer and graphic items */

    for( PtStruct = m_Pcb->m_Drawings; PtStruct != NULL; PtStruct = PtStruct->Next() )
    {
        switch( PtStruct->Type() )
        {
        case PCB_LINE_T:
            PlotDrawSegment( plotter, (DRAWSEGMENT*) PtStruct, aLayerMask, trace_mode );
            break;

        case PCB_TEXT_T:
            PlotTextePcb( plotter, (TEXTE_PCB*) PtStruct, aLayerMask, trace_mode );
            break;

        case PCB_DIMENSION_T:
            PlotDimension( plotter, (DIMENSION*) PtStruct, aLayerMask, trace_mode );
            break;

        case PCB_TARGET_T:
            PlotPcbTarget( plotter, (PCB_TARGET*) PtStruct, aLayerMask, trace_mode );
            break;

        case PCB_MARKER_T:
            break;

        default:
            DisplayError( this, wxT( "PlotSilkScreen() error: unexpected Type()" ) );
            break;
        }
    }

    /* Plot footprint outlines : */
    Plot_Edges_Modules( plotter, m_Pcb, aLayerMask, trace_mode );

    /* Plot pads (creates pads outlines, for pads on silkscreen layers) */
    int layersmask_plotpads = aLayerMask;
    // Calculate the mask layers of allowed layers for pads

    if( !g_PcbPlotOptions.m_PlotPadsOnSilkLayer )       // Do not plot pads on silk screen layers
        layersmask_plotpads &= ~(SILKSCREEN_LAYER_BACK | SILKSCREEN_LAYER_FRONT );

    if( layersmask_plotpads )
    {
        for( MODULE* Module = m_Pcb->m_Modules; Module; Module = Module->Next() )
        {
            for( D_PAD * pad = Module->m_Pads; pad != NULL; pad = pad->Next() )
            {
                /* See if the pad is on this layer */
                if( (pad->m_layerMask & layersmask_plotpads) == 0 )
                    continue;

                wxPoint shape_pos = pad->ReturnShapePos();

                switch( pad->m_PadShape & 0x7F )
                {
                case PAD_CIRCLE:
                    plotter->flash_pad_circle( shape_pos, pad->m_Size.x, FILAIRE );
                    break;

                case PAD_OVAL:
                    plotter->flash_pad_oval( shape_pos, pad->m_Size, pad->m_Orient, FILAIRE );
                    break;

                case PAD_TRAPEZOID:
                {
                    wxPoint coord[4];
                    pad->BuildPadPolygon( coord, wxSize(0,0), 0 );
                    plotter->flash_pad_trapez( shape_pos, coord, pad->m_Orient, FILAIRE );
                    break;
                }

                case PAD_RECT:
                default:
                    plotter->flash_pad_rect( shape_pos, pad->m_Size, pad->m_Orient, FILAIRE );
                    break;
                }
            }
        }
    }

    /* Plot footprints fields (ref, value ...) */
    for( MODULE* Module = m_Pcb->m_Modules; Module; Module = Module->Next() )
    {
        /* see if we want to plot VALUE and REF fields */
        trace_val = g_PcbPlotOptions.m_PlotValue;
        trace_ref = g_PcbPlotOptions.m_PlotReference;

        TEXTE_MODULE* text = Module->m_Reference;
        unsigned      textLayer = text->GetLayer();

        if( textLayer >= 32 )
        {
            wxString errMsg;

            errMsg.Printf( _( "Your BOARD has a bad layer number of %u for \
module\n %s's \"reference\" text." ),
                           textLayer, GetChars( Module->GetReference() ) );
            DisplayError( this, errMsg );
            return;
        }

        if( ( ( 1 << textLayer ) & aLayerMask ) == 0 )
            trace_ref = false;

        if( text->m_NoShow && !g_PcbPlotOptions.m_PlotInvisibleTexts )
            trace_ref = false;

        text = Module->m_Value;
        textLayer = text->GetLayer();

        if( textLayer > 32 )
        {
            wxString errMsg;

            errMsg.Printf( _( "Your BOARD has a bad layer number of %u for \
module\n %s's \"value\" text." ),
                           textLayer, GetChars( Module->GetReference() ) );
            DisplayError( this, errMsg );
            return;
        }

        if( ( (1 << textLayer) & aLayerMask ) == 0 )
            trace_val = false;

        if( text->m_NoShow && !g_PcbPlotOptions.m_PlotInvisibleTexts )
            trace_val = false;

        /* Plot text fields, if allowed */
        if( trace_ref )
            PlotTextModule( plotter, Module->m_Reference, trace_mode );

        if( trace_val )
            PlotTextModule( plotter, Module->m_Value, trace_mode );

        for( pt_texte = (TEXTE_MODULE*) Module->m_Drawings.GetFirst();
             pt_texte != NULL;
             pt_texte = pt_texte->Next() )
        {
            if( pt_texte->Type() != PCB_MODULE_TEXT_T )
                continue;

            if( !g_PcbPlotOptions.m_PlotTextOther )
                continue;

            if( (pt_texte->m_NoShow) && !g_PcbPlotOptions.m_PlotInvisibleTexts )
                continue;

            textLayer = pt_texte->GetLayer();

            if( textLayer >= 32 )
            {
                wxString errMsg;

                errMsg.Printf( _( "Your BOARD has a bad layer number of %u \
for module\n %s's \"module text\" text of %s." ),
                               textLayer, GetChars( Module->GetReference() ),
                               GetChars( pt_texte->m_Text ) );
                DisplayError( this, errMsg );
                return;
            }

            if( !( ( 1 << textLayer ) & aLayerMask ) )
                continue;

            PlotTextModule( plotter, pt_texte, trace_mode );
        }
    }

    /* Plot filled areas */
    for( int ii = 0; ii < m_Pcb->GetAreaCount(); ii++ )
    {
        ZONE_CONTAINER* edge_zone = m_Pcb->GetArea( ii );

        if( ( ( 1 << edge_zone->GetLayer() ) & aLayerMask ) == 0 )
            continue;

        PlotFilledAreas( plotter, edge_zone, trace_mode );
    }

    // Plot segments used to fill zone areas (outdated, but here for old boards
    // compatibility):
    for( SEGZONE* seg = m_Pcb->m_Zone; seg != NULL; seg = seg->Next() )
    {
        if( ( ( 1 << seg->GetLayer() ) & aLayerMask ) == 0 )
            continue;

        plotter->thick_segment( seg->m_Start, seg->m_End, seg->m_Width, trace_mode );
    }
}


static void PlotTextModule( PLOTTER* plotter, TEXTE_MODULE* pt_texte, GRTraceMode trace_mode )
{
    wxSize  size;
    wxPoint pos;
    int     orient, thickness;

    /* calculate some text parameters :*/
    size = pt_texte->m_Size;
    pos  = pt_texte->m_Pos;

    orient = pt_texte->GetDrawRotation();

    thickness = pt_texte->m_Thickness;

    if( trace_mode == FILAIRE )
        thickness = -1;

    if( pt_texte->m_Mirror )
        NEGATE( size.x );  // Text is mirrored

    // Non bold texts thickness is clamped at 1/6 char size by the low level draw function.
    // but in Pcbnew we do not manage bold texts and thickness up to 1/4 char size
    // (like bold text) and we manage the thickness.
    // So we set bold flag to true
    bool allow_bold = pt_texte->m_Bold || thickness;

    plotter->text( pos, BLACK,
                   pt_texte->m_Text,
                   orient, size,
                   pt_texte->m_HJustify, pt_texte->m_VJustify,
                   thickness, pt_texte->m_Italic, allow_bold );
}


void PlotDimension( PLOTTER* plotter, DIMENSION* Dimension, int aLayerMask,
                    GRTraceMode trace_mode )
{
    DRAWSEGMENT* DrawTmp;

    if( (g_TabOneLayerMask[Dimension->GetLayer()] & aLayerMask) == 0 )
        return;

    DrawTmp = new DRAWSEGMENT( NULL );

    DrawTmp->m_Width = (trace_mode==FILAIRE) ? -1 : Dimension->m_Width;
    DrawTmp->SetLayer( Dimension->GetLayer() );

    PlotTextePcb( plotter, Dimension->m_Text, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Dimension->m_crossBarOx;
    DrawTmp->m_Start.y = Dimension->m_crossBarOy;
    DrawTmp->m_End.x = Dimension->m_crossBarFx;
    DrawTmp->m_End.y = Dimension->m_crossBarFy;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Dimension->m_featureLineGOx;
    DrawTmp->m_Start.y = Dimension->m_featureLineGOy;
    DrawTmp->m_End.x = Dimension->m_featureLineGFx;
    DrawTmp->m_End.y = Dimension->m_featureLineGFy;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Dimension->m_featureLineDOx;
    DrawTmp->m_Start.y = Dimension->m_featureLineDOy;
    DrawTmp->m_End.x = Dimension->m_featureLineDFx;
    DrawTmp->m_End.y = Dimension->m_featureLineDFy;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Dimension->m_arrowD1Ox;
    DrawTmp->m_Start.y = Dimension->m_arrowD1Oy;
    DrawTmp->m_End.x = Dimension->m_arrowD1Fx;
    DrawTmp->m_End.y = Dimension->m_arrowD1Fy;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Dimension->m_arrowD2Ox;
    DrawTmp->m_Start.y = Dimension->m_arrowD2Oy;
    DrawTmp->m_End.x = Dimension->m_arrowD2Fx;
    DrawTmp->m_End.y = Dimension->m_arrowD2Fy;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Dimension->m_arrowG1Ox;
    DrawTmp->m_Start.y = Dimension->m_arrowG1Oy;
    DrawTmp->m_End.x = Dimension->m_arrowG1Fx;
    DrawTmp->m_End.y = Dimension->m_arrowG1Fy;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Dimension->m_arrowG2Ox;
    DrawTmp->m_Start.y = Dimension->m_arrowG2Oy;
    DrawTmp->m_End.x = Dimension->m_arrowG2Fx;
    DrawTmp->m_End.y = Dimension->m_arrowG2Fy;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    delete DrawTmp;
}


void PlotPcbTarget( PLOTTER* plotter, PCB_TARGET* Mire, int aLayerMask, GRTraceMode trace_mode )
{
    DRAWSEGMENT* DrawTmp;
    int          dx1, dx2, dy1, dy2, radius;

    if( (g_TabOneLayerMask[Mire->GetLayer()] & aLayerMask) == 0 )
        return;

    DrawTmp = new DRAWSEGMENT( NULL );

    DrawTmp->m_Width = ( trace_mode == FILAIRE ) ? -1 : Mire->m_Width;
    DrawTmp->SetLayer( Mire->GetLayer() );

    DrawTmp->m_Start.x = Mire->m_Pos.x; DrawTmp->m_Start.y = Mire->m_Pos.y;
    DrawTmp->m_End.x   = DrawTmp->m_Start.x + ( Mire->m_Size / 4 );
    DrawTmp->m_End.y   = DrawTmp->m_Start.y;
    DrawTmp->m_Shape   = S_CIRCLE;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Shape = S_SEGMENT;

    radius = Mire->m_Size / 2;
    dx1    = radius;
    dy1    = 0;
    dx2    = 0;
    dy2    = radius;

    if( Mire->m_Shape ) /* Shape X */
    {
        dx1 = dy1 = ( radius * 7 ) / 5;
        dx2 = dx1;
        dy2 = -dy1;
    }

    DrawTmp->m_Start.x = Mire->m_Pos.x - dx1;
    DrawTmp->m_Start.y = Mire->m_Pos.y - dy1;
    DrawTmp->m_End.x = Mire->m_Pos.x + dx1;
    DrawTmp->m_End.y = Mire->m_Pos.y + dy1;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    DrawTmp->m_Start.x = Mire->m_Pos.x - dx2;
    DrawTmp->m_Start.y = Mire->m_Pos.y - dy2;
    DrawTmp->m_End.x = Mire->m_Pos.x + dx2;
    DrawTmp->m_End.y = Mire->m_Pos.y + dy2;
    PlotDrawSegment( plotter, DrawTmp, aLayerMask, trace_mode );

    delete DrawTmp;
}


/* Plot footprints graphic items (outlines) */
void Plot_Edges_Modules( PLOTTER* plotter, BOARD* pcb, int aLayerMask, GRTraceMode trace_mode )
{
    for( MODULE* module = pcb->m_Modules; module; module = module->Next() )
    {
        for( EDGE_MODULE* edge = (EDGE_MODULE*) module->m_Drawings.GetFirst();
             edge;
             edge = edge->Next() )
        {
            if( edge->Type() != PCB_MODULE_EDGE_T )
                continue;

            if( ( g_TabOneLayerMask[edge->GetLayer()] & aLayerMask ) == 0 )
                continue;

            Plot_1_EdgeModule( plotter, edge, trace_mode );
        }
    }
}


/* Plot a graphic item (outline) relative to a footprint */
void Plot_1_EdgeModule( PLOTTER* plotter, EDGE_MODULE* PtEdge, GRTraceMode trace_mode )
{
    int     type_trace;         /* Type of item to plot. */
    int     thickness;          /* Segment thickness. */
    int     radius;             /* Circle radius. */
    int     StAngle, EndAngle;
    wxPoint pos, end;

    if( PtEdge->Type() != PCB_MODULE_EDGE_T )
        return;

    type_trace = PtEdge->m_Shape;
    thickness  = PtEdge->m_Width;

    pos = PtEdge->m_Start;
    end = PtEdge->m_End;

    switch( type_trace )
    {
    case S_SEGMENT:
        plotter->thick_segment( pos, end, thickness, trace_mode );
        break;

    case S_CIRCLE:
        radius = (int) hypot( (double) ( end.x - pos.x ),
                              (double) ( end.y - pos.y ) );
        plotter->thick_circle( pos, radius * 2, thickness, trace_mode );
        break;

    case S_ARC:
        radius = (int) hypot( (double) ( end.x - pos.x ),
                              (double) ( end.y - pos.y ) );
        StAngle  = ArcTangente( end.y - pos.y, end.x - pos.x );
        EndAngle = StAngle + PtEdge->m_Angle;
        plotter->thick_arc( pos,
                            -EndAngle,
                            -StAngle,
                            radius,
                            thickness,
                            trace_mode );
        break;

    case S_POLYGON:
    {
        std::vector<wxPoint> polyPoints = PtEdge->GetPolyPoints();

        if( polyPoints.size() <= 1 )  // Malformed polygon
            break;

        // We must compute true coordinates from m_PolyList
        // which are relative to module position, orientation 0
        MODULE* module = PtEdge->GetParentModule();

        static std::vector< wxPoint > cornerList;
        cornerList.clear();

        for( unsigned ii = 0; ii < polyPoints.size(); ii++ )
        {
            wxPoint corner = polyPoints[ii];

            if( module )
            {
                RotatePoint( &corner, module->m_Orient );
                corner += module->m_Pos;
            }

            cornerList.push_back( corner );
        }

        plotter->PlotPoly( cornerList, FILLED_SHAPE, thickness );
    }
    break;
    }
}


/* Plot a PCB Text, i;e. a text found on a copper or technical layer */
void PlotTextePcb( PLOTTER* plotter, TEXTE_PCB* pt_texte, int aLayerMask, GRTraceMode trace_mode )
{
    int     orient, thickness;
    wxPoint pos;
    wxSize  size;

    if( pt_texte->m_Text.IsEmpty() )
        return;

    if( ( g_TabOneLayerMask[pt_texte->GetLayer()] & aLayerMask ) == 0 )
        return;

    size = pt_texte->m_Size;
    pos  = pt_texte->m_Pos;
    orient    = pt_texte->m_Orient;
    thickness = ( trace_mode==FILAIRE ) ? -1 : pt_texte->m_Thickness;

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
            plotter->text( pos, BLACK,
                           txt,
                           orient, size,
                           pt_texte->m_HJustify, pt_texte->m_VJustify,
                           thickness, pt_texte->m_Italic, allow_bold );
            pos += offset;
        }

        delete (list);
    }
    else
    {
        plotter->text( pos, BLACK,
                       pt_texte->m_Text,
                       orient, size,
                       pt_texte->m_HJustify, pt_texte->m_VJustify,
                       thickness, pt_texte->m_Italic, allow_bold );
    }
}


/* Plot areas (given by .m_FilledPolysList member) in a zone
 */
void PlotFilledAreas( PLOTTER* plotter, ZONE_CONTAINER* aZone, GRTraceMode trace_mode )
{
    unsigned        imax = aZone->m_FilledPolysList.size();

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
        CPolyPt* corner = &aZone->m_FilledPolysList[ic];
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
                    plotter->PlotPoly( cornerList, FILLED_SHAPE );
                }
                else                            // We are using areas filled by
                {                               // segments: plot them )
                    for( unsigned iseg = 0; iseg < aZone->m_FillSegmList.size(); iseg++ )
                    {
                        wxPoint start = aZone->m_FillSegmList[iseg].m_Start;
                        wxPoint end   = aZone->m_FillSegmList[iseg].m_End;
                        plotter->thick_segment( start,
                                                end,
                                                aZone->m_ZoneMinThickness,
                                                trace_mode );
                    }
                }

                // Plot the current filled area outline
                if( aZone->m_ZoneMinThickness > 0 )
                    plotter->PlotPoly( cornerList, NO_FILL, aZone->m_ZoneMinThickness );
            }
            else
            {
                if( aZone->m_ZoneMinThickness > 0 )
                {
                    for( unsigned jj = 1; jj<cornerList.size(); jj++ )
                        plotter->thick_segment( cornerList[jj -1], cornerList[jj],
                                                ( trace_mode == FILAIRE ) ? -1 : aZone->m_ZoneMinThickness,
                                                trace_mode );
                }

                plotter->set_current_line_width( -1 );
            }

            cornerList.clear();
        }
    }
}


/* Plot items type DRAWSEGMENT on layers allowed by aLayerMask
 */
void PlotDrawSegment( PLOTTER* plotter, DRAWSEGMENT* pt_segm, int aLayerMask,
                      GRTraceMode trace_mode )
{
    wxPoint start, end;
    int     thickness;
    int     radius = 0, StAngle = 0, EndAngle = 0;

    if( (g_TabOneLayerMask[pt_segm->GetLayer()] & aLayerMask) == 0 )
        return;

    if( trace_mode == FILAIRE )
        thickness = g_PcbPlotOptions.m_PlotLineWidth;
    else
        thickness = pt_segm->m_Width;

    start = pt_segm->m_Start;
    end   = pt_segm->m_End;

    plotter->set_current_line_width( thickness );

    switch( pt_segm->m_Shape )
    {
    case S_CIRCLE:
        radius = (int) hypot( (double) ( end.x - start.x ),
                              (double) ( end.y - start.y ) );
        plotter->thick_circle( start, radius * 2, thickness, trace_mode );
        break;

    case S_ARC:
        radius = (int) hypot( (double) ( end.x - start.x ),
                              (double) ( end.y - start.y ) );
        StAngle  = ArcTangente( end.y - start.y, end.x - start.x );
        EndAngle = StAngle + pt_segm->m_Angle;
        plotter->thick_arc( start, -EndAngle, -StAngle, radius, thickness, trace_mode );
        break;

    case S_CURVE:
    {
        std::vector<wxPoint> bezierPoints = pt_segm->GetBezierPoints();
        for( unsigned i = 1; i < bezierPoints.size(); i++ )
            plotter->thick_segment( bezierPoints[i - 1],
                                    bezierPoints[i],
                                    thickness,
                                    trace_mode );
        break;
    }

    default:
        plotter->thick_segment( start, end, thickness, trace_mode );
    }
}


void PCB_BASE_FRAME::Plot_Layer( PLOTTER* plotter, int Layer, GRTraceMode trace_mode )
{
    // Specify that the contents of the "Edges Pcb" layer are to be plotted
    // in addition to the contents of the currently specified layer.
    int layer_mask = g_TabOneLayerMask[Layer];

    if( !g_PcbPlotOptions.m_ExcludeEdgeLayer )
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
        Plot_Standard_Layer( plotter, layer_mask, true, trace_mode,
                             g_PcbPlotOptions.m_SkipNPTH_Pads );

        // Adding drill marks, if required and if the plotter is able to plot them:
        if( g_PcbPlotOptions.m_DrillShapeOpt != PCB_PLOT_PARAMS::NO_DRILL_SHAPE )
        {
            if( plotter->GetPlotterType() == PLOT_FORMAT_POST )
                PlotDrillMark( plotter, trace_mode,
                               g_PcbPlotOptions.m_DrillShapeOpt ==
                               PCB_PLOT_PARAMS::SMALL_DRILL_SHAPE );
        }

        break;

    case SOLDERMASK_N_BACK:
    case SOLDERMASK_N_FRONT:
        Plot_Standard_Layer( plotter, layer_mask,
                             g_PcbPlotOptions.m_PlotViaOnMaskLayer, trace_mode );
        break;

    case SOLDERPASTE_N_BACK:
    case SOLDERPASTE_N_FRONT:
        Plot_Standard_Layer( plotter, layer_mask, false, trace_mode );
        break;

    case SILKSCREEN_N_FRONT:
    case SILKSCREEN_N_BACK:
        PlotSilkScreen( plotter, layer_mask, trace_mode );

        // Gerber: Subtract soldermask from silkscreen if enabled
        if( plotter->GetPlotterType() == PLOT_FORMAT_GERBER
            && g_PcbPlotOptions.GetSubtractMaskFromSilk() )
        {
            if( Layer == SILKSCREEN_N_FRONT )
            {
                layer_mask = g_TabOneLayerMask[SOLDERMASK_N_FRONT];
            }
            else
            {
                layer_mask = g_TabOneLayerMask[SOLDERMASK_N_BACK];
            }

            // Set layer polarity to negative
            plotter->SetLayerPolarity( false );
            Plot_Standard_Layer( plotter, layer_mask,
                                 g_PcbPlotOptions.m_PlotViaOnMaskLayer,
                                 trace_mode );
        }

        break;

    default:
        PlotSilkScreen( plotter, layer_mask, trace_mode );
        break;
    }
}


/* Plot a copper layer or mask in HPGL format.
 * HPGL unit = 0.98 mils (1 mil = 1.02041 unit HPGL).
 */
void PCB_BASE_FRAME::Plot_Standard_Layer( PLOTTER*    aPlotter,
                                          int         aLayerMask,
                                          bool        aPlotVia,
                                          GRTraceMode aPlotMode,
                                          bool aSkipNPTH_Pads  )
{
    wxPoint  pos;
    wxSize   size;
    wxString msg;

    // Plot pcb draw items.
    for( BOARD_ITEM* item = m_Pcb->m_Drawings; item; item = item->Next() )
    {
        switch( item->Type() )
        {
        case PCB_LINE_T:
            PlotDrawSegment( aPlotter, (DRAWSEGMENT*) item, aLayerMask, aPlotMode );
            break;

        case PCB_TEXT_T:
            PlotTextePcb( aPlotter, (TEXTE_PCB*) item, aLayerMask, aPlotMode );
            break;

        case PCB_DIMENSION_T:
            PlotDimension( aPlotter, (DIMENSION*) item, aLayerMask, aPlotMode );
            break;

        case PCB_TARGET_T:
            PlotPcbTarget( aPlotter, (PCB_TARGET*) item, aLayerMask, aPlotMode );
            break;

        case PCB_MARKER_T:
            break;

        default:
            wxMessageBox( wxT( "Plot_Standard_Layer() error : Unexpected Draw Type" ) );
            break;
        }
    }

    /* Draw footprint shapes without pads (pads will plotted later) */
    for( MODULE* module = m_Pcb->m_Modules; module; module = module->Next() )
    {
        for( BOARD_ITEM* item = module->m_Drawings; item; item = item->Next() )
        {
            switch( item->Type() )
            {
            case PCB_MODULE_EDGE_T:
                if( aLayerMask & g_TabOneLayerMask[ item->GetLayer() ] )
                    Plot_1_EdgeModule( aPlotter, (EDGE_MODULE*) item, aPlotMode );

                break;

            default:
                break;
            }
        }
    }

    /* Plot footprint pads */
    for( MODULE* module = m_Pcb->m_Modules; module; module = module->Next() )
    {
        for( D_PAD* pad = module->m_Pads; pad; pad = pad->Next() )
        {
            wxPoint shape_pos;

            if( (pad->m_layerMask & aLayerMask) == 0 )
                continue;

            shape_pos = pad->ReturnShapePos();
            pos = shape_pos;
            wxSize margin;

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

            size.x = pad->m_Size.x + ( 2 * margin.x );
            size.y = pad->m_Size.y + ( 2 * margin.y );

            /* Don't draw a null size item : */
            if( size.x <= 0 || size.y <= 0 )
                continue;

            switch( pad->m_PadShape )
            {
            case PAD_CIRCLE:
                if( aSkipNPTH_Pads &&
                    (pad->m_Size == pad->m_Drill) &&
                    (pad->m_Attribut == PAD_HOLE_NOT_PLATED) )
                    break;

                aPlotter->flash_pad_circle( pos, size.x, aPlotMode );
                break;

            case PAD_OVAL:
                if( aSkipNPTH_Pads &&
                    (pad->m_Size == pad->m_Drill) &&
                    (pad->m_Attribut == PAD_HOLE_NOT_PLATED) )
                    break;

                aPlotter->flash_pad_oval( pos, size, pad->m_Orient, aPlotMode );
                break;

            case PAD_TRAPEZOID:
            {
                wxPoint coord[4];
                pad->BuildPadPolygon( coord, margin, 0 );
                aPlotter->flash_pad_trapez( pos, coord, pad->m_Orient, aPlotMode );
            }
            break;

            case PAD_RECT:
            default:
                aPlotter->flash_pad_rect( pos, size, pad->m_Orient, aPlotMode );
                break;
            }
        }
    }

    /* Plot vias : */
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

            // If the current layer is a solder mask, use the global mask
            // clearance for vias
            if( ( aLayerMask & ( SOLDERMASK_LAYER_BACK | SOLDERMASK_LAYER_FRONT ) ) )
                via_margin = GetBoard()->GetBoardDesignSettings()->m_SolderMaskMargin;

            pos    = Via->m_Start;
            size.x = size.y = Via->m_Width + 2 * via_margin;

            /* Don't draw a null size item : */
            if( size.x <= 0 )
                continue;

            aPlotter->flash_pad_circle( pos, size.x, aPlotMode );
        }
    }

    /* Plot tracks (not vias) : */
    for( TRACK* track = m_Pcb->m_Track; track; track = track->Next() )
    {
        wxPoint end;

        if( track->Type() == PCB_VIA_T )
            continue;

        if( (g_TabOneLayerMask[track->GetLayer()] & aLayerMask) == 0 )
            continue;

        size.x = size.y = track->m_Width;
        pos    = track->m_Start;
        end    = track->m_End;

        aPlotter->thick_segment( pos, end, size.x, aPlotMode );
    }

    /* Plot zones (outdated, for old boards compatibility): */
    for( TRACK* track = m_Pcb->m_Zone; track; track = track->Next() )
    {
        wxPoint end;

        if( (g_TabOneLayerMask[track->GetLayer()] & aLayerMask) == 0 )
            continue;

        size.x = size.y = track->m_Width;
        pos    = track->m_Start;
        end    = track->m_End;

        aPlotter->thick_segment( pos, end, size.x, aPlotMode );
    }

    /* Plot filled ares */
    for( int ii = 0; ii < m_Pcb->GetAreaCount(); ii++ )
    {
        ZONE_CONTAINER* edge_zone = m_Pcb->GetArea( ii );

        if( ( ( 1 << edge_zone->GetLayer() ) & aLayerMask ) == 0 )
            continue;

        PlotFilledAreas( aPlotter, edge_zone, aPlotMode );
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
void PCB_BASE_FRAME::PlotDrillMark( PLOTTER*    aPlotter,
                                    GRTraceMode aTraceMode,
                                    bool        aSmallDrillShape )
{
    wxPoint   pos;
    wxSize    diam;
    MODULE*   Module;
    D_PAD*    PtPad;
    TRACK*    pts;

    if( aTraceMode == FILLED )
    {
        aPlotter->set_color( WHITE );
    }

    for( pts = m_Pcb->m_Track; pts != NULL; pts = pts->Next() )
    {
        if( pts->Type() != PCB_VIA_T )
            continue;

        pos = pts->m_Start;

        if( g_PcbPlotOptions.m_DrillShapeOpt == PCB_PLOT_PARAMS::SMALL_DRILL_SHAPE )
            diam.x = diam.y = SMALL_DRILL;
        else
            diam.x = diam.y = pts->GetDrillValue();

        aPlotter->flash_pad_circle( pos, diam.x, aTraceMode );
    }

    for( Module = m_Pcb->m_Modules; Module != NULL; Module = Module->Next() )
    {
        for( PtPad = Module->m_Pads; PtPad != NULL; PtPad = PtPad->Next() )
        {
            if( PtPad->m_Drill.x == 0 )
                continue;

            // Output hole shapes:
            pos = PtPad->m_Pos;

            if( PtPad->m_DrillShape == PAD_OVAL )
            {
                diam = PtPad->m_Drill;
                aPlotter->flash_pad_oval( pos, diam, PtPad->m_Orient, aTraceMode );
            }
            else
            {
                diam.x = aSmallDrillShape ? SMALL_DRILL : PtPad->m_Drill.x;
                aPlotter->flash_pad_circle( pos, diam.x, aTraceMode );
            }
        }
    }

    if( aTraceMode == FILLED )
    {
        aPlotter->set_color( BLACK );
    }
}
