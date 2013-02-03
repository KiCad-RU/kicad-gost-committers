/**
 * @file dialog_lib_edit_text.cpp
 * @brief dialog to editing graphic texts (not fields) in body components.
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 20011 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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

#include <fctsys.h>
#include <gr_basic.h>
#include <common.h>
#include <class_drawpanel.h>
#include <base_units.h>

#include <general.h>
#include <libeditframe.h>
#include <class_libentry.h>
#include <lib_text.h>

#include <dialog_lib_edit_text.h>


DIALOG_LIB_EDIT_TEXT::DIALOG_LIB_EDIT_TEXT( LIB_EDIT_FRAME* aParent, LIB_TEXT* aText ) :
    DIALOG_LIB_EDIT_TEXT_BASE( aParent )
{
    m_parent = aParent;
    m_graphicText = aText;
    initDlg();

    GetSizer()->SetSizeHints(this);
    Centre();
}


void DIALOG_LIB_EDIT_TEXT::initDlg( )
{
    wxString msg;

    m_TextValue->SetFocus();

    // Disable options for fieldedition, not existing in  graphic text
    m_Invisible->Show(false);

    if ( m_graphicText )
    {
        msg = ReturnStringFromValue( g_UserUnit, m_graphicText->m_Size.x );
        m_TextSize->SetValue( msg );
        m_TextValue->SetValue( m_graphicText->m_Text );

        if ( m_graphicText->GetUnit() == 0 )
            m_CommonUnit->SetValue( true );
        if ( m_graphicText->GetConvert() == 0 )
            m_CommonConvert->SetValue( true );
        if ( m_graphicText->m_Orient == TEXT_ORIENT_VERT )
            m_Orient->SetValue( true );

        int shape = 0;
        if ( m_graphicText->m_Italic )
            shape = 1;
        if ( m_graphicText->m_Bold )
            shape |= 2;

        m_TextShapeOpt->SetSelection( shape );

        switch ( m_graphicText->m_HJustify )
        {
            case GR_TEXT_HJUSTIFY_LEFT:
                m_TextHJustificationOpt->SetSelection( 0 );
                break;

            case GR_TEXT_HJUSTIFY_CENTER:
                m_TextHJustificationOpt->SetSelection( 1 );
                break;

            case GR_TEXT_HJUSTIFY_RIGHT:
                m_TextHJustificationOpt->SetSelection( 2 );
                break;

        }

        switch ( m_graphicText->m_VJustify )
        {
        case GR_TEXT_VJUSTIFY_BOTTOM:
            m_TextVJustificationOpt->SetSelection( 0 );
            break;

        case GR_TEXT_VJUSTIFY_CENTER:
            m_TextVJustificationOpt->SetSelection( 1 );
            break;

        case GR_TEXT_VJUSTIFY_TOP:
            m_TextVJustificationOpt->SetSelection( 2 );
            break;
        }
    }
    else
    {
        msg = ReturnStringFromValue( g_UserUnit, m_parent->m_textSize );
        m_TextSize->SetValue( msg );

        if ( ! m_parent->m_drawSpecificUnit )
            m_CommonUnit->SetValue( true );
        if ( ! m_parent->m_drawSpecificConvert )
            m_CommonConvert->SetValue( true );
        if ( m_parent->m_textOrientation == TEXT_ORIENT_VERT )
            m_Orient->SetValue( true );
    }

    msg = m_TextSizeText->GetLabel() + ReturnUnitSymbol();
    m_TextSizeText->SetLabel( msg );

    m_sdbSizerButtonsOK->SetDefault();
}


void DIALOG_LIB_EDIT_TEXT::OnCancelClick( wxCommandEvent& event )
{
    EndModal(wxID_CANCEL);
}


/* Updates the different parameters for the component being edited */
void DIALOG_LIB_EDIT_TEXT::OnOkClick( wxCommandEvent& event )
{
    wxString Line;

    Line = m_TextValue->GetValue();
    m_parent->m_textOrientation = m_Orient->GetValue() ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ;
    wxString msg = m_TextSize->GetValue();
    m_parent->m_textSize = ReturnValueFromString( g_UserUnit, msg );
    m_parent->m_drawSpecificConvert = m_CommonConvert->GetValue() ? false : true;
    m_parent->m_drawSpecificUnit = m_CommonUnit->GetValue() ? false : true;

    if( m_graphicText )
    {
        if( ! Line.IsEmpty() )
            m_graphicText->SetText( Line );
        else
            m_graphicText->SetText( wxT( "[null]" ) );

        m_graphicText->m_Size.x = m_graphicText->m_Size.y = m_parent->m_textSize;
        m_graphicText->m_Orient = m_parent->m_textOrientation;

        if( m_parent->m_drawSpecificUnit )
            m_graphicText->SetUnit( m_parent->GetUnit() );
        else
            m_graphicText->SetUnit( 0 );

        if( m_parent->m_drawSpecificConvert )
            m_graphicText->SetConvert( m_parent->GetConvert() );
        else
            m_graphicText->SetConvert( 0 );

        if( ( m_TextShapeOpt->GetSelection() & 1 ) != 0 )
            m_graphicText->m_Italic = true;
        else
            m_graphicText->m_Italic = false;

        if( ( m_TextShapeOpt->GetSelection() & 2 ) != 0 )
            m_graphicText->m_Bold = true;
        else
            m_graphicText->m_Bold = false;

        switch( m_TextHJustificationOpt->GetSelection() )
        {
        case 0:
            m_graphicText->m_HJustify = GR_TEXT_HJUSTIFY_LEFT;
            break;

        case 1:
            m_graphicText->m_HJustify = GR_TEXT_HJUSTIFY_CENTER;
            break;

        case 2:
            m_graphicText->m_HJustify = GR_TEXT_HJUSTIFY_RIGHT;
            break;
        }

        switch( m_TextVJustificationOpt->GetSelection() )
        {
        case 0:
            m_graphicText->m_VJustify = GR_TEXT_VJUSTIFY_BOTTOM;
            break;

        case 1:
            m_graphicText->m_VJustify = GR_TEXT_VJUSTIFY_CENTER;
            break;

        case 2:
            m_graphicText->m_VJustify = GR_TEXT_VJUSTIFY_TOP;
            break;
        }
    }

    if( m_parent->GetDrawItem() )
        m_parent->SetMsgPanel( m_parent->GetDrawItem() );

    EndModal(wxID_OK);
}
