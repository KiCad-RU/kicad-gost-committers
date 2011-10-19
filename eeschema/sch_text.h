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
 * @file sch_text.h
 * @brief Definitions of the SCH_TEXT class and derivatives for Eeschema.
 */

#ifndef CLASS_TEXT_LABEL_H
#define CLASS_TEXT_LABEL_H


#include "macros.h"
#include "sch_item_struct.h"


class LINE_READER;


/* Type of SCH_HIERLABEL and SCH_GLOBALLABEL
 * mainly used to handle the graphic associated shape
 */
typedef enum {
    NET_INPUT,
    NET_OUTPUT,
    NET_BIDI,
    NET_TRISTATE,
    NET_UNSPECIFIED,
    NET_TMAX        /* Last value */
} TypeSheetLabel;


extern const char* SheetLabelType[];    /* names of types of labels */

class SCH_TEXT : public SCH_ITEM, public EDA_TEXT
{
public:
    int  m_Shape;
    bool m_IsDangling;          // true if not connected (used to draw the "not
                                // connected" symbol
protected:
    int  m_SchematicOrientation;    /* orientation of texts (comments) and
                                     * labels in schematic
                                     *  0 = normal (horizontal, left
                                     * justified).
                                     *  1 = up (vertical)
                                     *  2 =  (horizontal, right justified).
                                     * This can be seen as the mirrored
                                     * position of 0
                                     *  3 = bottom . This can be seen as the
                                     * mirrored position of up
                                     *  this is perhaps a duplicate of m_Orient
                                     * and m_HJustified or m_VJustified,
                                     *  but is more easy to handle that 3
                                     * parameters in editions, Reading and
                                     * Saving file
                                     */

public:
    SCH_TEXT( const wxPoint& pos = wxPoint( 0, 0 ),
              const wxString& text = wxEmptyString,
              KICAD_T aType = SCH_TEXT_T );

    SCH_TEXT( const SCH_TEXT& aText );

    ~SCH_TEXT() { }

    virtual wxString GetClass() const
    {
        return wxT( "SCH_TEXT" );
    }

    /**
     * Function IncrementLabel
     * increments the label text.
     */
    void IncrementLabel();

    /**
     * Function SetOrientation
     * Set m_SchematicOrientation, and initialize
     * m_orient,m_HJustified and m_VJustified, according to the value of
     * m_SchematicOrientation (for a text )
     * must be called after changing m_SchematicOrientation
     * @param aSchematicOrientation =
     *  0 = normal (horizontal, left justified).
     *  1 = up (vertical)
     *  2 =  (horizontal, right justified). This can be seen as the mirrored
     * position of 0
     *  3 = bottom . This can be seen as the mirrored position of up
     */
    virtual void SetOrientation( int aSchematicOrientation );

    int GetOrientation() { return m_SchematicOrientation; }

    /**
     * Function GetSchematicTextOffset (virtual)
     * @return the offset between the SCH_TEXT position and the text itself
     * position
     * This offset depend on orientation, and the type of text
     * (room to draw an associated graphic symbol, or put the text above a
     * wire)
     */
    virtual wxPoint GetSchematicTextOffset() const;

    virtual void Draw( EDA_DRAW_PANEL* panel,
                       wxDC*           DC,
                       const wxPoint&  offset,
                       int             draw_mode,
                       int             Color = -1 );

    /**
     * Function CreateGraphicShape
     * Calculates the graphic shape (a polygon) associated to the text
     * @param aPoints = a buffer to fill with polygon corners coordinates
     * @param Pos = Postion of the shape
     * for texts and labels: do nothing
     * Mainly for derived classes (SCH_SHEET_PIN and Hierarchical labels)
     */
    virtual void CreateGraphicShape( std::vector <wxPoint>& aPoints, const wxPoint& Pos )
    {
        aPoints.clear();
    }

    virtual void SwapData( SCH_ITEM* aItem );

    /**
     * Function GetBoundingBox
     * returns the orthogonal, bounding box of this object for display purposes.
     * This box should be an enclosing perimeter for visible components of this
     * object, and the units should be in the pcb or schematic coordinate system.
     * It is OK to overestimate the size by a few counts.
     */
    EDA_RECT GetBoundingBox() const;

