/**
 * @file zones_convert_to_polygons_aux_functions.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 1992-2013 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <fctsys.h>
#include <polygons_defs.h>
#include <PolyLine.h>
#include <wxPcbStruct.h>
#include <trigo.h>

#include <class_board.h>
#include <class_module.h>
#include <class_zone.h>

#include <pcbnew.h>
#include <zones.h>

 /* Function TransformOutlinesShapeWithClearanceToPolygon
  * Convert the zone filled areas polygons to polygons
  * inflated (optional) by max( aClearanceValue, the zone clearance)
  * and copy them in aCornerBuffer
  * param aClearanceValue = the clearance around polygons
  * param aAddClearance = true to add a clearance area to the polygon
  *                      false to create the outline polygon.
  */
void ZONE_CONTAINER::TransformOutlinesShapeWithClearanceToPolygon(
            CPOLYGONS_LIST& aCornerBuffer,
            int aClearanceValue, bool aAddClearance )
{
    // Creates the zone outlines polygon (with linked holes if any)
    CPOLYGONS_LIST zoneOutines;
    BuildFilledSolidAreasPolygons( NULL, &zoneOutines );

    // add clearance to outline
    int clearance = 0;
    if( aAddClearance )
    {
        clearance = GetClearance();
        if( aClearanceValue > clearance )
            clearance = aClearanceValue;
    }
    // Calculate the polygon with clearance
    // holes are linked to the main outline, so only one polygon should be created.
    KI_POLYGON_SET polyset_zone_solid_areas;
    std::vector<KI_POLY_POINT> cornerslist;
    unsigned ic = 0;
    unsigned corners_count = zoneOutines.GetCornersCount();
    while( ic < corners_count )
    {
        cornerslist.clear();
        KI_POLYGON poly;
        {
            for( ; ic < corners_count; ic++ )
            {
                CPolyPt* corner = &zoneOutines[ic];
                cornerslist.push_back( KI_POLY_POINT( corner->x, corner->y ) );
                if( corner->end_contour )
                {
                    ic++;
                    break;
                }
            }

            bpl::set_points( poly, cornerslist.begin(), cornerslist.end() );
            polyset_zone_solid_areas.push_back( poly );
        }
    }

    polyset_zone_solid_areas += clearance;

    // Put the resulting polygon in aCornerBuffer corners list
    for( unsigned ii = 0; ii < polyset_zone_solid_areas.size(); ii++ )
    {
        KI_POLYGON& poly = polyset_zone_solid_areas[ii];
        CPolyPt   corner( 0, 0, false );

        for( unsigned jj = 0; jj < poly.size(); jj++ )
        {
            KI_POLY_POINT point = *(poly.begin() + jj);
            corner.x = point.x();
            corner.y = point.y();
            corner.end_contour = false;
            aCornerBuffer.Append( corner );
        }

        aCornerBuffer.CloseLastContour();
    }
}



/**
 * Function BuildUnconnectedThermalStubsPolygonList
 * Creates a set of polygons corresponding to stubs created by thermal shapes on pads
 * which are not connected to a zone (dangling bridges)
 * @param aCornerBuffer = a CPOLYGONS_LIST where to store polygons
 * @param aPcb = the board.
 * @param aZone = a pointer to the ZONE_CONTAINER  to examine.
 * @param aArcCorrection = a pointer to the ZONE_CONTAINER  to examine.
 * @param aRoundPadThermalRotation = the rotation in 1.0 degree for thermal stubs in round pads
 */

