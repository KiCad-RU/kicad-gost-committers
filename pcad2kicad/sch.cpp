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
 * @file sch.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/xml/xml.h>
#include <wx/filename.h>

#include <XMLtoObjectCommonProceduresUnit.h>
#include <common.h>

#include <sch.h>
#include <sch_module.h>

namespace PCAD2KICAD {

SCH::SCH()
{
    m_defaultMeasurementUnit = wxEmptyString;
    m_sizeX = 0;
    m_sizeY = 0;
}


SCH::~SCH()
{
    int i;

    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
        delete m_schComponents[i];
}


void SCH::DoAlias( wxString aAlias )
{
    int         i;
    wxString    a, n;

    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( (m_schComponents[i])->m_objType == wxT( "module" ) )
        {
            n   = aAlias;
            a   = GetWord( &n );
            n.Trim( true );
            n.Trim( false );

            if( ( (SCH_MODULE*) m_schComponents[i] )->m_name.text == n )
            {
                ( (SCH_MODULE*) m_schComponents[i] )->m_alias += wxT( ' ' ) + a;
            }
        }
    }
}

bool SCH::DoesModuleAlreadyExist( wxString aOriginalName )
{
    int i;

    for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
    {
        if( ( (SCH_MODULE* )m_schComponents[i] )->m_name.text == aOriginalName )
            return true;
    }

    return false;
}

// FULL PROCESS OF LIBRARY CONVERSION
void SCH::DoLibrary( wxXmlDocument* aXmlDoc, wxStatusBar* aStatusBar, wxString aActualConversion )
{
    wxXmlNode*  aNode;
    wxString    propValue;
    SCH_MODULE* module;

    aNode = FindNode( aXmlDoc->GetRoot(), wxT( "library" ) );

    if( aNode )    // ORIGINAL Modules
    {
        aNode = aNode->GetChildren();

        while( aNode )
        {
            if( aNode->GetName() == wxT( "compDef" ) )
            {
                FindNode( aNode, wxT( "originalName" ) )->GetAttribute( wxT( "Name" ),
                                                            &propValue );
                propValue.Trim( false );

                if( !DoesModuleAlreadyExist( propValue ) )
                {
                    module = new SCH_MODULE;
                    module->Parse( aNode, aStatusBar, m_defaultMeasurementUnit, aActualConversion );
                    m_schComponents.Add( module );
                }
            }

            aNode = aNode->GetNext();
        }
    }

    aNode = FindNode( aXmlDoc->GetRoot(), wxT( "library" ) );

    if( aNode )    // ALIASes
    {
        aNode = aNode->GetChildren();

        while( aNode )
        {
            if( aNode->GetName() == wxT( "compAlias" ) )
            {
                aNode->GetAttribute( wxT( "Name" ), &propValue );
                DoAlias( propValue );
            }

            aNode = aNode->GetNext();
        }
    }
}

void SCH::Parse( wxStatusBar* aStatusBar, wxXmlDocument* aXmlDoc, wxString aActualConversion )
{
    wxXmlNode*      iNode;
    SCH_SHEET*      sheet;
    bool            isJunction;

    // Defaut measurement units
    iNode = FindNode( aXmlDoc->GetRoot(), wxT( "asciiHeader" ) );

    if( iNode )
        m_defaultMeasurementUnit = FindNodeGetContent( iNode, wxT( "fileUnits" ) );

    // Allways also library file
    isJunction = false;
    aActualConversion = wxT( "SCHLIB" );

    DoLibrary( aXmlDoc, aStatusBar, aActualConversion );

    iNode = FindNode( aXmlDoc->GetRoot(), wxT( "schematicDesign" ) );

    if( iNode )
    {
        // SCHEMATIC FILE
        // iNode is schematicDesign node actually
        aActualConversion = wxT( "SCH" );

        iNode = iNode->GetChildren();
        while ( iNode )
        {
            if( iNode->GetName() == wxT( "sheet" ) )
            {
                sheet = new SCH_SHEET;
                sheet->Parse( iNode, m_defaultMeasurementUnit );
                m_sheets.Add(sheet);
            }

            iNode = iNode->GetNext();
        }
    }
}


void SCH::WriteToFile( wxString aFileName, char aFileType )
{
    wxFile  f;
    int     i;

    if( aFileType == wxT( 'L' ) )
    {
        // LIBRARY
        f.Open( aFileName, wxFile::write );
        f.Write( wxT( "EESchema-LIBRARY Version 2.3  Date: 01/1/2001-01:01:01\n" ) );
        f.Write( wxT( "\n" ) );

        for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
        {
            if( m_schComponents[i]->m_objType == wxT( "module" ) )
                m_schComponents[i]->WriteToFile( &f, aFileType );
        }

        f.Write( wxT( "# End Library\n" ) );
        f.Close();    // also modules descriptions

        wxFileName dcmFile( aFileName );
        dcmFile.SetExt( wxT( "dcm" ) );
        f.Open( dcmFile.GetFullPath(), wxFile::write );
        f.Write( wxT( "EESchema-DOCLIB  Version 2.0  Date: 01/01/2000-01:01:01\n" ) );

        for( i = 0; i < (int) m_schComponents.GetCount(); i++ )
        {
            if( m_schComponents[i]->m_objType == wxT(" module ") )
            {
                f.Write( wxT( "#\n" ) );
                f.Write( wxT( "$CMP " ) +
                         ValidateName( ( (SCH_MODULE*) m_schComponents[i] )->m_name.text ) +
                         wxT( "\n" ) );
                f.Write( wxT( "D " ) + ( (SCH_MODULE*) m_schComponents[i] )->m_moduleDescription +
                         wxT( "\n" ) );
                f.Write( wxT( "K \n" ) );    // no information available
                f.Write( wxT( "$ENDCMP\n" ) );
            }
        }

        f.Write( wxT( "#\n" ) );
        f.Write( wxT( "#End Doc Library\n" ) );
        f.Close();
    }    // LIBRARY

    if( aFileType == wxT( 'S' ) )
    {
        // SCHEMATICS

        for( i = 0; i < (int) m_sheets.GetCount(); i++ )
        {
            m_sheets[i]->WriteToFile( aFileName );
        }
    }
}

} // namespace PCAD2KICAD
