
#include <macros.h>
#include <search_stack.h>
#include <wx/tokenzr.h>


#if defined(__MINGW32__)
 #define PATH_SEPS          wxT(";\r\n")
#else
 #define PATH_SEPS          wxT(":;\r\n")       // unix == linux | mac
#endif


wxString SEARCH_STACK::FilenameWithRelativePathInSearchList( const wxString& aFullFilename )
{
    /* If the library path is already in the library search paths
     * list, just add the library name to the list.  Otherwise, add
     * the library name with the full or relative path.
     * the relative path, when possible is preferable,
     * because it preserve use of default libraries paths, when the path is a sub path of
     * these default paths
     * Note we accept only sub paths,
     * not relative paths starting by ../ that are not subpaths and are outside kicad libs paths
     */
    wxFileName fn = aFullFilename;
    wxString   filename = aFullFilename;

    unsigned   pathlen  = fn.GetPath().Len();   // path len, used to find the better (shortest)
                                                // subpath within defaults paths

    for( unsigned kk = 0; kk < GetCount(); kk++ )
    {
        fn = aFullFilename;

        // Search for the shortest subpath within 'this':
        if( fn.MakeRelativeTo( (*this)[kk] ) )
        {
            if( fn.GetPathWithSep().StartsWith( wxT("..") ) )  // Path outside kicad libs paths
                continue;

            if( pathlen > fn.GetPath().Len() )    // A better (shortest) subpath is found
            {
                filename = fn.GetPathWithSep() + fn.GetFullName();
                pathlen  = fn.GetPath().Len();
            }
        }
    }

    return filename;
}


void SEARCH_STACK::RemovePaths( const wxString& aPaths )
{
    wxStringTokenizer tokenizer( aPaths, PATH_SEPS, wxTOKEN_STRTOK );

    while( tokenizer.HasMoreTokens() )
    {
        wxString path = tokenizer.GetNextToken();

        if( Index( path, wxFileName::IsCaseSensitive() ) != wxNOT_FOUND )
        {
            Remove( path );
        }
    }
}


void SEARCH_STACK::AddPaths( const wxString& aPaths, int aIndex )
{
    bool                isCS = wxFileName::IsCaseSensitive();
    wxStringTokenizer   tokenizer( aPaths, PATH_SEPS, wxTOKEN_STRTOK );

    // appending all of them, on large or negative aIndex
    if( unsigned( aIndex ) >= GetCount() )
    {
        while( tokenizer.HasMoreTokens() )
        {
            wxString path = tokenizer.GetNextToken();

            if( wxFileName::IsDirReadable( path )
                && Index( path, isCS ) == wxNOT_FOUND )
            {
                Add( path );
            }
        }
    }

    // inserting all of them:
    else
    {
        while( tokenizer.HasMoreTokens() )
        {
            wxString path = tokenizer.GetNextToken();

            if( wxFileName::IsDirReadable( path )
                && Index( path, isCS ) == wxNOT_FOUND )
            {
                Insert( path, aIndex );
                aIndex++;
            }
        }
    }
}


const wxString SEARCH_STACK::LastVisitedPath( const wxString& aSubPathToSearch )
{
    wxString path;

    // Initialize default path to the main default lib path
    // this is the second path in list (the first is the project path).
    unsigned pcount = GetCount();

    if( pcount )
    {
        unsigned ipath = 0;

        if( (*this)[0] == wxGetCwd() )
            ipath = 1;

        // First choice of path:
        if( ipath < pcount )
            path = (*this)[ipath];

        // Search a sub path matching this SEARCH_PATH
        if( !IsEmpty() )
        {
            for( ; ipath < pcount; ipath++ )
            {
                if( (*this)[ipath].Contains( aSubPathToSearch ) )
                {
                    path = (*this)[ipath];
                    break;
                }
            }
        }
    }

    if( path.IsEmpty() )
        path = wxGetCwd();

    return path;
}


#if defined(DEBUG)
void SEARCH_STACK::Show( const char* aPrefix ) const
{
    printf( "%s SEARCH_STACK:\n", aPrefix );
    for( unsigned i=0;  i<GetCount();  ++i )
    {
        printf( "  [%2i]:%s\n", i, TO_UTF8( (*this)[i] ) );
    }
}
#endif
