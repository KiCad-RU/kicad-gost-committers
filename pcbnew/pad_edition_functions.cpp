/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file pad_edition_functions.cpp
 */

#include <fctsys.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <trigo.h>
#include <wxBasePcbFrame.h>

#include <pcbnew.h>
#include <class_board.h>
#include <class_module.h>
#include <class_pad.h>
#include <class_board_design_settings.h>

/* Exports the current pad settings to board design settings.
 */
void PCB_BASE_FRAME::Export_Pad_Settings( D_PAD* aPad )
{
    if( aPad == NULL )
        return;

    SetMsgPanel( aPad );

    D_PAD& mp = GetDesignSettings().m_Pad_Master;

    mp.SetShape( aPad->GetShape() );
    mp.SetAttribute( aPad->GetAttribute() );
    mp.SetLayerMask( aPad->GetLayerMask() );

    mp.SetOrientation( aPad->GetOrientation() - aPad->GetParent()->GetOrientation() );

    mp.SetSize( aPad->GetSize() );
    mp.SetDelta( aPad->GetDelta() );

    mp.SetOffset( aPad->GetOffset() );
    mp.SetDrillSize( aPad->GetDrillSize() );
    mp.SetDrillShape( aPad->GetDrillShape() );
}


/* Imports the board design settings to aPad
 * - The position, names, and keys are not modifed.
 */
void PCB_BASE_FRAME::Import_Pad_Settings( D_PAD* aPad, bool aDraw )
{
    if( aDraw )
    {
        aPad->SetFlags( DO_NOT_DRAW );
        m_canvas->RefreshDrawingRect( aPad->GetBoundingBox() );
        aPad->ClearFlags( DO_NOT_DRAW );
    }

    D_PAD& mp = GetDesignSettings().m_Pad_Master;

    aPad->SetShape( mp.GetShape() );
    aPad->SetLayerMask( mp.GetLayerMask() );
    aPad->SetAttribute( mp.GetAttribute() );
    aPad->SetOrientation( mp.GetOrientation() + aPad->GetParent()->GetOrientation() );
    aPad->SetSize( mp.GetSize() );
    aPad->SetDelta( wxSize( 0, 0 ) );
    aPad->SetOffset( mp.GetOffset() );
    aPad->SetDrillSize( mp.GetDrillSize() );
    aPad->SetDrillShape( mp.GetDrillShape() );

    switch( mp.GetShape() )
    {
    case PAD_TRAPEZOID:
        aPad->SetDelta( mp.GetDelta() );
        break;

    case PAD_CIRCLE:
        // ensure size.y == size.x
        aPad->SetSize( wxSize( aPad->GetSize().x, aPad->GetSize().x ) );
        break;

    default:
        ;
    }

    switch( mp.GetAttribute() )
    {
    case PAD_SMD:
    case PAD_CONN:
        aPad->SetDrillSize( wxSize( 0, 0 ) );
        aPad->SetOffset( wxPoint( 0, 0 ) );
        break;
    default:
        ;
    }

    if( aDraw )
        m_canvas->RefreshDrawingRect( aPad->GetBoundingBox() );

    aPad->GetParent()->SetLastEditTime();
}


/* Add a new pad to aModule.
 */
void PCB_BASE_FRAME::AddPad( MODULE* aModule, bool draw )
{
    // Last used pad name (pad num)
    wxString lastPadName = GetDesignSettings().m_Pad_Master.GetPadName();

    m_Pcb->m_Status_Pcb     = 0;
    aModule->SetLastEditTime();

    D_PAD* pad = new D_PAD( aModule );

    // Add the new pad to end of the module pad list.
    aModule->m_Pads.PushBack( pad );

    // Update the pad properties.
    Import_Pad_Settings( pad, false );
    pad->SetNetname( wxEmptyString );

    pad->SetPosition( GetScreen()->GetCrossHairPosition() );

    // Set the relative pad position
    // ( pad position for module orient, 0, and relative to the module position)

    wxPoint pos0 = pad->GetPosition() - aModule->GetPosition();
    RotatePoint( &pos0, -aModule->GetOrientation() );
    pad->SetPos0( pos0 );

    // Automatically increment the current pad number.
    long num    = 0;
    int  ponder = 1;

    while( lastPadName.Len() && lastPadName.Last() >= '0' && lastPadName.Last() <= '9' )
    {
        num += ( lastPadName.Last() - '0' ) * ponder;
        lastPadName.RemoveLast();
        ponder *= 10;
    }

    num++;  // Use next number for the new pad
    lastPadName << num;
    pad->SetPadName( lastPadName );

    GetDesignSettings().m_Pad_Master.SetPadName(lastPadName);

    aModule->CalculateBoundingBox();
    SetMsgPanel( pad );

    if( draw )
        m_canvas->RefreshDrawingRect( aModule->GetBoundingBox() );
}


/**
 * Function DeletePad
 * Delete the pad aPad.
 * Refresh the modified screen area
 * Refresh modified parameters of the parent module (bounding box, last date)
 * @param aPad = the pad to delete
 * @param aQuery = true to promt for confirmation, false to delete silently
 */
void PCB_BASE_FRAME::DeletePad( D_PAD* aPad, bool aQuery )
{
    MODULE*  module;

    if( aPad == NULL )
        return;

    module = (MODULE*) aPad->GetParent();
    module->SetLastEditTime();

    if( aQuery )
    {
        wxString msg;
        msg.Printf( _( "Delete Pad (module %s %s) " ),
                    GetChars( module->m_Reference->m_Text ),
                    GetChars( module->m_Value->m_Text ) );

        if( !IsOK( this, msg ) )
            return;
    }

    m_Pcb->m_Status_Pcb = 0;
    aPad->DeleteStructure();
    m_canvas->RefreshDrawingRect( module->GetBoundingBox() );
    module->CalculateBoundingBox();

    OnModify();
}


// Rotate selected pad 90 degrees.
void PCB_BASE_FRAME::RotatePad( D_PAD* aPad, wxDC* DC )
{
    if( aPad == NULL )
        return;

    MODULE* module = aPad->GetParent();

    module->SetLastEditTime();

    OnModify();

    if( DC )
        module->Draw( m_canvas, DC, GR_XOR );

    wxSize  sz = aPad->GetSize();
    EXCHG( sz.x, sz.y );
    aPad->SetSize( sz );

    sz = aPad->GetDrillSize();
    EXCHG( sz.x, sz.y );
    aPad->SetDrillSize( sz );

    wxPoint pt = aPad->GetOffset();
    EXCHG( pt.x, pt.y );
    aPad->SetOffset( pt );

    aPad->SetOffset( wxPoint( aPad->GetOffset().x, -aPad->GetOffset().y ) );

    sz = aPad->GetDelta();
    EXCHG( sz.x, sz.y );
    sz.x = -sz.x;
    aPad->SetDelta( sz );

    module->CalculateBoundingBox();
    SetMsgPanel( aPad );

    if( DC )
        module->Draw( m_canvas, DC, GR_OR );
}
