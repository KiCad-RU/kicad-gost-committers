/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2012 KiCad Developers, see CHANGELOG.TXT for contributors.
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
 * @file pcad2kicad_frame.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <wx/filename.h>

#include <pcad2kicad.h>
#include <bitmaps.h>

#include <LoadInputFile.h>
#include <pcb.h>
#include <sch.h>
#include <TextToXMLUnit.h>

/*
 *  PCad ASCII file description:
 *  www.eltm.ru/store/Altium/PCAD_2006_ASCII.pdf
 */

void PCAD2KICAD_FRAME::OnPcb( wxCommandEvent& event )
{
    PCB             pcb;
    wxArrayString   lines;

    wxFileDialog    fileDlg( this, wxT( "Open PCB file" ), wxEmptyString, wxEmptyString, wxT(
                                 "PCad PCB Board ASCII |*.pcb|PCad PCB Library ASCII |*.lia" ) );
    int             diag = fileDlg.ShowModal();

    if( diag != wxID_OK )
        return;

    m_actualConversion = wxT( "PCB" );
    wxString fileName = fileDlg.GetPath();
    m_inputFileName->SetLabel( fileName );

    LoadInputFile( fileName, m_statusBar, &lines );
    wxFileName xmlFile( fileName );
    xmlFile.SetExt( wxT( "xml" ) );
    TextToXML( m_statusBar, xmlFile.GetFullPath(), &lines );
    pcb.Parse( m_statusBar, xmlFile.GetFullPath(), m_actualConversion );

    m_statusBar->SetStatusText( wxT( "Generating output file.... " ) );
    wxFileName outFile( fileName );

    if( fileDlg.GetFilterIndex() == 1 )
    {
        outFile.SetExt( wxT( "mod" ) );
        pcb.WriteToFile( outFile.GetFullPath(), 'L' );
    }
    else
    {
        outFile.SetExt( wxT( "brd" ) );
        pcb.WriteToFile( outFile.GetFullPath(), 'P' );
    }

    m_statusBar->SetStatusText( wxT( "Done." ) );
    m_actualConversion = wxEmptyString;
}


void PCAD2KICAD_FRAME::OnSch( wxCommandEvent& event )
{
    SCH             sch;
    wxArrayString   lines;

    wxFileDialog    fileDlg( this, wxT( "Open sch file" ), wxEmptyString, wxEmptyString, wxT(
                                 "PCad SCH Schematics ASCII |*.sch|PCad SCH Library ASCII |*.lia" ) );
    int             diag = fileDlg.ShowModal();

    if( diag != wxID_OK )
        return;

    m_actualConversion = wxT( "SCH" );

    if( fileDlg.GetFilterIndex() == 1 )
        m_actualConversion = wxT( "SCHLIB" );

    wxString fileName = fileDlg.GetPath();
    m_inputFileName->SetLabel( fileName );

    LoadInputFile( fileName, m_statusBar, &lines );
    wxFileName xmlFile( fileName );
    xmlFile.SetExt( wxT( "xml" ) );
    TextToXML( m_statusBar, xmlFile.GetFullPath(), &lines );
    sch.Parse( m_statusBar, xmlFile.GetFullPath(), m_actualConversion );

    m_statusBar->SetStatusText( wxT( "Generating output file.... " ) );
    wxFileName outFile( fileName );

    if( fileDlg.GetFilterIndex() == 1 )
    {
        outFile.SetExt( wxT( "lib" ) );
        sch.WriteToFile( outFile.GetFullPath(), 'L' );
    }
    else
    {
        // we convert also library for schematics file
        outFile.SetExt( wxT( "KiCad.lib" ) );
        sch.WriteToFile( outFile.GetFullPath(), 'L' );
        outFile.SetExt( wxT( "KiCad.sch" ) );
        sch.WriteToFile( outFile.GetFullPath(), 'S' );
    }

    m_statusBar->SetStatusText( wxT( "Done." ) );
    m_actualConversion = wxEmptyString;
}


PCAD2KICAD_FRAME::PCAD2KICAD_FRAME( wxWindow* parent ) :
    PCAD2KICAD_FRAME_BASE( parent )
{
    // Give an icon
    // wxIcon icon;
    // icon.CopyFromBitmap( KiBitmap( icon_pcad2kicad_xpm ) );
    // SetIcon( icon );

    GetSizer()->SetSizeHints( this );

    // Set previous size and position
    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    if( m_FramePos == wxDefaultPosition )
        Centre();
}


PCAD2KICAD_FRAME::~PCAD2KICAD_FRAME()
{
    /* This needed for OSX: avoids furter OnDraw processing after this
     * destructor and before the native window is destroyed
     */
    this->Freeze();
}
