/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file gestfich.cpp
 * @brief Functions for file management
 */

// For compilers that support precompilation, includes "wx.h".
#include <fctsys.h>
#include <appl_wxstruct.h>
#include <confirm.h>

#include <common.h>
#include <macros.h>
#include <gestfich.h>

#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/dir.h>

/* List of default paths used to locate help files and KiCad library files.
 *
 * Under windows, KiCad search its files from the binary path file (first
 * argument when running "main")   So for a standard install, default paths
 * are not mandatory, but they exist, just in case.
 * KiCad is often installed in c:/Program Files/kicad or c:/kicad (or d: or
 * e: ... ) and the directory "share" has no meaning under windows.
 *
 * Under linux, the problem is more complex.
 * In fact there are 3 cases:
 * 1 - When released in a distribution:
 * binaries are in /usr/bin, KiCad libs in /usr/share/kicad/ and doc in
 * /usr/share/doc/kicad/
 * 2 - When compiled by an user:
 * binaries also can be  in /usr/local/bin, KiCad libs in
 * /usr/local/share/kicad/ and doc in /usr/local/share/doc/kicad/
 * 3 - When in an "universal tarball" or build for a server:
 * all files are in /usr/local/kicad
 * This is mandatory when KiCad is installed on a server (in a school for
 * instance) because one can export /usr/local/kicad and obviously the others
 * paths cannot be used (cannot be mounted by the client, because they are
 * already used).
 *
 * in cases 1 and 2 KiCad files cannot be found from the binary path.
 * in case 3 KiCad files can be found from the binary path only if this is
 * a KiCad binary file which is launched.
 * But if an user creates a symbolic link to the actual binary file to run
 * KiCad, the binary path is not good and the defaults paths must be used
 *
 * Note:
 * KiCad uses first the bin path lo locate KiCad tree.
 * if not found KiCad uses the environment variable KICAD to find its files
 * and at last KiCad uses the default paths.
 * So we can export (linux and windows) the variable KICAD:
 *  like export KICAD=/my_path/kicad if /my_path/kicad is not a default path
 */

// Path list for online help
static wxString    s_HelpPathList[] = {
#ifdef __WINDOWS__
    wxT( "c:/kicad/doc/help/" ),
    wxT( "d:/kicad/doc/help/" ),
    wxT( "c:/Program Files/kicad/doc/help/" ),
    wxT( "d:/Program Files/kicad/doc/help/" ),
#else
    wxT( "/usr/share/doc/kicad/help/" ),
    wxT( "/usr/local/share/doc/kicad/help/" ),
    wxT( "/usr/local/kicad/doc/help/" ),    // default install for "universal
                                            // tarballs" and build for a server
                                            // (new)
    wxT( "/usr/local/kicad/help/" ),        // default install for "universal
                                            // tarballs" and build for a server
                                            // (old)
#endif
    wxT( "end_list" )                       // End of list symbol, do not change
};


// Path list for KiCad data files
static wxString    s_KicadDataPathList[] = {
#ifdef __WINDOWS__
    wxT( "c:/kicad/share/" ),
    wxT( "d:/kicad/share/" ),
    wxT( "c:/kicad/" ),
    wxT( "d:/kicad/" ),
    wxT( "c:/Program Files/kicad/share/" ),
    wxT( "d:/Program Files/kicad/share/" ),
    wxT( "c:/Program Files/kicad/" ),
    wxT( "d:/Program Files/kicad/" ),
#else
    wxT( "/usr/share/kicad/" ),
    wxT( "/usr/local/share/kicad/" ),
    wxT( "/usr/local/kicad/share/" ),   // default data path for "universal
                                        // tarballs" and build for a server
                                        // (new)
    wxT( "/usr/local/kicad/" ),         // default data path for "universal
                                        // tarballs" and build for a server
                                        // (old)
#endif
    wxT( "end_list" )                   // End of list symbol, do not change
};