    /**
     * Function Save
     * writes the data structures for this object out to a FILE in "*.sch"
     * format.
     * @param aFile The FILE to write to.
     * @return bool - true if success writing else false.
     */
    bool Save( FILE* aFile ) const;

    /**
     * Load schematic text entry from \a aLine in a .sch file.
     *
     * @param aLine - Essentially this is file to read schematic text from.
     * @param aErrorMsg - Description of the error if an error occurs while loading the
     *                    schematic text.
     * @return True if the schematic text loaded successfully.
     */
    virtual bool Load( LINE_READER& aLine, wxString& aErrorMsg );

    /**
     * Function GetPenSize
     * @return the size of the "pen" that be used to draw or plot this item
     */
    int GetPenSize() const;

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
     * mirrors the item relative to \a aYaxisPosition.
     * @param aYaxis_position The y axis coordinate to mirror around.
     */
    virtual void Mirror_Y( int aYaxis_position );

    virtual void Rotate( wxPoint rotationPoint );

    virtual void Mirror_X( int aXaxis_position );

    /**
     * Compare schematic text entry against search string.
     *
     * @param aSearchData - Criterial to search against.
     * @param aAuxData - a pointer on auxiliary data, if needed. Can be null
     * @param aFindLocation - a wxPoint where to put the location of matched item. can be NULL.
     * @return True if this schematic text item matches the search criteria.
     */
    virtual bool Matches( wxFindReplaceData& aSearchData, void* aAuxData, wxPoint* aFindLocation );

    virtual void GetEndPoints( std::vector< DANGLING_END_ITEM >& aItemList );

    virtual bool IsDanglingStateChanged( std::vector< DANGLING_END_ITEM >& aItemList );

    virtual bool IsDangling() const { return m_IsDangling; }

    virtual bool IsSelectStateChanged( const wxRect& aRect );

    virtual void GetConnectionPoints( vector< wxPoint >& aPoints ) const;

    virtual bool CanIncrementLabel() const { return true; }

    virtual wxString GetSelectMenuText() const;

    virtual BITMAP_DEF GetMenuImage() const { return  add_text_xpm; }

    virtual void GetNetListItem( vector<NETLIST_OBJECT*>& aNetListItems,
                                 SCH_SHEET_PATH*          aSheetPath );

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os );
#endif

private:
    virtual bool doHitTest( const wxPoint& aPoint, int aAccuracy ) const;
    virtual bool doHitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy ) const;
    virtual EDA_ITEM* doClone() const;
    virtual void doPlot( PLOTTER* aPlotter );
    virtual wxPoint doGetPosition() const { return m_Pos; }
    virtual void doSetPosition( const wxPoint& aPosition ) { m_Pos = aPosition; }
};


class SCH_LABEL : public SCH_TEXT
{
public:
    SCH_LABEL( const wxPoint& pos = wxPoint( 0, 0 ), const wxString& text = wxEmptyString );

    SCH_LABEL( const SCH_LABEL& aLabel );

    ~SCH_LABEL() { }

    virtual void Draw( EDA_DRAW_PANEL* panel,
                       wxDC*           DC,
                       const wxPoint&  offset,
                       int             draw_mode,
                       int             Color = -1 );

    virtual wxString GetClass() const
    {
        return wxT( "SCH_LABEL" );
    }

    /**
     * Function SetOrientation
     * Set m_SchematicOrientation, and initialize
     * m_orient,m_HJustified and m_VJustified, according to the value of
     * m_SchematicOrientation (for a label)
     * must be called after changing m_SchematicOrientation
     * @param aSchematicOrientation =
     *  0 = normal (horizontal, left justified).
     *  1 = up (vertical)
     *  2 =  (horizontal, right justified). This can be seen as the mirrored
     * position of 0
     *  3 = bottom . This can be seen as the mirrored position of up
     */
    virtual void SetOrientation( int aSchematicOrientation );

    /**
     * Function GetSchematicTextOffset (virtual)
     * @return the offset between the SCH_TEXT position and the text itself
     * position
     * This offset depend on orientation, and the type of text
     * (room to draw an associated graphic symbol, or put the text above a
     * wire)
     */
    virtual wxPoint GetSchematicTextOffset() const;

    virtual void Mirror_X( int aXaxis_position );

    virtual void Rotate( wxPoint rotationPoint );

