/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 KiCad Developers, see change_log.txt for contributors.
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

#include <sch_base_frame.h>
#include <viewlib_frame.h>
#include <libeditframe.h>
#include <base_units.h>

SCH_BASE_FRAME::SCH_BASE_FRAME( wxWindow* aParent,
                                ID_DRAWFRAME_TYPE aWindowType,
                                const wxString& aTitle,
                                const wxPoint& aPosition, const wxSize& aSize,
                                long aStyle, const wxString & aFrameName ) :
    EDA_DRAW_FRAME( aParent, aWindowType, aTitle, aPosition, aSize, aStyle, aFrameName )
{
}


void SCH_BASE_FRAME::OnOpenLibraryViewer( wxCommandEvent& event )
{
    LIB_VIEW_FRAME * viewlibFrame = LIB_VIEW_FRAME::GetActiveLibraryViewer();;

    if( viewlibFrame )
        viewlibFrame->Show( true );
    else
        new LIB_VIEW_FRAME( this );
}


SCH_SCREEN* SCH_BASE_FRAME::GetScreen() const
{
    return (SCH_SCREEN*) EDA_DRAW_FRAME::GetScreen();
}


void SCH_BASE_FRAME::SetPageSettings( const PAGE_INFO& aPageSettings )
{
    GetScreen()->SetPageSettings( aPageSettings );
}


const PAGE_INFO& SCH_BASE_FRAME::GetPageSettings () const
{
    return GetScreen()->GetPageSettings();
}


const wxSize SCH_BASE_FRAME::GetPageSizeIU() const
{
    // GetSizeIU is compile time dependent:
    return GetScreen()->GetPageSettings().GetSizeIU();
}


const wxPoint& SCH_BASE_FRAME::GetOriginAxisPosition() const
{
    wxASSERT( GetScreen() );
    return GetScreen()->GetOriginAxisPosition();
}


void SCH_BASE_FRAME::SetOriginAxisPosition( const wxPoint& aPosition )
{
    wxASSERT( GetScreen() );
    GetScreen()->SetOriginAxisPosition( aPosition );
}


const TITLE_BLOCK& SCH_BASE_FRAME::GetTitleBlock() const
{
    wxASSERT( GetScreen() );
    return GetScreen()->GetTitleBlock();
}


void SCH_BASE_FRAME::SetTitleBlock( const TITLE_BLOCK& aTitleBlock )
{
    wxASSERT( GetScreen() );
    GetScreen()->SetTitleBlock( aTitleBlock );
}


void SCH_BASE_FRAME::UpdateStatusBar()
{
    wxString        line;
    int             dx, dy;
    BASE_SCREEN*    screen = GetScreen();

    if( !screen )
        return;

    EDA_DRAW_FRAME::UpdateStatusBar();

    // Display absolute coordinates:
    double dXpos = To_User_Unit( g_UserUnit, screen->GetCrossHairPosition().x );
    double dYpos = To_User_Unit( g_UserUnit, screen->GetCrossHairPosition().y );

    if ( g_UserUnit == MILLIMETRES )
    {
        dXpos = RoundTo0( dXpos, 100.0 );
        dYpos = RoundTo0( dYpos, 100.0 );
    }

    wxString absformatter;
    wxString locformatter;

    switch( g_UserUnit )
    {
    case INCHES:
        absformatter = wxT( "X %.3f  Y %.3f" );
        locformatter = wxT( "dx %.3f  dy %.3f  d %.3f" );
        break;

    case MILLIMETRES:
        absformatter = wxT( "X %.2f  Y %.2f" );
        locformatter = wxT( "dx %.2f  dy %.2f  d %.2f" );
        break;

    case UNSCALED_UNITS:
        absformatter = wxT( "X %f  Y %f" );
        locformatter = wxT( "dx %f  dy %f  d %f" );
        break;
    }

    line.Printf( absformatter, dXpos, dYpos );
    SetStatusText( line, 2 );

    // Display relative coordinates:
    dx = screen->GetCrossHairPosition().x - screen->m_O_Curseur.x;
    dy = screen->GetCrossHairPosition().y - screen->m_O_Curseur.y;
    dXpos = To_User_Unit( g_UserUnit, dx );
    dYpos = To_User_Unit( g_UserUnit, dy );

    if( g_UserUnit == MILLIMETRES )
    {
        dXpos = RoundTo0( dXpos, 100.0 );
        dYpos = RoundTo0( dYpos, 100.0 );
    }

    // We already decided the formatter above
    line.Printf( locformatter, dXpos, dYpos, sqrt( dXpos * dXpos + dYpos * dYpos ) );
    SetStatusText( line, 3 );

    // refresh units display
    DisplayUnitsMsg();
}
