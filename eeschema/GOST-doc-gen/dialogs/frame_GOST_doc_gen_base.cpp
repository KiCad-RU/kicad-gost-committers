///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "frame_GOST_doc_gen_base.h"

///////////////////////////////////////////////////////////////////////////

FRAME_GOST_DOC_GEN_BASE::FRAME_GOST_DOC_GEN_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_menubar1 = new wxMenuBar( 0 );
	m_menuFile = new wxMenu();
	wxMenuItem* m_menuFileGenerateComponentIndex;
	m_menuFileGenerateComponentIndex = new wxMenuItem( m_menuFile, wxID_ANY, wxString( _("Generate components index") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuFile->Append( m_menuFileGenerateComponentIndex );
	
	wxMenuItem* m_menuFileGenerateSpecification;
	m_menuFileGenerateSpecification = new wxMenuItem( m_menuFile, wxID_ANY, wxString( _("Generate specification") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuFile->Append( m_menuFileGenerateSpecification );
	
	m_menubar1->Append( m_menuFile, _("&File") ); 
	
	m_menuSettings = new wxMenu();
	wxMenuItem* m_menuSettingsAddaNewVariant;
	m_menuSettingsAddaNewVariant = new wxMenuItem( m_menuSettings, wxID_ANY, wxString( _("Add a new variant") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuSettings->Append( m_menuSettingsAddaNewVariant );
	
	wxMenuItem* m_menuSettingsRemoveExistingVariant;
	m_menuSettingsRemoveExistingVariant = new wxMenuItem( m_menuSettings, wxID_ANY, wxString( _("Remove existing variant") ) , wxEmptyString, wxITEM_NORMAL );
	m_menuSettings->Append( m_menuSettingsRemoveExistingVariant );
	m_menuSettingsRemoveExistingVariant->Enable( false );
	
	m_menubar1->Append( m_menuSettings, _("Settings") ); 
	
	this->SetMenuBar( m_menubar1 );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->AddGrowableRow( 1 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticBoxEditing = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Editing") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 8, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, _("Title:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText1->Wrap( -1 );
	fgSizer1->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 6 );
	
	m_combo_Name = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 );
	m_combo_Name->Append( _("Capacitor") );
	m_combo_Name->Append( _("Resistor") );
	m_combo_Name->Append( _("IC") );
	m_combo_Name->Append( _("Connector") );
	m_combo_Name->Append( _("Diode") );
	m_combo_Name->Append( _("Resistor array") );
	m_combo_Name->Append( _("Diode array") );
	m_combo_Name->Append( _("Zener") );
	m_combo_Name->Append( _("Choke") );
	m_combo_Name->Append( _("Transformer") );
	m_combo_Name->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	fgSizer1->Add( m_combo_Name, 0, wxALL, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, _("Type:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText2->Wrap( -1 );
	fgSizer1->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT|wxRIGHT, 6 );
	
	m_combo_Type = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 );
	m_combo_Type->Append( _("tantalum type C") );
	fgSizer1->Add( m_combo_Type, 0, wxALL, 5 );
	
	m_staticText3 = new wxStaticText( this, wxID_ANY, _("Subtype:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText3->Wrap( -1 );
	fgSizer1->Add( m_staticText3, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT|wxRIGHT, 6 );
	
	m_combo_SubType = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 );
	m_combo_SubType->Append( _("-X7R-50 V-") );
	m_combo_SubType->Append( _("-NPO-50 V-") );
	m_combo_SubType->Append( _("-Y5V-50 V-") );
	m_combo_SubType->Append( _("-16 V-") );
	fgSizer1->Add( m_combo_SubType, 0, wxALL, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Value:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText4->Wrap( -1 );
	fgSizer1->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT|wxRIGHT, 6 );
	
	m_combo_Value = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 ); 
	fgSizer1->Add( m_combo_Value, 0, wxALL, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, _("Precision:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText5->Wrap( -1 );
	fgSizer1->Add( m_staticText5, 0, wxALL, 5 );
	
	m_combo_Precision = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 );
	m_combo_Precision->Append( _("5%") );
	m_combo_Precision->Append( _("10%") );
	m_combo_Precision->Append( _("20%") );
	fgSizer1->Add( m_combo_Precision, 0, wxALL, 5 );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, _("Note:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText6->Wrap( -1 );
	fgSizer1->Add( m_staticText6, 0, wxALL, 5 );
	
	m_combo_Note = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 );
	m_combo_Note->Append( _("Not installed") );
	m_combo_Note->Append( _("The replacement is permitted with") );
	fgSizer1->Add( m_combo_Note, 0, wxALL, 5 );
	
	m_staticText7 = new wxStaticText( this, wxID_ANY, _("Designation:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText7->Wrap( -1 );
	fgSizer1->Add( m_staticText7, 0, wxALL, 5 );
	
	m_combo_Designation = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 ); 
	fgSizer1->Add( m_combo_Designation, 0, wxALL, 5 );
	
	m_staticText8 = new wxStaticText( this, wxID_ANY, _("Manufacturer:"), wxDefaultPosition, wxSize( -1,25 ), 0 );
	m_staticText8->Wrap( -1 );
	fgSizer1->Add( m_staticText8, 0, wxALL, 5 );
	
	m_combo_Manufacturer = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 230,25 ), 0, NULL, 0 ); 
	fgSizer1->Add( m_combo_Manufacturer, 0, wxALL, 5 );
	
	m_staticBoxEditing->Add( fgSizer1, 1, wxEXPAND, 5 );
	
	bSizer1->Add( m_staticBoxEditing, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Table represantation mode selection") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 3, 2, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_radio_FullList = new wxRadioButton( this, wxID_ANY, _("Full component list"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_radio_FullList, 0, wxALL, 5 );
	
	
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_radio_ConstPart = new wxRadioButton( this, wxID_ANY, _("Constant part"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_radio_ConstPart, 0, wxALL, 5 );
	
	
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_radio_VarPart = new wxRadioButton( this, wxID_ANY, _("Variable part, var. no."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer2->Add( m_radio_VarPart, 0, wxALL, 5 );
	
	m_combo_Variant = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,25 ), 0, NULL, 0 ); 
	fgSizer2->Add( m_combo_Variant, 0, wxALL, 5 );
	
	sbSizer2->Add( fgSizer2, 1, wxEXPAND, 5 );
	
	bSizer1->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Selected component by variant") ), wxVERTICAL );
	
	wxArrayString m_checkListCtrlChoices;
	m_checkListCtrl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), m_checkListCtrlChoices, 0 );
	sbSizer3->Add( m_checkListCtrl, 0, wxALL|wxEXPAND, 5 );
	
	bSizer1->Add( sbSizer3, 1, wxEXPAND, 5 );
	
	fgSizer6->Add( bSizer1, 1, wxALL|wxEXPAND, 6 );
	
	m_listCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SORT_ASCENDING|wxLC_VRULES );
	fgSizer6->Add( m_listCtrl, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer6 );
	this->Layout();
	fgSizer6->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_ACTIVATE, wxActivateEventHandler( FRAME_GOST_DOC_GEN_BASE::OnActivate ) );
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( FRAME_GOST_DOC_GEN_BASE::OnCloseWindow ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSize ) );
	this->Connect( m_menuFileGenerateComponentIndex->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnFileGenerateComponentIndex ) );
	this->Connect( m_menuFileGenerateSpecification->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnFileGenerateSpecification ) );
	this->Connect( m_menuSettingsAddaNewVariant->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSettingsAddaNewVariant ) );
	this->Connect( m_menuSettingsRemoveExistingVariant->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSettingsRemoveExistingVariant ) );
	m_combo_Name->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboName ), NULL, this );
	m_combo_Type->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboType ), NULL, this );
	m_combo_SubType->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboSubtype ), NULL, this );
	m_combo_Value->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboValue ), NULL, this );
	m_combo_Precision->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboPrecision ), NULL, this );
	m_combo_Note->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboNote ), NULL, this );
	m_combo_Designation->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboDesignation ), NULL, this );
	m_combo_Manufacturer->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboManufacturer ), NULL, this );
	m_radio_FullList->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnRadioFullList ), NULL, this );
	m_radio_ConstPart->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnRadioConstPart ), NULL, this );
	m_radio_VarPart->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnRadioVarPart ), NULL, this );
	m_combo_Variant->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSelChangeComboVariant ), NULL, this );
	m_checkListCtrl->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnItemChangedCheckListCtrl ), NULL, this );
	m_listCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( FRAME_GOST_DOC_GEN_BASE::OnClickListCtrl ), NULL, this );
}

