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
 * @file sch_symbol.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/tokenzr.h>

#include <common.h>

#include <sch_common.h>
#include <sch_symbol.h>

#include <trigo.h>

namespace PCAD2KICAD {

SCH_SYMBOL::SCH_SYMBOL()
{
    m_objType = wxT( "symbol" );
    InitTTextValue( &m_module );
    InitTTextValue( &m_reference );
    InitTTextValue( &m_value );
    InitTTextValue( &m_type );
    m_attachedSymbol  = wxEmptyString;
    m_attachedPattern = wxEmptyString;
    m_libName = wxEmptyString;
    m_isPower = false;
}


SCH_SYMBOL::~SCH_SYMBOL()
{
    int i;

    for( i = 0; i < (int)m_attributes.GetCount(); i++ )
    {
        delete m_attributes[i];
        m_attributes[i] = NULL;
    }
}


void SCH_SYMBOL::FindAttributes( XNODE* aNode )
{
    TATTR* attr;
    wxString  propValue;

    aNode = aNode->GetChildren();

    while( aNode )
    {
        if( aNode->GetName() == wxT( "attr" ) )
        {
            aNode->GetAttribute( wxT( "Name" ), &propValue );
            propValue.Trim( false );
            propValue.Trim( true );

            attr = new TATTR;
            attr->attrName = wxEmptyString;
            attr->attrValue = wxEmptyString;

            wxStringTokenizer tkz( propValue, wxT(" ") );

            if( tkz.HasMoreTokens() )
                attr->attrName = tkz.GetNextToken();

            if( attr->attrName.Len() + 1 < propValue.Len() )
                attr->attrValue = propValue.Mid( attr->attrName.Len() + 1 );

            if( attr->attrName != wxEmptyString )
                m_attributes.Add( attr );
        }

        aNode = aNode->GetNext();
    }
}


void SCH_SYMBOL::ParseNetlist( XNODE* aNode, wxString aReference )
{
    wxString  propValue;
    XNODE*    lNode = aNode;

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

                if( FindNode( lNode, wxT( "originalName" ) ) )
                {
                    FindNode( lNode,
                              wxT( "originalName" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    m_type.text = propValue;
                }

                FindAttributes( lNode );

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
                              wxT( "originalName" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    m_attachedPattern = propValue;
                }

                // Symbol
                if( FindNode( lNode, wxT( "originalName" ) ) )
                {
                    FindNode( lNode,
                              wxT( "originalName" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    m_attachedSymbol = propValue;
                }
                else if( FindNode( lNode, wxT( "compRef" ) ) )
                {
                    FindNode( lNode, wxT( "compRef" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    m_attachedSymbol = propValue;
                }

                // Component
                if( FindNode( lNode, wxT( "compRef" ) ) )
                {
                    FindNode( lNode, wxT( "compRef" ) )->GetAttribute( wxT( "Name" ), &propValue );
                    m_component = propValue;
                }
            }

            lNode = lNode->GetNext();
        }
    }
}


void SCH_SYMBOL::ParseLibrary( XNODE*   aNode, wxString aModule, wxString aComponent,
                               wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    wxString   propValue;
    XNODE* lNode = aNode;

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
                        if( propValue == wxT( "Type {Type}" ) || propValue == wxT( "Type" ) )
                            SetTextParameters( lNode, &m_type,
                                               aDefaultMeasurementUnit, aActualConversion );

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

    lNode = aNode;

    while( lNode->GetName() != wxT( "www.lura.sk" ) )
        lNode = lNode->GetParent();

    lNode = FindNode( lNode, wxT( "library" ) );

    if( lNode )
    {
        lNode = FindNode( lNode, wxT( "compDef" ) );

        while( lNode )
        {
            lNode->GetAttribute( wxT( "Name" ), &propValue );

            if( propValue == aComponent )
            {
                lNode = FindNode( lNode, wxT( "compHeader" ) );

                if( lNode )
                {
                    if( FindNode( lNode, wxT( "compType" ) ) )
                    {
                        propValue = FindNode( lNode, wxT( "compType" ) )->GetNodeContent();
                        propValue.Trim( false );
                        if( propValue == wxT( "Power" ) )
                            m_isPower = true;
                    }
                }
                break;
            }

            lNode = lNode->GetNext();
        }
    }
}


void SCH_SYMBOL::Parse( XNODE*   aNode,
                        wxString aDefaultMeasurementUnit, wxString aActualConversion )
{
    XNODE*      lNode;
    wxString    propValue, str;
    long        num;

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

    if( FindNode( aNode, wxT( "refDesRef" ) ) )
    {
        FindNode( aNode, wxT( "refDesRef" ) )->GetAttribute( wxT( "Name" ),
                                                             &propValue );
        propValue.Trim( false );
        propValue.Trim( true );
        m_reference.text = propValue;
    }

    ParseNetlist( aNode, m_reference.text );
    ParseLibrary( aNode, m_module.text, m_component, aDefaultMeasurementUnit, aActualConversion );

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
                m_reference.isLibValues = false;
            }
            else if( propValue == wxT( "Value" ) )
            {
                SetTextParameters( lNode, &m_value, aDefaultMeasurementUnit, aActualConversion );
                m_value.isLibValues = false;
            }
            else if( propValue == wxT( "Type" ) )
            {
                SetTextParameters( lNode, &m_type, aDefaultMeasurementUnit, aActualConversion );
                m_type.isLibValues = false;
            }
        }

        lNode = lNode->GetNext();
    }
}


void SCH_SYMBOL::WriteToFile( wxFile* aFile, char aFileType )
{
    wxString orientation, visibility;
    wxString boldStr;
    wxString italicStr;
    int      a, b, c, d, i;

    CorrectField( &m_reference );
    CorrectField( &m_value );
    CorrectField( &m_type );

    ReplaceTextQuotes( m_module.text );
    ReplaceTextQuotes( m_attachedSymbol );
    ReplaceTextSlashes( m_attachedSymbol );
    ReplaceTextQuotes( m_component );
    ReplaceTextSlashes( m_component );

    EscapeTextQuotes( m_reference.text );
    EscapeTextQuotes( m_value.text );
    EscapeTextQuotes( m_type.text );

    if( m_value.text == wxEmptyString )
        m_value.text = wxT( '~' );

    m_reference.text = ValidateReference( m_reference.text );

    if( m_isPower )
        m_reference.text.Prepend( wxT( '#' ) );

    // Go out
    aFile->Write( wxT( "L " ) + m_libName + wxT( ':' ) + ValidateName( m_attachedSymbol ) +
                  wxT( ' ' ) + m_reference.text + wxT( "\n" ) );
    aFile->Write( wxString::Format( wxT( "U %d 1 00000000\n" ), m_partNum ) );
    aFile->Write( wxString::Format( wxT( "P %d %d\n" ), m_positionX, m_positionY ) );

    // Reference
    boldStr = m_reference.isBold ? wxT( "B" ) : wxT( "N" );
    italicStr = m_reference.isItalic ? wxT( "I" ) : wxT( "N" );

    if( m_reference.textRotation == 900 )
        orientation = wxT( 'V' );
    else
        orientation = wxT( 'H' );

    if( m_reference.textIsVisible == 1 )
        visibility = wxT( "0000" );
    else
        visibility = wxT( "0001" );

    aFile->Write( wxT( "F 0 \"" ) + m_reference.text + wxT( "\" " ) + orientation + wxT( ' ' ) +
                  wxString::Format( wxT( "%d %d %d" ),
                                    m_reference.textPositionX + m_positionX,
                                    m_reference.textPositionY + m_positionY,
                                    GetCorrectedHeight( m_reference.textHeight,
                                                        m_reference.isTrueType ) ) +
                  wxT( ' ' ) + visibility + wxT( ' ' ) + GetJustifyString( &m_reference ) +
                  italicStr + boldStr + wxT( "\n" ) );

    // Value
    boldStr = m_value.isBold ? wxT( "B" ) : wxT( "N" );
    italicStr = m_value.isItalic ? wxT( "I" ) : wxT( "N" );

    if( m_value.textIsVisible == 1 )
        visibility = wxT( "0000" );
    else
        visibility = wxT( "0001" );

    if( m_value.textRotation == 900 )
        orientation = wxT( 'V' );
    else
        orientation = wxT( 'H' );

    aFile->Write( wxT( "F 1 \"" ) + m_value.text + wxT( "\" " ) + orientation + wxT( ' ' ) +
                  wxString::Format( wxT( "%d %d %d" ),
                                    m_value.textPositionX + m_positionX,
                                    m_value.textPositionY + m_positionY,
                                    GetCorrectedHeight( m_value.textHeight,
                                                        m_value.isTrueType ) ) +
                  wxT( ' ' ) + visibility + wxT( ' ' ) + GetJustifyString( &m_value ) +
                  italicStr + boldStr + wxT( "\n" ) );

    // Footprint
    aFile->Write( wxT( "F 2 \"" ) + m_libName + wxT( ':' ) + m_component +
                  wxT( "\" H 0 0 60 0001 C CNN\n" ) );

    // Type as a common KiCAD field
    boldStr = m_type.isBold ? wxT( "B" ) : wxT( "N" );
    italicStr = m_type.isItalic ? wxT( "I" ) : wxT( "N" );

    if( m_type.textIsVisible == 1 )
        visibility = wxT( "0000" );
    else
        visibility = wxT( "0001" );

    if( m_type.textRotation == 900 )
        orientation = wxT( 'V' );
    else
        orientation = wxT( 'H' );

    aFile->Write( wxT( "F 4 \"" ) + m_type.text + wxT( "\" " ) + orientation + wxT( ' ' ) +
                  wxString::Format( wxT( "%d %d %d" ),
                                    m_type.textPositionX + m_positionX,
                                    m_type.textPositionY + m_positionY,
                                    GetCorrectedHeight( m_type.textHeight, m_type.isTrueType ) ) +
                  wxT( ' ' ) + visibility + wxT( ' ' ) + GetJustifyString( &m_type ) +
                  italicStr + boldStr + wxT( " \"Type\"\n" ) );

    for( i = 0; i < (int)m_attributes.GetCount(); i++ )
    {
        aFile->Write( wxString::Format( wxT( "F %d \"" ), i + 5 ) +
                      (m_attributes[i])->attrValue + wxT( "\" H " ) +
                      wxString::Format( wxT( "%d %d %d" ),
                                        m_positionX,
                                        m_positionY,
                                        60 ) +
                      wxT( " 0001 C CNN \"" ) + (m_attributes[i])->attrName + wxT( "\"\n" ) );
    }

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
    aFile->Write( wxString::Format( wxT( " %d %d %d\n" ), m_partNum, m_positionX, m_positionY ) );

    // Mirror is negative in compare with PCad represenation...
    a = 0;
    b = 0;
    c = 0;
    d = 0;

    switch( m_rotation )
    {
    case 0:
        a = m_mirror ? -1 : 1;
        d = -1;
        break;
    case 900:
        b = m_mirror ? 1 : -1;
        c = -1;
        break;
    case 1800:
        a = m_mirror ? 1 : -1;
        d = 1;
        break;
    case 2700:
        b = m_mirror ? -1 : 1;
        c = 1;
        break;
    }

    aFile->Write( wxString::Format( wxT( " %d %d %d %d \n" ), a, b, c, d ) );
    // FOOTPRINT
}


void SCH_SYMBOL::SetLibName( wxString aName )
{
    m_libName = aName;
}


void SCH_SYMBOL::CorrectField( TTEXTVALUE* aValue )
{
    aValue->textPositionY = -aValue->textPositionY;

    if( aValue->isLibValues )
    {
        if( aValue->textRotation == 1800 || aValue->textRotation == 2700 )
            RotateJustify180( &aValue->justify );

        if( aValue->mirror )
            MirrorJustify( &aValue->justify );
    }
    else
    {
        if( m_mirror )
        {
            aValue->textPositionX = -aValue->textPositionX;
            MirrorJustify( &aValue->justify );
        }

        if( aValue->mirror )
        {
            if( !m_mirror )
                aValue->textRotation = -aValue->textRotation;
            MirrorJustify( &aValue->justify );
        }

        aValue->textRotation = NormalizeAnglePos( aValue->textRotation - m_rotation );

        if( aValue->textRotation == 1800 || aValue->textRotation == 2700 )
            RotateJustify180( &aValue->justify );

        RotatePoint( &aValue->textPositionX, &aValue->textPositionY, (double) m_rotation );
    }

    if( aValue->textRotation == 1800 )
        aValue->textRotation = 0;
    else if( aValue->textRotation == 2700 )
        aValue->textRotation = 900;
}

} // namespace PCAD2KICAD
