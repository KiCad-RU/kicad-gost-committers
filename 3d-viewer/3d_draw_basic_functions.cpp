/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jp.charras at wanadoo.fr
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
 * @file 3d_draw_basic_functions.cpp
 */

#include <fctsys.h>
#include <trigo.h>
#include <convert_basic_shapes_to_polygon.h>

#include <3d_viewer.h>
#include <info3d_visu.h>
#include <3d_draw_basic_functions.h>


// Imported function:
extern void Set_Object_Data( std::vector<S3D_VERTEX>& aVertices, double aBiuTo3DUnits );
extern void CheckGLError();

// Number of segments to approximate a circle by segments
#define SEGM_PER_CIRCLE 16

#ifndef CALLBACK
#define CALLBACK
#endif

// CALLBACK functions for GLU_TESS
static void CALLBACK    tessBeginCB( GLenum which );
static void CALLBACK    tessEndCB();
static void CALLBACK    tessErrorCB( GLenum errorCode );
static void CALLBACK    tessCPolyPt2Vertex( const GLvoid* data );

// 2 helper functions to set the current normal vector for gle items
static inline void SetNormalZpos()
{
    glNormal3f( 0.0, 0.0, 1.0 );
}

static inline void SetNormalZneg()
{
    glNormal3f( 0.0, 0.0, -1.0 );
}


/* Draw3D_VerticalPolygonalCylinder is a helper function.
 *
 * draws a "vertical cylinder" having a polygon shape
 * from Z position = aZpos to aZpos + aHeight
 * Used to create the vertical sides of 3D horizontal shapes with thickness.
 */
static void Draw3D_VerticalPolygonalCylinder( const std::vector<CPolyPt>& aPolysList,
                                              int aHeight, int aZpos,
                                              bool aInside, double aBiuTo3DUnits )
{
    if( aHeight == 0 )
        return;

    std::vector<S3D_VERTEX> coords;
    coords.resize( 4 );

    // Init Z position of the 4 points of a GL_QUAD
    if( aInside )
    {
        coords[0].z = aZpos;
        coords[1].z = aZpos + aHeight;
    }
    else
    {
        coords[0].z = aZpos + aHeight;
        coords[1].z = aZpos;
    }
    coords[2].z = coords[1].z;
    coords[3].z = coords[0].z;

    // Draw the vertical polygonal side
    int startContour = 0;
    for( unsigned ii = 0; ii < aPolysList.size(); ii++ )
    {
        unsigned jj = ii + 1;

        if( aPolysList[ii].end_contour || jj >= aPolysList.size() )
        {
            jj = startContour;
            startContour = ii + 1;
        }

        // Build the 4 vertices of each GL_QUAD
        coords[0].x = aPolysList[ii].x;
        coords[0].y = -aPolysList[ii].y;
        coords[1].x = coords[0].x;
        coords[1].y = coords[0].y;              // only z change
        coords[2].x = aPolysList[jj].x;
        coords[2].y = -aPolysList[jj].y;
        coords[3].x = coords[2].x;
        coords[3].y = coords[2].y;              // only z change

        // Creates the GL_QUAD
        Set_Object_Data( coords, aBiuTo3DUnits );
    }
}


void SetGLColor( int color )
{
    double          red, green, blue;
    StructColors    colordata = ColorRefs[color & MASKCOLOR];

    red     = colordata.m_Red / 255.0;
    blue    = colordata.m_Blue / 255.0;
    green   = colordata.m_Green / 255.0;
    glColor3f( red, green, blue );
}


/* draw all solid polygons found in aPolysList
 * aZpos = z position in board internal units
 * aThickness = thickness in board internal units
 * If aThickness = 0, a polygon area is drawn in a XY plane at Z position = aZpos.
 * If aThickness > 0, a solid object is drawn.
 *  The top side is located at aZpos + aThickness / 2
 *  The bottom side is located at aZpos - aThickness / 2
 */
