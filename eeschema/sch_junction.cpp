/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file sch_junction.cpp
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <macros.h>
#include <class_drawpanel.h>
#include <trigo.h>
#include <common.h>
#include <richio.h>
#include <plot_common.h>

#include <general.h>
#include <protos.h>
#include <sch_junction.h>
#include <class_netlist_object.h>


SCH_JUNCTION::SCH_JUNCTION( const wxPoint& pos ) :
    SCH_ITEM( NULL, SCH_JUNCTION_T )
{
#define DRAWJUNCTION_DIAMETER 32   /* Diameter of junction symbol between wires */
    m_pos    = pos;
    m_Layer  = LAYER_JUNCTION;
    m_size.x = m_size.y = DRAWJUNCTION_DIAMETER;
#undef DRAWJUNCTION_DIAMETER
}


bool SCH_JUNCTION::Save( FILE* aFile ) const
{
    bool success = true;

    if( fprintf( aFile, "Connection ~ %-4d %-4d\n", m_pos.x, m_pos.y ) == EOF )
    {
        success = false;
    }

    return success;
}


EDA_ITEM* SCH_JUNCTION::Clone() const
{
    return new SCH_JUNCTION( *this );
}


void SCH_JUNCTION::SwapData( SCH_ITEM* aItem )
{
    wxCHECK_RET( (aItem != NULL) && (aItem->Type() == SCH_JUNCTION_T),
                 wxT( "Cannot swap junction data with invalid item." ) );

    SCH_JUNCTION* item = (SCH_JUNCTION*) aItem;
    EXCHG( m_pos, item->m_pos );
    EXCHG( m_size, item->m_size );
}


bool SCH_JUNCTION::Load( LINE_READER& aLine, wxString& aErrorMsg )
{
    char name[256];
    char* line = (char*) aLine;

    while( (*line != ' ' ) && *line )
        line++;

    if( sscanf( line, "%s %d %d", name, &m_pos.x, &m_pos.y ) != 3 )
    {
        aErrorMsg.Printf( wxT( "Eeschema file connection load error at line %d, aborted" ),
                          aLine.LineNumber() );
        aErrorMsg << wxT( "\n" ) << FROM_UTF8( (char*) aLine );
        return false;
    }

    return true;
}


EDA_RECT SCH_JUNCTION::GetBoundingBox() const
{
    EDA_RECT rect;

    rect.SetOrigin( m_pos );
    rect.Inflate( ( GetPenSize() + m_size.x ) / 2 );

    return rect;
}


void SCH_JUNCTION::Draw( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
                         GR_DRAWMODE aDrawMode, EDA_COLOR_T aColor )
{
    EDA_COLOR_T color;

    if( aColor >= 0 )
        color = aColor;
    else
        color = ReturnLayerColor( m_Layer );

    GRSetDrawMode( aDC, aDrawMode );

    GRFilledCircle( aPanel->GetClipBox(), aDC, m_pos.x + aOffset.x, m_pos.y + aOffset.y,
                    ( m_size.x / 2 ), 0, color, color );
}


void SCH_JUNCTION::MirrorX( int aXaxis_position )
{
    m_pos.y -= aXaxis_position;
    NEGATE( m_pos.y );
    m_pos.y += aXaxis_position;
}


void SCH_JUNCTION::MirrorY( int aYaxis_position )
{
    m_pos.x -= aYaxis_position;
    NEGATE( m_pos.x );
    m_pos.x += aYaxis_position;
}


void SCH_JUNCTION::Rotate( wxPoint aPosition )
{
    RotatePoint( &m_pos, aPosition, 900 );
}


void SCH_JUNCTION::GetEndPoints( std::vector <DANGLING_END_ITEM>& aItemList )
{
    DANGLING_END_ITEM item( JUNCTION_END, this, m_pos );
    aItemList.push_back( item );
}


bool SCH_JUNCTION::IsSelectStateChanged( const wxRect& aRect )
{
    bool previousState = IsSelected();

    if( aRect.Contains( m_pos ) )
        m_Flags |= SELECTED;
    else
        m_Flags &= ~SELECTED;

    return previousState != IsSelected();
}


void SCH_JUNCTION::GetConnectionPoints( vector< wxPoint >& aPoints ) const
{
    aPoints.push_back( m_pos );
}


void SCH_JUNCTION::GetNetListItem( vector<NETLIST_OBJECT*>& aNetListItems,
                                   SCH_SHEET_PATH*          aSheetPath )
{
    NETLIST_OBJECT* item = new NETLIST_OBJECT();

    item->m_SheetList = *aSheetPath;
    item->m_SheetListInclude = *aSheetPath;
    item->m_Comp = (SCH_ITEM*) this;
    item->m_Type = NET_JUNCTION;
    item->m_Start = item->m_End = m_pos;

    aNetListItems.push_back( item );
}


#if defined(DEBUG)
void SCH_JUNCTION::Show( int nestLevel, std::ostream& os ) const
{
    // XML output:
    wxString s = GetClass();

    NestedSpace( nestLevel, os ) << '<' << s.Lower().mb_str() << m_pos << "/>\n";
}
#endif


bool SCH_JUNCTION::HitTest( const wxPoint& aPosition, int aAccuracy ) const
{
    EDA_RECT rect = GetBoundingBox();

    rect.Inflate( aAccuracy );

    return rect.Contains( aPosition );
}


bool SCH_JUNCTION::HitTest( const EDA_RECT& aRect, bool aContained, int aAccuracy ) const
{
    if( m_Flags & STRUCT_DELETED || m_Flags & SKIP_STRUCT )
        return false;

    EDA_RECT rect = aRect;

    rect.Inflate( aAccuracy );

    if( aContained )
        return rect.Contains( GetBoundingBox() );

    return rect.Intersects( GetBoundingBox() );
}


bool SCH_JUNCTION::doIsConnected( const wxPoint& aPosition ) const
{
    return m_pos == aPosition;
}


void SCH_JUNCTION::Plot( PLOTTER* aPlotter )
{
    aPlotter->SetColor( ReturnLayerColor( GetLayer() ) );
    aPlotter->Circle( m_pos, m_size.x, FILLED_SHAPE );
}
