/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2006 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file editrack-part2.cpp
 */


#include "fctsys.h"
#include "gr_basic.h"
#include "class_drawpanel.h"
#include "confirm.h"
#include "wxPcbStruct.h"
#include "pcbcommon.h"

#include "class_board.h"
#include "class_module.h"
#include "class_track.h"
#include "class_marker_pcb.h"

#include "pcbnew.h"
#include "drc_stuff.h"
#include "protos.h"


bool PCB_EDIT_FRAME::Other_Layer_Route( TRACK* aTrack, wxDC* DC )
{
    unsigned    itmp;

    if( aTrack == NULL )
    {
        if( getActiveLayer() != ((PCB_SCREEN*)GetScreen())->m_Route_Layer_TOP )
            setActiveLayer( ((PCB_SCREEN*)GetScreen())->m_Route_Layer_TOP );
        else
            setActiveLayer(((PCB_SCREEN*)GetScreen())->m_Route_Layer_BOTTOM );

        UpdateStatusBar();
        return true;
    }

    /* Avoid more than one via on the current location: */
    if( GetBoard()->GetViaByPosition( g_CurrentTrackSegment->m_End,
                                      g_CurrentTrackSegment->GetLayer() ) )
        return false;

    for( TRACK* segm = g_FirstTrackSegment;  segm;  segm = segm->Next() )
    {
        if( segm->Type() == PCB_VIA_T && g_CurrentTrackSegment->m_End == segm->m_Start )
            return false;
    }

    /* Is the current segment Ok (no DRC error) ? */
    if( Drc_On )
    {
        if( BAD_DRC==m_drc->Drc( g_CurrentTrackSegment, GetBoard()->m_Track ) )
            /* DRC error, the change layer is not made */
            return false;

        // Handle 2 segments.
        if( g_TwoSegmentTrackBuild && g_CurrentTrackSegment->Back() )
        {
            if( BAD_DRC == m_drc->Drc( g_CurrentTrackSegment->Back(), GetBoard()->m_Track ) )
                return false;
        }
    }

    /* Save current state before placing a via.
     * If the via cannot be placed this current state will be reused
     */
    itmp = g_CurrentTrackList.GetCount();
    Begin_Route( g_CurrentTrackSegment, DC );

    DrawPanel->m_mouseCaptureCallback( DrawPanel, DC, wxDefaultPosition, false );

    /* create the via */
    SEGVIA* via    = new SEGVIA( GetBoard() );
    via->m_Flags   = IS_NEW;
    via->m_Shape   = GetBoard()->GetBoardDesignSettings()->m_CurrentViaType;
    via->m_Width   = GetBoard()->GetCurrentViaSize();
    via->SetNet( GetBoard()->GetHighLightNetCode() );
    via->m_Start   = via->m_End = g_CurrentTrackSegment->m_End;

    // Usual via is from copper to component.
    // layer pair is LAYER_N_BACK and LAYER_N_FRONT.
    via->SetLayerPair( LAYER_N_BACK, LAYER_N_FRONT );
    via->SetDrillValue( GetBoard()->GetCurrentViaDrill() );

    int first_layer = getActiveLayer();
    int last_layer;

    // prepare switch to new active layer:
    if( first_layer != GetScreen()->m_Route_Layer_TOP )
        last_layer = GetScreen()->m_Route_Layer_TOP;
    else
        last_layer = GetScreen()->m_Route_Layer_BOTTOM;

    /* Adjust the actual via layer pair */
    switch ( via->Shape() )
    {
        case VIA_BLIND_BURIED:
            via->SetLayerPair( first_layer, last_layer );
            break;

        case VIA_MICROVIA:  // from external to the near neighbor inner layer
        {
            int last_inner_layer = GetBoard()->GetCopperLayerCount() - 2;
            if ( first_layer == LAYER_N_BACK )
                last_layer = LAYER_N_2;
            else if ( first_layer == LAYER_N_FRONT )
                last_layer = last_inner_layer;
            else if ( first_layer == LAYER_N_2 )
                last_layer = LAYER_N_BACK;
            else if ( first_layer == last_inner_layer )
                last_layer = LAYER_N_FRONT;

            // else error: will be removed later
            via->SetLayerPair( first_layer, last_layer );
            {
                NETINFO_ITEM* net = GetBoard()->FindNet( via->GetNet() );
                via->m_Width      = net->GetMicroViaSize();
            }
        }
            break;

        default:
            break;
    }

    if( Drc_On && BAD_DRC == m_drc->Drc( via, GetBoard()->m_Track ) )
    {
        /* DRC fault: the Via cannot be placed here ... */
        delete via;

        DrawPanel->m_mouseCaptureCallback( DrawPanel, DC, wxDefaultPosition, false );

        // delete the track(s) added in Begin_Route()
        while( g_CurrentTrackList.GetCount() > itmp )
        {
            Delete_Segment( DC, g_CurrentTrackSegment );
        }

         SetCurItem( g_CurrentTrackSegment, false );

        // Refresh DRC diag, erased by previous calls
        if( m_drc->GetCurrentMarker() )
            m_drc->GetCurrentMarker()->DisplayInfo( this );

        return false;
    }

    setActiveLayer( last_layer );

    TRACK*  lastNonVia = g_CurrentTrackSegment;

    /* A new via was created. It was Ok.
     */
    g_CurrentTrackList.PushBack( via );

    /* The via is now in linked list and we need a new track segment
     * after the via, starting at via location.
     * it will become the new current segment (from via to the mouse cursor)
     */

    TRACK* track = lastNonVia->Copy();

    /* the above creates a new segment from the last entered segment, with the
     * current width, flags, netcode, etc... values.
     * layer, start and end point are not correct,
     * and will be modified next
     */

    // set the layer to the new value
    track->SetLayer( getActiveLayer() );

    /* the start point is the via position and the end point is the cursor
     * which also is on the via (will change when moving mouse)
     */
    track->m_Start = track->m_End = via->m_Start;

    g_CurrentTrackList.PushBack( track );

    if( g_TwoSegmentTrackBuild )
    {
        // Create a second segment (we must have 2 track segments to adjust)
        g_CurrentTrackList.PushBack( g_CurrentTrackSegment->Copy() );
    }

    DrawPanel->m_mouseCaptureCallback( DrawPanel, DC, wxDefaultPosition, false );
    via->DisplayInfo( this );

    UpdateStatusBar();

    return true;
}


