///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_exchange_modules_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_EXCHANGE_MODULE_BASE::DIALOG_EXCHANGE_MODULE_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bUpperSizer;
	bUpperSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bLeftSizer;
	bLeftSizer = new wxBoxSizer( wxVERTICAL );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, _("Current Module"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	bLeftSizer->Add( m_staticText6, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_OldModule = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_OldModule->SetMaxLength( 0 ); 
	bLeftSizer->Add( m_OldModule, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxEXPAND, 5 );
	
	m_staticText7 = new wxStaticText( this, wxID_ANY, _("Current Value"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	bLeftSizer->Add( m_staticText7, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_OldValue = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_OldValue->SetMaxLength( 0 ); 
	bLeftSizer->Add( m_OldValue, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxEXPAND, 5 );
	
	m_staticText8 = new wxStaticText( this, wxID_ANY, _("New Module"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bLeftSizer->Add( m_staticText8, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_NewModule = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_NewModule->SetMaxLength( 0 ); 
	bLeftSizer->Add( m_NewModule, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxEXPAND, 5 );
	
	
	bUpperSizer->Add( bLeftSizer, 1, 0, 5 );
	
	wxBoxSizer* bMiddleSizer;
	bMiddleSizer = new wxBoxSizer( wxVERTICAL );
	
	wxString m_SelectionChoices[] = { _("Change module"), _("Change same modules"), _("Ch. same module+value"), _("Change all") };
	int m_SelectionNChoices = sizeof( m_SelectionChoices ) / sizeof( wxString );
	m_Selection = new wxRadioBox( this, ID_SELECTION_CLICKED, _("Browse Libs modules"), wxDefaultPosition, wxDefaultSize, m_SelectionNChoices, m_SelectionChoices, 1, wxRA_SPECIFY_COLS );
	m_Selection->SetSelection( 0 );
	bMiddleSizer->Add( m_Selection, 0, wxALL, 5 );
	
	
	bUpperSizer->Add( bMiddleSizer, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bRightSizer;
	bRightSizer = new wxBoxSizer( wxVERTICAL );
	
	m_OKbutton = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bRightSizer->Add( m_OKbutton, 0, wxALL, 5 );
	
	m_Quitbutton = new wxButton( this, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	bRightSizer->Add( m_Quitbutton, 0, wxALL, 5 );
	
	m_Browsebutton = new wxButton( this, wxID_ANY, _("Browse"), wxDefaultPosition, wxDefaultSize, 0 );
	bRightSizer->Add( m_Browsebutton, 0, wxALL, 5 );
	
	
	bUpperSizer->Add( bRightSizer, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bMainSizer->Add( bUpperSizer, 0, wxEXPAND, 5 );
	
	m_staticTextMsg = new wxStaticText( this, wxID_ANY, _("Messages:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextMsg->Wrap( -1 );
	bMainSizer->Add( m_staticTextMsg, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_WinMessages = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	m_WinMessages->SetMaxLength( 0 ); 
	m_WinMessages->SetMinSize( wxSize( 450,300 ) );
	
	bMainSizer->Add( m_WinMessages, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	this->SetSizer( bMainSizer );
	this->Layout();
	bMainSizer->Fit( this );
	
	// Connect Events
	m_Selection->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::OnSelectionClicked ), NULL, this );
	m_OKbutton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::OnOkClick ), NULL, this );
	m_Quitbutton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::OnQuit ), NULL, this );
	m_Browsebutton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::BrowseAndSelectFootprint ), NULL, this );
}

DIALOG_EXCHANGE_MODULE_BASE::~DIALOG_EXCHANGE_MODULE_BASE()
{
	// Disconnect Events
	m_Selection->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::OnSelectionClicked ), NULL, this );
	m_OKbutton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::OnOkClick ), NULL, this );
	m_Quitbutton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::OnQuit ), NULL, this );
	m_Browsebutton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EXCHANGE_MODULE_BASE::BrowseAndSelectFootprint ), NULL, this );
	
}
