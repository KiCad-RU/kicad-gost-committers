/**
 * @file dimension.cpp
 * @brief Dialog and code for editing a dimension object.
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <fctsys.h>
#include <confirm.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <wxPcbStruct.h>
#include <drawtxt.h>
#include <dialog_helpers.h>
#include <macros.h>
#include <base_units.h>

#include <class_board.h>
#include <class_pcb_text.h>
#include <class_dimension.h>

#include <pcbnew.h>
#include <dialog_dimension_editor_base.h>

/* Local functions */
static void BuildDimension( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                           const wxPoint& aPosition, bool aErase );

static void MoveDimensionText( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                               const wxPoint& aPosition, bool aErase );
static void AbortMoveDimensionText( EDA_DRAW_PANEL* aPanel, wxDC* aDC );


/* Local variables : */
static int status_dimension; /* Used in dimension creation:
                              * = 0 : initial value: no dimension in progress
                              * = 1 : First point created
                              * = 2 : Second point created, the text must be placed */

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
/* class DIALOG_DIMENSION_EDITOR */
/*********************************/

class DIALOG_DIMENSION_EDITOR : public DIALOG_DIMENSION_EDITOR_BASE
{
private:

    PCB_EDIT_FRAME* m_Parent;
    wxDC*           m_DC;
    DIMENSION*      CurrentDimension;

public:

    // Constructor and destructor
    DIALOG_DIMENSION_EDITOR( PCB_EDIT_FRAME* aParent, DIMENSION* aDimension, wxDC* aDC );
    ~DIALOG_DIMENSION_EDITOR()
    {
    }


private:
    void OnCancelClick( wxCommandEvent& event );
    void OnOKClick( wxCommandEvent& event );
};


DIALOG_DIMENSION_EDITOR::DIALOG_DIMENSION_EDITOR( PCB_EDIT_FRAME* aParent,
                                                  DIMENSION* aDimension, wxDC* aDC ) :
    DIALOG_DIMENSION_EDITOR_BASE( aParent )
{
    SetFocus();

    m_Parent = aParent;
    m_DC = aDC;

    CurrentDimension = aDimension;

    if( aDimension->m_Text.m_Mirror )
        m_rbMirror->SetSelection( 1 );
    else
        m_rbMirror->SetSelection( 0 );

    m_Name->SetValue( aDimension->m_Text.m_Text );

    // Enter size value in dialog
    PutValueInLocalUnits( *m_TxtSizeXCtrl, aDimension->m_Text.m_Size.x );
    AddUnitSymbol( *m_staticTextSizeX );
    PutValueInLocalUnits( *m_TxtSizeYCtrl, aDimension->m_Text.m_Size.y );
    AddUnitSymbol( *m_staticTextSizeY );

    // Enter lines thickness value in dialog
    PutValueInLocalUnits( *m_TxtWidthCtrl, aDimension->m_Width );
    AddUnitSymbol( *m_staticTextWidth );

    // Enter position value in dialog
    PutValueInLocalUnits( *m_textCtrlPosX, aDimension->m_Text.m_Pos.x );
    AddUnitSymbol( *m_staticTextPosX );
    PutValueInLocalUnits( *m_textCtrlPosY, aDimension->m_Text.m_Pos.y );
    AddUnitSymbol( *m_staticTextPosY );

    for( int layer = FIRST_NO_COPPER_LAYER;  layer<NB_LAYERS;  layer++ )
    {
        m_SelLayerBox->Append( aParent->GetBoard()->GetLayerName( layer ) );
    }

    m_SelLayerBox->SetSelection( aDimension->GetLayer() - FIRST_NO_COPPER_LAYER );

    GetSizer()->Fit( this );
    GetSizer()->SetSizeHints( this );
    Centre();
}


void DIALOG_DIMENSION_EDITOR::OnCancelClick( wxCommandEvent& event )
{
    EndModal( -1 );
}


