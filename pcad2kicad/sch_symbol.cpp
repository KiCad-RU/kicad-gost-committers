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
 * @file sch_symbol.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_symbol.h>

namespace PCAD2KICAD {

SCH_SYMBOL::SCH_SYMBOL()
{
    InitTTextValue( &m_module );
    InitTTextValue( &m_reference );
    InitTTextValue( &m_value );
    m_attachedSymbol    = wxEmptyString;
    m_attachedPattern   = wxEmptyString;
}


SCH_SYMBOL::~SCH_SYMBOL()
{
}

void SCH_SYMBOL::ParseNetlist( wxXmlNode* aNode, wxString aReference )
{
    wxString    propValue;
    wxXmlNode* lNode = aNode;

    // also symbol from library as name of component as is known in schematics library
    while( lNode->GetName() != wxT( "www.lura.sk" ) )
        lNode = lNode->GetParent();

    lNode = FindNode( lNode, wxT( "netlist" ) );

    if( lNode )
    {
        lNode = FindNode( lNode, wxT( "compInst" ) );

        while( lNode )
        {
            lNode->GetAttribute( wxT( "Name" ), &propValue );

            if( lNode->GetName() == wxT( "compInst" ) && propValue == aReference )
            {
                // Type - or Value , depends on version
                if( FindNode( lNode, wxT( "compValue" ) ) )
                {
                    FindNode( lNode,
                              wxT( "compValue" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    propValue.Trim( false );
                    propValue.Trim( true );
                    m_value.text = propValue;
                }
                //else if( FindNode( lNode, wxT( "originalName" ) ) )
                //{
                //    FindNode( lNode,
                //              wxT( "originalName" ) )->GetAttribute( wxT( "Name" ),
                //                                                     &propValue );
                //    m_typ.text = propValue;
                //}

                // Pattern
                if( FindNode( lNode, wxT( "patternName" ) ) )
                {
                    FindNode( lNode,
                              wxT( "patternName" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    m_attachedPattern = propValue;
                }
                else if( FindNode( lNode, wxT( "originalName" ) ) )
                {
                    FindNode( lNode,
                              wxT( "originalName" ) )->GetAttribute( wxT( "Name" ),
                                                                     &propValue );
                    m_attachedPattern = propValue;
                }

                // Symbol
                if( FindNode( lNode, wxT( "originalName" ) ) )
                {
                    FindNode( lNode,
                              wxT( "originalName" ) )->GetAttribute( wxT( "Name" ),
                                                                     &propValue );
                    m_attachedSymbol = propValue;
                }
                else if( FindNode( lNode, wxT( "compRef" ) ) )
                {
                    FindNode( lNode,
                              wxT( "compRef" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    m_attachedSymbol = propValue;
                }
            }

            lNode = lNode->GetNext();
        }
    }
}

void SCH_SYMBOL::ParseLibrary( wxXmlNode* aNode, wxString aModule,
                               wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    wxString   propValue;
    wxXmlNode* lNode = aNode;

    while( lNode->GetName() != wxT( "www.lura.sk" ) )
        lNode = lNode->GetParent();

    lNode = FindNode( lNode, wxT( "library" ) );

    if( lNode )
    {
        lNode = FindNode( lNode, wxT( "symbolDef" ) );

        while( lNode )
        {
            lNode->GetAttribute( wxT( "Name" ), &propValue );

            if( lNode->GetName() == wxT( "symbolDef" ) && propValue == aModule )
            {
                lNode = lNode->GetChildren();

                while( lNode )
                {
                    if( lNode->GetName() == wxT( "attr" ) )
                    {
                        // Reference
                        lNode->GetAttribute( wxT( "Name" ), &propValue );
                        propValue.Trim( false );
                        propValue.Trim( true );

                        if( propValue == wxT( "RefDes" ) )
                            SetTextParameters( lNode, &m_reference,
                                               aDefaultMeasurementUnit, aActualConversion );

                        // Type (type field is not supported in KiCad)
                        //if( propValue == wxT( "Type {Type}" ) || propValue == wxT( "Type" ) )
                        //    SetTextParameters( lNode, &m_typ,
                        //                       aDefaultMeasurementUnit, aActualConversion );

                        // Value
                        if( propValue == wxT( "Value" ) )
                            SetTextParameters( lNode, &m_value,
                                               aDefaultMeasurementUnit, aActualConversion );
                    }

                    lNode = lNode->GetNext();
                }
            }

            if( lNode )
                lNode = lNode->GetNext();
        }
    }
}

void SCH_SYMBOL::Parse( wxXmlNode* aNode,
                        wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    wxXmlNode*  lNode;
    wxString    propValue, str;
    long        num;

    m_objType = wxT( "symbol" );

    if( FindNode( aNode, wxT( "partNum" ) ) )
    {
        FindNode( aNode, wxT( "partNum" ) )->GetNodeContent().ToLong( &num );
        m_partNum = (int) num;
    }

    if( FindNode( aNode, wxT( "symbolRef" ) ) )
    {
        FindNode( aNode, wxT( "symbolRef" ) )->GetAttribute( wxT( "Name" ),
                                                             &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_module.text = propValue;
    }

    ParseLibrary( aNode, m_module.text,
                  aDefaultMeasurementUnit, aActualConversion );

    if( FindNode( aNode, wxT( "refDesRef" ) ) )
    {
        FindNode( aNode, wxT( "refDesRef" ) )->GetAttribute( wxT( "Name" ),
                                                             &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_reference.text = propValue;
    }

    ParseNetlist( aNode, m_reference.text );

    if( FindNode( aNode, wxT( "pt" ) ) )
    {
        SetPosition( FindNode( aNode, wxT( "pt" ) )->GetNodeContent(),
                     aDefaultMeasurementUnit, &m_positionX, &m_positionY, aActualConversion );
    }

    str = FindNodeGetContent( aNode, wxT( "isFlipped" ) );
    if( str == wxT( "True" ) )
        m_mirror = 1;

    if( FindNode( aNode, wxT( "rotation" ) ) )
        m_rotation = StrToInt1Units(
            FindNode( aNode, wxT( "rotation" ) )->GetNodeContent() );


    lNode = aNode->GetChildren();

    while( lNode )
    {
        if( lNode->GetName() == wxT( "attr" ) )
        {
            lNode->GetAttribute( wxT( "Name" ), &propValue );
            propValue.Trim( false );
            propValue.Trim( true );

            if( propValue == wxT( "RefDes" ) )
            {
                SetTextParameters( lNode, &m_reference, aDefaultMeasurementUnit,
                                   aActualConversion );
            }

            if( propValue == wxT( "Value" ) )
            {
                SetTextParameters( lNode, &m_value, aDefaultMeasurementUnit, aActualConversion );
            }
        }

        lNode = lNode->GetNext();
    }
}


void SCH_SYMBOL::WriteToFile( wxFile* aFile, char aFileType )
{
    char        orientation;
    wxString    visibility, str;
    int         a, b, c, d;

    CorrectTextPosition( &m_value, m_rotation );
    CorrectTextPosition( &m_reference, m_rotation );
    // Go out
    str = m_attachedSymbol;
    str.Replace( wxT( " " ), wxT( "~" ), true );
    aFile->Write( wxT( "L " ) + str + ' ' + m_reference.text + wxT( "\n" ) );
    aFile->Write( wxString::Format( "U %d 1 00000000\n", m_partNum ) );
    aFile->Write( wxString::Format( "P %d %d\n", m_positionX, m_positionY ) );

    // Reference
    if( m_reference.textRotation == 900 )
        orientation = 'V';
    else
        orientation = 'H';

    if( m_reference.textIsVisible == 1 )
        visibility = wxT( "0000" );
    else
        visibility = wxT( "0001" );

    aFile->Write( wxT( "F 0 \"" ) + m_reference.text + wxT( "\" " ) + orientation + ' ' +
                  wxString::Format( "%d %d %d",
                                    m_reference.correctedPositionX + m_positionX,
                                    m_reference.correctedPositionY + m_positionY,
                                    KiROUND( (double) m_reference.textHeight / 2.0 ) ) +
                  ' ' + visibility + wxT( " C C\n" ) );

    // Value
    if( m_value.textIsVisible == 1 )
        visibility = wxT( "0000" );
    else
        visibility = wxT( "0001" );

    if( m_value.textRotation == 900 || m_value.textRotation == 2700 )
        orientation = 'V';
    else
        orientation = 'H';

    aFile->Write( wxT( "F 1 \"" ) + m_value.text + wxT( "\" " ) + orientation + ' ' +
                  wxString::Format( "%d %d %d",
                                    m_value.correctedPositionX + m_positionX,
                                    m_value.correctedPositionY + m_positionY,
                                    KiROUND( (double) m_value.textHeight / 2.0 ) ) +
                  ' ' + visibility + wxT( " C C\n" ) );

// SOME ROTATION MATRICS ?????????????
// 1    2900 5200
/*  270 :
 *  No Mirror       0    -1   -1    0
 *  MirrorX      0    -1    1    0
 *  MirrorY      0    -1    1    0
 *
 *  180  :
 *  No Mirror      -1   0    0     1
 *  MirrorX     -1   0    0    -1
 *  MirrorY      1   0    0     1
 *
 *  R90  :
 *  No Mirror       0    1    1    0
 *  MirrorX      0    1   -1    0
 *  MirrorY      0    1    -1   0
 *
 *  0    :
 *  No Mirror       1   0    0   -1
 *  MirrorX      1   0    0    1
 *  MirrorY     -1   0    0   -1
 */
    aFile->Write( wxString::Format( " %d %d %d\n", m_partNum, m_positionX, m_positionY ) );
    // Miror is negative in compare with PCad represenation...
    a = 0; b = 0; c = 0; d = 0;

    if( m_mirror == 0 )
    {
        if( m_rotation == 0 )
        {
            a = 1; d = -1;
        }

        if( m_rotation == 900 )
        {
            b = -1; c = -1;
        }

        if( m_rotation == 1800 )
        {
            a = -1; d = 1;
        }

        if( m_rotation == 2700 )
        {
            b = 1; c = 1;
        }
    }

    if( m_mirror == 1 )
    {
        if( m_rotation == 0 )
        {
            a = -1; d = -1;
        }

        if( m_rotation == 900 )
        {
            b = 1; c = -1;
        }

        if( m_rotation == 1800 )
        {
            a = 1; d = 1;
        }

        if( m_rotation == 2700 )
        {
            b = -1; c = 1;
        }
    }

    aFile->Write( wxString::Format( " %d %d %d %d \n", a, b, c, d ) );
    // FOOTPRINT
}

} // namespace PCAD2KICAD
