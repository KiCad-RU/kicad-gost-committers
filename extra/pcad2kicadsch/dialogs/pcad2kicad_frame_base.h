///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __pcad2kicad_frame_base__
#define __pcad2kicad_frame_base__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statusbr.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class PCAD2KICAD_FRAME_BASE
///////////////////////////////////////////////////////////////////////////////
class PCAD2KICAD_FRAME_BASE : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* m_menubar;
		wxStatusBar* m_statusBar;
		wxPanel* m_panel4;
		wxButton* m_sch;
		wxButton* m_lib;
		wxStaticText* m_staticText1;
		wxStaticText* m_inputFileName;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClosePcbCalc( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSch( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLib( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		PCAD2KICAD_FRAME_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("PCad to Kicad converter"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 670,489 ), long style = wxDEFAULT_FRAME_STYLE|wxRESIZE_BORDER|wxFULL_REPAINT_ON_RESIZE|wxTAB_TRAVERSAL );
		~PCAD2KICAD_FRAME_BASE();
	
};

#endif //__pcad2kicad_frame_base__
