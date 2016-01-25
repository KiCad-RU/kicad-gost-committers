///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jan  1 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_REGULATOR_DATA_BASE_H__
#define __DIALOG_REGULATOR_DATA_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class DIALOG_SHIM;

#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_EDITOR_DATA_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_EDITOR_DATA_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxStaticText* m_staticTextName;
		wxTextCtrl* m_textCtrlName;
		wxStaticText* m_staticTextVref;
		wxTextCtrl* m_textCtrlVref;
		wxStaticText* m_staticTextVrefUnit;
		wxStaticText* m_staticTextType;
		wxChoice* m_choiceRegType;
		wxStaticText* m_RegulIadjTitle;
		wxTextCtrl* m_RegulIadjValue;
		wxStaticText* m_IadjUnitLabel;
		wxStaticLine* m_staticline2;
		wxStdDialogButtonSizer* m_sdbSizerButtons;
		wxButton* m_sdbSizerButtonsOK;
		wxButton* m_sdbSizerButtonsCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnRegTypeSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOKClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_EDITOR_DATA_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Regulator Parameters"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 292,200 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~DIALOG_EDITOR_DATA_BASE();
	
};

#endif //__DIALOG_REGULATOR_DATA_BASE_H__
