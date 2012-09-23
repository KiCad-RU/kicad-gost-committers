/**
 * @file dialog_edit_one_field.cpp
 * @brief dialog to editing a field ( not a graphic text) in current component.
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2004-2012 KiCad Developers, see change_log.txt for contributors.
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
#include <common.h>
#include <base_units.h>

#include <general.h>
#include <sch_base_frame.h>
#include <sch_component.h>
#include <template_fieldnames.h>
#include <class_libentry.h>
#include <lib_field.h>
#include <sch_component.h>
#include <template_fieldnames.h>

#include <dialog_edit_one_field.h>


void DIALOG_EDIT_ONE_FIELD::initDlg_base()
{
    wxString msg;

    m_TextValue->SetFocus();

    // Disable options for graphic text edition, not existing in fields
    m_CommonConvert->Show(false);
    m_CommonUnit->Show(false);

    msg = ReturnStringFromValue( g_UserUnit, m_textsize );
    m_TextSize->SetValue( msg );

    if( m_textorient == TEXT_ORIENT_VERT )
        m_Orient->SetValue( true );

    m_Invisible->SetValue( m_text_invisible );
    m_TextShapeOpt->SetSelection( m_textshape );

    switch ( m_textHjustify )
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

    switch ( m_textVjustify )
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

    msg = m_TextSizeText->GetLabel() + ReturnUnitSymbol();
    m_TextSizeText->SetLabel( msg );

    m_sdbSizerButtonsOK->SetDefault();
}


void DIALOG_LIB_EDIT_ONE_FIELD::initDlg()
{
    m_textsize = m_field->m_Size.x;
    m_TextValue->SetValue( m_field->m_Text );

    m_textorient = m_field->GetOrientation();

    m_text_invisible = m_field->IsVisible() ? false : true;

    m_textshape = 0;
    if( m_field->m_Italic )
        m_textshape = 1;
    if( m_field->m_Bold )
        m_textshape |= 2;

    m_textHjustify = m_field->m_HJustify;
    m_textVjustify =  m_field->m_VJustify;

    initDlg_base();
}

wxString DIALOG_LIB_EDIT_ONE_FIELD::GetTextField()
{
    wxString line = m_TextValue->GetValue();
    // Spaces are not allowed in fields, so replace them by '_'
    line.Replace( wxT( " " ), wxT( "_" ) );
    return line;
};

void DIALOG_EDIT_ONE_FIELD::TransfertDataToField()
{
    m_textorient = m_Orient->GetValue() ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ;
    wxString msg = m_TextSize->GetValue();
    m_textsize = ReturnValueFromString( g_UserUnit, msg );

    switch( m_TextHJustificationOpt->GetSelection() )
    {
    case 0:
        m_textHjustify = GR_TEXT_HJUSTIFY_LEFT;
        break;

    case 1:
        m_textHjustify = GR_TEXT_HJUSTIFY_CENTER;
        break;

    case 2:
        m_textHjustify = GR_TEXT_HJUSTIFY_RIGHT;
        break;
    }

    switch( m_TextVJustificationOpt->GetSelection() )
    {
    case 0:
        m_textVjustify = GR_TEXT_VJUSTIFY_BOTTOM;
        break;

    case 1:
        m_textVjustify = GR_TEXT_VJUSTIFY_CENTER;
        break;

    case 2:
        m_textVjustify = GR_TEXT_VJUSTIFY_TOP;
        break;
    }
}

void DIALOG_LIB_EDIT_ONE_FIELD::TransfertDataToField()
{
    DIALOG_EDIT_ONE_FIELD::TransfertDataToField();

    m_field->SetText( GetTextField() );

    m_field->m_Size.x = m_field->m_Size.y = m_textsize;
    m_field->m_Orient = m_textorient;

    if( m_Invisible->GetValue() )
        m_field->m_Attributs |= TEXT_NO_VISIBLE;
    else
        m_field->m_Attributs &= ~TEXT_NO_VISIBLE;

    if( ( m_TextShapeOpt->GetSelection() & 1 ) != 0 )
        m_field->m_Italic = true;
    else
        m_field->m_Italic = false;

    if( ( m_TextShapeOpt->GetSelection() & 2 ) != 0 )
        m_field->m_Bold = true;
    else
        m_field->m_Bold = false;

    m_field->m_HJustify = m_textHjustify;
    m_field->m_VJustify = m_textVjustify;
}


void DIALOG_SCH_EDIT_ONE_FIELD::initDlg()
{
    m_textsize = m_field->m_Size.x;
    m_TextValue->SetValue( m_field->m_Text );
    m_textorient = m_field->GetOrientation();
    m_text_invisible = m_field->IsVisible() ? false : true;

    m_textshape = 0;
    if( m_field->m_Italic )
        m_textshape = 1;
    if( m_field->m_Bold )
        m_textshape |= 2;

    m_textHjustify = m_field->m_HJustify;
    m_textVjustify =  m_field->m_VJustify;

    initDlg_base();
}


wxString DIALOG_SCH_EDIT_ONE_FIELD::GetTextField()
{
    wxString line = m_TextValue->GetValue();
    line.Trim( true );
    line.Trim( false );
    return line;
};

void DIALOG_SCH_EDIT_ONE_FIELD::TransfertDataToField()
{
    DIALOG_EDIT_ONE_FIELD::TransfertDataToField();

    m_field->SetText( GetTextField() );

    m_field->m_Size.x = m_field->m_Size.y = m_textsize;
    m_field->m_Orient = m_textorient;

    if( m_Invisible->GetValue() )
        m_field->m_Attributs |= TEXT_NO_VISIBLE;
    else
        m_field->m_Attributs &= ~TEXT_NO_VISIBLE;

    if( ( m_TextShapeOpt->GetSelection() & 1 ) != 0 )
        m_field->m_Italic = true;
    else
        m_field->m_Italic = false;

    if( ( m_TextShapeOpt->GetSelection() & 2 ) != 0 )
        m_field->m_Bold = true;
    else
        m_field->m_Bold = false;

    m_field->m_HJustify = m_textHjustify;
    m_field->m_VJustify = m_textVjustify;
}
