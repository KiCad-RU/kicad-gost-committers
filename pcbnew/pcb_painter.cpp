/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#include <class_board.h>
#include <class_track.h>
#include <class_module.h>
#include <class_pad.h>
#include <class_drawsegment.h>
#include <class_zone.h>
#include <class_pcb_text.h>
#include <class_colors_design_settings.h>
#include <class_marker_pcb.h>
#include <class_dimension.h>
#include <class_mire.h>
#include <class_netinfo.h>
#include <pcbstruct.h>

#include <view/view.h>
#include <pcb_painter.h>
#include <gal/graphics_abstraction_layer.h>

using namespace KiGfx;

PCB_RENDER_SETTINGS::PCB_RENDER_SETTINGS()
{
    // By default everything should be displayed as filled
    for( unsigned int i = 0; i < END_PCB_VISIBLE_LIST; ++i )
    {
        m_sketchModeSelect[i] = false;
    }

    update();
}


void PCB_RENDER_SETTINGS::ImportLegacyColors( COLORS_DESIGN_SETTINGS* aSettings )
{
    for( int i = 0; i < NB_LAYERS; i++ )
    {
        m_layerColors[i] = m_legacyColorMap[aSettings->GetLayerColor( i )];
    }

    for( int i = 0; i < END_PCB_VISIBLE_LIST; i++ )
    {
        m_layerColors[ITEM_GAL_LAYER( i )] = m_legacyColorMap[aSettings->GetItemColor( i )];
    }

    // Default colors for specific layers
    m_layerColors[ITEM_GAL_LAYER( VIAS_HOLES_VISIBLE )]         = COLOR4D( 0.5, 0.4, 0.0, 1.0 );
    m_layerColors[ITEM_GAL_LAYER( PADS_HOLES_VISIBLE )]         = COLOR4D( 0.0, 0.5, 0.5, 1.0 );
    m_layerColors[ITEM_GAL_LAYER( VIAS_VISIBLE )]               = COLOR4D( 0.7, 0.7, 0.7, 1.0 );
    m_layerColors[ITEM_GAL_LAYER( PADS_VISIBLE )]               = COLOR4D( 0.7, 0.7, 0.7, 1.0 );
    m_layerColors[ITEM_GAL_LAYER( PADS_NETNAMES_VISIBLE )]      = COLOR4D( 0.8, 0.8, 0.8, 0.7 );
    m_layerColors[ITEM_GAL_LAYER( PAD_FR_NETNAMES_VISIBLE )]    = COLOR4D( 0.8, 0.8, 0.8, 0.7 );
    m_layerColors[ITEM_GAL_LAYER( PAD_BK_NETNAMES_VISIBLE )]    = COLOR4D( 0.8, 0.8, 0.8, 0.7 );

    // Netnames for copper layers
    for( LAYER_NUM layer = FIRST_COPPER_LAYER; layer <= LAST_COPPER_LAYER; ++layer )
    {
        // Quick, dirty hack, netnames layers should be stored in usual layers
        m_layerColors[GetNetnameLayer( layer )] = COLOR4D( 0.8, 0.8, 0.8, 0.7 );
    }

    update();
}


void PCB_RENDER_SETTINGS::LoadDisplayOptions( const DISPLAY_OPTIONS& aOptions )
{
    m_hiContrastEnabled = aOptions.ContrastModeDisplay;
    m_padNumbers        = aOptions.DisplayPadNum;

    // Whether to draw tracks, vias & pads filled or as outlines
    m_sketchModeSelect[PADS_VISIBLE]   = !aOptions.DisplayPadFill;
    m_sketchModeSelect[VIAS_VISIBLE]   = !aOptions.DisplayViaFill;
    m_sketchModeSelect[TRACKS_VISIBLE] = !aOptions.DisplayPcbTrackFill;

    switch( aOptions.DisplayNetNamesMode )
    {
    case 0:
        m_netNamesOnPads = false;
        m_netNamesOnTracks = false;
        break;

    case 1:
        m_netNamesOnPads = true;
        m_netNamesOnTracks = false;
        break;

    case 2:
        m_netNamesOnPads = false;
        m_netNamesOnTracks = true;
        break;

    case 3:
        m_netNamesOnPads = true;
        m_netNamesOnTracks = true;
        break;
    }

    switch( aOptions.DisplayZonesMode )
    {
    case 0:
        m_displayZoneMode = DZ_SHOW_FILLED;
        break;

    case 1:
        m_displayZoneMode = DZ_HIDE_FILLED;
        break;

    case 2:
        m_displayZoneMode = DZ_SHOW_OUTLINED;
        break;
    }
}


