///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "pcad2kicad_frame_base.h"

///////////////////////////////////////////////////////////////////////////

PCAD2KICAD_FRAME_BASE::PCAD2KICAD_FRAME_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menubar = new wxMenuBar( 0 );
	this->SetMenuBar( m_menubar );
	
	m_statusBar = this->CreateStatusBar( 1, wxST_SIZEGRIP, wxID_ANY );
	wxBoxSizer* bmainFrameSizer;
	bmainFrameSizer = new wxBoxSizer( wxVERTICAL );
	
	m_panel4 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	m_sch = new wxButton( m_panel4, wxID_ANY, _("Convert P-Cad 200x ASCII schematic file"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_sch, 0, wxALL|wxEXPAND, 5 );
	
	m_lib = new wxButton( m_panel4, wxID_ANY, _("Convert P-Cad 200x ASCII library file"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_lib, 0, wxALL|wxEXPAND, 5 );
	
	m_staticText1 = new wxStaticText( m_panel4, wxID_ANY, _("Actual input file :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer4->Add( m_staticText1, 0, wxALL, 5 );
	
	m_inputFileName = new wxStaticText( m_panel4, wxID_ANY, _("FileName"), wxDefaultPosition, wxDefaultSize, 0 );
	m_inputFileName->Wrap( -1 );
	bSizer4->Add( m_inputFileName, 0, wxALL, 5 );
	
	m_panel4->SetSizer( bSizer4 );
	m_panel4->Layout();
	bSizer4->Fit( m_panel4 );
	bmainFrameSizer->Add( m_panel4, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( bmainFrameSizer );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( PCAD2KICAD_FRAME_BASE::OnClosePcbCalc ) );
	m_sch->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PCAD2KICAD_FRAME_BASE::OnSch ), NULL, this );
	m_lib->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PCAD2KICAD_FRAME_BASE::OnLib ), NULL, this );
}

PCAD2KICAD_FRAME_BASE::~PCAD2KICAD_FRAME_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( PCAD2KICAD_FRAME_BASE::OnClosePcbCalc ) );
	m_sch->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PCAD2KICAD_FRAME_BASE::OnSch ), NULL, this );
	m_lib->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PCAD2KICAD_FRAME_BASE::OnLib ), NULL, this );
	
}
