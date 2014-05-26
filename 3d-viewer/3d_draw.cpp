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

/**
 * @file 3d_draw.cpp
 */

#include <fctsys.h>
#include <common.h>
#include <trigo.h>
#include <pcbstruct.h>
#include <drawtxt.h>
#include <layers_id_colors_and_visibility.h>

#include <wxBasePcbFrame.h>
#include <class_board.h>
#include <class_module.h>
#include <class_track.h>
#include <class_edge_mod.h>
#include <class_zone.h>
#include <class_drawsegment.h>
#include <class_pcb_text.h>
#include <colors_selection.h>
#include <convert_basic_shapes_to_polygon.h>

#include <3d_viewer.h>
#include <3d_canvas.h>
#include <info3d_visu.h>
#include <trackball.h>
#include <3d_draw_basic_functions.h>

// Imported function:
extern void     CheckGLError();

/* Helper function
 * returns true if aLayer should be displayed, false otherwise
 */
static bool     Is3DLayerEnabled( LAYER_NUM aLayer );

/* returns the Z orientation parameter 1.0 or -1.0 for aLayer
 * Z orientation is 1.0 for all layers but "back" layers:
 *  LAYER_N_BACK , ADHESIVE_N_BACK, SOLDERPASTE_N_BACK ), SILKSCREEN_N_BACK
 * used to calculate the Z orientation parameter for glNormal3f
 */
static GLfloat  Get3DLayer_Z_Orientation( LAYER_NUM aLayer );

/* Helper function BuildPadShapeThickOutlineAsPolygon:
 * Build a pad shape outline as polygon, to draw pads on silkscreen layer
 * with a line thickness = aWidth
 * Used only to draw pads outlines on silkscreen layers.
 */
static void BuildPadShapeThickOutlineAsPolygon( D_PAD*          aPad,
                                                CPOLYGONS_LIST& aCornerBuffer,
                                                int             aWidth,
                                                int             aCircleToSegmentsCount,
                                                double          aCorrectionFactor )
{
    if( aPad->GetShape() == PAD_CIRCLE )    // Draw a ring
    {
        TransformRingToPolygon( aCornerBuffer, aPad->ShapePos(),
                                aPad->GetSize().x / 2, aCircleToSegmentsCount, aWidth );
        return;
    }

    // For other shapes, draw polygon outlines
    CPOLYGONS_LIST corners;
    aPad->BuildPadShapePolygon( corners, wxSize( 0, 0 ),
                                aCircleToSegmentsCount, aCorrectionFactor );

    // Add outlines as thick segments in polygon buffer
    for( unsigned ii = 0, jj = corners.GetCornersCount() - 1;
         ii < corners.GetCornersCount(); jj = ii, ii++ )
    {
        TransformRoundedEndsSegmentToPolygon( aCornerBuffer,
                                              corners.GetPos( jj ),
                                              corners.GetPos( ii ),
                                              aCircleToSegmentsCount, aWidth );
    }
}


void EDA_3D_CANVAS::Redraw()
{
    // SwapBuffer requires the window to be shown before calling
    if( !IsShown() )
        return;

    SetCurrent( *m_glRC );

    // Set the OpenGL viewport according to the client size of this canvas.
    // This is done here rather than in a wxSizeEvent handler because our
    // OpenGL rendering context (and thus viewport setting) is used with
    // multiple canvases: If we updated the viewport in the wxSizeEvent
    // handler, changing the size of one canvas causes a viewport setting that
    // is wrong when next another canvas is repainted.
    const wxSize ClientSize = GetClientSize();

    // *MUST* be called *after*  SetCurrent( ):
    glViewport( 0, 0, ClientSize.x, ClientSize.y );

    InitGL();

    glMatrixMode( GL_MODELVIEW );    // position viewer
    // transformations
    GLfloat mat[4][4];

    // Translate motion first, so rotations don't mess up the orientation...
    glTranslatef( m_draw3dOffset.x, m_draw3dOffset.y, 0.0F );

    build_rotmatrix( mat, g_Parm_3D_Visu.m_Quat );
    glMultMatrixf( &mat[0][0] );

    glRotatef( g_Parm_3D_Visu.m_Rot[0], 1.0, 0.0, 0.0 );
    glRotatef( g_Parm_3D_Visu.m_Rot[1], 0.0, 1.0, 0.0 );
    glRotatef( g_Parm_3D_Visu.m_Rot[2], 0.0, 0.0, 1.0 );

    if( ! m_glLists[GL_ID_BOARD] || ! m_glLists[GL_ID_TECH_LAYERS] )
        CreateDrawGL_List();

    if( g_Parm_3D_Visu.GetFlag( FL_AXIS ) && m_glLists[GL_ID_AXIS] )
        glCallList( m_glLists[GL_ID_AXIS] );

    // move the board in order to draw it with its center at 0,0 3D coordinates
    glTranslatef( -g_Parm_3D_Visu.m_BoardPos.x * g_Parm_3D_Visu.m_BiuTo3Dunits,
                  -g_Parm_3D_Visu.m_BoardPos.y * g_Parm_3D_Visu.m_BiuTo3Dunits,
                  0.0F );

    // draw all objects in lists
    // transparent objects should be drawn after opaque objects
    glCallList( m_glLists[GL_ID_BOARD] );
    glCallList( m_glLists[GL_ID_TECH_LAYERS] );

    if( g_Parm_3D_Visu.GetFlag( FL_COMMENTS ) || g_Parm_3D_Visu.GetFlag( FL_COMMENTS )  )
    {
        if( ! m_glLists[GL_ID_AUX_LAYERS] )
            CreateDrawGL_List();

        glCallList( m_glLists[GL_ID_AUX_LAYERS] );
    }

    if( g_Parm_3D_Visu.GetFlag( FL_MODULE ) )
    {
        if( ! m_glLists[GL_ID_3DSHAPES_SOLID] )
            CreateDrawGL_List();

        glCallList( m_glLists[GL_ID_3DSHAPES_SOLID] );
    }

    // Grid uses transparency: draw it after all objects
    if( g_Parm_3D_Visu.GetFlag( FL_GRID ) && m_glLists[GL_ID_GRID] )
        glCallList( m_glLists[GL_ID_GRID] );

    // This list must be drawn last, because it contains the
    // transparent gl objects, which should be drawn after all
    // non transparent objects
    if(  g_Parm_3D_Visu.GetFlag( FL_MODULE ) && m_glLists[GL_ID_3DSHAPES_TRANSP] )
        glCallList( m_glLists[GL_ID_3DSHAPES_TRANSP] );

    SwapBuffers();
}