void PCB_RENDER_SETTINGS::update()
{
    // Calculate darkened/highlighted variants of layer colors
    for( int i = 0; i < TOTAL_LAYER_COUNT; i++ )
    {
        m_layerColors[i].a   = m_layerOpacity;
        m_layerColorsHi[i]   = m_layerColors[i].Brightened( m_highlightFactor );
        m_layerColorsDark[i] = m_layerColors[i].Darkened( 1.0 - m_highlightFactor );
        m_layerColorsSel[i]  = m_layerColors[i].Brightened( m_selectFactor );
    }

    m_hiContrastColor = COLOR4D( m_hiContrastFactor, m_hiContrastFactor, m_hiContrastFactor,
                                 m_layerOpacity );
}


PCB_PAINTER::PCB_PAINTER( GAL* aGal ) :
    PAINTER( aGal )
{
}


const COLOR4D& PCB_PAINTER::GetColor( const VIEW_ITEM* aItem, int aLayer )
{
    int netCode = -1;

    if( aItem )
    {
        if( static_cast<const EDA_ITEM*>( aItem )->IsSelected() )
        {
            return m_pcbSettings->m_layerColorsSel[aLayer];
        }

        // Try to obtain the netcode for the item
        const BOARD_CONNECTED_ITEM* item = dynamic_cast<const BOARD_CONNECTED_ITEM*>( aItem );
        if( item )
            netCode = item->GetNet();
    }

    // Return grayish color for non-highlighted layers in the high contrast mode
    if( m_pcbSettings->m_hiContrastEnabled && m_pcbSettings->m_activeLayers.count( aLayer ) == 0 )
        return m_pcbSettings->m_hiContrastColor;

    // Single net highlight mode
    if( m_pcbSettings->m_highlightEnabled )
    {
        if( netCode == m_pcbSettings->m_highlightNetcode )
            return m_pcbSettings->m_layerColorsHi[aLayer];
        else
            return m_pcbSettings->m_layerColorsDark[aLayer];
    }

    // No special modificators enabled
    return m_pcbSettings->m_layerColors[aLayer];
}


bool PCB_PAINTER::Draw( const VIEW_ITEM* aItem, int aLayer )
{
    const BOARD_ITEM* item = static_cast<const BOARD_ITEM*>( aItem );

    // the "cast" applied in here clarifies which overloaded draw() is called
    switch( item->Type() )
    {
    case PCB_ZONE_T:
    case PCB_TRACE_T:
        draw( (TRACK*) aItem, aLayer );
        break;

    case PCB_VIA_T:
        draw( (SEGVIA*) aItem, aLayer );
        break;

    case PCB_PAD_T:
        draw( (D_PAD*) aItem, aLayer );
        break;

    case PCB_LINE_T:
    case PCB_MODULE_EDGE_T:
        draw( (DRAWSEGMENT*) aItem );
        break;

    case PCB_MODULE_T:
        draw( (MODULE*) aItem );
        break;

    case PCB_TEXT_T:
        draw( (TEXTE_PCB*) aItem, aLayer );
        break;

    case PCB_MODULE_TEXT_T:
        draw( (TEXTE_MODULE*) aItem, aLayer );
        break;

    case PCB_ZONE_AREA_T:
        draw( (ZONE_CONTAINER*) aItem );
        break;

    case PCB_DIMENSION_T:
        draw( (DIMENSION*) aItem, aLayer );
        break;

    case PCB_TARGET_T:
        draw( (PCB_TARGET*) aItem );
        break;

    default:
        // Painter does not know how to draw the object
        return false;
        break;
    }

    return true;
}


