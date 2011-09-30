
/* Set up color Layers for Eeschema
 */

#include "fctsys.h"
#include "gr_basic.h"
#include "wxstruct.h"
#include "class_drawpanel.h"

#include "general.h"
#include "protos.h"

#include "dialog_color_config.h"


#define ID_COLOR_SETUP  1800


static ColorButton GeneralColorButtons[] = {
    { _( "Wire" ), LAYER_WIRE },
    { _( "Bus" ), LAYER_BUS },
    { _( "Junction" ), LAYER_JUNCTION },
    { _( "Label" ), LAYER_LOCLABEL },
    { _( "Global label" ), LAYER_GLOBLABEL },
    { _( "Net name" ), LAYER_NETNAM },
    { _( "Notes" ), LAYER_NOTES },
    { _( "No Connect Symbol" ), LAYER_NOCONNECT },
    { wxT( "" ), -1 }                           // Sentinel marking end of list.
};

static ColorButton ComponentColorButtons[] = {
    { _( "Body" ), LAYER_DEVICE },
    { _( "Body background" ), LAYER_DEVICE_BACKGROUND },
    { _( "Pin" ), LAYER_PIN },
    { _( "Pin number" ), LAYER_PINNUM },
    { _( "Pin name" ), LAYER_PINNAM },
    { _( "Reference" ), LAYER_REFERENCEPART },
    { _( "Value" ), LAYER_VALUEPART },
    { _( "Fields" ), LAYER_FIELDS },
    { wxT( "" ), -1 }                           // Sentinel marking end of list.
};

static ColorButton SheetColorButtons[] = {
    { _( "Sheet" ), LAYER_SHEET },
    { _( "Sheet file name" ), LAYER_SHEETFILENAME },
    { _( "Sheet name" ), LAYER_SHEETNAME },
    { _( "Sheet label" ), LAYER_SHEETLABEL },
    { _( "Hierarchical label" ), LAYER_HIERLABEL },
    { wxT( "" ), -1 }                           // Sentinel marking end of list.
};

static ColorButton MiscColorButtons[] = {
    { _( "Erc warning" ), LAYER_ERC_WARN },
    { _( "Erc error" ), LAYER_ERC_ERR },
    { _( "Grid" ), LAYER_GRID },
    { wxT( "" ), -1 }                           // Sentinel marking end of list.
};


static ButtonIndex buttonGroups[] = {
    { _( "General" ), GeneralColorButtons },
    { _( "Component" ), ComponentColorButtons },
    { _( "Sheet" ), SheetColorButtons },
    { _( "Miscellaneous" ), MiscColorButtons },
    { wxT( "" ), NULL }
};


static int currentColors[ MAX_LAYER ];


IMPLEMENT_DYNAMIC_CLASS( DIALOG_COLOR_CONFIG, wxDialog )


DIALOG_COLOR_CONFIG::DIALOG_COLOR_CONFIG()
{
    Init();
}


DIALOG_COLOR_CONFIG::DIALOG_COLOR_CONFIG( EDA_DRAW_FRAME* aParent )
{
    m_Parent = aParent;
    Init();
    Create( aParent );
}


DIALOG_COLOR_CONFIG::~DIALOG_COLOR_CONFIG()
{
}


bool DIALOG_COLOR_CONFIG::Create( wxWindow*       aParent,
                                  wxWindowID      aId,
                                  const wxString& aCaption,
                                  const wxPoint&  aPosition,
                                  const wxSize&   aSize,
                                  long            aStyle )
{
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( aParent, aId, aCaption, aPosition, aSize, aStyle );

    CreateControls();

    if( GetSizer() )
    {
        GetSizer()->SetSizeHints( this );
    }

    return true;
}


void DIALOG_COLOR_CONFIG::Init()
{
    OuterBoxSizer  = NULL;
    MainBoxSizer   = NULL;
    ColumnBoxSizer = NULL;
    RowBoxSizer    = NULL;
    BitmapButton = NULL;
    m_SelBgColor = NULL;
    Line = NULL;
    StdDialogButtonSizer = NULL;
    Button = NULL;
}