// Helper function: initialize the copper color to draw the board
// in realistic mode.
static inline void SetGLCopperColor()
{
    // Generates a golden yellow color, near board "copper" color
    const double lum = 0.7/255.0;
    glColor4f( 255.0*lum, 223.0*lum, 0.0*lum, 1.0 );
}

// Helper function: initialize the color to draw the epoxy layers
// ( body board and solder mask layers) in realistic mode.
static inline void SetGLEpoxyColor( double aTransparency = 1.0 )
{
    // Generates an epoxy color, near board color
    const double lum = 0.2/255.0;
    glColor4f( 100.0*lum, 255.0*lum, 180.0*lum, aTransparency );
}

// Helper function: initialize the color to draw the non copper layers
// in realistic mode and normal mode.
static inline void SetGLTechLayersColor( LAYER_NUM aLayer )
{
    if( g_Parm_3D_Visu.IsRealisticMode() )
    {
        switch( aLayer )
        {
        case SOLDERPASTE_N_BACK:
        case SOLDERPASTE_N_FRONT:
            SetGLColor( DARKGRAY, 0.7 );
            break;

        case SILKSCREEN_N_BACK:
        case SILKSCREEN_N_FRONT:
            SetGLColor( LIGHTGRAY, 0.9 );
            break;

        case SOLDERMASK_N_BACK:
        case SOLDERMASK_N_FRONT:
            SetGLEpoxyColor( 0.7 );
            break;

        default:
            EDA_COLOR_T color = g_ColorsSettings.GetLayerColor( aLayer );
            SetGLColor( color, 0.7 );
            break;
        }
    }
    else
    {
        EDA_COLOR_T color = g_ColorsSettings.GetLayerColor( aLayer );
        SetGLColor( color, 0.7 );
    }
}


