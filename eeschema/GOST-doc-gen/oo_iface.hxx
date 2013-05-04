/* This program source code file is part of KiCad, a free EDA CAD application.
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
 * @file oo_iface.hxx
 */

#ifndef _OO_IFACE_HXX_
#define _OO_IFACE_HXX_

#include <common_doc_iface.h>

#ifdef __WINDOWS__
    #define WNT

    #ifdef __MSVC__
        #define CPPU_ENV msci
    #else
        #define CPPU_ENV gcc3
    #endif
#else
    #define SAL_UNX
    #define UNX
    #define CPPU_ENV gcc3
#endif

#include <cppuhelper/bootstrap.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
// #include <com/sun/star/style/ParagraphProperties.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;
using namespace com::sun::star::registry;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::table;
using namespace com::sun::star::awt::CharSet;
using namespace com::sun::star::awt::FontUnderline;
// using namespace com::sun::star::style::ParagraphProperties;
using namespace com::sun::star::document;
using namespace com::sun::star::sheet;


namespace GOST_DOC_GEN {

#define CONNECTION_STRING   "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"

class OO_IFACE : public COMMON_DOC_IFACE
{
public:
    OO_IFACE();
    ~OO_IFACE();

    bool Connect();
    bool LoadDocument( wxString aUrl );
    bool AppendDocument( wxString aUrl );
    void SelectTable( int aIndex );
    void PutCell( wxString aCellAddr,
                  wxString aStr,
                  int      aStyle );

private:
    Reference<XMultiComponentFactory> m_xMultiComponentFactoryClient;
    Reference<XComponentLoader>       m_xComponentLoader;
    Reference<XTextDocument>          m_xTextDocument;
    Reference<XIndexAccess>           m_xIndexedTables;
    Reference<XTextTable>             m_xCurTable;

    OUString wx2OUString( wxString aStr );
};

} // namespace GOST_DOC_GEN

#endif    // _OO_IFACE_HXX_
