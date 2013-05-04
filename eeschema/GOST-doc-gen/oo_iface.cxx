/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file oo_iface.cxx
 */

#include <macros.h>

#include <common_funcs.h>

#include <oo_iface.hxx>

namespace GOST_DOC_GEN {

OUString wx2OUString( wxString aStr )
{
    return OUString( TO_UTF8( aStr ),
                     strlen( TO_UTF8( aStr ) ),
                     RTL_TEXTENCODING_UTF8, 0 );
}


OO_IFACE::OO_IFACE()
{
}


OO_IFACE::~OO_IFACE()
{
    // dispose the local service manager
    Reference<XComponent>::query( m_xMultiComponentFactoryClient )->dispose();
}


bool OO_IFACE::Connect()
{
    wxString connection_str;
    OUString sConnectionString( RTL_CONSTASCII_USTRINGPARAM( CONNECTION_STRING ) );

#if defined (__WXMSW__)
    connection_str = wxT( "C:\\Program Files (x86)\\OpenOffice.org 3\\program\\soffice.exe" );
#else
    connection_str = wxT( "soffice" );
#endif

#if defined(__WXMSW__)
    connection_str += wxT(
         " -accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
                         );
#elif defined(NEW_OO_CONNECT_MODE)
    connection_str += wxT(
        " --invisible --accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
                         );
#else
    connection_str += wxT(
         " -invisible -accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
                         );
#endif

    if( !wxExecute( connection_str, wxEXEC_ASYNC ) )
        // for some reason this check does not work
        wxMessageBox( wxT( "Unable to launch the process: " ) + connection_str
                      + wxT( ".\nPlease make sure that OpenOffice / LibreOffice is installed." ),
                      wxEmptyString,
                      wxOK | wxICON_ERROR );
        // continue anyway in order to allow a user to run soffice from command shell


#if defined(NEW_OO_CONNECT_MODE)
    Reference< XComponentContext > xComponentContext(
        ::cppu::defaultBootstrap_InitialComponentContext() );
#else
    // Creates a simple registry service instance.
    Reference<XSimpleRegistry> xSimpleRegistry( ::cppu::createSimpleRegistry() );

    wxString filename = GetResourceFile( wxT( "GOST-doc-gen.rdb" ) );
    if( filename == wxEmptyString )
        return false;

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

        return false;
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
    m_xMultiComponentFactoryClient = Reference<XMultiComponentFactory>(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
     *  by the factory.
     */
    Reference<XInterface> xInterface;
    try
    {
        xInterface = m_xMultiComponentFactoryClient->createInstanceWithContext(
            OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ),
            xComponentContext );
    }
    catch( Exception& e )
    {
        wxMessageBox( wxString::FromUTF8(
            OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() ),
                      wxEmptyString, wxOK | wxICON_ERROR );

        return false;
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

    if( !connected )
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

        return false;
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
    m_xComponentLoader = Reference<XComponentLoader>(
        xMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ),
            xComponentContext ), UNO_QUERY );

    return true;
}


bool OO_IFACE::LoadDocument( wxString aUrl )
{
    /* Loads a component specified by an URL into the specified new or existing
     *  frame.
     */
    Sequence <PropertyValue> args1( 1 );
    args1[0].Name   = OUString::createFromAscii( "AsTemplate" );
    args1[0].Value  <<= true;

    Reference<XComponent> xWriterComponent;

    try
    {
        xWriterComponent = m_xComponentLoader->loadComponentFromURL(
            wx2OUString( aUrl ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), 0,
            args1 );
    }
    catch( Exception& e )
    {
        wxMessageBox( wxString::FromUTF8(
            OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() ),
                      wxEmptyString, wxOK | wxICON_ERROR );

        return false;
    }

    m_xTextDocument = Reference<XTextDocument>( xWriterComponent, UNO_QUERY );

    Reference<XTextTablesSupplier> xTablesSupplier( m_xTextDocument, UNO_QUERY );

    // get the tables collection
    Reference<XNameAccess> xNamedTables = xTablesSupplier->getTextTables();

    // now query the XIndexAccess from the tables collection
    m_xIndexedTables = Reference<XIndexAccess>( xNamedTables, UNO_QUERY );

    SelectTable( 0 );

    return true;
}


bool OO_IFACE::AppendDocument( wxString aUrl )
{
    Reference<XText>                xText = m_xTextDocument->getText();
    Reference<XTextRange>           xTextRange  = xText->getEnd();
    Reference<XTextCursor>          xTextCursor = xText->createTextCursorByRange( xTextRange );
    Reference<XDocumentInsertable>  xDocumentInsertable( xTextCursor, UNO_QUERY );

    try
    {
        xDocumentInsertable->insertDocumentFromURL(
            wx2OUString( aUrl ),
            Sequence < :: com::sun::star::beans::PropertyValue>() );
    }
    catch( Exception& e )
    {
        wxMessageBox( wxString::FromUTF8(
            OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() ),
                      wxEmptyString, wxOK | wxICON_ERROR );

        return false;
    }

    return true;
}


void OO_IFACE::SelectTable( int aIndex )
{
    m_xCurTable = Reference<XTextTable>( m_xIndexedTables->getByIndex( aIndex ), UNO_QUERY );
}


void OO_IFACE::PutCell( wxString aCellAddr,
                        wxString aStr,
                        int      aStyle )
{
    Reference<XCell>        xCell =
        m_xCurTable->getCellByName( wx2OUString( aCellAddr ) );
    Reference<XText>        xText = Reference<XText>( xCell, UNO_QUERY );
    Reference<XTextCursor>  xTextCursor = xText->createTextCursor();

    if( aStyle )
    {
        Reference<XPropertySet> CursorProperties( xTextCursor, UNO_QUERY );

        if( aStyle & TEXT_UNDERLINED )
            CursorProperties->setPropertyValue( OUString::createFromAscii( "CharUnderline" ),
                                                makeAny( (short) SINGLE ) );

        if( aStyle & TEXT_CENTERED )
            CursorProperties->setPropertyValue( OUString::createFromAscii( "ParaAdjust" ),
                                                makeAny( (short) 3 ) );
    }

    xTextCursor->setString( wx2OUString( aStr ) );
}

} // namespace GOST_DOC_GEN