void EDA_3D_CANVAS::BuildBoard3DView()
{
    BOARD* pcb = GetBoard();
    bool realistic_mode = g_Parm_3D_Visu.IsRealisticMode();

    // Number of segments to convert a circle to polygon
    // Boost polygon (at least v 1.54, v1.55 and previous) in very rare cases crashes
    // when using 16 segments to approximate a circle.
    // So using 18 segments is a workaround to try to avoid these crashes
    // ( We already used this trick in plot_board_layers.cpp,
    // see PlotSolderMaskLayer() )
    const int       segcountforcircle   = 18;
    double          correctionFactor    = 1.0 / cos( M_PI / (segcountforcircle * 2) );
    const int       segcountLowQuality  = 12;   // segments to draw a circle with low quality
                                                // to reduce time calculations
                                                // for holes and items which do not need
                                                // a fine representation
    double          correctionFactorLQ = 1.0 / cos( M_PI / (segcountLowQuality * 2) );

    CPOLYGONS_LIST  bufferPolys;
    bufferPolys.reserve( 200000 );              // Reserve for large board (tracks mainly)

    CPOLYGONS_LIST  bufferPcbOutlines;          // stores the board main outlines
    CPOLYGONS_LIST  allLayerHoles;              // Contains through holes, calculated only once
    allLayerHoles.reserve( 20000 );

    // Build a polygon from edge cut items
    wxString msg;

    if( !pcb->GetBoardPolygonOutlines( bufferPcbOutlines, allLayerHoles, &msg ) )
    {
        msg << wxT("\n\n") <<
            _("Unable to calculate the board outlines.\n"
              "Therefore use the board boundary box.");
        wxMessageBox( msg );
    }

    CPOLYGONS_LIST  bufferZonesPolys;
    bufferZonesPolys.reserve( 500000 );             // Reserve for large board ( copper zones mainly )

    CPOLYGONS_LIST  currLayerHoles;                 // Contains holes for the current layer
    bool            throughHolesListBuilt = false;  // flag to build the through hole polygon list only once
    bool            hightQualityMode = false;

    for( LAYER_NUM layer = FIRST_COPPER_LAYER; layer <= LAST_COPPER_LAYER;
         ++layer )
    {
        if( layer != LAST_COPPER_LAYER
            && layer >= g_Parm_3D_Visu.m_CopperLayersCount )
            continue;

        // Skip non enabled layers in normal mode,
        // and internal layers in realistic mode
        if( !Is3DLayerEnabled( layer ) )
            continue;

        bufferPolys.RemoveAllContours();
        bufferZonesPolys.RemoveAllContours();
        currLayerHoles.RemoveAllContours();

        // Draw tracks:
        for( TRACK* track = pcb->m_Track; track != NULL; track = track->Next() )
        {
            if( !track->IsOnLayer( layer ) )
                continue;

            track->TransformShapeWithClearanceToPolygon( bufferPolys,
                                                         0, segcountforcircle,
                                                         correctionFactor );

            // Add via hole
            if( track->Type() == PCB_VIA_T )
            {
                VIA *via = static_cast<VIA*>( track );
                VIATYPE_T viatype = via->GetViaType();
                int holediameter = via->GetDrillValue();
                int thickness = g_Parm_3D_Visu.GetCopperThicknessBIU();
                int hole_outer_radius = (holediameter + thickness) / 2;

                if( viatype != VIA_THROUGH )
                    TransformCircleToPolygon( currLayerHoles,
                                              via->GetStart(), hole_outer_radius,
                                              segcountLowQuality );
                else if( !throughHolesListBuilt )
                    TransformCircleToPolygon( allLayerHoles,
                                              via->GetStart(), hole_outer_radius,
                                              segcountLowQuality );
            }
        }

        // draw pads
        for( MODULE* module = pcb->m_Modules; module != NULL; module = module->Next() )
        {
            module->TransformPadsShapesWithClearanceToPolygon( layer,
                                                               bufferPolys,
                                                               0,
                                                               segcountforcircle,
                                                               correctionFactor );

            // Micro-wave modules may have items on copper layers
            module->TransformGraphicShapesWithClearanceToPolygonSet( layer,
                                                                     bufferPolys,
                                                                     0,
                                                                     segcountforcircle,
                                                                     correctionFactor );

            // Add pad hole, if any
            if( !throughHolesListBuilt )
            {
                D_PAD* pad = module->Pads();

                for( ; pad != NULL; pad = pad->Next() )
                    pad->BuildPadDrillShapePolygon( allLayerHoles, 0,
                                                    segcountLowQuality );
            }
        }

        // Draw copper zones
        if( g_Parm_3D_Visu.GetFlag( FL_ZONE ) )
        {
            for( int ii = 0; ii < pcb->GetAreaCount(); ii++ )
            {
                ZONE_CONTAINER* zone = pcb->GetArea( ii );
                LAYER_NUM       zonelayer = zone->GetLayer();

                if( zonelayer == layer )
                {
                    zone->TransformSolidAreasShapesToPolygonSet(
                        hightQualityMode ? bufferPolys : bufferZonesPolys,
                        segcountLowQuality, correctionFactorLQ );
                }
            }
        }

        // draw graphic items on copper layers (texts)
        for( BOARD_ITEM* item = pcb->m_Drawings; item; item = item->Next() )
        {
            if( !item->IsOnLayer( layer ) )
                continue;

            switch( item->Type() )
            {
            case PCB_LINE_T:    // should not exist on copper layers
                ( (DRAWSEGMENT*) item )->TransformShapeWithClearanceToPolygon(
                    bufferPolys, 0, segcountforcircle, correctionFactor );
                break;

            case PCB_TEXT_T:
                ( (TEXTE_PCB*) item )->TransformShapeWithClearanceToPolygonSet(
                    bufferPolys, 0, segcountforcircle, correctionFactor );
                break;

            default:
                break;
            }
        }

        // bufferPolys contains polygons to merge. Many overlaps .
        // Calculate merged polygons
        if( bufferPolys.GetCornersCount() == 0 )
            continue;

        KI_POLYGON_SET  currLayerPolyset;
        KI_POLYGON_SET  polysetHoles;

        // Add polygons, without holes
        bufferPolys.ExportTo( currLayerPolyset );

        // Add holes in polygon list
        currLayerHoles.Append( allLayerHoles );

        if( currLayerHoles.GetCornersCount() > 0 )
            currLayerHoles.ExportTo( polysetHoles );

        // Merge polygons, remove holes
        currLayerPolyset -= polysetHoles;

        int         thickness = g_Parm_3D_Visu.GetLayerObjectThicknessBIU( layer );
        int         zpos = g_Parm_3D_Visu.GetLayerZcoordBIU( layer );

        if( realistic_mode )
            SetGLCopperColor();
        else
        {
            EDA_COLOR_T color = g_ColorsSettings.GetLayerColor( layer );
            SetGLColor( color );
        }

        glNormal3f( 0.0, 0.0, Get3DLayer_Z_Orientation( layer ) );

        bufferPolys.RemoveAllContours();
        bufferPolys.ImportFrom( currLayerPolyset );
        Draw3D_SolidHorizontalPolyPolygons( bufferPolys, zpos,
                                            thickness,
                                            g_Parm_3D_Visu.m_BiuTo3Dunits );

        if( bufferZonesPolys.GetCornersCount() )
            Draw3D_SolidHorizontalPolyPolygons( bufferZonesPolys, zpos,
                                                thickness,
                                                g_Parm_3D_Visu.m_BiuTo3Dunits );

        throughHolesListBuilt = true;
    }

    // Draw vias holes (vertical cylinders)
    for( const TRACK* track = pcb->m_Track; track != NULL; track = track->Next() )
    {
        const VIA *via = dynamic_cast<const VIA*>(track);

        if( via )
            Draw3DViaHole( via );
    }

    // Draw pads holes (vertical cylinders)
    for( const MODULE* module = pcb->m_Modules; module != NULL; module = module->Next() )
    {
        for( D_PAD* pad = module->Pads(); pad != NULL; pad = pad->Next() )
            Draw3DPadHole( pad );
    }

    // Draw board substrate:
    if( bufferPcbOutlines.GetCornersCount() &&
        ( realistic_mode || g_Parm_3D_Visu.GetFlag( FL_SHOW_BOARD_BODY ) ) )
    {
        int copper_thickness = g_Parm_3D_Visu.GetCopperThicknessBIU();
        // a small offset between substrate and external copper layer to avoid artifacts
        // when drawing copper items on board
        int epsilon = Millimeter2iu( 0.01 );
        int zpos = g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_BACK );
        int board_thickness = g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_FRONT )
                            - g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_BACK );
        // items on copper layers and having a thickness = copper_thickness
        // are drawn from zpos - copper_thickness/2 to zpos + copper_thickness
        // therefore substrate position is copper_thickness/2 to
        // substrate_height - copper_thickness/2
        zpos += (copper_thickness + epsilon) / 2;
        board_thickness -= copper_thickness + epsilon;

        if( realistic_mode )
            SetGLEpoxyColor();
        else
        {
            EDA_COLOR_T color = g_ColorsSettings.GetLayerColor( EDGE_N );
            SetGLColor( color, 0.7 );
        }

        glNormal3f( 0.0, 0.0, Get3DLayer_Z_Orientation( LAYER_N_FRONT ) );
        KI_POLYGON_SET  currLayerPolyset;
        KI_POLYGON_SET  polysetHoles;

        // Add polygons, without holes
        bufferPcbOutlines.ExportTo( currLayerPolyset );

        // Build holes list
        allLayerHoles.ExportTo( polysetHoles );

        // remove holes
        currLayerPolyset -= polysetHoles;

        bufferPcbOutlines.RemoveAllContours();
        bufferPcbOutlines.ImportFrom( currLayerPolyset );

        // for Draw3D_SolidHorizontalPolyPolygons, zpos it the middle between bottom and top
        // sides
        Draw3D_SolidHorizontalPolyPolygons( bufferPcbOutlines, zpos + board_thickness/2,
                                            board_thickness, g_Parm_3D_Visu.m_BiuTo3Dunits );
    }
}