void PCB_PAINTER::draw( const TRACK* aTrack, int aLayer )
{
    VECTOR2D start( aTrack->GetStart() );
    VECTOR2D end( aTrack->GetEnd() );
    int      width = aTrack->GetWidth();
    int      netNumber = aTrack->GetNet();
    COLOR4D  color;

    if( m_pcbSettings->m_netNamesOnTracks && IsNetnameLayer( aLayer ) )
    {
        // If there is a net name - display it on the track
        if( netNumber != 0 )
        {
            VECTOR2D line = ( end - start );
            double length = line.EuclideanNorm();

            // Check if the track is long enough to have a netname displayed
            if( length < 10 * width )
                return;

            NETINFO_ITEM* net = ( (BOARD*) aTrack->GetParent() )->FindNet( netNumber );
            std::string netName = std::string( net->GetShortNetname().mb_str() );
            VECTOR2D textPosition = start + line / 2.0;     // center of the track
            double textOrientation = -atan( line.y / line.x );
            double textSize = std::min( static_cast<double>( width ), length / netName.length() );

            // Set a proper color for the label
            color = GetColor( aTrack, aTrack->GetLayer() );
            COLOR4D labelColor = GetColor( NULL, aLayer );

            if( color.GetBrightness() > 0.5 )
                m_gal->SetStrokeColor( labelColor.Inverted() );
            else
                m_gal->SetStrokeColor( labelColor );

            m_gal->SetLineWidth( width / 10.0 );
            m_gal->SetBold( false );
            m_gal->SetItalic( false );
            m_gal->SetMirrored( false );
            m_gal->SetGlyphSize( VECTOR2D( textSize * 0.7, textSize * 0.7 ) );
            m_gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_CENTER );
            m_gal->SetVerticalJustify( GR_TEXT_VJUSTIFY_CENTER );
            m_gal->StrokeText( netName, textPosition, textOrientation );
        }
    }
    else if( IsCopperLayer( aLayer ))
    {
        // Draw a regular track
        color = GetColor( aTrack, aLayer );
        m_gal->SetStrokeColor( color );
        m_gal->SetIsStroke( true );

        if( m_pcbSettings->m_sketchModeSelect[TRACKS_VISIBLE] )
        {
            // Outline mode
            m_gal->SetLineWidth( m_pcbSettings->m_outlineWidth );
            m_gal->SetIsFill( false );
        }
        else
        {
            // Filled mode
            m_gal->SetFillColor( color );
            m_gal->SetIsFill( true );
        }
        m_gal->DrawSegment( start, end, width );
    }
}


void PCB_PAINTER::draw( const SEGVIA* aVia, int aLayer )
{
    VECTOR2D center( aVia->GetStart() );
    double   radius;
    COLOR4D  color;

    // Choose drawing settings depending on if we are drawing via's pad or hole
    if( aLayer == ITEM_GAL_LAYER( VIAS_VISIBLE ) )
    {
        radius = aVia->GetWidth() / 2.0;
    }
    else if( aLayer == ITEM_GAL_LAYER( VIAS_HOLES_VISIBLE ) )
    {
        radius = aVia->GetDrillValue() / 2.0;
    }
    else
        return;

    color = GetColor( aVia, aLayer );

    if( m_pcbSettings->m_sketchModeSelect[VIAS_VISIBLE] )
    {
        // Outline mode
        m_gal->SetIsFill( false );
        m_gal->SetIsStroke( true );
        m_gal->SetLineWidth( m_pcbSettings->m_outlineWidth );
        m_gal->SetStrokeColor( color );
        m_gal->DrawCircle( center, radius );
    }
    else
    {
        // Filled mode
        m_gal->SetIsFill( true );
        m_gal->SetIsStroke( false );
        m_gal->SetFillColor( color );
        m_gal->DrawCircle( center, radius );
    }
}


