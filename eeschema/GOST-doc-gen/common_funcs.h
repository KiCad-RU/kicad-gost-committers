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
 * @file common_funcs.h
 */

#ifndef COMMONFUNCS_H
#define COMMONFUNCS_H

#include <wx/wx.h>

namespace GOST_DOC_GEN {

#define SPLIT_COMMA_ENA 0x0001
#define SPLIT_DOT_ENA   0x0002

WX_DEFINE_ARRAY_INT( int, INT_ARRAY );

int      FindOneOf( wxString aScannedStr, wxString aTokens );
int      FindFrom( wxString aScannedStr, wxString aToken, int aStart_pos );
bool     DefineRefDesPrefix( wxString aIn, wxString* aResult );
long     RefDesPostfix( wxString aIn );
void     SplitString( wxString aIn, wxArrayString* aResult, int aMax_len, int aSplit_ena );
void     ExtractLetterDigitSets( wxString aIn_str, wxArrayString* aSets );
bool     DoesStringExist( wxArrayString* aString_array, wxString aStr );
void     FormRefDesFromStringArray( wxArrayString* aString_array, wxString* aResult );
void     SortCByteArray( INT_ARRAY* aArr );
void     StringInsert( wxString* aStr, wxString aIns, int aPos );
wxString GetResourceFile( wxString aFileName );

} // namespace GOST_DOC_GEN

#endif    // COMMONFUNCS_H