void EDA_3D_CANVAS::BuildTechLayers3DView()
{
    BOARD* pcb = GetBoard();

    // Number of segments to draw a circle using segments
    const int       segcountforcircle   = 16;
    double          correctionFactor    = 1.0 / cos( M_PI / (segcountforcircle * 2) );
    const int       segcountLowQuality  = 12;   // segments to draw a circle with low quality
                                                // to reduce time calculations
                                                // for holes and items which do not need
                                                // a fine representation
    double          correctionFactorLQ = 1.0 / cos( M_PI / (segcountLowQuality * 2) );

    CPOLYGONS_LIST  bufferPolys;
    bufferPolys.reserve( 100000 );              // Reserve for large board
    CPOLYGONS_LIST  allLayerHoles;              // Contains through holes, calculated only once
    allLayerHoles.reserve( 20000 );

    CPOLYGONS_LIST  bufferPcbOutlines;          // stores the board main outlines
    // Build a polygon from edge cut items
    wxString msg;

    if( !pcb->GetBoardPolygonOutlines( bufferPcbOutlines, allLayerHoles, &msg ) )
    {
        msg << wxT("\n\n") <<
            _("Unable to calculate the board outlines.\n"
              "Therefore use the board boundary box.");
        wxMessageBox( msg );
    }

    int thickness = g_Parm_3D_Visu.GetCopperThicknessBIU();

    // Add via holes
    for( VIA* via = GetFirstVia( pcb->m_Track ); via != NULL;
            via = GetFirstVia( via->Next() ) )
    {
        VIATYPE_T viatype = via->GetViaType();
        int holediameter = via->GetDrillValue();
        int hole_outer_radius = (holediameter + thickness) / 2;

        if( viatype == VIA_THROUGH )
            TransformCircleToPolygon( allLayerHoles,
                    via->GetStart(), hole_outer_radius,
                    segcountLowQuality );
    }

    // draw pads holes
    for( MODULE* module = pcb->m_Modules; module != NULL; module = module->Next() )
    {
        // Add pad hole, if any
        D_PAD* pad = module->Pads();

        for( ; pad != NULL; pad = pad->Next() )
            pad->BuildPadDrillShapePolygon( allLayerHoles, 0,
                                                segcountLowQuality );
    }

    // draw graphic items, on technical layers

    KI_POLYGON_SET  brdpolysetHoles;
    allLayerHoles.ExportTo( brdpolysetHoles );

    for( LAYER_NUM layer = FIRST_NON_COPPER_LAYER; layer <= LAST_NON_COPPER_LAYER;
         ++layer )
    {
        // Skip user layers, which are not drawn here
        if( IsUserLayer( layer) )
            continue;

        if( !Is3DLayerEnabled( layer ) )
            continue;

        if( layer == EDGE_N && g_Parm_3D_Visu.GetFlag( FL_SHOW_BOARD_BODY )  )
            continue;

        bufferPolys.RemoveAllContours();

        for( BOARD_ITEM* item = pcb->m_Drawings; item; item = item->Next() )
        {
            if( !item->IsOnLayer( layer ) )
                continue;

            switch( item->Type() )
            {
            case PCB_LINE_T:
                ( (DRAWSEGMENT*) item )->TransformShapeWithClearanceToPolygon(
                    bufferPolys, 0, segcountforcircle, correctionFactor );
                break;

            case PCB_TEXT_T:
                ( (TEXTE_PCB*) item )->TransformShapeWithClearanceToPolygonSet(
                    bufferPolys, 0, segcountforcircle, correctionFactor );
                break;

            default:
                break;
            }
        }

        for( MODULE* module = pcb->m_Modules; module != NULL; module = module->Next() )
        {
            if( layer == SILKSCREEN_N_FRONT || layer == SILKSCREEN_N_BACK )
            {
                D_PAD*  pad = module->Pads();
                int     linewidth = g_DrawDefaultLineThickness;

                for( ; pad != NULL; pad = pad->Next() )
                {
                    if( !pad->IsOnLayer( layer ) )
                        continue;

                    BuildPadShapeThickOutlineAsPolygon( pad, bufferPolys,
                            linewidth, segcountforcircle, correctionFactor );
                }
            }
            else
                module->TransformPadsShapesWithClearanceToPolygon( layer,
                        bufferPolys, 0, segcountforcircle, correctionFactor );

            module->TransformGraphicShapesWithClearanceToPolygonSet( layer,
                    bufferPolys, 0, segcountforcircle, correctionFactor );
        }

        // Draw non copper zones
        if( g_Parm_3D_Visu.GetFlag( FL_ZONE ) )
        {
            for( int ii = 0; ii < pcb->GetAreaCount(); ii++ )
            {
                ZONE_CONTAINER* zone = pcb->GetArea( ii );

                if( !zone->IsOnLayer( layer ) )
                    continue;

                zone->TransformSolidAreasShapesToPolygonSet(
                        bufferPolys, segcountLowQuality, correctionFactorLQ );
            }
        }

        // bufferPolys contains polygons to merge. Many overlaps .
        // Calculate merged polygons and remove pads and vias holes
        if( bufferPolys.GetCornersCount() == 0 )
            continue;
        KI_POLYGON_SET  currLayerPolyset;
        KI_POLYGON_SET  polyset;

        // Solder mask layers are "negative" layers.
        // Shapes should be removed from the full board area.
        if( layer == SOLDERMASK_N_BACK || layer == SOLDERMASK_N_FRONT )
        {
            bufferPcbOutlines.ExportTo( currLayerPolyset );
            bufferPolys.Append( allLayerHoles );
            bufferPolys.ExportTo( polyset );
            currLayerPolyset -= polyset;
        }
        // Remove holes from Solder paste layers and siklscreen
        else if( layer == SOLDERPASTE_N_BACK || layer == SOLDERPASTE_N_FRONT
                 || layer == SILKSCREEN_N_BACK || layer == SILKSCREEN_N_FRONT  )
        {
            bufferPolys.ExportTo( currLayerPolyset );
            currLayerPolyset -= brdpolysetHoles;
        }
        else    // usuall layers, merge polys built from each item shape:
        {
            bufferPolys.ExportTo( polyset );
            currLayerPolyset += polyset;
        }

        int         thickness = g_Parm_3D_Visu.GetLayerObjectThicknessBIU( layer );
        int         zpos = g_Parm_3D_Visu.GetLayerZcoordBIU( layer );

        if( layer == EDGE_N )
        {
            thickness = g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_FRONT )
                        - g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_BACK );
            zpos = g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_BACK )
                   + (thickness / 2);
        }
        else
        {
            // for Draw3D_SolidHorizontalPolyPolygons, zpos it the middle between bottom and top
            // sides.
            // However for top layers, zpos should be the bottom layer pos,
            // and for bottom layers, zpos should be the top layer pos.
            if( Get3DLayer_Z_Orientation( layer ) > 0 )
                zpos += thickness/2;
            else
                zpos -= thickness/2 ;
        }

        bufferPolys.RemoveAllContours();
        bufferPolys.ImportFrom( currLayerPolyset );

        SetGLTechLayersColor( layer );
        glNormal3f( 0.0, 0.0, Get3DLayer_Z_Orientation( layer ) );
        Draw3D_SolidHorizontalPolyPolygons( bufferPolys, zpos,
                                            thickness, g_Parm_3D_Visu.m_BiuTo3Dunits );
    }
}