void PCB_PAINTER::draw( const D_PAD* aPad, int aLayer )
{
    COLOR4D     color;
    VECTOR2D    size;
    VECTOR2D    position( aPad->GetPosition() );
    PAD_SHAPE_T shape;
    double      m, n;
    double      orientation = aPad->GetOrientation();
    NORMALIZE_ANGLE_90( orientation );  // do not display descriptions upside down
    orientation = orientation * M_PI / 1800.0;

    // Draw description layer
    if( IsNetnameLayer( aLayer ) )
    {
        // Is anything that we can display enabled?
        if( m_pcbSettings->m_netNamesOnPads || m_pcbSettings->m_padNumbers )
        {
            bool displayNetname = ( m_pcbSettings->m_netNamesOnPads &&
                                    !aPad->GetNetname().empty() );
            size = VECTOR2D( aPad->GetSize() / 2 );
            double scale = m_gal->GetZoomFactor();
            double maxSize = PCB_RENDER_SETTINGS::MAX_FONT_SIZE / scale;

            // Font size limits
            if( size.x > maxSize )
                size.x = maxSize;
            if( size.y > maxSize )
                size.y = maxSize;

            // Keep the size ratio for the font, but make it smaller
            if( size.x < size.y )
            {
                orientation -= M_PI / 2.0;
                size.y = size.x * 4.0 / 3.0;
            }
            else if( size.x == size.y )
            {
                // If the text is displayed on a symmetrical pad, do not rotate it
                orientation = 0.0;
            }
            else
            {
                size.x = size.y * 3.0 / 4.0;
            }

            m_gal->Save();
            m_gal->Translate( position );
            m_gal->Rotate( -orientation );

            // Default font settings
            m_gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_CENTER );
            m_gal->SetVerticalJustify( GR_TEXT_VJUSTIFY_CENTER );
            m_gal->SetBold( false );
            m_gal->SetItalic( false );
            m_gal->SetMirrored( false );

            // Set a proper color for the label
            color = GetColor( aPad, aPad->GetLayer() );
            COLOR4D labelColor = GetColor( NULL, aLayer );

            if( color.GetBrightness() > 0.5 )
                m_gal->SetStrokeColor( labelColor.Inverted() );
            else
                m_gal->SetStrokeColor( labelColor );

            if( displayNetname && m_pcbSettings->m_padNumbers )
            {
                // Divide the space, when both pad numbers and netnames are enabled
                size = size / 2.0;
                m_gal->SetGlyphSize( size );
                m_gal->SetLineWidth( size.y / 8.0 );

                m_gal->StrokeText( std::string( aPad->GetNetname().mb_str() ),
                                   VECTOR2D( 0.0, size.y ), 0.0 );

                m_gal->StrokeText( std::string( aPad->GetPadName().mb_str() ),
                                   VECTOR2D( 0.0, -size.y / 2.0 ), 0.0 );
            }
            else
            {
                // There is only one thing to display
                if( displayNetname )
                {
                    m_gal->SetGlyphSize( size / 2.0 );
                    m_gal->SetLineWidth( size.y / 12.0 );
                    m_gal->StrokeText( std::string( aPad->GetNetname().mb_str() ),
                                       VECTOR2D( 0.0, 0.0 ), 0.0 );
                }

                if( m_pcbSettings->m_padNumbers )
                {
                    m_gal->SetGlyphSize( size );
                    m_gal->SetLineWidth( size.y / 10.0 );
                    m_gal->StrokeText( std::string( aPad->GetPadName().mb_str() ),
                                       VECTOR2D( 0.0, 0.0 ), 0.0 );
                }
            }

            m_gal->Restore();
        }
        return;
    }

    // Pad drawing
    color = GetColor( aPad, aLayer );
    if( m_pcbSettings->m_sketchModeSelect[PADS_VISIBLE] )
    {
        // Outline mode
        m_gal->SetIsFill( false );
        m_gal->SetIsStroke( true );
        m_gal->SetLineWidth( m_pcbSettings->m_outlineWidth );
        m_gal->SetStrokeColor( color );
    }
    else
    {
        // Filled mode
        m_gal->SetIsFill( true );
        m_gal->SetIsStroke( false );
        m_gal->SetFillColor( color );
    }

    m_gal->Save();
    m_gal->Translate( VECTOR2D( aPad->GetPosition() ) );
    m_gal->Rotate( -aPad->GetOrientation() * M_PI / 1800.0 );

    // Choose drawing settings depending on if we are drawing a pad itself or a hole
    if( aLayer == ITEM_GAL_LAYER( PADS_HOLES_VISIBLE ) )
    {
        // Drawing hole
        size  = VECTOR2D( aPad->GetDrillSize() ) / 2.0;
        shape = aPad->GetDrillShape();
    }
    else if( aLayer == SOLDERMASK_N_FRONT || aLayer == SOLDERMASK_N_BACK )
    {
        // Drawing soldermask
        int soldermaskMargin = aPad->GetSolderMaskMargin();

        m_gal->Translate( VECTOR2D( aPad->GetOffset() ) );
        size  = VECTOR2D( aPad->GetSize().x / 2.0 + soldermaskMargin,
                          aPad->GetSize().y / 2.0 + soldermaskMargin );
        shape = aPad->GetShape();
    }
    else if( aLayer == SOLDERPASTE_N_FRONT || aLayer == SOLDERPASTE_N_BACK )
    {
        // Drawing solderpaste
        int solderpasteMargin = aPad->GetLocalSolderPasteMargin();

        m_gal->Translate( VECTOR2D( aPad->GetOffset() ) );
        size  = VECTOR2D( aPad->GetSize().x / 2.0 + solderpasteMargin,
                          aPad->GetSize().y / 2.0 + solderpasteMargin );
        shape = aPad->GetShape();
    }
    else
    {
        // Drawing every kind of pad
        m_gal->Translate( VECTOR2D( aPad->GetOffset() ) );
        size  = VECTOR2D( aPad->GetSize() ) / 2.0;
        shape = aPad->GetShape();
    }

    switch( shape )
    {
    case PAD_OVAL:
        if( size.y >= size.x )
        {
            m = ( size.y - size.x );
            n = size.x;

            if( m_pcbSettings->m_sketchModeSelect[PADS_VISIBLE] )
            {
                // Outline mode
                m_gal->DrawArc( VECTOR2D( 0, -m ), n, -M_PI, 0 );
                m_gal->DrawArc( VECTOR2D( 0, m ),  n, M_PI, 0 );
                m_gal->DrawLine( VECTOR2D( -n, -m ), VECTOR2D( -n, m ) );
                m_gal->DrawLine( VECTOR2D( n, -m ),  VECTOR2D( n, m ) );
            }
            else
            {
                // Filled mode
                m_gal->DrawCircle( VECTOR2D( 0, -m ), n );
                m_gal->DrawCircle( VECTOR2D( 0, m ),  n );
                m_gal->DrawRectangle( VECTOR2D( -n, -m ), VECTOR2D( n, m ) );
            }
        }
        else
        {
            m = ( size.x - size.y );
            n = size.y;

            if( m_pcbSettings->m_sketchModeSelect[PADS_VISIBLE] )
            {
                // Outline mode
                m_gal->DrawArc( VECTOR2D( -m, 0 ), n, M_PI / 2, 3 * M_PI / 2 );
                m_gal->DrawArc( VECTOR2D( m, 0 ),  n, M_PI / 2, -M_PI / 2 );
                m_gal->DrawLine( VECTOR2D( -m, -n ), VECTOR2D( m, -n ) );
                m_gal->DrawLine( VECTOR2D( -m, n ),  VECTOR2D( m, n ) );
            }
            else
            {
                // Filled mode
                m_gal->DrawCircle( VECTOR2D( -m, 0 ), n );
                m_gal->DrawCircle( VECTOR2D( m, 0 ),  n );
                m_gal->DrawRectangle( VECTOR2D( -m, -n ), VECTOR2D( m, n ) );
            }
        }
        break;

    case PAD_RECT:
        m_gal->DrawRectangle( VECTOR2D( -size.x, -size.y ), VECTOR2D( size.x, size.y ) );
        break;

    case PAD_TRAPEZOID:
    {
        std::deque<VECTOR2D> pointList;
        wxPoint corners[4];

        VECTOR2D padSize = VECTOR2D( aPad->GetSize().x, aPad->GetSize().y ) / 2;
        VECTOR2D deltaPadSize = size - padSize; // = solder[Paste/Mask]Margin or 0
        VECTOR2D delta = VECTOR2D( aPad->GetDelta().x / 2,
                                   aPad->GetDelta().y / 2 );
        VECTOR2D inflate = VECTOR2D( delta.y * ( deltaPadSize.x / size.x ),
                                     delta.x * ( deltaPadSize.y / size.y ) );

        aPad->BuildPadPolygon( corners, wxSize( deltaPadSize.x, deltaPadSize.y ), 0.0 );
        pointList.push_back( VECTOR2D( corners[0] ) );
        pointList.push_back( VECTOR2D( corners[1] ) );
        pointList.push_back( VECTOR2D( corners[2] ) );
        pointList.push_back( VECTOR2D( corners[3] ) );

        if( m_pcbSettings->m_sketchModeSelect[PADS_VISIBLE] )
        {
            // Add the beginning point to close the outline
            pointList.push_back( pointList.front() );
            m_gal->DrawPolyline( pointList );
        }
        else
        {
            m_gal->DrawPolygon( pointList );
        }
    }
    break;

    case PAD_CIRCLE:
        m_gal->DrawCircle( VECTOR2D( 0.0, 0.0 ), size.x );
        break;

    case PAD_OCTAGON:    // it is not used anywhere, neither you can set it using pcbnew..
    case PAD_NONE:
        break;
    }

    m_gal->Restore();
}