void BuildUnconnectedThermalStubsPolygonList( CPOLYGONS_LIST& aCornerBuffer,
                                              BOARD*                aPcb,
                                              ZONE_CONTAINER*       aZone,
                                              double                aArcCorrection,
                                              double                aRoundPadThermalRotation )
{
    std::vector<wxPoint> corners_buffer;    // a local polygon buffer to store one stub
    corners_buffer.reserve( 4 );
    wxPoint  ptTest[4];

    int      zone_clearance = aZone->GetZoneClearance();

    EDA_RECT item_boundingbox;
    EDA_RECT zone_boundingbox  = aZone->GetBoundingBox();
    int      biggest_clearance = aPcb->GetDesignSettings().GetBiggestClearanceValue();
    biggest_clearance = std::max( biggest_clearance, zone_clearance );
    zone_boundingbox.Inflate( biggest_clearance );

    // half size of the pen used to draw/plot zones outlines
    int pen_radius = aZone->GetMinThickness() / 2;

    for( MODULE* module = aPcb->m_Modules;  module;  module = module->Next() )
    {
        for( D_PAD* pad = module->Pads(); pad != NULL; pad = pad->Next() )
        {
            // Rejects non-standard pads with tht-only thermal reliefs
            if( aZone->GetPadConnection( pad ) == THT_THERMAL
             && pad->GetAttribute() != PAD_STANDARD )
                continue;

            if( aZone->GetPadConnection( pad ) != THERMAL_PAD
             && aZone->GetPadConnection( pad ) != THT_THERMAL )
                continue;

            // check
            if( !pad->IsOnLayer( aZone->GetLayer() ) )
                continue;

            if( pad->GetNetCode() != aZone->GetNetCode() )
                continue;

            // Calculate thermal bridge half width
            int thermalBridgeWidth = aZone->GetThermalReliefCopperBridge( pad )
                                     - aZone->GetMinThickness();
            if( thermalBridgeWidth <= 0 )
                continue;

            // we need the thermal bridge half width
            // with a small extra size to be sure we create a stub
            // slightly larger than the actual stub
            thermalBridgeWidth = ( thermalBridgeWidth + 4 ) / 2;

            int thermalReliefGap = aZone->GetThermalReliefGap( pad );

            item_boundingbox = pad->GetBoundingBox();
            item_boundingbox.Inflate( thermalReliefGap );
            if( !( item_boundingbox.Intersects( zone_boundingbox ) ) )
                continue;

            // Thermal bridges are like a segment from a starting point inside the pad
            // to an ending point outside the pad

            // calculate the ending point of the thermal pad, outside the pad
            wxPoint endpoint;
            endpoint.x = ( pad->GetSize().x / 2 ) + thermalReliefGap;
            endpoint.y = ( pad->GetSize().y / 2 ) + thermalReliefGap;

            // Calculate the starting point of the thermal stub
            // inside the pad
            wxPoint startpoint;
            int copperThickness = aZone->GetThermalReliefCopperBridge( pad )
                                  - aZone->GetMinThickness();

            if( copperThickness < 0 )
                copperThickness = 0;

            // Leave a small extra size to the copper area inside to pad
            copperThickness += KiROUND( IU_PER_MM * 0.04 );

            startpoint.x = std::min( pad->GetSize().x, copperThickness );
            startpoint.y = std::min( pad->GetSize().y, copperThickness );

            startpoint.x /= 2;
            startpoint.y /= 2;

            // This is a CIRCLE pad tweak
            // for circle pads, the thermal stubs orientation is 45 deg
            double fAngle = pad->GetOrientation();
            if( pad->GetShape() == PAD_CIRCLE )
            {
                endpoint.x     = KiROUND( endpoint.x * aArcCorrection );
                endpoint.y     = endpoint.x;
                fAngle = aRoundPadThermalRotation;
            }

            // contour line width has to be taken into calculation to avoid "thermal stub bleed"
            endpoint.x += pen_radius;
            endpoint.y += pen_radius;
            // compute north, south, west and east points for zone connection.
            ptTest[0] = wxPoint( 0, endpoint.y );       // lower point
            ptTest[1] = wxPoint( 0, -endpoint.y );      // upper point
            ptTest[2] = wxPoint( endpoint.x, 0 );       // right point
            ptTest[3] = wxPoint( -endpoint.x, 0 );      // left point

            // Test all sides
            for( int i = 0; i < 4; i++ )
            {
                // rotate point
                RotatePoint( &ptTest[i], fAngle );

                // translate point
                ptTest[i] += pad->ShapePos();

                if( aZone->HitTestFilledArea( ptTest[i] ) )
                    continue;

                corners_buffer.clear();

                // polygons are rectangles with width of copper bridge value
                switch( i )
                {
                case 0:       // lower stub
                    corners_buffer.push_back( wxPoint( -thermalBridgeWidth, endpoint.y ) );
                    corners_buffer.push_back( wxPoint( +thermalBridgeWidth, endpoint.y ) );
                    corners_buffer.push_back( wxPoint( +thermalBridgeWidth, startpoint.y ) );
                    corners_buffer.push_back( wxPoint( -thermalBridgeWidth, startpoint.y ) );
                    break;

                case 1:       // upper stub
                    corners_buffer.push_back( wxPoint( -thermalBridgeWidth, -endpoint.y ) );
                    corners_buffer.push_back( wxPoint( +thermalBridgeWidth, -endpoint.y ) );
                    corners_buffer.push_back( wxPoint( +thermalBridgeWidth, -startpoint.y ) );
                    corners_buffer.push_back( wxPoint( -thermalBridgeWidth, -startpoint.y ) );
                    break;

                case 2:       // right stub
                    corners_buffer.push_back( wxPoint( endpoint.x, -thermalBridgeWidth ) );
                    corners_buffer.push_back( wxPoint( endpoint.x, thermalBridgeWidth ) );
                    corners_buffer.push_back( wxPoint( +startpoint.x, thermalBridgeWidth ) );
                    corners_buffer.push_back( wxPoint( +startpoint.x, -thermalBridgeWidth ) );
                    break;

                case 3:       // left stub
                    corners_buffer.push_back( wxPoint( -endpoint.x, -thermalBridgeWidth ) );
                    corners_buffer.push_back( wxPoint( -endpoint.x, thermalBridgeWidth ) );
                    corners_buffer.push_back( wxPoint( -startpoint.x, thermalBridgeWidth ) );
                    corners_buffer.push_back( wxPoint( -startpoint.x, -thermalBridgeWidth ) );
                    break;
                }


                // add computed polygon to list
                for( unsigned ic = 0; ic < corners_buffer.size(); ic++ )
                {
                    wxPoint cpos = corners_buffer[ic];
                    RotatePoint( &cpos, fAngle );                               // Rotate according to module orientation
                    cpos += pad->ShapePos();                              // Shift origin to position
                    CPolyPt corner;
                    corner.x = cpos.x;
                    corner.y = cpos.y;
                    corner.end_contour = ( ic < (corners_buffer.size() - 1) ) ? false : true;
                    aCornerBuffer.Append( corner );
                }
            }
        }
    }
}
