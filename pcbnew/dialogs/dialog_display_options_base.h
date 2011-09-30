///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  6 2011)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __dialog_display_options_base__
#define __dialog_display_options_base__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/radiobox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_DISPLAY_OPTIONS_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_DISPLAY_OPTIONS_BASE : public wxDialog 
{
	private:
	
	protected:
		enum
		{
			wxID_DISPLAY_TRACK = 1000,
			ID_VIAS_SHAPES,
			ID_VIAS_HOLES,
			ID_SHOW_CLEARANCE,
			ID_EDGES_MODULES,
			ID_TEXT_MODULES,
			ID_PADS_SHAPES,
		};
		
		wxRadioBox* m_OptDisplayTracks;
		wxRadioBox* m_OptDisplayVias;
		wxRadioBox* m_OptDisplayViaHole;
		wxRadioBox* m_ShowNetNamesOption;
		wxRadioBox* m_OptDisplayTracksClearance;
		wxRadioBox* m_OptDisplayModEdges;
		wxRadioBox* m_OptDisplayModTexts;
		wxRadioBox* m_OptDisplayPads;
		wxCheckBox* m_OptDisplayPadClearence;
		wxCheckBox* m_OptDisplayPadNumber;
		wxCheckBox* m_OptDisplayPadNoConn;
		wxRadioBox* m_OptDisplayDrawings;
		wxRadioBox* m_Show_Page_Limits;
		
		wxButton* m_buttonOK;
		wxButton* m_buttonCANCEL;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnOkClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancelClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_DISPLAY_OPTIONS_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Display options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 731,291 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~DIALOG_DISPLAY_OPTIONS_BASE();
	
};

#endif //__dialog_display_options_base__
