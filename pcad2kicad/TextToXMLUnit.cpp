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
 * @file TextToXMLUnit.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <wx/xml/xml.h>

namespace PCAD2KICAD {

void TextToXML( wxStatusBar* aStatusBar, wxString aXMLFileName, wxArrayString* aLines )
{
    wxXmlNode*      cNode, * iNode;
    wxXmlDocument   xmlDoc;
    wxString        ls, propValue, content;
    int             i;

    xmlDoc.SetFileEncoding( wxT( "WINDOWS-1251" ) );
    iNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "www.lura.sk" ) );
    xmlDoc.SetRoot( iNode );

    i = aLines->GetCount() - 1;

    if( aStatusBar )
        aStatusBar->SetStatusText( wxT( "Creating XML file..." ) );

    while( i >= 0 )
    {
        ls = (*aLines)[i--];

        if( ls == wxT( "GoUP" ) )
        {
            iNode = iNode->GetParent();
        }
        else if( ls == wxT( "GoDOWN" ) )
        {
            ls = (*aLines)[i--];
            cNode = new wxXmlNode( wxXML_ELEMENT_NODE, ls );
            iNode->AddChild( cNode );

            while( i >= 0 )
            {
                ls = (*aLines)[i];

                if( ls == wxT( "GoUP" ) || ls == wxT( "GoDOWN" ) )
                {
                    iNode = cNode;
                    break;
                }
                else
                {
                    if( (ls.Len() > 0) && (ls[0] == '"') )
                    {
                        ls = ls.Mid( 1, ls.Len() - 2 );

                        if( cNode->GetAttribute( wxT( "Name" ), &propValue ) )
                        {
                            cNode->DeleteProperty( wxT( "Name" ) );
                            cNode->AddProperty( wxT( "Name" ), propValue + ' ' + ls );
                        }
                        else
                            cNode->AddProperty( wxT( "Name" ), ls );
                    }
                    else
                    {
                        // update node content
                        content = cNode->GetNodeContent() + ' ' + ls;

                        if( cNode->GetChildren() )
                            cNode->GetChildren()->SetContent( content );
                        else
                            cNode->AddChild( new wxXmlNode( wxXML_TEXT_NODE,
                                                            wxEmptyString,
                                                            content ) );
                    }

                    i--;
                }
            }
        }
        else
        {
            if( ls.Len() > 0 && ls[0] == '"' )
            {
                ls = ls.Mid( 1, ls.Len() - 2 );
            }

            cNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "value" ) );
            iNode->AddChild( cNode );

            if( ls.Len() > 0 && ls[0] == '"' )
            {
                ls = ls.Mid( 1, ls.Len() - 2 );
            }

            cNode->AddChild( new wxXmlNode( wxXML_TEXT_NODE, wxEmptyString, ls ) );
        }
    }

    if( aStatusBar )
        aStatusBar->SetStatusText( wxT( "Saving XML file : " ) + aXMLFileName );

    xmlDoc.Save( aXMLFileName );
}

} // namespace PCAD2KICAD
