/**
 * @file database.cpp
 */

#include "fctsys.h"
#include "gr_basic.h"
#include "macros.h"
#include "confirm.h"
#include "eda_doc.h"
#include "kicad_string.h"
#include "wxstruct.h"

#include "general.h"
#include "protos.h"
#include "class_library.h"
#include "dialog_helpers.h"

#include <boost/foreach.hpp>


/*
 * Routine name selection of a component library for loading,
 * Keys leading the list of the keywords filter
 * If Keys = "", research components that correspond
 * BufName mask (with * and?)
 *
 * Returns
 * true if the selected component
 * FALSE canceled order
 * Place the name of the component has loaded, select from a list in
 * BufName
 */
wxString DataBaseGetName( EDA_DRAW_FRAME* frame, wxString& Keys, wxString& BufName )
{
    wxArrayString  nameList;
    wxString       msg;

#ifndef KICAD_KEEPCASE
    BufName.MakeUpper();
#endif
    Keys.MakeUpper();

    /* Review the list of libraries for counting. */
    BOOST_FOREACH( CMP_LIBRARY& lib, CMP_LIBRARY::GetLibraryList() )
    {
        lib.SearchEntryNames( nameList, BufName, Keys );
    }

    if( nameList.IsEmpty() )
    {
        msg = _( "No components found matching " );

        if( !BufName.IsEmpty() )
        {
            msg += _( "name search criteria <" ) + BufName + wxT( "> " );

            if( !Keys.IsEmpty() )
                msg += _( "and " );
        }

        if( !Keys.IsEmpty() )
            msg += _( "key search criteria <" ) + Keys + wxT( "> " );

        DisplayError( frame, msg );

        return wxEmptyString;
    }

    // Show candidate list:
    wxString cmpname;
    EDA_LIST_DIALOG dlg( frame, _( "Select Component" ), nameList, cmpname, DisplayCmpDoc );

    if( dlg.ShowModal() != wxID_OK )
        return wxEmptyString;

    cmpname = dlg.GetTextSelection();
    return cmpname;
}


void DisplayCmpDoc( wxString& Name )
{
    LIB_ALIAS* CmpEntry = NULL;

    CmpEntry = CMP_LIBRARY::FindLibraryEntry( Name );

    if( CmpEntry == NULL )
        return;

    wxLogDebug( wxT( "Selected component <%s>, m_Doc: <%s>, m_KeyWord: <%s>." ),
                GetChars( Name ), GetChars( CmpEntry->GetDescription() ),
                GetChars( CmpEntry->GetKeyWords() ) );

    Name  = wxT( "Description: " ) + CmpEntry->GetDescription();
    Name += wxT( "\nKey Words: " ) + CmpEntry->GetKeyWords();
}
