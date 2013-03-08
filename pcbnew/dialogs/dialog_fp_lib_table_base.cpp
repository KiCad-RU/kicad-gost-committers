///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_fp_lib_table_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_FP_LIB_TABLE_BASE::DIALOG_FP_LIB_TABLE_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_splitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH );
	m_splitter->Connect( wxEVT_IDLE, wxIdleEventHandler( DIALOG_FP_LIB_TABLE_BASE::m_splitterOnIdle ), NULL, this );
	m_splitter->SetMinimumPaneSize( 10 );
	
	m_top = new wxPanel( m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* m_top_sizer;
	m_top_sizer = new wxStaticBoxSizer( new wxStaticBox( m_top, wxID_ANY, _("Library Tables by Scope") ), wxVERTICAL );
	
	m_auinotebook = new wxAuiNotebook( m_top, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_BOTTOM );
	m_global_panel = new wxPanel( m_auinotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_global_panel->SetToolTip( _("Module libraries which  are visible for all projects") );
	
	wxBoxSizer* m_global_sizer;
	m_global_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_global_grid = new wxGrid( m_global_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	m_global_grid->CreateGrid( 1, 5 );
	m_global_grid->EnableEditing( true );
	m_global_grid->EnableGridLines( true );
	m_global_grid->EnableDragGridSize( true );
	m_global_grid->SetMargins( 0, 0 );
	
	// Columns
	m_global_grid->AutoSizeColumns();
	m_global_grid->EnableDragColMove( false );
	m_global_grid->EnableDragColSize( true );
	m_global_grid->SetColLabelSize( 30 );
	m_global_grid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_global_grid->EnableDragRowSize( true );
	m_global_grid->SetRowLabelSize( 40 );
	m_global_grid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_global_grid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	m_global_sizer->Add( m_global_grid, 1, wxALL|wxEXPAND, 5 );
	
	
	m_global_panel->SetSizer( m_global_sizer );
	m_global_panel->Layout();
	m_global_sizer->Fit( m_global_panel );
	m_auinotebook->AddPage( m_global_panel, _("Global Libraries"), true, wxNullBitmap );
	m_project_panel = new wxPanel( m_auinotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_project_panel->SetToolTip( _("Module libraries which  are visible for curent project only") );
	
	wxBoxSizer* m_project_sizer;
	m_project_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_project_grid = new wxGrid( m_project_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	m_project_grid->CreateGrid( 1, 5 );
	m_project_grid->EnableEditing( true );
	m_project_grid->EnableGridLines( true );
	m_project_grid->EnableDragGridSize( true );
	m_project_grid->SetMargins( 0, 0 );
	
	// Columns
	m_project_grid->AutoSizeColumns();
	m_project_grid->EnableDragColMove( false );
	m_project_grid->EnableDragColSize( true );
	m_project_grid->SetColLabelSize( 30 );
	m_project_grid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_project_grid->EnableDragRowSize( true );
	m_project_grid->SetRowLabelSize( 40 );
	m_project_grid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_project_grid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	m_project_sizer->Add( m_project_grid, 1, wxALL|wxEXPAND, 5 );
	
	
	m_project_panel->SetSizer( m_project_sizer );
	m_project_panel->Layout();
	m_project_sizer->Fit( m_project_panel );
	m_auinotebook->AddPage( m_project_panel, _("Project Specific Libraries"), false, wxNullBitmap );
	
	m_top_sizer->Add( m_auinotebook, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxHORIZONTAL );
	
	m_append_button = new wxButton( m_top, wxID_ANY, _("Append Row"), wxDefaultPosition, wxDefaultSize, 0 );
	m_append_button->SetToolTip( _("Add a pcb library row to this table") );
	
	bSizer51->Add( m_append_button, 0, wxALL, 5 );
	
	m_delete_button = new wxButton( m_top, wxID_ANY, _("Delete Row"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete_button->SetToolTip( _("Remove a PCB library from this library table") );
	
	bSizer51->Add( m_delete_button, 0, wxALL, 5 );
	
	m_move_up_button = new wxButton( m_top, wxID_ANY, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_move_up_button->SetToolTip( _("Move the currently selected row up one position") );
	
	bSizer51->Add( m_move_up_button, 0, wxALL, 5 );
	
	m_move_down_button = new wxButton( m_top, wxID_ANY, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_move_down_button->SetToolTip( _("Move the currently selected row down one position") );
	
	bSizer51->Add( m_move_down_button, 0, wxALL, 5 );
	
	
	m_top_sizer->Add( bSizer51, 0, wxALIGN_CENTER|wxBOTTOM, 8 );
	
	
	m_top->SetSizer( m_top_sizer );
	m_top->Layout();
	m_top_sizer->Fit( m_top );
	m_bottom = new wxPanel( m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* m_bottom_sizer;
	m_bottom_sizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( m_bottom, wxID_ANY, _("Path Substitutions") ), wxVERTICAL );
	
	m_path_subs_grid = new wxGrid( m_bottom, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	m_path_subs_grid->CreateGrid( 1, 2 );
	m_path_subs_grid->EnableEditing( true );
	m_path_subs_grid->EnableGridLines( true );
	m_path_subs_grid->EnableDragGridSize( false );
	m_path_subs_grid->SetMargins( 0, 0 );
	
	// Columns
	m_path_subs_grid->SetColSize( 0, 150 );
	m_path_subs_grid->SetColSize( 1, 500 );
	m_path_subs_grid->AutoSizeColumns();
	m_path_subs_grid->EnableDragColMove( false );
	m_path_subs_grid->EnableDragColSize( true );
	m_path_subs_grid->SetColLabelSize( 30 );
	m_path_subs_grid->SetColLabelValue( 0, _("Environment Variable") );
	m_path_subs_grid->SetColLabelValue( 1, _("Path Segment") );
	m_path_subs_grid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_path_subs_grid->EnableDragRowSize( true );
	m_path_subs_grid->SetRowLabelSize( 40 );
	m_path_subs_grid->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_path_subs_grid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	sbSizer1->Add( m_path_subs_grid, 1, wxALL|wxEXPAND, 5 );
	
	
	m_bottom_sizer->Add( sbSizer1, 1, wxALL|wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( m_bottom, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( m_bottom, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	m_bottom_sizer->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 5 );
	
	
	m_bottom->SetSizer( m_bottom_sizer );
	m_bottom->Layout();
	m_bottom_sizer->Fit( m_bottom );
	m_splitter->SplitHorizontally( m_top, m_bottom, 398 );
	bSizer1->Add( m_splitter, 2, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_auinotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( DIALOG_FP_LIB_TABLE_BASE::pageChangedHandler ), NULL, this );
	m_global_grid->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftClick ), NULL, this );
	m_global_grid->Connect( wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftDClick ), NULL, this );
	m_global_grid->Connect( wxEVT_GRID_CELL_RIGHT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellRightClick ), NULL, this );
	m_global_grid->Connect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCmdSelectCell ), NULL, this );
	m_project_grid->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftClick ), NULL, this );
	m_project_grid->Connect( wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftDClick ), NULL, this );
	m_project_grid->Connect( wxEVT_GRID_CELL_RIGHT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellRightClick ), NULL, this );
	m_project_grid->Connect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCmdSelectCell ), NULL, this );
	m_append_button->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::appendRowHandler ), NULL, this );
	m_delete_button->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::deleteRowHandler ), NULL, this );
	m_move_up_button->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::moveUpHandler ), NULL, this );
	m_move_down_button->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::moveDownHandler ), NULL, this );
	m_sdbSizer1Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_FP_LIB_TABLE_BASE::onCancelButtonClick ), NULL, this );
	m_sdbSizer1OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_FP_LIB_TABLE_BASE::onOKButtonClick ), NULL, this );
}

