/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2013 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2013 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file common_funcs.cpp
 */

#include <appl_wxstruct.h>
#include <common_funcs.h>

namespace GOST_DOC_GEN {

void StringInsert( wxString* aStr, wxString aIns, int aPos )
{
    *aStr = (*aStr).Left( aPos ) + aIns + (*aStr).Right( (*aStr).Len() - aPos );
}


int FindOneOf( wxString aScannedStr, wxString aTokens )
{
    size_t  i;
    int     pos;

    for( pos = 0; pos < (int)aScannedStr.Len(); pos++ )
    {
        for( i = 0; i < aTokens.Len(); i++ )
            if( aScannedStr[pos] == aTokens[i] )
                return pos;
    }

    return wxNOT_FOUND;
}


int FindFrom( wxString aScannedStr, wxString aToken, int aStart_pos )
{
    wxString tmp;

    if( aStart_pos < 0 )
        return wxNOT_FOUND;

    tmp = aScannedStr.Right( aScannedStr.Len() - aStart_pos );
    return tmp.Find( aToken );
}


bool DefineRefDesPrefix( wxString aIn, wxString* aResult )
{
    int pos;

    pos = FindOneOf( aIn, wxT( "0123456789" ) );

    if( pos==wxNOT_FOUND )
    {
        *aResult = wxT( "" );
        return false;
    }

    *aResult = aIn.Left( pos );
    return true;
}


long RefDesPostfix( wxString aIn )
{
    long res;
    int pos;

    pos = FindOneOf( aIn, wxT( "0123456789" ) );

    if( pos==wxNOT_FOUND )
        return -1;

    aIn.Mid( pos ).ToLong( &res, 10 );
    return res;
}


void SplitString( wxString aIn, wxArrayString* aResult, int aMax_len, int aSplit_ena )
{
    wxString    str, split_ena_str;
    int         cur_pos, i, separator_len;
    bool        next_str;

    aResult->Clear();
    cur_pos = 0;
    str     = wxT( "" );
    split_ena_str = wxT( "_@" );

    if( aSplit_ena & SPLIT_COMMA_ENA )
        split_ena_str += wxT( ',' );

    if( aSplit_ena & SPLIT_DOT_ENA )
        split_ena_str += wxT( '.' );

    while( aIn != wxT( "" ) )
    {
        next_str = false;
        i = FindOneOf( aIn, split_ena_str );

        if( i==wxNOT_FOUND )
        {
            i = aIn.Len();
            separator_len = 0;
        }
        else
        {
            if( aIn[i] == wxT( ',' ) && (aSplit_ena & SPLIT_COMMA_ENA) )
                separator_len = 1;
            else if( (aSplit_ena & SPLIT_DOT_ENA)
                     && ( (size_t) (i + 2) < aIn.Len() )
                     && aIn[i] == wxT( '.' )
                     && aIn[i + 1] == wxT( '.' )
                     && aIn[i + 2] == wxT( '.' ) )
                separator_len = 3;
            else if( aIn[i] == wxT( '.' )
                     && (aSplit_ena & SPLIT_DOT_ENA) )
                separator_len = 1;
            else if( aIn[i] == wxT( '_' ) )
                separator_len = 1;
            else    // '@'
            {
                next_str = true;
                separator_len = 0;
                aIn.Remove( i, 1 );
            }
        }

        if( (int) str.Len() + i + separator_len > aMax_len )
        {
            str.Replace( wxT( "_" ), wxT( " " ) );
            str.Trim( true );
            aResult->Add( str );
            str = wxT( "" );
        }
        else
        {
            str += aIn.Left( i + separator_len );
            aIn.Remove( 0, i + separator_len );

            if( next_str )
            {
                str.Replace( wxT( "_" ), wxT( " " ) );
                str.Trim( true );
                aResult->Add( str );
                str = wxT( "" );
            }
        }
    }

    if( str != wxT( "" ) )
    {
        str.Replace( wxT( "_" ), wxT( " " ) );
        str.Trim( true );
        aResult->Add( str );
    }
}


void ExtractLetterDigitSets( wxString aIn_str, wxArrayString* aSets )
{
    int i, dig_start, dig_end;

    aSets->Clear();

    while( aIn_str != wxT( "" ) )
    {
        dig_start   = FindOneOf( aIn_str, wxT( "0123456789" ) );
        dig_end     = aIn_str.Len();

        if( dig_start != wxNOT_FOUND )
        {
            i = dig_start;

            for( ; (size_t) i < aIn_str.Len(); i++ )
            {
                if( aIn_str[i] < wxT( '0' ) || aIn_str[i] > wxT( '9' ) )
                {
                    dig_end = i;
                    break;
                }
            }
        }

        aSets->Add( aIn_str.Left( dig_end ) );
        aIn_str.Remove( 0, dig_end );
    }
}


bool DoesStringExist( wxArrayString* aString_array, wxString aStr )
{
    size_t i;

    for( i = 0; i<(*aString_array).GetCount(); i++ )
        if( (*aString_array)[i] == aStr )
            return true;

    return false;
}


void FormRefDesFromStringArray( wxArrayString* aString_array, wxString* aResult )
{
    wxString    subgroup, base_refdes_prefix, next_refdes_prefix;
    int         base_refdes_postfix, next_refdes_postfix, cur_pos, qty, i, end_item;

    cur_pos     = 0;
    end_item    = aString_array->GetCount() - 1;

    if( end_item==-1 )
    {
        *aResult = wxT( "" );
        return;
    }

    if( cur_pos==end_item )
    {
        *aResult = (*aString_array)[cur_pos];
        return;
    }

    *aResult = wxT( "" );

    while( cur_pos <= end_item )
    {
        base_refdes_postfix = RefDesPostfix( (*aString_array)[cur_pos] );
        DefineRefDesPrefix( (*aString_array)[cur_pos], &base_refdes_prefix );
        qty = 1;

        if( cur_pos < end_item )
        {
            for( i = cur_pos; i<end_item; i++ )
            {
                next_refdes_postfix = RefDesPostfix( (*aString_array)[i + 1] );
                DefineRefDesPrefix( (*aString_array)[i + 1], &next_refdes_prefix );

                if( (base_refdes_postfix!=next_refdes_postfix - 1)
                    || (base_refdes_prefix!=next_refdes_prefix) )
                    break;

                base_refdes_postfix = next_refdes_postfix;
                qty++;
            }
        }

        if( qty==1 )
            (*aResult) += (*aString_array)[cur_pos] + wxT( "," );
        else if( qty==2 )
            (*aResult) += (*aString_array)[cur_pos] + wxT( "," )
                      + (*aString_array)[cur_pos + 1] + wxT( "," );
        else
            (*aResult) += (*aString_array)[cur_pos] + wxT( "..." ) +
                         (*aString_array)[cur_pos + qty - 1] + wxT( "," );

        cur_pos += qty;
    }

    (*aResult).Remove( (*aResult).Len() - 1, 1 );    // delete last comma
}


void SortCByteArray( INT_ARRAY* aArr )
{
    int i, j, item_qty = aArr->GetCount();
    int tmp;

    for( i = 0; i < item_qty - 1; i++ )
    {
        // compare two neighbour values
        for( j = 0; j < item_qty - i - 1; j++ )
        {
            if( (*aArr)[j] > (*aArr)[j + 1] )
            {
                // if the values are placed out of order then they are swapped
                tmp = (*aArr)[j];
                (*aArr)[j] = (*aArr)[j + 1];
                (*aArr)[j + 1] = tmp;
            }
        }
    }
}


wxString GetResourceFile( wxString aFileName )
{
    wxArrayString subdirs;
    wxString res;

    subdirs.Add( wxT( "share" ) );

#ifndef __WXMSW__
    /* Up on level relative to binary path with "share/kicad" appended for
     * all other platforms. */
    subdirs.Add( wxT( "kicad" ) );
#endif

    subdirs.Add( wxT( "GOST-doc-gen" ) );

    res = wxGetApp().FindFileInSearchPaths( aFileName, &subdirs );

    if( res == wxEmptyString )
    {
        wxMessageBox( wxT( "Unable to open file: " ) + aFileName,
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
    }

    return res;
}


wxString AddUrlPrefix( wxString aFileName )
{
#ifdef __WXMSW__
    return wxT( "file:///" ) + aFileName;
#else
    return wxT( "file://" ) + aFileName;
#endif
}

#ifdef __WXMSW__
wxString FindWindowsOOInstallationPath()
{
    wxString fullPath;
    wxString sofficeExe( wxT( "soffice.exe" ) );

    for( char letter = 'C'; letter <= 'H'; letter++ )
    {
        for( char ver = '2'; ver <= '4'; ver++ )
        {
            fullPath = wxString( letter ) + wxT( ":\\Program Files (x86)\\OpenOffice.org " ) +
                       wxString( ver ) + wxT( "\\program\\" );
            if( wxFileExists( fullPath + sofficeExe ) )
                return fullPath;

            fullPath = wxString( letter ) + wxT( ":\\Program Files\\OpenOffice.org " ) +
                       wxString( ver ) + wxT( "\\program\\" );
            if( wxFileExists( fullPath + sofficeExe ) )
                return fullPath;

            fullPath = wxString( letter ) + wxT( ":\\Program Files (x86)\\LibreOffice " ) +
                       wxString( ver ) + wxT( ".0\\program\\" );
            if( wxFileExists( fullPath + sofficeExe ) )
                return fullPath;

            fullPath = wxString( letter ) + wxT( ":\\Program Files\\LibreOffice " ) +
                       wxString( ver ) + wxT( ".0\\program\\" );
            if( wxFileExists( fullPath + sofficeExe ) )
                return fullPath;
        }
    }

    wxMessageBox( wxT( "Unable to find OpenOffice or LibreOffice installation path" ),
                  wxEmptyString,
                  wxOK | wxICON_ERROR );

    return wxEmptyString;
}
#endif  // __WXMSW__

} // namespace GOST_DOC_GEN
