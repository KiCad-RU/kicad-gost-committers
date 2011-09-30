/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2008 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2004-2007 KiCad Developers, see change_log.txt for contributors.
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

#include "collectors.h"
#include "class_board_item.h"             // class BOARD_ITEM

#include "class_module.h"
#include "class_pad.h"


/*  This module contains out of line member functions for classes given in
  * collectors.h.  Those classes augment the functionality of class PCB_EDIT_FRAME.
 */


// see collectors.h
const KICAD_T GENERAL_COLLECTOR::AllBoardItems[] = {
    // there are some restrictions on the order of items in the general case.
    // all items in m_Drawings for instance should be contiguous.
    //  *** all items in a same list (shown here) must be contiguous ****
    TYPE_MARKER_PCB,             // in m_markers
    TYPE_TEXTE,                  // in m_Drawings
    TYPE_DRAWSEGMENT,            // in m_Drawings
    TYPE_DIMENSION,              // in m_Drawings
    PCB_TARGET_T,                // in m_Drawings
    TYPE_VIA,                    // in m_Tracks
    TYPE_TRACK,                  // in m_Tracks
    TYPE_PAD,                    // in modules
    TYPE_TEXTE_MODULE,           // in modules
    TYPE_MODULE,                 // in m_Modules
    TYPE_ZONE,                   // in m_Zones
    TYPE_ZONE_CONTAINER,         // in m_ZoneDescriptorList
    EOT
};


/*
  * const KICAD_T GENERAL_COLLECTOR::PrimaryItems[] = {
  * TYPE_TEXTE,
  * TYPE_DRAWSEGMENT,
  * TYPE_DIMENSION,
  * TYPE_VIA,
  * TYPE_TRACK,
  * TYPE_MODULE,
  * EOT
  * };
 */


const KICAD_T GENERAL_COLLECTOR::AllButZones[] = {
    TYPE_MARKER_PCB,
    TYPE_TEXTE,
    TYPE_DRAWSEGMENT,
    TYPE_DIMENSION,
    PCB_TARGET_T,
    TYPE_VIA,
    TYPE_TRACK,
    TYPE_PAD,
    TYPE_TEXTE_MODULE,
    TYPE_MODULE,
    TYPE_ZONE_CONTAINER,         // if it is visible on screen, it should be selectable
    EOT
};


const KICAD_T GENERAL_COLLECTOR::ModuleItems[] = {
    TYPE_MODULE,
    EOT
};


const KICAD_T GENERAL_COLLECTOR::PadsOrModules[] = {
    TYPE_PAD,
    TYPE_MODULE,
    EOT
};


const KICAD_T GENERAL_COLLECTOR::PadsTracksOrZones[] = {
    TYPE_PAD,
    TYPE_VIA,
    TYPE_TRACK,
    TYPE_ZONE,
    TYPE_ZONE_CONTAINER,
    EOT
};


const KICAD_T GENERAL_COLLECTOR::ModulesAndTheirItems[] = {
    TYPE_TEXTE_MODULE,
    TYPE_EDGE_MODULE,
    TYPE_PAD,
    TYPE_MODULE,
    EOT
};


const KICAD_T GENERAL_COLLECTOR::Tracks[] = {
    TYPE_TRACK,
    TYPE_VIA,
    EOT
};

const KICAD_T GENERAL_COLLECTOR::Zones[] = {
    TYPE_ZONE_CONTAINER,
    EOT
};



/**
 * Function Inspect
 * is the examining function within the INSPECTOR which is passed to the
 * Iterate function.  Searches and collects all the objects that the old
 * function PcbGeneralLocateAndDisplay() would find, except that it keeps all
 * that it finds and does not do any displaying.
 *
 * @param testItem An EDA_ITEM to examine.
 * @param testData The const void* testData, not used here.
 * @return SEARCH_RESULT - SEARCH_QUIT if the Iterator is to stop the scan,
 *   else SCAN_CONTINUE;
 */