/**
 * Function BuildBoard3DAuxLayers
 * Called by CreateDrawGL_List()
 * Fills the OpenGL GL_ID_BOARD draw list with items
 * on aux layers only
 */
void EDA_3D_CANVAS::BuildBoard3DAuxLayers()
{
    const int   segcountforcircle   = 16;
    double      correctionFactor    = 1.0 / cos( M_PI / (segcountforcircle * 2) );
    BOARD* pcb = GetBoard();
    CPOLYGONS_LIST  bufferPolys;
    bufferPolys.reserve( 5000 );    // Reserve for items not on board

    for( LAYER_NUM layer = FIRST_USER_LAYER; layer <= LAST_USER_LAYER;
         ++layer )
    {
        if( !Is3DLayerEnabled( layer ) )
            continue;

        bufferPolys.RemoveAllContours();

        for( BOARD_ITEM* item = pcb->m_Drawings; item; item = item->Next() )
        {
            if( !item->IsOnLayer( layer ) )
                continue;

            switch( item->Type() )
            {
            case PCB_LINE_T:
                ( (DRAWSEGMENT*) item )->TransformShapeWithClearanceToPolygon(
                    bufferPolys, 0, segcountforcircle, correctionFactor );
                break;

            case PCB_TEXT_T:
                ( (TEXTE_PCB*) item )->TransformShapeWithClearanceToPolygonSet(
                    bufferPolys, 0, segcountforcircle, correctionFactor );
                break;

            default:
                break;
            }
        }

        for( MODULE* module = pcb->m_Modules; module != NULL; module = module->Next() )
        {
            module->TransformPadsShapesWithClearanceToPolygon( layer,
                                                               bufferPolys,
                                                               0,
                                                               segcountforcircle,
                                                               correctionFactor );

            module->TransformGraphicShapesWithClearanceToPolygonSet( layer,
                                                                     bufferPolys,
                                                                     0,
                                                                     segcountforcircle,
                                                                     correctionFactor );
        }

        // bufferPolys contains polygons to merge. Many overlaps .
        // Calculate merged polygons and remove pads and vias holes
        if( bufferPolys.GetCornersCount() == 0 )
            continue;
        KI_POLYGON_SET  currLayerPolyset;
        KI_POLYGON_SET  polyset;
        bufferPolys.ExportTo( polyset );
        currLayerPolyset += polyset;

        int         thickness = g_Parm_3D_Visu.GetLayerObjectThicknessBIU( layer );
        int         zpos = g_Parm_3D_Visu.GetLayerZcoordBIU( layer );
        // for Draw3D_SolidHorizontalPolyPolygons,
        // zpos it the middle between bottom and top sides.
        // However for top layers, zpos should be the bottom layer pos,
        // and for bottom layers, zpos should be the top layer pos.
        if( Get3DLayer_Z_Orientation( layer ) > 0 )
            zpos += thickness/2;
        else
            zpos -= thickness/2 ;

        bufferPolys.RemoveAllContours();
        bufferPolys.ImportFrom( currLayerPolyset );

        SetGLTechLayersColor( layer );
        glNormal3f( 0.0, 0.0, Get3DLayer_Z_Orientation( layer ) );
        Draw3D_SolidHorizontalPolyPolygons( bufferPolys, zpos,
                                            thickness, g_Parm_3D_Visu.m_BiuTo3Dunits );
    }
}

