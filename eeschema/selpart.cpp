/**
 * @file selpart.cpp
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <confirm.h>
#include <wxstruct.h>

#include <general.h>
#include <protos.h>
#include <class_library.h>
#include <dialog_helpers.h>


CMP_LIBRARY* SelectLibraryFromList( EDA_DRAW_FRAME* frame )
{
    static wxString OldLibName;
    wxArrayString   libNamesList;
    CMP_LIBRARY*    Lib = NULL;

    int count = CMP_LIBRARY::GetLibraryCount();
    if( count == 0 )
    {
        DisplayError( frame, _( "No component libraries are loaded." ) );
        return NULL;
    }

    libNamesList = CMP_LIBRARY::GetLibraryNames();

    EDA_LIST_DIALOG dlg( frame, _( "Select Library" ), libNamesList, OldLibName );

    if( dlg.ShowModal() != wxID_OK )
        return NULL;

    wxString libname = dlg.GetTextSelection();

    if( libname.IsEmpty() )
        return NULL;

    Lib = CMP_LIBRARY::FindLibrary( libname );

    if( Lib != NULL )
        OldLibName = libname;

    return Lib;
}

extern void DisplayCmpDocAndKeywords( wxString& Name );

int DisplayComponentsNamesInLib( EDA_DRAW_FRAME* frame,
                                 CMP_LIBRARY* Library,
                                 wxString& Buffer, wxString& OldName )
{
    wxArrayString  nameList;

    if( Library == NULL )
        Library = SelectLibraryFromList( frame );

    if( Library == NULL )
        return 0;

    Library->GetEntryNames( nameList );

    EDA_LIST_DIALOG dlg( frame, _( "Select Component" ), nameList, OldName, DisplayCmpDocAndKeywords );

    if( dlg.ShowModal() != wxID_OK )
        return 0;

    Buffer = dlg.GetTextSelection();

    return 1;
}


int GetNameOfPartToLoad( EDA_DRAW_FRAME* frame, CMP_LIBRARY* Library, wxString& BufName )
{
    int             ii;
    static wxString OldCmpName;

    ii = DisplayComponentsNamesInLib( frame, Library, BufName, OldCmpName );
    if( ii <= 0 || BufName.IsEmpty() )
        return 0;

    OldCmpName = BufName;
    return 1;
}
