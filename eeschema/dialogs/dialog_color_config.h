/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007 G. Harland
 * Copyright (C) 1992-2014 KiCad Developers, see CHANGELOG.TXT for contributors.
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

#ifndef DIALOG_COLOR_CONFIG_H_
#define DIALOG_COLOR_CONFIG_H_

#include <wx/statline.h>


class wxBoxSizer;
class wxStaticLine;
class wxStdDialogButtonSizer;


extern void SeedLayers();


/***********************************************/
/* Derived class for the frame color settings. */
/***********************************************/

class DIALOG_COLOR_CONFIG : public wxDialog
{
private:
    DECLARE_DYNAMIC_CLASS( DIALOG_COLOR_CONFIG )

    EDA_DRAW_FRAME*         m_parent;
    wxBoxSizer*             m_outerBoxSizer;
    wxBoxSizer*             m_mainBoxSizer;
    wxBoxSizer*             m_columnBoxSizer;
    wxBoxSizer*             m_rowBoxSizer;
    wxBitmapButton*         m_bitmapButton;
    wxRadioBox*             m_SelBgColor;
    wxStaticLine*           m_line;
    wxStdDialogButtonSizer* m_stdDialogButtonSizer;

    // Creation
    bool Create( wxWindow* aParent,
                 wxWindowID aId = wxID_ANY,
                 const wxString& aCaption =  _( "EESchema Colors" ),
                 const wxPoint& aPosition = wxDefaultPosition,
                 const wxSize& aSize = wxDefaultSize,
                 long aStyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

    // Initializes member variables
    void Init();

    // Creates the controls and sizers
    void CreateControls();

    wxBitmap GetBitmapResource( const wxString& aName );
    wxIcon   GetIconResource( const wxString& aName );
    static bool ShowToolTips();

    bool    UpdateColorsSettings();
    void    SetColor( wxCommandEvent& aEvent );
    void    OnOkClick( wxCommandEvent& aEvent );
    void    OnCancelClick( wxCommandEvent& aEvent );
    void    OnApplyClick( wxCommandEvent& aEvent );

public:
    // Constructors and destructor
    DIALOG_COLOR_CONFIG();
    DIALOG_COLOR_CONFIG( EDA_DRAW_FRAME* aParent );
    ~DIALOG_COLOR_CONFIG();
};

#endif    // DIALOG_COLOR_CONFIG_H_