void EDA_3D_CANVAS::CreateDrawGL_List()
{
    BOARD* pcb = GetBoard();

    wxBusyCursor    dummy;

    // Build 3D board parameters:
    g_Parm_3D_Visu.InitSettings( pcb );

    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );

    // Create axis gl list (if it is not shown, the list will be not called
    Draw3DAxis();

    // Create grid gl list
    if( ! m_glLists[GL_ID_GRID] )
    {
        m_glLists[GL_ID_GRID] = glGenLists( 1 );
        glNewList( m_glLists[GL_ID_GRID], GL_COMPILE );

        Draw3DGrid( g_Parm_3D_Visu.m_3D_Grid );
        glEndList();
    }

    // Create Board full gl lists:

// For testing purpose only, display calculation time to generate 3D data
// #define PRINT_CALCULATION_TIME

#ifdef PRINT_CALCULATION_TIME
    unsigned strtime = GetRunningMicroSecs();
#endif

    if( ! m_glLists[GL_ID_BOARD] )
    {
        m_glLists[GL_ID_BOARD] = glGenLists( 1 );
        glNewList( m_glLists[GL_ID_BOARD], GL_COMPILE );
        BuildBoard3DView();
        glEndList();
    }

    if( ! m_glLists[GL_ID_TECH_LAYERS] )
    {
        m_glLists[GL_ID_TECH_LAYERS] = glGenLists( 1 );
        glNewList( m_glLists[GL_ID_TECH_LAYERS], GL_COMPILE );
        BuildTechLayers3DView();
        glEndList();
    }

    if( ! m_glLists[GL_ID_AUX_LAYERS] )
    {
        m_glLists[GL_ID_AUX_LAYERS] = glGenLists( 1 );
        glNewList( m_glLists[GL_ID_AUX_LAYERS], GL_COMPILE );
        BuildBoard3DAuxLayers();
        glEndList();
    }


    // draw modules 3D shapes
    if( ! m_glLists[GL_ID_3DSHAPES_SOLID] && g_Parm_3D_Visu.GetFlag( FL_MODULE ) )
    {
        m_glLists[GL_ID_3DSHAPES_SOLID] = glGenLists( 1 );

        // GL_ID_3DSHAPES_TRANSP is an auxiliary list for 3D shapes;
        // Ensure it is cleared before rebuilding it
        if( m_glLists[GL_ID_3DSHAPES_TRANSP] )
            glDeleteLists( m_glLists[GL_ID_3DSHAPES_TRANSP], 1 );

        m_glLists[GL_ID_3DSHAPES_TRANSP] = glGenLists( 1 );
        BuildFootprintShape3DList( m_glLists[GL_ID_3DSHAPES_SOLID],
                                   m_glLists[GL_ID_3DSHAPES_TRANSP] );
    }

    // Test for errors
    CheckGLError();

#ifdef PRINT_CALCULATION_TIME
    unsigned    endtime = GetRunningMicroSecs();
    wxString    msg;
    msg.Printf( "Built data %.1f ms", (double) (endtime - strtime) / 1000 );
    Parent()->SetStatusText( msg, 0 );
#endif
}


void EDA_3D_CANVAS::BuildFootprintShape3DList( GLuint aOpaqueList,
                                               GLuint aTransparentList)
{
        // aOpaqueList is the gl list for non transparent items
        // aTransparentList is the gl list for non transparent items,
        // which need to be drawn after all other items

        BOARD* pcb = GetBoard();
        glNewList( aOpaqueList, GL_COMPILE );
        bool loadTransparentObjects = false;

        for( MODULE* module = pcb->m_Modules; module; module = module->Next() )
            module->ReadAndInsert3DComponentShape( this, !loadTransparentObjects,
                                                   loadTransparentObjects );

        glEndList();

        glNewList( aTransparentList, GL_COMPILE );
        loadTransparentObjects = true;

        for( MODULE* module = pcb->m_Modules; module; module = module->Next() )
            module->ReadAndInsert3DComponentShape( this, !loadTransparentObjects,
                                                   loadTransparentObjects );

        glEndList();
}

void EDA_3D_CANVAS::Draw3DAxis()
{
    if( ! m_glLists[GL_ID_AXIS] )
    {
        m_glLists[GL_ID_AXIS] = glGenLists( 1 );
        glNewList( m_glLists[GL_ID_AXIS], GL_COMPILE );

        glEnable( GL_COLOR_MATERIAL );
        SetGLColor( WHITE );
        glBegin( GL_LINES );
        glNormal3f( 0.0f, 0.0f, 1.0f );     // Normal is Z axis
        glVertex3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( 1.0f, 0.0f, 0.0f );     // X axis
        glVertex3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( 0.0f, -1.0f, 0.0f );    // Y axis
        glNormal3f( 1.0f, 0.0f, 0.0f );     // Normal is Y axis
        glVertex3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( 0.0f, 0.0f, 0.3f );     // Z axis
        glEnd();

        glEndList();
    }
}

