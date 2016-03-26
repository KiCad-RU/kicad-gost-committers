/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2013 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2013-2015 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file doc_specification.cpp
 */

#include <wx/progdlg.h>

#include <common_doc_iface.h>
#include <doc_common.h>
#include <doc_specification.h>

namespace GOST_DOC_GEN {

// returns true if both components have identical attributes except ref des
bool DOC_SPECIFICATION::CompareCompPos( TCOMPONENT_ATTRS* aComp1, TCOMPONENT_ATTRS* aComp2 )
{
    for( int i = 0; i < ATTR_QTY; i++ )
    {
        if( aComp1->attrs[i] != aComp2->attrs[i] )
            return false;
    }

    return true;
}


void DOC_SPECIFICATION::SpecPosList_ExtractPartOfDB( COMPONENT_DB*             aComponentDB,
                                                     COMPONENT_ARRAY*          aAll_components,
                                                     TCOMPONENT_ATTRS_ARRAY*   aAllCompPositions,
                                                     int                       aType )
{
    COMPONENT* pComponent;
    pTCOMPONENT_ATTRS componentAttrs;
    int  i, item_var_i, CompPos_i;
    bool found;

    aAllCompPositions->Clear();

    for( i = 0; i < (int) aAll_components->GetCount(); i++ )
    {
        pComponent = (COMPONENT*) (*aAll_components)[i];

        if( ( aType & PARTTYPE_ALL_DETAILS )
            || ( ( aType & PARTTYPE_ASSEMBLY_UNITS )
                 && ( pComponent->ComponentType==wxT( "AssemblyUnit" ) ) )
            || ( ( aType & PARTTYPE_DETAILS ) && ( pComponent->ComponentType==wxT( "Detail" ) ) )
            || ( (aType & PARTTYPE_STANDARD_DETAILS)
                 && ( pComponent->ComponentType==wxT( "StandardDetail" ) ) )
            || ( ( aType & PARTTYPE_GOODS ) && ( pComponent->ComponentType==wxT( "Goods" ) ) )
            || ( ( !aType ) && !( pComponent->ComponentType==wxT( "Documentation" ) )
                 && !( pComponent->ComponentType==wxT( "AssemblyUnit" ) )
                 && !( pComponent->ComponentType==wxT( "StandardDetail" ) )
                 && !( pComponent->ComponentType==wxT( "Detail" ) )
                 && !( pComponent->ComponentType==wxT( "Goods" ) ) ) )
        {
            for( item_var_i = 0;
                 item_var_i < (int) pComponent->m_comp_attr_variants.GetCount();
                 item_var_i++ )
            {
                componentAttrs = (pTCOMPONENT_ATTRS) pComponent->m_comp_attr_variants[item_var_i];

                if( componentAttrs->attrs[ATTR_NAME] != wxT( "" )
                    && componentAttrs->attrs[ATTR_NOTE] != aComponentDB->m_notInstalledStr )
                {
                    found = false;

                    for( CompPos_i = 0;
                         CompPos_i < (int) aAllCompPositions->GetCount();
                         CompPos_i++ )
                    {
                        if( CompareCompPos( (pTCOMPONENT_ATTRS) (*aAllCompPositions)[CompPos_i],
                                            componentAttrs ) )
                        {
                            found = true;
                            break;
                        }
                    }

                    if( !found )
                        aAllCompPositions->Add( pComponent->m_comp_attr_variants[item_var_i] );
                }
            }
        }
    }
}


void DOC_SPECIFICATION::Specification_GeneratePosList( COMPONENT_DB*    aComponentDB,
                                                       COMPONENT_ARRAY* aAll_components,
                                                       wxArrayString*   aSpecification_positions )
{
    wxString                str;
    TCOMPONENT_ATTRS_ARRAY  allCompPositions;

    aSpecification_positions->Clear();

    // form all component positions list (assembly units)
    SpecPosList_ExtractPartOfDB( aComponentDB, aAll_components,
                                 &allCompPositions, PARTTYPE_ASSEMBLY_UNITS );
    Specification_ProcessSingleVariant( NULL, (wxArrayPtrVoid*) &allCompPositions, -1, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aComponentDB, aAll_components,
                                 &allCompPositions, PARTTYPE_DETAILS );

    Specification_ProcessSingleVariant( NULL, (wxArrayPtrVoid*) &allCompPositions, -1, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aComponentDB, aAll_components,
                                 &allCompPositions, PARTTYPE_STANDARD_DETAILS );

    Specification_ProcessSingleVariant( NULL, (wxArrayPtrVoid*) &allCompPositions, -1, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aComponentDB, aAll_components, &allCompPositions, 0 );

    Specification_ProcessSingleVariant( NULL, (wxArrayPtrVoid*) &allCompPositions, -1, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aComponentDB, aAll_components,
                                 &allCompPositions, PARTTYPE_GOODS );

    Specification_ProcessSingleVariant( NULL, (wxArrayPtrVoid*) &allCompPositions, -1, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );
}


// returns false if error
bool DOC_SPECIFICATION::CreateNewSpecificationDoc( COMPONENT_DB* aComponentDB,
                                                   COMMON_DOC_IFACE* aDocIface )
{
    COMPONENT_ARRAY singleVariantComponents, AllVariantsComponents;
    int             variant, idx;
    wxString        str, var_str, fileName;
    wxArrayString   specification_positions;
    bool            comps_absent;
    wxString        designation;
    wxString        sch_type = wxT( "Схема электрическая_" );

    wxProgressDialog progressDlg( _( "Generating specification document..." ),
                                  _( "Please wait a moment" ), 1 );

    if ( !aDocIface->Connect() )
        return false;

    progressDlg.Pulse();

    if( ( fileName = GetResourceFile( wxT( "templates/SpecificationFirstSheet_template.odt" ) ) )
        == wxEmptyString
        || !aDocIface->LoadDocument( fileName ) )
    {
        aDocIface->Disconnect();
        return false;
    }

    progressDlg.Pulse();

    m_current_row = 4;
    m_current_sheet = 0;
    m_specification_pos_field = 1;

    // Generate positions list
    Specification_GeneratePosList( aComponentDB, &aComponentDB->m_AllComponents,
                                   &specification_positions );

    // check type of the schematic by designation ending
    if( aComponentDB->m_designation != wxEmptyString )
    {
        idx = aComponentDB->m_designation.Find( wxT( 'Э' ), true);
        if( idx != wxNOT_FOUND && idx > 3 )
        {
            designation = aComponentDB->m_designation;
            switch( designation[ idx + 1 ].GetValue() )
            {
            case '1':
                sch_type += wxT( "структурная" );
                break;
            case '2':
                sch_type += wxT( "функциональная" );
                break;
            case '3':
                sch_type += wxT( "принципиальная" );
                break;
            case '4':
                sch_type += wxT( "соединений" );
                break;
            case '5':
                sch_type += wxT( "подключения" );
                break;
            case '6':
                sch_type += wxT( "общая" );
                break;
            case '7':
                sch_type += wxT( "расположения" );
                break;
            }
        }
    }

    if( designation == wxEmptyString )
    {
        // by default
        if( aComponentDB->m_designation != wxEmptyString )
            designation = aComponentDB->m_designation.Trim() + wxT( " Э3" );
        sch_type += wxT( "принципиальная" );

    }

    progressDlg.Pulse();

    aDocIface->SelectTable( 0 );

    // fill 'design name' field
    aDocIface->PutCell( ADDR_DESIGN_NAME, aComponentDB->m_designName, 0 );
    // fill 'designation' field
    aDocIface->PutCell( ADDR_DESIGNATION,
                        ChangeSuffixOfDesignation( designation ),
                        0 );
    // fill 'first use' field
    aDocIface->PutCell( ADDR_FIRST_USE, aComponentDB->m_specFirstUse, 0 );

    // fill 'developer' field
    aDocIface->PutCell( ADDR_DEVELOPER, aComponentDB->m_developerField, 0 );

    // fill 'verifier' field
    aDocIface->PutCell( ADDR_VERIFIER, aComponentDB->m_verifierField, 0 );

    // fill 'approver' field
    aDocIface->PutCell( ADDR_APPROVER, aComponentDB->m_approverField, 0 );

    // fill 'company' field
    aDocIface->PutCell( ADDR_COMPANY, aComponentDB->m_companyName, 0 );

    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );
    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "Документация" ), wxT( "" ), wxT( "" ),
                                 TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

    // form the constant part of specification (documents)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_DOCUMENTATION, 0, wxT( "" ) );

    progressDlg.Pulse();

    if( singleVariantComponents.GetCount() )
    {
        // process the constant part (documents)
        Specification_ProcessSingleVariant( aDocIface,
                                            (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            aComponentDB,
                                            &specification_positions,
                                            SPEC_NO_SORTING );
        progressDlg.Pulse();
    }
    else
    {
        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface,
                                     aComponentDB->m_assemblyDrawingFmt,
                                     0,
                                     ChangeSuffixOfDesignation( designation, wxT( " СБ" ) ),
                                     wxT( "Сборочный чертеж" ),
                                     wxT( "" ),
                                     wxT( "" ),
                                     0,
                                     1,
                                     aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        if( aComponentDB->m_circuitDrawingFmt.Len() <= 2 )
            OO_PrintSpecificationDocRow( aDocIface,
                                         aComponentDB->m_circuitDrawingFmt,
                                         0,
                                         designation,
                                         sch_type,
                                         wxT( "" ),
                                         wxT( "" ),
                                         0,
                                         1,
                                         aComponentDB );
        else
            OO_PrintSpecificationDocRow( aDocIface, wxT( "*" ), 0,
                                         designation,
                                         sch_type,
                                         wxT( "" ),
                                         wxT( "*) " ) + aComponentDB->m_circuitDrawingFmt, 0, 1,
                                         aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface,
                                     wxT( "A4" ), 0,
                                     ChangeSuffixOfDesignation( designation, wxT( "П" ), true ),
                                     wxT( "Перечень элементов" ), wxT( "" ),
                                     wxT( "" ), 0, 1, aComponentDB );
    }

    // form the constant part of specification (assembly units)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_ASSEMBLY_UNITS, 0, wxT( "" ) );

    progressDlg.Pulse();

    if( singleVariantComponents.GetCount() > 0 )
    {
        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Сборочные единицы" ), wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        // process the constant part (assembly units)
        Specification_ProcessSingleVariant( aDocIface,
                                            (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            aComponentDB,
                                            &specification_positions,
                                            0 );

        progressDlg.Pulse();
    }


    // form the constant part of specification (details)
    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "Детали" ), wxT( "" ), wxT( "" ),
                                 TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_DETAILS, 0, wxT( "" ) );

    progressDlg.Pulse();

    if( singleVariantComponents.GetCount() > 0 )
    {
        // process the constant part (details)
        Specification_ProcessSingleVariant( aDocIface,
                                            (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            aComponentDB,
                                            &specification_positions,
                                            0 );
        progressDlg.Pulse();
    }
    else
    {
        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface,
                                     wxT( "" ), m_specification_pos_field++,
                                     aComponentDB->m_PCBDesignation,
                                     wxT( "Плата печатная" ), wxT( "1" ), wxT(
                                         "" ), 0, 1, aComponentDB );
    }


    // form the constant part of specification (standard details)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_STANDARD_DETAILS,
                                             0, wxT( "" ) );

    progressDlg.Pulse();

    if( singleVariantComponents.GetCount() > 0 )
    {
        m_current_row = SECOND_SHEET_LAST_STR_I;
        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Стандартные изделия" ), wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        // process the constant part (standard details)
        Specification_ProcessSingleVariant( aDocIface,
                                            (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            aComponentDB, &specification_positions, 0 );
        progressDlg.Pulse();
    }

    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 5, aComponentDB );

    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

    OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                 wxT( "Прочие изделия" ), wxT( "" ), wxT( "" ),
                                 TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

    // form the constant part of specification (not a set)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION, 0,
                                             wxT( "" ) );
    progressDlg.Pulse();

    // process the constant part
    Specification_ProcessSingleVariant( aDocIface,
                                        (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                        aComponentDB, &specification_positions, 0 );
    progressDlg.Pulse();

    // form the constant part of specification (a set)
    Form_a_set( aDocIface, aComponentDB, PARTTYPE_SPECIFICATION | PARTTYPE_A_SET, 0,
                &specification_positions );

    progressDlg.Pulse();

    // form the constant part of specification (goods)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_GOODS, 0, wxT( "" ) );

    progressDlg.Pulse();

    if( singleVariantComponents.GetCount() > 0 )
    {
        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Материалы" ), wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        // process the constant part (goods)
        Specification_ProcessSingleVariant( aDocIface,
                                            (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            aComponentDB,
                                            &specification_positions,
                                            0 );
        progressDlg.Pulse();
    }


    // form the variable part of specification
    if( aComponentDB->m_variantIndexes.GetCount() )
    {
        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 10, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Переменные данные_для исполнений" ), wxT( "" ),
                                     wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );
    }

    for( int var_i = 0; var_i < (int) aComponentDB->m_variantIndexes.GetCount(); var_i++ )
    {
        comps_absent = true;

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

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 7, aComponentDB );

        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                     wxT( "" ), str, wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        str = aComponentDB->m_designName;

        if( variant > 0 )
        {
            str += wxT( " - " );

            if( var_str.Len() == 1 )
                str += wxT( '0' );

            str += var_str;
        }

        str.Replace( wxT( " " ), wxT( "_" ) );
        OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                     wxT( "" ), str, wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );


        // form the variable part of specification (assembly units)
        aComponentDB->ExtractPartOfComponentsDB(
            &singleVariantComponents,
            PARTTYPE_SPECIFICATION | PARTTYPE_VAR |
            PARTTYPE_ASSEMBLY_UNITS, variant, wxT( "" ) );

        progressDlg.Pulse();

        if( singleVariantComponents.GetCount() > 0 )
        {
            comps_absent = false;
            OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                         wxT( "" ), wxT( "" ), wxT( "" ),
                                         wxT( "" ), 0, 4, aComponentDB );

            OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                         wxT( "" ), wxT( "Сборочные единицы" ), wxT( "" ), wxT( "" ),
                                         TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

            // process the variable part (assembly units)
            Specification_ProcessSingleVariant( aDocIface,
                                                (wxArrayPtrVoid*) &singleVariantComponents,
                                                variant,
                                                aComponentDB,
                                                &specification_positions,
                                                0 );
            progressDlg.Pulse();
        }

        // form the variable part of specification (details)
        aComponentDB->ExtractPartOfComponentsDB(
            &singleVariantComponents,
            PARTTYPE_SPECIFICATION | PARTTYPE_VAR |
            PARTTYPE_DETAILS, variant, wxT( "" ) );

        progressDlg.Pulse();

        if( singleVariantComponents.GetCount() > 0 )
        {
            comps_absent = false;
            OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                         wxT( "" ), wxT( "" ), wxT( "" ),
                                         wxT( "" ), 0, 4, aComponentDB );

            OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                         wxT( "" ), wxT( "Детали" ), wxT( "" ), wxT( "" ),
                                         TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

            // process the variable part (details)
            Specification_ProcessSingleVariant( aDocIface,
                                                (wxArrayPtrVoid*) &singleVariantComponents,
                                                variant,
                                                aComponentDB,
                                                &specification_positions,
                                                0 );
            progressDlg.Pulse();
        }

        // form the variable part of specification (not a set)
        aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents,
                                                 PARTTYPE_SPECIFICATION | PARTTYPE_VAR, variant,
                                                 wxT( "" ) );
        progressDlg.Pulse();

        if( singleVariantComponents.GetCount() > 0 )
            comps_absent = false;

        Specification_ProcessSingleVariant( aDocIface,
                                            (wxArrayPtrVoid*) &singleVariantComponents, variant,
                                            aComponentDB,
                                            &specification_positions,
                                            0 );
        progressDlg.Pulse();

        // form the variable part of specification (a set)
        comps_absent &= ~Form_a_set( aDocIface,
                                     aComponentDB,
                                     PARTTYPE_SPECIFICATION | PARTTYPE_VAR | PARTTYPE_A_SET,
                                     variant,
                                     &specification_positions );
        progressDlg.Pulse();

        if( comps_absent )
        {
            OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                         wxT( "" ), wxT( "" ), wxT( "" ),
                                         wxT( "" ), 0, 1, aComponentDB );

            OO_PrintSpecificationDocRow( aDocIface, wxT( "" ), 0,
                                         wxT( "" ), wxT( "Отсутствуют" ), wxT( "" ),
                                         wxT( "" ), 0, 1, aComponentDB );
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
                        ChangeSuffixOfDesignation( designation ),
                        0 );

    // fill 'sheets qty' field
    aDocIface->SelectTable( 0 );
    aDocIface->PutCell( ADDR_SHEET_QTY,
                        wxString::Format( wxT( "%d" ), m_current_sheet + 1 ),
                        0 );

    aDocIface->Disconnect();

    return true;
}

} // namespace GOST_DOC_GEN
