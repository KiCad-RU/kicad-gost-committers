/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_pcbnew_config_libs_and_paths.h
// Author:      jean-pierre Charras
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef _DIALOG_PCBNEW_CONFIG_LIBS_H_
#define _DIALOG_PCBNEW_CONFIG_LIBS_H_

#include "dialog_pcbnew_config_libs_and_paths_fbp.h"

class DIALOG_PCBNEW_CONFIG_LIBS : public DIALOG_PCBNEW_CONFIG_LIBS_FBP
{
private:
    wxConfig* m_Config;
    bool m_LibListChanged;
    bool m_LibPathChanged;
    wxString m_UserLibDirBufferImg;         // Copy of original g_UserLibDirBuffer

private:

    // event handlers, overiding the fbp handlers
    void Init();
    void OnCloseWindow( wxCloseEvent& event );
    void OnSaveCfgClick( wxCommandEvent& event );
    void OnRemoveLibClick( wxCommandEvent& event );
    void OnAddOrInsertLibClick( wxCommandEvent& event );
    void OnAddOrInsertPath( wxCommandEvent& event );
	void OnOkClick( wxCommandEvent& event );
	void OnCancelClick( wxCommandEvent& event );
    void OnRemoveUserPath( wxCommandEvent& event );
	void OnBrowseModDocFile( wxCommandEvent& event );
	void OnButtonUpClick( wxCommandEvent& event );
	void OnButtonDownClick( wxCommandEvent& event );


public:
    DIALOG_PCBNEW_CONFIG_LIBS( PCB_EDIT_FRAME * parent );
    ~DIALOG_PCBNEW_CONFIG_LIBS() {};

    PCB_EDIT_FRAME* GetParent() { return (PCB_EDIT_FRAME*) wxDialog::GetParent(); }
};

#endif  // _DIALOG_PCBNEW_CONFIG_LIBS_H_