// draw a 3D grid: an horizontal grid (XY plane and Z = 0,
// and a vertical grid (XZ plane and Y = 0)
void EDA_3D_CANVAS::Draw3DGrid( double aGriSizeMM )
{
    double      zpos = 0.0;
    EDA_COLOR_T gridcolor = DARKGRAY;           // Color of grid lines
    EDA_COLOR_T gridcolor_marker = LIGHTGRAY;   // Color of grid lines every 5 lines
    const double scale = g_Parm_3D_Visu.m_BiuTo3Dunits;
    const double transparency = 0.3;

    glNormal3f( 0.0, 0.0, 1.0 );

    wxSize  brd_size = g_Parm_3D_Visu.m_BoardSize;
    wxPoint brd_center_pos = g_Parm_3D_Visu.m_BoardPos;
    NEGATE( brd_center_pos.y );

    int     xsize   = std::max( brd_size.x, Millimeter2iu( 100 ) );
    int     ysize   = std::max( brd_size.y, Millimeter2iu( 100 ) );

    // Grid limits, in 3D units
    double  xmin    = (brd_center_pos.x - xsize / 2) * scale;
    double  xmax    = (brd_center_pos.x + xsize / 2) * scale;
    double  ymin    = (brd_center_pos.y - ysize / 2) * scale;
    double  ymax    = (brd_center_pos.y + ysize / 2) * scale;
    double  zmin    = Millimeter2iu( -50 ) * scale;
    double  zmax    = Millimeter2iu( 100 ) * scale;

    // Draw horizontal grid centered on 3D origin (center of the board)
    for( int ii = 0; ; ii++ )
    {
        if( (ii % 5) )
            SetGLColor( gridcolor, transparency );
        else
            SetGLColor( gridcolor_marker, transparency );

        int delta = KiROUND( ii * aGriSizeMM * IU_PER_MM );

        if( delta <= xsize / 2 )    // Draw grid lines parallel to X axis
        {
            glBegin( GL_LINES );
            glVertex3f( (brd_center_pos.x + delta) * scale, -ymin, zpos );
            glVertex3f( (brd_center_pos.x + delta) * scale, -ymax, zpos );
            glEnd();

            if( ii != 0 )
            {
                glBegin( GL_LINES );
                glVertex3f( (brd_center_pos.x - delta) * scale, -ymin, zpos );
                glVertex3f( (brd_center_pos.x - delta) * scale, -ymax, zpos );
                glEnd();
            }
        }

        if( delta <= ysize / 2 )    // Draw grid lines parallel to Y axis
        {
            glBegin( GL_LINES );
            glVertex3f( xmin, -(brd_center_pos.y + delta) * scale, zpos );
            glVertex3f( xmax, -(brd_center_pos.y + delta) * scale, zpos );
            glEnd();

            if( ii != 0 )
            {
                glBegin( GL_LINES );
                glVertex3f( xmin, -(brd_center_pos.y - delta) * scale, zpos );
                glVertex3f( xmax, -(brd_center_pos.y - delta) * scale, zpos );
                glEnd();
            }
        }

        if( ( delta > ysize / 2 ) && ( delta > xsize / 2 ) )
            break;
    }

    // Draw vertical grid n Z axis
    glNormal3f( 0.0, -1.0, 0.0 );

    // Draw vertical grid lines (parallel to Z axis)
    for( int ii = 0; ; ii++ )
    {
        if( (ii % 5) )
            SetGLColor( gridcolor, transparency );
        else
            SetGLColor( gridcolor_marker, transparency );

        double delta = ii * aGriSizeMM * IU_PER_MM;

        glBegin( GL_LINES );
        glVertex3f( (brd_center_pos.x + delta) * scale, -brd_center_pos.y * scale, zmin );
        glVertex3f( (brd_center_pos.x + delta) * scale, -brd_center_pos.y * scale, zmax );
        glEnd();

        if( ii != 0 )
        {
            glBegin( GL_LINES );
            glVertex3f( (brd_center_pos.x - delta) * scale, -brd_center_pos.y * scale, zmin );
            glVertex3f( (brd_center_pos.x - delta) * scale, -brd_center_pos.y * scale, zmax );
            glEnd();
        }

        if( delta > xsize / 2 )
            break;
    }

    // Draw horizontal grid lines on Z axis
    for( int ii = 0; ; ii++ )
    {
        if( (ii % 5) )
            SetGLColor( gridcolor, transparency);
        else
            SetGLColor( gridcolor_marker, transparency );

        double delta = ii * aGriSizeMM * IU_PER_MM * scale;

        if( delta <= zmax )
        {
            // Draw grid lines on Z axis (positive Z axis coordinates)
            glBegin( GL_LINES );
            glVertex3f( xmin, -brd_center_pos.y * scale, delta );
            glVertex3f( xmax, -brd_center_pos.y * scale, delta );
            glEnd();
        }

        if( delta <= -zmin && ( ii != 0 ) )
        {
            // Draw grid lines on Z axis (negative Z axis coordinates)
            glBegin( GL_LINES );
            glVertex3f( xmin, -brd_center_pos.y * scale, -delta );
            glVertex3f( xmax, -brd_center_pos.y * scale, -delta );
            glEnd();
        }

        if( ( delta > zmax ) && ( delta > -zmin ) )
            break;
    }
}


void EDA_3D_CANVAS::Draw3DViaHole( const VIA* aVia )
{
    LAYER_NUM   top_layer, bottom_layer;
    int         inner_radius    = aVia->GetDrillValue() / 2;
    int         thickness       = g_Parm_3D_Visu.GetCopperThicknessBIU();

    aVia->LayerPair( &top_layer, &bottom_layer );

    // Drawing via hole:
    if( g_Parm_3D_Visu.IsRealisticMode() )
        SetGLCopperColor();
    else
    {
        EDA_COLOR_T color = g_ColorsSettings.GetItemColor( VIAS_VISIBLE + aVia->GetViaType() );
        SetGLColor( color );
    }

    int         height = g_Parm_3D_Visu.GetLayerZcoordBIU( top_layer ) -
                         g_Parm_3D_Visu.GetLayerZcoordBIU( bottom_layer ) - thickness;
    int         zpos = g_Parm_3D_Visu.GetLayerZcoordBIU( bottom_layer ) + thickness / 2;

    Draw3D_ZaxisCylinder( aVia->GetStart(), inner_radius + thickness / 2, height,
                          thickness, zpos, g_Parm_3D_Visu.m_BiuTo3Dunits );
}


