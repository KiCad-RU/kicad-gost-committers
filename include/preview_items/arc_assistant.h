/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Kicad Developers, see AUTHORS.txt for contributors.
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

#ifndef PREVIEW_ITEMS_ARC_ASSISTANT_H
#define PREVIEW_ITEMS_ARC_ASSISTANT_H

#include <base_struct.h>
#include <preview_items/arc_geom_manager.h>

namespace KIGFX {
namespace PREVIEW {
/**
 * Class SELECTION_AREA
 *
 * Represents an assitant draw when interactively drawing an
 * arc on a canvas.
 */
class ARC_ASSISTANT : public EDA_ITEM
{
public:

    ARC_ASSISTANT( const ARC_GEOM_MANAGER& aManager );

    const BOX2I ViewBBox() const override;

    /**
     * Draw the assistance (with reference to the contstruction manager
     */
    void ViewDraw( int aLayer, KIGFX::VIEW* aView ) const override final;

#if defined(DEBUG)
    void Show( int x, std::ostream& st ) const override
    {
    }

#endif

    /**
     * Get class name
     * @return  string "ARC_ASSISTANT"
     */
    wxString GetClass() const override
    {
        return "ARC_ASSISTANT";
    }

private:

    const ARC_GEOM_MANAGER& m_constructMan;
};
}       // PREVIEW
}       // KIGFX

#endif  // PREVIEW_ITEMS_ARC_ASSISTANT_H
