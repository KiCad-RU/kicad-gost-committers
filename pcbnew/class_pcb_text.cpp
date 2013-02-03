/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file class_pcb_text.cpp
 * @brief Class TEXTE_PCB texts on copper or technical layers implementation
 */

#include <fctsys.h>
#include <wxstruct.h>
#include <gr_basic.h>
#include <base_struct.h>
#include <drawtxt.h>
#include <kicad_string.h>
#include <trigo.h>
#include <pcbcommon.h>
#include <colors_selection.h>
#include <richio.h>
#include <class_drawpanel.h>
#include <macros.h>
#include <msgpanel.h>
#include <base_units.h>

#include <class_board.h>
#include <class_pcb_text.h>

#include <protos.h>


TEXTE_PCB::TEXTE_PCB( BOARD_ITEM* parent ) :
    BOARD_ITEM( parent, PCB_TEXT_T ),
    EDA_TEXT()
{
    m_MultilineAllowed = true;
}


TEXTE_PCB:: ~TEXTE_PCB()
{
}


void TEXTE_PCB::Copy( TEXTE_PCB* source )
{
    m_Parent    = source->m_Parent;
    Pback       = Pnext = NULL;
    m_Mirror    = source->m_Mirror;
    m_Size      = source->m_Size;
    m_Orient    = source->m_Orient;
    m_Pos       = source->m_Pos;
    m_Layer     = source->m_Layer;
    m_Thickness = source->m_Thickness;
    m_Attributs = source->m_Attributs;
    m_Italic    = source->m_Italic;
    m_Bold      = source->m_Bold;
    m_HJustify  = source->m_HJustify;
    m_VJustify  = source->m_VJustify;
    m_MultilineAllowed = m_MultilineAllowed;

    m_Text = source->m_Text;
}


void TEXTE_PCB::Draw( EDA_DRAW_PANEL* panel, wxDC* DC,
                      GR_DRAWMODE DrawMode, const wxPoint& offset )
{
    BOARD* brd = GetBoard();

    if( brd->IsLayerVisible( m_Layer ) == false )
        return;

    EDA_COLOR_T color = brd->GetLayerColor( m_Layer );

    EDA_DRAW_MODE_T fillmode = FILLED;

    if( DisplayOpt.DisplayDrawItems == SKETCH )
        fillmode = SKETCH;

    EDA_COLOR_T anchor_color = UNSPECIFIED_COLOR;

    if( brd->IsElementVisible( ANCHOR_VISIBLE ) )
        anchor_color = brd->GetVisibleElementColor( ANCHOR_VISIBLE );

    EDA_TEXT::Draw( panel, DC, offset, color,
                    DrawMode, fillmode, anchor_color );
}


void TEXTE_PCB::GetMsgPanelInfo( std::vector< MSG_PANEL_ITEM >& aList )
{
    wxString    msg;
    BOARD*      board;
    BOARD_ITEM* parent = (BOARD_ITEM*) m_Parent;

    wxASSERT( parent );

    if( parent->Type() == PCB_DIMENSION_T )
        board = (BOARD*) parent->GetParent();
    else
        board = (BOARD*) parent;

    wxASSERT( board );

    if( m_Parent && m_Parent->Type() == PCB_DIMENSION_T )
        aList.push_back( MSG_PANEL_ITEM( _( "DIMENSION" ), m_Text, DARKGREEN ) );
    else
        aList.push_back( MSG_PANEL_ITEM( _( "PCB Text" ), m_Text, DARKGREEN ) );

    aList.push_back( MSG_PANEL_ITEM( _( "Layer" ),
                                     board->GetLayerName( m_Layer ), BLUE ) );

    if( !m_Mirror )
        aList.push_back( MSG_PANEL_ITEM( _( "Mirror" ), _( "No" ), DARKGREEN ) );
    else
        aList.push_back( MSG_PANEL_ITEM( _( "Mirror" ), _( "Yes" ), DARKGREEN ) );

    msg.Printf( wxT( "%.1f" ), (float) m_Orient / 10 );
    aList.push_back( MSG_PANEL_ITEM( _( "Orientation" ), msg, DARKGREEN ) );

    msg = ::CoordinateToString( m_Thickness );
    aList.push_back( MSG_PANEL_ITEM( _( "Thickness" ), msg, MAGENTA ) );

    msg = ::CoordinateToString( m_Size.x );
    aList.push_back( MSG_PANEL_ITEM( _( "Size X" ), msg, RED ) );

    msg = ::CoordinateToString( m_Size.y );
    aList.push_back( MSG_PANEL_ITEM( _( "Size Y" ), msg, RED ) );
}


void TEXTE_PCB::Rotate( const wxPoint& aRotCentre, double aAngle )
{
    RotatePoint( &m_Pos, aRotCentre, aAngle );
    m_Orient += aAngle;
    NORMALIZE_ANGLE_360( m_Orient );
}


void TEXTE_PCB::Flip(const wxPoint& aCentre )
{
    m_Pos.y  = aCentre.y - ( m_Pos.y - aCentre.y );
//    NEGATE( m_Orient );   not needed: m_Mirror handles this
    if( GetLayer() == LAYER_N_BACK
        || GetLayer() == LAYER_N_FRONT
        || GetLayer() == SILKSCREEN_N_BACK
        || GetLayer() == SILKSCREEN_N_FRONT )
    {
        m_Mirror = not m_Mirror;      /* inverse mirror */
    }
    SetLayer( BOARD::ReturnFlippedLayerNumber( GetLayer() ) );
}


wxString TEXTE_PCB::GetSelectMenuText() const
{
    wxString text, shorttxt;

    if( m_Text.Len() < 12 )
        shorttxt << m_Text;
    else
        shorttxt += m_Text.Left( 10 ) + wxT( ".." );

    text.Printf( _( "Pcb Text %s on %s"),
                 GetChars ( shorttxt ), GetChars( GetLayerName() ) );

    return text;
}


EDA_ITEM* TEXTE_PCB::Clone() const
{
    return new TEXTE_PCB( *this );
}


#if defined(DEBUG)

void TEXTE_PCB::Show( int nestLevel, std::ostream& os ) const
{
    // for now, make it look like XML:
    NestedSpace( nestLevel, os ) << '<' << GetClass().Lower().mb_str() <<
    " string=\"" << m_Text.mb_str() << "\"/>\n";

//    NestedSpace( nestLevel, os ) << "</" << GetClass().Lower().mb_str()
//                                 << ">\n";
}


#endif
