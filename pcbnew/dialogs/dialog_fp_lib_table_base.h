///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 30 2013)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_FP_LIB_TABLE_BASE_H__
#define __DIALOG_FP_LIB_TABLE_BASE_H__

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
#include <wx/aui/auibook.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_FP_LIB_TABLE_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_FP_LIB_TABLE_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxAuiNotebook* m_auinotebook;
		wxPanel* m_global_panel;
		wxGrid* m_global_grid;
		wxPanel* m_project_panel;
		wxGrid* m_project_grid;
		wxButton* m_append_button;
		wxButton* m_delete_button;
		wxButton* m_move_up_button;
		wxButton* m_move_down_button;
		wxButton* m_edit_options;
		wxGrid* m_path_subs_grid;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onKeyDown( wxKeyEvent& event ) { event.Skip(); }
		virtual void pageChangedHandler( wxAuiNotebookEvent& event ) { event.Skip(); }
		virtual void onGridCellLeftClick( wxGridEvent& event ) { event.Skip(); }
		virtual void onGridCellLeftDClick( wxGridEvent& event ) { event.Skip(); }
		virtual void onGridCellRightClick( wxGridEvent& event ) { event.Skip(); }
		virtual void onGridCmdSelectCell( wxGridEvent& event ) { event.Skip(); }
		virtual void appendRowHandler( wxMouseEvent& event ) { event.Skip(); }
		virtual void deleteRowHandler( wxMouseEvent& event ) { event.Skip(); }
		virtual void moveUpHandler( wxMouseEvent& event ) { event.Skip(); }
		virtual void moveDownHandler( wxMouseEvent& event ) { event.Skip(); }
		virtual void optionsEditor( wxMouseEvent& event ) { event.Skip(); }
		virtual void onCancelButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void onOKButtonClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_FP_LIB_TABLE_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("PCB Library Tables"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 900,600 ), long style = wxCAPTION|wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~DIALOG_FP_LIB_TABLE_BASE();
	
};

#endif //__DIALOG_FP_LIB_TABLE_BASE_H__
