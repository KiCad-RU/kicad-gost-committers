/***
 * @file board_items_to_polygon_shape_transform.cpp
 * @brief function to convert shapes of items ( pads, tracks... ) to polygons
 */

/* Function to convert pads and tranck shapes to polygons
 * Used to fill zones areas
 */
#include <vector>

#include <fctsys.h>
#include <polygons_defs.h>
#include <drawtxt.h>
#include <pcbnew.h>
#include <wxPcbStruct.h>
#include <trigo.h>
#include <class_pad.h>
#include <class_track.h>
#include <class_drawsegment.h>
#include <class_pcb_text.h>
#include <class_zone.h>
#include <class_module.h>
#include <class_edge_mod.h>
#include <convert_basic_shapes_to_polygon.h>

/* generate pads shapes on layer aLayer as polygons,
 * and adds these polygons to aCornerBuffer
 * aCornerBuffer = the buffer to store polygons
 * aInflateValue = an additionnal size to add to pad shapes
 * aCircleToSegmentsCount = number of segments to approximate a circle
 * aCorrectionFactor = the correction to apply to a circle radius
 *  to generate the polygon.
 *  if aCorrectionFactor = 1.0, the polygon is inside the circle
 *  the radius of circle approximated by segments is
 *  initial radius * aCorrectionFactor
 */
void MODULE::TransformPadsShapesWithClearanceToPolygon( LAYER_NUM aLayer,
                        std::vector <CPolyPt>& aCornerBuffer,
                        int                    aInflateValue,
                        int                    aCircleToSegmentsCount,
                        double                 aCorrectionFactor )
{
    D_PAD* pad = Pads();

    wxSize margin;
    for( ; pad != NULL; pad = pad->Next() )
    {
        if( !pad->IsOnLayer(aLayer) )
            continue;


        switch( aLayer )
        {
        case SOLDERMASK_N_FRONT:
        case SOLDERMASK_N_BACK:
            margin.x = margin.y = pad->GetSolderMaskMargin() + aInflateValue;
            break;

        case SOLDERPASTE_N_FRONT:
        case SOLDERPASTE_N_BACK:
            margin = pad->GetSolderPasteMargin();
            margin.x += aInflateValue;
            margin.y += aInflateValue;
            break;

        default:
            margin.x = margin.y = aInflateValue;
            break;
        }

        pad->BuildPadShapePolygon( aCornerBuffer, margin,
                                   aCircleToSegmentsCount, aCorrectionFactor );
    }
}

/* generate shapes of graphic items (outlines) on layer aLayer as polygons,
 * and adds these polygons to aCornerBuffer
 * aCornerBuffer = the buffer to store polygons
 * aInflateValue = a value to inflate shapes
 * aCircleToSegmentsCount = number of segments to approximate a circle
 * aCorrectionFactor = the correction to apply to the circle radius
 *  to generate the polygon.
 *  if aCorrectionFactor = 1.0, the polygon is inside the circle
 *  the radius of circle approximated by segments is
 *  initial radius * aCorrectionFactor
 */
void MODULE::TransformGraphicShapesWithClearanceToPolygonSet(
                        LAYER_NUM aLayer,
                        std::vector <CPolyPt>& aCornerBuffer,
                        int                    aInflateValue,
                        int                    aCircleToSegmentsCount,
                        double                 aCorrectionFactor )
{
    EDGE_MODULE* outline;
    for( EDA_ITEM* item = GraphicalItems(); item != NULL; item = item->Next() )
    {
        switch( item->Type() )
        {
        case PCB_MODULE_TEXT_T:
            break;

        case PCB_MODULE_EDGE_T:
            outline = (EDGE_MODULE*) item;
            if( outline->GetLayer() != aLayer )
                break;

            switch( outline->GetShape() )
            {
            case S_SEGMENT:
                TransformRoundedEndsSegmentToPolygon( aCornerBuffer,
                                           outline->GetStart(),
                                           outline->GetEnd(),
                                           aCircleToSegmentsCount,
                                           outline->GetWidth() );
                break;

            case S_CIRCLE:
                TransformRingToPolygon( aCornerBuffer, outline->GetCenter(),
                                outline->GetRadius(), aCircleToSegmentsCount,
                                outline->GetWidth() );
                break;

            case S_ARC:
                TransformArcToPolygon( aCornerBuffer,
                            outline->GetCenter(), outline->GetArcStart(),
                            outline->GetAngle(),
                            aCircleToSegmentsCount, outline->GetWidth() );
                break;

            case S_POLYGON:
                // for outline shape = S_POLYGON:
                // We must compute true coordinates from m_PolyPoints
                // which are relative to module position and module orientation = 0
                for( unsigned ii = 0; ii < outline->GetPolyPoints().size(); ii++ )
                {
                    CPolyPt corner( outline->GetPolyPoints()[ii] );
                    RotatePoint( &corner.x, &corner.y, GetOrientation() );
                    corner.x += GetPosition().x;
                    corner.y += GetPosition().y;
                    aCornerBuffer.push_back( corner );
                }
                aCornerBuffer.back().end_contour = true;
                break;

            default:
                D( printf( "Error: Shape %d not implemented!\n",
                    ((EDGE_MODULE*) item)->GetShape() ); )
                break;
            }
                break;

            default:
                break;
        }
    }
}

 /* Function TransformSolidAreasShapesToPolygonSet
 * Convert solid areas full shapes to polygon set
 * (the full shape is the polygon area with a thick outline)
 * Used in 3D view
 * Arcs (ends of segments) are approximated by segments
 * aCornerBuffer = a buffer to store the polygons
 * aCircleToSegmentsCount = the number of segments to approximate a circle
 * aCorrectionFactor = the correction to apply to arcs radius to roughly
 * keep arc radius when approximated by segments
 */
