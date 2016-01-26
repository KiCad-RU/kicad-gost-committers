/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Cirilo Bernardo <cirilo.bernardo@gmail.com>
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


#include "dlg_select_3dmodel.h"
#include "project.h"
#include "3d_cache/3d_info.h"
#include "3d_cache/3d_cache.h"
#include "3d_cache/dialogs/panel_prev_model.h"
#include "3d_cache_dialogs.h"

#define ID_FILE_TREE    ( wxID_LAST + 1 )
#define ID_SET_DIR      ( ID_FILE_TREE + 1 )
#define ID_CFG_PATHS    ( ID_SET_DIR + 1 )


wxBEGIN_EVENT_TABLE( DLG_SELECT_3DMODEL, wxDialog )
    EVT_DIRCTRL_SELECTIONCHANGED( ID_FILE_TREE, DLG_SELECT_3DMODEL::OnSelectionChanged )
    EVT_DIRCTRL_FILEACTIVATED( ID_FILE_TREE, DLG_SELECT_3DMODEL::OnFileActivated )
    EVT_CHOICE( ID_SET_DIR, DLG_SELECT_3DMODEL::SetRootDir )
    EVT_BUTTON( ID_CFG_PATHS, DLG_SELECT_3DMODEL::Cfg3DPaths )
wxEND_EVENT_TABLE()


DLG_SELECT_3DMODEL::DLG_SELECT_3DMODEL( wxWindow* aParent, S3D_CACHE* aCacheManager,
    S3D_INFO* aModelItem, wxString& prevModelSelectDir, int& prevModelWildcard ) :
    wxDialog( aParent, wxID_ANY, _( "Select 3D Model" ), wxDefaultPosition,
             wxSize( 500,200 ), wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX
             | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxSYSTEM_MENU ),
    m_model( aModelItem ), m_cache( aCacheManager ), m_previousDir( prevModelSelectDir ),
    m_previousFilterIndex( prevModelWildcard )
{
    this->SetSizeHints( wxSize( 500,200 ), wxDefaultSize );

    if( NULL != m_cache )
        m_resolver = m_cache->GetResolver();
    else
        m_resolver = NULL;

    wxBoxSizer* bSizer0 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer( wxVERTICAL );

    // set to NULL to avoid segfaults when m_FileTree is instantiated
    // and wxGenericDirCtrl events are posted
    m_preview = NULL;
    dirChoices = NULL;

    m_FileTree = new wxGenericDirCtrl( this, ID_FILE_TREE, prevModelSelectDir, wxDefaultPosition,
        wxSize( 300,100 ), wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_EDIT_LABELS
        | wxDIRCTRL_SELECT_FIRST | wxDIRCTRL_SHOW_FILTERS|wxSUNKEN_BORDER, wxEmptyString, 0 );


    m_FileTree->ShowHidden( false );
    m_FileTree->SetMinSize( wxSize( 300,100 ) );
    m_FileTree->SetLabel( wxT( "3D_MODEL_SELECTOR" ) );

    bSizer2->Add( m_FileTree, 1, wxEXPAND | wxALL, 5 );
    bSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

    // m_preview must me instantiated after m_FileTree or else it will not
    // function as desired due to the constructor depending on the existence
    // of m_FileTree to determine the previewer's configuration
    wxBoxSizer* previewSizer;
    previewSizer = new wxBoxSizer( wxVERTICAL );
    m_preview = new PANEL_PREV_3D( this, m_cache );
    previewSizer->Add( m_preview, 1, wxEXPAND | wxALL, 5 );
    bSizer1->Add( previewSizer, 0, wxEXPAND, 5 );

    // create the filter list
    if( NULL != m_cache )
    {
        std::list< wxString > const* fl = m_cache->GetFileFilters();
        std::list< wxString >::const_iterator sL = fl->begin();
        std::list< wxString >::const_iterator eL = fl->end();
        wxString filter;

        while( sL != eL )
        {
            filter.Append( *sL );

            ++sL;

            if( sL != eL )
                filter.Append( wxT( "|" ) );

        }

        if( !filter.empty() )
            m_FileTree->SetFilter( filter );
        else
            m_FileTree->SetFilter( wxT( "*.*" ) );

        if( prevModelWildcard >= 0 && prevModelWildcard < (int)fl->size() )
            m_FileTree->SetFilterIndex( prevModelWildcard );
        else
        {
            prevModelWildcard = 0;
            m_FileTree->SetFilterIndex( 0 );
        }
    }
    else
    {
        m_FileTree->SetFilter( wxT( "*.*" ) );
        prevModelWildcard = 0;
        m_FileTree->SetFilterIndex( 0 );
    }

    // Add the path choice box and config button
    wxBoxSizer* hboxDirChoice = new wxBoxSizer( wxHORIZONTAL );
    dirChoices = new wxChoice( this, ID_SET_DIR, wxDefaultPosition,
        wxSize( 320, 20 ) );
    dirChoices->SetMinSize( wxSize( 320, 12 ) );

    wxStaticText* stDirChoice = new wxStaticText( this, -1, _( "Paths:" ) );
    wxButton* cfgPaths = new wxButton( this, ID_CFG_PATHS, _( "Configure Paths" ) );
    hboxDirChoice->Add( stDirChoice, 0, wxALL | wxCENTER, 5 );
    hboxDirChoice->Add( dirChoices, 1, wxEXPAND | wxALL, 5 );
    hboxDirChoice->Add( cfgPaths, 0, wxALL, 5 );

    wxButton* btn_OK = new wxButton( this, wxID_OK, _T( "OK" ) );
    wxButton* btn_Cancel = new wxButton( this, wxID_CANCEL, _T( "Cancel" ) );

    wxStdDialogButtonSizer* hSizer1 = new wxStdDialogButtonSizer();
    hSizer1->AddButton( btn_OK );
    hSizer1->AddButton( btn_Cancel );
    hSizer1->Realize();
    bSizer0->Add( bSizer1, 1, wxALL | wxEXPAND, 5 );
    bSizer0->Add( hboxDirChoice, 0, wxALL | wxEXPAND, 5 );
    bSizer0->Add( hSizer1, 0, wxALL | wxEXPAND, 5 );

    updateDirChoiceList();

    this->SetSizerAndFit( bSizer0 );
    this->Layout();
    this->Centre( wxBOTH );
}


