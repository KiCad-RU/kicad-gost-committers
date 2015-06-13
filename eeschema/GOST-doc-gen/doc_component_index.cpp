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

#include <wx/progdlg.h>

#include <common_doc_iface.h>
#include <doc_common.h>
#include <doc_component_index.h>

namespace GOST_DOC_GEN {

// returns false if error
bool DOC_COMPONENT_INDEX::CreateNewCompIndexDoc( COMPONENT_DB* aComponentDB,
                                                 COMMON_DOC_IFACE* aDocIface )
{
    COMPONENT_ARRAY singleVariantComponents;
    int             variant;
    wxString        str, refdes_prefix_reserved, var_str, fileName;
    wxArrayString   letter_digit_sets;
    bool            comps_absent;

    wxProgressDialog progressDlg( _( "Generating component index document..." ),
                                  _( "Please wait a moment" ), 1 );

    if( !aDocIface->Connect() )
        return false;

    progressDlg.Pulse();

    if( ( fileName = GetResourceFile( wxT( "templates/CompIndexFirstSheet_template.odt" ) ) )
        == wxEmptyString
        || !aDocIface->LoadDocument( fileName ) )
    {
        aDocIface->Disconnect();
        return false;
    }

    aDocIface->SelectTable( 0 );

    m_current_row     = 4;
    m_current_sheet   = 0;

    // fill 'design name' field
    aDocIface->PutCell( ADDR_DESIGN_NAME,
                        aComponentDB->m_designName + wxT( "\nПеречень элементов" ),
                        0 );

    // fill 'designation' field
    aDocIface->PutCell( ADDR_DESIGNATION,
                        ChangeSuffixOfDesignation( aComponentDB->m_designation, wxT( "П" ), true ), 0 );

    // fill 'first use' field
    aDocIface->PutCell( ADDR_FIRST_USE, aComponentDB->m_designation, 0 );

    // fill 'developer' field
    aDocIface->PutCell( ADDR_DEVELOPER, aComponentDB->m_developerField, 0 );

    // fill 'verifier' field
    aDocIface->PutCell( ADDR_VERIFIER, aComponentDB->m_verifierField, 0 );

    // fill 'approver' field
    aDocIface->PutCell( ADDR_APPROVER, aComponentDB->m_approverField, 0 );

    // fill 'company' field
    aDocIface->PutCell( ADDR_COMPANY, aComponentDB->m_companyName, 0 );

    // form the constant part of components index (not a set)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, 0, 0, wxT( "" ) );

    progressDlg.Pulse();

    // process the constant part (not a set)
    ProcessSingleVariant( aDocIface,
                          &singleVariantComponents,
                          -1,
                          aComponentDB );

    progressDlg.Pulse();

    // form the constant part of components index (a set)
    Form_a_set( aDocIface, aComponentDB, PARTTYPE_A_SET, 0, NULL );

    progressDlg.Pulse();

    // form the variable part of components index
    if( aComponentDB->m_variantIndexes.GetCount() )
    {
        OO_PrintCompIndexDocRow( aDocIface, wxT( "" ), wxT( "" ), 0,
                                 wxT( "" ), 0, 7, aComponentDB );

        OO_PrintCompIndexDocRow( aDocIface, wxT( "" ),
                                 wxT( "Переменные данные для исполнений" ),
                                 0, wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );
    }

    for( int var_i = 0; var_i < (int) aComponentDB->m_variantIndexes.GetCount(); var_i++ )
    {
        comps_absent = true;
        // not a set
        variant = aComponentDB->m_variantIndexes[var_i];
        str     = aComponentDB->m_designation;

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

        str = aComponentDB->m_designName;

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

        progressDlg.Pulse();

        if( singleVariantComponents.GetCount()>0 )
            comps_absent = false;

        ProcessSingleVariant( aDocIface,
                              &singleVariantComponents,
                              variant,
                              aComponentDB );

        progressDlg.Pulse();

        // a set
        comps_absent &= ~Form_a_set( aDocIface,
                                     aComponentDB,
                                     PARTTYPE_VAR | PARTTYPE_A_SET,
                                     variant,
                                     NULL );

        progressDlg.Pulse();

        if( comps_absent )
        {
            OO_PrintCompIndexDocRow( aDocIface, wxT( "" ), wxT( "" ), 0,
                                     wxT( "" ), 0, 1, aComponentDB );

            OO_PrintCompIndexDocRow( aDocIface, wxT( "" ),
                                     wxT( "Отсутствуют" ), 0, wxT( "" ), 0, 1, aComponentDB );
        }
    }


    // print the last sheet
    if( ( fileName = GetResourceFile( wxT( "templates/CompIndexLastSheet_template.odt" ) ) )
        == wxEmptyString
        || !aDocIface->AppendDocument( fileName ) )
    {
        aDocIface->Disconnect();
        return false;
    }

    progressDlg.Pulse();

    m_current_sheet++;
    aDocIface->SelectTable( m_current_sheet );

    // fill 'sheet number' field
    aDocIface->PutCell( ADDR_LASTSHEET_NUMBER,
                        wxString::Format( wxT( "%d" ), m_current_sheet + 1 ),
                        0 );

    // fill 'designation' field
    aDocIface->PutCell( ADDR_LASTSHEET_DESIGNATION,
                        ChangeSuffixOfDesignation( aComponentDB->m_designation, wxT( "П" ), true ), 0 );

    // fill 'sheets qty' field
    aDocIface->SelectTable( 0 );
    aDocIface->PutCell( ADDR_SHEET_QTY,
                        wxString::Format( wxT( "%d" ), m_current_sheet + 1 ),
                        0 );

    aDocIface->Disconnect();

    return true;
}

} // namespace GOST_DOC_GEN
