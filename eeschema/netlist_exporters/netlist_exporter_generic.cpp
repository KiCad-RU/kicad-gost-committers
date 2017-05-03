/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2013 jp.charras at wanadoo.fr
 * Copyright (C) 2013-2017 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 1992-2017 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <build_version.h>
#include <sch_base_frame.h>
#include <class_library.h>

#include <schframe.h>
#include "netlist_exporter_generic.h"

static bool sortPinsByNumber( LIB_PIN* aPin1, LIB_PIN* aPin2 );

bool NETLIST_EXPORTER_GENERIC::WriteNetlist( const wxString& aOutFileName, unsigned aNetlistOptions )
{
    // Prepare list of nets generation
    for( unsigned ii = 0; ii < m_masterList->size(); ii++ )
        m_masterList->GetItem( ii )->m_Flag = 0;

    // output the XML format netlist.
    wxXmlDocument   xdoc;

    xdoc.SetRoot( makeRoot( GNL_ALL ) );

    return xdoc.Save( aOutFileName, 2 /* indent bug, today was ignored by wxXml lib */ );
}


XNODE* NETLIST_EXPORTER_GENERIC::makeRoot( int aCtl )
{
    XNODE*      xroot = node( "export" );

    xroot->AddAttribute( "version", "D" );

    if( aCtl & GNL_HEADER )
        // add the "design" header
        xroot->AddChild( makeDesignHeader() );

    if( aCtl & GNL_COMPONENTS )
        xroot->AddChild( makeComponents() );

    if( aCtl & GNL_PARTS )
        xroot->AddChild( makeLibParts() );

    if( aCtl & GNL_LIBRARIES )
        // must follow makeGenericLibParts()
        xroot->AddChild( makeLibraries() );

    if( aCtl & GNL_NETS )
        xroot->AddChild( makeListOfNets() );

    return xroot;
}


/// Holder for multi-unit component fields
struct COMP_FIELDS
{
    wxString value;
    wxString datasheet;
    wxString footprint;

    std::map< wxString, wxString >   f;
};


void NETLIST_EXPORTER_GENERIC::addComponentFields( XNODE* xcomp, SCH_COMPONENT* comp, SCH_SHEET_PATH* aSheet )
{
    if( comp->GetUnitCount() > 1 )
    {
        // Sadly, each unit of a component can have its own unique fields.  This block
        // finds the last non blank field and records it.  Last guy wins and the order
        // of units occuring in a schematic hierarchy is variable.  Therefore user
        // is best off setting fields into only one unit.  But this scavenger algorithm
        // will find any non blank fields in all units and use the last non-blank field
        // for each unique field name.

        COMP_FIELDS fields;
        wxString    ref = comp->GetRef( aSheet );

        SCH_SHEET_LIST sheetList( g_RootSheet );

        for( unsigned i = 0;  i < sheetList.size();  i++ )
        {
            for( EDA_ITEM* item = sheetList[i].LastDrawList();  item;  item = item->Next() )
            {
                if( item->Type() != SCH_COMPONENT_T )
                    continue;

                SCH_COMPONENT*  comp2 = (SCH_COMPONENT*) item;

                wxString ref2 = comp2->GetRef( &sheetList[i] );

                if( ref2.CmpNoCase( ref ) != 0 )
                    continue;

                // The last guy wins.  User should only set fields in any one unit.

                if( !comp2->GetField( VALUE )->IsVoid() )
                    fields.value = comp2->GetField( VALUE )->GetText();

                if( !comp2->GetField( FOOTPRINT )->IsVoid() )
                    fields.footprint = comp2->GetField( FOOTPRINT )->GetText();

                if( !comp2->GetField( DATASHEET )->IsVoid() )
                    fields.datasheet = comp2->GetField( DATASHEET )->GetText();

                for( int fldNdx = MANDATORY_FIELDS;  fldNdx < comp2->GetFieldCount();  ++fldNdx )
                {
                    SCH_FIELD*  f = comp2->GetField( fldNdx );

                    if( f->GetText().size() )
                    {
                        fields.f[ f->GetName() ] = f->GetText();
                    }
                }
            }
        }

        xcomp->AddChild( node( "value", fields.value ) );

        if( fields.footprint.size() )
            xcomp->AddChild( node( "footprint", fields.footprint ) );

        if( fields.datasheet.size() )
            xcomp->AddChild( node( "datasheet", fields.datasheet ) );

        if( fields.f.size() )
        {
            XNODE* xfields;
            xcomp->AddChild( xfields = node( "fields" ) );

            // non MANDATORY fields are output alphabetically
            for( std::map< wxString, wxString >::const_iterator it = fields.f.begin();
                    it != fields.f.end();  ++it )
            {
                XNODE*  xfield;
                xfields->AddChild( xfield = node( "field", it->second ) );
                xfield->AddAttribute( "name", it->first );
            }
        }
    }
    else
    {
        xcomp->AddChild( node( "value", comp->GetField( VALUE )->GetText() ) );

        if( !comp->GetField( FOOTPRINT )->IsVoid() )
            xcomp->AddChild( node( "footprint", comp->GetField( FOOTPRINT )->GetText() ) );

        if( !comp->GetField( DATASHEET )->IsVoid() )
            xcomp->AddChild( node( "datasheet", comp->GetField( DATASHEET )->GetText() ) );

        // Export all user defined fields within the component,
        // which start at field index MANDATORY_FIELDS.  Only output the <fields>
        // container element if there are any <field>s.
        if( comp->GetFieldCount() > MANDATORY_FIELDS )
        {
            XNODE* xfields;
            xcomp->AddChild( xfields = node( "fields" ) );

            for( int fldNdx = MANDATORY_FIELDS; fldNdx < comp->GetFieldCount(); ++fldNdx )
            {
                SCH_FIELD*  f = comp->GetField( fldNdx );

                // only output a field if non empty and not just "~"
                if( !f->IsVoid() )
                {
                    XNODE*  xfield;
                    xfields->AddChild( xfield = node( "field", f->GetText() ) );
                    xfield->AddAttribute( "name", f->GetName() );
                }
            }
        }
    }
}


