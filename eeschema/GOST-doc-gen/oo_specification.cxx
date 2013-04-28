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
 * @file oo_specification.cxx
 */

#include <oo_common.hxx>
#include <oo_specification.hxx>

namespace GOST_DOC_GEN {

// returns TRUE if both components have identical attributes except ref des
bool CompareCompPos( TCOMPONENT_ATTRS* aComp1, TCOMPONENT_ATTRS* aComp2 )
{
    for( int i = 0; i < ATTR_QTY; i++ )
    {
        // skip comparison of ATTR_TYPE1
        if( i==ATTR_TYPE1)
            continue;

        if( aComp1->attrs[i] != aComp2->attrs[i] )
            return FALSE;
    }

    return TRUE;
}


void SpecPosList_ExtractPartOfDB( COMPONENT_ARRAY*          aAll_components,
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

        if( (aType & PARTTYPE_ALL_DETAILS)
            || ( (aType & PARTTYPE_ASSEMBLY_UNITS)
                 && ( pComponent->ComponentType==wxT( "AssemblyUnit" ) ) )
            || ( (aType & PARTTYPE_DETAILS) && ( pComponent->ComponentType==wxT( "Detail" ) ) )
            || ( (aType & PARTTYPE_STANDARD_DETAILS)
                 && ( pComponent->ComponentType==wxT( "StandardDetail" ) ) )
            || ( (aType & PARTTYPE_GOODS) && ( pComponent->ComponentType==wxT( "Goods" ) ) )
            || ( (!aType) && !( pComponent->ComponentType==wxT( "Documentation" ) )
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
                    && componentAttrs->attrs[ATTR_NOTE] != wxT( "Не устанавливается" ) )
                {
                    found = FALSE;

                    for( CompPos_i = 0;
                         CompPos_i < (int) aAllCompPositions->GetCount();
                         CompPos_i++ )
                    {
                        if( CompareCompPos( (pTCOMPONENT_ATTRS) (*aAllCompPositions)[CompPos_i],
                                            componentAttrs ) )
                        {
                            found = TRUE;
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


void Specification_GeneratePosList( COMPONENT_ARRAY* aAll_components,
                                    wxArrayString*   aSpecification_positions )
{
    wxString                str;
    TCOMPONENT_ATTRS_ARRAY  allCompPositions;

    aSpecification_positions->Clear();

    // form all component positions list (assembly units)
    SpecPosList_ExtractPartOfDB( aAll_components, &allCompPositions, PARTTYPE_ASSEMBLY_UNITS );
    Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &allCompPositions, -1, NULL, NULL, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aAll_components, &allCompPositions, PARTTYPE_DETAILS );

    Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &allCompPositions, -1, NULL, NULL, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aAll_components, &allCompPositions, PARTTYPE_STANDARD_DETAILS );

    Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &allCompPositions, -1, NULL, NULL, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aAll_components, &allCompPositions, 0 );

    Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &allCompPositions, -1, NULL, NULL, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );

    SpecPosList_ExtractPartOfDB( aAll_components, &allCompPositions, PARTTYPE_GOODS );

    Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &allCompPositions, -1, NULL, NULL, NULL,
                                        aSpecification_positions, SPEC_GEN_POS_LIST );
}


// returns FALSE if error
bool OO_CreateNewSpecificationDoc( COMPONENT_DB* aComponentDB )
{
    COMPONENT_ARRAY singleVariantComponents, AllVariantsComponents;
    int             variant;
    wxString        str, var_str;
    wxArrayString   specification_positions;
    bool            comps_absent;

    OUString        sConnectionString( RTL_CONSTASCII_USTRINGPARAM( CONNECTION_STRING ) );

#if defined(NEW_OO_CONNECT_MODE)
    Reference< XComponentContext > xComponentContext(::cppu::defaultBootstrap_InitialComponentContext());
#else
    // Creates a simple registry service instance.
    Reference<XSimpleRegistry> xSimpleRegistry( ::cppu::createSimpleRegistry() );

    wxString filename = GetResourceFile( wxT( "GOST-doc-gen.rdb" ) );
    if( filename == wxEmptyString )
        return FALSE;

    try
    {
        // Connects the registry to a persistent data source represented by an URL.
        xSimpleRegistry->open( wx2OUString( filename ), sal_True, sal_False );
    }
    catch( Exception& e )
    {
        wxMessageBox( wxString::FromUTF8(
            OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() ),
                      wxEmptyString, wxOK | wxICON_ERROR );

        return FALSE;
    }

    /* Bootstraps an initial component context with service manager upon a given
     *  registry. This includes insertion of initial services:
     *  - (registry) service manager, shared lib loader,
     *  - simple registry, nested registry,
     *  - implementation registration
     *  - registry typedescription provider, typedescription manager (also
     *    installs it into cppu core)
     */
    Reference<XComponentContext> xComponentContext(
        ::cppu::bootstrap_InitialComponentContext( xSimpleRegistry ) );
#endif

    /* Gets the service manager instance to be used (or null). This method has
     *  been added for convenience, because the service manager is a often used
     *  object.
     */
    Reference<XMultiComponentFactory> xMultiComponentFactoryClient(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
     *  by the factory.
     */
    Reference<XInterface> xInterface;
    try
    {
        xInterface = xMultiComponentFactoryClient->createInstanceWithContext(
            OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ),
            xComponentContext );
    }
    catch( Exception& e )
    {
        wxMessageBox( wxString::FromUTF8(
            OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() ),
                      wxEmptyString, wxOK | wxICON_ERROR );

        return FALSE;
    }

    Reference<XUnoUrlResolver> resolver( xInterface, UNO_QUERY );

    // Resolves the component context from the office, on the uno URL given by argv[1].
    bool connected = false;
    OUString last_err;
    // try to connect, if failed then wait for Open Office gets ready
    for( int retry = 0; retry < 10; retry++ )
    {
        try
        {
            xInterface = Reference<XInterface>(
                resolver->resolve( sConnectionString ), UNO_QUERY );
        }
        catch( Exception& e )
        {
            last_err = e.Message;
            wxMilliSleep( 1000 );
            continue;
        }

        connected = true;
        break;
    }

    if( !connected)
    {
        wxMessageBox(
            wxT( "Error: cannot establish a connection using '" ) +
            wxString::FromUTF8( OUStringToOString( sConnectionString,
                                                   RTL_TEXTENCODING_UTF8 ).getStr() ) +
            wxT( "':\n       " ) +
            wxString::FromUTF8( OUStringToOString( last_err,
                                                   RTL_TEXTENCODING_UTF8 ).getStr() ) +
            wxT( "\n" ),
            wxEmptyString, wxOK | wxICON_ERROR );

        return FALSE;
    }

    // gets the server component context as property of the office component factory
    Reference<XPropertySet> xPropSet( xInterface, UNO_QUERY );
    xPropSet->getPropertyValue( OUString::createFromAscii( "DefaultContext" ) ) >>=
        xComponentContext;

    // gets the service manager from the office
    Reference<XMultiComponentFactory> xMultiComponentFactoryServer(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
     *  by the factory. Important: using the office component context.
     */
    Reference <XComponentLoader> xComponentLoader(
        xMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ),
            xComponentContext ), UNO_QUERY );