// Path list for KiCad binary files
static wxString    s_KicadBinaryPathList[] = {
#ifdef __WINDOWS__
    wxT( "c:/kicad/bin/" ),
    wxT( "d:/kicad/bin/" ),
    wxT( "c:/Program Files/kicad/bin/" ),
    wxT( "d:/Program Files/kicad/bin/" ),
#else
    wxT( "/usr/bin/" ),
    wxT( "/usr/local/bin/" ),
    wxT( "/usr/local/kicad/bin/" ),
#endif
    wxT( "end_list" )                   // End of list symbol, do not change
};


wxString MakeReducedFileName( const wxString& fullfilename,
                              const wxString& default_path,
                              const wxString& default_ext )
{
    wxString reduced_filename = fullfilename;
    wxString Cwd, ext, path;

    Cwd  = default_path;
    ext  = default_ext;
    path = wxPathOnly( reduced_filename ) + UNIX_STRING_DIR_SEP;
    reduced_filename.Replace( WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP );
    Cwd.Replace( WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP );

    if( Cwd.Last() != '/' )
        Cwd += UNIX_STRING_DIR_SEP;

    path.Replace( WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP );

#ifdef __WINDOWS__

    // names are case insensitive under windows
    path.MakeLower();
    Cwd.MakeLower();
    ext.MakeLower();
#endif

    // if the path is "default_path" -> remove it
    wxString root_path = path.Left( Cwd.Length() );

    if( root_path == Cwd )
    {
        reduced_filename.Remove( 0, Cwd.Length() );
    }
    else    // if the path is the current path -> change path to ./
    {
        Cwd = wxGetCwd() + UNIX_STRING_DIR_SEP;
#ifdef __WINDOWS__
        Cwd.MakeLower();
#endif
        Cwd.Replace( WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP );

        if( path == Cwd )
        {   // the path is the current path -> path = "./"
            reduced_filename.Remove( 0, Cwd.Length() );
            wxString tmp = wxT( "./" ) + reduced_filename;
            reduced_filename = tmp;
        }
    }

    // remove extension if == default_ext:
    if( !ext.IsEmpty() && reduced_filename.Contains( ext ) )
        reduced_filename.Truncate( reduced_filename.Length() - ext.Length() );

    return reduced_filename;
}


void AddDelimiterString( wxString& string )
{
    wxString text;

    if( !string.StartsWith( wxT( "\"" ) ) )
        text = wxT( "\"" );

    text += string;

    if( (text.Last() != '"' ) || (text.length() <= 1) )
        text += wxT( "\"" );

    string = text;
}


bool EDA_DirectorySelector( const wxString& Title,
                            wxString&       Path,
                            int             flag,
                            wxWindow*       Frame,
                            const wxPoint&  Pos )
{
    int          ii;
    bool         selected = false;

    wxDirDialog* DirFrame = new wxDirDialog( Frame,
                                             wxString( Title ),
                                             Path,
                                             flag,
                                             Pos );

    ii = DirFrame->ShowModal();

    if( ii == wxID_OK )
    {
        Path     = DirFrame->GetPath();
        selected = true;
    }

    DirFrame->Destroy();
    return selected;
}