void ZONE_CONTAINER::TransformSolidAreasShapesToPolygonSet(
        std::vector <CPolyPt>& aCornerBuffer,
        int                    aCircleToSegmentsCount,
        double                 aCorrectionFactor )
{
    unsigned cornerscount = GetFilledPolysList().size();
    std::vector <CPolyPt> polygonslist;

    if( cornerscount == 0 )
        return;

    // add filled areas polygons
    aCornerBuffer.insert( aCornerBuffer.end(), m_FilledPolysList.begin(),
                          m_FilledPolysList.end() );

    // add filled areas outlines, which are drawn with thich lines
    wxPoint seg_start, seg_end;
    int i_start_contour = 0;
    for( unsigned ic = 0; ic < cornerscount; ic++ )
    {
        seg_start.x = m_FilledPolysList[ ic ].x;
        seg_start.y = m_FilledPolysList[ ic ].y;
        unsigned ic_next = ic+1;

        if( !m_FilledPolysList[ic].end_contour &&
            ic_next < cornerscount )
        {
            seg_end.x = m_FilledPolysList[ ic_next ].x;
            seg_end.y = m_FilledPolysList[ ic_next ].y;
        }
        else
        {
            seg_end.x = m_FilledPolysList[ i_start_contour ].x;
            seg_end.y = m_FilledPolysList[ i_start_contour ].y;
            i_start_contour = ic_next;
        }

        TransformRoundedEndsSegmentToPolygon( aCornerBuffer, seg_start, seg_end,
                                              aCircleToSegmentsCount,
                                              GetMinThickness() );
    }
}

/**
 * Function TransformBoundingBoxWithClearanceToPolygon
 * Convert the text bonding box to a rectangular polygon
 * Used in filling zones calculations
 * Circles and arcs are approximated by segments
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aClearanceValue = the clearance around the pad
 */
void TEXTE_PCB::TransformBoundingBoxWithClearanceToPolygon(
                    std::vector <CPolyPt>& aCornerBuffer,
                    int                    aClearanceValue ) const
{
    if( GetText().Length() == 0 )
        return;

    CPolyPt  corners[4];    // Buffer of polygon corners

    EDA_RECT rect = GetTextBox( -1 );
    rect.Inflate( aClearanceValue );
    corners[0].x = rect.GetOrigin().x;
    corners[0].y = rect.GetOrigin().y;
    corners[1].y = corners[0].y;
    corners[1].x = rect.GetRight();
    corners[2].x = corners[1].x;
    corners[2].y = rect.GetBottom();
    corners[3].y = corners[2].y;
    corners[3].x = corners[0].x;

    for( int ii = 0; ii < 4; ii++ )
    {
        // Rotate polygon
        RotatePoint( &corners[ii].x, &corners[ii].y, m_Pos.x, m_Pos.y, m_Orient );
        aCornerBuffer.push_back( corners[ii] );
    }

    aCornerBuffer.back().end_contour = true;
}


/* Function TransformShapeWithClearanceToPolygonSet
 * Convert the text shape to a set of polygons (one by segment)
 * Used in filling zones calculations and 3D view
 * Circles and arcs are approximated by segments
 * aCornerBuffer = vector <CPolyPt> to store the polygon corners
 * aClearanceValue = the clearance around the text
 * aCircleToSegmentsCount = the number of segments to approximate a circle
 * aCorrectionFactor = the correction to apply to circles radius to keep
 * clearance when the circle is approximated by segment bigger or equal
 * to the real clearance value (usually near from 1.0)
 */
// These variables are parameters used in addTextSegmToPoly.
// But addTextSegmToPoly is a call-back function,
// so we cannot send them as arguments.
int s_textWidth;
int s_textCircle2SegmentCount;
std::vector <CPolyPt>* s_cornerBuffer;

