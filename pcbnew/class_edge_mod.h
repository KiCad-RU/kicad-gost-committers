/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file class_edge_mod.h
 * @brief EDGE_MODULE class definition.
 */

#ifndef CLASS_EDGE_MOD_H_
#define CLASS_EDGE_MOD_H_


#include <class_drawsegment.h>


class LINE_READER;
class EDA_3D_CANVAS;
class EDA_DRAW_FRAME;


class EDGE_MODULE : public DRAWSEGMENT
{
public:
    EDGE_MODULE( MODULE* parent, STROKE_T aShape = S_SEGMENT );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.
    // EDGE_MODULE( const EDGE_MODULE& );

    ~EDGE_MODULE();

    EDGE_MODULE* Next() const { return (EDGE_MODULE*) Pnext; }
    EDGE_MODULE* Back() const { return (EDGE_MODULE*) Pback; }

    void Copy( EDGE_MODULE* source );           // copy structure

    void SetStart0( const wxPoint& aPoint )     { m_Start0 = aPoint; }
    const wxPoint& GetStart0() const            { return m_Start0; }

    void SetEnd0( const wxPoint& aPoint )       { m_End0 = aPoint; }
    const wxPoint& GetEnd0() const              { return m_End0; }

    void SetDrawCoord();

    /* drawing functions */
    void Draw( EDA_DRAW_PANEL* panel, wxDC* DC,
               GR_DRAWMODE aDrawMode, const wxPoint& offset = ZeroOffset );

    void Draw3D( EDA_3D_CANVAS* glcanvas );

    void DisplayInfo( EDA_DRAW_FRAME* frame );

    wxString GetClass() const
    {
        return wxT( "MGRAPHIC" );

        // return wxT( "EDGE" );  ?
    }

    wxString GetSelectMenuText() const;

    BITMAP_DEF GetMenuImage() const { return  show_mod_edge_xpm; }

    EDA_ITEM* Clone() const;

#if defined(DEBUG)
    void Show( int nestLevel, std::ostream& os ) const;     // overload
#endif

//protected:  @todo: is it just me?

    wxPoint m_Start0;       // Start point or center, relative to module origin, orient 0.
    wxPoint m_End0;         // End point, relative to module origin, orient 0.
};

#endif    // CLASS_EDGE_MOD_H_
