
/* File: dialog_print_for_modedit.cpp */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <pcbnew.h>
#include <wxPcbStruct.h>
#include <module_editor_frame.h>
#include <pcbplot.h>

#include <dialog_print_for_modedit_base.h>
#include <printout_controler.h>

static double s_scaleList[] =
{ 0, 0.5, 0.7, 1.0, 1.4, 2.0, 3.0, 4.0, 8.0, 16.0 };


// static print data and page setup data, to remember settings during the session
static PRINT_PARAMETERS  s_Parameters;
static wxPrintData* s_PrintData;
static wxPageSetupDialogData* s_pageSetupData = (wxPageSetupDialogData*) NULL;


/**
 * Class DIALOG_PRINT_FOR_MODEDIT
 * is derived from DIALOG_PRINT_FOR_MODEDIT_BASE which is created by wxFormBuilder.
 */
class DIALOG_PRINT_FOR_MODEDIT : public DIALOG_PRINT_FOR_MODEDIT_BASE
{
public:
    DIALOG_PRINT_FOR_MODEDIT( PCB_BASE_FRAME* parent );

private:
    PCB_BASE_FRAME* m_parent;
    wxConfig*       m_config;

    void OnCloseWindow( wxCloseEvent& event );

    /// Open a dialog box for printer setup (printer options, page size ...)
    void OnPageSetup( wxCommandEvent& event );

    void OnPrintPreview( wxCommandEvent& event );

    /// Called on activate Print button
    void OnPrintButtonClick( wxCommandEvent& event );

    void OnButtonCancelClick( wxCommandEvent& event ) { Close(); }
    void InitValues( );
};


void FOOTPRINT_EDIT_FRAME::ToPrinter( wxCommandEvent& event )
{
    const PAGE_INFO& pageInfo = GetPageSettings();

    if( s_PrintData == NULL )  // First print
    {
        s_PrintData = new wxPrintData();

        if( !s_PrintData->Ok() )
        {
            DisplayError( this, _( "Error Init Printer info" ) );
        }
        s_PrintData->SetQuality( wxPRINT_QUALITY_HIGH );      // Default resolution = HIGHT;
    }

    if( s_pageSetupData == NULL )
        s_pageSetupData = new wxPageSetupDialogData( *s_PrintData );

    s_pageSetupData->SetPaperId( pageInfo.GetPaperId() );
    s_pageSetupData->GetPrintData().SetOrientation( pageInfo.GetWxOrientation() );
    s_PrintData->SetOrientation( pageInfo.GetWxOrientation() );

    *s_PrintData = s_pageSetupData->GetPrintData();
    s_Parameters.m_PageSetupData = s_pageSetupData;

    DIALOG_PRINT_FOR_MODEDIT dlg( this );

    dlg.ShowModal();
}


DIALOG_PRINT_FOR_MODEDIT::DIALOG_PRINT_FOR_MODEDIT( PCB_BASE_FRAME* parent ) :
    DIALOG_PRINT_FOR_MODEDIT_BASE( parent )
{
    m_parent = parent;
    s_Parameters.m_ForceCentered = true;
    m_config = wxGetApp().GetSettings();
    InitValues();

    m_buttonPrint->SetDefault();
    GetSizer()->SetSizeHints( this );
}


void DIALOG_PRINT_FOR_MODEDIT::InitValues( )
{
    // Read the scale adjust option
    int scale_Select = 3; // default selected scale = ScaleList[3] = 1
    if( m_config )
    {
        m_config->Read( OPTKEY_PRINT_MODULE_SCALE, &scale_Select );
        m_config->Read( OPTKEY_PRINT_MONOCHROME_MODE, &s_Parameters.m_Print_Black_and_White, 1);
    }

    s_Parameters.m_PenDefaultSize = g_DrawDefaultLineThickness;
    m_ScaleOption->SetSelection( scale_Select );

    if( s_Parameters.m_Print_Black_and_White )
        m_ModeColorOption->SetSelection( 1 );
}



void DIALOG_PRINT_FOR_MODEDIT::OnCloseWindow( wxCloseEvent& event )
{
    if( m_config )
    {
        m_config->Write( OPTKEY_PRINT_MODULE_SCALE, m_ScaleOption->GetSelection() );
        m_config->Write( OPTKEY_PRINT_MONOCHROME_MODE, s_Parameters.m_Print_Black_and_White);
    }
    EndModal( 0 );
}


void DIALOG_PRINT_FOR_MODEDIT::OnPageSetup( wxCommandEvent& event )
{
    wxPageSetupDialog pageSetupDialog( this, s_pageSetupData );
    pageSetupDialog.ShowModal();

    (*s_PrintData) = pageSetupDialog.GetPageSetupDialogData().GetPrintData();
    (*s_pageSetupData) = pageSetupDialog.GetPageSetupDialogData();
}



void DIALOG_PRINT_FOR_MODEDIT::OnPrintPreview( wxCommandEvent& event )


/* Open and display a previewer frame for printing
 */
{
    s_Parameters.m_Print_Black_and_White = m_ModeColorOption->GetSelection();
    s_Parameters.m_PrintScale = s_scaleList[m_ScaleOption->GetSelection()];

    // Pass two printout objects: for preview, and possible printing.
    wxString        title   = _( "Print Preview" );
    wxPrintPreview* preview =
        new wxPrintPreview( new BOARD_PRINTOUT_CONTROLLER( s_Parameters, m_parent, title ),
                            new BOARD_PRINTOUT_CONTROLLER( s_Parameters, m_parent, title ),
                            s_PrintData );

    if( preview == NULL )
    {
        DisplayError( this, wxT( "OnPrintPreview() problem" ) );
        return;
    }

     // Uses the parent position and size.
    // @todo uses last position and size ans store them when exit in m_config
    wxPoint         WPos  = m_parent->GetPosition();
    wxSize          WSize = m_parent->GetSize();

    wxPreviewFrame* frame = new wxPreviewFrame( preview, this, title, WPos, WSize );

    frame->Initialize();
    frame->Show( true );
}


void DIALOG_PRINT_FOR_MODEDIT::OnPrintButtonClick( wxCommandEvent& event )
{
    PCB_PLOT_PARAMS plot_opts = m_parent->GetPlotSettings();

    s_Parameters.m_Print_Black_and_White = m_ModeColorOption->GetSelection();
    s_Parameters.m_PrintScale = s_scaleList[m_ScaleOption->GetSelection()];

    plot_opts.SetFineScaleAdjustX( s_Parameters.m_XScaleAdjust );
    plot_opts.SetFineScaleAdjustY( s_Parameters.m_YScaleAdjust );
    plot_opts.SetScale( s_Parameters.m_PrintScale );

    m_parent->SetPlotSettings( plot_opts );

    wxPrintDialogData printDialogData( *s_PrintData );
    wxPrinter         printer( &printDialogData );

    BOARD_PRINTOUT_CONTROLLER      printout( s_Parameters, m_parent, _( "Print Footprint" ) );

#if !defined(__WINDOWS__) && !wxCHECK_VERSION(2,9,0)
    wxDC*             dc = printout.GetDC();
    ( (wxPostScriptDC*) dc )->SetResolution( 600 );  // Postscript DC resolution is 600 ppi
#endif

    if( !printer.Print( this, &printout, true ) )
    {
        if( wxPrinter::GetLastError() == wxPRINTER_ERROR )
            DisplayError( this, _( "There was a problem printing" ) );
        return;
    }
    else
    {
        *s_PrintData = printer.GetPrintDialogData().GetPrintData();
    }


}
