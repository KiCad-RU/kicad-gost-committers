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
 * @file LoadInputFile.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/textfile.h>

#include <XMLtoObjectCommonProceduresUnit.h>

namespace PCAD2KICAD {

int DoDepth( wxString* aStr )
{
    int result = 0;

    aStr->Trim( false );

    if( aStr->Len() > 0 )
    {
        if( (*aStr)[0] == wxT( '(' ) )
        {
            result  = 1;
            *aStr   = aStr->Mid( 1 );
        }
    }

    if( aStr->Len() > 0 )
    {
        if( (*aStr)[0] == wxT( ')' ) )
        {
            result  = -1;
            *aStr   = aStr->Mid( 1 );
        }
    }

    return result;
}


static wxString dummyLines[44] =
{
    // wxT( "(padShape (layerType Signal)" ),
    // wxT( "(padShape (layerType Plane)" ),
    // wxT( "(padShape (layerType NonSignal)" ),
    wxT( "(textStyleDisplayTType " ),
    wxT( "(textStyleAllowTType " ),
    wxT( "(attr \"PadToPadClearance\"" ),
    wxT( "(attr \"PadToLineClearance\"" ),
    wxT( "(attr \"LineToLineClearance\"" ),
    wxT( "(attr \"ViaToPadClearance\"" ),
    wxT( "(attr \"ViaToLineClearance\"" ),
    wxT( "(attr \"ViaToViaClearance\"" ),
    wxT( "(fieldSetRef " ),
    wxT( "(attr \"SimType\"" ),
    wxT( "(attr \"SimPins\"" ),
    wxT( "(attr \"SimNetlist\"" ),
    wxT( "(attr \"SimField1\"" ),
    wxT( "(attr \"SimField2\"" ),
    wxT( "(attr \"SimField3\"" ),
    wxT( "(attr \"SimField4\"" ),
    wxT( "(attr \"SimField5\"" ),
    wxT( "(attr \"SimField6\"" ),
    wxT( "(attr \"SimField7\"" ),
    wxT( "(attr \"SimField8\"" ),
    wxT( "(attr \"SimField9\"" ),
    wxT( "(attr \"SimField10\"" ),
    wxT( "(attr \"SimField11\"" ),
    wxT( "(attr \"SimField12\"" ),
    wxT( "(attr \"SimField13\"" ),
    wxT( "(attr \"SimField14\"" ),
    wxT( "(attr \"SimField15\"" ),
    wxT( "(attr \"SimField16\"" ),
    wxT( "(attr \"Part Field 1\"" ),
    wxT( "(attr \"Part Field 2\"" ),
    wxT( "(attr \"Part Field 3\"" ),
    wxT( "(attr \"Part Field 4\"" ),
    wxT( "(attr \"Part Field 5\"" ),
    wxT( "(attr \"Part Field 6\"" ),
    wxT( "(attr \"Part Field 7\"" ),
    wxT( "(attr \"Part Field 8\"" ),
    wxT( "(attr \"Part Field 9\"" ),
    wxT( "(attr \"Part Field 10\"" ),
    wxT( "(attr \"Part Field 11\"" ),
    wxT( "(attr \"Part Field 12\"" ),
    wxT( "(attr \"Part Field 13\"" ),
    wxT( "(attr \"Part Field 14\"" ),
    wxT( "(attr \"Part Field 15\"" ),
    wxT( "(attr \"Part Field 16\"" )
};

// SKIP UNCONVERTED LINES
static bool LineIsOk( wxString aLine )
{
    bool result = true;

    for( int i = 0; i < 44; i++ )
    {
        if( aLine.Find( dummyLines[i] ) != wxNOT_FOUND )
            result = false;
    }

    return result;
}


static wxString GetLine( wxTextFile* aFile, bool aFirstLine )
{
    wxString result;

    // int idx;

    result = wxT( "END OF INPUT FILE" );

    while( !aFile->Eof() )
    {
        if( aFirstLine )
        {
            result      = aFile->GetFirstLine();
            aFirstLine  = false;
        }
        else
            result = aFile->GetNextLine();

        if( LineIsOk( result ) )
        {
            // fix copyright symbol
            /*idx = result.Find(wxT("\251"));
             *  if (idx != wxNOT_FOUND) {
             *   result = result.Left(idx) + wxT("\302") + result.Mid(idx);
             *  }*/
            return result;
        }
    }

    if( aFile->Eof() )
        result = wxT( "END OF INPUT FILE" );

    return result;
}


void LoadInputFile( wxString aFileName, wxStatusBar* aStatusBar, wxArrayString* aLines )
{
    wxArrayString   lines;
    wxTextFile      f;
    wxString        s, w;
    int             fileLine = 0, depth = 0, i = 0;

    if( aStatusBar )
        aStatusBar->SetStatusText( wxT( "Opening file : " ) + aFileName );

    f.Open( aFileName );
    // preallocate memory for array to increase performance
    size_t lines_qty = f.GetLineCount();
    lines.Alloc( lines_qty * 10 );

    s = GetLine( &f, true );

    if( aStatusBar )
        aStatusBar->SetStatusText( wxT( "Processing input file..." ) );

    while( s != wxT( "END OF INPUT FILE" ) )
    {
        fileLine++;

        s.Trim( false );

        while( s.Len() > 0 )
        {
            i       = DoDepth( &s );
            depth   = depth + i;

            if( i == -1 )
                lines.Add( wxT( "GoUP" ) );

            if( i == 1 )
                lines.Add( wxT( "GoDOWN" ) );

            if( i == 0 )
            {
                w = GetWord( &s );

                if( w.Len() > 0 )
                {
                    if( w[0] == wxT( '-' ) )
                        w = wxT( '_' ) + w;

                    if( w[0] == wxT( '+' ) )
                        w = wxT( '_' ) + w;

                    lines.Add( w );
                }
            }
        }

        s = GetLine( &f, false );
    }

    if( aStatusBar )
        aStatusBar->SetStatusText( wxT( "Input file processed  : " ) +
                                   wxString::Format( wxT( "%d" ), fileLine ) +
                                   wxT( " lines." ) );

    // preallocate memory for array to increase performance
    aLines->Alloc( lines.GetCount() );

    // reverse order of lines
    for( i = lines.GetCount() - 1; i>=0; i-- )
        aLines->Add( lines[i] );

    f.Close();
}

} // namespace PCAD2KICAD