void Draw3D_SolidHorizontalPolyPolygons( const std::vector<CPolyPt>& aPolysList,
                                         int aZpos, int aThickness, double aBiuTo3DUnits )
{
    GLUtesselator* tess = gluNewTess();

    gluTessCallback( tess, GLU_TESS_BEGIN, ( void (CALLBACK*) () )tessBeginCB );
    gluTessCallback( tess, GLU_TESS_END, ( void (CALLBACK*) () )tessEndCB );
    gluTessCallback( tess, GLU_TESS_ERROR, ( void (CALLBACK*) () )tessErrorCB );
    gluTessCallback( tess, GLU_TESS_VERTEX, ( void (CALLBACK*) () )tessCPolyPt2Vertex );

    GLdouble    v_data[3];
    double      zpos = ( aZpos + (aThickness / 2) ) * aBiuTo3DUnits;
    g_Parm_3D_Visu.m_CurrentZpos = zpos;
    v_data[2] = aZpos + (aThickness / 2);

    // Set normal to toward positive Z axis, for a solid object only (to draw the top side)
    if( aThickness )
        SetNormalZpos();

    // gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

    // Draw solid areas contained in this list
    std::vector<CPolyPt> polylist = aPolysList;    // temporary copy for gluTessVertex

    for( int side = 0; side < 2; side++ )
    {
        int startContour = 1;

        for( unsigned ii = 0; ii < polylist.size(); ii++ )
        {
            if( startContour == 1 )
            {
                gluTessBeginPolygon( tess, NULL );
                gluTessBeginContour( tess );
                startContour = 0;
            }

            v_data[0]   = polylist[ii].x * aBiuTo3DUnits;
            v_data[1]   = -polylist[ii].y * aBiuTo3DUnits;
            // gluTessVertex store pointers on data, not data, so do not store
            // different corners values in a temporary variable
            // but send pointer on each CPolyPt value in polylist
            // before calling gluDeleteTess
            gluTessVertex( tess, v_data, &polylist[ii] );

            if( polylist[ii].end_contour == 1 )
            {
                gluTessEndContour( tess );
                gluTessEndPolygon( tess );
                startContour = 1;
            }
        }

        if( aThickness == 0 )
            break;

        // Prepare the bottom side of solid areas
        zpos = ( aZpos - (aThickness / 2) ) * aBiuTo3DUnits;
        g_Parm_3D_Visu.m_CurrentZpos = zpos;
        v_data[2] = zpos;
        // Now;, set normal to toward negative Z axis, for the solid object bottom side
        SetNormalZneg();
    }

    gluDeleteTess( tess );

    if( aThickness == 0 )
        return;

    // Build the 3D data : vertical side
    Draw3D_VerticalPolygonalCylinder( polylist, aThickness, aZpos - (aThickness / 2), false, aBiuTo3DUnits );
}


/* draw the solid polygon found in aPolysList
 * The first polygonj is the main polygon, others are holes
 * See Draw3D_SolidHorizontalPolyPolygons for more info
 */
void Draw3D_SolidHorizontalPolygonWithHoles( const std::vector<CPolyPt>& aPolysList,
                                             int aZpos, int aThickness,
                                             double aBiuTo3DUnits )
{
    std::vector<CPolyPt> polygon;

    ConvertPolysListWithHolesToOnePolygon( aPolysList, polygon );
    Draw3D_SolidHorizontalPolyPolygons( polygon, aZpos, aThickness, aBiuTo3DUnits );
}


/* draw a cylinder (a tube) using 3D primitives.
 * the cylinder axis is parallel to the Z axis
 * If aHeight = height of the cylinder is 0, only one ring will be drawn
 * If aThickness = 0, only one cylinder will be drawn
 */
void Draw3D_ZaxisCylinder( wxPoint aCenterPos, int aRadius,
                           int aHeight, int aThickness,
                           int aZpos, double aBiuTo3DUnits )
{
    const int slice = SEGM_PER_CIRCLE;
    std::vector <CPolyPt> outer_cornerBuffer;

    TransformCircleToPolygon( outer_cornerBuffer, aCenterPos,
                              aRadius + (aThickness / 2), slice );

    std::vector<S3D_VERTEX> coords;
    coords.resize( 4 );

    std::vector <CPolyPt> inner_cornerBuffer;
    if( aThickness )    // build the the vertical inner polygon (hole)
        TransformCircleToPolygon( inner_cornerBuffer, aCenterPos,
                                  aRadius - (aThickness / 2), slice );

    if( aHeight )
    {
        // Draw the vertical outer side
        Draw3D_VerticalPolygonalCylinder( outer_cornerBuffer,
                                      aHeight, aZpos, false, aBiuTo3DUnits );
        if( aThickness )
            // Draws the vertical inner side (hole)
            Draw3D_VerticalPolygonalCylinder( inner_cornerBuffer,
                                          aHeight, aZpos, true, aBiuTo3DUnits );
    }

    if( aThickness )
    {
        // draw top (front) and bottom (back) horizontal sides (rings)
        SetNormalZpos();
        outer_cornerBuffer.insert( outer_cornerBuffer.end(),
                             inner_cornerBuffer.begin(), inner_cornerBuffer.end() );
        std::vector<CPolyPt> polygon;

        ConvertPolysListWithHolesToOnePolygon( outer_cornerBuffer, polygon );
        // draw top (front) horizontal ring
        Draw3D_SolidHorizontalPolyPolygons( polygon, aZpos + aHeight, 0, aBiuTo3DUnits );

        if( aHeight )
        {
            // draw bottom (back) horizontal ring
            SetNormalZneg();
            Draw3D_SolidHorizontalPolyPolygons( polygon, aZpos, 0, aBiuTo3DUnits );
        }
    }

    SetNormalZpos();
}


