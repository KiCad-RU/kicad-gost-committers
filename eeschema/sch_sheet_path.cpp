/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file sch_sheet_path.cpp
 * @brief SCH_SHEET_PATH class implementation.
 */

#include "fctsys.h"

#include "general.h"
#include "dlist.h"
#include "class_sch_screen.h"
#include "sch_item_struct.h"

#include "netlist.h"
#include "class_library.h"
#include "sch_sheet.h"
#include "sch_sheet_path.h"
#include "sch_component.h"
#include "template_fieldnames.h"

#include "dialogs/dialog_schematic_find.h"


static const wxString traceFindReplace( wxT( "KicadFindReplace" ) );


SCH_SHEET_PATH::SCH_SHEET_PATH()
{
    for( int i = 0; i<DSLSZ; i++ )
        m_sheets[i] = NULL;

    m_numSheets = 0;
}


bool SCH_SHEET_PATH::BuildSheetPathInfoFromSheetPathValue( const wxString& aPath, bool aFound )
{
    if( aFound )
        return true;

    if(  GetSheetsCount() == 0 )
        Push( g_RootSheet );

    if( aPath == Path() )
        return true;

    SCH_ITEM* schitem = LastDrawList();

    while( schitem && GetSheetsCount() < NB_MAX_SHEET )
    {
        if( schitem->Type() == SCH_SHEET_T )
        {
            SCH_SHEET* sheet = (SCH_SHEET*) schitem;
            Push( sheet );

            if( aPath == Path() )
                return true;

            if( BuildSheetPathInfoFromSheetPathValue( aPath ) )
                return true;

            Pop();
        }
        schitem = schitem->Next();
    }

    return false;
}


int SCH_SHEET_PATH::Cmp( const SCH_SHEET_PATH& aSheetPathToTest ) const
{
    if( m_numSheets > aSheetPathToTest.m_numSheets )
        return 1;

    if( m_numSheets < aSheetPathToTest.m_numSheets )
        return -1;

    //otherwise, same number of sheets.
    for( unsigned i = 0; i<m_numSheets; i++ )
    {
        if( m_sheets[i]->m_TimeStamp > aSheetPathToTest.m_sheets[i]->m_TimeStamp )
            return 1;

        if( m_sheets[i]->m_TimeStamp < aSheetPathToTest.m_sheets[i]->m_TimeStamp )
            return -1;
    }

    return 0;
}


SCH_SHEET* SCH_SHEET_PATH::Last()
{
    if( m_numSheets )
        return m_sheets[m_numSheets - 1];

    return NULL;
}


SCH_SCREEN* SCH_SHEET_PATH::LastScreen()
{
    SCH_SHEET* lastSheet = Last();

    if( lastSheet )
        return lastSheet->GetScreen();

    return NULL;
}


SCH_ITEM* SCH_SHEET_PATH::LastDrawList()
{
    SCH_SHEET* lastSheet = Last();

    if( lastSheet && lastSheet->GetScreen() )
        return lastSheet->GetScreen()->GetDrawItems();

    return NULL;
}


SCH_ITEM* SCH_SHEET_PATH::FirstDrawList()
{
    SCH_ITEM* item = NULL;

    if( m_numSheets && m_sheets[0]->GetScreen() )
        item = m_sheets[0]->GetScreen()->GetDrawItems();

    /* @fixme - These lists really should be one of the boost pointer containers.  This
     *          is a brain dead hack to allow reverse iteration of EDA_ITEM linked
     *          list.
     */
    SCH_ITEM* lastItem = NULL;

    while( item != NULL )
    {
        lastItem = item;
        item = item->Next();
    }

    return lastItem;
}


void SCH_SHEET_PATH::Push( SCH_SHEET* aSheet )
{
    wxCHECK_RET( m_numSheets < DSLSZ,
                 wxString::Format( _( "Schematic sheets can only be nested %d levels deep." ),
                                   DSLSZ ) );

    m_sheets[ m_numSheets ] = aSheet;
    m_numSheets++;
}


SCH_SHEET* SCH_SHEET_PATH::Pop()
{
    if( m_numSheets > 0 )
    {
        m_numSheets--;
        return m_sheets[m_numSheets];
    }

    return NULL;
}