DIALOG_FP_LIB_TABLE_BASE::~DIALOG_FP_LIB_TABLE_BASE()
{
	// Disconnect Events
	m_auinotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( DIALOG_FP_LIB_TABLE_BASE::pageChangedHandler ), NULL, this );
	m_global_grid->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftClick ), NULL, this );
	m_global_grid->Disconnect( wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftDClick ), NULL, this );
	m_global_grid->Disconnect( wxEVT_GRID_CELL_RIGHT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellRightClick ), NULL, this );
	m_global_grid->Disconnect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCmdSelectCell ), NULL, this );
	m_project_grid->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftClick ), NULL, this );
	m_project_grid->Disconnect( wxEVT_GRID_CELL_LEFT_DCLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellLeftDClick ), NULL, this );
	m_project_grid->Disconnect( wxEVT_GRID_CELL_RIGHT_CLICK, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCellRightClick ), NULL, this );
	m_project_grid->Disconnect( wxEVT_GRID_SELECT_CELL, wxGridEventHandler( DIALOG_FP_LIB_TABLE_BASE::onGridCmdSelectCell ), NULL, this );
	m_append_button->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::appendRowHandler ), NULL, this );
	m_delete_button->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::deleteRowHandler ), NULL, this );
	m_move_up_button->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::moveUpHandler ), NULL, this );
	m_move_down_button->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DIALOG_FP_LIB_TABLE_BASE::moveDownHandler ), NULL, this );
	m_sdbSizer1Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_FP_LIB_TABLE_BASE::onCancelButtonClick ), NULL, this );
	m_sdbSizer1OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_FP_LIB_TABLE_BASE::onOKButtonClick ), NULL, this );
	
}