// This is a call back function, used by DrawGraphicText to draw the 3D text shape:
static void addTextSegmToPoly( int x0, int y0, int xf, int yf )
{
    TransformRoundedEndsSegmentToPolygon( *s_cornerBuffer,
                                           wxPoint( x0, y0), wxPoint( xf, yf ),
                                           s_textCircle2SegmentCount, s_textWidth );
}

void TEXTE_PCB::TransformShapeWithClearanceToPolygonSet(
                            std::vector <CPolyPt>& aCornerBuffer,
                            int                    aClearanceValue,
                            int                    aCircleToSegmentsCount,
                            double                 aCorrectionFactor ) const
{
    wxSize size = GetSize();

    if( IsMirrored() )
        NEGATE( size.x );

    s_cornerBuffer = &aCornerBuffer;
    s_textWidth  = GetThickness() + ( 2 * aClearanceValue );
    s_textCircle2SegmentCount = aCircleToSegmentsCount;
    EDA_COLOR_T color = BLACK;  // not actually used, but needed by DrawGraphicText

    if( IsMultilineAllowed() )
    {
        wxPoint        pos  = GetTextPosition();
        wxArrayString* list = wxStringSplit( GetText(), '\n' );
        wxPoint        offset;

        offset.y = GetInterline();
        RotatePoint( &offset, GetOrientation() );

        for( unsigned i = 0; i<list->Count(); i++ )
        {
            wxString txt = list->Item( i );
            DrawGraphicText( NULL, NULL, pos, color,
                             txt, GetOrientation(), size,
                             GetHorizJustify(), GetVertJustify(),
                             GetThickness(), IsItalic(),
                             true, addTextSegmToPoly );
            pos += offset;
        }

        delete list;
    }
    else
    {
        DrawGraphicText( NULL, NULL, GetTextPosition(), (EDA_COLOR_T) color,
                         GetText(), GetOrientation(), size,
                         GetHorizJustify(), GetVertJustify(),
                         GetThickness(), IsItalic(),
                         true, addTextSegmToPoly );
    }
}


/**
 * Function TransformShapeWithClearanceToPolygon
 * Convert the track shape to a closed polygon
 * Used in filling zones calculations
 * Circles and arcs are approximated by segments
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aClearanceValue = the clearance around the pad
 * @param aCircleToSegmentsCount = the number of segments to approximate a circle
 * @param aCorrectionFactor = the correction to apply to circles radius to keep
 * clearance when the circle is approxiamted by segment bigger or equal
 * to the real clearance value (usually near from 1.0)
 */
void DRAWSEGMENT::TransformShapeWithClearanceToPolygon( std::vector <CPolyPt>& aCornerBuffer,
                                                        int                    aClearanceValue,
                                                        int                    aCircleToSegmentsCount,
                                                        double                 aCorrectionFactor ) const
{
    switch( m_Shape )
    {
    case S_CIRCLE:
        TransformRingToPolygon( aCornerBuffer, GetCenter(), GetRadius(),
                                aCircleToSegmentsCount,
                                m_Width + (2 * aClearanceValue) ) ;
        break;

    case S_ARC:
        TransformArcToPolygon( aCornerBuffer, GetCenter(),
                               GetArcStart(), m_Angle,
                               aCircleToSegmentsCount,
                               m_Width + (2 * aClearanceValue) );
        break;

    default:
        TransformRoundedEndsSegmentToPolygon( aCornerBuffer, m_Start, m_End,
                                              aCircleToSegmentsCount,
                                              m_Width + (2 * aClearanceValue) );
        break;
    }
}


/**
 * Function TransformShapeWithClearanceToPolygon
 * Convert the track shape to a closed polygon
 * Used in filling zones calculations
 * Circles (vias) and arcs (ends of tracks) are approximated by segments
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aClearanceValue = the clearance around the pad
 * @param aCircleToSegmentsCount = the number of segments to approximate a circle
 * @param aCorrectionFactor = the correction to apply to circles radius to keep
 * clearance when the circle is approximated by segment bigger or equal
 * to the real clearance value (usually near from 1.0)
 */
void TRACK:: TransformShapeWithClearanceToPolygon( std:: vector < CPolyPt>& aCornerBuffer,
                                                   int                      aClearanceValue,
                                                   int                      aCircleToSegmentsCount,
                                                   double                   aCorrectionFactor ) const
{
    switch( Type() )
    {
    case PCB_VIA_T:
    {
        int radius = (m_Width / 2) + aClearanceValue;
        radius = KiROUND( radius * aCorrectionFactor );
        TransformCircleToPolygon( aCornerBuffer, m_Start, radius, aCircleToSegmentsCount );
    }
        break;

    default:
        TransformRoundedEndsSegmentToPolygon( aCornerBuffer,
                                              m_Start, m_End,
                                              aCircleToSegmentsCount,
                                              m_Width + ( 2 * aClearanceValue) );
        break;
    }
}


