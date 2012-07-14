///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 11 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_pcb_text_properties_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_PCB_TEXT_PROPERTIES_BASE::DIALOG_PCB_TEXT_PROPERTIES_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );
	
	m_TextLabel = new wxStaticText( this, wxID_ANY, _("Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TextLabel->Wrap( -1 );
	bSizer9->Add( m_TextLabel, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_TextContentCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	m_TextContentCtrl->SetToolTip( _("Enter the text placed on selected layer.") );
	m_TextContentCtrl->SetMinSize( wxSize( 400,60 ) );
	
	bSizer9->Add( m_TextContentCtrl, 1, wxBOTTOM|wxRIGHT|wxLEFT|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 6, 4, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->AddGrowableCol( 2 );
	fgSizer1->AddGrowableCol( 3 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_SizeXLabel = new wxStaticText( this, wxID_ANY, _("Size X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SizeXLabel->Wrap( -1 );
	fgSizer1->Add( m_SizeXLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_PositionXLabel = new wxStaticText( this, wxID_ANY, _("Position X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PositionXLabel->Wrap( -1 );
	fgSizer1->Add( m_PositionXLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_LayerLabel = new wxStaticText( this, wxID_ANY, _("Layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_LayerLabel->Wrap( -1 );
	fgSizer1->Add( m_LayerLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_staticText10 = new wxStaticText( this, wxID_ANY, _("Display:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer1->Add( m_staticText10, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_SizeXCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_SizeXCtrl, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	m_PositionXCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_PositionXCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	wxArrayString m_LayerSelectionCtrlChoices;
	m_LayerSelectionCtrl = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_LayerSelectionCtrlChoices, 0 );
	m_LayerSelectionCtrl->SetSelection( 0 );
	m_LayerSelectionCtrl->SetToolTip( _("Select the layer on which text should lay.") );
	
	fgSizer1->Add( m_LayerSelectionCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	wxString m_DisplayCtrlChoices[] = { _("Normal"), _("Mirrored") };
	int m_DisplayCtrlNChoices = sizeof( m_DisplayCtrlChoices ) / sizeof( wxString );
	m_DisplayCtrl = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_DisplayCtrlNChoices, m_DisplayCtrlChoices, 0 );
	m_DisplayCtrl->SetSelection( 0 );
	fgSizer1->Add( m_DisplayCtrl, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	m_SizeYLabel = new wxStaticText( this, wxID_ANY, _("Size Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SizeYLabel->Wrap( -1 );
	fgSizer1->Add( m_SizeYLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_PositionYLabel = new wxStaticText( this, wxID_ANY, _("Position Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PositionYLabel->Wrap( -1 );
	fgSizer1->Add( m_PositionYLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_staticText9 = new wxStaticText( this, wxID_ANY, _("Style:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	fgSizer1->Add( m_staticText9, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_staticText11 = new wxStaticText( this, wxID_ANY, _("Justification:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	fgSizer1->Add( m_staticText11, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_SizeYCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_SizeYCtrl, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	m_PositionYCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_PositionYCtrl, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	wxString m_StyleCtrlChoices[] = { _("Normal"), _("Italic") };
	int m_StyleCtrlNChoices = sizeof( m_StyleCtrlChoices ) / sizeof( wxString );
	m_StyleCtrl = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_StyleCtrlNChoices, m_StyleCtrlChoices, 0 );
	m_StyleCtrl->SetSelection( 0 );
	fgSizer1->Add( m_StyleCtrl, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	wxString m_justifyChoiceChoices[] = { _("Left"), _("Center"), _("Right") };
	int m_justifyChoiceNChoices = sizeof( m_justifyChoiceChoices ) / sizeof( wxString );
	m_justifyChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_justifyChoiceNChoices, m_justifyChoiceChoices, 0 );
	m_justifyChoice->SetSelection( 0 );
	fgSizer1->Add( m_justifyChoice, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	m_ThicknessLabel = new wxStaticText( this, wxID_ANY, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ThicknessLabel->Wrap( -1 );
	fgSizer1->Add( m_ThicknessLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_orientationLabel = new wxStaticText( this, wxID_ANY, _("Orientation (0.1 deg):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_orientationLabel->Wrap( -1 );
	fgSizer1->Add( m_orientationLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_ThicknessCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_ThicknessCtrl, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	m_OrientationCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_OrientationCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer9->Add( fgSizer1, 1, wxEXPAND, 5 );
	
	m_StandardSizer = new wxStdDialogButtonSizer();
	m_StandardSizerOK = new wxButton( this, wxID_OK );
	m_StandardSizer->AddButton( m_StandardSizerOK );
	m_StandardSizerCancel = new wxButton( this, wxID_CANCEL );
	m_StandardSizer->AddButton( m_StandardSizerCancel );
	m_StandardSizer->Realize();
	
	bSizer9->Add( m_StandardSizer, 0, wxALIGN_BOTTOM|wxALIGN_RIGHT|wxALL, 5 );
	
	
	bMainSizer->Add( bSizer9, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bMainSizer );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_PCB_TEXT_PROPERTIES_BASE::OnClose ) );
	m_StandardSizerCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PCB_TEXT_PROPERTIES_BASE::OnCancelClick ), NULL, this );
	m_StandardSizerOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PCB_TEXT_PROPERTIES_BASE::OnOkClick ), NULL, this );
}

DIALOG_PCB_TEXT_PROPERTIES_BASE::~DIALOG_PCB_TEXT_PROPERTIES_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_PCB_TEXT_PROPERTIES_BASE::OnClose ) );
	m_StandardSizerCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PCB_TEXT_PROPERTIES_BASE::OnCancelClick ), NULL, this );
	m_StandardSizerOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PCB_TEXT_PROPERTIES_BASE::OnOkClick ), NULL, this );
	
}