void DIALOG_DIMENSION_EDITOR::OnOKClick( wxCommandEvent& event )
{
    if( m_DC )     // Delete old text.
    {
        CurrentDimension->Draw( m_Parent->GetCanvas(), m_DC, GR_XOR );
    }

    m_Parent->SaveCopyInUndoList(CurrentDimension, UR_CHANGED);

    if( m_Name->GetValue() != wxEmptyString )
    {
        CurrentDimension->SetText( m_Name->GetValue() );
    }

    wxString msg;

    // Get new size value:
    msg = m_TxtSizeXCtrl->GetValue();
    CurrentDimension->m_Text.m_Size.x = ReturnValueFromString( g_UserUnit, msg );
    msg = m_TxtSizeYCtrl->GetValue();
    CurrentDimension->m_Text.m_Size.y = ReturnValueFromString( g_UserUnit, msg );

    // Get new position value:
    // It will be copied later in dimension, because
    msg = m_textCtrlPosX->GetValue();
    CurrentDimension->m_Text.m_Pos.x = ReturnValueFromString( g_UserUnit, msg );
    msg = m_textCtrlPosY->GetValue();
    CurrentDimension->m_Text.m_Pos.y = ReturnValueFromString( g_UserUnit, msg );

    // Get new line thickness value:
    msg = m_TxtWidthCtrl->GetValue();
    int width = ReturnValueFromString( g_UserUnit, msg );
    int maxthickness = Clamp_Text_PenSize( width, CurrentDimension->m_Text.m_Size );

    if( width > maxthickness )
    {
        DisplayError( NULL,
                      _( "The text thickness is too large for the text size. It will be clamped") );
        width = maxthickness;
    }

    CurrentDimension->m_Text.m_Thickness = CurrentDimension->m_Width = width ;

    CurrentDimension->m_Text.m_Mirror = ( m_rbMirror->GetSelection() == 1 ) ? true : false;

    CurrentDimension->SetLayer( m_SelLayerBox->GetCurrentSelection() + FIRST_NO_COPPER_LAYER );

    if( m_DC )     // Display new text
    {
        CurrentDimension->Draw( m_Parent->GetCanvas(), m_DC, GR_OR );
    }

    m_Parent->OnModify();
    EndModal( 1 );
}


static void AbortBuildDimension( EDA_DRAW_PANEL* Panel, wxDC* aDC )
{
    DIMENSION* Dimension = (DIMENSION*) Panel->GetScreen()->GetCurItem();

    if( Dimension )
    {
        if( Dimension->IsNew() )
        {
            Dimension->Draw( Panel, aDC, GR_XOR );
            Dimension->DeleteStructure();
        }
        else
        {
            Dimension->Draw( Panel, aDC, GR_OR );
        }
    }

    status_dimension = 0;
    ((PCB_EDIT_FRAME*)Panel->GetParent())->SetCurItem( NULL );
}


DIMENSION* PCB_EDIT_FRAME::EditDimension( DIMENSION* aDimension, wxDC* aDC )
{
    wxPoint pos;

    if( aDimension == NULL )
    {
        status_dimension = 1;
        pos = GetScreen()->GetCrossHairPosition();

        aDimension = new DIMENSION( GetBoard() );
        aDimension->SetFlags( IS_NEW );

        aDimension->SetLayer( getActiveLayer() );

        aDimension->m_crossBarO = aDimension->m_crossBarF = pos;
        aDimension->m_featureLineDO = aDimension->m_featureLineDF = pos;
        aDimension->m_featureLineGO = aDimension->m_featureLineGF = pos;
        aDimension->m_arrowG1O = aDimension->m_arrowG1F = pos;
        aDimension->m_arrowG2O = aDimension->m_arrowG2F = pos;
        aDimension->m_arrowD1O = aDimension->m_arrowD1F = pos;
        aDimension->m_arrowD2O = aDimension->m_arrowD2F = pos;

        aDimension->m_Text.m_Size   = GetBoard()->GetDesignSettings().m_PcbTextSize;
        int width = GetBoard()->GetDesignSettings().m_PcbTextWidth;
        int maxthickness = Clamp_Text_PenSize(width, aDimension->m_Text.m_Size );

        if( width > maxthickness )
        {
            width = maxthickness;
        }

        aDimension->m_Text.m_Thickness = aDimension->m_Width = width ;

        aDimension->AdjustDimensionDetails( );

        aDimension->Draw( m_canvas, aDC, GR_XOR );

        m_canvas->SetMouseCapture( BuildDimension, AbortBuildDimension );
        return aDimension;
    }

    // Dimension != NULL
    if( status_dimension == 1 )
    {
        status_dimension = 2;
        return aDimension;
    }

    aDimension->Draw( m_canvas, aDC, GR_OR );
    aDimension->ClearFlags();

    /* ADD this new item in list */
    GetBoard()->Add( aDimension );

    // Add store it in undo/redo list
    SaveCopyInUndoList( aDimension, UR_NEW );

    OnModify();
    m_canvas->SetMouseCapture( NULL, NULL );

    return NULL;
}


