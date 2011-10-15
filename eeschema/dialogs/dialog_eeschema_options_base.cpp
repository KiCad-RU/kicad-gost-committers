///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_eeschema_options_base.h"

///////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( DIALOG_EESCHEMA_OPTIONS_BASE, wxDialog )
	EVT_CHOICE( wxID_ANY, DIALOG_EESCHEMA_OPTIONS_BASE::_wxFB_OnChooseUnits )
END_EVENT_TABLE()

DIALOG_EESCHEMA_OPTIONS_BASE::DIALOG_EESCHEMA_OPTIONS_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bOptionsSizer;
	bOptionsSizer = new wxBoxSizer( wxVERTICAL );
	
	m_notebook1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_notebook1->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	m_panel1 = new wxPanel( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* p1mainSizer;
	p1mainSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 8, 3, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->AddGrowableCol( 2 );
	fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText2 = new wxStaticText( m_panel1, wxID_ANY, _("Measurement &units:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	fgSizer1->Add( m_staticText2, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString m_choiceUnitsChoices;
	m_choiceUnits = new wxChoice( m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceUnitsChoices, 0 );
	m_choiceUnits->SetSelection( 0 );
	fgSizer1->Add( m_choiceUnits, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	
	fgSizer1->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 3 );
	
	m_staticText3 = new wxStaticText( m_panel1, wxID_ANY, _("&Grid size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	fgSizer1->Add( m_staticText3, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	wxArrayString m_choiceGridSizeChoices;
	m_choiceGridSize = new wxChoice( m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceGridSizeChoices, 0 );
	m_choiceGridSize->SetSelection( 0 );
	fgSizer1->Add( m_choiceGridSize, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticGridUnits = new wxStaticText( m_panel1, wxID_ANY, _("mils"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticGridUnits->Wrap( -1 );
	fgSizer1->Add( m_staticGridUnits, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText5 = new wxStaticText( m_panel1, wxID_ANY, _("Default &line width:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	fgSizer1->Add( m_staticText5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinLineWidth = new wxSpinCtrl( m_panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 0, 100, 0 );
	fgSizer1->Add( m_spinLineWidth, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticLineWidthUnits = new wxStaticText( m_panel1, wxID_ANY, _("mils"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticLineWidthUnits->Wrap( -1 );
	fgSizer1->Add( m_staticLineWidthUnits, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText7 = new wxStaticText( m_panel1, wxID_ANY, _("Default text &size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	fgSizer1->Add( m_staticText7, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinTextSize = new wxSpinCtrl( m_panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 0, 1000, 0 );
	fgSizer1->Add( m_spinTextSize, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticTextSizeUnits = new wxStaticText( m_panel1, wxID_ANY, _("mils"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextSizeUnits->Wrap( -1 );
	fgSizer1->Add( m_staticTextSizeUnits, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText9 = new wxStaticText( m_panel1, wxID_ANY, _("Repeat draw item &horizontal displacement:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	fgSizer1->Add( m_staticText9, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinRepeatHorizontal = new wxSpinCtrl( m_panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, -500, 500, 0 );
	fgSizer1->Add( m_spinRepeatHorizontal, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticRepeatXUnits = new wxStaticText( m_panel1, wxID_ANY, _("mils"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticRepeatXUnits->Wrap( -1 );
	fgSizer1->Add( m_staticRepeatXUnits, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText12 = new wxStaticText( m_panel1, wxID_ANY, _("Repeat draw item &vertical displacement:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizer1->Add( m_staticText12, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinRepeatVertical = new wxSpinCtrl( m_panel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, -500, 500, 100 );
	fgSizer1->Add( m_spinRepeatVertical, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticRepeatYUnits = new wxStaticText( m_panel1, wxID_ANY, _("mils"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticRepeatYUnits->Wrap( -1 );
	fgSizer1->Add( m_staticRepeatYUnits, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_staticText16 = new wxStaticText( m_panel1, wxID_ANY, _("&Repeat label increment:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer1->Add( m_staticText16, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinRepeatLabel = new wxSpinCtrl( m_panel1, wxID_ANY, wxT("1"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxSP_WRAP, 0, 10, 1 );
	fgSizer1->Add( m_spinRepeatLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	
	fgSizer1->Add( 0, 0, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 3 );
	
	m_staticText221 = new wxStaticText( m_panel1, wxID_ANY, _("Auto save time interval:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText221->Wrap( -1 );
	fgSizer1->Add( m_staticText221, 1, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	m_spinAutoSaveInterval = new wxSpinCtrl( m_panel1, ID_M_SPINAUTOSAVEINTERVAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1000, 10 );
	fgSizer1->Add( m_spinAutoSaveInterval, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText23 = new wxStaticText( m_panel1, wxID_ANY, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	fgSizer1->Add( m_staticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );
	
	bSizer3->Add( fgSizer1, 0, wxALIGN_CENTER|wxEXPAND, 0 );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	m_checkShowGrid = new wxCheckBox( m_panel1, wxID_ANY, _("Show g&rid"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_checkShowGrid, 0, wxALL|wxEXPAND, 3 );
	
	m_checkShowHiddenPins = new wxCheckBox( m_panel1, wxID_ANY, _("Show hi&dden pins"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_checkShowHiddenPins, 0, wxALL|wxEXPAND, 3 );
	
	m_checkAutoPan = new wxCheckBox( m_panel1, wxID_ANY, _("Enable automatic &panning"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_checkAutoPan, 0, wxALL|wxEXPAND, 3 );
	
	m_checkHVOrientation = new wxCheckBox( m_panel1, wxID_ANY, _("Allow buses and wires to be placed in H or V &orientation only"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_checkHVOrientation, 0, wxALL|wxEXPAND, 3 );
	
	m_checkPageLimits = new wxCheckBox( m_panel1, wxID_ANY, _("Show p&age limits"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_checkPageLimits, 0, wxALL|wxEXPAND, 3 );
	
	bSizer3->Add( bSizer2, 0, wxEXPAND, 0 );
	
	
	bSizer3->Add( 0, 0, 1, wxALL|wxEXPAND, 10 );
	
	p1mainSizer->Add( bSizer3, 1, wxALL|wxEXPAND, 12 );
	
	m_panel1->SetSizer( p1mainSizer );
	m_panel1->Layout();
	p1mainSizer->Fit( m_panel1 );
	m_notebook1->AddPage( m_panel1, _("General Options"), true );
	m_panel2 = new wxPanel( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel2->SetToolTip( _("User defined field names for schematic components. ") );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText211 = new wxStaticText( m_panel2, wxID_ANY, _("Please enter fieldnames which you want presented in the component fieldname (property) editors.  Names may not include (, ), or \" characters."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText211->Wrap( 400 );
	bSizer8->Add( m_staticText211, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	bSizer6->Add( bSizer8, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 8, 2, 0, 0 );
	fgSizer2->AddGrowableCol( 1 );
	fgSizer2->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText15 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 1"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	fgSizer2->Add( m_staticText15, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName1 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName1, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText161 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 2"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText161->Wrap( -1 );
	fgSizer2->Add( m_staticText161, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName2 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName2, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText17 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 3"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	fgSizer2->Add( m_staticText17, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName3 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName3, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText18 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 4"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	fgSizer2->Add( m_staticText18, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName4 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName4, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText19 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 5"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	fgSizer2->Add( m_staticText19, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName5 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName5, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText20 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 6"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	fgSizer2->Add( m_staticText20, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName6 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName6, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText21 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 7"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	fgSizer2->Add( m_staticText21, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName7 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName7, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	m_staticText22 = new wxStaticText( m_panel2, wxID_ANY, _("Custom field 8"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	fgSizer2->Add( m_staticText22, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 3 );
	
	m_fieldName8 = new wxTextCtrl( m_panel2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_fieldName8, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 3 );
	
	bSizer7->Add( fgSizer2, 1, wxALIGN_CENTER|wxEXPAND, 5 );
	
	bSizer6->Add( bSizer7, 1, wxALL|wxEXPAND, 12 );
	
	m_panel2->SetSizer( bSizer6 );
	m_panel2->Layout();
	bSizer6->Fit( m_panel2 );
	m_notebook1->AddPage( m_panel2, _("Template Field Names"), false );
	
	bOptionsSizer->Add( m_notebook1, 1, wxEXPAND, 0 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	bOptionsSizer->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 12 );
	
	mainSizer->Add( bOptionsSizer, 1, 0, 12 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	mainSizer->Fit( this );
	
	this->Centre( wxBOTH );
}

DIALOG_EESCHEMA_OPTIONS_BASE::~DIALOG_EESCHEMA_OPTIONS_BASE()
{
}
