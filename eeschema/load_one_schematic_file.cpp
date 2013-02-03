/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jean-pierre.charras@gipsa-lab.inpg.fr
 * Copyright (C) 2008 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file load_one_schematic_file.cpp
 * @brief Code to load and save Eeschema files.
 */

#include <fctsys.h>
#include <confirm.h>
#include <kicad_string.h>
#include <wxEeschemaStruct.h>
#include <richio.h>

#include <general.h>
#include <sch_bus_entry.h>
#include <sch_marker.h>
#include <sch_junction.h>
#include <sch_line.h>
#include <sch_no_connect.h>
#include <sch_component.h>
#include <sch_polyline.h>
#include <sch_text.h>
#include <sch_sheet.h>
#include <sch_bitmap.h>
#include <wildcards_and_files_ext.h>


bool ReadSchemaDescr( LINE_READER* aLine, wxString& aMsgDiag, SCH_SCREEN* Window );

static void LoadLayers( LINE_READER* aLine );


bool SCH_EDIT_FRAME::LoadOneEEFile( SCH_SCREEN* aScreen, const wxString& aFullFileName, bool append )
{
    char            name1[256];
    bool            itemLoaded = false;
    SCH_ITEM*       item;
    wxString        msgDiag;            // Error and log messages
    char*           line;
    wxFileName      fn;

    if( aScreen == NULL )
        return false;

    if( aFullFileName.IsEmpty() )
        return false;

    fn = aFullFileName;
    CheckForAutoSaveFile( fn, SchematicBackupFileExtension );

    wxLogTrace( traceAutoSave, wxT( "Loading schematic file " ) + aFullFileName );

    aScreen->SetCurItem( NULL );
    if( !append )
        aScreen->SetFileName( aFullFileName );

    FILE* f;
    wxString fname = aFullFileName;
#ifdef __WINDOWS__
    fname.Replace( wxT("/"), wxT("\\") );
#else
    fname.Replace( wxT("\\"), wxT("/") );
#endif

    if( ( f = wxFopen( fname, wxT( "rt" ) ) ) == NULL )
    {
        msgDiag = _( "Failed to open " ) + aFullFileName;
        DisplayError( this, msgDiag );
        return false;
    }

    // reader now owns the open FILE.
    FILE_LINE_READER    reader( f, aFullFileName );

    msgDiag = _( "Loading " ) + aScreen->GetFileName();
    PrintMsg( msgDiag );

    if( !reader.ReadLine()
        || strncmp( (char*)reader + 9, SCHEMATIC_HEAD_STRING,
                    sizeof( SCHEMATIC_HEAD_STRING ) - 1 ) != 0 )
    {
        msgDiag = aFullFileName + _( " is NOT an Eeschema file!" );
        DisplayError( this, msgDiag );
        return false;
    }

    line = reader.Line();

    // get the file version here.
    char *strversion = line + 9 + sizeof( SCHEMATIC_HEAD_STRING );

    // Skip blanks
    while( *strversion && *strversion < '0' )
        strversion++;

    int  version = atoi( strversion );

    if( version > EESCHEMA_VERSION )
    {
        msgDiag = aFullFileName + _( " was created by a more recent \
version of Eeschema and may not load correctly. Please consider updating!" );
        DisplayInfoMessage( this, msgDiag );
    }

#if 0
    // Compile it if the new version is unreadable by previous Eeschema versions
    else if( version < EESCHEMA_VERSION )
    {
        MsgDiag = aFullFileName + _( " was created by an older version of \
Eeschema. It will be stored in the new file format when you save this file \
again." );

        DisplayInfoMessage( this, MsgDiag );
    }
#endif

    if( !reader.ReadLine() || strncmp( reader, "LIBS:", 5 ) != 0 )
    {
        msgDiag = aFullFileName + _( " is NOT an Eeschema file!" );
        DisplayError( this, msgDiag );
        return false;
    }

    LoadLayers( &reader );

    while( reader.ReadLine() )
    {
        line = reader.Line();

        item = NULL;

        char* sline = line;

        while( (*sline != ' ' ) && *sline )
            sline++;

        switch( line[0] )
        {
        case '$':           // identification block
            if( line[1] == 'C' )
                item = new SCH_COMPONENT();
            else if( line[1] == 'S' )
                item = new SCH_SHEET();
            else if( line[1] == 'D' )
                itemLoaded = ReadSchemaDescr( &reader, msgDiag, aScreen );
            else if( line[1] == 'B' )
                item = new SCH_BITMAP();
            break;

        case 'L':        // Its a library item.
            item = new SCH_COMPONENT();
            break;

        case 'W':        // Its a Segment (WIRE or BUS) item.
            item = new SCH_LINE();
            break;

        case 'E':        // Its a WIRE or BUS item.
            item = new SCH_BUS_ENTRY();
            break;

        case 'P':        // Its a polyline item.
            item = new SCH_POLYLINE();
            break;

        case 'C':        // It is a connection item.
            item = new SCH_JUNCTION();
            break;

        case 'K':                       // It is a Marker item.
            // Markers are no more read from file. they are only created on
            // demand in schematic
            break;

        case 'N':                       // It is a NoConnect item.
            item = new SCH_NO_CONNECT();
            break;

        case 'T':                       // It is a text item.
            if( sscanf( sline, "%s", name1 ) != 1 )
            {
                msgDiag.Printf( _( "Eeschema file text load error at line %d" ),
                                reader.LineNumber() );
                itemLoaded = false;
            }
            else if( name1[0] == 'L' )
                item = new SCH_LABEL();
            else if( name1[0] == 'G' && version > 1 )
                item = new SCH_GLOBALLABEL();
            else if( (name1[0] == 'H') || (name1[0] == 'G' && version == 1) )
                item = new SCH_HIERLABEL();
            else
                item = new SCH_TEXT();
            break;

        default:
            itemLoaded = false;
            msgDiag.Printf( _( "Eeschema file undefined object at line %d, aborted" ),
                            reader.LineNumber() );
            msgDiag << wxT( "\n" ) << FROM_UTF8( line );
        }

        if( item )
        {
            itemLoaded = item->Load( reader, msgDiag );

            if( !itemLoaded )
            {
                SAFE_DELETE( item );
            }
            else
            {
                aScreen->Append( item );
            }
        }

        if( !itemLoaded )
        {
            DisplayError( this, msgDiag );
            break;
        }
    }

#if 0 && defined (DEBUG)
    aScreen->Show( 0, std::cout );
#endif

    aScreen->TestDanglingEnds();

    msgDiag = _( "Done Loading " ) + aScreen->GetFileName();
    PrintMsg( msgDiag );

    return true;    // Although it may be that file is only partially loaded.
}


