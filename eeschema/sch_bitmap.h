/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2011 jean-pierre.charras
 * Copyright (C) 2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file sch_bitmap.h
 *
 */

#ifndef _SCH_BITMAP_H_
#define _SCH_BITMAP_H_


#include <sch_item_struct.h>
#include <class_bitmap_base.h>


class SCH_BITMAP : public SCH_ITEM
{
    wxPoint      m_pos;                 // XY coordinates of center of the bitmap
    BITMAP_BASE* m_image;               // the BITMAP_BASE item

public:
    SCH_BITMAP( const wxPoint& pos = wxPoint( 0, 0 ) );

    SCH_BITMAP( const SCH_BITMAP& aSchBitmap );

    ~SCH_BITMAP()
    {
        delete m_image;
    }

    SCH_ITEM& operator=( const SCH_ITEM& aItem );

    /*
     * Accessors:
     */
    double GetPixelScaleFactor() const { return m_image->GetPixelScaleFactor(); }
    void SetPixelScaleFactor( double aSF ) { m_image->SetPixelScaleFactor( aSF ); }

    BITMAP_BASE* GetImage()
    {
        wxCHECK_MSG( m_image != NULL, NULL, "Invalid SCH_BITMAP initialization, m_image is NULL." );

        return m_image;
    }

    /**
     * Function GetScalingFactor
     * @return the scaling factor from pixel size to actual draw size
     * this scaling factor  depend on m_pixelScaleFactor and m_Scale
     * m_pixelScaleFactor gives the scaling factor between a pixel size and
     * the internal schematic units
     * m_Scale is an user dependant value, and gives the "zoom" value
     *  m_Scale = 1.0 = original size of bitmap.
     *  m_Scale < 1.0 = the bitmap is drawn smaller than its original size.
     *  m_Scale > 1.0 = the bitmap is drawn bigger than its original size.
     */
    double GetScalingFactor() const
    {
        return m_image->GetScalingFactor();
    }


    wxString GetClass() const override
    {
        return wxT( "SCH_BITMAP" );
    }


    /**
     * Function GetSize
     * @returns the actual size (in user units, not in pixels) of the image
     */
    wxSize GetSize() const;

    const EDA_RECT GetBoundingBox() const override;

    void SwapData( SCH_ITEM* aItem ) override;

    void Draw( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
               GR_DRAWMODE aDrawMode, EDA_COLOR_T aColor = UNSPECIFIED_COLOR ) override;

    /**
     * Function ReadImageFile
     * Reads and stores an image file. Init the bitmap used to draw this item
     * format.
     * @param aFullFilename The full filename of the image file to read.
     * @return bool - true if success reading else false.
     */
    bool ReadImageFile( const wxString& aFullFilename );

    bool Save( FILE* aFile ) const override;

    bool Load( LINE_READER& aLine, wxString& aErrorMsg ) override;

    void Move( const wxPoint& aMoveVector ) override
    {
        m_pos += aMoveVector;
    }


    void MirrorY( int aYaxis_position ) override;

    void MirrorX( int aXaxis_position ) override;

    void Rotate( wxPoint aPosition ) override;

    bool IsSelectStateChanged( const wxRect& aRect ) override;

    wxString GetSelectMenuText() const override { return wxString( _( "Image" ) ); }

    BITMAP_DEF GetMenuImage() const override { return image_xpm; }

    wxPoint GetPosition() const override { return m_pos; }

    void SetPosition( const wxPoint& aPosition ) override { m_pos = aPosition; }

    bool HitTest( const wxPoint& aPosition, int aAccuracy ) const override;

    bool HitTest( const EDA_RECT& aRect, bool aContained = false, int aAccuracy = 0 ) const override;

    void Plot( PLOTTER* aPlotter ) override;

    EDA_ITEM* Clone() const override;

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const override;
#endif
};


#endif    // _SCH_BITMAP_H_
