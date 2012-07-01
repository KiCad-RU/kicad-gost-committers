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
 * @file sch_junction.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_junction.h>

SCH_JUNCTION::SCH_JUNCTION( int aX, int aY, wxString aNet ) : m_net( aNet )
{
    m_positionX = aX;
    m_positionY = aY;
    m_objType   = 'J';
}


SCH_JUNCTION::~SCH_JUNCTION()
{
}


void SCH_JUNCTION::Parse( wxXmlNode*    aNode,
                          wxString      aDefaultMeasurementUnit,
                          wxString      aActualConversion )
{
    wxString propValue;

    m_net = wxEmptyString;

    if( FindNode( aNode->GetChildren(), wxT( "pt" ) ) )
    {
        SetPosition( FindNode( aNode->GetChildren(), wxT( "pt" ) )->GetNodeContent(),
                     aDefaultMeasurementUnit, &m_positionX, &m_positionY, aActualConversion );
    }

    if( FindNode( aNode->GetChildren(), wxT( "netNameRef" ) ) )
    {
        FindNode( aNode->GetChildren(),
                  wxT( "netNameRef" ) )->GetAttribute( wxT( "Name" ), &propValue );

        propValue.Trim( true );
        propValue.Trim( false );
        m_net = propValue;
    }
}


void SCH_JUNCTION::WriteToFile( wxFile* aFile, char aFileType )
{
    aFile->Write( wxString::Format( "Connection ~ %d %d\n", m_positionX, m_positionY ) );
}
