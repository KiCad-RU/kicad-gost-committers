/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012-2013 Alexander Lunev <al.lunev@yahoo.com>
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
#include <wx/xml/xml.h>
#include <common.h>

#include <pcad2kicad.h>
#include <bitmaps.h>

#include <s_expr_loader.h>
#include <sch.h>
#include <appl_wxstruct.h>
#include <build_version.h>
#include <wx/aboutdlg.h>

using namespace PCAD2KICAD;

/*
 *  PCad ASCII file description:
 *  www.eurointech.ru/products/Altium/PCAD_2006_ASCII.pdf
 */

void PCAD2KICAD_FRAME::OnLib( wxCommandEvent& event )
{
    SCH             sch;
    wxXmlDocument   xmlDoc;

    wxFileDialog    fileDlg( this, wxT( "Open lia file" ), wxEmptyString, wxEmptyString,
                             wxT( "P-Cad 200x ASCII schematic library |*.lia" ) );
    int             diag = fileDlg.ShowModal();

    if( diag != wxID_OK )
        return;

    LOCALE_IO toggle;    // toggles on, then off, the C locale.

    m_actualConversion = wxT( "SCHLIB" );

    wxString fileName = fileDlg.GetPath();
    m_inputFileName->SetLabel( fileName );

    LoadInputFile( fileName, &xmlDoc );
    sch.Parse( m_statusBar, &xmlDoc, m_actualConversion );

    m_statusBar->SetStatusText( wxT( "Generating output file.... " ) );
    wxFileName outFile( fileName );

    outFile.SetExt( wxT( "lib" ) );
    sch.WriteToFile( outFile.GetFullPath(), wxT( 'L' ) );

    m_statusBar->SetStatusText( wxT( "Done." ) );
}


void PCAD2KICAD_FRAME::OnSch( wxCommandEvent& event )
{
    SCH             sch;
    wxXmlDocument   xmlDoc;

    wxFileDialog    fileDlg( this, wxT( "Open sch file" ), wxEmptyString, wxEmptyString,
                             wxT( "P-Cad 200x ASCII schematic |*.sch" ) );
    int             diag = fileDlg.ShowModal();

    if( diag != wxID_OK )
        return;

    LOCALE_IO toggle;    // toggles on, then off, the C locale.

    m_actualConversion = wxT( "SCH" );

    wxString fileName = fileDlg.GetPath();
    m_inputFileName->SetLabel( fileName );

    LoadInputFile( fileName, &xmlDoc );
    sch.Parse( m_statusBar, &xmlDoc, m_actualConversion );

    m_statusBar->SetStatusText( wxT( "Generating output file.... " ) );
    wxFileName outFile( fileName );

    // we convert also library for schematics file
    outFile.SetExt( wxT( "KiCad.lib" ) );
    sch.WriteToFile( outFile.GetFullPath(), wxT( 'L' ) );
    outFile.SetExt( wxT( "KiCad" ) );
    sch.WriteToFile( outFile.GetFullPath(), wxT( 'S' ) );

    m_statusBar->SetStatusText( wxT( "Done." ) );
}


PCAD2KICAD_FRAME::PCAD2KICAD_FRAME( wxWindow* parent ) :
    PCAD2KICAD_FRAME_BASE( parent, wxID_ANY, wxGetApp().GetTitle() + wxT(" ") + GetBuildVersion() )
{
    // Give an icon
    // wxIcon icon;
    // icon.CopyFromBitmap( KiBitmap( icon_pcad2kicad_xpm ) );
    // SetIcon( icon );

    wxMenu *menu_help = new wxMenu;
    menu_help->Append( wxID_ABOUT, wxT( "&About pcad2kicadsch ..." ) );
    m_menubar->Append( menu_help, wxT( "&Help" ) );

    Connect( wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED,
             wxCommandEventHandler( PCAD2KICAD_FRAME::OnAbout ) );

    GetSizer()->SetSizeHints( this );

    // Set previous size and position
    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    if( m_FramePos == wxDefaultPosition )
        Centre();
}

void PCAD2KICAD_FRAME::OnAbout( wxCommandEvent& event )
{
    wxAboutDialogInfo info;

    wxString description;
    description
        << wxT( "An utility which allows schematic capture and schematic library file conversion "
                "from P-CAD 200x ASCII to KiCad." );

    wxString copyright;
    copyright
        << wxT( "(C) 2007, 2008 Lubo Racko <developer@lura.sk>\n" )
        << wxT( "(C) 2007, 2008, 2012-2013 Alexander Lunev <al.lunev@yahoo.com>\n" )
        << wxT( "(C) 1992-2013 KiCad Developers" );

    wxString license;
    license
        << wxT( "GNU General Public License (GPL) version 2\n" )
        << wxT( "http://www.gnu.org/licenses" );

    info.SetVersion( GetBuildVersion() );
    info.SetDescription( description );
    info.SetCopyright( copyright );
    info.SetLicense( license );

    info.SetWebSite( wxT( "https://code.launchpad.net/~pcad2kicad-committers/kicad/pcad2kicad" ) );

    wxAboutBox(info);
}

PCAD2KICAD_FRAME::~PCAD2KICAD_FRAME()
{
    /* This needed for OSX: avoids furter OnDraw processing after this
     * destructor and before the native window is destroyed
     */
    this->Freeze();
}
