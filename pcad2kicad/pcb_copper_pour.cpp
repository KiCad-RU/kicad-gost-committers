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
 * @file pcb_copper_pour.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <pcb_copper_pour.h>

PCB_COPPER_POUR::PCB_COPPER_POUR( PCB_CALLBACKS*    aCallbacks,
                                  BOARD*            aBoard ) : PCB_POLYGON( aCallbacks, aBoard )
{
}


PCB_COPPER_POUR::~PCB_COPPER_POUR()
{
}


bool PCB_COPPER_POUR::Parse( wxXmlNode*     aNode,
                             int            aPCadLayer,
                             wxString       aDefaultMeasurementUnit,
                             wxString       aActualConversion,
                             wxStatusBar*   aStatusBar )
{
    wxXmlNode*      lNode, * tNode, * cNode;
    wxString        pourType, str, propValue;
    int             pourSpacing, thermalWidth;
    VERTICES_ARRAY* island, * cutout;

    // aStatusBar->SetStatusText( aStatusBar->GetStatusText() + wxT( " CooperPour..." ) );
    m_PCadLayer     = aPCadLayer;
    m_KiCadLayer    = GetKiCadLayer();
    m_timestamp     = GetNewTimestamp();

    //str = FindNode( aNode, wxT( "pourType" ) )->GetNodeContent();
    //str.Trim( false );
    //pourType = str.MakeUpper();

    lNode = FindNode( aNode, wxT( "netNameRef" ) );

    if( lNode )
    {
        lNode->GetAttribute( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_net = propValue;
    }

    if( FindNode( aNode, wxT( "width" ) ) )
        SetWidth( FindNode( aNode, wxT( "width" ) )->GetNodeContent(),
                  aDefaultMeasurementUnit, &m_width, aActualConversion );

    if( FindNode( aNode, wxT( "pourSpacing" ) ) )
        SetWidth( FindNode( aNode, wxT( "pourSpacing" ) )->GetNodeContent(),
                  aDefaultMeasurementUnit, &pourSpacing, aActualConversion );

    if( FindNode( aNode, wxT( "thermalWidth" ) ) )
        SetWidth( FindNode( aNode, wxT( "thermalWidth" ) )->GetNodeContent(),
                  aDefaultMeasurementUnit, &thermalWidth, aActualConversion );

    lNode = FindNode( aNode, wxT( "pcbPoly" ) );

    if( lNode )
    {
        // retrieve copper pour outline
        FormPolygon( lNode, &m_outline, aDefaultMeasurementUnit, aActualConversion );

        m_positionX = m_outline[0]->x;
        m_positionY = m_outline[0]->y;

        lNode = FindNode( aNode, wxT( "island" ) );

        while( lNode )
        {
            tNode = FindNode( lNode, wxT( "islandOutline" ) );

            if( tNode )
            {
                island = new VERTICES_ARRAY;
                FormPolygon( tNode, island, aDefaultMeasurementUnit, aActualConversion );
                m_islands.Add( island );
                tNode = FindNode( lNode, wxT( "cutout" ) );

                while( tNode )
                {
                    cNode = FindNode( tNode, wxT( "cutoutOutline" ) );

                    if( cNode )
                    {
                        cutout = new VERTICES_ARRAY;
                        FormPolygon( cNode, cutout, aDefaultMeasurementUnit, aActualConversion );
                        m_cutouts.Add( cutout );
                    }

                    tNode = tNode->GetNext();
                }
            }

            /*tNode:=lNode.ChildNodes.FindNode('thermal');
             *  while  Assigned(tNode) do
             *  begin
             *   DrawThermal(tNode, componentCopperPour.fill_lines, PCADlayer, thermalWidth,
             *               componentCopperPour.timestamp);
             *   tNode := tNode.NextSibling;
             *  end;*/

            lNode = lNode->GetNext();
        }
    }
    else
    {
        return false;
    }

    return true;
}
