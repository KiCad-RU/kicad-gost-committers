/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file sch_no_connect.h
 */

#ifndef _SCH_NO_CONNECT_H_
#define _SCH_NO_CONNECT_H_


#include "sch_item_struct.h"


class SCH_NO_CONNECT : public SCH_ITEM
{
    wxPoint m_Pos;                      /* XY coordinates of NoConnect. */

public:
    wxSize  m_Size;                     // size of this symbol

public:
    SCH_NO_CONNECT( const wxPoint& pos = wxPoint( 0, 0 ) );

    SCH_NO_CONNECT( const SCH_NO_CONNECT& aNoConnect );

    ~SCH_NO_CONNECT() { }

    virtual wxString GetClass() const
    {
        return wxT( "SCH_NO_CONNECT" );
    }

    /**
     * Function GetPenSize
     * @return the size of the "pen" that be used to draw or plot this item
     */
    virtual int GetPenSize() const;

    virtual void SwapData( SCH_ITEM* aItem );

    virtual void Draw( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
                       int aDrawMode, int aColor = -1 );

    /**
     * Function Save
     * writes the data structures for this object out to a FILE in "*.sch"
     * format.
     * @param aFile The FILE to write to.
     * @return bool - true if success writing else false.
     */
    bool Save( FILE* aFile ) const;

    /**
     * Load schematic no connect entry from \a aLine in a .sch file.
     *
     * @param aLine - Essentially this is file to read schematic no connect from.
     * @param aErrorMsg - Description of the error if an error occurs while loading the
     *                    schematic no connect.
     * @return True if the schematic no connect loaded successfully.
     */
    virtual bool Load( LINE_READER& aLine, wxString& aErrorMsg );

    /**
     * Function GetBoundingBox
     * returns the orthogonal, bounding box of this object for display
     * purposes.  This box should be an enclosing perimeter for visible
     * components of this object, and the units should be in the pcb or
     * schematic coordinate system.  It is OK to overestimate the size
     * by a few counts.
     */
    EDA_RECT GetBoundingBox() const;

    // Geometric transforms (used in block operations):

    /** virtual function Move
     * move item to a new position.
     * @param aMoveVector = the displacement vector
     */
    virtual void Move( const wxPoint& aMoveVector )
    {
        m_Pos += aMoveVector;
    }

    /**
     * Function Mirror_Y
     * mirrors item relative to \a aYaxis_position.
     * @param aYaxis_position = the y axis position
     */
    virtual void Mirror_Y( int aYaxis_position );

    virtual void Mirror_X( int aXaxis_position );

    virtual void Rotate( wxPoint rotationPoint );

    virtual bool IsSelectStateChanged( const wxRect& aRect );

    virtual bool IsConnectable() const { return true; }

    virtual void GetConnectionPoints( vector< wxPoint >& aPoints ) const;

    virtual wxString GetSelectMenuText() const { return wxString( _( "No Connect" ) ); }

    virtual BITMAP_DEF GetMenuImage() const { return noconn_xpm; }

    virtual void GetNetListItem( vector<NETLIST_OBJECT*>& aNetListItems,
                                 SCH_SHEET_PATH*          aSheetPath );

private:
    virtual bool doIsConnected( const wxPoint& aPosition ) const;
    virtual bool doHitTest( const wxPoint& aPoint, int aAccuracy ) const;
    virtual bool doHitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy ) const;
    virtual EDA_ITEM* doClone() const;
    virtual void doPlot( PLOTTER* aPlotter );
    virtual wxPoint doGetPosition() const { return m_Pos; }
    virtual void doSetPosition( const wxPoint& aPosition ) { m_Pos = aPosition; }
};


#endif    // _SCH_NO_CONNECT_H_