wxString EDA_FileSelector( const wxString& Title,
                           const wxString& Path,
                           const wxString& FileName,
                           const wxString& Ext,
                           const wxString& Mask,
                           wxWindow*       Frame,
                           int             flag,
                           const bool      keep_working_directory,
                           const wxPoint&  Pos )
{
    wxString fullfilename;
    wxString curr_cwd    = wxGetCwd();
    wxString defaultname = FileName;
    wxString defaultpath = Path;
    wxString dotted_Ext = wxT(".") + Ext;

#ifdef __WINDOWS__
    defaultname.Replace( wxT( "/" ), wxT( "\\" ) );
    defaultpath.Replace( wxT( "/" ), wxT( "\\" ) );
#endif

    if( defaultpath.IsEmpty() )
        defaultpath = wxGetCwd();

    wxSetWorkingDirectory( defaultpath );

#if 0 && defined (DEBUG)
    printf( "defaultpath=\"%s\" defaultname=\"%s\" Ext=\"%s\" Mask=\"%s\" flag=%d keep_working_directory=%d\n",
            TO_UTF8( defaultpath ),
            TO_UTF8( defaultname ),
            TO_UTF8( Ext ),
            TO_UTF8( Mask ),
            flag,
            keep_working_directory );
#endif

    fullfilename = wxFileSelector( wxString( Title ),
                                   defaultpath,
                                   defaultname,
                                   dotted_Ext,
                                   Mask,
                                   flag, /* open mode wxFD_OPEN, wxFD_SAVE .. */
                                   Frame,
                                   Pos.x, Pos.y );

    if( keep_working_directory )
        wxSetWorkingDirectory( curr_cwd );

    return fullfilename;
}


wxString FindKicadHelpPath()
{
    wxString FullPath, LangFullPath, tmp;
    wxString LocaleString;
    bool     PathFound = false;

    /* find kicad/help/ */
    tmp = wxGetApp().GetExecutablePath();

    if( tmp.Last() == '/' )
        tmp.RemoveLast();

    FullPath     = tmp.BeforeLast( '/' ); // cd ..
    FullPath    += wxT( "/doc/help/" );
    LocaleString = wxGetApp().GetLocale()->GetCanonicalName();

    wxString path_tmp = FullPath;
#ifdef __WINDOWS__
    path_tmp.MakeLower();
#endif
    if( path_tmp.Contains( wxT( "kicad" ) ) )
    {
        if( wxDirExists( FullPath ) )
            PathFound = true;
    }

    /* find kicad/help/ from environment variable  KICAD */
    if( !PathFound && wxGetApp().IsKicadEnvVariableDefined() )
    {
        FullPath = wxGetApp().GetKicadEnvVariable() + wxT( "/doc/help/" );

        if( wxDirExists( FullPath ) )
            PathFound = true;
    }

    /* find kicad/help/ from "s_HelpPathList" */
    int ii = 0;

    while( !PathFound )
    {
        FullPath = s_HelpPathList[ii++];

        if( FullPath == wxT( "end_list" ) )
            break;

        if( wxDirExists( FullPath ) )
            PathFound = true;
    }

    if( PathFound )
    {
        LangFullPath = FullPath + LocaleString + UNIX_STRING_DIR_SEP;

        if( wxDirExists( LangFullPath ) )
            return LangFullPath;

        LangFullPath = FullPath + LocaleString.Left( 2 ) + UNIX_STRING_DIR_SEP;

        if( wxDirExists( LangFullPath ) )
            return LangFullPath;

        LangFullPath = FullPath + wxT( "en/" );

        if( wxDirExists( LangFullPath ) )
        {
            return LangFullPath;
        }
        else
        {
            LangFullPath = FullPath + wxT( "fr/" );

            if( wxDirExists( LangFullPath ) )
                return LangFullPath;
        }
        return FullPath;
    }
    return wxEmptyString;
}


wxString FindKicadFile( const wxString& shortname )
{
    wxString FullFileName;

    /* Test the presence of the file in the directory shortname of
     * the KiCad binary path.
     */
    FullFileName = wxGetApp().GetExecutablePath() + shortname;

    if( wxFileExists( FullFileName ) )
        return FullFileName;

    /* Test the presence of the file in the directory shortname
     * defined by the environment variable KiCad.
     */
    if( wxGetApp().IsKicadEnvVariableDefined() )
    {
        FullFileName = wxGetApp().GetKicadEnvVariable() + shortname;

        if( wxFileExists( FullFileName ) )
            return FullFileName;
    }

    /* find binary file from default path list:
     *  /usr/local/kicad/linux or c:/kicad/winexe
     *  (see s_KicadDataPathList) */
    int ii = 0;

    while( 1 )
    {
        if( s_KicadBinaryPathList[ii] == wxT( "end_list" ) )
            break;

        FullFileName = s_KicadBinaryPathList[ii++] + shortname;

        if( wxFileExists( FullFileName ) )
            return FullFileName;
    }

    return shortname;
}


