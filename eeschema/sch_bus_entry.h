/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file sch_bus_entry.h
 *
 */

#ifndef _SCH_BUS_ENTRY_H_
#define _SCH_BUS_ENTRY_H_

#include <sch_item_struct.h>


/**
 * Class SCH_BUS_ENTRY_BASE
 *
 * Base class for a bus or wire entry.
 */
class SCH_BUS_ENTRY_BASE : public SCH_ITEM
{
protected:
    wxPoint m_pos;
    wxSize  m_size;

public:
    SCH_BUS_ENTRY_BASE( KICAD_T aType, const wxPoint& pos = wxPoint( 0, 0 ), char shape = '\\' );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    ~SCH_BUS_ENTRY_BASE() { }

    wxPoint m_End() const;

    /**
     * function GetBusEntryShape
     * @return the shape of the bus entry, as an ascii code '/' or '\'
     */
    char GetBusEntryShape() const;

    /**
     * function SetBusEntryShape
     * @param aShape = the shape of the bus entry, as an ascii code '/' or '\'
     */
    void SetBusEntryShape( char aShape );

    wxSize GetSize() const { return m_size; }

    void SetSize( const wxSize& aSize ) { m_size = aSize; }

    void SwapData( SCH_ITEM* aItem );

    void Draw( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
               GR_DRAWMODE aDrawMode, EDA_COLOR_T aColor = UNSPECIFIED_COLOR );

    static bool Load( LINE_READER& aLine, wxString& aErrorMsg, SCH_ITEM **out );

    const EDA_RECT GetBoundingBox() const;    // Virtual

    void Move( const wxPoint& aMoveVector )
    {
        m_pos += aMoveVector;
    }


    void MirrorY( int aYaxis_position );

    void MirrorX( int aXaxis_position );

    void Rotate( wxPoint aPosition );

    void GetEndPoints( std::vector <DANGLING_END_ITEM>& aItemList );

    bool IsSelectStateChanged( const wxRect& aRect );

    bool IsConnectable() const { return true; }

    void GetConnectionPoints( std::vector< wxPoint >& aPoints ) const;

    BITMAP_DEF GetMenuImage() const { return  add_entry_xpm; }

    wxPoint GetPosition() const { return m_pos; }

    void SetPosition( const wxPoint& aPosition ) { m_pos = aPosition; }

    bool HitTest( const wxPoint& aPosition, int aAccuracy ) const;

    bool HitTest( const EDA_RECT& aRect, bool aContained = false, int aAccuracy = 0 ) const;

    void Plot( PLOTTER* aPlotter );

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const { ShowDummy( os ); } // override
#endif
};

/**
 * Class SCH_BUS_WIRE_ENTRY
 *
 * Class for a wire to bus entry.
 */
class SCH_BUS_WIRE_ENTRY : public SCH_BUS_ENTRY_BASE
{
public:
    SCH_BUS_WIRE_ENTRY( const wxPoint& pos = wxPoint( 0, 0 ), char shape = '\\' );

    ~SCH_BUS_WIRE_ENTRY() { }

    wxString GetClass() const
    {
        return wxT( "SCH_BUS_WIRE_ENTRY" );
    }

    bool Save( FILE* aFile ) const;

    int GetPenSize() const;

    wxString GetSelectMenuText() const;

    EDA_ITEM* Clone() const;
};

/**
 * Class SCH_BUS_WIRE_ENTRY
 *
 * Class for a bus to bus entry.
 */
class SCH_BUS_BUS_ENTRY : public SCH_BUS_ENTRY_BASE
{
public:
    SCH_BUS_BUS_ENTRY( const wxPoint& pos = wxPoint( 0, 0 ), char shape = '\\' );

    ~SCH_BUS_BUS_ENTRY() { }

    wxString GetClass() const
    {
        return wxT( "SCH_BUS_BUS_ENTRY" );
    }

    bool Save( FILE* aFile ) const;

    int GetPenSize() const;

    wxString GetSelectMenuText() const;

    EDA_ITEM* Clone() const;
};

#endif    // _SCH_BUS_ENTRY_H_