void PCB_PAINTER::draw( const DRAWSEGMENT* aSegment )
{
    COLOR4D color = GetColor( NULL, aSegment->GetLayer() );

    m_gal->SetIsFill( false );
    m_gal->SetIsStroke( true );
    m_gal->SetStrokeColor( color );
    m_gal->SetLineWidth( aSegment->GetWidth() );

    switch( aSegment->GetShape() )
    {
    case S_SEGMENT:
        m_gal->DrawLine( VECTOR2D( aSegment->GetStart() ), VECTOR2D( aSegment->GetEnd() ) );
        break;

    case S_RECT:
        wxASSERT_MSG( false, wxT( "Not tested yet" ) );
        m_gal->DrawRectangle( VECTOR2D( aSegment->GetStart() ), VECTOR2D( aSegment->GetEnd() ) );
        break;

    case S_ARC:
        m_gal->DrawArc( VECTOR2D( aSegment->GetCenter() ), aSegment->GetRadius(),
                        aSegment->GetArcAngleStart() * M_PI / 1800.0,
                        ( aSegment->GetArcAngleStart() + aSegment->GetAngle() ) * M_PI / 1800.0 );
        break;

    case S_CIRCLE:
        m_gal->DrawCircle( VECTOR2D( aSegment->GetCenter() ), aSegment->GetRadius() );
        break;

    case S_POLYGON:
    {
        std::deque<VECTOR2D> pointsList;

        m_gal->SetIsFill( true );
        m_gal->SetIsStroke( false );
        m_gal->SetFillColor( color );

        m_gal->Save();
        m_gal->Translate( VECTOR2D( aSegment->GetPosition() ) );

        MODULE* module = aSegment->GetParentModule();
        if( module )
        {
            m_gal->Rotate( -module->GetOrientation() * M_PI / 1800.0 );
        }

        std::copy( aSegment->GetPolyPoints().begin(), aSegment->GetPolyPoints().end(),
                   std::back_inserter( pointsList ) );

        m_gal->SetLineWidth( aSegment->GetWidth() );
        m_gal->DrawPolyline( pointsList );
        m_gal->DrawPolygon( pointsList );

        m_gal->Restore();
        break;
    }

    case S_CURVE:
        m_gal->DrawCurve( VECTOR2D( aSegment->GetStart() ),
                          VECTOR2D( aSegment->GetBezControl1() ),
                          VECTOR2D( aSegment->GetBezControl2() ),
                          VECTOR2D( aSegment->GetEnd() ) );
        break;

    case S_LAST:
        break;
    }
}


