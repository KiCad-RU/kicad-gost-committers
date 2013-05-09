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
 * @file doc_component_index.cpp
 */

#include <common_doc_iface.h>
#include <doc_common.h>
#include <doc_component_index.h>

namespace GOST_DOC_GEN {

// returns false if error
bool CreateNewCompIndexDoc( COMPONENT_DB* aComponentDB,
                            COMMON_DOC_IFACE* aDocIface )
{
    COMPONENT_ARRAY singleVariantComponents;
    int             variant;
    wxString        str, refdes_prefix_reserved, var_str;
    wxArrayString   letter_digit_sets;
    bool            comps_absent;

    if ( !aDocIface->Connect() )
        return false;

    if ( !aDocIface->LoadDocument(
        GetResourceFile( wxT( "templates/CompIndexFirstSheet_template.odt" ) ) ) )
    {
        aDocIface->Disconnect();
        return false;
    }

    aDocIface->SelectTable( 0 );

    current_row     = 4;
    current_sheet   = 0;

    // fill 'design name' field
    aDocIface->PutCell( wxT( "B28" ), wxT( "\n" ) + aComponentDB->DesignName, 0 );
    // fill 'designation' field
    aDocIface->PutCell( wxT( "B27" ),
                        wxT( "\n" ) + aComponentDB->Designation + wxT( "ПЭ3" ), 0 );
    // fill 'first use' field
    aDocIface->PutCell( wxT( "B2" ), aComponentDB->Designation, 0 );

    // form the constant part of components index (not a set)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, 0, 0, wxT( "" ) );

    // process the constant part (not a set)
    ProcessSingleVariant( aDocIface,
                          &singleVariantComponents,
                          -1,
                          aComponentDB );

    // form the constant part of components index (a set)
    Form_a_set( aDocIface, aComponentDB, PARTTYPE_A_SET, 0, NULL );

    // form the variable part of components index
    if( aComponentDB->VariantsIndexes.GetCount() )
    {
        OO_PrintCompIndexDocRow( aDocIface, wxT( "" ), wxT( "" ), 0,
                                 wxT( "" ), 0, 7, aComponentDB );

        OO_PrintCompIndexDocRow( aDocIface, wxT( "" ),
                                 wxT( "Переменные данные для исполнений" ),
                                 0, wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );
    }

    for( int var_i = 0; var_i < (int) aComponentDB->VariantsIndexes.GetCount(); var_i++ )
    {
        comps_absent = true;
        // not a set
        variant = aComponentDB->VariantsIndexes[var_i];
        str     = aComponentDB->Designation;

        if( variant > 0 )
        {
            var_str = wxString::Format( wxT( "%d" ), variant );
            str += wxT( '-' );

            if( var_str.Len() == 1 )
                str += wxT( '0' );

            str += var_str;
        }

        OO_PrintCompIndexDocRow( aDocIface, wxT( "" ), wxT( "" ), 0,
                                 wxT( "" ), 0, 5, aComponentDB );

        OO_PrintCompIndexDocRow( aDocIface, wxT( "" ), str, 0,
                                 wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        str = aComponentDB->DesignName;

        if( variant > 0 )
        {
            str += wxT( " - " );

            if( var_str.Len() == 1 )
                str += wxT( '0' );

            str += var_str;
        }

        OO_PrintCompIndexDocRow( aDocIface, wxT( "" ), str, 0,
                                 wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_VAR, variant,
                                                 wxT( "" ) );

        if( singleVariantComponents.GetCount()>0 )
            comps_absent = false;

        ProcessSingleVariant( aDocIface,
                              &singleVariantComponents,
                              variant,
                              aComponentDB );

        // a set
        comps_absent &= ~Form_a_set( aDocIface,
                                     aComponentDB,
                                     PARTTYPE_VAR | PARTTYPE_A_SET,
                                     variant,
                                     NULL );

        if( comps_absent )
        {
            OO_PrintCompIndexDocRow( aDocIface, wxT( "" ), wxT( "" ), 0,
                                     wxT( "" ), 0, 1, aComponentDB );

            OO_PrintCompIndexDocRow( aDocIface, wxT( "" ),
                                     wxT( "Отсутствуют" ), 0, wxT( "" ), 0, 1, aComponentDB );
        }
    }


    // print the last sheet
    if ( !aDocIface->AppendDocument(
        GetResourceFile( wxT( "templates/CompIndexLastSheet_template.odt" ) ) ) )
    {
        aDocIface->Disconnect();
        return false;
    }

    current_sheet++;
    aDocIface->SelectTable( current_sheet );

    // fill 'sheet number' field
    aDocIface->PutCell( wxT( "C28.1.2" ),
                        wxT( "\n" ) + wxString::Format( wxT( "%d" ), current_sheet + 1 ),
                        0 );

    // fill 'designation' field
    aDocIface->PutCell( wxT( "B28" ),
                        wxT( "\n" ) + aComponentDB->Designation + wxT( "ПЭ3" ), 0 );

    // fill 'sheets qty' field
    aDocIface->SelectTable( 0 );
    aDocIface->PutCell( wxT( "C28.5.2" ),
                        wxString::Format( wxT( "%d" ), current_sheet + 1 ),
                        0 );

    aDocIface->Disconnect();

    return true;
}

} // namespace GOST_DOC_GEN