    /* Loads a component specified by an URL into the specified new or existing
     *  frame.
     */
    Sequence <PropertyValue> args1( 1 );
    args1[0].Name   = OUString::createFromAscii( "AsTemplate" );
    args1[0].Value  <<= true;

    Reference<XComponent> xWriterComponent;

    try
    {
        xWriterComponent = xComponentLoader->loadComponentFromURL(
            wx2OUString( GetResourceFile(
                wxT( "templates/SpecificationFirstSheet_template.odt" ) ) ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), 0,
            args1 );
    }
    catch( Exception& e )
    {
        wxMessageBox( wxString::FromUTF8(
            OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() ),
                      wxEmptyString, wxOK | wxICON_ERROR );

        return FALSE;
    }

    Reference<XTextDocument> xTextDocument( xWriterComponent, UNO_QUERY );

    Reference<XTextTablesSupplier> xTablesSupplier( xTextDocument, UNO_QUERY );

    // get the tables collection
    Reference<XNameAccess> xNamedTables = xTablesSupplier->getTextTables();

    // now query the XIndexAccess from the tables collection
    Reference<XIndexAccess> xIndexedTables( xNamedTables, UNO_QUERY );
    Reference<XText> xText = xTextDocument->getText();

    current_row = 4;
    current_sheet = 0;
    specification_pos_field = 1;