static void BuildDimension( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
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
        Dimension->m_featureLineDO = pos;
        Dimension->m_crossBarF  = Dimension->m_featureLineDO;
        Dimension->AdjustDimensionDetails( );
    }
    else
    {
        wxPoint delta;
        int dx, dy;
        float angle, depl;
        delta = Dimension->m_featureLineDO - Dimension->m_featureLineGO;

        /* Calculating the direction of travel perpendicular to the selected axis. */
        angle = atan2( (double)delta.y, (double)delta.x ) + (M_PI / 2);

        delta = pos - Dimension->m_featureLineDO;
        depl   = ( delta.x * cos( angle ) ) + ( delta.y * sin( angle ) );
        dx = (int) ( depl * cos( angle ) );
        dy = (int) ( depl * sin( angle ) );
        Dimension->m_crossBarO.x = Dimension->m_featureLineGO.x + dx;
        Dimension->m_crossBarO.y = Dimension->m_featureLineGO.y + dy;
        Dimension->m_crossBarF.x = Dimension->m_featureLineDO.x + dx;
        Dimension->m_crossBarF.y = Dimension->m_featureLineDO.y + dy;

        Dimension->AdjustDimensionDetails( );
    }

    Dimension->Draw( aPanel, aDC, GR_XOR );
}


void PCB_EDIT_FRAME::ShowDimensionPropertyDialog( DIMENSION* aDimension, wxDC* aDC )
{
    if( aDimension == NULL )
        return;

    DIALOG_DIMENSION_EDITOR* frame = new DIALOG_DIMENSION_EDITOR( this, aDimension, aDC );
    frame->ShowModal();
    frame->Destroy();
}


void PCB_EDIT_FRAME::DeleteDimension( DIMENSION* aDimension, wxDC* aDC )
{
    if( aDimension == NULL )
        return;

    if( aDC )
        aDimension->Draw( m_canvas, aDC, GR_XOR );

    SaveCopyInUndoList( aDimension, UR_DELETED );
    aDimension->UnLink();
    OnModify();
}

/* Initialize parameters to move a pcb text
 */
static wxPoint initialTextPosition;
void PCB_EDIT_FRAME::BeginMoveDimensionText( DIMENSION* aItem, wxDC* DC )
{
    if( aItem == NULL )
        return;

    // Store the initial position for undo/abort command
    initialTextPosition = aItem->m_Text.m_Pos;

    aItem->Draw( m_canvas, DC, GR_XOR );
    aItem->SetFlags( IS_MOVED );
    SetMsgPanel( aItem );

    GetScreen()->SetCrossHairPosition( aItem->m_Text.m_Pos );
    m_canvas->MoveCursorToCrossHair();

    m_canvas->SetMouseCapture( MoveDimensionText, AbortMoveDimensionText );
    SetCurItem( aItem );
    m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );
}


/* Move dimension text following the cursor. */
static void MoveDimensionText( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                            bool aErase )
{
    DIMENSION* dimension = (DIMENSION*) aPanel->GetScreen()->GetCurItem();

    if( dimension == NULL )
        return;

    if( aErase )
        dimension->Draw( aPanel, aDC, GR_XOR );

    dimension->m_Text.m_Pos = aPanel->GetScreen()->GetCrossHairPosition();

    dimension->Draw( aPanel, aDC, GR_XOR );
}

/*
 * Abort current text edit progress.
 *
 * If a text is selected, its initial coord are regenerated
 */
void AbortMoveDimensionText( EDA_DRAW_PANEL* aPanel, wxDC* aDC )
{
    DIMENSION* dimension = (DIMENSION*) aPanel->GetScreen()->GetCurItem();
    ( (PCB_EDIT_FRAME*) aPanel->GetParent() )->SetCurItem( NULL );

    aPanel->SetMouseCapture( NULL, NULL );

    if( dimension == NULL )  // Should not occur
        return;

    dimension->Draw( aPanel, aDC, GR_XOR );
    dimension->m_Text.m_Pos = initialTextPosition;
    dimension->ClearFlags();
    dimension->Draw( aPanel, aDC, GR_OR );
}

/*
 *  Place the current dimension text being moving
 */
void PCB_EDIT_FRAME::PlaceDimensionText( DIMENSION* aItem, wxDC* DC )
{
    m_canvas->SetMouseCapture( NULL, NULL );
    SetCurItem( NULL );

    if( aItem == NULL )
        return;

    aItem->Draw( m_canvas, DC, GR_OR );
    OnModify();

    EXCHG( aItem->m_Text.m_Pos, initialTextPosition );
    SaveCopyInUndoList( aItem, UR_CHANGED );
    EXCHG( aItem->m_Text.m_Pos, initialTextPosition );

    aItem->ClearFlags();
}