/* Function TransformShapeWithClearanceToPolygon
 * Convert the pad shape to a closed polygon
 * Used in filling zones calculations and 3D view generation
 * Circles and arcs are approximated by segments
 * aCornerBuffer = a vector < CPolyPt> to store the polygon corners
 * aClearanceValue = the clearance around the pad
 * aCircleToSegmentsCount = the number of segments to approximate a circle
 * aCorrectionFactor = the correction to apply to circles radius to keep
 * clearance when the circle is approximated by segment bigger or equal
 * to the real clearance value (usually near from 1.0)
 */
void D_PAD:: TransformShapeWithClearanceToPolygon( std:: vector < CPolyPt>& aCornerBuffer,
                                                   int                      aClearanceValue,
                                                   int                      aCircleToSegmentsCount,
                                                   double                   aCorrectionFactor ) const
{
    wxPoint corner_position;
    int     angle;
    int     dx = (m_Size.x / 2) + aClearanceValue;
    int     dy = (m_Size.y / 2) + aClearanceValue;

    int     delta = 3600 / aCircleToSegmentsCount;  // rot angle in 0.1 degree
    wxPoint PadShapePos = ReturnShapePos();         /* Note: for pad having a shape offset,
                                                     * the pad position is NOT the shape position */
    wxSize  psize = m_Size;                         /* pad size unsed in RECT and TRAPEZOIDAL pads
                                                     * trapezoidal pads are considered as rect
                                                     * pad shape having they boudary box size */

    switch( m_PadShape )
    {
    case PAD_CIRCLE:
        dx = (int) ( dx * aCorrectionFactor );
        TransformCircleToPolygon( aCornerBuffer, PadShapePos, dx,
                                  aCircleToSegmentsCount );
        break;

    case PAD_OVAL:
        // An oval pad has the same shape as a segment with rounded ends
        angle = m_Orient;
        {
        int width;
        wxPoint shape_offset;
        if( dy > dx )   // Oval pad X/Y ratio for choosing translation axis
        {
            dy = (int) ( dy * aCorrectionFactor );
            shape_offset.y = dy - dx;
            width = dx * 2;
        }
        else    //if( dy <= dx )
        {
            dx = (int) ( dx * aCorrectionFactor );
            shape_offset.x = dy - dx;
            width = dy * 2;
        }

        RotatePoint( &shape_offset, angle );
        wxPoint start = PadShapePos - shape_offset;
        wxPoint end = PadShapePos + shape_offset;
        TransformRoundedEndsSegmentToPolygon( aCornerBuffer, start, end,
                                              aCircleToSegmentsCount, width );
        }
        break;

    default:
    case PAD_TRAPEZOID:
        psize.x += std::abs( m_DeltaSize.y );
        psize.y += std::abs( m_DeltaSize.x );

    // fall through
    case PAD_RECT:
        // Easy implementation for rectangular cutouts with rounded corners
        angle = m_Orient;

        // Corner rounding radius
        int rounding_radius = (int) ( aClearanceValue * aCorrectionFactor );
        int angle_pg;  // Polygon increment angle

        for( int i = 0; i < aCircleToSegmentsCount / 4 + 1; i++ )
        {
            corner_position = wxPoint( 0, -rounding_radius );
            RotatePoint( &corner_position, (1800 / aCircleToSegmentsCount) );

            // Start at half increment offset
            angle_pg = i * delta;
            RotatePoint( &corner_position, angle_pg );

            // Rounding vector rotation
            corner_position -= psize / 2;            // Rounding vector + Pad corner offset
            RotatePoint( &corner_position, angle );

            // Rotate according to module orientation
            corner_position += PadShapePos;          // Shift origin to position
            CPolyPt polypoint( corner_position.x, corner_position.y );
            aCornerBuffer.push_back( polypoint );
        }

        for( int i = 0; i < aCircleToSegmentsCount / 4 + 1; i++ )
        {
            corner_position = wxPoint( -rounding_radius, 0 );
            RotatePoint( &corner_position, (1800 / aCircleToSegmentsCount) );
            angle_pg = i * delta;
            RotatePoint( &corner_position, angle_pg );
            corner_position -= wxPoint( psize.x / 2, -psize.y / 2 );
            RotatePoint( &corner_position, angle );
            corner_position += PadShapePos;
            CPolyPt polypoint( corner_position.x, corner_position.y );
            aCornerBuffer.push_back( polypoint );
        }

        for( int i = 0; i < aCircleToSegmentsCount / 4 + 1; i++ )
        {
            corner_position = wxPoint( 0, rounding_radius );
            RotatePoint( &corner_position, (1800 / aCircleToSegmentsCount) );
            angle_pg = i * delta;
            RotatePoint( &corner_position, angle_pg );
            corner_position += psize / 2;
            RotatePoint( &corner_position, angle );
            corner_position += PadShapePos;
            CPolyPt polypoint( corner_position.x, corner_position.y );
            aCornerBuffer.push_back( polypoint );
        }

        for( int i = 0; i < aCircleToSegmentsCount / 4 + 1; i++ )
        {
            corner_position = wxPoint( rounding_radius, 0 );
            RotatePoint( &corner_position, (1800 / aCircleToSegmentsCount) );
            angle_pg = i * delta;
            RotatePoint( &corner_position, angle_pg );
            corner_position -= wxPoint( -psize.x / 2, psize.y / 2 );
            RotatePoint( &corner_position, angle );
            corner_position += PadShapePos;
            CPolyPt polypoint( corner_position.x, corner_position.y );
            aCornerBuffer.push_back( polypoint );
        }

        aCornerBuffer.back().end_contour = true;
        break;
    }
}

