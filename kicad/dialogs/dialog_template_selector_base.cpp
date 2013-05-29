///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_template_selector_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_TEMPLATE_SELECTOR_BASE::DIALOG_TEMPLATE_SELECTOR_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 640,480 ), wxDefaultSize );
	
	wxBoxSizer* bmainSizer;
	bmainSizer = new wxBoxSizer( wxVERTICAL );
	
	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	bmainSizer->Add( m_notebook, 0, wxEXPAND | wxALL, 3 );
	
	m_htmlWin = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxHW_SCROLLBAR_AUTO );
	bmainSizer->Add( m_htmlWin, 1, wxALL|wxEXPAND, 3 );
	
	m_staticTextTpath = new wxStaticText( this, wxID_ANY, wxT("Templates path"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextTpath->Wrap( -1 );
	bmainSizer->Add( m_staticTextTpath, 0, wxRIGHT|wxLEFT, 5 );
	
	m_textCtrlTemplatePath = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bmainSizer->Add( m_textCtrlTemplatePath, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton( this, wxID_OK );
	m_sdbSizer->AddButton( m_sdbSizerOK );
	m_sdbSizerCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer->AddButton( m_sdbSizerCancel );
	m_sdbSizer->Realize();
	
	bmainSizer->Add( m_sdbSizer, 0, wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bmainSizer );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_notebook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( DIALOG_TEMPLATE_SELECTOR_BASE::OnPageChange ), NULL, this );
}

DIALOG_TEMPLATE_SELECTOR_BASE::~DIALOG_TEMPLATE_SELECTOR_BASE()
{
	// Disconnect Events
	m_notebook->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( DIALOG_TEMPLATE_SELECTOR_BASE::OnPageChange ), NULL, this );
	
}

TEMPLATE_SELECTION_PANEL_BASE::TEMPLATE_SELECTION_PANEL_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	m_SizerBase = new wxBoxSizer( wxHORIZONTAL );
	
	m_scrolledWindow1 = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL );
	m_scrolledWindow1->SetScrollRate( 5, 5 );
	m_SizerChoice = new wxBoxSizer( wxHORIZONTAL );
	
	
	m_scrolledWindow1->SetSizer( m_SizerChoice );
	m_scrolledWindow1->Layout();
	m_SizerChoice->Fit( m_scrolledWindow1 );
	m_SizerBase->Add( m_scrolledWindow1, 0, wxEXPAND | wxALL, 3 );
	
	
	this->SetSizer( m_SizerBase );
	this->Layout();
}

TEMPLATE_SELECTION_PANEL_BASE::~TEMPLATE_SELECTION_PANEL_BASE()
{
}

TEMPLATE_WIDGET_BASE::TEMPLATE_WIDGET_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetMinSize( wxSize( 74,-1 ) );
	this->SetMaxSize( wxSize( 74,-1 ) );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	m_bitmapIcon = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 64,64 ), 0 );
	bSizer4->Add( m_bitmapIcon, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	m_staticTitle = new wxStaticText( this, wxID_ANY, wxT("Project Template Title"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
	m_staticTitle->Wrap( 150 );
	bSizer4->Add( m_staticTitle, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 3 );
	
	
	this->SetSizer( bSizer4 );
	this->Layout();
}

TEMPLATE_WIDGET_BASE::~TEMPLATE_WIDGET_BASE()
{
}
