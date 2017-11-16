/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012-2015 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file sch_line.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_line.h>

namespace PCAD2KICAD {

const int LEAD_OFFSET = 100;

SCH_LINE::SCH_LINE( wxString aLineType )
{
    m_objType        = wxT( "line" );
    m_lineType       = aLineType;
    m_toX            = 0;
    m_toY            = 0;
    m_net            = wxEmptyString;
    m_start_endStyle = wxT( "Rounded" );
    m_end_endStyle   = wxT( "Rounded" );
    InitTTextValue( &m_labelText );
}


void SCH_LINE::Parse( XNODE*   aNode, int aSymbolIndex,
                      wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    XNODE*      lNode;
    wxString    propValue, str;

    m_partNum   = aSymbolIndex;

    if( FindNode( aNode, wxT( "width" ) ) )
        m_width = StrToIntUnits( FindNode( aNode, wxT( "width" ) )->GetNodeContent(),
                                 wxT( ' ' ), aActualConversion );
    else
        m_width = 1; // default

    lNode = FindNode( aNode, wxT( "pt" ) );

    if( lNode )
    {
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_positionX, &m_positionY, aActualConversion );

        lNode = lNode->GetNext();

        if( lNode && lNode->GetName() == wxT( "endStyle" ) )
        {
            m_start_endStyle = lNode->GetNodeContent();
            m_start_endStyle.Trim( false );
            m_start_endStyle.Trim( true );
            lNode = lNode->GetNext();
        }

        while( lNode && lNode->GetName() != wxT( "pt" ) )
            lNode = lNode->GetNext();

        if( lNode )
        {
            SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                         &m_toX, &m_toY, aActualConversion );

            lNode = lNode->GetNext();

            if( lNode && lNode->GetName() == wxT( "endStyle" ) )
            {
                m_end_endStyle = lNode->GetNodeContent();
                m_end_endStyle.Trim( false );
                m_end_endStyle.Trim( true );
                lNode = lNode->GetNext();
            }
        }
    }

    if( FindNode( aNode, wxT( "netNameRef" ) ) )
    {
        FindNode( aNode,
                  wxT( "netNameRef" ) )->GetAttribute( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_net = propValue;
    }

    m_labelText.textIsVisible = 0;  // LABELS
    m_labelText.text = m_net;       // can be better ?

    lNode = aNode->GetParent();

    if( lNode->GetName() == wxT( "wire" ) )
    {
        lNode = FindNode( lNode, wxT( "dispName" ) );

        if( lNode )
        {
            str = lNode->GetNodeContent();
            str.Trim( false );
            str.Trim( true );

            if( str == wxT( "True" ) )
                m_labelText.textIsVisible = 1;

            lNode   = lNode->GetParent();
            lNode   = FindNode( lNode, wxT( "text" ) );

            if( lNode )
            {
                if( FindNode( lNode, wxT( "pt" ) ) )
                    SetPosition( FindNode( lNode, wxT( "pt" ) )->GetNodeContent(),
                                 aDefaultMeasurementUnit, &m_labelText.textPositionX,
                                 &m_labelText.textPositionY, aActualConversion );

                if( FindNode( lNode, wxT( "rotation" ) ) )
                    m_labelText.textRotation = StrToInt1Units(
                        FindNode( lNode, wxT( "rotation" ) )->GetNodeContent() );
            }
        }
    }
}


SCH_LINE::~SCH_LINE()
{
}

void SCH_LINE::SetPosOffset( int aX_offs, int aY_offs )
{
    SCH_COMPONENT::SetPosOffset( aX_offs, aY_offs );

    m_toX   += aX_offs;
    m_toY   += aY_offs;
    m_labelText.textPositionX += aX_offs;
    m_labelText.textPositionY += aY_offs;
}