void PCB_PAINTER::draw( const MODULE* aModule )
{
    // For modules we have to draw a selection box if needed
    if( aModule->IsSelected() )
        drawSelectionBox( aModule );
}


void PCB_PAINTER::draw( const TEXTE_PCB* aText, int aLayer )
{
    if( aLayer == ITEM_GAL_LAYER( SELECTION ) )
    {
        if( aText->IsSelected() )
            drawSelectionBox( aText );
    }
    else
    {
        if( aText->GetText().Length() == 0 )
            return;

        COLOR4D  strokeColor = GetColor( NULL, aText->GetLayer() );
        VECTOR2D position( aText->GetTextPosition().x, aText->GetTextPosition().y );
        double   orientation = aText->GetOrientation() * M_PI / 1800.0;

        m_gal->SetStrokeColor( strokeColor );
        m_gal->SetLineWidth( aText->GetThickness() );
        m_gal->SetTextAttributes( aText );
        m_gal->StrokeText( std::string( aText->GetText().mb_str() ), position, orientation );
    }
}


void PCB_PAINTER::draw( const TEXTE_MODULE* aText, int aLayer )
{
    if( aLayer == ITEM_GAL_LAYER( SELECTION ) )
    {
        if( aText->IsSelected() )
            drawSelectionBox( aText );
    }
    else
    {
        if( aText->GetLength() == 0 )
            return;

        COLOR4D  strokeColor = GetColor( NULL, aLayer );
        VECTOR2D position( aText->GetTextPosition().x, aText->GetTextPosition().y);
        double   orientation = aText->GetDrawRotation() * M_PI / 1800.0;

        m_gal->SetStrokeColor( strokeColor );
        m_gal->SetLineWidth( aText->GetThickness() );
        m_gal->SetTextAttributes( aText );
        m_gal->StrokeText( std::string( aText->GetText().mb_str() ), position, orientation );
    }

}


