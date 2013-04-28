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
 * @file oo_common.hxx
 */

#ifndef OO_COMMON_HXX
#define OO_COMMON_HXX


#define SAL_UNX
#define UNX
#define CPPU_ENV gcc3

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

#include <wx/wx.h>
#include <component_db.h>

namespace GOST_DOC_GEN {

#define CONNECTION_STRING   "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"

#define NOTE_LENGTH_MAX             28

#define SPEC_NAME_LENGTH_MAX        34
#define SPEC_NOTE_LENGTH_MAX        11

#define FIRST_SHEET_LAST_STR_I      26
#define SECOND_SHEET_LAST_STR_I     29

#define TEXT_UNDERLINED             0x0001
#define TEXT_CENTERED               0x0002

#define WORDFORM_PLURAL             1
#define WORDFORM_SINGULAR_GENITIVE  2
#define WORDFORM_PLURAL_GENITIVE    3

#define SPEC_NO_SORTING             0x0001
#define SPEC_GEN_POS_LIST           0x0002

extern int current_row;
extern int current_sheet;
extern int specification_pos_field;

extern wxString GetResourceFile( wxString aFileName );
extern OUString wx2OUString( wxString aStr );

extern void OO_PrintCompIndexDocCell( Reference<XTextTable>    aTable,
                                      wxString                 aCell_address,
                                      wxString                 aStr,
                                      int                      aStyle );

extern void OO_PrintCompIndexDocRow( Reference<XTextDocument>  aTextDocument,
                                     Reference<XIndexAccess>   aIndexedTables,
                                     wxString                  aRef_des,
                                     wxString                  aName,
                                     int                       aQty,
                                     wxString                  aNote,
                                     int                       aStyle,
                                     int                       aReserve_strings,
                                     COMPONENT_DB*             aComponentDB );

extern void ProcessSingleVariant( COMPONENT_ARRAY*         aSingleVariantComponents,
                                  int                      aVariant,
                                  Reference<XTextDocument> aTextDocument,
                                  Reference<XIndexAccess>  aIndexedTables,
                                  COMPONENT_DB*            aComponentDB );

extern void OO_PrintSpecificationDocRow( Reference<XTextDocument>  aTextDocument,
                                         Reference<XIndexAccess>   aIndexedTables,
                                         wxString                  aFormat,
                                         int                       aPos,
                                         wxString                  aDesignation,
                                         wxString                  aName,
                                         wxString                  aQty,
                                         wxString                  aNote,
                                         int                       aStyle,
                                         int                       aReserve_strings,
                                         COMPONENT_DB*             aComponentDB );

extern void Specification_ProcessSingleVariant( wxArrayPtrVoid*            aSingleVariantComponents,
                                                int                        aVariant,
                                                Reference<XTextDocument>   aTextDocument,
                                                Reference<XIndexAccess>    aIndexedTables,
                                                COMPONENT_DB*              aComponentDB,
                                                wxArrayString*             aSpecification_positions,
                                                int                        aMode );

extern bool Form_a_set( COMPONENT_DB*              aComponentDB,
                        int                        aPart_type,
                        int                        aVariant,
                        wxArrayString*             aSpecification_positions,
                        Reference<XTextDocument>   aTextDocument,
                        Reference<XIndexAccess>    aIndexedTables );

} // namespace GOST_DOC_GEN

#endif    // OO_COMMON_HXX
