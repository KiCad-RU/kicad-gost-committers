///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug 24 2011)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_pad_properties_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_PAD_PROPERTIES_BASE::DIALOG_PAD_PROPERTIES_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* m_MainSizer;
	m_MainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bGeneralSizer;
	bGeneralSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* m_LeftBoxSizer;
	m_LeftBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	m_PadNumText = new wxStaticText( this, wxID_ANY, _("Number:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadNumText->Wrap( -1 );
	m_LeftBoxSizer->Add( m_PadNumText, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadNumCtrl = new wxTextCtrl( this, wxID_PADNUMCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_LeftBoxSizer->Add( m_PadNumCtrl, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxEXPAND, 5 );
	
	m_PadNameText = new wxStaticText( this, wxID_ANY, _("Net name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadNameText->Wrap( -1 );
	m_LeftBoxSizer->Add( m_PadNameText, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadNetNameCtrl = new wxTextCtrl( this, wxID_PADNETNAMECTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_LeftBoxSizer->Add( m_PadNetNameCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Pad Geometry") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizerGeometry;
	fgSizerGeometry = new wxFlexGridSizer( 14, 3, 0, 0 );
	fgSizerGeometry->SetFlexibleDirection( wxBOTH );
	fgSizerGeometry->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Position X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_PadPosition_X_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_PadPosition_X_Ctrl, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadPosX_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadPosX_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadPosX_Unit, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	m_staticText41 = new wxStaticText( this, wxID_ANY, _("Position Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText41, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_PadPosition_Y_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_PadPosition_Y_Ctrl, 0, wxALL, 5 );
	
	m_PadPosY_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadPosY_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadPosY_Unit, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticline7 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline7, 0, wxEXPAND | wxALL, 5 );
	
	m_staticline8 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline8, 0, wxEXPAND | wxALL, 5 );
	
	m_staticline9 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline9, 0, wxEXPAND | wxALL, 5 );
	
	m_textPadDrillX = new wxStaticText( this, wxID_ANY, _("Drill X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textPadDrillX->Wrap( -1 );
	fgSizerGeometry->Add( m_textPadDrillX, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxALIGN_RIGHT, 5 );
	
	m_PadDrill_X_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_PadDrill_X_Ctrl, 0, wxALL, 5 );
	
	m_PadDrill_X_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadDrill_X_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadDrill_X_Unit, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_textPadDrillY = new wxStaticText( this, wxID_ANY, _("Drill Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textPadDrillY->Wrap( -1 );
	fgSizerGeometry->Add( m_textPadDrillY, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_PadDrill_Y_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_PadDrill_Y_Ctrl, 0, wxALL, 5 );
	
	m_PadDrill_Y_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadDrill_Y_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadDrill_Y_Unit, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );
	
	m_staticline5 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline5, 0, wxEXPAND | wxALL, 5 );
	
	m_staticline6 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline6, 0, wxEXPAND | wxALL, 5 );
	
	m_staticText12 = new wxStaticText( this, wxID_ANY, _("Shape size X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText12, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_ShapeSize_X_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_ShapeSize_X_Ctrl, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadShapeSizeX_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadShapeSizeX_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadShapeSizeX_Unit, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	m_staticText15 = new wxStaticText( this, wxID_ANY, _("Shape size Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText15, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_ShapeSize_Y_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_ShapeSize_Y_Ctrl, 0, wxALL, 5 );
	
	m_PadShapeSizeY_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadShapeSizeY_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadShapeSizeY_Unit, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText17 = new wxStaticText( this, wxID_ANY, _("Shape offset X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );
	
	m_ShapeOffset_X_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_ShapeOffset_X_Ctrl, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadShapeOffsetX_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadShapeOffsetX_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadShapeOffsetX_Unit, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText19 = new wxStaticText( this, wxID_ANY, _("Shape offset Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText19, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_ShapeOffset_Y_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_ShapeOffset_Y_Ctrl, 0, wxALL, 5 );
	
	m_PadShapeOffsetY_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadShapeOffsetY_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadShapeOffsetY_Unit, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText21 = new wxStaticText( this, wxID_ANY, _("Shape delta dim"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText21, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 5 );
	
	m_ShapeDelta_Ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_ShapeDelta_Ctrl, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadShapeDelta_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadShapeDelta_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadShapeDelta_Unit, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText23 = new wxStaticText( this, wxID_ANY, _("Trap. direction"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	fgSizerGeometry->Add( m_staticText23, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizerbdir;
	bSizerbdir = new wxBoxSizer( wxHORIZONTAL );
	
	m_radioBtnDeltaXdir = new wxRadioButton( this, wxID_DDIRX, _(">"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radioBtnDeltaXdir->SetValue( true ); 
	bSizerbdir->Add( m_radioBtnDeltaXdir, 0, wxALL, 5 );
	
	m_radioBtnDeltaYdir = new wxRadioButton( this, wxID_DDIRY, _("^"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizerbdir->Add( m_radioBtnDeltaYdir, 0, wxALL, 5 );
	
	fgSizerGeometry->Add( bSizerbdir, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_staticTextDDirInfo = new wxStaticText( this, wxID_ANY, _("Rot 0"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextDDirInfo->Wrap( -1 );
	fgSizerGeometry->Add( m_staticTextDDirInfo, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	m_staticline10 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline10, 0, wxEXPAND | wxALL, 5 );
	
	m_staticline101 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline101, 0, wxEXPAND | wxALL, 5 );
	
	m_staticline1011 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizerGeometry->Add( m_staticline1011, 0, wxEXPAND | wxALL, 5 );
	
	m_staticText38 = new wxStaticText( this, wxID_ANY, _("Length die"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText38->Wrap( -1 );
	m_staticText38->SetToolTip( _("Wire length from pad to die on chip ( used to calculate actual track length)") );
	
	fgSizerGeometry->Add( m_staticText38, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 5 );
	
	m_LengthDieCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizerGeometry->Add( m_LengthDieCtrl, 0, wxALL, 5 );
	
	m_PadLengthDie_Unit = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadLengthDie_Unit->Wrap( -1 );
	fgSizerGeometry->Add( m_PadLengthDie_Unit, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );
	
	sbSizer2->Add( fgSizerGeometry, 1, wxEXPAND, 5 );
	
	m_LeftBoxSizer->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	bGeneralSizer->Add( m_LeftBoxSizer, 0, wxBOTTOM|wxLEFT|wxEXPAND, 5 );
	
	wxBoxSizer* bMiddleSizer;
	bMiddleSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bMiddleUpperSizer;
	bMiddleUpperSizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_DrillShapeBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	wxString m_PadShapeChoices[] = { _("Circle"), _("Oval"), _("Rect"), _("Trapezoidal") };
	int m_PadShapeNChoices = sizeof( m_PadShapeChoices ) / sizeof( wxString );
	m_PadShape = new wxRadioBox( this, ID_LISTBOX_SHAPE_PAD, _("Pad Shape"), wxDefaultPosition, wxDefaultSize, m_PadShapeNChoices, m_PadShapeChoices, 1, wxRA_SPECIFY_COLS );
	m_PadShape->SetSelection( 0 );
	m_DrillShapeBoxSizer->Add( m_PadShape, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_DrillShapeCtrlChoices[] = { _("Circle"), _("Oval") };
	int m_DrillShapeCtrlNChoices = sizeof( m_DrillShapeCtrlChoices ) / sizeof( wxString );
	m_DrillShapeCtrl = new wxRadioBox( this, ID_RADIOBOX_DRILL_SHAPE, _("Drill Shape"), wxDefaultPosition, wxDefaultSize, m_DrillShapeCtrlNChoices, m_DrillShapeCtrlChoices, 1, wxRA_SPECIFY_COLS );
	m_DrillShapeCtrl->SetSelection( 0 );
	m_DrillShapeBoxSizer->Add( m_DrillShapeCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	bMiddleUpperSizer->Add( m_DrillShapeBoxSizer, 0, wxBOTTOM, 5 );
	
	wxBoxSizer* m_MiddleRightBoxSizer;
	m_MiddleRightBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	wxString m_PadOrientChoices[] = { _("0"), _("90"), _("-90"), _("180"), _("Custom") };
	int m_PadOrientNChoices = sizeof( m_PadOrientChoices ) / sizeof( wxString );
	m_PadOrient = new wxRadioBox( this, ID_LISTBOX_ORIENT_PAD, _("Pad Orient"), wxDefaultPosition, wxDefaultSize, m_PadOrientNChoices, m_PadOrientChoices, 1, wxRA_SPECIFY_COLS );
	m_PadOrient->SetSelection( 0 );
	m_MiddleRightBoxSizer->Add( m_PadOrient, 0, wxALL|wxEXPAND, 5 );
	
	m_PadOrientText = new wxStaticText( this, wxID_ANY, _("Pad Orient (0.1 deg)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_PadOrientText->Wrap( -1 );
	m_MiddleRightBoxSizer->Add( m_PadOrientText, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadOrientCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_MiddleRightBoxSizer->Add( m_PadOrientCtrl, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	bMiddleUpperSizer->Add( m_MiddleRightBoxSizer, 0, wxBOTTOM, 5 );
	
	m_panelShowPad = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxSIMPLE_BORDER|wxTAB_TRAVERSAL );
	m_panelShowPad->SetBackgroundColour( wxColour( 0, 0, 0 ) );
	
	bMiddleUpperSizer->Add( m_panelShowPad, 1, wxEXPAND, 5 );
	
	bMiddleSizer->Add( bMiddleUpperSizer, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizeModuleInfo;
	sbSizeModuleInfo = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Footprint Orientation") ), wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer4->AddGrowableCol( 1 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticTitleModuleRot = new wxStaticText( this, wxID_ANY, _("Rotation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTitleModuleRot->Wrap( -1 );
	fgSizer4->Add( m_staticTitleModuleRot, 0, wxALIGN_RIGHT|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_staticModuleRotValue = new wxStaticText( this, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticModuleRotValue->Wrap( -1 );
	fgSizer4->Add( m_staticModuleRotValue, 0, wxTOP|wxRIGHT|wxLEFT|wxEXPAND, 5 );
	
	m_staticTitleModuleSide = new wxStaticText( this, wxID_ANY, _("Board side:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTitleModuleSide->Wrap( -1 );
	fgSizer4->Add( m_staticTitleModuleSide, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	m_staticModuleSideValue = new wxStaticText( this, wxID_ANY, _("Front side"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticModuleSideValue->Wrap( -1 );
	fgSizer4->Add( m_staticModuleSideValue, 0, wxALL|wxEXPAND, 5 );
	
	sbSizeModuleInfo->Add( fgSizer4, 1, wxEXPAND, 5 );
	
	m_staticTextWarningPadFlipped = new wxStaticText( this, wxID_ANY, _("Warning:\nThis pad is flipped on board.\nBack and front layers will be swapped."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextWarningPadFlipped->Wrap( -1 );
	m_staticTextWarningPadFlipped->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
	
	sbSizeModuleInfo->Add( m_staticTextWarningPadFlipped, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	bMiddleSizer->Add( sbSizeModuleInfo, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	wxStaticBoxSizer* sbClearancesSizer;
	sbClearancesSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Local Settings") ), wxVERTICAL );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText40 = new wxStaticText( this, wxID_ANY, _("Pad connection to zones:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText40->Wrap( -1 );
	bSizer12->Add( m_staticText40, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxLEFT, 5 );
	
	wxString m_ZoneConnectionChoiceChoices[] = { _("From parent module"), _("Solid"), _("Thermal relief"), _("None") };
	int m_ZoneConnectionChoiceNChoices = sizeof( m_ZoneConnectionChoiceChoices ) / sizeof( wxString );
	m_ZoneConnectionChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ZoneConnectionChoiceNChoices, m_ZoneConnectionChoiceChoices, 0 );
	m_ZoneConnectionChoice->SetSelection( 0 );
	bSizer12->Add( m_ZoneConnectionChoice, 0, wxALL, 5 );
	
	bSizer13->Add( bSizer12, 0, wxEXPAND, 5 );
	
	m_staticTextWarning = new wxStaticText( this, wxID_ANY, _("Set clearances to 0\nto use Parent footprint or global values"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextWarning->Wrap( -1 );
	m_staticTextWarning->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
	
	bSizer13->Add( m_staticTextWarning, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	sbClearancesSizer->Add( bSizer13, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgClearancesGridSizer;
	fgClearancesGridSizer = new wxFlexGridSizer( 5, 3, 0, 0 );
	fgClearancesGridSizer->SetFlexibleDirection( wxBOTH );
	fgClearancesGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticTextNetClearance = new wxStaticText( this, wxID_ANY, _("Net pad clearance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextNetClearance->Wrap( -1 );
	m_staticTextNetClearance->SetToolTip( _("This is the local net clearance for  pad.\nIf 0, the footprint local value or the Netclass value is used") );
	
	fgClearancesGridSizer->Add( m_staticTextNetClearance, 0, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_NetClearanceValueCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgClearancesGridSizer->Add( m_NetClearanceValueCtrl, 0, wxALL|wxEXPAND, 5 );
	
	m_NetClearanceUnits = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_NetClearanceUnits->Wrap( -1 );
	fgClearancesGridSizer->Add( m_NetClearanceUnits, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgClearancesGridSizer->Add( m_staticline1, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgClearancesGridSizer->Add( m_staticline2, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	m_staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgClearancesGridSizer->Add( m_staticline3, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	m_MaskClearanceTitle = new wxStaticText( this, wxID_ANY, _("Solder mask clearance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_MaskClearanceTitle->Wrap( -1 );
	m_MaskClearanceTitle->SetToolTip( _("This is the local clearance between this  pad and the solder mask\nIf 0, the footprint local value or the global value is used") );
	
	fgClearancesGridSizer->Add( m_MaskClearanceTitle, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_SolderMaskMarginCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgClearancesGridSizer->Add( m_SolderMaskMarginCtrl, 0, wxALL|wxEXPAND, 5 );
	
	m_SolderMaskMarginUnits = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SolderMaskMarginUnits->Wrap( -1 );
	fgClearancesGridSizer->Add( m_SolderMaskMarginUnits, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticTextSolderPaste = new wxStaticText( this, wxID_ANY, _("Solder paste clearance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextSolderPaste->Wrap( -1 );
	m_staticTextSolderPaste->SetToolTip( _("This is the local clearance between this pad and the solder paste.\nIf 0 the footprint value or the global value is used..\nThe final clearance value is the sum of this value and the clearance value ratio\nA negative value means a smaller mask size than pad size") );
	
	fgClearancesGridSizer->Add( m_staticTextSolderPaste, 0, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_SolderPasteMarginCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgClearancesGridSizer->Add( m_SolderPasteMarginCtrl, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_SolderPasteMarginUnits = new wxStaticText( this, wxID_ANY, _("Inch"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SolderPasteMarginUnits->Wrap( -1 );
	fgClearancesGridSizer->Add( m_SolderPasteMarginUnits, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticTextRatio = new wxStaticText( this, wxID_ANY, _("Solder mask ratio clearance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextRatio->Wrap( -1 );
	m_staticTextRatio->SetToolTip( _("This is the local clearance ratio in per cent between this pad and the solder paste.\nA value of 10 means the clearance value is 10 per cent of the pad size\nIf 0 the footprint value or the global value is used..\nThe final clearance value is the sum of this value and the clearance value\nA negative value means a smaller mask size than pad size.") );
	
	fgClearancesGridSizer->Add( m_staticTextRatio, 0, wxLEFT|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_SolderPasteMarginRatioCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgClearancesGridSizer->Add( m_SolderPasteMarginRatioCtrl, 0, wxALL|wxEXPAND, 5 );
	
	m_SolderPasteRatioMarginUnits = new wxStaticText( this, wxID_ANY, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SolderPasteRatioMarginUnits->Wrap( -1 );
	fgClearancesGridSizer->Add( m_SolderPasteRatioMarginUnits, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	sbClearancesSizer->Add( fgClearancesGridSizer, 1, wxEXPAND, 5 );
	
	bMiddleSizer->Add( sbClearancesSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	bGeneralSizer->Add( bMiddleSizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* m_RightBoxSizer;
	m_RightBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	wxString m_PadTypeChoices[] = { _("Standard"), _("SMD"), _("Conn"), _("NPTH, Mechanical") };
	int m_PadTypeNChoices = sizeof( m_PadTypeChoices ) / sizeof( wxString );
	m_PadType = new wxRadioBox( this, ID_LISTBOX_TYPE_PAD, _("Pad Type"), wxDefaultPosition, wxDefaultSize, m_PadTypeNChoices, m_PadTypeChoices, 1, wxRA_SPECIFY_COLS );
	m_PadType->SetSelection( 0 );
	m_RightBoxSizer->Add( m_PadType, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* m_LayersSizer;
	m_LayersSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Layers") ), wxVERTICAL );
	
	wxString m_rbCopperLayersSelChoices[] = { _("Front layer"), _("Back layer"), _("All copper layers"), _("No copper layers") };
	int m_rbCopperLayersSelNChoices = sizeof( m_rbCopperLayersSelChoices ) / sizeof( wxString );
	m_rbCopperLayersSel = new wxRadioBox( this, wxID_ANY, _("Copper Layers"), wxDefaultPosition, wxDefaultSize, m_rbCopperLayersSelNChoices, m_rbCopperLayersSelChoices, 1, wxRA_SPECIFY_COLS );
	m_rbCopperLayersSel->SetSelection( 2 );
	m_LayersSizer->Add( m_rbCopperLayersSel, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizerTechlayers;
	sbSizerTechlayers = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Technical Layers") ), wxVERTICAL );
	
	m_PadLayerAdhCmp = new wxCheckBox( this, wxID_ANY, _("Adhesive Cmp"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerAdhCmp, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerAdhCu = new wxCheckBox( this, wxID_ANY, _("Adhesive Copper"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerAdhCu, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerPateCmp = new wxCheckBox( this, wxID_ANY, _("Solder paste Cmp"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerPateCmp, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerPateCu = new wxCheckBox( this, wxID_ANY, _("Solder paste Copper"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerPateCu, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerSilkCmp = new wxCheckBox( this, wxID_ANY, _("Silkscreen Cmp"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerSilkCmp, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerSilkCu = new wxCheckBox( this, wxID_ANY, _("Silkscreen Copper"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerSilkCu, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerMaskCmp = new wxCheckBox( this, wxID_ANY, _("Solder mask Cmp"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerMaskCmp, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerMaskCu = new wxCheckBox( this, wxID_ANY, _("Solder mask Copper"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerMaskCu, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerDraft = new wxCheckBox( this, wxID_ANY, _("Draft layer"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerDraft, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerECO1 = new wxCheckBox( this, wxID_ANY, _("E.C.O.1 layer"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerECO1, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_PadLayerECO2 = new wxCheckBox( this, wxID_ANY, _("E.C.O.2 layer"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerTechlayers->Add( m_PadLayerECO2, 0, wxALL, 5 );
	
	m_LayersSizer->Add( sbSizerTechlayers, 1, wxALL|wxEXPAND, 5 );
	
	m_RightBoxSizer->Add( m_LayersSizer, 0, wxALL, 5 );
	
	bGeneralSizer->Add( m_RightBoxSizer, 0, wxBOTTOM|wxRIGHT|wxEXPAND, 5 );
	
	m_MainSizer->Add( bGeneralSizer, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	m_MainSizer->Add( m_sdbSizer1, 0, wxTOP|wxBOTTOM|wxEXPAND, 5 );
	
	this->SetSizer( m_MainSizer );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_PadNumCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadNetNameCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadDrill_X_Ctrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadDrill_Y_Ctrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeSize_X_Ctrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeSize_Y_Ctrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeOffset_X_Ctrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeOffset_Y_Ctrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeDelta_Ctrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_radioBtnDeltaXdir->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_radioBtnDeltaYdir->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadShape->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnPadShapeSelection ), NULL, this );
	m_DrillShapeCtrl->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnDrillShapeSelected ), NULL, this );
	m_PadOrient->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::PadOrientEvent ), NULL, this );
	m_PadOrientCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_panelShowPad->Connect( wxEVT_PAINT, wxPaintEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnPaintShowPanel ), NULL, this );
	m_NetClearanceValueCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadType->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::PadTypeSelected ), NULL, this );
	m_rbCopperLayersSel->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerAdhCmp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerAdhCu->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerPateCmp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerPateCu->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerSilkCmp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerSilkCu->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerMaskCmp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerMaskCu->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerDraft->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerECO1->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerECO2->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_sdbSizer1Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnCancelButtonClick ), NULL, this );
	m_sdbSizer1OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::PadPropertiesAccept ), NULL, this );
}

DIALOG_PAD_PROPERTIES_BASE::~DIALOG_PAD_PROPERTIES_BASE()
{
	// Disconnect Events
	m_PadNumCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadNetNameCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadDrill_X_Ctrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadDrill_Y_Ctrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeSize_X_Ctrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeSize_Y_Ctrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeOffset_X_Ctrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeOffset_Y_Ctrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_ShapeDelta_Ctrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_radioBtnDeltaXdir->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_radioBtnDeltaYdir->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadShape->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnPadShapeSelection ), NULL, this );
	m_DrillShapeCtrl->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnDrillShapeSelected ), NULL, this );
	m_PadOrient->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::PadOrientEvent ), NULL, this );
	m_PadOrientCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_panelShowPad->Disconnect( wxEVT_PAINT, wxPaintEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnPaintShowPanel ), NULL, this );
	m_NetClearanceValueCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnValuesChanged ), NULL, this );
	m_PadType->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::PadTypeSelected ), NULL, this );
	m_rbCopperLayersSel->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerAdhCmp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerAdhCu->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerPateCmp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerPateCu->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerSilkCmp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerSilkCu->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerMaskCmp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerMaskCu->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerDraft->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerECO1->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_PadLayerECO2->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnSetLayers ), NULL, this );
	m_sdbSizer1Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::OnCancelButtonClick ), NULL, this );
	m_sdbSizer1OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_PAD_PROPERTIES_BASE::PadPropertiesAccept ), NULL, this );
	
}