void MODULE::ReadAndInsert3DComponentShape( EDA_3D_CANVAS* glcanvas,
                                    bool aAllowNonTransparentObjects,
                                    bool aAllowTransparentObjects )
{

    // Read from disk and draws the footprint 3D shapes if exists
    S3D_MASTER* shape3D = m_3D_Drawings;
    double zpos = g_Parm_3D_Visu.GetModulesZcoord3DIU( IsFlipped() );

    glPushMatrix();

    glTranslatef( m_Pos.x * g_Parm_3D_Visu.m_BiuTo3Dunits,
                  -m_Pos.y * g_Parm_3D_Visu.m_BiuTo3Dunits,
                  zpos );

    if( m_Orient )
        glRotatef( (double) m_Orient / 10, 0.0, 0.0, 1.0 );

    if( IsFlipped() )
    {
        glRotatef( 180.0, 0.0, 1.0, 0.0 );
        glRotatef( 180.0, 0.0, 0.0, 1.0 );
    }

    for( ; shape3D != NULL; shape3D = shape3D->Next() )
    {
        shape3D->SetLoadNonTransparentObjects( aAllowNonTransparentObjects );
        shape3D->SetLoadTransparentObjects( aAllowTransparentObjects );

        if( shape3D->Is3DType( S3D_MASTER::FILE3D_VRML ) )
            shape3D->ReadData();
    }

    glPopMatrix();
}


// Draw 3D pads.
void EDA_3D_CANVAS::Draw3DPadHole( const D_PAD* aPad )
{
    // Draw the pad hole
    wxSize  drillsize   = aPad->GetDrillSize();
    bool    hasHole     = drillsize.x && drillsize.y;

    if( !hasHole )
        return;

    // Store here the points to approximate hole by segments
    CPOLYGONS_LIST  holecornersBuffer;
    int             thickness   = g_Parm_3D_Visu.GetCopperThicknessBIU();
    int             height      = g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_FRONT ) -
                                  g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_BACK );

    if( g_Parm_3D_Visu.IsRealisticMode() )
        SetGLCopperColor();
    else
        SetGLColor( DARKGRAY );

    int holeZpoz    = g_Parm_3D_Visu.GetLayerZcoordBIU( LAYER_N_BACK ) + thickness / 2;
    int holeHeight  = height - thickness;

    if( drillsize.x == drillsize.y )    // usual round hole
    {
        Draw3D_ZaxisCylinder( aPad->GetPosition(),
                              (drillsize.x + thickness) / 2, holeHeight,
                              thickness, holeZpoz, g_Parm_3D_Visu.m_BiuTo3Dunits );
    }
    else    // Oblong hole
    {
        wxPoint ends_offset;
        int     width;

        if( drillsize.x > drillsize.y )    // Horizontal oval
        {
            ends_offset.x = ( drillsize.x - drillsize.y ) / 2;
            width = drillsize.y;
        }
        else    // Vertical oval
        {
            ends_offset.y = ( drillsize.y - drillsize.x ) / 2;
            width = drillsize.x;
        }

        RotatePoint( &ends_offset, aPad->GetOrientation() );

        wxPoint start   = aPad->GetPosition() + ends_offset;
        wxPoint end     = aPad->GetPosition() - ends_offset;
        int     hole_radius = ( width + thickness ) / 2;

        // Draw the hole
        Draw3D_ZaxisOblongCylinder( start, end, hole_radius, holeHeight,
                                    thickness, holeZpoz, g_Parm_3D_Visu.m_BiuTo3Dunits );
    }
}


bool Is3DLayerEnabled( LAYER_NUM aLayer )
{
    DISPLAY3D_FLG flg;
    bool realistic_mode = g_Parm_3D_Visu.IsRealisticMode();

    // see if layer needs to be shown
    // check the flags
    switch( aLayer )
    {
    case ADHESIVE_N_BACK:
    case ADHESIVE_N_FRONT:
        flg = FL_ADHESIVE;
        break;

    case SOLDERPASTE_N_BACK:
    case SOLDERPASTE_N_FRONT:
        flg = FL_SOLDERPASTE;
        break;

    case SILKSCREEN_N_BACK:
    case SILKSCREEN_N_FRONT:
        flg = FL_SILKSCREEN;
        break;

    case SOLDERMASK_N_BACK:
    case SOLDERMASK_N_FRONT:
        flg = FL_SOLDERMASK;
        break;

    case DRAW_N:
    case COMMENT_N:
        if( realistic_mode )
            return false;

        flg = FL_COMMENTS;
        break;

    case ECO1_N:
    case ECO2_N:
        if( realistic_mode )
            return false;

        flg = FL_ECO;
        break;

    case LAYER_N_BACK:
    case LAYER_N_FRONT:
        return g_Parm_3D_Visu.m_BoardSettings->IsLayerVisible( aLayer )
               || realistic_mode;
        break;

    default:
        // the layer is an internal copper layer
        if( realistic_mode )
            return false;

        return g_Parm_3D_Visu.m_BoardSettings->IsLayerVisible( aLayer );
    }

    // if the layer has a flag, return the flag
    return g_Parm_3D_Visu.GetFlag( flg ) &&
           g_Parm_3D_Visu.m_BoardSettings->IsLayerVisible( aLayer );
}


GLfloat Get3DLayer_Z_Orientation( LAYER_NUM aLayer )
{
    double nZ = 1.0;

    if( ( aLayer == LAYER_N_BACK )
        || ( aLayer == ADHESIVE_N_BACK )
        || ( aLayer == SOLDERPASTE_N_BACK )
        || ( aLayer == SILKSCREEN_N_BACK )
        || ( aLayer == SOLDERMASK_N_BACK ) )
        nZ = -1.0;

    return nZ;
}