    /**
     * Function GetBoundingBox
     * returns the orthogonal, bounding box of this object for display purposes.
     * This box should be an enclosing perimeter for visible components of this
     * object, and the units should be in the pcb or schematic coordinate system.
     * It is OK to overestimate the size by a few counts.
     */
    EDA_RECT GetBoundingBox() const;

    /**
     * Function Save
     * writes the data structures for this object out to a FILE in "*.sch"
     * format.
     * @param aFile The FILE to write to.
     * @return bool - true if success writing else false.
     */
    bool Save( FILE* aFile ) const;

    /**
     * Load schematic label entry from \a aLine in a .sch file.
     *
     * @param aLine - Essentially this is file to read schematic label from.
     * @param aErrorMsg - Description of the error if an error occurs while loading the
     *                    schematic label.
     * @return True if the schematic label loaded successfully.
     */
    virtual bool Load( LINE_READER& aLine, wxString& aErrorMsg );

    virtual bool IsConnectable() const { return true; }

    virtual wxString GetSelectMenuText() const;

    virtual BITMAP_DEF GetMenuImage() const { return  add_line_label_xpm; }

private:
    virtual bool doHitTest( const wxPoint& aPoint, int aAccuracy ) const;
    virtual bool doIsConnected( const wxPoint& aPosition ) const { return m_Pos == aPosition; }
    virtual EDA_ITEM* doClone() const;
};


class SCH_GLOBALLABEL : public SCH_TEXT
{
public:
    SCH_GLOBALLABEL( const wxPoint& pos = wxPoint( 0, 0 ), const wxString& text = wxEmptyString );

    SCH_GLOBALLABEL( const SCH_GLOBALLABEL& aGlobalLabel );

    ~SCH_GLOBALLABEL() { }

    virtual void Draw( EDA_DRAW_PANEL* panel,
                       wxDC*           DC,
                       const wxPoint&  offset,
                       int             draw_mode,
                       int             Color = -1 );

    virtual wxString GetClass() const
    {
        return wxT( "SCH_GLOBALLABEL" );
    }

    /**
     * Function SetOrientation
     * Set m_SchematicOrientation, and initialize
     * m_orient,m_HJustified and m_VJustified, according to the value of
     * m_SchematicOrientation
     * must be called after changing m_SchematicOrientation
     * @param aSchematicOrientation =
     *  0 = normal (horizontal, left justified).
     *  1 = up (vertical)
     *  2 = (horizontal, right justified). This can be seen as the mirrored
     *      position of 0
     *  3 = bottom . This can be seen as the mirrored position of up
     */
    virtual void SetOrientation( int aSchematicOrientation );

    /**
     * Function GetSchematicTextOffset (virtual)
     * @return the offset between the SCH_TEXT position and the text itself
     * position
     * This offset depend on orientation, and the type of text
     * (room to draw an associated graphic symbol, or put the text above a
     * wire)
     */
    virtual wxPoint GetSchematicTextOffset() const;

    /**
     * Function Save
     * writes the data structures for this object out to a FILE in "*.sch"
     * format.
     * @param aFile The FILE to write to.
     * @return bool - true if success writing else false.
     */
    bool Save( FILE* aFile ) const;

    /**
     * Load schematic global label entry from \a aLine in a .sch file.
     *
     * @param aLine - Essentially this is file to read schematic global label from.
     * @param aErrorMsg - Description of the error if an error occurs while loading the
     *                    schematic global label.
     * @return True if the schematic global label loaded successfully.
     */
    virtual bool Load( LINE_READER& aLine, wxString& aErrorMsg );

    /**
     * Function GetBoundingBox
     * returns the orthogonal, bounding box of this object for display purposes.
     * This box should be an enclosing perimeter for visible components of this
     * object, and the units should be in the pcb or schematic coordinate system.
     * It is OK to overestimate the size by a few counts.
     */
    EDA_RECT GetBoundingBox() const;

    /**
     * Function CreateGraphicShape (virual)
     * Calculates the graphic shape (a polygon) associated to the text
     * @param aPoints = a buffer to fill with polygon corners coordinates
     * @param aPos = Position of the shape
     */
    virtual void CreateGraphicShape( std::vector <wxPoint>& aPoints, const wxPoint& aPos );

    /** virtual function Mirror_Y
     * mirror item relative to an Y axis
     * @param aYaxis_position = the y axis position
     */
    virtual void Mirror_Y( int aYaxis_position );