/*
 * Function Draw3D_ZaxisOblongCylinder:
 * draw a segment with an oblong hole.
 * Used to draw oblong holes
 * If aHeight = height of the cylinder is 0, only one ring will be drawn
 * If aThickness = 0, only one cylinder will be drawn
 */
void Draw3D_ZaxisOblongCylinder( wxPoint aAxis1Pos, wxPoint aAxis2Pos,
                                 int aRadius, int aHeight, int aThickness,
                                 int aZpos, double aBiuTo3DUnits  )
{
    const int slice = SEGM_PER_CIRCLE;

    // Build the points to approximate oblong cylinder by segments
    std::vector <CPolyPt> outer_cornerBuffer;

    int segm_width = (aRadius * 2) + aThickness;
    TransformRoundedEndsSegmentToPolygon( outer_cornerBuffer, aAxis1Pos,
                                          aAxis2Pos, slice, segm_width );

    // Draw the oblong outer cylinder
    if( aHeight )
        Draw3D_VerticalPolygonalCylinder( outer_cornerBuffer, aHeight, aZpos,
                                          false, aBiuTo3DUnits );

    if( aThickness )
    {
        std::vector <CPolyPt> inner_cornerBuffer;
        segm_width = aRadius * 2;
        TransformRoundedEndsSegmentToPolygon( inner_cornerBuffer, aAxis1Pos,
                                              aAxis2Pos, slice, segm_width );

        // Draw the oblong inner cylinder
        if( aHeight )
            Draw3D_VerticalPolygonalCylinder( inner_cornerBuffer, aHeight,
                                              aZpos, true, aBiuTo3DUnits );

        // Build the horizontal full polygon shape
        // (outer polygon shape - inner polygon shape)
        outer_cornerBuffer.insert( outer_cornerBuffer.end(),
                                   inner_cornerBuffer.begin(),
                                   inner_cornerBuffer.end() );

        std::vector<CPolyPt> polygon;
        ConvertPolysListWithHolesToOnePolygon( outer_cornerBuffer, polygon );

        // draw top (front) horizontal side (ring)
        SetNormalZpos();
        Draw3D_SolidHorizontalPolyPolygons( polygon, aZpos + aHeight, 0, aBiuTo3DUnits );

        if( aHeight )
        {
            // draw bottom (back) horizontal side (ring)
            SetNormalZneg();
            Draw3D_SolidHorizontalPolyPolygons( polygon, aZpos, 0, aBiuTo3DUnits );
        }
    }

    SetNormalZpos();
}


/* draw a thick segment using 3D primitives, in a XY plane
 * wxPoint aStart, wxPoint aEnd = YX position of end in board units
 * aWidth = width of segment in board units
 * aThickness = thickness of segment in board units
 * aZpos = z position of segment in board units
 */
void Draw3D_SolidSegment( const wxPoint& aStart, const wxPoint& aEnd,
                          int aWidth, int aThickness, int aZpos, double aBiuTo3DUnits )
{
    std::vector <CPolyPt>   cornerBuffer;
    const int               slice = SEGM_PER_CIRCLE;

    TransformRoundedEndsSegmentToPolygon( cornerBuffer, aStart, aEnd, slice, aWidth );

    Draw3D_SolidHorizontalPolyPolygons( cornerBuffer, aZpos, aThickness, aBiuTo3DUnits );
}


void Draw3D_ArcSegment( const wxPoint&  aCenterPos, const wxPoint& aStartPoint,
                        int aArcAngle, int aWidth, int aThickness,
                        int aZpos, double aBiuTo3DUnits )
{
    const int   slice = SEGM_PER_CIRCLE;

    std::vector <CPolyPt> cornerBuffer;
    TransformArcToPolygon( cornerBuffer, aCenterPos, aStartPoint, aArcAngle,
                            slice, aWidth );

    Draw3D_SolidHorizontalPolyPolygons( cornerBuffer, aZpos, aThickness, aBiuTo3DUnits );

}


// /////////////////////////////////////////////////////////////////////////////
// GLU_TESS CALLBACKS
// /////////////////////////////////////////////////////////////////////////////

void CALLBACK tessBeginCB( GLenum which )
{
    glBegin( which );
}


void CALLBACK tessEndCB()
{
    glEnd();
}


void CALLBACK tessCPolyPt2Vertex( const GLvoid* data )
{
    // cast back to double type
    const CPolyPt* ptr = (const CPolyPt*) data;

    glVertex3d( ptr->x * g_Parm_3D_Visu.m_BiuTo3Dunits,
                -ptr->y * g_Parm_3D_Visu.m_BiuTo3Dunits,
                g_Parm_3D_Visu.m_CurrentZpos );
}


void CALLBACK tessErrorCB( GLenum errorCode )
{
#if defined(DEBUG)
    const GLubyte* errorStr;

    errorStr = gluErrorString( errorCode );

    // DEBUG //
    D( printf( "Tess ERROR: %s\n", errorStr ); )
#endif
}
