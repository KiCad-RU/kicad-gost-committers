/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jp.charras at wanadoo.fr
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
 * @file info3d_visu.cpp
 */

#include <fctsys.h>

#include <common.h>

#include <class_board_design_settings.h>
#include <class_board.h>

#include <3d_viewer.h>
#include <info3d_visu.h>
#include <trackball.h>

// Thickness of copper
// TODO: define the actual copper thickness by user
#define COPPER_THICKNESS (int)(0.035 * IU_PER_MM)   // for 35 u
#define TECH_LAYER_THICKNESS (int)(0.04 * IU_PER_MM)
#define EPOXY_THICKNESS (int)(1.6 * IU_PER_MM)   // for 1.6 mm


/* INFO3D_VISU in an helper class to store parameters like scaling factors,
 * board size, Z coordinates of layers ...
 * to create the 3D GLList
 */
INFO3D_VISU::INFO3D_VISU()
{
    int ii;

    m_Beginx    = m_Beginy = 0.0;   // position of mouse
    m_Zoom      = 1.0;
    m_3D_Grid   = 10.0;             // Grid value in mm
    trackball( m_Quat, 0.0, 0.0, 0.0, 0.0 );

    for( ii = 0; ii < 4; ii++ )
        m_Rot[ii] = 0.0;

    m_CopperLayersCount = 2;
    m_BoardSettings     = NULL;
    m_CopperThickness   = 0;
    m_EpoxyThickness    = 0;
    m_NonCopperLayerThickness = 0;

    // default all special item layers Visible
    for( ii = 0; ii < FL_LAST; ii++ )
        m_DrawFlags[ii] = true;

    m_DrawFlags[FL_GRID] = false;
    m_DrawFlags[FL_USE_COPPER_THICKNESS] = false;
}


INFO3D_VISU::~INFO3D_VISU()
{
}


/* Initialize info 3D Parameters from aBoard
 */
void INFO3D_VISU::InitSettings( BOARD* aBoard )
{
    EDA_RECT bbbox = aBoard->ComputeBoundingBox( false );

    if( bbbox.GetWidth() == 0 && bbbox.GetHeight() == 0 )
    {
        bbbox.SetWidth( 100 * IU_PER_MM );
        bbbox.SetHeight( 100 * IU_PER_MM );
    }

    m_BoardSettings = &aBoard->GetDesignSettings();

    m_BoardSize = bbbox.GetSize();
    m_BoardPos  = bbbox.Centre();

    m_BoardPos.y = -m_BoardPos.y;
    m_CopperLayersCount = aBoard->GetCopperLayerCount();

    // Ensure the board has 2 sides for 3D views, because it is hard to find
    // a *really* single side board in the true life...
    if( m_CopperLayersCount < 2 )
        m_CopperLayersCount = 2;

    m_BiuTo3Dunits = 2.0 / std::max( m_BoardSize.x, m_BoardSize.y );

    m_EpoxyThickness = aBoard->GetDesignSettings().GetBoardThickness() * m_BiuTo3Dunits;

    // TODO use value defined by user (currently use default values by ctor
    m_CopperThickness   = COPPER_THICKNESS * m_BiuTo3Dunits;
    m_NonCopperLayerThickness = TECH_LAYER_THICKNESS * m_BiuTo3Dunits;

    // Init  Z position of each layer
    // calculate z position for each copper layer
    int layer;
    int copper_layers_cnt = m_CopperLayersCount;

    for( layer = 0; layer < copper_layers_cnt; layer++ )
    {
        m_LayerZcoord[layer] =
            m_EpoxyThickness * layer / (copper_layers_cnt - 1);
    }

    double  zpos_copper_back    = m_LayerZcoord[0];
    double  zpos_copper_front   = m_EpoxyThickness;

    // Fill remaining unused copper layers and front layer zpos
    // with m_EpoxyThickness
    for( ; layer <= LAST_COPPER_LAYER; layer++ )
    {
        m_LayerZcoord[layer] = m_EpoxyThickness;
    }

    // calculate z position for each non copper layer
    for( int layer_id = FIRST_NO_COPPER_LAYER; layer_id < NB_LAYERS; layer_id++ )
    {
        double zpos;
        #define NonCopperLayerThicknessMargin 1.1

        switch( layer_id )
        {
        case ADHESIVE_N_BACK:
            zpos = zpos_copper_back -
                   4 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        case ADHESIVE_N_FRONT:
            zpos = zpos_copper_front +
                   4 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        case SOLDERPASTE_N_BACK:
            zpos = zpos_copper_back -
                   3 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        case SOLDERPASTE_N_FRONT:
            zpos = zpos_copper_front +
                   3 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        case SOLDERMASK_N_BACK:
            zpos = zpos_copper_back -
                   1 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        case SOLDERMASK_N_FRONT:
            zpos = zpos_copper_front +
                   1 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        case SILKSCREEN_N_BACK:
            zpos = zpos_copper_back -
                   2 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        case SILKSCREEN_N_FRONT:
            zpos = zpos_copper_front +
                   2 * m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;

        default:
            zpos = zpos_copper_front +
                   (layer_id - FIRST_NO_COPPER_LAYER + 5) *
                   m_NonCopperLayerThickness * NonCopperLayerThicknessMargin;
            break;
        }

        m_LayerZcoord[layer_id] = zpos;
    }
}

/* return the Z position of 3D shapes, in 3D Units
 * aIsFlipped: true for modules on Front (top) layer, false
 * if on back (bottom) layer
 * Note: in draw functions, the copper has a thickness = m_CopperThickness
 * Vias and tracks are draw with the top side position = m_CopperThickness/2
 * and the bottom side position = -m_CopperThickness/2 from the Z layer position
 */
double INFO3D_VISU::GetModulesZcoord3DIU( bool aIsFlipped )
{
    if(  aIsFlipped )
        return m_LayerZcoord[LAYER_N_BACK] - ( m_CopperThickness / 2 );
    else
        return m_LayerZcoord[LAYER_N_FRONT] + ( m_CopperThickness / 2 );
}