void SCH_LINE::WriteToFile( wxFile* aFile, char aFileType )
{
    wxString lt;

    if( aFileType == wxT( 'L' ) )
        aFile->Write( wxString::Format( wxT( "P 2 %d 0 %d %d %d %d %d N\n" ),
                                        m_partNum,
                                        m_width,
                                        m_positionX,
                                        m_positionY,
                                        m_toX,
                                        m_toY ) );

    if( aFileType == wxT( 'S' ) )
    {
        if( m_lineType == wxT( 'W' ) )
        {
            lt = wxT( "Wire" );

            //TODO refactor
            bool is_horiz_wire = ( m_positionY == m_toY );
            bool is_norm_pt_order = ( is_horiz_wire && ( m_positionX < m_toX ) ) ||
                                    ( !is_horiz_wire && ( m_positionY < m_toY ) );
            int rx_offset;
            int ry_offset;
            int lx_offset;
            int ly_offset;

            if( m_start_endStyle != wxT( "Rounded" ) )
            {
                if( is_horiz_wire )
                {
                    if( is_norm_pt_order )
                        m_positionX += LEAD_OFFSET;
                    else
                        m_positionX -= LEAD_OFFSET;
                }
                else
                {
                    if( is_norm_pt_order )
                        m_positionY += LEAD_OFFSET;
                    else
                        m_positionY -= LEAD_OFFSET;
                }

                // normal point order horizontal wire
                rx_offset = -LEAD_OFFSET;
                ry_offset = -LEAD_OFFSET;
                lx_offset = -LEAD_OFFSET;
                ly_offset = LEAD_OFFSET;

                if( !is_horiz_wire )
                {
                    rx_offset = -rx_offset;
                    ly_offset = -ly_offset;
                }

                if( !is_norm_pt_order )
                {
                    rx_offset = -rx_offset;
                    ry_offset = -ry_offset;
                    lx_offset = -lx_offset;
                    ly_offset = -ly_offset;
                }

                if( m_start_endStyle == wxT( "RightLead" ) ||
                    m_start_endStyle == wxT( "TwoLeads" ) )
                {
                    aFile->Write( wxT( "Entry " ) + lt + wxT( " Line\n" ) );
                    aFile->Write( wxString::Format( wxT( "               %d %d %d %d\n" ),
                                                    m_positionX, m_positionY,
                                                    m_positionX + rx_offset,
                                                    m_positionY + ry_offset ) );
                }

                if( m_start_endStyle == wxT( "LeftLead" ) ||
                    m_start_endStyle == wxT( "TwoLeads" ) )
                {
                    aFile->Write( wxT( "Entry " ) + lt + wxT( " Line\n" ) );
                    aFile->Write( wxString::Format( wxT( "               %d %d %d %d\n" ),
                                                    m_positionX, m_positionY,
                                                    m_positionX + lx_offset,
                                                    m_positionY + ly_offset ) );
                }
            }

            if( m_end_endStyle != wxT( "Rounded" ) )
            {
                if( is_horiz_wire )
                {
                    if( is_norm_pt_order )
                        m_toX -= LEAD_OFFSET;
                    else
                        m_toX += LEAD_OFFSET;
                }
                else
                {
                    if( is_norm_pt_order )
                        m_toY -= LEAD_OFFSET;
                    else
                        m_toY += LEAD_OFFSET;
                }

                // normal point order horizontal wire
                rx_offset = LEAD_OFFSET;
                ry_offset = LEAD_OFFSET;
                lx_offset = LEAD_OFFSET;
                ly_offset = -LEAD_OFFSET;

                if( !is_horiz_wire )
                {
                    rx_offset = -rx_offset;
                    ly_offset = -ly_offset;
                }

                if( !is_norm_pt_order )
                {
                    rx_offset = -rx_offset;
                    ry_offset = -ry_offset;
                    lx_offset = -lx_offset;
                    ly_offset = -ly_offset;
                }

                if( m_end_endStyle == wxT( "RightLead" ) ||
                    m_end_endStyle == wxT( "TwoLeads" ) )
                {
                    aFile->Write( wxT( "Entry " ) + lt + wxT( " Line\n" ) );
                    aFile->Write( wxString::Format( wxT( "               %d %d %d %d\n" ),
                                                    m_toX, m_toY,
                                                    m_toX + rx_offset,
                                                    m_toY + ry_offset ) );
                }

                if( m_end_endStyle == wxT( "LeftLead" ) ||
                    m_end_endStyle == wxT( "TwoLeads" ) )
                {
                    aFile->Write( wxT( "Entry " ) + lt + wxT( " Line\n" ) );
                    aFile->Write( wxString::Format( wxT( "               %d %d %d %d\n" ),
                                                    m_toX, m_toY,
                                                    m_toX + lx_offset,
                                                    m_toY + ly_offset ) );
                }
            }
        }

        if( m_lineType == wxT( 'B' ) )
            lt = wxT( "Bus" );

        if( m_lineType == wxT( 'N' ) )
            lt = wxT( "Notes" );

        aFile->Write( wxT( "Wire " ) + lt + wxT( " Line\n" ) );
        aFile->Write( wxString::Format( wxT( "               %d %d %d %d\n" ),
                                        m_positionX, m_positionY, m_toX, m_toY ) );
    }
}


void SCH_LINE::WriteLabelToFile( wxFile* aFile, char aFileType )
{
    wxString lr;

    if( m_labelText.textIsVisible == 1 )
    {
        if( m_labelText.textRotation == 0 )
            lr = wxT( '0' );
        else
            lr = wxT( '1' );

        aFile->Write( wxString::Format( wxT( "Text Label %d %d" ),
                                        m_labelText.textPositionX, m_labelText.textPositionY ) +
                      wxT( ' ' ) + lr + wxT( ' ' ) + wxT( " 60 ~\n" ) );
        aFile->Write( m_labelText.text + wxT( "\n" ) );
    }
}

} // namespace PCAD2KICAD