int ExecuteFile( wxWindow* frame, const wxString& ExecFile, const wxString& param,
                 wxProcess *callback )
{
    wxString FullFileName;


    FullFileName = FindKicadFile( ExecFile );

#ifdef __WXMAC__
    if( wxFileExists( FullFileName ) || wxDir::Exists( FullFileName ) )
    {
        return ProcessExecute( wxGetApp().GetExecutablePath() + wxT( "/" )
                               + ExecFile + wxT( " " )
                               + param, wxEXEC_ASYNC, callback );
    }
    else
    {
        return ProcessExecute( wxT( "/usr/bin/open " ) + param, wxEXEC_ASYNC, callback );
    }
#else
    if( wxFileExists( FullFileName ) )
    {
        if( !param.IsEmpty() )
            FullFileName += wxT( " " ) + param;

        return ProcessExecute( FullFileName, wxEXEC_ASYNC, callback );
    }
#endif
    wxString msg;
    msg.Printf( _( "Command <%s> could not found" ), GetChars( FullFileName ) );
    DisplayError( frame, msg, 20 );
    return -1;
}


wxString ReturnKicadDatasPath()
{
    bool     PathFound = false;
    wxString data_path;

    if( wxGetApp().IsKicadEnvVariableDefined() ) // Path defined by the KICAD environment variable.
    {
        data_path = wxGetApp().GetKicadEnvVariable();
        PathFound = true;
    }
    else    // Path of executables.
    {
        wxString tmp = wxGetApp().GetExecutablePath();
#ifdef __WINDOWS__
        tmp.MakeLower();
#endif
        if( tmp.Contains( wxT( "kicad" ) ) )
        {
#ifdef __WINDOWS__
            tmp = wxGetApp().GetExecutablePath();
#endif
            if( tmp.Last() == '/' )
                tmp.RemoveLast();

            data_path  = tmp.BeforeLast( '/' ); // id cd ../
            data_path += UNIX_STRING_DIR_SEP;

            // Old versions of KiCad use kicad/ as default for data
            // and last versions kicad/share/
            // So we search for kicad/share/ first
            wxString old_path = data_path;
            data_path += wxT( "share/" );

            if( wxDirExists( data_path ) )
            {
                PathFound = true;
            }
            else if( wxDirExists( old_path ) )
            {
                data_path = old_path;
                PathFound = true;
            }
        }
    }

    /* find KiCad from default path list:
     *  /usr/local/kicad/ or c:/kicad/
     *  (see s_KicadDataPathList) */
    int ii = 0;

    while( !PathFound )
    {
        if( s_KicadDataPathList[ii] == wxT( "end_list" ) )
            break;

        data_path = s_KicadDataPathList[ii++];

        if( wxDirExists( data_path ) )
            PathFound = true;
    }

    if( PathFound )
    {
        data_path.Replace( WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP );

        if( data_path.Last() != '/' )
            data_path += UNIX_STRING_DIR_SEP;
    }
    else
    {
        data_path.Empty();
    }

    return data_path;
}