void DIALOG_COLOR_CONFIG::CreateControls()
{
    wxStaticText* label;
    int color;
    int buttonId = 1800;
    ButtonIndex* groups = buttonGroups;

    OuterBoxSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( OuterBoxSizer );

    MainBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    OuterBoxSizer->Add( MainBoxSizer, 1, wxGROW | wxLEFT | wxRIGHT, 5 );

    while( groups->m_Buttons != NULL )
    {
        ColorButton* buttons = groups->m_Buttons;

        ColumnBoxSizer = new wxBoxSizer( wxVERTICAL );
        MainBoxSizer->Add( ColumnBoxSizer, 1, wxALIGN_TOP | wxLEFT | wxTOP, 5 );
        RowBoxSizer = new wxBoxSizer( wxHORIZONTAL );
        ColumnBoxSizer->Add( RowBoxSizer, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

        // Add a text string to identify the column of color select buttons.
        label = new wxStaticText( this, wxID_ANY, groups->m_Name );

        // Make the column label font bold.
        wxFont font( label->GetFont() );
        font.SetWeight( wxFONTWEIGHT_BOLD );
        label->SetFont( font );

        RowBoxSizer->Add( label, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

        while( buttons->m_Layer >= 0 )
        {
            RowBoxSizer = new wxBoxSizer( wxHORIZONTAL );
            ColumnBoxSizer->Add( RowBoxSizer, 0, wxGROW | wxALL, 0 );

            wxMemoryDC iconDC;
            wxBitmap   bitmap( BUTT_SIZE_X, BUTT_SIZE_Y );

            iconDC.SelectObject( bitmap );
            color = currentColors[ buttons->m_Layer ] = g_LayerDescr.LayerColor[ buttons->m_Layer ];
            iconDC.SetPen( *wxBLACK_PEN );
            wxBrush brush;
            brush.SetColour( ColorRefs[ color ].m_Red,
                             ColorRefs[ color ].m_Green,
                             ColorRefs[ color ].m_Blue );
            brush.SetStyle( wxSOLID );

            iconDC.SetBrush( brush );
            iconDC.DrawRectangle( 0, 0, BUTT_SIZE_X, BUTT_SIZE_Y );

            BitmapButton = new wxBitmapButton( this, buttonId, bitmap, wxDefaultPosition,
                                               wxSize( BUTT_SIZE_X+8, BUTT_SIZE_Y+6 ) );
            BitmapButton->SetClientData( (void*) buttons );
            RowBoxSizer->Add( BitmapButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5 );

            label = new wxStaticText( this, wxID_ANY, wxGetTranslation( buttons->m_Name ) );
            RowBoxSizer->Add( label, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5 );
            buttonId += 1;
            buttons++;
        }

        groups++;
    }

    Connect( 1800, buttonId - 1, wxEVT_COMMAND_BUTTON_CLICKED,
             wxCommandEventHandler( DIALOG_COLOR_CONFIG::SetColor ) );

    // Add a spacer to improve appearance.
    ColumnBoxSizer->AddSpacer( 5 );

    wxArrayString m_SelBgColorStrings;
    m_SelBgColorStrings.Add( _( "White" ) );
    m_SelBgColorStrings.Add( _( "Black" ) );
    m_SelBgColor = new wxRadioBox( this, wxID_ANY, _( "Background Color" ),
                                   wxDefaultPosition, wxDefaultSize,
                                   m_SelBgColorStrings, 1, wxRA_SPECIFY_COLS );
    m_SelBgColor->SetSelection( ( g_DrawBgColor == BLACK ) ? 1 : 0 );
    ColumnBoxSizer->Add( m_SelBgColor, 1, wxGROW | wxRIGHT | wxTOP | wxBOTTOM, 5 );

    // Provide a line to separate all of the controls added so far from the
    // "OK", "Cancel", and "Apply" buttons (which will be added after that
    // line).
    Line = new wxStaticLine( this, -1, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    OuterBoxSizer->Add( Line, 0, wxGROW | wxALL, 5 );

    // Provide a StdDialogButtonSizer to accommodate the OK, Cancel, and Apply
    // buttons; using that type of sizer results in those buttons being
    // automatically located in positions appropriate for each (OS) version of
    // KiCad.
    StdDialogButtonSizer = new wxStdDialogButtonSizer;
    OuterBoxSizer->Add( StdDialogButtonSizer, 0, wxGROW | wxALL, 10 );

    Button = new wxButton( this, wxID_OK, _( "OK" ), wxDefaultPosition, wxDefaultSize, 0 );
    StdDialogButtonSizer->AddButton( Button );

    Button = new wxButton( this, wxID_CANCEL, _( "Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );
    StdDialogButtonSizer->AddButton( Button );
    Button->SetFocus();

    Button = new wxButton( this, wxID_APPLY, _( "Apply" ), wxDefaultPosition, wxDefaultSize, 0 );
    StdDialogButtonSizer->AddButton( Button );

    Connect( wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED,
             wxCommandEventHandler( DIALOG_COLOR_CONFIG::OnOkClick ) );
    Connect( wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
             wxCommandEventHandler( DIALOG_COLOR_CONFIG::OnCancelClick ) );
    Connect( wxID_APPLY, wxEVT_COMMAND_BUTTON_CLICKED,
             wxCommandEventHandler( DIALOG_COLOR_CONFIG::OnApplyClick ) );

    StdDialogButtonSizer->Realize();

    // Dialog now needs to be resized, but the associated command is found elsewhere.
}


void DIALOG_COLOR_CONFIG::SetColor( wxCommandEvent& event )
{
    wxBitmapButton* button = (wxBitmapButton*) event.GetEventObject();

    wxCHECK_RET( button != NULL, wxT( "Color button event object is NULL." ) );

    ColorButton* colorButton = (ColorButton*) button->GetClientData();

    wxCHECK_RET( colorButton != NULL, wxT( "Client data not set for color button." ) );

    int color = DisplayColorFrame( this, colorButton->m_Layer );

    if( color < 0 || currentColors[ colorButton->m_Layer ] == color )
        return;

    currentColors[ colorButton->m_Layer ] = color;

    wxMemoryDC iconDC;

    wxBitmap bitmap = button->GetBitmapLabel();
    iconDC.SelectObject( bitmap );
    wxBrush  brush;
    iconDC.SetPen( *wxBLACK_PEN );
    brush.SetColour( ColorRefs[ color ].m_Red,
                     ColorRefs[ color ].m_Green,
                     ColorRefs[ color ].m_Blue );
    brush.SetStyle( wxSOLID );

    iconDC.SetBrush( brush );
    iconDC.DrawRectangle( 0, 0, BUTT_SIZE_X, BUTT_SIZE_Y );
    button->SetBitmapLabel( bitmap );
    button->Refresh();

    Refresh( false );
}


bool DIALOG_COLOR_CONFIG::UpdateColorsSettings()
{
    // Update color of background
    if( m_SelBgColor->GetSelection() == 0 )
        g_DrawBgColor = WHITE;
    else
        g_DrawBgColor = BLACK;

    bool warning = false;
    for( int ii = 0;  ii < MAX_LAYERS;  ii++ )
    {
        g_LayerDescr.LayerColor[ ii ] = currentColors[ ii ];
        if( g_DrawBgColor == g_LayerDescr.LayerColor[ ii ] )
            warning = true;
    }

    m_Parent->SetGridColor( g_LayerDescr.LayerColor[LAYER_GRID] );
    if( g_DrawBgColor == g_LayerDescr.LayerColor[ LAYER_GRID ] )
        warning = true;

    return warning;
}


void DIALOG_COLOR_CONFIG::OnOkClick( wxCommandEvent& event )
{
    bool warning = UpdateColorsSettings();

    // Prompt the user if an item has the same color as the background
    // because this item cannot be seen:
    if( warning )
        wxMessageBox(
    _("Warning:\nSome items have the same color as the background\nand they will not be seen on screen")
                    );

    m_Parent->DrawPanel->Refresh();

    EndModal( 1 );
}


void DIALOG_COLOR_CONFIG::OnCancelClick( wxCommandEvent& event )
{
    EndModal( -1 );
}


void DIALOG_COLOR_CONFIG::OnApplyClick( wxCommandEvent& event )
{
    UpdateColorsSettings();
    m_Parent->DrawPanel->Refresh();
}


void SeedLayers()
{
    LayerStruct* LayerPointer = &g_LayerDescr;
    int          pt;

    LayerPointer->CommonColor = WHITE;
    LayerPointer->Flags = 0;
    pt = 0;
    LayerPointer->CurrentWidth = 1;

    /* seed Up the Layer colours, set all user layers off */
    for( pt = 0; pt < MAX_LAYERS; pt++ )
    {
        LayerPointer->LayerStatus[pt] = 0;
        LayerPointer->LayerColor[pt]  = DARKGRAY;
    }

    LayerPointer->NumberOfLayers = pt - 1;
    /* Specific colors: update by reading the config. */
}


EDA_Colors ReturnLayerColor( int Layer )
{
    if( g_LayerDescr.Flags == 0 )
        return (EDA_Colors) g_LayerDescr.LayerColor[Layer];
    else
        return (EDA_Colors) g_LayerDescr.CommonColor;
}
