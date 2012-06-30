/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2007, 2008, 2012 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file pcb_arc.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <pcb_arc.h>


PCB_ARC::PCB_ARC( PCB_CALLBACKS* aCallbacks, BOARD* aBoard ) : PCB_COMPONENT( aCallbacks, aBoard )
{
    m_objType   = 'A';
    m_startX    = 0;
    m_startY    = 0;
    m_angle     = 0;
    m_width     = 0;
}


PCB_ARC::~PCB_ARC()
{
}


void PCB_ARC::Parse( wxXmlNode* aNode,
                     int        aLayer,
                     wxString   aDefaultMeasurementUnit,
                     wxString   aActualConversion )
{
    wxXmlNode*  lNode;
    double      r, a;

    m_PCadLayer     = aLayer;
    m_KiCadLayer    = GetKiCadLayer();
    SetWidth( FindNode( aNode->GetChildren(), wxT( "width" ) )->GetNodeContent(),
              aDefaultMeasurementUnit, &m_width, aActualConversion );

    if( aNode->GetName() == wxT( "triplePointArc" ) )
    {
        // origin
        lNode = FindNode( aNode->GetChildren(), wxT( "pt" ) );

        if( lNode )
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_positionX, &m_positionY, aActualConversion );

        lNode = lNode->GetNext();

        if( lNode )
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_startX, &m_startY, aActualConversion );

        // now temporary, it can be fixed later.....
        m_angle = 3600;
    }

    if( aNode->GetName() == wxT( "arc" ) )
    {
        lNode = FindNode( aNode->GetChildren(), wxT( "pt" ) );

        if( lNode )
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_positionX, &m_positionY, aActualConversion );

        lNode = FindNode( aNode->GetChildren(), wxT( "radius" ) );
        r = StrToIntUnits( lNode->GetNodeContent(), ' ', aActualConversion );
        a = StrToInt1Units( FindNode( aNode->GetChildren(),
                                      wxT( "startAngle" ) )->GetNodeContent() );
        m_startX    = KiROUND( m_positionX + r * sin( (a - 900.0) * M_PI / 1800.0 ) );
        m_startY    = KiROUND( m_positionY - r * cos( (a - 900.0) * M_PI / 1800.0 ) );
        m_angle     = StrToInt1Units( FindNode( aNode->GetChildren(),
                                                wxT( "sweepAngle" ) )->GetNodeContent() );
    }
}


void PCB_ARC::WriteToFile( wxFile* aFile, char aFileType )
{
/*
 *  DC ox oy fx fy w  DC is a Draw Circle  DC Xcentre Ycentre Xpoint Ypoint Width Layer
 *  DA x0 y0 x1 y1 angle width layer  DA is a Draw ArcX0,y0 = Start point x1,y1 = end point
 */
    if( aFileType == 'L' )    // Library component
    {
        aFile->Write( wxString::Format( "DA %d %d %d %d %d %d %d\n",
                                        m_positionX, m_positionY, m_startX,
                                        m_startY, m_angle, m_width,
                                        m_KiCadLayer ) ); // ValueString
    }

    if( aFileType == 'P' )    // PCB
    {
        aFile->Write( wxString::Format( "Po 2 %d %d %d %d %d",
                                        m_positionX, m_positionY,
                                        m_startX, m_startY, m_width ) );
        aFile->Write( wxString::Format( "De %d 0 %d 0 0\n", m_KiCadLayer, -m_angle ) );
    }
}


void PCB_ARC::SetPosOffset( int aX_offs, int aY_offs )
{
    PCB_COMPONENT::SetPosOffset( aX_offs, aY_offs );

    m_startX    += aX_offs;
    m_startY    += aY_offs;
}


void PCB_ARC::AddToModule( MODULE* aModule )
{
}

void PCB_ARC::AddToBoard()
{
}