    virtual void Mirror_X( int aXaxis_position );

    virtual void Rotate( wxPoint rotationPoint );

    virtual bool IsConnectable() const { return true; }

    virtual wxString GetSelectMenuText() const;

    virtual BITMAP_DEF GetMenuImage() const { return  add_glabel_xpm; }

private:
    virtual bool doHitTest( const wxPoint& aPoint, int aAccuracy ) const;
    virtual bool doIsConnected( const wxPoint& aPosition ) const { return m_Pos == aPosition; }
    virtual EDA_ITEM* doClone() const;
};


class SCH_HIERLABEL : public SCH_TEXT
{
public:
    SCH_HIERLABEL( const wxPoint& pos = wxPoint( 0, 0 ),
                   const wxString& text = wxEmptyString,
                   KICAD_T aType = SCH_HIERARCHICAL_LABEL_T );

    SCH_HIERLABEL( const SCH_HIERLABEL& aHierLabel );

    ~SCH_HIERLABEL() { }

    virtual void Draw( EDA_DRAW_PANEL* panel,
                       wxDC*           DC,
                       const wxPoint&  offset,
                       int             draw_mode,
                       int             Color = -1 );

    virtual wxString GetClass() const
    {
        return wxT( "SCH_HIERLABEL" );
    }

    /**
     * Function SetOrientation
     * Set m_SchematicOrientation, and initialize
     * m_orient,m_HJustified and m_VJustified, according to the value of
     * m_SchematicOrientation
     * must be called after changing m_SchematicOrientation
     * @param aSchematicOrientation =
     *  0 = normal (horizontal, left justified).
     *  1 = up (vertical)
     *  2 =  (horizontal, right justified). This can be seen as the mirrored
     * position of 0
     *  3 = bottom . This can be seen as the mirrored position of up
     */
    virtual void SetOrientation( int aSchematicOrientation );

    /**
     * Function GetSchematicTextOffset (virtual)
     * @return the offset between the SCH_TEXT position and the text itself
     * position
     * This offset depend on orientation, and the type of text
     * (room to draw an associated graphic symbol, or put the text above a
     * wire)
     */
    virtual wxPoint GetSchematicTextOffset() const;

    /**
     * Function CreateGraphicShape
     * Calculates the graphic shape (a polygon) associated to the text
     * @param aPoints = a buffer to fill with polygon corners coordinates
     * @param Pos = Postion of the shape
     */
    virtual void CreateGraphicShape( std::vector <wxPoint>& aPoints, const wxPoint& Pos );

    /**
     * Function Save
     * writes the data structures for this object out to a FILE in "*.sch"
     * format.
     * @param aFile The FILE to write to.
     * @return bool - true if success writing else false.
     */
    bool Save( FILE* aFile ) const;

    /**
     * Load schematic hierarchical label entry from \a aLine in a .sch file.
     *
     * @param aLine - Essentially this is file to read schematic hierarchical label from.
     * @param aErrorMsg - Description of the error if an error occurs while loading the
     *                    schematic hierarchical label.
     * @return True if the schematic hierarchical label loaded successfully.
     */
    virtual bool Load( LINE_READER& aLine, wxString& aErrorMsg );

    /**
     * Function GetBoundingBox
     * returns the orthogonal, bounding box of this object for display purposes.
     * This box should be an enclosing perimeter for visible components of this
     * object, and the units should be in the pcb or schematic coordinate system.
     * It is OK to overestimate the size by a few counts.
     */
    EDA_RECT GetBoundingBox() const;

    /** virtual function Mirror_Y
     * mirror item relative to an Y axis
     * @param aYaxis_position = the y axis position
     */
    virtual void Mirror_Y( int aYaxis_position );

    virtual void Mirror_X( int aXaxis_position );

    virtual void Rotate( wxPoint rotationPoint );

    virtual bool IsConnectable() const { return true; }

    virtual wxString GetSelectMenuText() const;

    virtual BITMAP_DEF GetMenuImage() const { return  add_hierarchical_label_xpm; }

private:
    virtual bool doHitTest( const wxPoint& aPoint, int aAccuracy ) const;
    virtual bool doIsConnected( const wxPoint& aPosition ) const { return m_Pos == aPosition; }
    virtual EDA_ITEM* doClone() const;
};

#endif /* CLASS_TEXT_LABEL_H */
