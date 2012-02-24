///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug 24 2011)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_PAD_PROPERTIES_BASE_H__
#define __DIALOG_PAD_PROPERTIES_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/radiobox.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_PAD_PROPERTIES_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_PAD_PROPERTIES_BASE : public wxDialog 
{
	private:
	
	protected:
		enum
		{
			wxID_DIALOG_EDIT_PAD = 1000,
			wxID_PADNUMCTRL,
			wxID_PADNETNAMECTRL,
			wxID_DDIRX,
			wxID_DDIRY,
			ID_LISTBOX_SHAPE_PAD,
			ID_RADIOBOX_DRILL_SHAPE,
			ID_LISTBOX_ORIENT_PAD,
			ID_LISTBOX_TYPE_PAD,
		};
		
		wxStaticText* m_PadNumText;
		wxTextCtrl* m_PadNumCtrl;
		wxStaticText* m_PadNameText;
		wxTextCtrl* m_PadNetNameCtrl;
		wxStaticText* m_staticText4;
		wxTextCtrl* m_PadPosition_X_Ctrl;
		wxStaticText* m_PadPosX_Unit;
		wxStaticText* m_staticText41;
		wxTextCtrl* m_PadPosition_Y_Ctrl;
		wxStaticText* m_PadPosY_Unit;
		wxStaticLine* m_staticline7;
		wxStaticLine* m_staticline8;
		wxStaticLine* m_staticline9;
		wxStaticText* m_textPadDrillX;
		wxTextCtrl* m_PadDrill_X_Ctrl;
		wxStaticText* m_PadDrill_X_Unit;
		wxStaticText* m_textPadDrillY;
		wxTextCtrl* m_PadDrill_Y_Ctrl;
		wxStaticText* m_PadDrill_Y_Unit;
		wxStaticLine* m_staticline4;
		wxStaticLine* m_staticline5;
		wxStaticLine* m_staticline6;
		wxStaticText* m_staticText12;
		wxTextCtrl* m_ShapeSize_X_Ctrl;
		wxStaticText* m_PadShapeSizeX_Unit;
		wxStaticText* m_staticText15;
		wxTextCtrl* m_ShapeSize_Y_Ctrl;
		wxStaticText* m_PadShapeSizeY_Unit;
		wxStaticText* m_staticText17;
		wxTextCtrl* m_ShapeOffset_X_Ctrl;
		wxStaticText* m_PadShapeOffsetX_Unit;
		wxStaticText* m_staticText19;
		wxTextCtrl* m_ShapeOffset_Y_Ctrl;
		wxStaticText* m_PadShapeOffsetY_Unit;
		wxStaticText* m_staticText21;
		wxTextCtrl* m_ShapeDelta_Ctrl;
		wxStaticText* m_PadShapeDelta_Unit;
		wxStaticText* m_staticText23;
		wxRadioButton* m_radioBtnDeltaXdir;
		wxRadioButton* m_radioBtnDeltaYdir;
		wxStaticText* m_staticTextDDirInfo;
		wxStaticLine* m_staticline10;
		wxStaticLine* m_staticline101;
		wxStaticLine* m_staticline1011;
		wxStaticText* m_staticText38;
		wxTextCtrl* m_LengthDieCtrl;
		wxStaticText* m_PadLengthDie_Unit;
		wxBoxSizer* m_DrillShapeBoxSizer;
		wxRadioBox* m_PadShape;
		wxRadioBox* m_DrillShapeCtrl;
		wxRadioBox* m_PadOrient;
		wxStaticText* m_PadOrientText;
		wxTextCtrl* m_PadOrientCtrl;
		wxPanel* m_panelShowPad;
		wxStaticText* m_staticTitleModuleRot;
		wxStaticText* m_staticModuleRotValue;
		wxStaticText* m_staticTitleModuleSide;
		wxStaticText* m_staticModuleSideValue;
		wxStaticText* m_staticTextWarningPadFlipped;
		wxStaticText* m_staticText40;
		wxChoice* m_ZoneConnectionChoice;
		wxStaticText* m_staticTextWarning;
		wxStaticText* m_staticTextNetClearance;
		wxTextCtrl* m_NetClearanceValueCtrl;
		wxStaticText* m_NetClearanceUnits;
		wxStaticLine* m_staticline1;
		wxStaticLine* m_staticline2;
		wxStaticLine* m_staticline3;
		wxStaticText* m_MaskClearanceTitle;
		wxTextCtrl* m_SolderMaskMarginCtrl;
		wxStaticText* m_SolderMaskMarginUnits;
		wxStaticText* m_staticTextSolderPaste;
		wxTextCtrl* m_SolderPasteMarginCtrl;
		wxStaticText* m_SolderPasteMarginUnits;
		wxStaticText* m_staticTextRatio;
		wxTextCtrl* m_SolderPasteMarginRatioCtrl;
		wxStaticText* m_SolderPasteRatioMarginUnits;
		wxRadioBox* m_PadType;
		wxRadioBox* m_rbCopperLayersSel;
		wxCheckBox* m_PadLayerAdhCmp;
		wxCheckBox* m_PadLayerAdhCu;
		wxCheckBox* m_PadLayerPateCmp;
		wxCheckBox* m_PadLayerPateCu;
		wxCheckBox* m_PadLayerSilkCmp;
		wxCheckBox* m_PadLayerSilkCu;
		wxCheckBox* m_PadLayerMaskCmp;
		wxCheckBox* m_PadLayerMaskCu;
		wxCheckBox* m_PadLayerDraft;
		wxCheckBox* m_PadLayerECO1;
		wxCheckBox* m_PadLayerECO2;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnValuesChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPadShapeSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDrillShapeSelected( wxCommandEvent& event ) { event.Skip(); }
		virtual void PadOrientEvent( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPaintShowPanel( wxPaintEvent& event ) { event.Skip(); }
		virtual void PadTypeSelected( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSetLayers( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancelButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void PadPropertiesAccept( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_PAD_PROPERTIES_BASE( wxWindow* parent, wxWindowID id = wxID_DIALOG_EDIT_PAD, const wxString& title = _("Pad Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 900,800 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxSUNKEN_BORDER ); 
		~DIALOG_PAD_PROPERTIES_BASE();
	
};

#endif //__DIALOG_PAD_PROPERTIES_BASE_H__