/*
 * Function BuildPadShapePolygon
 * Build the Corner list of the polygonal shape,
 * depending on shape, extra size (clearance ...) pad and orientation
 * Note: for Round and oval pads this function is equivalent to
 * TransformShapeWithClearanceToPolygon, but not for other shapes
 */
void D_PAD::BuildPadShapePolygon( std::vector <CPolyPt>& aCornerBuffer,
                                  wxSize aInflateValue, int aSegmentsPerCircle,
                                  double aCorrectionFactor ) const
{
    wxPoint corners[4];
    wxPoint PadShapePos = ReturnShapePos();         /* Note: for pad having a shape offset,
                                                     * the pad position is NOT the shape position */
    switch( m_PadShape )
    {
    case PAD_CIRCLE:
    case PAD_OVAL:
        TransformShapeWithClearanceToPolygon( aCornerBuffer, aInflateValue.x,
                                              aSegmentsPerCircle, aCorrectionFactor );
        break;

    default:
    case PAD_TRAPEZOID:
    case PAD_RECT:
        BuildPadPolygon( corners, aInflateValue, m_Orient );
        for( int ii = 0; ii < 4; ii++ )
        {
            corners[ii] += PadShapePos;          // Shift origin to position
            CPolyPt polypoint( corners[ii].x, corners[ii].y );
            aCornerBuffer.push_back( polypoint );
        }

        aCornerBuffer.back().end_contour = true;
        break;
    }
}

/*
 * Function BuildPadDrillShapePolygon
 * Build the Corner list of the polygonal drill shape,
 * depending on shape pad hole and orientation
 * return false if the pad has no hole, true otherwise
 */
bool D_PAD::BuildPadDrillShapePolygon( std::vector <CPolyPt>& aCornerBuffer,
                                       int aInflateValue, int aSegmentsPerCircle ) const
{
    wxSize drillsize = GetDrillSize();
    bool hasHole = drillsize.x && drillsize.y;

    if( ! hasHole )
        return false;

    drillsize.x += aInflateValue;
    drillsize.y += aInflateValue;

    if( drillsize.x == drillsize.y )    // usual round hole
    {
        TransformCircleToPolygon( aCornerBuffer, GetPosition(),
                                  drillsize.x /2, aSegmentsPerCircle );
    }
    else    // Oblong hole
    {
        wxPoint ends_offset;
        int width;

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

        RotatePoint( &ends_offset, GetOrientation() );

        wxPoint start  = GetPosition() + ends_offset;
        wxPoint end  = GetPosition() - ends_offset;

        // Prepare the shape creation
        TransformRoundedEndsSegmentToPolygon( aCornerBuffer, start, end,
                                              aSegmentsPerCircle, width );
    }

    return true;
}

/**
 * Function CreateThermalReliefPadPolygon
 * Add holes around a pad to create a thermal relief
 * copper thickness is min (dx/2, aCopperWitdh) or min (dy/2, aCopperWitdh)
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aPad     = the current pad used to create the thermal shape
 * @param aThermalGap = gap in thermal shape
 * @param aCopperThickness = stubs thickness in thermal shape
 * @param aMinThicknessValue = min copper thickness allowed
 * @param aCircleToSegmentsCount = the number of segments to approximate a circle
 * @param aCorrectionFactor = the correction to apply to circles radius to keep
 * @param aThermalRot = for rond pads the rotation of thermal stubs (450 usually for 45 deg.)
 */

/* thermal reliefs are created as 4 polygons.
 * each corner of a polygon if calculated for a pad at position 0, 0, orient 0,
 * and then moved and rotated acroding to the pad position and orientation
 */

/*
 * Note 1: polygons are drawm using outlines witk a thickness = aMinThicknessValue
 * so shapes must take in account this outline thickness
 *
 * Note 2:
 *      Trapezoidal pads are not considered here because they are very special case
 *      and are used in microwave applications and they *DO NOT* have a thermal relief that
 *      change the shape by creating stubs and destroy their properties.
 */
