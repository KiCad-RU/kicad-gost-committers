/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2012 KiCad Developers, see CHANGELOG.TXT for contributors.
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
 * @file sch_sheet.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/xml/xml.h>
#include <wx/filename.h>

#include <pcad2kicad_common.h>
#include <common.h>

#include <sch_sheet.h>
#include <sch_arc.h>
#include <sch_bus.h>
#include <sch_module.h>
#include <sch_pin.h>
#include <sch_port.h>
#include <sch_symbol.h>

namespace PCAD2KICAD {

SCH_SHEET::SCH_SHEET()
{
    m_sizeX = 0;
    m_sizeY = 0;
}


SCH_SHEET::~SCH_SHEET()
{
    int i;

    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
        delete m_schComponents[i];
}


// LinesIntersect procedure is copied from website
// http://www.pdas.com/lineint.htm
// extended format replaced by integer
void SCH_SHEET::LinesIntersect( const int aX1, const int aY1,
                          const int aX2, const int aY2,     // first line
                          const int aX3, const int aY3,
                          const int aX4, const int aY4,     // second line
                          int* aCode,                       // =0 OK; =1 lines parallel
                          int* aX, int* aY )
{
    // intersection point
    long double a1, a2, b1, b2, c1, c2;    // Coefficients of line eqns./
    long double denom;

    a1  = aY2 - aY1;
    b1  = aX1 - aX2;

    // a1*x + b1*y + c1 = 0 is line 1
    c1 = (long double) aX2 * (long double) aY1 - (long double) aX1 * (long double) aY2;

    a2  = aY4 - aY3;
    b2  = aX3 - aX4;

    // a2*x + b2*y + c2 = 0 is line 2
    c2 = (long double) aX4 * (long double) aY3 - (long double) aX3 * (long double) aY4;

    denom = a1 * b2 - a2 * b1;

    if( denom == 0 )
    {
        *aCode = 1;
    }
    else
    {
        *aX     = KiROUND( (b1 * c2 - b2 * c1) / denom );
        *aY     = KiROUND( (a2 * c1 - a1 * c2) / denom );
        *aCode  = 0;
    }
}


bool SCH_SHEET::IsPointOnLine( int aX, int aY, SCH_LINE* aLine )
{
    long double a, b, c;    // Coefficients of line eqns./
    bool        px, py, result = false;

    a   = aLine->m_toY - aLine->m_positionY;
    b   = aLine->m_positionX - aLine->m_toX;

    // a1*x + b1*y + c1 = 0 is line 1
    c = (long double) aLine->m_toX * (long double) aLine->m_positionY -
        (long double) aLine->m_positionX * (long double) aLine->m_toY;
    px = false; py = false;

    if( a * (long double) aX + b * (long double) aY + c == 0 )
    {
        if( aLine->m_positionX <= aLine->m_toX && aX >= aLine->m_positionX && aX <= aLine->m_toX )
            px = true;

        if( aLine->m_toX <= aLine->m_positionX && aX >= aLine->m_toX && aX <= aLine->m_positionX )
            px = true;

        if( aLine->m_positionY <= aLine->m_toY && aY >= aLine->m_positionY && aY <= aLine->m_toY )
            py = true;

        if( aLine->m_toY <= aLine->m_positionY && aY >= aLine->m_toY && aY <= aLine->m_positionY )
            py = true;

        if( px && py )
            result = true;
    }

    return result;
}


SCH_JUNCTION* SCH_SHEET::CheckJunction( SCH_LINE* aSchLine, int aIndex )
{
    SCH_JUNCTION*   result = NULL;
    int             i, j, code, x, y;
    bool            p;

    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( (m_schComponents[i])->m_objType == wxT( "line" )
            && aIndex != i )    // not  for itself
        {
            if( aSchLine->m_net == ( (SCH_LINE*) m_schComponents[i] )->m_net )
            {
                // IS JUNCTION ????
                LinesIntersect( aSchLine->m_positionX,
                                aSchLine->m_positionY,
                                aSchLine->m_toX,
                                aSchLine->m_toY,
                                ( (SCH_LINE*) m_schComponents[i] )->m_positionX,
                                ( (SCH_LINE*) m_schComponents[i] )->m_positionY,
                                ( (SCH_LINE*) m_schComponents[i] )->m_toX,
                                ( (SCH_LINE*) m_schComponents[i] )->m_toY,
                                &code,
                                &x,
                                &y );

                if( code == 0 )
                {
                    // there is intersection, is it inside line segment ?
                    if( IsPointOnLine( x, y, aSchLine ) )
                    {
                        // Point Candidate
                        // Firstly to check if the other line with different NET
                        // is crossing this point, if so, there is not point !
                        p = true;

                        for( j = 0; j < (int) m_schComponents.GetCount(); j++ )
                        {
                            if( (m_schComponents[j])->m_objType == wxT( "line" )
                                && p )    // not  for itself
                            {
                                if( ( (SCH_LINE*) m_schComponents[j] )->m_net !=
                                    ( (SCH_LINE*) m_schComponents[i] )->m_net )
                                    if( IsPointOnLine( x, y, (SCH_LINE*) m_schComponents[j] ) )
                                        p = false;




                                // NOT POINT - net cross
                            }
                        }

                        if( p )
                            result = new SCH_JUNCTION( x, y, aSchLine->m_net );
                    }
                }
            }
        }
    }