bool DLG_SELECT_3DMODEL::TransferDataFromWindow()
{
    if( NULL == m_model || NULL == m_FileTree )
        return true;

    m_model->scale.x = 1.0;
    m_model->scale.y = 1.0;
    m_model->scale.z = 1.0;

    m_model->rotation.x = 0.0;
    m_model->rotation.y = 0.0;
    m_model->rotation.z = 0.0;

    m_model->offset = m_model->rotation;
    m_model->filename.empty();

    wxString fname = m_FileTree->GetFilePath();

    if( fname.empty() )
        return true;

    m_previousDir = m_FileTree->GetPath();
    m_previousFilterIndex = m_FileTree->GetFilterIndex();

    m_preview->GetModelData( m_model );

    return true;
}


void DLG_SELECT_3DMODEL::OnSelectionChanged( wxTreeEvent& event )
{
    if( NULL != m_preview )
        m_preview->UpdateModelName( m_FileTree->GetFilePath() );

    event.Skip();
    return;
}


void DLG_SELECT_3DMODEL::OnFileActivated( wxTreeEvent& event )
{
    if( NULL != m_preview )
        m_preview->UpdateModelName( m_FileTree->GetFilePath() );

    event.Skip();
    SetEscapeId( wxID_OK );
    Close();

    return;
}


void DLG_SELECT_3DMODEL::SetRootDir( wxCommandEvent& event )
{
    if( !m_FileTree )
        return;

    m_FileTree->SetPath( dirChoices->GetString( dirChoices->GetSelection() ) );

    return;
}


void DLG_SELECT_3DMODEL::Cfg3DPaths( wxCommandEvent& event )
{
    if( S3D::Configure3DPaths( this, m_resolver ) )
        updateDirChoiceList();
}


void DLG_SELECT_3DMODEL::updateDirChoiceList( void )
{
    if( NULL == m_FileTree || NULL == m_resolver || NULL == dirChoices )
        return;

    std::list< S3D_ALIAS > const* md = m_resolver->GetPaths();
    std::list< S3D_ALIAS >::const_iterator sL = md->begin();
    std::list< S3D_ALIAS >::const_iterator eL = md->end();
    std::vector< wxString > cl;

    while( sL != eL )
    {
        if( !sL->m_pathexp.empty() && !sL->m_duplicate )
            cl.push_back( sL->m_pathexp );

        ++sL;
    }

    if( !cl.empty() )
    {
        dirChoices->Clear();
        dirChoices->Append( (int)cl.size(), &cl[0] );
        dirChoices->Select( 0 );
    }

    return;
}
