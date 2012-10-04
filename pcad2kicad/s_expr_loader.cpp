/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
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
 * @file s_expr_loader.cpp
 */

#include <assert.h>
#include <dsnlexer.h>
#include <macros.h>
#include <wx/xml/xml.h>

namespace PCAD2KICAD {

static KEYWORD empty_keywords[1] = {};

void LoadInputFile( wxString aFileName, wxXmlDocument* aXmlDoc )
{
    int tok;
    wxXmlNode* iNode = NULL, *cNode = NULL;
    wxString str;
    bool growing = false;
    bool attr = false;

    FILE* fp = wxFopen( aFileName, wxT( "rt" ) );

    if( !fp )
    {
        str.Printf( _( "Unable to open file '%s'" ), aFileName.GetData() );
        THROW_IO_ERROR( str );
    }

    // lexer now owns fp, will close on exception or return
    DSNLEXER lexer( empty_keywords, 0, fp,  aFileName );

    iNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "www.lura.sk" ) );

    while( ( tok = lexer.NextTok() ) != DSN_EOF )
    {
        if( growing && ( tok == DSN_LEFT || tok == DSN_RIGHT ) )
        {
            if( attr )
            {
                cNode->AddProperty( wxT( "Name" ), str.Trim( false ) );
            }
            else
            {
                cNode->AddChild( new wxXmlNode( wxXML_TEXT_NODE, wxEmptyString, str ) );
            }

            growing = false;
            attr = false;
        }

        if( tok == DSN_RIGHT )
        {
            iNode = iNode->GetParent();
        }
        else if( tok == DSN_LEFT )
        {
            tok = lexer.NextTok();
            str = wxEmptyString;
            cNode = new wxXmlNode( wxXML_ELEMENT_NODE, lexer.CurText() );
            iNode->AddChild( cNode );
            iNode = cNode;
            growing = true;
        }
        else
        {
            str += wxT( ' ' );
            str += lexer.CurText();
            if( tok == DSN_STRING )
                attr = true;
        }
    }

    if( iNode )
    {
        aXmlDoc->SetRoot( iNode );
        //aXmlDoc->Save( wxT( "test.xml" ) );
    }
}

} // namespace PCAD2KICAD