    // Generate positions list
    Specification_GeneratePosList( &aComponentDB->m_AllComponents, &specification_positions );

    Reference<XTextTable> xFirstTable( xIndexedTables->getByIndex( 0 ), UNO_QUERY );
    // fill 'design name' field
    OO_PrintCompIndexDocCell( xFirstTable, wxT( "B28" ), wxT( "\n" ) + aComponentDB->DesignName, 0 );
    // fill 'designation' field
    OO_PrintCompIndexDocCell( xFirstTable, wxT( "B27" ),
                              wxT( "\n" ) + aComponentDB->Designation, 0 );
    // fill 'first use' field
    OO_PrintCompIndexDocCell( xFirstTable, wxT( "B2" ), aComponentDB->SpecFirstUse, 0 );

    OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );
    OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                 wxT( "Документация" ), wxT( "" ), wxT( "" ),
                                 TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

    // form the constant part of specification (documents)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_DOCUMENTATION, 0, wxT( "" ) );

    if( singleVariantComponents.GetCount() )
    {
        // process the constant part (documents)
        Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            xTextDocument, xIndexedTables, aComponentDB,
                                            &specification_positions,
                                            SPEC_NO_SORTING );
    }
    else
    {
        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument,
                                     xIndexedTables,
                                     aComponentDB->AssemblyDrawingFmt,
                                     0,
                                     aComponentDB->Designation + wxT( " СБ" ),
                                     wxT( "Сборочный чертеж" ),
                                     wxT( "" ),
                                     wxT( "" ),
                                     0,
                                     1,
                                     aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        if( aComponentDB->CircuitDrawingFmt.Len() <= 2 )
            OO_PrintSpecificationDocRow( xTextDocument,
                                         xIndexedTables,
                                         aComponentDB->CircuitDrawingFmt,
                                         0,
                                         aComponentDB->Designation + wxT( " Э3" ),
                                         wxT( "Схема электрическая принципиальная" ),
                                         wxT( "" ),
                                         wxT( "" ),
                                         0,
                                         1,
                                         aComponentDB );
        else
            OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "*" ), 0,
                                         aComponentDB->Designation + wxT( " Э3" ),
                                         wxT( "Схема электрическая принципиальная" ),
                                         wxT( "" ),
                                         wxT( "*) " ) + aComponentDB->CircuitDrawingFmt, 0, 1,
                                         aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables,
                                     wxT( "A4" ), 0, aComponentDB->Designation + wxT( " ПЭ3" ),
                                     wxT( "Перечень элементов" ), wxT( "" ),
                                     wxT( "" ), 0, 1, aComponentDB );
    }

    // form the constant part of specification (assembly units)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_ASSEMBLY_UNITS, 0, wxT( "" ) );

    if( singleVariantComponents.GetCount() > 0 )
    {
        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Сборочные единицы" ), wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        // process the constant part (assembly units)
        Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            xTextDocument, xIndexedTables, aComponentDB,
                                            &specification_positions,
                                            0 );
    }

    // form the constant part of specification (details)
    OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

    OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                 wxT( "Детали" ), wxT( "" ), wxT( "" ),
                                 TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_DETAILS, 0, wxT( "" ) );

    if( singleVariantComponents.GetCount() > 0 )
    {
        // process the constant part (details)
        Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            xTextDocument, xIndexedTables, aComponentDB,
                                            &specification_positions,
                                            0 );
    }
    else
    {
        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables,
                                     wxT( "" ), specification_pos_field++,
                                     aComponentDB->PCBDesignation,
                                     wxT( "Плата печатная" ), wxT( "1" ), wxT(
                                         "" ), 0, 1, aComponentDB );
    }


    // form the constant part of specification (standard details)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_STANDARD_DETAILS,
                                             0, wxT( "" ) );

    if( singleVariantComponents.GetCount() > 0 )
    {
        current_row = SECOND_SHEET_LAST_STR_I;
        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Стандартные изделия" ), wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        // process the constant part (standard details)
        Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            xTextDocument, xIndexedTables, aComponentDB,
                                            &specification_positions,
                                            0 );
    }

    OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 5, aComponentDB );

    OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                 wxT( "" ), wxT( "" ), wxT( "" ), 0, 1, aComponentDB );

    OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                 wxT( "Прочие изделия" ), wxT( "" ), wxT( "" ),
                                 TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

    // form the constant part of specification (not a set)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION, 0,
                                             wxT( "" ) );

    // process the constant part
    Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                        xTextDocument, xIndexedTables, aComponentDB,
                                        &specification_positions,
                                        0 );

    // form the constant part of specification (a set)
    Form_a_set( aComponentDB, PARTTYPE_SPECIFICATION | PARTTYPE_A_SET, 0, &specification_positions,
                xTextDocument, xIndexedTables );

    // form the constant part of specification (goods)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_SPECIFICATION |
                                             PARTTYPE_GOODS, 0, wxT( "" ) );

    if( singleVariantComponents.GetCount() > 0 )
    {
        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 4, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Материалы" ), wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        // process the constant part (goods)
        Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents, -1,
                                            xTextDocument, xIndexedTables, aComponentDB,
                                            &specification_positions,
                                            0 );
    }


    // form the variable part of specification
    if( aComponentDB->VariantsIndexes.GetCount() )
    {
        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 10, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "Переменные данные для исполнений" ), wxT( "" ),
                                     wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );
    }

    for( int var_i = 0; var_i < (int) aComponentDB->VariantsIndexes.GetCount(); var_i++ )
    {
        comps_absent = TRUE;

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

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0, wxT( "" ),
                                     wxT( "" ), wxT( "" ), wxT( "" ), 0, 7, aComponentDB );

        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                     wxT( "" ), str, wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        str = aComponentDB->DesignName;

        if( variant > 0 )
        {
            str += wxT( " - " );

            if( var_str.Len() == 1 )
                str += wxT( '0' );

            str += var_str;
        }

        str.Replace( wxT( " " ), wxT( "_" ) );
        OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                     wxT( "" ), str, wxT( "" ), wxT( "" ),
                                     TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );


        // form the variable part of specification (assembly units)
        aComponentDB->ExtractPartOfComponentsDB(
            &singleVariantComponents,
            PARTTYPE_SPECIFICATION | PARTTYPE_VAR |
            PARTTYPE_ASSEMBLY_UNITS, variant, wxT( "" ) );

        if( singleVariantComponents.GetCount() > 0 )
        {
            comps_absent = FALSE;
            OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                         wxT( "" ), wxT( "" ), wxT( "" ),
                                         wxT( "" ), 0, 4, aComponentDB );

            OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                         wxT( "" ), wxT( "Сборочные единицы" ), wxT( "" ), wxT( "" ),
                                         TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

            // process the variable part (assembly units)
            Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents,
                                                variant, xTextDocument, xIndexedTables,
                                                aComponentDB,
                                                &specification_positions,
                                                0 );
        }

        // form the variable part of specification (details)
        aComponentDB->ExtractPartOfComponentsDB(
            &singleVariantComponents,
            PARTTYPE_SPECIFICATION | PARTTYPE_VAR |
            PARTTYPE_DETAILS, variant, wxT( "" ) );

        if( singleVariantComponents.GetCount() > 0 )
        {
            comps_absent = FALSE;
            OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                         wxT( "" ), wxT( "" ), wxT( "" ),
                                         wxT( "" ), 0, 4, aComponentDB );

            OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                         wxT( "" ), wxT( "Детали" ), wxT( "" ), wxT( "" ),
                                         TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

            // process the variable part (details)
            Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents,
                                                variant, xTextDocument, xIndexedTables,
                                                aComponentDB,
                                                &specification_positions,
                                                0 );
        }

        // form the variable part of specification (not a set)
        aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents,
                                                 PARTTYPE_SPECIFICATION | PARTTYPE_VAR, variant,
                                                 wxT( "" ) );

        if( singleVariantComponents.GetCount() > 0 )
            comps_absent = FALSE;

        Specification_ProcessSingleVariant( (wxArrayPtrVoid*) &singleVariantComponents, variant,
                                            xTextDocument, xIndexedTables, aComponentDB,
                                            &specification_positions,
                                            0 );

        // form the variable part of specification (a set)
        comps_absent &= ~Form_a_set( aComponentDB,
                                     PARTTYPE_SPECIFICATION | PARTTYPE_VAR | PARTTYPE_A_SET,
                                     variant,
                                     &specification_positions,
                                     xTextDocument,
                                     xIndexedTables );

        if( comps_absent )
        {
            OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                         wxT( "" ), wxT( "" ), wxT( "" ),
                                         wxT( "" ), 0, 1, aComponentDB );

            OO_PrintSpecificationDocRow( xTextDocument, xIndexedTables, wxT( "" ), 0,
                                         wxT( "" ), wxT( "Отсутствуют" ), wxT( "" ),
                                         wxT( "" ), 0, 1, aComponentDB );
        }
    }


    // print the last sheet
    Reference<XTextRange>           xTextRange  = xText->getEnd();
    Reference<XTextCursor>          xTextCursor = xText->createTextCursorByRange( xTextRange );
    Reference<XDocumentInsertable>  xDocumentInsertable( xTextCursor, UNO_QUERY );

    try
    {
        xDocumentInsertable->insertDocumentFromURL(
            wx2OUString( GetResourceFile(
                wxT( "templates/CompsIndexLastSheet_template.odt" ) ) ),
            Sequence < :: com::sun::star::beans::PropertyValue>() );
    }
    catch( Exception& e )
    {
        wxMessageBox( wxString::FromUTF8(
            OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() ),
                      wxEmptyString, wxOK | wxICON_ERROR );

        return FALSE;
    }

    current_sheet++;
    Reference<XTextTable> xTable( xIndexedTables->getByIndex( current_sheet ), UNO_QUERY );

    // fill 'sheet number' field
    OO_PrintCompIndexDocCell( xTable, wxT( "C28.1.2" ),
                              wxT( "\n" ) + wxString::Format( wxT( "%d" ), current_sheet + 1 ),
                              0 );
    // fill 'sheets qty' field
    OO_PrintCompIndexDocCell( xFirstTable, wxT( "C28.5.2" ),
                              wxString::Format( wxT( "%d" ), current_sheet + 1 ),
                              0 );
    // fill 'designation' field
    OO_PrintCompIndexDocCell( xTable, wxT( "B28" ), wxT( "\n" ) + aComponentDB->Designation, 0 );

    // dispose the local service manager
    Reference<XComponent>::query( xMultiComponentFactoryClient )->dispose();

    return TRUE;
}

} // namespace GOST_DOC_GEN
