// PolyLine.h ... definition of CPolyLine class

//
// A polyline contains one or more contours, where each contour
// is defined by a list of corners and side-styles
// There may be multiple contours in a polyline.
// The last contour may be open or closed, any others must be closed.
// All of the corners and side-styles are concatenated into 2 arrays,
// separated by setting the end_contour flag of the last corner of
// each contour.
//
// When used for copper (or technical layers) areas, the first contour is the outer edge
// of the area, subsequent ones are "holes" in the copper.

#ifndef POLYLINE_H
#define POLYLINE_H

#include <vector>

#include <pad_shapes.h>
#include <wx/gdicmn.h>      // for wxPoint definition

#include <polygons_defs.h>

class CRect
{
public:
    int left, right, top, bottom;
};

class CSegment
{
public:
    wxPoint m_Start;
    wxPoint m_End;

    CSegment() { };
    CSegment( const wxPoint& aStart, const wxPoint& aEnd )
    {
        m_Start = aStart;
        m_End   = aEnd;
    }

    CSegment( int x0, int y0, int x1, int y1 )
    {
        m_Start.x   = x0; m_Start.y = y0;
        m_End.x     = x1; m_End.y = y1;
    }
};

class CPolyPt : public wxPoint
{
public:
    CPolyPt( int aX = 0, int aY = 0, bool aEnd = false, int aUtility = 0 ) :
        wxPoint( aX, aY ), end_contour( aEnd ), m_utility( aUtility )
    {}

    // / Pure copy constructor is here to dis-ambiguate from the
    // / specialized CPolyPt( const wxPoint& ) constructor version below.
    CPolyPt( const CPolyPt& aPt ) :
        wxPoint( aPt.x, aPt.y ), end_contour( aPt.end_contour ), m_utility( aPt.m_utility )
    {}

    CPolyPt( const wxPoint& aPoint ) :
        wxPoint( aPoint ), end_contour( false ), m_utility( 0 )
    {}


    bool    end_contour;
    int     m_utility;

    bool operator ==( const CPolyPt& cpt2 ) const
    { return (x == cpt2.x) && (y == cpt2.y) && (end_contour == cpt2.end_contour); }

    bool operator !=( CPolyPt& cpt2 ) const
    { return (x != cpt2.x) || (y != cpt2.y) || (end_contour != cpt2.end_contour); }
};


class CPolyLine
{
public:
    enum HATCH_STYLE { NO_HATCH, DIAGONAL_FULL, DIAGONAL_EDGE };    // hatch styles

    // constructors/destructor
    CPolyLine();
    ~CPolyLine();

    /**
     * Function ImportSettings
     * Copy settings (layer, hatch styles) from aPoly
     * @param aPoly is the CPolyLine to import settings
     */
    void ImportSettings( const CPolyLine * aPoly );

    // functions for modifying the CPolyLine contours

    /* initialize a contour
     * set layer, hatch style, and starting point
     */
    void        Start( int layer, int x, int y, int hatch );

    void        AppendCorner( int x, int y );
    void        InsertCorner( int ic, int x, int y );

    /**
     * Function DeleteCorner
     * remove the given corner. if it is the last point of a contour
     * keep the controur closed by modifying the previous corner
     * @param ic = the index of the corner to delete
     */
    void        DeleteCorner ( int ic );
    void        MoveCorner( int ic, int x, int y );
    void        CloseLastContour();
    void        RemoveContour( int icont );

    /**
     * Function IsPolygonSelfIntersecting
     * Test a CPolyLine for self-intersection of vertex (all contours).
     *
     * @return :
     *  false if no intersecting sides
     *  true if intersecting sides
     * When a CPolyLine is self intersectic, it need to be normalized.
     * (converted to non intersecting polygons)
     */
    bool IsPolygonSelfIntersecting();

    /**
     * Function Chamfer
     * returns a chamfered version of a polygon.
     * @param aDistance is the chamfering distance.
     * @return CPolyLine* - Pointer to new polygon.
     */
    CPolyLine*  Chamfer( unsigned int aDistance );

    /**
     * Function Fillet
     * returns a filleted version of a polygon.
     * @param aRadius is the fillet radius.
     * @param aSegments is the number of segments / fillet.
     * @return CPolyLine* - Pointer to new polygon.
     */
    CPolyLine*  Fillet( unsigned int aRadius, unsigned int aSegments );

    /**
     * Function RemoveNullSegments
     * Removes corners which create a null segment edge
     * (i.e. when 2 successive corners are at the same location)
     * @return the count of removed corners.
     */
     int        RemoveNullSegments();

    void        RemoveAllContours( void );

    // Remove or create hatch
    void        UnHatch();
    void        Hatch();

    // Transform functions
    void        MoveOrigin( int x_off, int y_off );

    // misc. functions
    CRect       GetBounds();
    CRect       GetCornerBounds();
    CRect       GetCornerBounds( int icont );
    void        Copy( CPolyLine* src );
    bool        TestPointInside( int x, int y );
    bool        IsCutoutContour( int icont );

