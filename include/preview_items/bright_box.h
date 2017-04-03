/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#ifndef __BRIGHT_BOX_H
#define __BRIGHT_BOX_H

#include <math/box2.h>
#include <view/view.h>
#include <base_struct.h>
#include <layers_id_colors_and_visibility.h>
#include <gal/color4d.h>

/**
 * Class BRIGHT_BOX
 *
 * Draws a decoration to indicate a brightened item.
 */
class BRIGHT_BOX : public EDA_ITEM
{
public:
    BRIGHT_BOX();
    ~BRIGHT_BOX() {}

    virtual const BOX2I ViewBBox() const override
    {
        if( !m_item )
        {
            BOX2I bb;
            bb.SetMaximum();
            return bb;
        }

        return m_item->ViewBBox();
    }

    void ViewDraw( int aLayer, KIGFX::VIEW* aView ) const override;

    void ViewGetLayers( int aLayers[], int& aCount ) const override
    {
        aLayers[0] = LAYER_GP_OVERLAY ;
        aCount = 1;
    }

#if defined(DEBUG)
    void Show( int x, std::ostream& st ) const override
    {
    }
#endif

    /** Get class name
     * @return  string "BRIGHT_BOX"
     */
    virtual wxString GetClass() const override
    {
        return wxT( "BRIGHT_BOX" );
    }

    void SetItem( EDA_ITEM* aItem );

    void SetLineWidth( double aWidth )
    {
        m_lineWidth = aWidth;
    }

    void SetColor( KIGFX::COLOR4D aColor )
    {
        m_color = aColor;
    }

protected:
    static const KIGFX::COLOR4D BOX_COLOR;
    static const double LINE_WIDTH;

    EDA_ITEM* m_item;
    double m_lineWidth;
    KIGFX::COLOR4D m_color;
};

#endif
