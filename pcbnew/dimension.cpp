/**
 * @file dimension.cpp
 * @brief Dialog and code for editing a deminsion object.
 */

#include "fctsys.h"
#include "confirm.h"
#include "class_drawpanel.h"
#include "pcbnew.h"
#include "wxPcbStruct.h"
#include "class_board_design_settings.h"
#include "drawtxt.h"
#include "dialog_helpers.h"

/* Local functions */
static void Exit_EditDimension( EDA_DRAW_PANEL* Panel, wxDC* DC );
static void Montre_Position_New_Dimension( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                                           const wxPoint& aPosition, bool aErase );

/* Local variables : */
static int status_dimension; /* Used in cimension creation:
                              * = 0 : initial value: no dimension in progress
                              *  = 1 : First point created
                              *  = 2 : Secont point created, the text must be placed */

/*
 *  A dimension has this shape:
 *  It has 2 reference points, and a text
 * |            |
 * |    dist    |
 * |<---------->|
 * |            |
 *
 */


/*********************************/
/* class DIMENSION_EDITOR_DIALOG */
/*********************************/

class DIMENSION_EDITOR_DIALOG : public wxDialog
{
private:

    PCB_EDIT_FRAME* m_Parent;
    wxDC*           m_DC;
    DIMENSION*      CurrentDimension;
    wxTextCtrl*     m_Name;
    EDA_SIZE_CTRL*  m_TxtSizeCtrl;
    EDA_VALUE_CTRL* m_TxtWidthCtrl;
    wxRadioBox*     m_Mirror;
    wxComboBox*     m_SelLayerBox;

public:

