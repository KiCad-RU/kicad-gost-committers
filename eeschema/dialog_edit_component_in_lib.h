/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_edit_component_in_lib.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     02/03/2006 08:51:09
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 02/03/2006 08:51:09

#ifndef _DIALOG_EDIT_COMPONENT_IN_LIB_H_
#define _DIALOG_EDIT_COMPONENT_IN_LIB_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/notebook.h"
#include "wx/spinctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxNotebook;
class wxSpinCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_WINEDA_PARTPROPERTIESFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_WINEDA_PARTPROPERTIESFRAME_TITLE _("Lib Component Properties")
#define SYMBOL_WINEDA_PARTPROPERTIESFRAME_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_PARTPROPERTIESFRAME_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_PARTPROPERTIESFRAME_POSITION wxDefaultPosition
#define ID_LIBEDIT_NOTEBOOK 10001
#define ID_PANEL_BASIC 10002
#define ID_SPINCTRL1 10010
#define ID_SPINCTRL 10009
#define ID_CHECKBOX 10007
#define ID_CHECKBOX1 10008
#define ID_PANEL_DOC 10004
#define ID_TEXTCTRL 10011
#define ID_TEXTCTRL1 10012
#define ID_TEXTCTRL2 10013
#define ID_COPY_DOC_TO_ALIAS 10014
#define ID_BROWSE_DOC_FILES 10005
#define ID_PANEL_ALIAS 10003
////@end control identifiers
#define ID_DELETE_ONE_FOOTPRINT_FILTER 10020
#define ID_DELETE_ALL_FOOTPRINT_FILTER 10021
#define ID_ADD_FOOTPRINT_FILTER 10022

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * WinEDA_PartPropertiesFrame class declaration
 */

class WinEDA_PartPropertiesFrame: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( WinEDA_PartPropertiesFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_PartPropertiesFrame( );
    WinEDA_PartPropertiesFrame( WinEDA_LibeditFrame* parent, wxWindowID id = SYMBOL_WINEDA_PARTPROPERTIESFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_PARTPROPERTIESFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_PARTPROPERTIESFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_PARTPROPERTIESFRAME_SIZE, long style = SYMBOL_WINEDA_PARTPROPERTIESFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_PARTPROPERTIESFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_PARTPROPERTIESFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_PARTPROPERTIESFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_PARTPROPERTIESFRAME_SIZE, long style = SYMBOL_WINEDA_PARTPROPERTIESFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_PartPropertiesFrame event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_COPY_DOC_TO_ALIAS
    void OnCopyDocToAliasClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BROWSE_DOC_FILES
    void OnBrowseDocFilesClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

////@end WinEDA_PartPropertiesFrame event handler declarations

////@begin WinEDA_PartPropertiesFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_PartPropertiesFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

	void InitBuffers(void);
	void BuildPanelBasic(void);
	void BuildPanelDoc(void);
	void BuildPanelAlias(void);
	void BuildPanelEditField();
	void PartPropertiesAccept(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void DeleteAllAliasOfPart(wxCommandEvent& event);
	void DeleteAliasOfPart(wxCommandEvent& event);
	void AddAliasOfPart(wxCommandEvent& event);
	bool ChangeNbUnitsPerPackage(int newUnit);
	bool SetUnsetConvert(void);
	void CopyDocToAlias(wxCommandEvent& event);
	void BrowseAndSelectDocFile(wxCommandEvent& event);
	void SelectNewField(wxCommandEvent& event);
	void CopyFieldDataToBuffer(LibDrawField * Field);
	void CopyBufferToFieldData(LibDrawField * Field);
	void CopyDataToPanelField(void);
	void CopyPanelFieldToData(void);

	void BuildPanelFootprintFilter(void);
	void DeleteAllFootprintFilter(wxCommandEvent& event);
	void DeleteOneFootprintFilter(wxCommandEvent& event);
	void AddFootprintFilter(wxCommandEvent& event);

////@begin WinEDA_PartPropertiesFrame member variables
    wxBoxSizer* m_GeneralBoxSizer;
    wxNotebook* m_NoteBook;
    wxPanel* m_PanelBasic;
    wxBoxSizer* m_PanelBasicBoxSizer;
    wxStaticBoxSizer* m_OptionsBoxSizer;
    wxSpinCtrl* SelNumberOfUnits;
    wxSpinCtrl* m_SetSkew;
    wxCheckBox* m_OptionPower;
    wxCheckBox* m_OptionPartsLocked;
    wxPanel* m_PanelDoc;
    wxBoxSizer* m_PanelDocBoxSizer;
    wxTextCtrl* m_Doc;
    wxTextCtrl* m_Keywords;
    wxTextCtrl* m_Docfile;
    wxButton* m_ButtonCopyDoc;
    wxButton* m_ButtonDeleteAllAlias;
    wxButton* m_ButtonDeleteOneAlias;
    wxPanel* m_PanelAlias;
////@end WinEDA_PartPropertiesFrame member variables

	WinEDA_LibeditFrame * m_Parent;
	int m_CurrentFieldId;
	wxString m_Title;

	wxPanel * m_PanelField;
	wxPanel * m_PanelFootprintFilter;
	wxButton * m_ButtonDeleteAllFootprintFilter;
	wxButton * m_ButtonDeleteOneFootprintFilter;

	wxCheckBox * AsConvertButt;
	wxCheckBox * ShowPinNumButt;
	wxCheckBox * ShowPinNameButt;
	wxCheckBox * m_PinsNameInsideButt;

	wxListBox * m_PartAliasList;
	wxListBox * m_FootprintFilterListBox;
	
	wxRadioBox * m_FieldSelection;
	wxCheckBox * m_ShowFieldTextCtrl;
	wxCheckBox * m_VorientFieldTextCtrl;
	wxRadioBox * m_FieldHJustifyCtrl;
	wxRadioBox * m_FieldVJustifyCtrl;
	WinEDA_GraphicTextCtrl * m_FieldTextCtrl;
	WinEDA_EnterText * m_FieldNameCtrl;
	WinEDA_PositionCtrl * m_FieldPositionCtrl;
	int m_FieldFlags[NUMBER_OF_FIELDS];
	int m_FieldOrient[NUMBER_OF_FIELDS];
	int m_FieldHJustify[NUMBER_OF_FIELDS];
	int m_FieldVJustify[NUMBER_OF_FIELDS];
	int m_FieldSize[NUMBER_OF_FIELDS];
	wxString m_FieldText[NUMBER_OF_FIELDS];
	wxString m_FieldName[NUMBER_OF_FIELDS];
	wxPoint m_FieldPosition[NUMBER_OF_FIELDS];
	
	bool m_RecreateToolbar;
	int m_AliasLocation;

};

#endif
    // _DIALOG_EDIT_COMPONENT_IN_LIB_H_
