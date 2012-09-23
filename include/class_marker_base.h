/***************************************/
/* Markers: used to show a drc problem */
/***************************************/

#ifndef _CLASS_MARKER_BASE_H
#define _CLASS_MARKER_BASE_H

#include <class_drc_item.h>
#include <gr_basic.h>

class MARKER_BASE
{
public:
    wxPoint               m_Pos;                 ///< position of the marker
protected:
    std::vector <wxPoint> m_Corners;             ///< Corner list for shape definition (a polygon)
    int                   m_MarkerType;          ///< Can be used as a flag
    EDA_COLOR_T           m_Color;               ///< color
    EDA_RECT              m_ShapeBoundingBox;    ///< Bounding box of the graphic symbol, relative
                                                 ///< to the position of the shape, used for Hit
                                                 ///< Tests
    int                   m_ScalingFactor;       ///< Scaling factor for m_Size and m_Corners (can
                                                 ///< set the physical size
    DRC_ITEM              m_drc;

    void init();

public:

    MARKER_BASE();

    /**
     * Constructor
     * @param aErrorCode The categorizing identifier for an error
     * @param aMarkerPos The position of the MARKER on the BOARD
     * @param aText Text describing the first of two objects
     * @param aPos The position of the first of two objects
     * @param bText Text describing the second of the two conflicting objects
     * @param bPos The position of the second of two objects
     */
    MARKER_BASE( int aErrorCode, const wxPoint& aMarkerPos,
                 const wxString& aText, const wxPoint& aPos,
                 const wxString& bText, const wxPoint& bPos );

    /**
     * Constructor
     * @param aErrorCode The categorizing identifier for an error
     * @param aMarkerPos The position of the MARKER on the BOARD
     * @param aText Text describing the object
     * @param aPos The position of the object
     */
    MARKER_BASE( int aErrorCode, const wxPoint& aMarkerPos,
                 const wxString& aText, const wxPoint& aPos );

    /**
     * Contructor
     * makes a copy of \a aMarker but does not copy the DRC_ITEM.
     *
     * @param aMarker The marker to copy.
     */
    MARKER_BASE( const MARKER_BASE& aMarker );

    ~MARKER_BASE();

    /**
     * Function DrawMarker
     * draws the shape is the polygon defined in m_Corners (array of wxPoints).
     */
    void DrawMarker( EDA_DRAW_PANEL* aPanel, wxDC* aDC, GR_DRAWMODE aDrawMode,
                     const wxPoint& aOffset );

    /**
     * Function GetPos
     * returns the position of this MARKER, const.
     */
    const wxPoint& GetPos() const
    {
        return m_Pos;
    }

    /**
     * Function SetColor
     * Set the color of this marker
     */
    void SetColor( EDA_COLOR_T aColor )
    {
        m_Color = aColor;
    }

    /**
     * Function to set/get error levels (warning, fatal ..)
     * this value is stored in m_MarkerType
     */
    void SetErrorLevel( int aErrorLevel )
    {
        m_MarkerType &= ~0xFF00;
        aErrorLevel  &= 0xFF;
        m_MarkerType |= aErrorLevel << 8;
    }

    int GetErrorLevel() const
    {
        return (m_MarkerType >> 8) & 0xFF;
    }

    /** Functions to set/get marker type (DRC, ERC, or other)
     * this value is stored in m_MarkerType
     */
    void SetMarkerType( int aMarkerType )
    {
        m_MarkerType &= ~0xFF;
        aMarkerType  &= 0xFF;
        m_MarkerType |= aMarkerType;
    }

    int GetMarkerType() const
    {
        return m_MarkerType & 0xFF;
    }

    /**
     * Function SetData
     * fills in all the reportable data associated with a MARKER.
     * @param aErrorCode The categorizing identifier for an error
     * @param aMarkerPos The position of the MARKER on the BOARD
     * @param aText Text describing the first of two objects
     * @param aPos The position of the first of two objects
     * @param bText Text describing the second of the two conflicting objects
     * @param bPos The position of the second of two objects
     */
    void SetData( int aErrorCode, const wxPoint& aMarkerPos,
                  const wxString& aText, const wxPoint& aPos,
                  const wxString& bText, const wxPoint& bPos );

    /**
     * Function SetData
     * fills in all the reportable data associated with a MARKER.
     * @param aErrorCode The categorizing identifier for an error
     * @param aMarkerPos The position of the MARKER on the BOARD
     * @param aText Text describing the object
     * @param aPos The position of the object
     */
    void SetData( int aErrorCode, const wxPoint& aMarkerPos,
                  const wxString& aText, const wxPoint& aPos );

    /**
     * Function SetAuxiliaryData
     * initialize data for the second (auxiliary) item
     * @param aAuxiliaryText = the second text (main text) concerning the second schematic or
     *                         board item
     * @param aAuxiliaryPos = position the second item
     */
    void SetAuxiliaryData( const wxString& aAuxiliaryText, const wxPoint& aAuxiliaryPos )
    {
        m_drc.SetAuxiliaryData( aAuxiliaryText, aAuxiliaryPos );
    }

    void SetShowNoCoordinate()
    {
        m_drc.SetShowNoCoordinate();
    }

    /**
     * Function GetReporter
     * returns the DRC_ITEM held within this MARKER so that its
     * interface may be used.
     * @return const& DRC_ITEM
     */
    const DRC_ITEM& GetReporter() const
    {
        return m_drc;
    }

    /**
     * Function DisplayMarkerInfo
     * displays the full info of this marker, in a HTML window.
     */
    void DisplayMarkerInfo( EDA_DRAW_FRAME* aFrame );

    /**
     * Function HitTestMarker
     * tests if the given wxPoint is within the bounds of this object.
     * @param ref_pos A wxPoint to test
     * @return bool - true if a hit, else false
     */
    bool     HitTestMarker( const wxPoint& ref_pos ) const;

    /**
     * Function GetBoundingBoxMarker
     * returns the orthogonal, bounding box of this object for display purposes.
     * This box should be an enclosing perimeter for visible components of this
     * object, and the units should be in the pcb or schematic coordinate system.
     * It is OK to overestimate the size by a few counts.
     */
    EDA_RECT GetBoundingBoxMarker() const;
};


#endif      //  _CLASS_MARKER_BASE_H
