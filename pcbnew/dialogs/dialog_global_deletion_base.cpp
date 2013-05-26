///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_global_deletion_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_GLOBAL_DELETION_BASE::DIALOG_GLOBAL_DELETION_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizerUpper;
	bSizerUpper = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizerLeft;
	sbSizerLeft = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Items to Delete") ), wxVERTICAL );
	
	m_DelZones = new wxCheckBox( this, wxID_ANY, _("Zones"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelZones, 0, wxALL, 5 );
	
	m_DelTexts = new wxCheckBox( this, wxID_ANY, _("Text"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelTexts, 0, wxALL, 5 );
	
	m_DelBoardEdges = new wxCheckBox( this, wxID_ANY, _("Board outlines"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelBoardEdges, 0, wxALL, 5 );
	
	m_DelDrawings = new wxCheckBox( this, wxID_ANY, _("Drawings"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelDrawings, 0, wxALL, 5 );
	
	m_DelModules = new wxCheckBox( this, wxID_ANY, _("Footprints"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelModules, 0, wxALL, 5 );
	
	m_DelTracks = new wxCheckBox( this, wxID_ANY, _("Tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelTracks, 0, wxALL, 5 );
	
	m_DelMarkers = new wxCheckBox( this, wxID_ANY, _("Markers"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelMarkers, 0, wxALL, 5 );
	
	m_DelAlls = new wxCheckBox( this, wxID_ANY, _("Clear Board"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerLeft->Add( m_DelAlls, 0, wxALL, 5 );
	
	
	bSizerUpper->Add( sbSizerLeft, 2, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizerRight;
	bSizerRight = new wxBoxSizer( wxVERTICAL );
	
	sbTrackFilter = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Track Filter") ), wxVERTICAL );
	
	m_TrackFilterAR = new wxCheckBox( this, wxID_ANY, _("Automatically routed tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TrackFilterAR->SetValue(true); 
	sbTrackFilter->Add( m_TrackFilterAR, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_TrackFilterLocked = new wxCheckBox( this, wxID_ANY, _("Locked tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	sbTrackFilter->Add( m_TrackFilterLocked, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_TrackFilterNormal = new wxCheckBox( this, wxID_ANY, _("Normal tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TrackFilterNormal->SetValue(true); 
	sbTrackFilter->Add( m_TrackFilterNormal, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_TrackFilterVias = new wxCheckBox( this, wxID_ANY, _("Vias"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TrackFilterVias->SetValue(true); 
	sbTrackFilter->Add( m_TrackFilterVias, 0, wxALL, 5 );
	
	
	bSizerRight->Add( sbTrackFilter, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_rbLayersOptionChoices[] = { _("All layers"), _("Current layer only") };
	int m_rbLayersOptionNChoices = sizeof( m_rbLayersOptionChoices ) / sizeof( wxString );
	m_rbLayersOption = new wxRadioBox( this, wxID_ANY, _("Layer Filter"), wxDefaultPosition, wxDefaultSize, m_rbLayersOptionNChoices, m_rbLayersOptionChoices, 1, wxRA_SPECIFY_COLS );
	m_rbLayersOption->SetSelection( 0 );
	bSizerRight->Add( m_rbLayersOption, 0, wxALL|wxEXPAND, 5 );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, _("Current layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizerRight->Add( m_staticText1, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_textCtrlCurrLayer = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_textCtrlCurrLayer->SetMaxLength( 0 ); 
	bSizerRight->Add( m_textCtrlCurrLayer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	bSizerUpper->Add( bSizerRight, 3, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	
	bSizerMain->Add( bSizerUpper, 1, wxALL|wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizerMain->Add( m_staticline1, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	bSizerMain->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bSizerMain );
	this->Layout();
	bSizerMain->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_DelTracks->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_GLOBAL_DELETION_BASE::OnCheckDeleteTracks ), NULL, this );
	m_sdbSizer1Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GLOBAL_DELETION_BASE::OnCancelClick ), NULL, this );
	m_sdbSizer1OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GLOBAL_DELETION_BASE::OnOkClick ), NULL, this );
}

DIALOG_GLOBAL_DELETION_BASE::~DIALOG_GLOBAL_DELETION_BASE()
{
	// Disconnect Events
	m_DelTracks->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_GLOBAL_DELETION_BASE::OnCheckDeleteTracks ), NULL, this );
	m_sdbSizer1Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GLOBAL_DELETION_BASE::OnCancelClick ), NULL, this );
	m_sdbSizer1OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GLOBAL_DELETION_BASE::OnOkClick ), NULL, this );
	
}
