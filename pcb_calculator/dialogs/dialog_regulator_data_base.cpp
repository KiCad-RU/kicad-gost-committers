///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jan  1 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_regulator_data_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_EDITOR_DATA_BASE::DIALOG_EDITOR_DATA_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizerPrms;
	fgSizerPrms = new wxFlexGridSizer( 4, 3, 0, 0 );
	fgSizerPrms->AddGrowableCol( 1 );
	fgSizerPrms->AddGrowableRow( 0 );
	fgSizerPrms->SetFlexibleDirection( wxHORIZONTAL );
	fgSizerPrms->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticTextName = new wxStaticText( this, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextName->Wrap( -1 );
	fgSizerPrms->Add( m_staticTextName, 0, wxALL, 5 );
	
	m_textCtrlName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerPrms->Add( m_textCtrlName, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizerPrms->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticTextVref = new wxStaticText( this, wxID_ANY, _("Vref"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextVref->Wrap( -1 );
	fgSizerPrms->Add( m_staticTextVref, 0, wxALL, 5 );
	
	m_textCtrlVref = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerPrms->Add( m_textCtrlVref, 0, wxALL|wxEXPAND, 5 );
	
	m_staticTextVrefUnit = new wxStaticText( this, wxID_ANY, _("Volt"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextVrefUnit->Wrap( -1 );
	fgSizerPrms->Add( m_staticTextVrefUnit, 0, wxALL, 5 );
	
	m_staticTextType = new wxStaticText( this, wxID_ANY, _("Type"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextType->Wrap( -1 );
	fgSizerPrms->Add( m_staticTextType, 0, wxALL, 5 );
	
	wxString m_choiceRegTypeChoices[] = { _("Separate sense pin"), _("3 terminals regulator") };
	int m_choiceRegTypeNChoices = sizeof( m_choiceRegTypeChoices ) / sizeof( wxString );
	m_choiceRegType = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceRegTypeNChoices, m_choiceRegTypeChoices, 0 );
	m_choiceRegType->SetSelection( 0 );
	fgSizerPrms->Add( m_choiceRegType, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizerPrms->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_RegulIadjTitle = new wxStaticText( this, wxID_ANY, _("Iadj"), wxDefaultPosition, wxDefaultSize, 0 );
	m_RegulIadjTitle->Wrap( -1 );
	fgSizerPrms->Add( m_RegulIadjTitle, 0, wxALL, 5 );
	
	m_RegulIadjValue = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerPrms->Add( m_RegulIadjValue, 0, wxALL|wxEXPAND, 5 );
	
	m_IadjUnitLabel = new wxStaticText( this, wxID_ANY, _("uA"), wxDefaultPosition, wxDefaultSize, 0 );
	m_IadjUnitLabel->Wrap( -1 );
	fgSizerPrms->Add( m_IadjUnitLabel, 0, wxALL, 5 );
	
	
	bSizerMain->Add( fgSizerPrms, 0, wxEXPAND, 5 );
	
	
	bSizerMain->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizerMain->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );
	
	m_sdbSizerButtons = new wxStdDialogButtonSizer();
	m_sdbSizerButtonsOK = new wxButton( this, wxID_OK );
	m_sdbSizerButtons->AddButton( m_sdbSizerButtonsOK );
	m_sdbSizerButtonsCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizerButtons->AddButton( m_sdbSizerButtonsCancel );
	m_sdbSizerButtons->Realize();
	
	bSizerMain->Add( m_sdbSizerButtons, 0, wxEXPAND|wxALL, 5 );
	
	
	this->SetSizer( bSizerMain );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_choiceRegType->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( DIALOG_EDITOR_DATA_BASE::OnRegTypeSelection ), NULL, this );
	m_sdbSizerButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDITOR_DATA_BASE::OnOKClick ), NULL, this );
}

DIALOG_EDITOR_DATA_BASE::~DIALOG_EDITOR_DATA_BASE()
{
	// Disconnect Events
	m_choiceRegType->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( DIALOG_EDITOR_DATA_BASE::OnRegTypeSelection ), NULL, this );
	m_sdbSizerButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDITOR_DATA_BASE::OnOKClick ), NULL, this );
	
}