void PCB_PAINTER::draw( const ZONE_CONTAINER* aZone )
{
    COLOR4D color = GetColor( NULL, aZone->GetLayer() );
    std::deque<VECTOR2D> corners;
    PCB_RENDER_SETTINGS::DisplayZonesMode displayMode = m_pcbSettings->m_displayZoneMode;

    // Draw the outline
    m_gal->SetStrokeColor( color );
    m_gal->SetIsFill( false );
    m_gal->SetIsStroke( true );
    m_gal->SetLineWidth( m_pcbSettings->m_outlineWidth );

    const CPolyLine* outline = aZone->Outline();
    for( int i = 0; i < outline->GetCornersCount(); ++i )
    {
        corners.push_back( VECTOR2D( outline->GetPos( i ) ) );
    }
    // The last point for closing the polyline
    corners.push_back( VECTOR2D( outline->GetPos( 0 ) ) );
    m_gal->DrawPolyline( corners );
    corners.clear();

    // Draw the filling
    if( displayMode != PCB_RENDER_SETTINGS::DZ_HIDE_FILLED )
    {
        const std::vector<CPolyPt> polyPoints = aZone->GetFilledPolysList().GetList();
        if( polyPoints.size() == 0 )  // Nothing to draw
            return;

        // Set up drawing options
        m_gal->SetFillColor( color );
        m_gal->SetLineWidth( aZone->GetThermalReliefCopperBridge() / 2.0 );

        if( displayMode == PCB_RENDER_SETTINGS::DZ_SHOW_FILLED )
        {
            m_gal->SetIsFill( true );
            m_gal->SetIsStroke( true );
        }
        else if( displayMode == PCB_RENDER_SETTINGS::DZ_SHOW_OUTLINED )
        {
            m_gal->SetIsFill( false );
            m_gal->SetIsStroke( true );
        }

        std::vector<CPolyPt>::const_iterator polyIterator;
        for( polyIterator = polyPoints.begin(); polyIterator != polyPoints.end(); polyIterator++ )
        {
            // Find out all of polygons and then draw them
            corners.push_back( VECTOR2D( *polyIterator ) );

            if( polyIterator->end_contour )
            {
                if( displayMode == PCB_RENDER_SETTINGS::DZ_SHOW_FILLED )
                {
                    m_gal->DrawPolygon( corners );
                    m_gal->DrawPolyline( corners );
                }
                else if( displayMode == PCB_RENDER_SETTINGS::DZ_SHOW_OUTLINED )
                {
                    m_gal->DrawPolyline( corners );
                }

                corners.clear();
            }
        }
    }
}


