///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec  4 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_footprint_wizard_list_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_FOOTPRINT_WIZARD_LIST_BASE::DIALOG_FOOTPRINT_WIZARD_LIST_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 400,300 ), wxDefaultSize );
	
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );
	
	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_panelGenerators = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizerpanelGen;
	bSizerpanelGen = new wxBoxSizer( wxVERTICAL );
	
	m_footprintGeneratorsGrid = new wxGrid( m_panelGenerators, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	m_footprintGeneratorsGrid->CreateGrid( 0, 3 );
	m_footprintGeneratorsGrid->EnableEditing( false );
	m_footprintGeneratorsGrid->EnableGridLines( true );
	m_footprintGeneratorsGrid->EnableDragGridSize( false );
	m_footprintGeneratorsGrid->SetMargins( 0, 0 );
	
	// Columns
	m_footprintGeneratorsGrid->SetColSize( 0, 40 );
	m_footprintGeneratorsGrid->SetColSize( 1, 160 );
	m_footprintGeneratorsGrid->SetColSize( 2, 325 );
	m_footprintGeneratorsGrid->EnableDragColMove( false );
	m_footprintGeneratorsGrid->EnableDragColSize( true );
	m_footprintGeneratorsGrid->SetColLabelSize( 20 );
	m_footprintGeneratorsGrid->SetColLabelValue( 0, wxEmptyString );
	m_footprintGeneratorsGrid->SetColLabelValue( 1, _("Name") );
	m_footprintGeneratorsGrid->SetColLabelValue( 2, _("Description") );
	m_footprintGeneratorsGrid->SetColLabelAlignment( wxALIGN_LEFT, wxALIGN_CENTRE );
	
	// Rows
	m_footprintGeneratorsGrid->AutoSizeRows();
	m_footprintGeneratorsGrid->EnableDragRowSize( true );
	m_footprintGeneratorsGrid->SetRowLabelSize( 1 );
	m_footprintGeneratorsGrid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_footprintGeneratorsGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	m_footprintGeneratorsGrid->SetMinSize( wxSize( 485,120 ) );
	
	bSizerpanelGen->Add( m_footprintGeneratorsGrid, 1, wxALL|wxEXPAND, 5 );
	
	
	m_panelGenerators->SetSizer( bSizerpanelGen );
	m_panelGenerators->Layout();
	bSizerpanelGen->Fit( m_panelGenerators );
	m_notebook->AddPage( m_panelGenerators, _("Available footprint generators"), true );
	m_panelInfo = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* m_bsizerPanelInfo;
	m_bsizerPanelInfo = new wxBoxSizer( wxVERTICAL );
	
	m_staticText1 = new wxStaticText( m_panelInfo, wxID_ANY, _("Search paths:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	m_bsizerPanelInfo->Add( m_staticText1, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_tcSearchPaths = new wxTextCtrl( m_panelInfo, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	m_tcSearchPaths->SetMinSize( wxSize( -1,60 ) );
	
	m_bsizerPanelInfo->Add( m_tcSearchPaths, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_staticText11 = new wxStaticText( m_panelInfo, wxID_ANY, _("Not loadable python scripts:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	m_bsizerPanelInfo->Add( m_staticText11, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_tcNotLoaded = new wxTextCtrl( m_panelInfo, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	m_tcNotLoaded->SetMinSize( wxSize( -1,60 ) );
	
	m_bsizerPanelInfo->Add( m_tcNotLoaded, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	m_panelInfo->SetSizer( m_bsizerPanelInfo );
	m_panelInfo->Layout();
	m_bsizerPanelInfo->Fit( m_panelInfo );
	m_notebook->AddPage( m_panelInfo, _("Messages"), false );
	
	bSizerMain->Add( m_notebook, 1, wxEXPAND | wxALL, 5 );
	
	m_staticline = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizerMain->Add( m_staticline, 0, wxEXPAND | wxALL, 5 );
	
	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton( this, wxID_OK );
	m_sdbSizer->AddButton( m_sdbSizerOK );
	m_sdbSizerCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer->AddButton( m_sdbSizerCancel );
	m_sdbSizer->Realize();
	
	bSizerMain->Add( m_sdbSizer, 0, wxEXPAND|wxALL, 5 );
	
	
	this->SetSizer( bSizerMain );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_footprintGeneratorsGrid->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( DIALOG_FOOTPRINT_WIZARD_LIST_BASE::OnCellFpGeneratorClick ), NULL, this );
	m_footprintGeneratorsGrid->Connect( wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler( DIALOG_FOOTPRINT_WIZARD_LIST_BASE::OnCellFpGeneratorDoubleClick ), NULL, this );
}

DIALOG_FOOTPRINT_WIZARD_LIST_BASE::~DIALOG_FOOTPRINT_WIZARD_LIST_BASE()
{
	// Disconnect Events
	m_footprintGeneratorsGrid->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( DIALOG_FOOTPRINT_WIZARD_LIST_BASE::OnCellFpGeneratorClick ), NULL, this );
	m_footprintGeneratorsGrid->Disconnect( wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler( DIALOG_FOOTPRINT_WIZARD_LIST_BASE::OnCellFpGeneratorDoubleClick ), NULL, this );
	
}