XNODE* NETLIST_EXPORTER_GENERIC::makeComponents()
{
    XNODE*      xcomps = node( "components" );

    wxString    timeStamp;

    m_ReferencesAlreadyFound.Clear();

    SCH_SHEET_LIST sheetList( g_RootSheet );

    // Output is xml, so there is no reason to remove spaces from the field values.
    // And XML element names need not be translated to various languages.

    for( unsigned i = 0;  i < sheetList.size();  i++ )
    {
        for( EDA_ITEM* schItem = sheetList[i].LastDrawList();  schItem;  schItem = schItem->Next() )
        {
            SCH_COMPONENT*  comp = findNextComponent( schItem, &sheetList[i] );
            if( !comp )
                break;  // No component left

            schItem = comp;

            XNODE* xcomp;  // current component being constructed

            // Output the component's elements in order of expected access frequency.
            // This may not always look best, but it will allow faster execution
            // under XSL processing systems which do sequential searching within
            // an element.

            xcomps->AddChild( xcomp = node( "comp" ) );
            xcomp->AddAttribute( "ref", comp->GetRef( &sheetList[i] ) );

            addComponentFields( xcomp, comp, &sheetList[i] );

            XNODE*  xlibsource;
            xcomp->AddChild( xlibsource = node( "libsource" ) );

            // "logical" library name, which is in anticipation of a better search
            // algorithm for parts based on "logical_lib.part" and where logical_lib
            // is merely the library name minus path and extension.
            LIB_PART* part = m_libs->FindLibPart( comp->GetLibId() );
            if( part )
                xlibsource->AddAttribute( "lib", part->GetLib()->GetLogicalName() );

            // We only want the symbol name, not the full LIB_ID.
            xlibsource->AddAttribute( "part", FROM_UTF8( comp->GetLibId().GetLibItemName() ) );

            XNODE* xsheetpath;

            xcomp->AddChild( xsheetpath = node( "sheetpath" ) );
            xsheetpath->AddAttribute( "names", sheetList[i].PathHumanReadable() );
            xsheetpath->AddAttribute( "tstamps", sheetList[i].Path() );

            timeStamp.Printf( "%8.8lX", (unsigned long)comp->GetTimeStamp() );
            xcomp->AddChild( node( "tstamp", timeStamp ) );
        }
    }

    return xcomps;
}