wxString SCH_SHEET_PATH::Path() const
{
    wxString s, t;

    s = wxT( "/" );     // This is the root path

    // start at 1 to avoid the root sheet,
    // which does not need to be added to the path
    // it's timestamp changes anyway.
    for( unsigned i = 1; i < m_numSheets; i++ )
    {
        t.Printf( _( "%8.8lX/" ), m_sheets[i]->m_TimeStamp );
        s = s + t;
    }

    return s;
}


wxString SCH_SHEET_PATH::PathHumanReadable() const
{
    wxString s, t;

    s = wxT( "/" );

    // start at 1 to avoid the root sheet, as above.
    for( unsigned i = 1; i< m_numSheets; i++ )
    {
        s = s + m_sheets[i]->m_SheetName + wxT( "/" );
    }

    return s;
}


void SCH_SHEET_PATH::UpdateAllScreenReferences()
{
    EDA_ITEM* t = LastDrawList();

    while( t )
    {
        if( t->Type() == SCH_COMPONENT_T )
        {
            SCH_COMPONENT* component = (SCH_COMPONENT*) t;
            component->GetField( REFERENCE )->m_Text = component->GetRef( this );
            component->SetUnit( component->GetUnitSelection( this ) );
        }

        t = t->Next();
    }
}


void SCH_SHEET_PATH::AnnotatePowerSymbols( int* aReference )
{
    int ref = 1;

    if( aReference != NULL )
        ref = *aReference;

    for( EDA_ITEM* item = LastDrawList(); item != NULL; item = item->Next() )
    {
        if( item->Type() != SCH_COMPONENT_T )
                continue;

        SCH_COMPONENT* component = (SCH_COMPONENT*) item;
        LIB_COMPONENT* entry = CMP_LIBRARY::FindLibraryComponent( component->GetLibName() );

        if( ( entry == NULL ) || !entry->IsPower() )
            continue;

        wxString refstr = component->GetPrefix();

        //str will be "C?" or so after the ClearAnnotation call.
        while( refstr.Last() == '?' )
            refstr.RemoveLast();

        if( !refstr.StartsWith( wxT( "#" ) ) )
            refstr = wxT( "#" ) + refstr;

        refstr << wxT( "0" ) << ref;
        component->SetRef( this, refstr );
        ref++;
    }

    if( aReference != NULL )
        *aReference = ref;
}


void SCH_SHEET_PATH::GetComponents( SCH_REFERENCE_LIST& aReferences, bool aIncludePowerSymbols )
{
    // Search to sheet path number:
    int sheetnumber = 1;    // 1 = root
    SCH_SHEET_LIST sheetList;

    for( SCH_SHEET_PATH* path = sheetList.GetFirst(); path != NULL;
         path = sheetList.GetNext(), sheetnumber++ )
    {
        if( Cmp(*path) == 0 )
            break;
    }

    for( SCH_ITEM* item = LastDrawList(); item != NULL; item = item->Next() )
    {
        if( item->Type() == SCH_COMPONENT_T )
        {
            SCH_COMPONENT* component = (SCH_COMPONENT*) item;

            // Skip pseudo components, which have a reference starting with #.  This mainly
            // effects power symbols.
            if( !aIncludePowerSymbols && component->GetRef( this )[0] == wxT( '#' ) )
                continue;

            LIB_COMPONENT* entry = CMP_LIBRARY::FindLibraryComponent( component->GetLibName() );

            if( entry == NULL )
                continue;

            SCH_REFERENCE reference = SCH_REFERENCE( component, entry, *this );
            reference.SetSheetNumber( sheetnumber );
            aReferences.AddItem( reference );
        }
    }
}


SCH_ITEM* SCH_SHEET_PATH::FindNextItem( KICAD_T aType, SCH_ITEM* aLastItem, bool aWrap )
{
    bool hasWrapped = false;
    bool firstItemFound = false;
    SCH_ITEM* drawItem = LastDrawList();

    while( drawItem != NULL )
    {
        if( drawItem->Type() == aType )
        {
            if( aLastItem == NULL || firstItemFound )
            {
                return drawItem;
            }
            else if( !firstItemFound && drawItem == aLastItem )
            {
                firstItemFound = true;
            }
        }

        drawItem = drawItem->Next();

        if( drawItem == NULL && aLastItem && aWrap && !hasWrapped )
        {
            hasWrapped = true;
            drawItem = LastDrawList();
        }
    }

    return NULL;
}


