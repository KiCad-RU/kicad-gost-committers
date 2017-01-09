///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Dec  4 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_FOOTPRINT_WIZARD_LIST_BASE_H__
#define __DIALOG_FOOTPRINT_WIZARD_LIST_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class DIALOG_SHIM;

#include "dialog_shim.h"
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/font.h>
#include <wx/grid.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/notebook.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_FOOTPRINT_WIZARD_LIST_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_FOOTPRINT_WIZARD_LIST_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxNotebook* m_notebook;
		wxPanel* m_panelGenerators;
		wxGrid* m_footprintGeneratorsGrid;
		wxPanel* m_panelInfo;
		wxStaticText* m_staticText1;
		wxTextCtrl* m_tcSearchPaths;
		wxStaticText* m_staticText11;
		wxTextCtrl* m_tcNotLoaded;
		wxButton* m_buttonShowTrace;
		wxStdDialogButtonSizer* m_sdbSizer;
		wxButton* m_sdbSizerOK;
		wxButton* m_sdbSizerCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnCellFpGeneratorClick( wxGridEvent& event ) { event.Skip(); }
		virtual void OnCellFpGeneratorDoubleClick( wxGridEvent& event ) { event.Skip(); }
		virtual void onShowTrace( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_FOOTPRINT_WIZARD_LIST_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Footprint Generators"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 572,467 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~DIALOG_FOOTPRINT_WIZARD_LIST_BASE();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_FOOTPRINT_WIZARD_LOG
///////////////////////////////////////////////////////////////////////////////
class DIALOG_FOOTPRINT_WIZARD_LOG : public DIALOG_SHIM
{
	private:
	
	protected:
		wxStdDialogButtonSizer* m_sdbSizer;
		wxButton* m_sdbSizerOK;
	
	public:
		wxTextCtrl* m_Message;
		
		DIALOG_FOOTPRINT_WIZARD_LOG( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Traceback of errors in not loadable python scripts"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 646,361 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~DIALOG_FOOTPRINT_WIZARD_LOG();
	
};

#endif //__DIALOG_FOOTPRINT_WIZARD_LIST_BASE_H__