XNODE* NETLIST_EXPORTER_GENERIC::makeDesignHeader()
{
    SCH_SCREEN* screen;
    XNODE*     xdesign = node( "design" );
    XNODE*     xtitleBlock;
    XNODE*     xsheet;
    XNODE*     xcomment;
    wxString   sheetTxt;
    wxFileName sourceFileName;

    // the root sheet is a special sheet, call it source
    xdesign->AddChild( node( "source", g_RootSheet->GetScreen()->GetFileName() ) );

    xdesign->AddChild( node( "date", DateAndTime() ) );

    // which Eeschema tool
    xdesign->AddChild( node( "tool", wxString( "Eeschema " ) + GetBuildVersion() ) );

    /*
        Export the sheets information
    */
    SCH_SHEET_LIST sheetList( g_RootSheet );

    for( unsigned i = 0;  i < sheetList.size();  i++ )
    {
        screen = sheetList[i].LastScreen();

        xdesign->AddChild( xsheet = node( "sheet" ) );

        // get the string representation of the sheet index number.
        // Note that sheet->GetIndex() is zero index base and we need to increment the
        // number by one to make it human readable
        sheetTxt.Printf( "%u", i + 1 );
        xsheet->AddAttribute( "number", sheetTxt );
        xsheet->AddAttribute( "name", sheetList[i].PathHumanReadable() );
        xsheet->AddAttribute( "tstamps", sheetList[i].Path() );


        TITLE_BLOCK tb = screen->GetTitleBlock();

        xsheet->AddChild( xtitleBlock = node( "title_block" ) );

        xtitleBlock->AddChild( node( "title", tb.GetTitle() ) );
        xtitleBlock->AddChild( node( "company", tb.GetCompany() ) );
        xtitleBlock->AddChild( node( "rev", tb.GetRevision() ) );
        xtitleBlock->AddChild( node( "date", tb.GetDate() ) );

        // We are going to remove the fileName directories.
        sourceFileName = wxFileName( screen->GetFileName() );
        xtitleBlock->AddChild( node( "source", sourceFileName.GetFullName() ) );

        xtitleBlock->AddChild( xcomment = node( "comment" ) );
        xcomment->AddAttribute( "number", "1" );
        xcomment->AddAttribute( "value", tb.GetComment1() );

        xtitleBlock->AddChild( xcomment = node( "comment" ) );
        xcomment->AddAttribute( "number", "2" );
        xcomment->AddAttribute( "value", tb.GetComment2() );

        xtitleBlock->AddChild( xcomment = node( "comment" ) );
        xcomment->AddAttribute( "number", "3" );
        xcomment->AddAttribute( "value", tb.GetComment3() );

        xtitleBlock->AddChild( xcomment = node( "comment" ) );
        xcomment->AddAttribute( "number", "4" );
        xcomment->AddAttribute( "value", tb.GetComment4() );
    }

    return xdesign;
}


XNODE* NETLIST_EXPORTER_GENERIC::makeLibraries()
{
    XNODE*  xlibs = node( "libraries" );     // auto_ptr

    for( std::set<void*>::iterator it = m_Libraries.begin(); it!=m_Libraries.end();  ++it )
    {
        PART_LIB*    lib = (PART_LIB*) *it;
        XNODE*      xlibrary;

        xlibs->AddChild( xlibrary = node( "library" ) );
        xlibrary->AddAttribute( "logical", lib->GetLogicalName() );
        xlibrary->AddChild( node( "uri",  lib->GetFullFileName() ) );

        // @todo: add more fun stuff here
    }

    return xlibs;
}


XNODE* NETLIST_EXPORTER_GENERIC::makeLibParts()
{
    XNODE*      xlibparts = node( "libparts" );   // auto_ptr

    LIB_PINS    pinList;
    LIB_FIELDS  fieldList;

    m_Libraries.clear();

    for( std::set<LIB_PART*>::iterator it = m_LibParts.begin(); it!=m_LibParts.end();  ++it )
    {
        LIB_PART* lcomp = *it;
        PART_LIB* library = lcomp->GetLib();

        m_Libraries.insert( library );  // inserts component's library if unique

        XNODE* xlibpart;
        xlibparts->AddChild( xlibpart = node( "libpart" ) );
        xlibpart->AddAttribute( "lib", library->GetLogicalName() );
        xlibpart->AddAttribute( "part", lcomp->GetName()  );

        if( lcomp->GetAliasCount() )
        {
            wxArrayString aliases = lcomp->GetAliasNames( false );
            if( aliases.GetCount() )
            {
                XNODE* xaliases = node( "aliases" );
                xlibpart->AddChild( xaliases );
                for( unsigned i=0;  i<aliases.GetCount();  ++i )
                {
                    xaliases->AddChild( node( "alias", aliases[i] ) );
                }
            }
        }

        //----- show the important properties -------------------------
        if( !lcomp->GetAlias( 0 )->GetDescription().IsEmpty() )
            xlibpart->AddChild( node( "description", lcomp->GetAlias( 0 )->GetDescription() ) );

        if( !lcomp->GetAlias( 0 )->GetDocFileName().IsEmpty() )
            xlibpart->AddChild( node( "docs",  lcomp->GetAlias( 0 )->GetDocFileName() ) );

        // Write the footprint list
        if( lcomp->GetFootPrints().GetCount() )
        {
            XNODE*  xfootprints;
            xlibpart->AddChild( xfootprints = node( "footprints" ) );

            for( unsigned i=0; i<lcomp->GetFootPrints().GetCount(); ++i )
            {
                xfootprints->AddChild( node( "fp", lcomp->GetFootPrints()[i] ) );
            }
        }

        //----- show the fields here ----------------------------------
        fieldList.clear();
        lcomp->GetFields( fieldList );

        XNODE*     xfields;
        xlibpart->AddChild( xfields = node( "fields" ) );

        for( unsigned i=0;  i<fieldList.size();  ++i )
        {
            if( !fieldList[i].GetText().IsEmpty() )
            {
                XNODE*     xfield;
                xfields->AddChild( xfield = node( "field", fieldList[i].GetText() ) );
                xfield->AddAttribute( "name", fieldList[i].GetName(false) );
            }
        }

        //----- show the pins here ------------------------------------
        pinList.clear();
        lcomp->GetPins( pinList, 0, 0 );

        /* we must erase redundant Pins references in pinList
         * These redundant pins exist because some pins
         * are found more than one time when a component has
         * multiple parts per package or has 2 representations (DeMorgan conversion)
         * For instance, a 74ls00 has DeMorgan conversion, with different pin shapes,
         * and therefore each pin  appears 2 times in the list.
         * Common pins (VCC, GND) can also be found more than once.
         */
        sort( pinList.begin(), pinList.end(), sortPinsByNumber );
        for( int ii = 0; ii < (int)pinList.size()-1; ii++ )
        {
            if( pinList[ii]->GetNumber() == pinList[ii+1]->GetNumber() )
            {   // 2 pins have the same number, remove the redundant pin at index i+1
                pinList.erase(pinList.begin() + ii + 1);
                ii--;
            }
        }

        if( pinList.size() )
        {
            XNODE*     pins;

            xlibpart->AddChild( pins = node( "pins" ) );
            for( unsigned i=0; i<pinList.size();  ++i )
            {
                XNODE*     pin;

                pins->AddChild( pin = node( "pin" ) );
                pin->AddAttribute( "num", pinList[i]->GetNumberString() );
                pin->AddAttribute( "name", pinList[i]->GetName() );
                pin->AddAttribute( "type", pinList[i]->GetCanonicalElectricalTypeName() );

                // caution: construction work site here, drive slowly
            }
        }
    }

    return xlibparts;
}