SCH_ITEM* SCH_SHEET_PATH::FindPreviousItem( KICAD_T aType, SCH_ITEM* aLastItem, bool aWrap )
{
    bool hasWrapped = false;
    bool firstItemFound = false;
    SCH_ITEM* drawItem = FirstDrawList();

    while( drawItem != NULL )
    {
        if( drawItem->Type() == aType )
        {
            if( aLastItem == NULL || firstItemFound )
            {
                return drawItem;
            }
            else if( !firstItemFound && drawItem == aLastItem )
            {
                firstItemFound = true;
            }
        }

        drawItem = drawItem->Back();

        if( drawItem == NULL && aLastItem && aWrap && !hasWrapped )
        {
            hasWrapped = true;
            drawItem = FirstDrawList();
        }
    }

    return NULL;
}


SCH_ITEM* SCH_SHEET_PATH::MatchNextItem( wxFindReplaceData& aSearchData,
                                         SCH_ITEM*          aLastItem,
                                         wxPoint*           aFindLocation )
{
    bool hasWrapped = false;
    bool firstItemFound = false;
    bool wrap = ( aSearchData.GetFlags() & FR_SEARCH_WRAP ) != 0;
    SCH_ITEM* drawItem = LastDrawList();

    while( drawItem != NULL )
    {
        if( aLastItem && !firstItemFound )
        {
            firstItemFound = ( drawItem == aLastItem );
        }
        else
        {
            if( drawItem->Matches( aSearchData, this, aFindLocation ) )
                return drawItem;
        }

        drawItem = drawItem->Next();

        if( drawItem == NULL && aLastItem && firstItemFound && wrap && !hasWrapped )
        {
            hasWrapped = true;
            drawItem = LastDrawList();
        }
        else if( hasWrapped && aLastItem && firstItemFound && (drawItem == aLastItem) )
        {
            // Exit when wrapped around to the first item found.
            drawItem = NULL;
        }
    }

    return NULL;
}


bool SCH_SHEET_PATH::SetComponentFootprint( const wxString& aReference, const wxString& aFootPrint,
                                            bool aSetVisible )
{
    SCH_SCREEN* screen = LastScreen();

    if( screen == NULL )
        return false;

    return screen->SetComponentFootprint( this, aReference, aFootPrint, aSetVisible );
}


SCH_SHEET_PATH& SCH_SHEET_PATH::operator=( const SCH_SHEET_PATH& d1 )
{
    if( this == &d1 )     // Self assignment is bad!
        return *this;

    m_numSheets = d1.m_numSheets;

    unsigned i;

    for( i = 0; i < m_numSheets; i++ )
        m_sheets[i] = d1.m_sheets[i];

    for( ; i < DSLSZ; i++ )
        m_sheets[i] = 0;

    return *this;
}


bool SCH_SHEET_PATH::operator==( const SCH_SHEET_PATH& d1 ) const
{
    if( m_numSheets != d1.m_numSheets )
        return false;

    for( unsigned i = 0; i < m_numSheets; i++ )
    {
        if( m_sheets[i] != d1.m_sheets[i] )
            return false;
    }

    return true;
}


/********************************************************************/
/* Class SCH_SHEET_LIST to handle the list of Sheets in a hierarchy */
/********************************************************************/


SCH_SHEET_LIST::SCH_SHEET_LIST( SCH_SHEET* aSheet )
{
    m_index = 0;
    m_count = 0;
    m_List  = NULL;

    if( aSheet == NULL )
        aSheet = g_RootSheet;

    BuildSheetList( aSheet );
}


SCH_SHEET_PATH* SCH_SHEET_LIST::GetFirst()
{
    m_index = 0;

    if( GetCount() > 0 )
        return &( m_List[0] );

    return NULL;
}


SCH_SHEET_PATH* SCH_SHEET_LIST::GetNext()
{
    if( m_index < GetCount() )
        m_index++;

    return GetSheet( m_index );
}


SCH_SHEET_PATH* SCH_SHEET_LIST::GetLast()
{
    if( GetCount() == 0 )
        return NULL;

    m_index = GetCount() - 1;

    return GetSheet( m_index );
}