wxString& EDA_APP::GetEditorName()
{
    wxString editorname = m_EditorName;

    // We get the preferred editor name from environment variable first.
    if( editorname.IsEmpty() )
    {
    // If there is no EDITOR variable set, try the desktop default
        if(!wxGetEnv( wxT( "EDITOR" ), &editorname ))
        {
#ifdef __WXMAC__
          editorname = "/usr/bin/open";
#elif __WXX11__
          editorname = "/usr/bin/xdg-open";
#endif
        }
    }
    if( editorname.IsEmpty() ) // We must get a preferred editor name
    {
        DisplayInfoMessage( NULL,
                            _( "No default editor found, you must choose it" ) );
        wxString mask( wxT( "*" ) );

#ifdef __WINDOWS__
        mask += wxT( ".exe" );
#endif
        editorname = EDA_FileSelector( _( "Preferred Editor:" ), wxEmptyString,
                                       wxEmptyString, wxEmptyString, mask,
                                       NULL, wxFD_OPEN, true );
    }

    if( !editorname.IsEmpty() )
    {
        m_EditorName = editorname;
        m_commonSettings->Write( wxT( "Editor" ), m_EditorName );
    }

    return m_EditorName;
}


bool OpenPDF( const wxString& file )
{
    wxString command;
    wxString filename = file;
    wxString type;
    bool     success = false;

    wxGetApp().ReadPdfBrowserInfos();

    if( !wxGetApp().UseSystemPdfBrowser() )    //  Run the preferred PDF Browser
    {
        AddDelimiterString( filename );
        command = wxGetApp().GetPdfBrowserFileName() + wxT( " " ) + filename;
    }
    else
    {
        wxFileType* filetype = NULL;
        wxFileType::MessageParameters params( filename, type );
        filetype = wxTheMimeTypesManager->GetFileTypeFromExtension( wxT( "pdf" ) );

        if( filetype )
            success = filetype->GetOpenCommand( &command, params );

        delete filetype;

#ifndef __WINDOWS__

        // Bug ? under linux wxWidgets returns acroread as PDF viewer, even if
        // it does not exist.
        if( command.StartsWith( wxT( "acroread" ) ) ) // Workaround
            success = false;
#endif

        if( success && !command.IsEmpty() )
        {
            success = ProcessExecute( command );

            if( success )
                return success;
        }

        success = false;
        command.Empty();

        if( !success )
        {
#ifndef __WINDOWS__
            AddDelimiterString( filename );

            /* here is a list of PDF viewers candidates */
            const static wxString tries[] =
            {
                wxT( "/usr/bin/evince" ),
                wxT( "/usr/bin/gpdf" ),
                wxT( "/usr/bin/konqueror" ),
                wxT( "/usr/bin/kpdf" ),
                wxT( "/usr/bin/xpdf" ),
                wxT( "/usr/bin/open" ),     // BSD and OSX file & dir opener
                wxT( "/usr/bin/xdg-open" ), // Freedesktop file & dir opener
                wxT( "" ),
            };

            for( int ii = 0; ; ii++ )
            {
                if( tries[ii].IsEmpty() )
                    break;

                if( wxFileExists( tries[ii] ) )
                {
                    command = tries[ii] + wxT( " " ) + filename;
                    break;
                }
            }

#endif
        }
    }

    if( !command.IsEmpty() )
    {
        success = ProcessExecute( command );

        if( !success )
        {
            wxString msg = _( "Problem while running the PDF viewer" );
            msg << _( "\n command is " ) << command;
            DisplayError( NULL, msg );
        }
    }
    else
    {
        wxString msg = _( "Unable to find a PDF viewer for" );
        msg << wxT( " " ) << filename;
        DisplayError( NULL, msg );
        success = false;
    }

    return success;
}


void OpenFile( const wxString& file )
{
    wxString    command;
    wxString    filename = file;

    wxFileName  CurrentFileName( filename );
    wxString    ext, type;

    ext = CurrentFileName.GetExt();
    wxFileType* filetype = wxTheMimeTypesManager->GetFileTypeFromExtension( ext );

    bool        success = false;

    wxFileType::MessageParameters params( filename, type );

    if( filetype )
        success = filetype->GetOpenCommand( &command, params );

    delete filetype;

    if( success && !command.IsEmpty() )
        ProcessExecute( command );
}


wxString QuoteFullPath( wxFileName& fn, wxPathFormat format )
{
    return wxT( "\"" ) + fn.GetFullPath( format ) + wxT( "\"" );
}
