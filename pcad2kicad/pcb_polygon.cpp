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
 * @file pcb_polygon.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <pcb_polygon.h>


PCB_POLYGON::PCB_POLYGON( PCB_CALLBACKS* aCallbacks ) : PCB_COMPONENT( aCallbacks )
{
    m_width     = 0;
    m_objType   = 'Z';
}


PCB_POLYGON::~PCB_POLYGON()
{
    int i, island;

    for( i = 0; i < (int) m_outline.GetCount(); i++ )
    {
        delete m_outline[i];
    }

    for( island = 0; island < (int) m_cutouts.GetCount(); island++ )
    {
        for( i = 0; i < (int) m_cutouts[island]->GetCount(); i++ )
        {
            delete (*m_cutouts[island])[i];
        }

        delete m_cutouts[island];
    }

    for( island = 0; island < (int) m_islands.GetCount(); island++ )
    {
        for( i = 0; i < (int) m_islands[island]->GetCount(); i++ )
        {
            delete (*m_islands[island])[i];
        }

        delete m_islands[island];
    }
}


void PCB_POLYGON::FormPolygon( wxXmlNode* aNode, VERTICES_ARRAY* aPolygon,
                               wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    wxXmlNode*  lNode;
    double      x, y;

    lNode = FindNode( aNode->GetChildren(), wxT( "pt" ) );

    while( lNode )
    {
        if( lNode->GetName() == wxT( "pt" ) )
        {
            SetDoublePrecisionPosition(
                lNode->GetNodeContent(), aDefaultMeasurementUnit, &x, &y, aActualConversion );
            aPolygon->Add( new wxRealPoint( x, y ) );
        }

        lNode = lNode->GetNext();
    }
}


bool PCB_POLYGON::Parse( wxXmlNode*     aNode,
                         int            aPCadLayer,
                         wxString       aDefaultMeasurementUnit,
                         wxString       aActualConversion,
                         wxStatusBar*   aStatusBar )
{
    wxXmlNode*  lNode;
    wxString    propValue;

    aStatusBar->SetStatusText( aStatusBar->GetStatusText() + wxT( " Polygon..." ) );
    m_PCadLayer     = aPCadLayer;
    m_KiCadLayer    = GetKiCadLayer();
    m_timestamp     = GetNewTimestamp();

    lNode = FindNode( aNode->GetChildren(), wxT( "netNameRef" ) );

    if( lNode )
    {
        lNode->GetPropVal( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_net = propValue;
    }

    // retrieve polygon outline
    FormPolygon( aNode, &m_outline, aDefaultMeasurementUnit, aActualConversion );

    m_positionX = m_outline[0]->x;
    m_positionY = m_outline[0]->y;

    // fill the polygon with the same contour as its outline is
    m_islands.Add( new VERTICES_ARRAY );
    FormPolygon( aNode, m_islands[0], aDefaultMeasurementUnit, aActualConversion );

    return true;
}


void PCB_POLYGON::WriteToFile( wxFile* aFile, char aFileType )
{
}


void PCB_POLYGON::WriteOutlineToFile( wxFile* aFile, char aFileType )
{
    int     i, island;
    char    c;

    if( m_outline.GetCount() > 0 )
    {
        aFile->Write( wxT( "$CZONE_OUTLINE\n" ) );
        aFile->Write( wxString::Format( "ZInfo %8X 0 \"", m_timestamp ) + m_net + wxT( "\"\n" ) );
        aFile->Write( wxString::Format( "ZLayer %d\n", m_KiCadLayer ) );
        aFile->Write( wxString::Format( "ZAux %d E\n", (int) m_outline.GetCount() ) );
        // print outline
        c = '0';

        for( i = 0; i < (int) m_outline.GetCount(); i++ )
        {
            if( i == (int) m_outline.GetCount() - 1 )
                c = '1';

            aFile->Write( wxString::Format( "ZCorner %d %d %c\n",
                                            KiROUND( m_outline[i]->x ), KiROUND( m_outline[i]->y ),
                                            c ) );
        }

        // print cutouts
        for( island = 0; island < (int) m_cutouts.GetCount(); island++ )
        {
            c = '0';

            for( i = 0; i < (int) m_cutouts[island]->GetCount(); i++ )
            {
                if( i == (int) m_cutouts[island]->GetCount() - 1 )
                    c = '1';

                aFile->Write( wxString::Format( "ZCorner %d %d %c\n",
                                                KiROUND( (*m_cutouts[island])[i]->x ),
                                                KiROUND( (*m_cutouts[island])[i]->y ), c ) );
            }
        }

        // print filled islands
        for( island = 0; island < (int) m_islands.GetCount(); island++ )
        {
            aFile->Write( wxT( "$POLYSCORNERS\n" ) );
            c = '0';

            for( i = 0; i < (int) m_islands[island]->GetCount(); i++ )
            {
                if( i == (int) m_islands[island]->GetCount() - 1 )
                    c = '1';

                aFile->Write( wxString::Format( "%d %d %c 0\n",
                                                KiROUND( (*m_islands[island])[i]->x ),
                                                KiROUND( (*m_islands[island])[i]->y ), c ) );
            }

            aFile->Write( wxT( "$endPOLYSCORNERS\n" ) );
        }

        aFile->Write( wxT( "$endCZONE_OUTLINE\n" ) );
    }
}


void PCB_POLYGON::SetPosOffset( int aX_offs, int aY_offs )
{
    int i, island;

    PCB_COMPONENT::SetPosOffset( aX_offs, aY_offs );

    for( i = 0; i < (int) m_outline.GetCount(); i++ )
    {
        m_outline[i]->x += aX_offs;
        m_outline[i]->y += aY_offs;
    }

    for( island = 0; island < (int) m_islands.GetCount(); island++ )
    {
        for( i = 0; i < (int) m_islands[island]->GetCount(); i++ )
        {
            (*m_islands[island])[i]->x  += aX_offs;
            (*m_islands[island])[i]->y  += aY_offs;
        }
    }

    for( island = 0; island < (int) m_cutouts.GetCount(); island++ )
    {
        for( i = 0; i < (int) m_cutouts[island]->GetCount(); i++ )
        {
            (*m_cutouts[island])[i]->x  += aX_offs;
            (*m_cutouts[island])[i]->y  += aY_offs;
        }
    }
}