SEARCH_RESULT GENERAL_COLLECTOR::Inspect( EDA_ITEM* testItem, const void* testData )
{
    BOARD_ITEM* item   = (BOARD_ITEM*) testItem;
    MODULE*     module = NULL;
    D_PAD*      pad    = NULL;
    bool        pad_through = false;

#if 0   // debugging
    static int  breakhere = 0;

    switch( item->Type() )
    {
    case TYPE_PAD:
    {
        MODULE* m = (MODULE*) item->GetParent();

        if( m->GetReference() == wxT( "Y2" ) )
        {
            breakhere++;
        }
    }
        break;

    case TYPE_VIA:
        breakhere++;
        break;

    case TYPE_TRACK:
        breakhere++;
        break;

    case TYPE_ZONE:
        breakhere++;
        break;

    case TYPE_TEXTE:
        breakhere++;
        break;

    case TYPE_DRAWSEGMENT:
        breakhere++;
        break;

    case TYPE_DIMENSION:
        breakhere++;
        break;

    case TYPE_TEXTE_MODULE:
    {
        TEXTE_MODULE* tm = (TEXTE_MODULE*) item;

        if( tm->m_Text == wxT( "10uH" ) )
        {
            breakhere++;
        }
    }
        break;

    case TYPE_MODULE:
    {
        MODULE* m = (MODULE*) item;

        if( m->GetReference() == wxT( "C98" ) )
        {
            breakhere++;
        }
    }
        break;

    default:
        breakhere++;
        break;
    }

#endif


    switch( item->Type() )
    {
    case TYPE_PAD:
        // there are pad specific visibility controls.
        // Criterias to select a pad is:
        // for smd pads: the module parent must be seen, and pads on the corresponding
        // board side must be seen
        // if pad is a thru hole, then it can be visible when its parent module is not.
        // for through pads: pads on Front or Back board sides must be seen
        pad = (D_PAD*) item;

        if( (pad->m_Attribut != PAD_SMD) &&
            (pad->m_Attribut != PAD_CONN) )    // a hole is present, so multiple layers
        {
            // proceed to the common tests below, but without the parent module test,
            // by leaving module==NULL, but having pad != null
            pad_through = true;
        }
        else  // smd, so use pads test after module test
        {
            module = (MODULE*) item->GetParent();
        }

        break;

    case TYPE_VIA:
        break;

    case TYPE_TRACK:
        break;

    case TYPE_ZONE:
        break;

    case TYPE_ZONE_CONTAINER:
        break;

    case TYPE_TEXTE:
        break;

    case TYPE_DRAWSEGMENT:
        break;

    case TYPE_DIMENSION:
        break;

    case PCB_TARGET_T:
        break;

    case TYPE_TEXTE_MODULE:
        module = (MODULE*) item->GetParent();

        if( m_Guide->IgnoreMTextsMarkedNoShow() && ( (TEXTE_MODULE*) item )->m_NoShow )
            goto exit;

        if( module )
        {
            if( m_Guide->IgnoreMTextsOnCopper() && module->GetLayer()==LAYER_N_BACK )
                goto exit;

            if( m_Guide->IgnoreMTextsOnCmp() && module->GetLayer()==LAYER_N_FRONT )
                goto exit;
        }
        break;

    case TYPE_MODULE:
        module = (MODULE*) item;
        break;

    default:
        break;
    }

    // common tests:

    if( module )    // true from case TYPE_PAD, TYPE_TEXTE_MODULE, or TYPE_MODULE
    {
        if( m_Guide->IgnoreModulesOnCu() && module->GetLayer()==LAYER_N_BACK )
            goto exit;

        if( m_Guide->IgnoreModulesOnCmp() && module->GetLayer()==LAYER_N_FRONT )
            goto exit;
    }

    // Pads are not sensitive to the layer visibility controls.
    // They all have their own separate visibility controls
    // skip them if not visible
    if ( pad )
    {
        if( m_Guide->IgnorePads() )
            goto exit;

        if( ! pad_through )
        {
            if( m_Guide->IgnorePadsOnFront() && pad->IsOnLayer(LAYER_N_FRONT ) )
                goto exit;

            if( m_Guide->IgnorePadsOnBack() && pad->IsOnLayer(LAYER_N_BACK ) )
                goto exit;
        }
    }

    if( item->IsOnLayer( m_Guide->GetPreferredLayer() ) || m_Guide->IgnorePreferredLayer() )
    {
        int layer = item->GetLayer();

        // Modules and their subcomponents: text and pads are not sensitive to the layer
        // visibility controls.  They all have their own separate visibility controls
        if( module || pad || m_Guide->IsLayerVisible( layer ) || !m_Guide->IgnoreNonVisibleLayers() )
        {
            if( !m_Guide->IsLayerLocked( layer ) || !m_Guide->IgnoreLockedLayers() )
            {
                if( !item->IsLocked() || !m_Guide->IgnoreLockedItems() )
                {
                    if( item->HitTest( m_RefPos ) )
                    {
                        Append( item );
                        goto exit;
                    }
                }
            }
        }
    }


    if( m_Guide->IncludeSecondary() )
    {
        // for now, "secondary" means "tolerate any layer".  It has
        // no effect on other criteria, since there is a separate "ignore" control for
        // those in the COLLECTORS_GUIDE

        int layer = item->GetLayer();

        // Modules and their subcomponents: text and pads are not sensitive to the layer
        // visibility controls.  They all have their own separate visibility controls
        if( module || pad || m_Guide->IsLayerVisible( layer ) || !m_Guide->IgnoreNonVisibleLayers() )
        {
            if( !m_Guide->IsLayerLocked( layer ) || !m_Guide->IgnoreLockedLayers() )
            {
                if( !item->IsLocked() || !m_Guide->IgnoreLockedItems() )
                {
                    if( item->HitTest( m_RefPos ) )
                    {
                        Append2nd( item );
                        goto exit;
                    }
                }
            }
        }
    }

exit:
    return SEARCH_CONTINUE;     // always when collecting
}