static void LoadLayers( LINE_READER* aLine )
{
    /* read the layer descr
     * legacy code, not actually used, so this section is just skipped
     * read lines like
     * EELAYER 25  0
     * EELAYER END
     */

    while( aLine->ReadLine() )
    {
        if( strnicmp( *aLine, "EELAYER END", 11 ) == 0 )
            break;
    }
}

/// Get the length of a string constant, at compile time
#define SZ( x )         (sizeof(x)-1)

static const char delims[] = " \t\r\n";

/* Read the schematic header. */
bool ReadSchemaDescr( LINE_READER* aLine, wxString& aMsgDiag, SCH_SCREEN* aScreen )
{
    char*   line = aLine->Line();

    char*   pageType = strtok( line + SZ( "$Descr" ), delims );
    char*   width    = strtok( NULL, delims );
    char*   height   = strtok( NULL, delims );
    char*   orient   = strtok( NULL, delims );

    wxString pagename = FROM_UTF8( pageType );

    PAGE_INFO       pageInfo;
    TITLE_BLOCK     tb;

    if( !pageInfo.SetType( pagename ) )
    {
        aMsgDiag.Printf( _( "Eeschema file dimension definition error \
line %d,\nAbort reading file.\n" ),
                         aLine->LineNumber() );
        aMsgDiag << FROM_UTF8( line );
    }

    if( pagename == PAGE_INFO::Custom )
    {
        if( width && height )
        {
            int w = atoi( width );
            int h = atoi( height );

            pageInfo.SetWidthMils( w );
            pageInfo.SetHeightMils( h );
        }
    }

    // non custom size, set portrait if its present
    else if( orient && !strcmp( orient, "portrait" ) )
    {
        pageInfo.SetPortrait( true );
    }

    aScreen->SetPageSettings( pageInfo );

    for(;;)
    {
        char    buf[1024];

        if( !aLine->ReadLine() )
            return true;

        line = aLine->Line();

        if( strnicmp( line, "$End", 4 ) == 0 )
        {
            aScreen->SetTitleBlock( tb );
            break;
        }

        if( strnicmp( line, "Sheet", 2 ) == 0 )
            sscanf( line + 5, " %d %d",
                    &aScreen->m_ScreenNumber, &aScreen->m_NumberOfScreens );

        if( strnicmp( line, "Title", 2 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetTitle( FROM_UTF8( buf ) );
            continue;
        }

        if( strnicmp( line, "Date", 2 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetDate( FROM_UTF8( buf ) );
            continue;
        }

        if( strnicmp( line, "Rev", 2 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetRevision( FROM_UTF8( buf ) );
            continue;
        }

        if( strnicmp( line, "Comp", 4 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetCompany( FROM_UTF8( buf ) );
            continue;
        }

        if( strnicmp( line, "Comment1", 8 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetComment1( FROM_UTF8( buf ) );
            continue;
        }

        if( strnicmp( line, "Comment2", 8 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetComment2( FROM_UTF8( buf ) );
            continue;
        }

        if( strnicmp( line, "Comment3", 8 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetComment3( FROM_UTF8( buf ) );
            continue;
        }

        if( strnicmp( line, "Comment4", 8 ) == 0 )
        {
            ReadDelimitedText( buf, line, 256 );
            tb.SetComment4( FROM_UTF8( buf ) );
            continue;
        }
    }

    return true;
}
