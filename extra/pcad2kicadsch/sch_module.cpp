/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012-2013 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2012-2016 KiCad Developers, see CHANGELOG.TXT for contributors.
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
 * @file sch_module.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <sch_arc.h>
#include <sch_common.h>
#include <sch_ieeesymbol.h>
#include <sch_line.h>
#include <sch_module.h>
#include <sch_pin.h>
#include <sch_text.h>

#include <trigo.h>

namespace PCAD2KICAD {

SCH_MODULE::SCH_MODULE()
{
    m_objType   = wxT( "module" );
    InitTTextValue( &m_name );
    InitTTextValue( &m_reference );
    m_numParts = 0;
    m_attachedPattern   = wxEmptyString;
    m_moduleDescription = wxEmptyString;
    m_alias = wxEmptyString;
    m_component = wxEmptyString;
    m_libName = wxEmptyString;
    m_isPower = false;
}


SCH_MODULE::~SCH_MODULE()
{
    int i;

    for( i = 0; i < (int) m_moduleObjects.GetCount(); i++ )
        delete m_moduleObjects[i];
}


void SCH_MODULE::Parse( XNODE*   aNode, wxStatusBar* aStatusBar,
                        wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    wxString    propValue, str;
    XNODE*      lNode, * tNode;
    SCH_PIN*    pin;
    long        num;
    int         i;

    aNode->GetAttribute( wxT( "Name" ), &propValue );
    m_component = propValue;

    FindNode( aNode, wxT( "originalName" ) )->GetAttribute( wxT( "Name" ), &propValue );
    propValue.Trim( false );
    m_name.text = propValue;
    aStatusBar->SetStatusText( _( "Creating Component : " ) + m_name.text );

    lNode = FindNode( aNode, wxT( "compHeader" ) );

    if( lNode )
    {
        if( FindNode( lNode, wxT( "refDesPrefix" ) ) )
            FindNode( lNode,
                      wxT( "refDesPrefix" ) )->GetAttribute( wxT( "Name" ), &m_reference.text );


        if( FindNode( lNode, wxT( "numParts" ) ) )
        {
            FindNode( lNode, wxT( "numParts" ) )->GetNodeContent().ToLong( &num );
            m_numParts = (int) num;
        }

        if( FindNode( lNode, wxT( "compType" ) ) )
        {
            propValue = FindNode( lNode, wxT( "compType" ) )->GetNodeContent();
            propValue.Trim( false );
            if( propValue == wxT( "Power" ) )
                m_isPower = true;
        }
    }

    tNode = aNode->GetChildren();

    while( tNode )
    {
        if( tNode->GetName() == wxT( "compPin" ) )
        {
            pin = new SCH_PIN;
            pin->Parse( tNode );
            pin->m_isPower = m_isPower;
            m_moduleObjects.Add( pin );
        }

        if( tNode->GetName() == wxT( "attachedSymbol" ) )
        {
            str = FindNodeGetContent( tNode, wxT( "altType" ) );

            if( str == wxT( "Normal" ) )
            {
                if( FindNode( tNode, wxT( "partNum" ) ) )
                    FindNode( tNode, wxT( "partNum" ) )->GetNodeContent().ToLong( &num );

                if( FindNode( tNode, wxT( "symbolName" ) ) )
                {
                    FindNode( tNode,
                              wxT( "symbolName" ) )->GetAttribute( wxT( "Name" ), &str );

                    // if necessary grow m_attachedSymbols array
                    if( num + 1 > (int)m_attachedSymbols.GetCount() )
                        m_attachedSymbols.Add( wxEmptyString,
                                               num + 1 - m_attachedSymbols.GetCount() );

                    m_attachedSymbols[(int) num] = str;
                }
            }
        }

        if( tNode->GetName() == wxT( "attachedPattern" ) )
        {
            if( FindNode( tNode, wxT( "patternName" ) ) )
                FindNode( tNode,
                          wxT( "patternName" ) )->GetAttribute( wxT( "Name" ),
                                                                &m_attachedPattern );
        }

        if( tNode->GetName() == wxT( "attr" ) )
        {
            tNode->GetAttribute( wxT( "Name" ), &propValue );

            if( propValue.Len() > 13 && propValue.Left( 12 ) == wxT( "Description " ) )
                m_moduleDescription = propValue.Left( 13 );
        }

        tNode = tNode->GetNext();
    }

    for( i = 0; i < m_numParts; i++ )
        FindAndProcessSymbolDef( aNode, i + 1, aDefaultMeasurementUnit, aActualConversion );
}


void SCH_MODULE::SetPinProperties( XNODE*   aNode, int aSymbolIndex,
                                   wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    SCH_PIN*    schPin;
    int         i;

    for( i = 0; i < (int) m_moduleObjects.GetCount(); i++ )
    {
        if( (m_moduleObjects[i])->m_objType == wxT( "pin" ) )
        {
            schPin = (SCH_PIN*) m_moduleObjects[i];
            schPin->ParsePinProperties( aNode, aSymbolIndex,
                                        aDefaultMeasurementUnit, aActualConversion );
        }
    }
}


void SCH_MODULE::FindAndProcessSymbolDef( XNODE*        aNode,
                                          int           aSymbolIndex,
                                          wxString      aDefaultMeasurementUnit,
                                          wxString      aActualConversion )
{
    XNODE*      tNode, * ttNode;
    wxString    propValue, propValue2;
    SCH_LINE*   line;
    SCH_ARC*    arc;
    SCH_TEXT*   text;
    SCH_IEEESYMBOL* ieeesym;

    tNode = aNode;

    while( tNode->GetName() != wxT( "www.lura.sk" ) )
        tNode = tNode->GetParent();

    tNode = FindNode( tNode, wxT( "library" ) );

    if( tNode )
    {
        tNode = FindNode( tNode, wxT( "symbolDef" ) );

        while( tNode )
        {
            tNode->GetAttribute( wxT( "Name" ), &propValue );

            if( FindNode( tNode, wxT( "originalName" ) ) )
                FindNode( tNode,
                          wxT( "originalName" ) )->GetAttribute( wxT( "Name" ), &propValue2 );



            if( tNode->GetName() == wxT( "symbolDef" )
                && ( propValue == m_attachedSymbols[aSymbolIndex]
                     || (FindNode( tNode, wxT( "originalName" ) )
                         && propValue2 == m_attachedSymbols[aSymbolIndex]) ) )
            {
                ttNode  = tNode;
                tNode   = FindNode( ttNode, wxT( "pin" ) );

                while( tNode )
                {
                    if( tNode->GetName() == wxT( "pin" )
                        && FindNode( tNode, wxT( "pinNum" ) ) )
                    {
                        SetPinProperties( tNode, aSymbolIndex,
                                          aDefaultMeasurementUnit, aActualConversion );
                    }

                    tNode = tNode->GetNext();
                }

                tNode = FindNode( ttNode, wxT( "line" ) );

                while( tNode )
                {
                    if( tNode->GetName() == wxT( "line" ) )
                    {
                        line = new SCH_LINE;
                        line->Parse( tNode, aSymbolIndex,
                                     aDefaultMeasurementUnit, aActualConversion );
                        m_moduleObjects.Add( line );
                    }

                    tNode = tNode->GetNext();
                }

                tNode = FindNode( ttNode, wxT( "arc" ) );

                while( tNode )
                {
                    if( tNode->GetName() == wxT( "arc" ) )
                    {
                        arc = new SCH_ARC;
                        arc->Parse( tNode, aSymbolIndex,
                                    aDefaultMeasurementUnit, aActualConversion );
                        m_moduleObjects.Add( arc );
                    }

                    tNode = tNode->GetNext();
                }

                tNode = FindNode( ttNode, wxT( "triplePointArc" ) );

                while( tNode )
                {
                    if( tNode->GetName() == wxT( "triplePointArc" ) )
                    {
                        arc = new SCH_ARC;
                        arc->Parse( tNode, aSymbolIndex,
                                    aDefaultMeasurementUnit, aActualConversion );
                        m_moduleObjects.Add( arc );
                    }

                    tNode = tNode->GetNext();
                }

                tNode = ttNode->GetChildren();

                while( tNode )
                {
                    if( tNode->GetName() == wxT( "attr" ) )
                    {
                        // Reference
                        tNode->GetAttribute( wxT( "Name" ), &propValue );
                        propValue.Trim( false );
                        propValue.Trim( true );

                        if( propValue == wxT( "RefDes" ) )
                            SetTextParameters( tNode, &m_reference,
                                               aDefaultMeasurementUnit, aActualConversion );

                        // Type
                        if( propValue == wxT( "Type {Type}" ) || propValue == wxT( "Type" ) )
                            SetTextParameters( tNode, &m_name,
                                               aDefaultMeasurementUnit, aActualConversion );
                    }

                    tNode = tNode->GetNext();
                }

                tNode = FindNode( ttNode, wxT( "text" ) );

                while( tNode )
                {
                    if( tNode->GetName() == wxT( "text" ) )
                    {
                        text = new SCH_TEXT( aSymbolIndex );
                        text->Parse( tNode, aDefaultMeasurementUnit, aActualConversion );
                        m_moduleObjects.Add( text );
                    }

                    tNode = tNode->GetNext();
                }

                tNode = FindNode( ttNode, wxT( "ieeeSymbol" ) );

                while( tNode )
                {
                    if( tNode->GetName() == wxT( "ieeeSymbol" ) )
                    {
                        ieeesym = new SCH_IEEESYMBOL();
                        ieeesym->Parse( tNode, aSymbolIndex,
                                        aDefaultMeasurementUnit, aActualConversion );
                        m_moduleObjects.Add( ieeesym );
                    }

                    tNode = tNode->GetNext();
                }
            }

            if( tNode )
                tNode = tNode->GetNext();
        }
    }
}


void SCH_MODULE::WriteToFile( wxFile* aFile, char aFileType )
{
    int i, symbolIndex;
    wxString orientation, visibility;

    CorrectLibText( &m_name );
    CorrectLibText( &m_reference );

    ReplaceTextQuotes( m_name.text );
    ReplaceTextSlashes( m_name.text );
    ReplaceTextQuotes( m_attachedPattern );
    ReplaceTextQuotes( m_component );
    ReplaceTextSlashes( m_component );

    // Go out
    aFile->Write( wxT( "\n" ) );
    aFile->Write( wxT( "#\n" ) );
    aFile->Write( wxT( "# " ) + ValidateName( m_name.text ) + wxT( "\n" ) );
    aFile->Write( wxT( "#\n" ) );
    if( m_isPower )
        aFile->Write( wxT( "DEF " ) + ValidateName( m_name.text ) + wxT( " #PWR 0 40 Y Y " ) +
                      wxString::Format( wxT( "%d F P\n" ), m_numParts ) );
    else
        aFile->Write( wxT( "DEF " ) + ValidateName( m_name.text ) + wxT( " U 0 40 Y Y " ) +
                      wxString::Format( wxT( "%d F N\n" ), m_numParts ) );

    EscapeTextQuotes( m_reference.text );

    // REFERENCE
    if( m_isPower )
    {
        m_reference.text = wxT( "#PWR" );
        m_reference.textIsVisible = false;
    }

    orientation = ( m_reference.textRotation == 900 ) ? wxT( 'V' ) : wxT( 'H' );
    visibility = m_reference.textIsVisible ? wxT( 'V' ) : wxT( 'I' );

    aFile->Write( wxT( "F0 \"" ) + m_reference.text + wxT( "\" " ) +
                  wxString::Format( wxT( "%d %d %d" ),
                                    m_reference.textPositionX, m_reference.textPositionY,
                                    GetCorrectedHeight( m_reference.textHeight,
                                                        m_reference.isTrueType ) ) + " " +
                  orientation + " " + visibility + " " + GetJustifyString( &m_reference ) + "\n" );
    // NAME
    orientation = ( m_name.textRotation == 900 ) ? wxT( 'V' ) : wxT( 'H' );
    visibility = m_name.textIsVisible ? wxT( 'V' ) : wxT( 'I' );

    aFile->Write( wxT( "F1 \"" ) + ValidateName( m_name.text ) + wxT( "\" " ) +
                  wxString::Format( wxT( "%d %d %d" ),
                                    m_name.textPositionX, m_name.textPositionY,
                                    GetCorrectedHeight( m_name.textHeight, m_name.isTrueType ) ) +
                  " " + orientation + " " + visibility + " " + GetJustifyString( &m_name ) +
                  "\n" );
    // FOOTPRINT
    aFile->Write( wxT( "F2 \"" ) + m_libName + wxT( ':' ) + m_component +
                  wxT( "\" 0 0 50 H I C C\n" ) );

    // Footprints filter
    if( m_attachedPattern.Len() > 0 )
    {
        // $FPLIST  //SCHModule.AttachedPattern
        // 14DIP300*
        // SO14*
        // $ENDFPLIST
        aFile->Write( wxT( "$FPLIST\n" ) );
        aFile->Write( wxT( ' ' ) + m_attachedPattern + wxT( "*\n" ) );
        aFile->Write( wxT( "$ENDFPLIST\n" ) );
    }

    // Alias
    if( m_alias.Len() > 0 )
    {
        // ALIAS 74LS37 7400 74HCT00 74HC00
        aFile->Write( wxT( "ALIAS" ) + m_alias + wxT( "\n" ) );
    }

    aFile->Write( wxT( "DRAW\n" ) );

    for( symbolIndex = 1; symbolIndex <= m_numParts; symbolIndex++ )
    {
        // LINES=POLYGONS
        for( i = 0; i < (int) m_moduleObjects.GetCount(); i++ )
        {
            if( m_moduleObjects[i]->m_objType == wxT( "line" ) )
                if( ( (SCH_LINE*) m_moduleObjects[i] )->m_partNum == symbolIndex )
                    m_moduleObjects[i]->WriteToFile( aFile, aFileType );
        }

        // ARCS
        for( i = 0; i < (int) m_moduleObjects.GetCount(); i++ )
        {
            if( m_moduleObjects[i]->m_objType == wxT( "arc" ) )
                if( ( (SCH_ARC*) m_moduleObjects[i] )->m_partNum == symbolIndex )
                    m_moduleObjects[i]->WriteToFile( aFile, aFileType );
        }

        // TEXTS
        for( i = 0; i < (int) m_moduleObjects.GetCount(); i++ )
        {
            if( m_moduleObjects[i]->m_objType == wxT( "text" ) )
                if( ( (SCH_TEXT*) m_moduleObjects[i] )->m_partNum == symbolIndex )
                    m_moduleObjects[i]->WriteToFile( aFile, aFileType );
        }

        // IEEE symbols
        for( i = 0; i < (int) m_moduleObjects.GetCount(); i++ )
        {
            if( m_moduleObjects[i]->m_objType == wxT( "ieeeSymbol" ) )
                if( ( (SCH_IEEESYMBOL*) m_moduleObjects[i] )->m_partNum == symbolIndex )
                    m_moduleObjects[i]->WriteToFile( aFile, aFileType );
        }

        // PINS
        for( i = 0; i < (int) m_moduleObjects.GetCount(); i++ )
        {
            if( m_moduleObjects[i]->m_objType == wxT( "pin" ) )
                if( ( (SCH_PIN*) m_moduleObjects[i] )->m_partNum == symbolIndex )
                    m_moduleObjects[i]->WriteToFile( aFile, aFileType );
        }
    }

    aFile->Write( wxT( "ENDDRAW\n" ) );     // ??
    aFile->Write( wxT( "ENDDEF\n" ) );      // ??
}


void SCH_MODULE::SetLibName( wxString aName )
{
    m_libName = aName;
}

} // namespace PCAD2KICAD