    return result;
}


void SCH_SHEET::Parse( XNODE* aNode, wxString aDefaultMeasurementUnit )
{
    SCH_COMPONENT*  schComp;
    SCH_JUNCTION*   schJunction;
    SCH_LINE*       line;
    SCH_PORT*       port;
    SCH_BUS*        bus;
    SCH_JUNCTION*   junction;
    SCH_SYMBOL*     symbol;
    bool            isJunction;
    int             i;

    aNode->GetAttribute( wxT( "Name" ), &m_name );

    aNode = aNode->GetChildren();
    while( aNode )
    {
        if( aNode->GetName() == wxT( "symbol" ) )
        {
            symbol = new SCH_SYMBOL;
            symbol->Parse( aNode, aDefaultMeasurementUnit, wxT( "SCH" ) );
            m_schComponents.Add( symbol );
        }

        if( aNode->GetName() == wxT( "wire" ) )
            if( FindNode( aNode, wxT( "line" ) ) )
            {
                line = new SCH_LINE;
                line->Parse( FindNode( aNode, wxT( "line" ) ), 0,
                             aDefaultMeasurementUnit, wxT( "SCH" ) );
                m_schComponents.Add( line );
            }

        if( aNode->GetName() == wxT( "port" ) )
        {
            port = new SCH_PORT;
            port->Parse( aNode, aDefaultMeasurementUnit, wxT( "SCH" ) );
            m_schComponents.Add( port );
        }

        if( aNode->GetName() == wxT( "bus" ) )
        {
            bus = new SCH_BUS;
            bus->Parse( aNode, aDefaultMeasurementUnit, wxT( "SCH" ) );
            m_schComponents.Add( bus );
        }

        if( aNode->GetName() == wxT( "junction" ) )
        {
            isJunction  = true;
            junction    = new SCH_JUNCTION();
            junction->Parse( aNode, aDefaultMeasurementUnit, wxT( "SCH" ) );
            m_schComponents.Add( junction );
        }

        aNode = aNode->GetNext();
    }

    // POSTPROCESS -- SET/OPTIMIZE NEW SCH POSITION

    m_sizeX = 1000000;
    m_sizeY = 0;

    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        schComp = m_schComponents[i];

        if( schComp->m_positionY < m_sizeY )
            m_sizeY = schComp->m_positionY;
        if( schComp->m_positionX < m_sizeX && schComp->m_positionX > 0 )
            m_sizeX = schComp->m_positionX;
    }

    // correction
    m_sizeY = m_sizeY - 1000;
    m_sizeX = m_sizeX - 1000;
    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        schComp = m_schComponents[i];
        schComp->SetPosOffset( -m_sizeX, -m_sizeY );
    }

    // final sheet settings
    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        schComp = m_schComponents[i];

        if( schComp->m_positionY < m_sizeY )
            m_sizeY = schComp->m_positionY;

        if( schComp->m_positionX > m_sizeX )
            m_sizeX = schComp->m_positionX;
    }

    m_sizeY = -m_sizeY;    // is in absolute units
    m_sizeX += 1000;
    m_sizeY += 1000;

    // A4 is minimum $Descr A4 11700 8267
    if( m_sizeX < 11700 )
        m_sizeX = 11700;

    if( m_sizeY < 8267 )
        m_sizeY = 8267;

    // POSTPROCESS -- CREATE JUNCTIONS FROM NEWTLIST
    if( !isJunction )
    {
        if( wxMessageBox( wxT(
                              "There are not JUNCTIONS in your schematics file . \
It can be, that your design is without Junctions. \
But it can be that your input file is in obsolete format. \
Would you like to run postprocess and create junctions from Netlist/Wires information ?    \
YOU HAVE TO CHECK/CORRECT Juntions in converted design, placement is only approximation !"                                                                                                                                                                                                                                                                                                     ),
                          wxEmptyString, wxYES_NO ) == wxYES )
        {
            for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
            {
                schComp = m_schComponents[i];

                if( schComp->m_objType == wxT( "line" ) )
                {
                    schJunction = CheckJunction( (SCH_LINE*) schComp, i );

                    if( schJunction )
                        m_schComponents.Add( schJunction );
                }
            }
        }
    }
}