SCH_SHEET_PATH* SCH_SHEET_LIST::GetPrevious()
{
    if( m_index == 0 )
        return NULL;

    m_index -= 1;

    return GetSheet( m_index );
}


SCH_SHEET_PATH* SCH_SHEET_LIST::GetSheet( int aIndex )
{
    if( aIndex < GetCount() )
        return &( m_List[aIndex] );

    return NULL;
}


SCH_SHEET_PATH* SCH_SHEET_LIST::GetSheet( const wxString aPath, bool aHumanReadable )
{
    SCH_SHEET_PATH* sheet = GetFirst();
    wxString sheetPath;

    while( sheet != NULL )
    {
        sheetPath = ( aHumanReadable ) ? sheet->PathHumanReadable() : sheet->Path();

        if( sheetPath == aPath )
            return sheet;

        sheet = GetNext();
    }

    return NULL;
}


void SCH_SHEET_LIST::BuildSheetList( SCH_SHEET* aSheet )
{
    if( m_List == NULL )
    {
        int count = aSheet->CountSheets();
        m_count = count;
        m_index = 0;
        m_List = new SCH_SHEET_PATH[ count ];
        m_currList.Clear();
    }

    m_currList.Push( aSheet );
    m_List[m_index] = m_currList;
    m_index++;

    if( aSheet->GetScreen() != NULL )
    {
        EDA_ITEM* strct = m_currList.LastDrawList();

        while( strct )
        {
            if( strct->Type() == SCH_SHEET_T )
            {
                SCH_SHEET* sheet = (SCH_SHEET*) strct;
                BuildSheetList( sheet );
            }

            strct = strct->Next();
        }
    }

    m_currList.Pop();
}


bool SCH_SHEET_LIST::IsModified()
{
    for( SCH_SHEET_PATH* sheet = GetFirst(); sheet != NULL; sheet = GetNext() )
    {
        if( sheet->LastScreen() && sheet->LastScreen()->IsModify() )
            return true;
    }

    return false;
}


bool SCH_SHEET_LIST::IsAutoSaveRequired()
{
    for( SCH_SHEET_PATH* sheet = GetFirst(); sheet != NULL; sheet = GetNext() )
    {
        if( sheet->LastScreen() && sheet->LastScreen()->IsSave() )
            return true;
    }

    return false;
}


void SCH_SHEET_LIST::ClearModifyStatus()
{
    for( SCH_SHEET_PATH* sheet = GetFirst(); sheet != NULL; sheet = GetNext() )
    {
        if( sheet->LastScreen() )
            sheet->LastScreen()->ClrModify();
    }
}


void SCH_SHEET_LIST::AnnotatePowerSymbols()
{
    int ref = 1;

    for( SCH_SHEET_PATH* path = GetFirst();  path != NULL;  path = GetNext() )
        path->AnnotatePowerSymbols( &ref );
}


void SCH_SHEET_LIST::GetComponents( SCH_REFERENCE_LIST& aReferences,
                                    bool                aIncludePowerSymbols )
{
    for( SCH_SHEET_PATH* path = GetFirst();  path != NULL;  path = GetNext() )
        path->GetComponents( aReferences, aIncludePowerSymbols );
}


SCH_ITEM* SCH_SHEET_LIST::FindNextItem( KICAD_T aType, SCH_SHEET_PATH** aSheetFoundIn,
                                        SCH_ITEM* aLastItem, bool aWrap )
{
    bool hasWrapped = false;
    bool firstItemFound = false;
    SCH_ITEM* drawItem = NULL;
    SCH_SHEET_PATH* sheet = GetFirst();

    while( sheet != NULL )
    {
        drawItem = sheet->LastDrawList();

        while( drawItem != NULL )
        {
            if( drawItem->Type() == aType )
            {
                if( aLastItem == NULL || firstItemFound )
                {
                    if( aSheetFoundIn )
                        *aSheetFoundIn = sheet;

                    return drawItem;
                }
                else if( !firstItemFound && drawItem == aLastItem )
                {
                    firstItemFound = true;
                }
            }

            drawItem = drawItem->Next();
        }

        sheet = GetNext();

        if( sheet == NULL && aLastItem && aWrap && !hasWrapped )
        {
            hasWrapped = true;
            sheet = GetFirst();
        }
    }

    return NULL;
}