FRAME_GOST_DOC_GEN_BASE::~FRAME_GOST_DOC_GEN_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_ACTIVATE, wxActivateEventHandler( FRAME_GOST_DOC_GEN_BASE::OnActivate ) );
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( FRAME_GOST_DOC_GEN_BASE::OnCloseWindow ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSize ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnFileGenerateComponentIndex ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnFileGenerateSpecification ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSettingsAddaNewVariant ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSettingsRemoveExistingVariant ) );
	m_combo_Name->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboName ), NULL, this );
	m_combo_Type->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboType ), NULL, this );
	m_combo_SubType->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboSubtype ), NULL, this );
	m_combo_Value->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboValue ), NULL, this );
	m_combo_Precision->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboPrecision ), NULL, this );
	m_combo_Note->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboNote ), NULL, this );
	m_combo_Designation->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboDesignation ), NULL, this );
	m_combo_Manufacturer->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnEditChangeComboManufacturer ), NULL, this );
	m_radio_FullList->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnRadioFullList ), NULL, this );
	m_radio_ConstPart->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnRadioConstPart ), NULL, this );
	m_radio_VarPart->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnRadioVarPart ), NULL, this );
	m_combo_Variant->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnSelChangeComboVariant ), NULL, this );
	m_checkListCtrl->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( FRAME_GOST_DOC_GEN_BASE::OnItemChangedCheckListCtrl ), NULL, this );
	m_listCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( FRAME_GOST_DOC_GEN_BASE::OnClickListCtrl ), NULL, this );
	
}
