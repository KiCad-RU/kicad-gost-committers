///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2011)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_page_settings_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_PAGES_SETTINGS_BASE::DIALOG_PAGES_SETTINGS_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bUpperSizerH;
	bUpperSizerH = new wxBoxSizer( wxHORIZONTAL );
	
	wxFlexGridSizer* LeftColumnSizer;
	LeftColumnSizer = new wxFlexGridSizer( 6, 1, 0, 0 );
	LeftColumnSizer->AddGrowableRow( 0 );
	LeftColumnSizer->AddGrowableRow( 1 );
	LeftColumnSizer->AddGrowableRow( 2 );
	LeftColumnSizer->SetFlexibleDirection( wxBOTH );
	LeftColumnSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Paper Size:") ), wxVERTICAL );
	
	wxString m_paperSizeComboBoxChoices[] = { _("A4"), _("A3"), _("A2"), _("A1"), _("A0"), _("A"), _("B"), _("C"), _("D"), _("E"), _("USLetter"), _("USLegal"), _("USLedger"), _("User (Custom)") };
	int m_paperSizeComboBoxNChoices = sizeof( m_paperSizeComboBoxChoices ) / sizeof( wxString );
	m_paperSizeComboBox = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_paperSizeComboBoxNChoices, m_paperSizeComboBoxChoices, 0 );
	m_paperSizeComboBox->SetSelection( 0 );
	sbSizer9->Add( m_paperSizeComboBox, 0, wxALL|wxEXPAND, 5 );
	
	LeftColumnSizer->Add( sbSizer9, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Orientation:") ), wxVERTICAL );
	
	wxString m_orientationComboBoxChoices[] = { _("Landscape"), _("Portrait") };
	int m_orientationComboBoxNChoices = sizeof( m_orientationComboBoxChoices ) / sizeof( wxString );
	m_orientationComboBox = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_orientationComboBoxNChoices, m_orientationComboBoxChoices, 0 );
	m_orientationComboBox->SetSelection( 0 );
	sbSizer8->Add( m_orientationComboBox, 0, wxALL|wxEXPAND, 5 );
	
	LeftColumnSizer->Add( sbSizer8, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Custom Page Size:") ), wxVERTICAL );
	
	wxBoxSizer* bSizerXsize;
	bSizerXsize = new wxBoxSizer( wxVERTICAL );
	
	UserPageSizeX = new wxStaticText( this, wxID_ANY, _("Width:"), wxDefaultPosition, wxDefaultSize, 0 );
	UserPageSizeX->Wrap( -1 );
	bSizerXsize->Add( UserPageSizeX, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_TextUserSizeX = new wxTextCtrl( this, ID_TEXTCTRL_USER_PAGE_SIZE_X, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerXsize->Add( m_TextUserSizeX, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL|wxEXPAND, 5 );
	
	sbSizer10->Add( bSizerXsize, 1, wxALIGN_TOP|wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizerYsize;
	bSizerYsize = new wxBoxSizer( wxVERTICAL );
	
	UserPageSizeY = new wxStaticText( this, wxID_ANY, _("Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	UserPageSizeY->Wrap( -1 );
	bSizerYsize->Add( UserPageSizeY, 0, wxALIGN_TOP|wxALL, 5 );
	
	m_TextUserSizeY = new wxTextCtrl( this, ID_TEXTCTRL_USER_PAGE_SIZE_Y, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerYsize->Add( m_TextUserSizeY, 0, wxALIGN_TOP|wxALL|wxEXPAND, 5 );
	
	sbSizer10->Add( bSizerYsize, 1, wxALIGN_TOP|wxALL|wxEXPAND, 5 );
	
	LeftColumnSizer->Add( sbSizer10, 1, wxALL|wxEXPAND, 5 );
	
	bUpperSizerH->Add( LeftColumnSizer, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* RightColumnSizer;
	RightColumnSizer = new wxFlexGridSizer( 8, 1, 0, 0 );
	RightColumnSizer->AddGrowableCol( 0 );
	RightColumnSizer->AddGrowableRow( 0 );
	RightColumnSizer->AddGrowableRow( 1 );
	RightColumnSizer->AddGrowableRow( 2 );
	RightColumnSizer->AddGrowableRow( 3 );
	RightColumnSizer->AddGrowableRow( 4 );
	RightColumnSizer->AddGrowableRow( 5 );
	RightColumnSizer->AddGrowableRow( 6 );
	RightColumnSizer->AddGrowableRow( 7 );
	RightColumnSizer->SetFlexibleDirection( wxBOTH );
	RightColumnSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* SheetInfoSizer;
	SheetInfoSizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_TextSheetCount = new wxStaticText( this, wxID_ANY, _("Number of sheets: %d"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TextSheetCount->Wrap( -1 );
	SheetInfoSizer->Add( m_TextSheetCount, 0, wxALL, 5 );
	
	
	SheetInfoSizer->Add( 5, 5, 1, wxEXPAND, 5 );
	
	m_TextSheetNumber = new wxStaticText( this, wxID_ANY, _("Sheet number: %d"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TextSheetNumber->Wrap( -1 );
	SheetInfoSizer->Add( m_TextSheetNumber, 0, wxALL, 5 );
	
	RightColumnSizer->Add( SheetInfoSizer, 1, 0, 5 );
	
	wxStaticBoxSizer* RevisionSizer;
	RevisionSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Revision:") ), wxHORIZONTAL );
	
	m_TextRevision = new wxTextCtrl( this, ID_TEXTCTRL_REVISION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TextRevision->SetMinSize( wxSize( 100,-1 ) );
	
	RevisionSizer->Add( m_TextRevision, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_RevisionExport = new wxCheckBox( this, ID_CHECKBOX_REVISION, _("Export to other sheets"), wxDefaultPosition, wxDefaultSize, 0 );
	RevisionSizer->Add( m_RevisionExport, 0, wxALL, 5 );
	
	RightColumnSizer->Add( RevisionSizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* TitleSizer;
	TitleSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Title:") ), wxHORIZONTAL );
	
	m_TextTitle = new wxTextCtrl( this, ID_TEXTCTRL_TITLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TextTitle->SetMinSize( wxSize( 400,-1 ) );
	
	TitleSizer->Add( m_TextTitle, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_TitleExport = new wxCheckBox( this, wxID_ANY, _("Export to other sheets"), wxDefaultPosition, wxDefaultSize, 0 );
	TitleSizer->Add( m_TitleExport, 0, wxALL, 5 );
	
	RightColumnSizer->Add( TitleSizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* CompanySizer;
	CompanySizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Company:") ), wxHORIZONTAL );
	
	m_TextCompany = new wxTextCtrl( this, ID_TEXTCTRL_COMPANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TextCompany->SetMinSize( wxSize( 400,-1 ) );
	
	CompanySizer->Add( m_TextCompany, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_CompanyExport = new wxCheckBox( this, ID_CHECKBOX_COMPANY, _("Export to other sheets"), wxDefaultPosition, wxDefaultSize, 0 );
	CompanySizer->Add( m_CompanyExport, 0, wxALL, 5 );
	
	RightColumnSizer->Add( CompanySizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* Comment1Sizer;
	Comment1Sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Comment1:") ), wxHORIZONTAL );
	
	m_TextComment1 = new wxTextCtrl( this, ID_TEXTCTRL_COMMENT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TextComment1->SetMinSize( wxSize( 400,-1 ) );
	
	Comment1Sizer->Add( m_TextComment1, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_Comment1Export = new wxCheckBox( this, ID_CHECKBOX_COMMENT1, _("Export to other sheets"), wxDefaultPosition, wxDefaultSize, 0 );
	Comment1Sizer->Add( m_Comment1Export, 0, wxALL, 5 );
	
	RightColumnSizer->Add( Comment1Sizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* Comment2Sizer;
	Comment2Sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Comment2:") ), wxHORIZONTAL );
	
	m_TextComment2 = new wxTextCtrl( this, ID_TEXTCTRL_COMMENT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TextComment2->SetMinSize( wxSize( 400,-1 ) );
	
	Comment2Sizer->Add( m_TextComment2, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_Comment2Export = new wxCheckBox( this, ID_CHECKBOX_COMMENT2, _("Export to other sheets"), wxDefaultPosition, wxDefaultSize, 0 );
	Comment2Sizer->Add( m_Comment2Export, 0, wxALL, 5 );
	
	RightColumnSizer->Add( Comment2Sizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* Comment3Sizer;
	Comment3Sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Comment3:") ), wxHORIZONTAL );
	
	m_TextComment3 = new wxTextCtrl( this, ID_TEXTCTRL_COMMENT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TextComment3->SetMinSize( wxSize( 400,-1 ) );
	
	Comment3Sizer->Add( m_TextComment3, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_Comment3Export = new wxCheckBox( this, ID_CHECKBOX_COMMENT3, _("Export to other sheets"), wxDefaultPosition, wxDefaultSize, 0 );
	Comment3Sizer->Add( m_Comment3Export, 0, wxALL, 5 );
	
	RightColumnSizer->Add( Comment3Sizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* Comment4Sizer;
	Comment4Sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Comment4:") ), wxHORIZONTAL );
	
	m_TextComment4 = new wxTextCtrl( this, ID_TEXTCTRL_COMMENT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TextComment4->SetMinSize( wxSize( 400,-1 ) );
	
	Comment4Sizer->Add( m_TextComment4, 1, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_Comment4Export = new wxCheckBox( this, ID_CHECKBOX_COMMENT4, _("Export to other sheets"), wxDefaultPosition, wxDefaultSize, 0 );
	Comment4Sizer->Add( m_Comment4Export, 0, wxALL, 5 );
	
	RightColumnSizer->Add( Comment4Sizer, 1, wxEXPAND, 5 );
	
	bUpperSizerH->Add( RightColumnSizer, 1, wxEXPAND, 5 );
	
	bMainSizer->Add( bUpperSizerH, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	bMainSizer->Add( m_sdbSizer1, 0, wxEXPAND|wxALIGN_RIGHT|wxALL, 5 );
	
	this->SetSizer( bMainSizer );
	this->Layout();
	bMainSizer->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnCloseWindow ) );
	m_paperSizeComboBox->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::onPaperSizeChoice ), NULL, this );
	m_TextUserSizeX->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnTextctrlUserPageSizeXTextUpdated ), NULL, this );
	m_TextUserSizeY->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnTextctrlUserPageSizeYTextUpdated ), NULL, this );
	m_TitleExport->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnCheckboxTitleClick ), NULL, this );
	m_sdbSizer1Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnCancelClick ), NULL, this );
	m_sdbSizer1OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnOkClick ), NULL, this );
}

DIALOG_PAGES_SETTINGS_BASE::~DIALOG_PAGES_SETTINGS_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnCloseWindow ) );
	m_paperSizeComboBox->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::onPaperSizeChoice ), NULL, this );
	m_TextUserSizeX->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnTextctrlUserPageSizeXTextUpdated ), NULL, this );
	m_TextUserSizeY->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnTextctrlUserPageSizeYTextUpdated ), NULL, this );
	m_TitleExport->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnCheckboxTitleClick ), NULL, this );
	m_sdbSizer1Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnCancelClick ), NULL, this );
	m_sdbSizer1OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAGES_SETTINGS_BASE::OnOkClick ), NULL, this );
	
}
