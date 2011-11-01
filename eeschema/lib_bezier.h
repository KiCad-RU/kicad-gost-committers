/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file lib_bezier.h
 */

#ifndef _LIB_BEZIER_H_
#define _LIB_BEZIER_H_

#include "lib_draw_item.h"


/**
 * Class LIB_BEZIER
 * defines bezier curve graphic body item.
 */
class LIB_BEZIER : public LIB_ITEM
{
    int m_Width;                           // Line width
    std::vector<wxPoint> m_BezierPoints;   // list of parameter (3|4)
    std::vector<wxPoint> m_PolyPoints;     // list of points (>= 2)

    /**
     * Draw the bezier curve.
     */
    void drawGraphic( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
                      int aColor, int aDrawMode, void* aData, const TRANSFORM& aTransform );

public:
    LIB_BEZIER( LIB_COMPONENT * aParent );
    LIB_BEZIER( const LIB_BEZIER& aBezier );
    ~LIB_BEZIER() { }

    virtual wxString GetClass() const
    {
        return wxT( "LIB_BEZIER" );
    }


    /**
     * Write bezier curve object out to a FILE in "*.lib" format.
     *
     * @param aFormatter A reference to an OUTPUTFORMATTER to write the component library
     *                   bezier curve to.
     * @return True if success writing else false.
     */
    virtual bool Save( OUTPUTFORMATTER& aFormatter );

    virtual bool Load( LINE_READER& aLineReader, wxString& aErrorMsg );

    void         AddPoint( const wxPoint& aPoint );

    /**
     * @return the number of corners
     */
    unsigned GetCornerCount() const { return m_PolyPoints.size(); }

    /**
     * Test if the given point is within the bounds of this object.
     *
     * @param aRefPos - A wxPoint to test
     * @return true if a hit, else false
     */
    virtual bool HitTest( const wxPoint& aRefPos );

    /**
     * @param aPosRef = a wxPoint to test
     * @param aThreshold = max distance to a segment
     * @param aTransform = the transform matrix
     * @return true if the point aPosRef is near a segment
     */
    virtual bool HitTest( wxPoint aPosRef, int aThreshold, const TRANSFORM& aTransform );

    /**
     * Function GetBoundingBox
     * @return the boundary box for this, in library coordinates
     */
    virtual EDA_RECT GetBoundingBox() const;

    /**
     * Function GetPenSize
     * @return the size of the "pen" that be used to draw or plot this item
     */
    virtual int GetPenSize( ) const;

    virtual void DisplayInfo( EDA_DRAW_FRAME* aFrame );

protected:
    virtual EDA_ITEM* doClone() const;

    /**
     * Provide the bezier curve draw object specific comparison.
     *
     * The sort order for each bezier curve segment point is as follows:
     *      - Bezier point horizontal (X) point position.
     *      - Bezier point vertical (Y) point position.
     */
    virtual int DoCompare( const LIB_ITEM& aOther ) const;

    virtual void DoOffset( const wxPoint& aOffset );
    virtual bool DoTestInside( EDA_RECT& aRect ) const;
    virtual void DoMove( const wxPoint& aPosition );
    virtual wxPoint DoGetPosition() const { return m_PolyPoints[0]; }
    virtual void DoMirrorHorizontal( const wxPoint& aCenter );
    virtual void DoMirrorVertical( const wxPoint& aCenter );
    virtual void DoRotate( const wxPoint& aCenter, bool aRotateCCW = true );
    virtual void DoPlot( PLOTTER* aPlotter, const wxPoint& aOffset, bool aFill,
                         const TRANSFORM& aTransform );
    virtual int DoGetWidth() const { return m_Width; }
    virtual void DoSetWidth( int aWidth ) { m_Width = aWidth; }
};


#endif     // _LIB_BEZIER_H_