    /**
     * Function AppendArc.
     * Adds segments to current contour to approximate the given arc
     */
    void        AppendArc( int xi, int yi, int xf, int yf, int xc, int yc, int num );

    // access functions
    void       SetLayer( int aLayer ) { m_layer = aLayer; }
    int        GetLayer() const { return m_layer; }
    int        GetNumCorners();
    int        GetNumSides();
    int        GetClosed();
    int        GetContoursCount();
    int        GetContour( int ic );
    int        GetContourStart( int icont );
    int        GetContourEnd( int icont );
    int        GetContourSize( int icont );

    int        GetX( int ic ) const { return m_CornersList[ic].x; }
    int        GetY( int ic ) const { return m_CornersList[ic].y; }

    const wxPoint& GetPos( int ic ) const { return m_CornersList[ic]; }

    int GetEndContour( int ic );

    int        GetUtility( int ic ) const { return m_CornersList[ic].m_utility; };
    void       SetUtility( int ic, int utility ) { m_CornersList[ic].m_utility = utility; };

    int        GetHatchPitch() const { return m_hatchPitch; }
    static int GetDefaultHatchPitchMils() { return 20; }    // default hatch pitch value in mils

    enum HATCH_STYLE GetHatchStyle() const { return m_hatchStyle; }
    void       SetHatch( int aHatchStyle, int aHatchPitch, bool aRebuildHatch )
    {
        SetHatchPitch( aHatchPitch );
        m_hatchStyle = (enum HATCH_STYLE) aHatchStyle;
        if( aRebuildHatch )
            Hatch();
    }

    void    SetX( int ic, int x );
    void    SetY( int ic, int y );
    void    SetEndContour( int ic, bool end_contour );

    void       SetHatchStyle( enum HATCH_STYLE style )
    {
        m_hatchStyle = style;
    }

    void       SetHatchPitch( int pitch ) { m_hatchPitch = pitch; }

    /**
     * Function NormalizeAreaOutlines
     * Convert a self-intersecting polygon to one (or more) non self-intersecting polygon(s)
     * @param aNewPolygonList = a std::vector<CPolyLine*> reference where to store new CPolyLine
     * needed by the normalization
     * @return the polygon count (always >= 1, because there is at least one polygon)
     * There are new polygons only if the polygon count  is > 1
     */
    int NormalizeAreaOutlines( std::vector<CPolyLine*>* aNewPolygonList );

    // Bezier Support
    void    AppendBezier( int x1, int y1, int x2, int y2, int x3, int y3 );
    void    AppendBezier( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4 );

    /**
     * Function Distance
     * Calculates the distance between a point and the zone:
     * @param aPoint the coordinate of the point.
     * @return int = distance between the point and outline.
     *               0 if the point is inside
     */
    int     Distance( const wxPoint& aPoint );

    /**
     * Function Distance
     * Calculates the distance between a segment and the zone:
     * @param aStart the starting point of the segment.
     * @param aEnd  the ending point of the segment.
     * @param aWidth  the width of the segment.
     * @return int = distance between the segment and outline.
     *               0 if segment intersects or is inside
     */
    int     Distance( wxPoint aStart, wxPoint aEnd, int aWidth );

private:
    int                     m_layer;                // layer to draw on
    enum HATCH_STYLE        m_hatchStyle;           // hatch style, see enum above
    int                     m_hatchPitch;           // for DIAGONAL_EDGE hatched outlines, basic distance between 2 hatch lines
                                                    // and the len of eacvh segment
                                                    // for DIAGONAL_FULL, the pitch is twice this value
    int                     m_utility;              // a flag used in some calculations

public:
    std::vector <CPolyPt>   m_CornersList;          // array of points for corners
    std::vector <CSegment>  m_HatchLines;           // hatch lines showing the polygon area
};

/**
 * Function CopyPolysListToKiPolygonWithHole
 * converts the outline contours aPolysList to a KI_POLYGON_WITH_HOLES
 *
 * @param aPolysList = the list of corners of contours
 * @param aPolygoneWithHole = a KI_POLYGON_WITH_HOLES to populate
 */
void CopyPolysListToKiPolygonWithHole( const std::vector<CPolyPt>&  aPolysList,
                                       KI_POLYGON_WITH_HOLES&       aPolygoneWithHole );


/**
 * Function ConvertPolysListWithHolesToOnePolygon
 * converts the outline contours aPolysListWithHoles with holes to one polygon
 * with no holes (only one contour)
 * holes are linked to main outlines by overlap segments, to give only one polygon
 *
 * @param aPolysListWithHoles = the list of corners of contours (haing holes
 * @param aOnePolyList = a polygon with no holes
 */
void ConvertPolysListWithHolesToOnePolygon( const std::vector<CPolyPt>&  aPolysListWithHoles,
                                            std::vector<CPolyPt>&  aOnePolyList );

#endif    // #ifndef POLYLINE_H