void PCB_EDIT_FRAME::DisplayNetStatus( wxDC* DC )
{
    TRACK* pt_segm;
    int    layerMask = (1 << getActiveLayer());
    wxPoint pos = GetScreen()->RefPos( true );

    pt_segm = GetBoard()->GetTrace( GetBoard()->m_Track, pos, layerMask );

    if( pt_segm == NULL )
        GetBoard()->DisplayInfo( this );
    else
        TestNetConnection( DC, pt_segm->GetNet() );
}


void PCB_EDIT_FRAME::Show_1_Ratsnest( EDA_ITEM* item, wxDC* DC )
{
    D_PAD*   pt_pad = NULL;
    MODULE*  Module = NULL;

    if( GetBoard()->IsElementVisible(RATSNEST_VISIBLE) )
        return;

    if( ( GetBoard()->m_Status_Pcb & LISTE_RATSNEST_ITEM_OK ) == 0 )
        Compile_Ratsnest( DC, true );

    if( item )
    {
        if( item->Type() == PCB_PAD_T )
        {
            pt_pad = (D_PAD*) item;
            Module = (MODULE*) pt_pad->GetParent();
        }

        if( pt_pad ) /* Displaying the ratsnest of the corresponding net. */
        {
            pt_pad->DisplayInfo( this );

            for( unsigned ii = 0; ii < GetBoard()->GetRatsnestsCount(); ii++ )
            {
                RATSNEST_ITEM* net = &GetBoard()->m_FullRatsnest[ii];

                if( net->GetNet() == pt_pad->GetNet() )
                {
                    if( ( net->m_Status & CH_VISIBLE ) != 0 )
                        continue;

                    net->m_Status |= CH_VISIBLE;

                    if( ( net->m_Status & CH_ACTIF ) == 0 )
                        continue;

                    net->Draw( DrawPanel, DC, GR_XOR, wxPoint( 0, 0 ) );
                }
            }
        }
        else
        {
            if( item->Type() == PCB_MODULE_TEXT_T )
            {
                if( item->GetParent() && ( item->GetParent()->Type() == PCB_MODULE_T ) )
                    Module = (MODULE*) item->GetParent();
            }
            else if( item->Type() == PCB_MODULE_T )
            {
                Module = (MODULE*) item;
            }

            if( Module )
            {
                Module->DisplayInfo( this );
                pt_pad = Module->m_Pads;

                for( ; pt_pad != NULL; pt_pad = (D_PAD*) pt_pad->Next() )
                {
                    for( unsigned ii = 0; ii < GetBoard()->GetRatsnestsCount(); ii++ )
                    {
                        RATSNEST_ITEM* net = &GetBoard()->m_FullRatsnest[ii];

                        if( ( net->m_PadStart == pt_pad ) || ( net->m_PadEnd == pt_pad ) )
                        {
                            if( net->m_Status & CH_VISIBLE )
                                continue;

                            net->m_Status |= CH_VISIBLE;

                            if( (net->m_Status & CH_ACTIF) == 0 )
                                continue;

                            net->Draw( DrawPanel, DC, GR_XOR, wxPoint( 0, 0 ) );
                        }
                    }
                }

                pt_pad = NULL;
            }
        }
    }

    /* Erase if no pad or module has been selected. */
    if( ( pt_pad == NULL ) && ( Module == NULL ) )
    {
        DrawGeneralRatsnest( DC );

        for( unsigned ii = 0; ii < GetBoard()->GetRatsnestsCount(); ii++ )
            GetBoard()->m_FullRatsnest[ii].m_Status &= ~CH_VISIBLE;
    }
}


void PCB_EDIT_FRAME::HighlightUnconnectedPads( wxDC* DC )
{
    for( unsigned ii = 0; ii < GetBoard()->GetRatsnestsCount(); ii++ )
    {
        RATSNEST_ITEM* net = &GetBoard()->m_FullRatsnest[ii];

        if( (net->m_Status & CH_ACTIF) == 0 )
            continue;

        net->m_PadStart->Draw( DrawPanel, DC, GR_OR | GR_HIGHLIGHT );
        net->m_PadEnd->Draw( DrawPanel, DC, GR_OR | GR_HIGHLIGHT );
    }
}