void    CreateThermalReliefPadPolygon( std::vector<CPolyPt>& aCornerBuffer,
                                       D_PAD&                aPad,
                                       int                   aThermalGap,
                                       int                   aCopperThickness,
                                       int                   aMinThicknessValue,
                                       int                   aCircleToSegmentsCount,
                                       double                aCorrectionFactor,
                                       int                   aThermalRot )
{
    wxPoint corner, corner_end;
    wxPoint PadShapePos = aPad.ReturnShapePos();    /* Note: for pad having a shape offset,
                                                     * the pad position is NOT the shape position */
    wxSize  copper_thickness;

    int     dx = aPad.GetSize().x / 2;
    int     dy = aPad.GetSize().y / 2;

    int     delta = 3600 / aCircleToSegmentsCount; // rot angle in 0.1 degree

    /* Keep in account the polygon outline thickness
     * aThermalGap must be increased by aMinThicknessValue/2 because drawing external outline
     * with a thickness of aMinThicknessValue will reduce gap by aMinThicknessValue/2
     */
    aThermalGap += aMinThicknessValue / 2;

    /* Keep in account the polygon outline thickness
     * copper_thickness must be decreased by aMinThicknessValue because drawing outlines
     * with a thickness of aMinThicknessValue will increase real thickness by aMinThicknessValue
     */
    aCopperThickness -= aMinThicknessValue;

    if( aCopperThickness < 0 )
        aCopperThickness = 0;

    copper_thickness.x = std::min( dx, aCopperThickness );
    copper_thickness.y = std::min( dy, aCopperThickness );

    switch( aPad.GetShape() )
    {
    case PAD_CIRCLE:    // Add 4 similar holes
        {
            /* we create 4 copper holes and put them in position 1, 2, 3 and 4
             * here is the area of the rectangular pad + its thermal gap
             * the 4 copper holes remove the copper in order to create the thermal gap
             * 4 ------ 1
             * |        |
             * |        |
             * |        |
             * |        |
             * 3 ------ 2
             * holes 2, 3, 4 are the same as hole 1, rotated 90, 180, 270 deg
             */

            // Build the hole pattern, for the hole in the X >0, Y > 0 plane:
            // The pattern roughtly is a 90 deg arc pie
            std::vector <wxPoint> corners_buffer;

            // Radius of outer arcs of the shape corrected for arc approximation by lines
            int outer_radius = (int) ( (dx + aThermalGap) * aCorrectionFactor );

            // Crosspoint of thermal spoke sides, the first point of polygon buffer
            corners_buffer.push_back( wxPoint( copper_thickness.x / 2, copper_thickness.y / 2 ) );

            // Add an intermediate point on spoke sides, to allow a > 90 deg angle between side
            // and first seg of arc approx
            corner.x = copper_thickness.x / 2;
            int y = outer_radius - (aThermalGap / 4);
            corner.y = (int) sqrt( ( ( (double) y * y ) - (double) corner.x * corner.x ) );

            if( aThermalRot != 0 )
                corners_buffer.push_back( corner );

            // calculate the starting point of the outter arc
            corner.x = copper_thickness.x / 2;

            double dtmp = sqrt( ( (double) outer_radius * outer_radius ) -
                                ( (double) corner.x * corner.x ) );
            corner.y = (int) dtmp;
            RotatePoint( &corner, 90 ); // 9 degrees is the spoke fillet size

            // calculate the ending point of the outter arc
            corner_end.x = corner.y;
            corner_end.y = corner.x;

            // calculate intermediate points (y coordinate from corner.y to corner_end.y
            while( (corner.y > corner_end.y)  && (corner.x < corner_end.x) )
            {
                corners_buffer.push_back( corner );
                RotatePoint( &corner, delta );
            }

            corners_buffer.push_back( corner_end );

            /* add an intermediate point, to avoid angles < 90 deg between last arc approx line
             * and radius line
             */
            corner.x = corners_buffer[1].y;
            corner.y = corners_buffer[1].x;
            corners_buffer.push_back( corner );

            // Now, add the 4 holes ( each is the pattern, rotated by 0, 90, 180 and 270  deg
            // aThermalRot = 450 (45.0 degrees orientation) work fine.
            int angle_pad = aPad.GetOrientation();              // Pad orientation
            int th_angle  = aThermalRot;

            for( unsigned ihole = 0; ihole < 4; ihole++ )
            {
                for( unsigned ii = 0; ii < corners_buffer.size(); ii++ )
                {
                    corner = corners_buffer[ii];
                    RotatePoint( &corner, th_angle + angle_pad );          // Rotate by segment angle and pad orientation
                    corner += PadShapePos;
                    aCornerBuffer.push_back( CPolyPt( corner.x, corner.y ) );
                }

                aCornerBuffer.back().end_contour = true;
                th_angle += 900;       // Note: th_angle in in 0.1 deg.
            }
        }
        break;

    case PAD_OVAL:
        {
            // Oval pad support along the lines of round and rectangular pads
            std::vector <wxPoint> corners_buffer;               // Polygon buffer as vector

            int     dx = (aPad.GetSize().x / 2) + aThermalGap;     // Cutout radius x
            int     dy = (aPad.GetSize().y / 2) + aThermalGap;     // Cutout radius y

            wxPoint shape_offset;

            // We want to calculate an oval shape with dx > dy.
            // if this is not the case, exchange dx and dy, and rotate the shape 90 deg.
            int supp_angle = 0;

            if( dx < dy )
            {
                EXCHG( dx, dy );
                supp_angle = 900;
                EXCHG( copper_thickness.x, copper_thickness.y );
            }

            int deltasize = dx - dy;        // = distance between shape position and the 2 demi-circle ends centre
            // here we have dx > dy
            // Radius of outer arcs of the shape:
            int outer_radius = dy;     // The radius of the outer arc is radius end + aThermalGap

            // Some coordinate fiddling, depending on the shape offset direction
            shape_offset = wxPoint( deltasize, 0 );

            // Crosspoint of thermal spoke sides, the first point of polygon buffer
            corner.x = copper_thickness.x / 2;
            corner.y = copper_thickness.y / 2;
            corners_buffer.push_back( corner );

            // Arc start point calculation, the intersecting point of cutout arc and thermal spoke edge
            // If copper thickness is more than shape offset, we need to calculate arc intercept point.
            if( copper_thickness.x > deltasize )
            {
                corner.x = copper_thickness.x / 2;
                corner.y =  (int) sqrt( ( (double) outer_radius * outer_radius ) -
                                        ( (double) ( corner.x - delta ) * ( corner.x - deltasize ) ) );
                corner.x -= deltasize;

                /* creates an intermediate point, to have a > 90 deg angle
                 * between the side and the first segment of arc approximation
                 */
                wxPoint intpoint = corner;
                intpoint.y -= aThermalGap / 4;
                corners_buffer.push_back( intpoint + shape_offset );
                RotatePoint( &corner, 90 ); // 9 degrees of thermal fillet
            }
            else
            {
                corner.x = copper_thickness.x / 2;
                corner.y = outer_radius;
                corners_buffer.push_back( corner );
            }

            // Add an intermediate point on spoke sides, to allow a > 90 deg angle between side
            // and first seg of arc approx
            wxPoint last_corner;
            last_corner.y = copper_thickness.y / 2;
            int     px = outer_radius - (aThermalGap / 4);
            last_corner.x =
                (int) sqrt( ( ( (double) px * px ) - (double) last_corner.y * last_corner.y ) );

            // Arc stop point calculation, the intersecting point of cutout arc and thermal spoke edge
            corner_end.y = copper_thickness.y / 2;
            corner_end.x =
                (int) sqrt( ( (double) outer_radius *
                             outer_radius ) - ( (double) corner_end.y * corner_end.y ) );
            RotatePoint( &corner_end, -90 ); // 9 degrees of thermal fillet

            // calculate intermediate arc points till limit is reached
            while( (corner.y > corner_end.y)  && (corner.x < corner_end.x) )
            {
                corners_buffer.push_back( corner + shape_offset );
                RotatePoint( &corner, delta );
            }

            //corners_buffer.push_back(corner + shape_offset);      // TODO: about one mil geometry error forms somewhere.
            corners_buffer.push_back( corner_end + shape_offset );
            corners_buffer.push_back( last_corner + shape_offset );         // Enabling the line above shows intersection point.

            /* Create 2 holes, rotated by pad rotation.
             */
            int angle = aPad.GetOrientation() + supp_angle;

            for( int irect = 0; irect < 2; irect++ )
            {
                for( unsigned ic = 0; ic < corners_buffer.size(); ic++ )
                {
                    wxPoint cpos = corners_buffer[ic];
                    RotatePoint( &cpos, angle );
                    cpos += PadShapePos;
                    aCornerBuffer.push_back( CPolyPt( cpos.x, cpos.y ) );
                }

                aCornerBuffer.back().end_contour = true;
                angle = AddAngles( angle, 1800 ); // this is calculate hole 3
            }

            // Create holes, that are the mirrored from the previous holes
            for( unsigned ic = 0; ic < corners_buffer.size(); ic++ )
            {
                wxPoint swap = corners_buffer[ic];
                swap.x = -swap.x;
                corners_buffer[ic] = swap;
            }

            // Now add corner 4 and 2 (2 is the corner 4 rotated by 180 deg
            angle = aPad.GetOrientation() + supp_angle;

            for( int irect = 0; irect < 2; irect++ )
            {
                for( unsigned ic = 0; ic < corners_buffer.size(); ic++ )
                {
                    wxPoint cpos = corners_buffer[ic];
                    RotatePoint( &cpos, angle );
                    cpos += PadShapePos;
                    aCornerBuffer.push_back( CPolyPt( cpos.x, cpos.y ) );
                }

                aCornerBuffer.back().end_contour = true;
                angle = AddAngles( angle, 1800 );
            }
        }
        break;

    case PAD_RECT:       // draw 4 Holes
        {
            /* we create 4 copper holes and put them in position 1, 2, 3 and 4
             * here is the area of the rectangular pad + its thermal gap
             * the 4 copper holes remove the copper in order to create the thermal gap
             * 4 ------ 1
             * |        |
             * |        |
             * |        |
             * |        |
             * 3 ------ 2
             * hole 3 is the same as hole 1, rotated 180 deg
             * hole 4 is the same as hole 2, rotated 180 deg and is the same as hole 1, mirrored
             */

            // First, create a rectangular hole for position 1 :
            // 2 ------- 3
            //  |        |
            //  |        |
            //  |        |
            // 1  -------4

            // Modified rectangles with one corner rounded. TODO: merging with oval thermals
            // and possibly round too.

            std::vector <wxPoint> corners_buffer;               // Polygon buffer as vector

            int dx = (aPad.GetSize().x / 2) + aThermalGap;         // Cutout radius x
            int dy = (aPad.GetSize().y / 2) + aThermalGap;         // Cutout radius y

            // The first point of polygon buffer is left lower corner, second the crosspoint of
            // thermal spoke sides, the third is upper right corner and the rest are rounding
            // vertices going anticlockwise. Note the inveted Y-axis in CG.
            corners_buffer.push_back( wxPoint( -dx, -(aThermalGap / 4 + copper_thickness.y / 2) ) );    // Adds small miters to zone
            corners_buffer.push_back( wxPoint( -(dx - aThermalGap / 4), -copper_thickness.y / 2 ) );    // fill and spoke corner
            corners_buffer.push_back( wxPoint( -copper_thickness.x / 2, -copper_thickness.y / 2 ) );
            corners_buffer.push_back( wxPoint( -copper_thickness.x / 2, -(dy - aThermalGap / 4) ) );
            corners_buffer.push_back( wxPoint( -(aThermalGap / 4 + copper_thickness.x / 2), -dy ) );

            int angle = aPad.GetOrientation();
            int rounding_radius = (int) ( aThermalGap * aCorrectionFactor );    // Corner rounding radius
            int angle_pg;                                                       // Polygon increment angle

            for( int i = 0; i < aCircleToSegmentsCount / 4 + 1; i++ )
            {
                wxPoint corner_position = wxPoint( 0, -rounding_radius );

                // Start at half increment offset
                RotatePoint( &corner_position, 1800 / aCircleToSegmentsCount );
                angle_pg = i * delta;

                RotatePoint( &corner_position, angle_pg );          // Rounding vector rotation
                corner_position -= aPad.GetSize() / 2;              // Rounding vector + Pad corner offset

                corners_buffer.push_back( wxPoint( corner_position.x, corner_position.y ) );
            }

            for( int irect = 0; irect < 2; irect++ )
            {
                for( unsigned ic = 0; ic < corners_buffer.size(); ic++ )
                {
                    wxPoint cpos = corners_buffer[ic];
                    RotatePoint( &cpos, angle );            // Rotate according to module orientation
                    cpos += PadShapePos;                    // Shift origin to position
                    aCornerBuffer.push_back( CPolyPt( cpos.x, cpos.y ) );
                }

                aCornerBuffer.back().end_contour = true;
                angle = AddAngles( angle, 1800 );       // this is calculate hole 3
            }

            // Create holes, that are the mirrored from the previous holes
            for( unsigned ic = 0; ic < corners_buffer.size(); ic++ )
            {
                wxPoint swap = corners_buffer[ic];
                swap.x = -swap.x;
                corners_buffer[ic] = swap;
            }

            // Now add corner 4 and 2 (2 is the corner 4 rotated by 180 deg
            for( int irect = 0; irect < 2; irect++ )
            {
                for( unsigned ic = 0; ic < corners_buffer.size(); ic++ )
                {
                    wxPoint cpos = corners_buffer[ic];
                    RotatePoint( &cpos, angle );
                    cpos += PadShapePos;
                    aCornerBuffer.push_back( CPolyPt( cpos.x, cpos.y ) );
                }

                aCornerBuffer.back().end_contour = true;
                angle = AddAngles( angle, 1800 );
            }

        }
        break;

    default:
        ;
    }
}
