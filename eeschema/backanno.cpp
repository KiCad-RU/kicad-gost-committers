/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2013 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file backanno.cpp
 * @brief Functions for backannotating footprint information.
 */

#include <fctsys.h>
#include <confirm.h>
#include <kicad_string.h>
#include <gestfich.h>
#include <appl_wxstruct.h>
#include <wxEeschemaStruct.h>
#include <build_version.h>
#include <wildcards_and_files_ext.h>

#include <general.h>
#include <sch_sheet_path.h>
#include <sch_component.h>
#include <netlist.h>



bool SCH_EDIT_FRAME::ProcessCmpToFootprintLinkFile( wxString& aFullFilename,
                                                    bool aForceFieldsVisibleAttribute,
                                                    bool aFieldsVisibleAttributeState )
{
    // Build a flat list of components in schematic:
    SCH_REFERENCE_LIST referencesList;
    SCH_SHEET_LIST SheetList;
    SheetList.GetComponents( referencesList, false );

    FILE* cmpFile = wxFopen( aFullFilename, wxT( "rt" ) );
    if( cmpFile == NULL )
        return false;

    // cmpFileReader dtor will close cmpFile
    FILE_LINE_READER cmpFileReader( cmpFile, aFullFilename );

    // Now, for each component found in file,
    // replace footprint field value by the new value:
    wxString reference;
    wxString footprint;
    wxString buffer;
    wxString value;

    while( cmpFileReader.ReadLine() )
    {
        buffer = FROM_UTF8( cmpFileReader.Line() );

        if( ! buffer.StartsWith( wxT("BeginCmp") ) )
            continue;

        // Begin component description.
        reference.Empty();
        footprint.Empty();

        while( cmpFileReader.ReadLine() )
        {
            buffer = FROM_UTF8( cmpFileReader.Line() );

            if( buffer.StartsWith( wxT("EndCmp") ) )
                break;

            // store string value, stored between '=' and ';' delimiters.
            value = buffer.AfterFirst( '=' );
            value = value.BeforeLast( ';');
            value.Trim(true);
            value.Trim(false);

            if( buffer.StartsWith( wxT("Reference") ) )
            {
                reference = value;
                continue;
            }

            if( buffer.StartsWith( wxT("IdModule  =" ) ) )
            {
                footprint = value;
                continue;
            }
        }

        // A block is read: initialize the footprint field of the correponding component
        // if the footprint name is not empty
        if( reference.IsEmpty() )
            continue;
        // Search the component in the flat list
        for( unsigned ii = 0; ii < referencesList.GetCount(); ii++ )
        {
            if( reference.CmpNoCase( referencesList[ii].GetRef() ) == 0 )
            {
                // We have found a candidate.
                // Note: it can be not unique (multiple parts per package)
                // So we *do not* stop the search here
                SCH_COMPONENT* component = referencesList[ii].GetComponent();
                SCH_FIELD * fpfield = component->GetField( FOOTPRINT );
                fpfield->m_Text = footprint;

                if( aForceFieldsVisibleAttribute )
                {
                    if( aFieldsVisibleAttributeState )
                        component->GetField( FOOTPRINT )->m_Attributs &= ~TEXT_NO_VISIBLE;
                    else
                        component->GetField( FOOTPRINT )->m_Attributs |= TEXT_NO_VISIBLE;
                }
            }
        }
    }
    return true;
}


bool SCH_EDIT_FRAME::LoadCmpToFootprintLinkFile()
{
    wxString path = wxGetCwd();

    wxFileDialog dlg( this, _( "Load Component-Footprint Link File" ),
                      path, wxEmptyString,
                      ComponentFileExtensionWildcard,
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    if( dlg.ShowModal() == wxID_CANCEL )
        return false;

    wxString filename = dlg.GetPath();
    wxString title  = wxGetApp().GetAppName() + wxT( " " ) + GetBuildVersion();
    title += wxT( " " ) + filename;
    SetTitle( title );

    int response = wxMessageBox( _( "Do you want to force all the footprint fields visibility?" ),
                                 _( "Field Visibility Change" ),
                                 wxYES_NO | wxICON_QUESTION | wxCANCEL, this );

    if( response == wxCANCEL )
        return false;

    bool changevisibility = response == wxYES;
    bool visible = false;

    if( changevisibility )
    {
        response = wxMessageBox( _( "Do you want to make all the footprint fields visible?" ),
                                     _( "Field Visibility Option" ),
                                     wxYES_NO | wxICON_QUESTION | wxCANCEL, this );
        if( response == wxCANCEL )
            return false;

        visible = response == wxYES;
    }

    if( ! ProcessCmpToFootprintLinkFile( filename, changevisibility, visible ) )
    {
        wxString msg;
        msg.Printf( _( "Failed to open component-footprint link file <%s>" ), filename.GetData() );
        DisplayError( this, msg );
        return false;
    }

    OnModify();
    return true;
}