    // Constructor and destructor
    DIMENSION_EDITOR_DIALOG( PCB_EDIT_FRAME* parent, DIMENSION* Dimension, wxDC* DC );
    ~DIMENSION_EDITOR_DIALOG()
    {
    }


private:
    void    OnCancelClick( wxCommandEvent& event );
    void    OnOkClick( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( DIMENSION_EDITOR_DIALOG, wxDialog )
    EVT_BUTTON( wxID_OK, DIMENSION_EDITOR_DIALOG::OnOkClick )
    EVT_BUTTON( wxID_CANCEL, DIMENSION_EDITOR_DIALOG::OnCancelClick )
END_EVENT_TABLE()


DIMENSION_EDITOR_DIALOG::DIMENSION_EDITOR_DIALOG( PCB_EDIT_FRAME* parent,
                                                  DIMENSION* Dimension, wxDC* DC
                                                  ) :
    wxDialog( parent, -1, wxString( _( "Dimension properties" ) ) )
{
    wxButton* Button;

    m_Parent = parent;
    m_DC = DC;
    Centre();

    CurrentDimension = Dimension;

    wxBoxSizer* MainBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    SetSizer( MainBoxSizer );
    wxBoxSizer* LeftBoxSizer  = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer* RightBoxSizer = new wxBoxSizer( wxVERTICAL );
    MainBoxSizer->Add( LeftBoxSizer, 0, wxGROW | wxALL, 5 );
    MainBoxSizer->Add( RightBoxSizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    /* Creation des boutons de commande */
    Button = new wxButton( this, wxID_OK, _( "OK" ) );
    RightBoxSizer->Add( Button, 0, wxGROW | wxALL, 5 );

    Button = new wxButton( this, wxID_CANCEL, _( "Cancel" ) );
    RightBoxSizer->Add( Button, 0, wxGROW | wxALL, 5 );

    wxString display_msg[2] = { _( "Normal" ), _( "Mirror" ) };
    m_Mirror = new wxRadioBox( this, -1, _( "Display" ),
                               wxDefaultPosition, wxSize( -1, -1 ), 2, display_msg,
                               1, wxRA_SPECIFY_COLS );

    if( Dimension->m_Text->m_Mirror )
        m_Mirror->SetSelection( 1 );

    RightBoxSizer->Add( m_Mirror, 0, wxGROW | wxALL, 5 );

    LeftBoxSizer->Add( new wxStaticText( this, -1, _( "Text:" ) ),
                       0, wxGROW | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5 );

    m_Name = new wxTextCtrl( this, -1, Dimension->m_Text->m_Text,
                             wxDefaultPosition, wxSize( 200, -1 ) );

    m_Name->SetInsertionPoint( 1 );

    LeftBoxSizer->Add( m_Name,
                       0,
                       wxGROW | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
                       5 );

    m_TxtSizeCtrl = new EDA_SIZE_CTRL( this, _( "Size" ),
                                       Dimension->m_Text->m_Size,
                                       g_UserUnit, LeftBoxSizer, m_Parent->m_InternalUnits );

    m_TxtWidthCtrl = new EDA_VALUE_CTRL( this, _( "Width" ),
                                         Dimension->m_Width,
                                         g_UserUnit, LeftBoxSizer, m_Parent->m_InternalUnits );

    wxStaticText* text = new wxStaticText( this, -1, _( "Layer:" ) );
    LeftBoxSizer->Add( text, 0, wxGROW | wxLEFT | wxRIGHT | wxTOP, 5 );
    m_SelLayerBox = new wxComboBox( this, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize,
                                    0, NULL, wxCB_READONLY );
    LeftBoxSizer->Add( m_SelLayerBox, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    for( int layer = FIRST_NO_COPPER_LAYER;  layer<NB_LAYERS;  layer++ )
    {
        m_SelLayerBox->Append( parent->GetBoard()->GetLayerName( layer ) );
    }

    m_SelLayerBox->SetSelection( Dimension->GetLayer() - FIRST_NO_COPPER_LAYER );

    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
}


void DIMENSION_EDITOR_DIALOG::OnCancelClick( wxCommandEvent& event )
{
    EndModal( -1 );
}


void DIMENSION_EDITOR_DIALOG::OnOkClick( wxCommandEvent& event )
{
    if( m_DC )     // Delete old text.
    {
        CurrentDimension->Draw( m_Parent->DrawPanel, m_DC, GR_XOR );
    }

    m_Parent->SaveCopyInUndoList(CurrentDimension, UR_CHANGED);

    if( m_Name->GetValue() != wxEmptyString )
    {
        CurrentDimension->SetText( m_Name->GetValue() );
    }

    CurrentDimension->m_Text->m_Size  = m_TxtSizeCtrl->GetValue();

    int width = m_TxtWidthCtrl->GetValue();
    int maxthickness = Clamp_Text_PenSize( width, CurrentDimension->m_Text->m_Size );

    if( width > maxthickness )
    {
        DisplayError( NULL,
                      _( "The text thickness is too large for the text size. It will be clamped") );
        width = maxthickness;
    }

    CurrentDimension->m_Text->m_Thickness = CurrentDimension->m_Width = width ;

    CurrentDimension->m_Text->m_Mirror = ( m_Mirror->GetSelection() == 1 ) ? true : false;

    CurrentDimension->SetLayer( m_SelLayerBox->GetCurrentSelection() + FIRST_NO_COPPER_LAYER );

    CurrentDimension->AdjustDimensionDetails( true );

    if( m_DC )     // Display new text
    {
        CurrentDimension->Draw( m_Parent->DrawPanel, m_DC, GR_OR );
    }

    m_Parent->OnModify();
    EndModal( 1 );
}


static void Exit_EditDimension( EDA_DRAW_PANEL* Panel, wxDC* DC )
{
    DIMENSION* Dimension = (DIMENSION*) Panel->GetScreen()->GetCurItem();

    if( Dimension )
    {
        if( Dimension->IsNew() )
        {
            Dimension->Draw( Panel, DC, GR_XOR );
            Dimension->DeleteStructure();
        }
        else
        {
            Dimension->Draw( Panel, DC, GR_OR );
        }
    }

    status_dimension = 0;
    ((PCB_EDIT_FRAME*)Panel->GetParent())->SetCurItem( NULL );
}


DIMENSION* PCB_EDIT_FRAME::Begin_Dimension( DIMENSION* Dimension, wxDC* DC )
{
    wxPoint pos;

    if( Dimension == NULL )       /* debut reel du trace */
    {
        status_dimension = 1;
        pos = GetScreen()->GetCrossHairPosition();

        Dimension = new DIMENSION( GetBoard() );
        Dimension->m_Flags = IS_NEW;

        Dimension->SetLayer( getActiveLayer() );

        Dimension->Barre_ox = Dimension->Barre_fx = pos.x;
        Dimension->Barre_oy = Dimension->Barre_fy = pos.y;

        Dimension->TraitD_ox = Dimension->TraitD_fx = pos.x;
        Dimension->TraitD_oy = Dimension->TraitD_fy = pos.y;

        Dimension->TraitG_ox = Dimension->TraitG_fx = pos.x;
        Dimension->TraitG_oy = Dimension->TraitG_fy = pos.y;

        Dimension->FlecheG1_ox = Dimension->FlecheG1_fx = pos.x;
        Dimension->FlecheG1_oy = Dimension->FlecheG1_fy = pos.y;

        Dimension->FlecheG2_ox = Dimension->FlecheG2_fx = pos.x;
        Dimension->FlecheG2_oy = Dimension->FlecheG2_fy = pos.y;

        Dimension->FlecheD1_ox = Dimension->FlecheD1_fx = pos.x;
        Dimension->FlecheD1_oy = Dimension->FlecheD1_fy = pos.y;

        Dimension->FlecheD2_ox = Dimension->FlecheD2_fx = pos.x;
        Dimension->FlecheD2_oy = Dimension->FlecheD2_fy = pos.y;

        Dimension->m_Text->m_Size   = GetBoard()->GetBoardDesignSettings()->m_PcbTextSize;
        int width = GetBoard()->GetBoardDesignSettings()->m_PcbTextWidth;
        int maxthickness = Clamp_Text_PenSize(width, Dimension->m_Text->m_Size );

        if( width > maxthickness )
        {
            width = maxthickness;
        }

        Dimension->m_Text->m_Thickness = Dimension->m_Width = width ;

        Dimension->AdjustDimensionDetails( );

        Dimension->Draw( DrawPanel, DC, GR_XOR );

        DrawPanel->SetMouseCapture( Montre_Position_New_Dimension, Exit_EditDimension );
        return Dimension;
    }

    // Dimension != NULL
    if( status_dimension == 1 )
    {
        status_dimension = 2;
        return Dimension;
    }

    Dimension->Draw( DrawPanel, DC, GR_OR );
    Dimension->m_Flags = 0;

    /* ADD this new item in list */
    GetBoard()->Add( Dimension );

    // Add store it in undo/redo list
    SaveCopyInUndoList( Dimension, UR_NEW );

    OnModify();
    DrawPanel->SetMouseCapture( NULL, NULL );

    return NULL;
}


static void Montre_Position_New_Dimension( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                                           const wxPoint& aPosition, bool aErase )
{
    PCB_SCREEN* screen   = (PCB_SCREEN*) aPanel->GetScreen();
    DIMENSION*  Dimension = (DIMENSION*) screen->GetCurItem();
    wxPoint     pos = screen->GetCrossHairPosition();

    if( Dimension == NULL )
        return;

    // Erase previous dimension.
    if( aErase )
    {
        Dimension->Draw( aPanel, aDC, GR_XOR );
    }

    Dimension->SetLayer( screen->m_Active_Layer );

    if( status_dimension == 1 )
    {
        Dimension->TraitD_ox = pos.x;
        Dimension->TraitD_oy = pos.y;
        Dimension->Barre_fx  = Dimension->TraitD_ox;
        Dimension->Barre_fy  = Dimension->TraitD_oy;
        Dimension->AdjustDimensionDetails( );
    }
    else
    {
        int   deltax, deltay, dx, dy;
        float angle, depl;
        deltax = Dimension->TraitD_ox - Dimension->TraitG_ox;
        deltay = Dimension->TraitD_oy - Dimension->TraitG_oy;

        /* Calculating the direction of travel perpendicular to the selected axis. */
        angle = atan2( (double)deltay, (double)deltax ) + (M_PI / 2);

        deltax = pos.x - Dimension->TraitD_ox;
        deltay = pos.y - Dimension->TraitD_oy;
        depl   = ( deltax * cos( angle ) ) + ( deltay * sin( angle ) );
        dx = (int) ( depl * cos( angle ) );
        dy = (int) ( depl * sin( angle ) );
        Dimension->Barre_ox = Dimension->TraitG_ox + dx;
        Dimension->Barre_oy = Dimension->TraitG_oy + dy;
        Dimension->Barre_fx = Dimension->TraitD_ox + dx;
        Dimension->Barre_fy = Dimension->TraitD_oy + dy;

        Dimension->AdjustDimensionDetails( );
    }

    Dimension->Draw( aPanel, aDC, GR_XOR );
}


void PCB_EDIT_FRAME::Install_Edit_Dimension( DIMENSION* Dimension, wxDC* DC )
{
    if( Dimension == NULL )
        return;

    DIMENSION_EDITOR_DIALOG* frame = new DIMENSION_EDITOR_DIALOG( this, Dimension, DC );
    frame->ShowModal();
    frame->Destroy();
}


void PCB_EDIT_FRAME::Delete_Dimension( DIMENSION* Dimension, wxDC* DC )
{
    if( Dimension == NULL )
        return;

    if( DC )
        Dimension->Draw( DrawPanel, DC, GR_XOR );

    SaveCopyInUndoList(Dimension, UR_DELETED);
    Dimension->UnLink();
    OnModify();
}
