///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_netlist_fbp.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_NETLIST_FBP::DIALOG_NETLIST_FBP( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bUpperSizer;
	bUpperSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bLeftSizer;
	bLeftSizer = new wxBoxSizer( wxVERTICAL );
	
	wxString m_Select_By_TimestampChoices[] = { _("Reference"), _("Timestamp") };
	int m_Select_By_TimestampNChoices = sizeof( m_Select_By_TimestampChoices ) / sizeof( wxString );
	m_Select_By_Timestamp = new wxRadioBox( this, wxID_ANY, _("Module Selection"), wxDefaultPosition, wxDefaultSize, m_Select_By_TimestampNChoices, m_Select_By_TimestampChoices, 1, wxRA_SPECIFY_COLS );
	m_Select_By_Timestamp->SetSelection( 0 );
	m_Select_By_Timestamp->SetToolTip( _("Select how footprints are recognized:\nby their reference (U1, R3...) (normal setting)\nor their time stamp (special setting after a full schematic reannotation)") );
	
	bLeftSizer->Add( m_Select_By_Timestamp, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxString m_cmpNameSourceOptChoices[] = { _("From netlist"), _("From separate .cmp file") };
	int m_cmpNameSourceOptNChoices = sizeof( m_cmpNameSourceOptChoices ) / sizeof( wxString );
	m_cmpNameSourceOpt = new wxRadioBox( this, wxID_ANY, _("Module Name Source"), wxDefaultPosition, wxDefaultSize, m_cmpNameSourceOptNChoices, m_cmpNameSourceOptChoices, 1, wxRA_SPECIFY_COLS );
	m_cmpNameSourceOpt->SetSelection( 0 );
	m_cmpNameSourceOpt->SetToolTip( _("Source of footprints names for component:\n- the netlist (if you have filled the footprint field of each component  in schematic)\n- the .cmp file created by CvPcb") );
	
	bLeftSizer->Add( m_cmpNameSourceOpt, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxString m_ChangeExistingFootprintCtrlChoices[] = { _("Keep"), _("Change") };
	int m_ChangeExistingFootprintCtrlNChoices = sizeof( m_ChangeExistingFootprintCtrlChoices ) / sizeof( wxString );
	m_ChangeExistingFootprintCtrl = new wxRadioBox( this, wxID_ANY, _("Exchange Module"), wxDefaultPosition, wxDefaultSize, m_ChangeExistingFootprintCtrlNChoices, m_ChangeExistingFootprintCtrlChoices, 1, wxRA_SPECIFY_COLS );
	m_ChangeExistingFootprintCtrl->SetSelection( 0 );
	m_ChangeExistingFootprintCtrl->SetToolTip( _("Keep or change an existing footprint when the netlist gives a different footprint") );
	
	bLeftSizer->Add( m_ChangeExistingFootprintCtrl, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	
	bUpperSizer->Add( bLeftSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bMiddleSizer;
	bMiddleSizer = new wxBoxSizer( wxVERTICAL );
	
	wxString m_DeleteBadTracksChoices[] = { _("Keep"), _("Delete") };
	int m_DeleteBadTracksNChoices = sizeof( m_DeleteBadTracksChoices ) / sizeof( wxString );
	m_DeleteBadTracks = new wxRadioBox( this, wxID_ANY, _("Unconnected Tracks"), wxDefaultPosition, wxDefaultSize, m_DeleteBadTracksNChoices, m_DeleteBadTracksChoices, 1, wxRA_SPECIFY_COLS );
	m_DeleteBadTracks->SetSelection( 0 );
	m_DeleteBadTracks->SetToolTip( _("Keep or delete bad tracks after a netlist change") );
	
	bMiddleSizer->Add( m_DeleteBadTracks, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxString m_RemoveExtraFootprintsCtrlChoices[] = { _("Keep"), _("Delete") };
	int m_RemoveExtraFootprintsCtrlNChoices = sizeof( m_RemoveExtraFootprintsCtrlChoices ) / sizeof( wxString );
	m_RemoveExtraFootprintsCtrl = new wxRadioBox( this, wxID_ANY, _("Extra Footprints"), wxDefaultPosition, wxDefaultSize, m_RemoveExtraFootprintsCtrlNChoices, m_RemoveExtraFootprintsCtrlChoices, 1, wxRA_SPECIFY_COLS );
	m_RemoveExtraFootprintsCtrl->SetSelection( 0 );
	m_RemoveExtraFootprintsCtrl->SetToolTip( _("Remove footprints found on the Board but not in netlist\nNote: only not locked footprints will be removed") );
	
	bMiddleSizer->Add( m_RemoveExtraFootprintsCtrl, 0, wxALL|wxEXPAND, 5 );
	
	
	bUpperSizer->Add( bMiddleSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bRightSizerButtons;
	bRightSizerButtons = new wxBoxSizer( wxVERTICAL );
	
	m_buttonBrowse = new wxButton( this, ID_OPEN_NELIST, _("Open Netlist File"), wxDefaultPosition, wxDefaultSize, 0 );
	bRightSizerButtons->Add( m_buttonBrowse, 0, wxEXPAND|wxALL, 5 );
	
	m_buttonRead = new wxButton( this, ID_READ_NETLIST_FILE, _("Read Current Netlist"), wxDefaultPosition, wxDefaultSize, 0 );
	m_buttonRead->SetDefault(); 
	m_buttonRead->SetToolTip( _("Read the current netlist and update connections and connectivity info") );
	
	bRightSizerButtons->Add( m_buttonRead, 0, wxEXPAND|wxALL, 5 );
	
	m_buttonFPTest = new wxButton( this, ID_TEST_NETLIST, _("Test Footprints"), wxDefaultPosition, wxDefaultSize, 0 );
	m_buttonFPTest->SetToolTip( _("Read the current neltist file and list missing and extra footprints") );
	
	bRightSizerButtons->Add( m_buttonFPTest, 0, wxEXPAND|wxALL, 5 );
	
	m_buttonRebild = new wxButton( this, ID_COMPILE_RATSNEST, _("Rebuild Board Connectivity"), wxDefaultPosition, wxDefaultSize, 0 );
	m_buttonRebild->SetToolTip( _("Rebuild the full ratsnest (usefull after a manual pad netname edition)") );
	
	bRightSizerButtons->Add( m_buttonRebild, 0, wxEXPAND|wxALL, 5 );
	
	m_buttonSaveMessages = new wxButton( this, wxID_ANY, _("Save Messages to File"), wxDefaultPosition, wxDefaultSize, 0 );
	bRightSizerButtons->Add( m_buttonSaveMessages, 0, wxALL|wxEXPAND, 5 );
	
	m_buttonClose = new wxButton( this, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	bRightSizerButtons->Add( m_buttonClose, 0, wxALL|wxEXPAND, 5 );
	
	
	bUpperSizer->Add( bRightSizerButtons, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxTOP, 5 );
	
	
	bMainSizer->Add( bUpperSizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bCenterSizer;
	bCenterSizer = new wxBoxSizer( wxVERTICAL );
	
	m_checkDryRun = new wxCheckBox( this, wxID_ANY, _("Only report changes in message panel"), wxDefaultPosition, wxDefaultSize, 0 );
	bCenterSizer->Add( m_checkDryRun, 0, wxALL, 5 );
	
	
	bMainSizer->Add( bCenterSizer, 0, wxALL|wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bMainSizer->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bLowerSizer;
	bLowerSizer = new wxBoxSizer( wxVERTICAL );
	
	m_staticTextNetfilename = new wxStaticText( this, wxID_ANY, _("Netlist File:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextNetfilename->Wrap( -1 );
	bLowerSizer->Add( m_staticTextNetfilename, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_NetlistFilenameCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_NetlistFilenameCtrl->SetMaxLength( 0 ); 
	bLowerSizer->Add( m_NetlistFilenameCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, _("Messages:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bLowerSizer->Add( m_staticText1, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_MessageWindow = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CHARWRAP|wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP );
	m_MessageWindow->SetMaxLength( 0 ); 
	m_MessageWindow->SetMinSize( wxSize( -1,150 ) );
	
	bLowerSizer->Add( m_MessageWindow, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	bMainSizer->Add( bLowerSizer, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	
	this->SetSizer( bMainSizer );
	this->Layout();
	bMainSizer->Fit( this );
	
	// Connect Events
	m_buttonBrowse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnOpenNetlistClick ), NULL, this );
	m_buttonRead->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnReadNetlistFileClick ), NULL, this );
	m_buttonRead->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUIValidNetlistFile ), NULL, this );
	m_buttonFPTest->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnTestFootprintsClick ), NULL, this );
	m_buttonFPTest->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUIValidNetlistFile ), NULL, this );
	m_buttonRebild->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnCompileRatsnestClick ), NULL, this );
	m_buttonRebild->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUIValidNetlistFile ), NULL, this );
	m_buttonSaveMessages->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnSaveMessagesToFile ), NULL, this );
	m_buttonSaveMessages->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUISaveMessagesToFile ), NULL, this );
	m_buttonClose->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnCancelClick ), NULL, this );
}

DIALOG_NETLIST_FBP::~DIALOG_NETLIST_FBP()
{
	// Disconnect Events
	m_buttonBrowse->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnOpenNetlistClick ), NULL, this );
	m_buttonRead->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnReadNetlistFileClick ), NULL, this );
	m_buttonRead->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUIValidNetlistFile ), NULL, this );
	m_buttonFPTest->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnTestFootprintsClick ), NULL, this );
	m_buttonFPTest->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUIValidNetlistFile ), NULL, this );
	m_buttonRebild->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnCompileRatsnestClick ), NULL, this );
	m_buttonRebild->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUIValidNetlistFile ), NULL, this );
	m_buttonSaveMessages->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnSaveMessagesToFile ), NULL, this );
	m_buttonSaveMessages->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( DIALOG_NETLIST_FBP::OnUpdateUISaveMessagesToFile ), NULL, this );
	m_buttonClose->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NETLIST_FBP::OnCancelClick ), NULL, this );
	
}