// see collectors.h
void GENERAL_COLLECTOR::Collect( BOARD_ITEM* aItem, const KICAD_T aScanList[],
                                 const wxPoint& aRefPos, const COLLECTORS_GUIDE& aGuide )
{
    Empty();        // empty the collection, primary criteria list
    Empty2nd();     // empty the collection, secondary criteria list

    // remember guide, pass it to Inspect()
    SetGuide( &aGuide );

    SetScanTypes( aScanList );

    // remember where the snapshot was taken from and pass refPos to
    // the Inspect() function.
    SetRefPos( aRefPos );

    // visit the board or module with the INSPECTOR (me).
    aItem->Visit(   this,       // INSPECTOR* inspector
                    NULL,       // const void* testData, not used here
                    m_ScanTypes );

    SetTimeNow();               // when snapshot was taken

    // record the length of the primary list before concatonating on to it.
    m_PrimaryLength = m_List.size();

    // append 2nd list onto end of the first list
    for( unsigned i = 0;  i<m_List2nd.size();  ++i )
        Append( m_List2nd[i] );

    Empty2nd();
}


// see collectors.h
SEARCH_RESULT TYPE_COLLECTOR::Inspect( EDA_ITEM* testItem, const void* testData )
{
    // The Vist() function only visits the testItem if its type was in the
    // the scanList, so therefore we can collect anything given to us here.
    Append( testItem );

    return SEARCH_CONTINUE;     // always when collecting
}

void TYPE_COLLECTOR::Collect( BOARD_ITEM* aBoard, const KICAD_T aScanList[] )
{
    Empty();        // empty any existing collection

    // visit the board with the INSPECTOR (me).
    aBoard->Visit(      this,       // INSPECTOR* inspector
                        NULL,       // const void* testData,
                        aScanList );
}

//EOF
