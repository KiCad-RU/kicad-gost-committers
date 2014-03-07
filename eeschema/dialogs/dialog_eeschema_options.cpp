/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file dialog_eeschema_options.cpp
 */

#include <fctsys.h>
#include <class_base_screen.h>

#include <dialog_eeschema_options.h>


DIALOG_EESCHEMA_OPTIONS::DIALOG_EESCHEMA_OPTIONS( wxWindow* parent ) :
    DIALOG_EESCHEMA_OPTIONS_BASE( parent )
{
    m_choiceUnits->SetFocus();
    m_sdbSizer1OK->SetDefault();

    wxListItem col0;
    col0.SetId( 0 );
    col0.SetText( _( "ID" ) );
    col0.SetWidth( 50 );

    wxListItem col1;
    col1.SetId( 1 );
    col1.SetText( _( "Name" ) );
    col1.SetWidth( 150 );

    wxListItem col2;
    col2.SetId( 2 );
    col2.SetText( _( "Default Value" ) );
    col2.SetWidth( 250 );

    templateFieldListCtrl->InsertColumn( 0, col0 );
    templateFieldListCtrl->InsertColumn( 1, col1 );
    templateFieldListCtrl->InsertColumn( 2, col2 );
}


void DIALOG_EESCHEMA_OPTIONS::SetUnits( const wxArrayString& units, int select )
{
    wxASSERT( units.GetCount() > 0
              && ( select >= 0 && (size_t) select < units.GetCount() ) );

    m_choiceUnits->Append( units );
    m_choiceUnits->SetSelection( select );
}

void DIALOG_EESCHEMA_OPTIONS::SetRefIdSeparator( wxChar aSep, wxChar aFirstId)
{
    // m_choiceSeparatorRefId displays one of
    // "A" ".A" "-A" "_A" ".1" "-1" "_1" option

    int sel = 0;
    switch( aSep )
    {
        default:
        case 0:
            aFirstId = 'A';     // cannot use a number without separator
            break;

        case '.':
            sel = 1;
            break;

        case '-':
            sel = 2;
            break;

        case '_':
            sel = 3;
            break;
    }

    if( aFirstId == '1' )
        sel = 4;

    m_choiceSeparatorRefId->SetSelection( sel );
}

void DIALOG_EESCHEMA_OPTIONS::GetRefIdSeparator( int& aSep, int& aFirstId)
{
    // m_choiceSeparatorRefId displays one of
    // "A" ".A" "-A" "_A" ".1" "-1" "_1" option

    aFirstId = 'A';
    switch(  m_choiceSeparatorRefId->GetSelection() )
    {
        default:
        case 0: aSep = 0; break;
        case 1: aSep = '.'; break;
        case 2: aSep = '-'; break;
        case 3: aSep = '_'; break;
        case 4: aFirstId = '1'; aSep = '.'; break;
        case 5: aFirstId = '1'; aSep = '-'; break;
        case 6: aFirstId = '1'; aSep = '_'; break;
    }
}


void DIALOG_EESCHEMA_OPTIONS::SetGridSizes( const GRIDS& grid_sizes, int grid_id )
{
    wxASSERT( grid_sizes.size() > 0 );

    int select = wxNOT_FOUND;

    for( size_t i = 0; i < grid_sizes.size(); i++ )
    {
        wxString tmp;
        tmp.Printf( wxT( "%0.1f" ), grid_sizes[i].m_Size.x );
        m_choiceGridSize->Append( tmp );

        if( grid_sizes[i].m_Id == grid_id )
            select = (int) i;
    }

    m_choiceGridSize->SetSelection( select );
}


void DIALOG_EESCHEMA_OPTIONS::OnAddButtonClick( wxCommandEvent& event )
{
    long itemindex = templateFieldListCtrl->InsertItem( 0, wxT( "ID" ) );
    templateFieldListCtrl->SetItem( itemindex, 1, wxT( "Next Fieldname" ) );
    templateFieldListCtrl->SetItem( itemindex, 2, wxT( "A Default value" ) );

    event.Skip();
}


void DIALOG_EESCHEMA_OPTIONS::SetTemplateFields( const TEMPLATE_FIELDNAMES& aFields )
{
    printf( "SetTemplateFields %d\n", aFields.size() );

    for( size_t i = 0; i < aFields.size(); i++ )
    {
        long itemindex = templateFieldListCtrl->InsertItem( 0, wxT( "?" ) );
        templateFieldListCtrl->SetItem( itemindex, 1, aFields[i].m_Name );
        templateFieldListCtrl->SetItem( itemindex, 2, aFields[i].m_Value );
    }
}


TEMPLATE_FIELDNAMES* DIALOG_EESCHEMA_OPTIONS::GetTemplateFields( void )
{
    return &templateFields;
}