void PCB_PAINTER::draw( const DIMENSION* aDimension, int aLayer )
{
    if( aLayer == ITEM_GAL_LAYER( SELECTION ) )
    {
        if( aDimension->IsSelected() )
            drawSelectionBox( aDimension );
    }
    else
    {
        int layer = aDimension->GetLayer();
        COLOR4D strokeColor = GetColor( NULL, layer );

        m_gal->SetStrokeColor( strokeColor );
        m_gal->SetIsFill( false );
        m_gal->SetIsStroke( true );
        m_gal->SetLineWidth( aDimension->GetWidth() );

        // Draw an arrow
        m_gal->DrawLine( VECTOR2D( aDimension->m_crossBarO ), VECTOR2D( aDimension->m_crossBarF ) );
        m_gal->DrawLine( VECTOR2D( aDimension->m_featureLineGO ),
                         VECTOR2D( aDimension->m_featureLineGF ) );
        m_gal->DrawLine( VECTOR2D( aDimension->m_featureLineDO ),
                         VECTOR2D( aDimension->m_featureLineDF ) );
        m_gal->DrawLine( VECTOR2D( aDimension->m_arrowD1O ), VECTOR2D( aDimension->m_arrowD1F ) );
        m_gal->DrawLine( VECTOR2D( aDimension->m_arrowD2O ), VECTOR2D( aDimension->m_arrowD2F ) );
        m_gal->DrawLine( VECTOR2D( aDimension->m_arrowG1O ), VECTOR2D( aDimension->m_arrowG1F ) );
        m_gal->DrawLine( VECTOR2D( aDimension->m_arrowG2O ), VECTOR2D( aDimension->m_arrowG2F ) );

        // Draw text
        draw( &aDimension->Text(), layer );
    }
}


void PCB_PAINTER::draw( const PCB_TARGET* aTarget )
{
    COLOR4D  strokeColor = GetColor( NULL, aTarget->GetLayer() );
    VECTOR2D position( aTarget->GetPosition() );
    double   size, radius;

    m_gal->SetLineWidth( aTarget->GetWidth() );
    m_gal->SetStrokeColor( strokeColor );
    m_gal->SetIsFill( false );
    m_gal->SetIsStroke( true );

    m_gal->Save();
    m_gal->Translate( position );

    if( aTarget->GetShape() )
    {
        // shape x
        m_gal->Rotate( M_PI / 4.0 );
        size   = 2.0 * aTarget->GetSize() / 3.0;
        radius = aTarget->GetSize() / 2.0;
    }
    else
    {
        // shape +
        size   = aTarget->GetSize() / 2.0;
        radius = aTarget->GetSize() / 3.0;
    }

    m_gal->DrawLine( VECTOR2D( -size, 0.0 ),
                     VECTOR2D(  size, 0.0 ) );
    m_gal->DrawLine( VECTOR2D( 0.0, -size ),
                     VECTOR2D( 0.0,  size ) );
    m_gal->DrawCircle( VECTOR2D( 0.0, 0.0 ), radius );

    m_gal->Restore();
}


void PCB_PAINTER::drawSelectionBox( const VIEW_ITEM* aItem ) const
{
    BOX2I boundingBox = aItem->ViewBBox();

    m_gal->SetIsStroke( false );
    m_gal->SetIsFill( true );
    m_gal->SetFillColor( COLOR4D( 1.0, 1.0, 1.0, 0.5 ) );
    m_gal->DrawRectangle( boundingBox.GetOrigin(), boundingBox.GetEnd() );
}