void SCH_SHEET::WriteToFile( wxString aFileName )
{
    wxFile  f;
    int     i;

    f.Open( aFileName + wxT( "_" ) + m_name + wxT( ".sch" ), wxFile::write );
    f.Write( wxT( "EESchema Schematic File Version 1\n" ) );
    wxFileName tmpFile( aFileName );
    tmpFile.SetExt( wxEmptyString );
    f.Write( wxT( "LIBS:" ) + aFileName + wxT( "\n" ) );
    f.Write( wxT( "EELAYER 43  0\n" ) );
    f.Write( wxT( "EELAYER END\n" ) );
    f.Write( wxT( "$Descr User " ) + wxString::Format( wxT( "%d" ),
                                                       m_sizeX ) + wxT( ' ' ) +
             wxString::Format( wxT( "%d" ), m_sizeY ) + wxT( "\n" ) );
    f.Write( wxT( "$EndDescr\n" ) );

    // Junctions
    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( m_schComponents[i]->m_objType == wxT( "junction" ) )
            m_schComponents[i]->WriteToFile( &f, wxT( 'S' ) );
    }

    // Lines
    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( m_schComponents[i]->m_objType == wxT( "line" ) )
            m_schComponents[i]->WriteToFile( &f, wxT( 'S' ) );
    }

    // Ports
    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( m_schComponents[i]->m_objType == wxT( "port" ) )
            m_schComponents[i]->WriteToFile( &f, wxT( 'S' ) );
    }

    // Labels of lines - line and bus names
    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( m_schComponents[i]->m_objType == wxT( "line" ) )
            ( (SCH_LINE*) m_schComponents[i] )->WriteLabelToFile( &f, wxT( 'S' ) );
    }

    // Symbols
    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( m_schComponents[i]->m_objType == wxT( "symbol" ) )
        {
            f.Write( wxT( "$Comp\n" ) );
            m_schComponents[i]->WriteToFile( &f, wxT( 'S' ) );
            f.Write( wxT( "$EndComp\n" ) );
        }
    }

    f.Write( wxT( "$EndSCHEMATC\n" ) );
    f.Close();
}

} // namespace PCAD2KICAD
