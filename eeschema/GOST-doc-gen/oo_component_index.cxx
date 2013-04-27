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
 * @file oo_component_index.cxx
 */

#include <oo_common.hxx>
#include <oo_component_index.hxx>

namespace GOST_DOC_GEN {

// returns FALSE if error
bool OO_CreateNewCompIndexDoc( COMPONENT_DB* aComponentDB )
{
    COMPONENT_ARRAY singleVariantComponents;
    int             variant;
    wxString        str, refdes_prefix_reserved, var_str;
    wxArrayString   letter_digit_sets;
    bool            comps_absent;

    OUString        sConnectionString( RTL_CONSTASCII_USTRINGPARAM( CONNECTION_STRING ) );

#if defined(NEW_OO_CONNECT_MODE)
    Reference< XComponentContext > xComponentContext(::cppu::defaultBootstrap_InitialComponentContext());
#else
    // Creates a simple registry service instance.
    Reference<XSimpleRegistry> xSimpleRegistry( ::cppu::createSimpleRegistry() );

    wxString filename = GetResourceFile( wxT( "GOST-doc-gen.rdb" ) );
    // Connects the registry to a persistent data source represented by an URL.
    xSimpleRegistry->open( wx2OUString( filename ), sal_True, sal_False );

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
            OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() ) );

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
            wxMilliSleep( 500 );
            continue;
        }

        connected = true;
        break;
    }

    if( !connected )
    {
        wxMessageBox(
            wxT( "Error: cannot establish a connection using '" ) +
            wxString::FromUTF8( OUStringToOString( sConnectionString,
                                                   RTL_TEXTENCODING_ASCII_US ).getStr() ) +
            wxT( "':\n       " ) +
            wxString::FromUTF8( OUStringToOString( last_err,
                                                   RTL_TEXTENCODING_ASCII_US ).getStr() ) +
            wxT( "\n" ) );

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

    Reference<XComponent> xWriterComponent = xComponentLoader->loadComponentFromURL(
        wx2OUString(
            GetResourceFile( wxT( "templates/CompsIndexFirstSheet_template.odt" ) ) ),
        OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), 0,
        args1 );

    Reference<XTextDocument> xTextDocument( xWriterComponent, UNO_QUERY );

    Reference<XTextTablesSupplier> xTablesSupplier( xTextDocument, UNO_QUERY );

    // get the tables collection
    Reference<XNameAccess> xNamedTables = xTablesSupplier->getTextTables();

    // now query the XIndexAccess from the tables collection
    Reference<XIndexAccess> xIndexedTables( xNamedTables, UNO_QUERY );
    Reference<XText> xText = xTextDocument->getText();

    Reference<XTextTable> xFirstTable( xIndexedTables->getByIndex( 0 ), UNO_QUERY );

    current_row     = 4;
    current_sheet   = 0;

    // fill 'design name' field
    OO_PrintCompIndexDocCell( xFirstTable, wxT( "B28" ), wxT( "\n" ) + aComponentDB->DesignName, 0 );
    // fill 'designation' field
    OO_PrintCompIndexDocCell( xFirstTable, wxT( "B27" ),
                              wxT( "\n" ) + aComponentDB->Designation + wxT( "ПЭ3" ), 0 );
    // fill 'first use' field
    OO_PrintCompIndexDocCell( xFirstTable, wxT( "B2" ), aComponentDB->Designation, 0 );

    // form the constant part of components index (not a set)
    aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, 0, 0, wxT( "" ) );

    // process the constant part (not a set)
    ProcessSingleVariant( &singleVariantComponents,
                          -1,
                          xTextDocument,
                          xIndexedTables,
                          aComponentDB );

    // form the constant part of components index (a set)
    Form_a_set( aComponentDB, PARTTYPE_A_SET, 0, NULL, xTextDocument, xIndexedTables );

    // form the variable part of components index
    if( aComponentDB->VariantsIndexes.GetCount() )
    {
        OO_PrintCompIndexDocRow( xTextDocument, xIndexedTables, wxT( "" ), wxT( "" ), 0,
                                 wxT( "" ), 0, 7, aComponentDB );

        OO_PrintCompIndexDocRow( xTextDocument, xIndexedTables, wxT( "" ),
                                 wxT( "Переменные данные для исполнений" ),
                                 0, wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );
    }

    for( int var_i = 0; var_i < (int) aComponentDB->VariantsIndexes.GetCount(); var_i++ )
    {
        comps_absent = TRUE;
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

        OO_PrintCompIndexDocRow( xTextDocument, xIndexedTables, wxT( "" ), wxT( "" ), 0,
                                 wxT( "" ), 0, 5, aComponentDB );

        OO_PrintCompIndexDocRow( xTextDocument, xIndexedTables, wxT( "" ), str, 0,
                                 wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        str = aComponentDB->DesignName;

        if( variant > 0 )
        {
            str += wxT( " - " );

            if( var_str.Len() == 1 )
                str += wxT( '0' );

            str += var_str;
        }

        OO_PrintCompIndexDocRow( xTextDocument, xIndexedTables, wxT( "" ), str, 0,
                                 wxT( "" ), TEXT_UNDERLINED | TEXT_CENTERED, 1, aComponentDB );

        aComponentDB->ExtractPartOfComponentsDB( &singleVariantComponents, PARTTYPE_VAR, variant,
                                                 wxT( "" ) );

        if( singleVariantComponents.GetCount()>0 )
            comps_absent = FALSE;

        ProcessSingleVariant( &singleVariantComponents,
                              variant,
                              xTextDocument,
                              xIndexedTables,
                              aComponentDB );

        // a set
        comps_absent &= ~Form_a_set( aComponentDB,
                                     PARTTYPE_VAR | PARTTYPE_A_SET,
                                     variant,
                                     NULL,
                                     xTextDocument,
                                     xIndexedTables );

        if( comps_absent )
        {
            OO_PrintCompIndexDocRow( xTextDocument, xIndexedTables, wxT( "" ), wxT( "" ), 0,
                                     wxT( "" ), 0, 1, aComponentDB );

            OO_PrintCompIndexDocRow( xTextDocument, xIndexedTables, wxT( "" ),
                                     wxT( "Отсутствуют" ), 0, wxT( "" ), 0, 1, aComponentDB );
        }
    }


    // print the last sheet
    Reference<XTextRange>           xTextRange  = xText->getEnd();
    Reference<XTextCursor>          xTextCursor = xText->createTextCursorByRange( xTextRange );
    Reference<XDocumentInsertable>  xDocumentInsertable( xTextCursor, UNO_QUERY );

    xDocumentInsertable->insertDocumentFromURL(
        wx2OUString( GetResourceFile( wxT( "templates/CompsIndexLastSheet_template.odt" ) ) ),
        Sequence < :: com::sun::star::beans::PropertyValue>() );

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
    OO_PrintCompIndexDocCell( xTable, wxT( "B28" ),
                              wxT( "\n" ) + aComponentDB->Designation + wxT( "ПЭ3" ), 0 );

    // dispose the local service manager
    Reference<XComponent>::query( xMultiComponentFactoryClient )->dispose();

    return TRUE;
}

} // namespace GOST_DOC_GEN
