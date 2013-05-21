///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __frame_GOST_doc_gen_base__
#define __frame_GOST_doc_gen_base__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/checklst.h>
#include <wx/listctrl.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class FRAME_GOST_DOC_GEN_BASE
///////////////////////////////////////////////////////////////////////////////
class FRAME_GOST_DOC_GEN_BASE : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menubar1;
		wxMenu* m_menuFile;
		wxMenu* m_menuSettings;
		wxStaticBoxSizer* m_staticBoxEditing;
		wxStaticText* m_staticText1;
		wxComboBox* m_combo_Name;
		wxStaticText* m_staticText2;
		wxComboBox* m_combo_Type;
		wxStaticText* m_staticText3;
		wxComboBox* m_combo_SubType;
		wxStaticText* m_staticText4;
		wxComboBox* m_combo_Value;
		wxStaticText* m_staticText5;
		wxComboBox* m_combo_Precision;
		wxStaticText* m_staticText6;
		wxComboBox* m_combo_Note;
		wxStaticText* m_staticText7;
		wxComboBox* m_combo_Designation;
		wxStaticText* m_staticText8;
		wxComboBox* m_combo_Manufacturer;
		wxRadioButton* m_radio_FullList;
		
		wxRadioButton* m_radio_ConstPart;
		
		wxRadioButton* m_radio_VarPart;
		wxComboBox* m_combo_Variant;
		wxCheckListBox* m_checkListCtrl;
		wxListCtrl* m_listCtrl;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnActivate( wxActivateEvent& event ) { event.Skip(); }
		virtual void OnCloseWindow( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSize( wxSizeEvent& event ) { event.Skip(); }
		virtual void OnFileGenerateComponentIndex( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFileGenerateSpecification( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSettingsAddaNewVariant( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSettingsRemoveExistingVariant( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSettingsDebugOn( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboName( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboType( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboSubtype( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboValue( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboPrecision( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboNote( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboDesignation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditChangeComboManufacturer( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRadioFullList( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRadioConstPart( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRadioVarPart( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelChangeComboVariant( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnItemChangedCheckListCtrl( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClickListCtrl( wxListEvent& event ) { event.Skip(); }
		
	
	public:
		wxMenuItem* m_menuSettingsDebugOn;
		
		FRAME_GOST_DOC_GEN_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("GOST Component Manager"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxFRAME_FLOAT_ON_PARENT|wxMAXIMIZE|wxTAB_TRAVERSAL );
		~FRAME_GOST_DOC_GEN_BASE();
	
};

#endif //__frame_GOST_doc_gen_base__