SCH_ITEM* SCH_SHEET_LIST::FindPreviousItem( KICAD_T aType, SCH_SHEET_PATH** aSheetFoundIn,
                                            SCH_ITEM* aLastItem, bool aWrap )
{
    bool hasWrapped = false;
    bool firstItemFound = false;
    SCH_ITEM* drawItem = NULL;
    SCH_SHEET_PATH* sheet = GetLast();

    while( sheet != NULL )
    {
        drawItem = sheet->FirstDrawList();

        while( drawItem != NULL )
        {
            if( drawItem->Type() == aType )
            {
                if( aLastItem == NULL || firstItemFound )
                {
                    if( aSheetFoundIn )
                        *aSheetFoundIn = sheet;

                    return drawItem;
                }
                else if( !firstItemFound && drawItem == aLastItem )
                {
                    firstItemFound = true;
                }
            }

            drawItem = drawItem->Back();
        }

        sheet = GetPrevious();

        if( sheet == NULL && aLastItem && aWrap && !hasWrapped )
        {
            hasWrapped = true;
            sheet = GetLast();
        }
    }

    return NULL;
}


SCH_ITEM* SCH_SHEET_LIST::MatchNextItem( wxFindReplaceData& aSearchData,
                                         wxString&          aSheetFoundIn,
                                         SCH_ITEM*          aLastItem,
                                         wxPoint*           aFindLocation )
{
    bool firstItemFound = false;
    bool hasWrapped = false;
    bool wrap = ( aSearchData.GetFlags() & FR_SEARCH_WRAP ) != 0;
    SCH_ITEM* drawItem = NULL;
    SCH_SHEET_PATH* sheet = GetFirst();
    SCH_SHEET_PATH* sheetFirstItemFoundIn = NULL;

    wxLogTrace( traceFindReplace, wxT( "Searching schematic for " ) + aSearchData.GetFindString() );

    while( sheet != NULL )
    {
        wxLogTrace( traceFindReplace, wxT( "Searching sheet " + sheet->PathHumanReadable() ) );

        drawItem = sheet->LastDrawList();

        while( drawItem != NULL )
        {
            if( aLastItem && !firstItemFound )
            {
                if( aSheetFoundIn.IsEmpty() )
                    firstItemFound = (drawItem == aLastItem);
                else
                    firstItemFound = ( (drawItem == aLastItem) &&
                                       (sheet->PathHumanReadable() == aSheetFoundIn) );

                if( firstItemFound )
                {
                    sheetFirstItemFoundIn = sheet;

                    wxLogTrace( traceFindReplace, wxT( "First item %p found in sheet %s" ),
                                sheetFirstItemFoundIn,
                                GetChars( sheetFirstItemFoundIn->PathHumanReadable() ) );
                }
            }
            else
            {
                // Search has wrapped all the way around to the first item found so stop.
                if( hasWrapped && aLastItem && (aLastItem == drawItem)
                    && (sheet == sheetFirstItemFoundIn ) )
                {
                    wxLogTrace( traceFindReplace,
                                wxT( "Wrapped around to item %p in sheet %s" ),
                                sheetFirstItemFoundIn,
                                GetChars( sheetFirstItemFoundIn->PathHumanReadable() ) );

                    return NULL;
                }

                if( drawItem->Matches( aSearchData, sheet, aFindLocation ) )
                {
                    aSheetFoundIn = sheet->PathHumanReadable();

                    return drawItem;
                }
            }

            drawItem = drawItem->Next();
        }

        sheet = GetNext();

        if( sheet == NULL && aLastItem && firstItemFound && wrap && !hasWrapped )
        {
            hasWrapped = true;
            sheet = GetFirst();
        }
    }

    return NULL;
}


bool SCH_SHEET_LIST::SetComponentFootprint( const wxString& aReference,
                                            const wxString& aFootPrint, bool aSetVisible )
{
    bool found = false;

    for( SCH_SHEET_PATH* path = GetFirst();  path != NULL;  path = GetNext() )
        found = path->SetComponentFootprint( aReference, aFootPrint, aSetVisible );

    return found;
}
