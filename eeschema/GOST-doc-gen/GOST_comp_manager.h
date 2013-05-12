/* This program source code file is part of KiCad, a free EDA CAD application.
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
 * @file GOST_comp_manager.h
 */

#ifndef _GOST_COMP_MANAGER_H_
#define _GOST_COMP_MANAGER_H_

#include <netlist.h>
#include <wxEeschemaStruct.h>
#include <dialogs/frame_GOST_doc_gen_base.h>

#define TABLE_REFDES_COL       0
#define TABLE_NAME_COL         1
#define TABLE_NOTE_COL         2
#define TABLE_FORVARIANTS_COL  3

namespace GOST_DOC_GEN {
    class COMPONENT_DB;
}

class GOST_COMP_MANAGER : public FRAME_GOST_DOC_GEN_BASE
{
private:
    SCH_EDIT_FRAME* m_schEditFrame;
    GOST_DOC_GEN::COMPONENT_DB* m_componentDB;
    bool m_onEditChangeComboBoxLock;
    bool m_onItemChangedCheckListCtrlLock;

public:
    GOST_COMP_MANAGER( wxWindow* parent );
    ~GOST_COMP_MANAGER();

private:
    void OnEditChangeComboName( wxCommandEvent& event );
    void OnEditChangeComboType( wxCommandEvent& event );
    void OnEditChangeComboSubtype( wxCommandEvent& event );
    void OnEditChangeComboValue( wxCommandEvent& event );
    void OnEditChangeComboPrecision( wxCommandEvent& event );
    void OnEditChangeComboNote( wxCommandEvent& event );
    void OnEditChangeComboDesignation( wxCommandEvent& event );
    void OnEditChangeComboManufacturer( wxCommandEvent& event );

    void OnClickListCtrl( wxListEvent& event );
    void OnItemChangedCheckListCtrl( wxCommandEvent& event );
    void OnRadioFullList( wxCommandEvent& event );
    void OnRadioConstPart( wxCommandEvent& event );
    void OnRadioVarPart(wxCommandEvent& event);
    void OnSelChangeComboVariant( wxCommandEvent& event );
    void OnSettingsAddaNewVariant( wxCommandEvent& event );

    void OnFileGenerateComponentIndex( wxCommandEvent& event );
    void OnFileGenerateSpecification( wxCommandEvent& event );

    void OnActivate( wxActivateEvent& event );
    void OnCloseWindow( wxCloseEvent& event );
    void OnSize( wxSizeEvent & event );

    void DisableComboBoxes();
    void FormCheckListBoxContent( int aDBitem );
    void FormFullListString( int aItem, int aList_str_ind, bool aNew_string );
    void FormConstPartString( int aItem, int aList_str_ind, bool aNew_string );
    void FormVarPartString( int aItem, int aList_str_ind, bool aNew_string );
    void FormVariantList();
    void EditAttribute( int aComp_attr, wxString aStr );
    //void UpdateListCtrlContent();
};

#endif    // _GOST_COMP_MANAGER_H_