XNODE* NETLIST_EXPORTER_GENERIC::makeListOfNets()
{
    XNODE*      xnets = node( "nets" );      // auto_ptr if exceptions ever get used.
    wxString    netCodeTxt;
    wxString    netName;
    wxString    ref;

    XNODE*      xnet = 0;
    int         netCode;
    int         lastNetCode = -1;
    int         sameNetcodeCount = 0;


    /*  output:
        <net code="123" name="/cfcard.sch/WAIT#">
            <node ref="R23" pin="1"/>
            <node ref="U18" pin="12"/>
        </net>
    */

    m_LibParts.clear();     // must call this function before using m_LibParts.

    for( unsigned ii = 0; ii < m_masterList->size(); ii++ )
    {
        NETLIST_OBJECT* nitem = m_masterList->GetItem( ii );
        SCH_COMPONENT*  comp;

        // New net found, write net id;
        if( ( netCode = nitem->GetNet() ) != lastNetCode )
        {
            sameNetcodeCount = 0;   // item count for this net
            netName = nitem->GetNetName();
            lastNetCode  = netCode;
        }

        if( nitem->m_Type != NET_PIN )
            continue;

        if( nitem->m_Flag != 0 )     // Redundant pin, skip it
            continue;

        comp = nitem->GetComponentParent();

        // Get the reference for the net name and the main parent component
        ref = comp->GetRef( &nitem->m_SheetPath );
        if( ref[0] == wxChar( '#' ) )
            continue;

        if( ++sameNetcodeCount == 1 )
        {
            xnets->AddChild( xnet = node( "net" ) );
            netCodeTxt.Printf( "%d", netCode );
            xnet->AddAttribute( "code", netCodeTxt );
            xnet->AddAttribute( "name", netName );
        }

        XNODE*      xnode;
        xnet->AddChild( xnode = node( "node" ) );
        xnode->AddAttribute( "ref", ref );
        xnode->AddAttribute( "pin",  nitem->GetPinNumText() );
    }

    return xnets;
}


XNODE* NETLIST_EXPORTER_GENERIC::node( const wxString& aName, const wxString& aTextualContent /* = wxEmptyString*/ )
{
    XNODE* n = new XNODE( wxXML_ELEMENT_NODE, aName );

    if( aTextualContent.Len() > 0 )     // excludes wxEmptyString, the parameter's default value
        n->AddChild( new XNODE( wxXML_TEXT_NODE, wxEmptyString, aTextualContent ) );

    return n;
}


static bool sortPinsByNumber( LIB_PIN* aPin1, LIB_PIN* aPin2 )
{
    // return "lhs < rhs"
    return RefDesStringCompare( aPin1->GetNumberString(), aPin2->GetNumberString() ) < 0;
}
