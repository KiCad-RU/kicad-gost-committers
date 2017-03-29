/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 KiCad Developers, see AUTHORS.txt for contributors.
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

#include "microwave_inductor.h"

#include <wx/wx.h>

#include <base_units.h>
#include <validators.h>
#include <wxPcbStruct.h>

#include <class_pad.h>
#include <class_edge_mod.h>
#include <class_module.h>


using namespace MWAVE;

/**
 * Function  gen_arc
 * generates an arc using arc approximation by lines:
 * Center aCenter
 * Angle "angle" (in 0.1 deg)
 * @param  aBuffer = a buffer to store points.
 * @param  aStartPoint = starting point of arc.
 * @param  aCenter = arc centre.
 * @param  a_ArcAngle = arc length in 0.1 degrees.
 */
static void gen_arc( std::vector <wxPoint>& aBuffer,
                     const wxPoint&         aStartPoint,
                     const wxPoint&         aCenter,
                     int                     a_ArcAngle )
{
    const int SEGM_COUNT_PER_360DEG = 16;
    auto    first_point = aStartPoint - aCenter;
    int     seg_count   = ( ( abs( a_ArcAngle ) ) * SEGM_COUNT_PER_360DEG ) / 3600;

    if( seg_count == 0 )
        seg_count = 1;

    double increment_angle = (double) a_ArcAngle * M_PI / 1800 / seg_count;

    // Creates nb_seg point to approximate arc by segments:
    for( int ii = 1; ii <= seg_count; ii++ )
    {
        double  rot_angle = increment_angle * ii;
        double  fcos = cos( rot_angle );
        double  fsin = sin( rot_angle );
        wxPoint currpt;

        // Rotate current point:
        currpt.x = KiROUND( ( first_point.x * fcos + first_point.y * fsin ) );
        currpt.y = KiROUND( ( first_point.y * fcos - first_point.x * fsin ) );

        auto corner = aCenter + currpt;
        aBuffer.push_back( corner );
    }
}


/**
 * Function BuildCornersList_S_Shape
 * Create a path like a S-shaped coil
 * @param  aBuffer =  a buffer where to store points (ends of segments)
 * @param  aStartPoint = starting point of the path
 * @param  aEndPoint = ending point of the path
 * @param  aLength = full length of the path
 * @param  aWidth = segment width
 */
static int BuildCornersList_S_Shape( std::vector <wxPoint>& aBuffer,
                              const wxPoint& aStartPoint,
                              const wxPoint& aEndPoint,
                              int aLength, int aWidth )
{
/* We must determine:
 * segm_count = number of segments perpendicular to the direction
 * segm_len = length of a strand
 * radius = radius of rounded parts of the coil
 * stubs_len = length of the 2 stubs( segments parallel to the direction)
 *         connecting the start point to the start point of the S shape
 *         and the ending point to the end point of the S shape
 * The equations are (assuming the area size of the entire shape is Size:
 * Size.x = 2 * radius + segm_len
 * Size.y = (segm_count + 2 ) * 2 * radius + 2 * stubs_len
 * inductorPattern.m_length = 2 * delta // connections to the coil
 *             + (segm_count-2) * segm_len      // length of the strands except 1st and last
 *             + (segm_count) * (PI * radius)   // length of rounded
 * segm_len + / 2 - radius * 2)                 // length of 1st and last bit
 *
 * The constraints are:
 * segm_count >= 2
 * radius < m_Size.x
 * Size.y = (radius * 4) + (2 * stubs_len)
 * segm_len > radius * 2
 *
 * The calculation is conducted in the following way:
 * first:
 * segm_count = 2
 * radius = 4 * Size.x (arbitrarily fixed value)
 * Then:
 * Increasing the number of segments to the desired length
 * (radius decreases if necessary)
 */
    wxPoint size;

    // This scale factor adjusts the arc length to handle
    // the arc to segment approximation.
    // because we use SEGM_COUNT_PER_360DEG segment to approximate a circle,
    // the trace len must be corrected when calculated using arcs
    // this factor adjust calculations and must be changed if SEGM_COUNT_PER_360DEG is modified
    // because trace using segment is shorter the corresponding arc
    // ADJUST_SIZE is the ratio between tline len and the arc len for an arc
    // of 360/ADJUST_SIZE angle
    #define ADJUST_SIZE 0.988

    auto    pt       = aEndPoint - aStartPoint;
    double  angle    = -ArcTangente( pt.y, pt.x );
    int     min_len  = KiROUND( EuclideanNorm( pt ) );
    int     segm_len = 0;           // length of segments
    int     full_len;               // full len of shape (sum of length of all segments + arcs)


    /* Note: calculations are made for a vertical coil (more easy calculations)
     * and after points are rotated to their actual position
     * So the main direction is the Y axis.
     * the 2 stubs are on the Y axis
     * the others segments are parallel to the X axis.
     */

    // Calculate the size of area (for a vertical shape)
    size.x = min_len / 2;
    size.y = min_len;

    // Choose a reasonable starting value for the radius of the arcs.
    int radius = std::min( aWidth * 5, size.x / 4 );

    int segm_count;     // number of full len segments
                        // the half size segments (first and last segment) are not counted here
    int stubs_len = 0;  // length of first or last segment (half size of others segments)

    for( segm_count = 0; ; segm_count++ )
    {
        stubs_len = ( size.y - ( radius * 2 * (segm_count + 2 ) ) ) / 2;

        if( stubs_len < size.y / 10 ) // Reduce radius.
        {
            stubs_len = size.y / 10;
            radius    = ( size.y - (2 * stubs_len) ) / ( 2 * (segm_count + 2) );

            if( radius < aWidth ) // Radius too small.
            {
                // Unable to create line: Requested length value is too large for room
                return 0;
            }
        }

        segm_len  = size.x - ( radius * 2 );
        full_len  = 2 * stubs_len;               // Length of coil connections.
        full_len += segm_len * segm_count;       // Length of full length segments.
        full_len += KiROUND( ( segm_count + 2 ) * M_PI * ADJUST_SIZE * radius );    // Ard arcs len
        full_len += segm_len - (2 * radius);     // Length of first and last segments
                                                 // (half size segments len = segm_len/2 - radius).

        if( full_len >= aLength )
            break;
    }

    // Adjust len by adjusting segm_len:
    int delta_size = full_len - aLength;

    // reduce len of the segm_count segments + 2 half size segments (= 1 full size segment)
    segm_len -= delta_size / (segm_count + 1);

    // Generate first line (the first stub) and first arc (90 deg arc)
    pt = aStartPoint;
    aBuffer.push_back( pt );
    pt.y += stubs_len;
    aBuffer.push_back( pt );

    auto centre = pt;
    centre.x -= radius;
    gen_arc( aBuffer, pt, centre, -900 );
    pt = aBuffer.back();

    int half_size_seg_len = segm_len / 2 - radius;

    if( half_size_seg_len )
    {
        pt.x -= half_size_seg_len;
        aBuffer.push_back( pt );
    }

    // Create shape.
    int ii;
    int sign = 1;
    segm_count += 1;    // increase segm_count to create the last half_size segment

    for( ii = 0; ii < segm_count; ii++ )
    {
        int arc_angle;

        if( ii & 1 ) // odd order arcs are greater than 0
            sign = -1;
        else
            sign = 1;

        arc_angle = 1800 * sign;
        centre    = pt;
        centre.y += radius;
        gen_arc( aBuffer, pt, centre, arc_angle );
        pt    = aBuffer.back();
        pt.x += segm_len * sign;
        aBuffer.push_back( pt );
    }

    // The last point is false:
    // it is the end of a full size segment, but must be
    // the end of the second half_size segment. Change it.
    sign *= -1;
    aBuffer.back().x = aStartPoint.x + radius * sign;

    // create last arc
    pt        = aBuffer.back();
    centre    = pt;
    centre.y += radius;
    gen_arc( aBuffer, pt, centre, 900 * sign );
    aBuffer.back();

    // Rotate point
    angle += 900;

    for( unsigned jj = 0; jj < aBuffer.size(); jj++ )
    {
        RotatePoint( &aBuffer[jj].x, &aBuffer[jj].y, aStartPoint.x, aStartPoint.y, angle );
    }

    // push last point (end point)
    aBuffer.push_back( aEndPoint );

    return 1;
}


MODULE* MWAVE::CreateMicrowaveInductor( INDUCTOR_PATTERN& inductorPattern,
                    PCB_EDIT_FRAME* aPcbFrame, wxString& aErrorMessage )
{
    /* Build a microwave inductor footprint.
     * - Length Mself.lng
     * - Extremities Mself.m_Start and Mself.m_End
     * We must determine:
     * Mself.nbrin = number of segments perpendicular to the direction
     * (The coil nbrin will demicercles + 1 + 2 1 / 4 circle)
     * Mself.lbrin = length of a strand
     * Mself.radius = radius of rounded parts of the coil
     * Mself.delta = segments extremities connection between him and the coil even
     *
     * The equations are
     * Mself.m_Size.x = 2 * Mself.radius + Mself.lbrin
     * Mself.m_Size.y * Mself.delta = 2 + 2 * Mself.nbrin * Mself.radius
     * Mself.lng = 2 * Mself.delta / / connections to the coil
     + (Mself.nbrin-2) * Mself.lbrin / / length of the strands except 1st and last
     + (Mself.nbrin 1) * (PI * Mself.radius) / / length of rounded
     * Mself.lbrin + / 2 - Melf.radius * 2) / / length of 1st and last bit
     *
     * The constraints are:
     * Nbrin >= 2
     * Mself.radius < Mself.m_Size.x
     * Mself.m_Size.y = Mself.radius * 4 + 2 * Mself.raccord
     * Mself.lbrin> Mself.radius * 2
     *
     * The calculation is conducted in the following way:
     * Initially:
     * Nbrin = 2
     * Radius = 4 * m_Size.x (arbitrarily fixed value)
     * Then:
     * Increasing the number of segments to the desired length
     * (Radius decreases if necessary)
     */

    D_PAD*   pad;
    int      ll;
    wxString msg;

    auto pt = inductorPattern.m_End - inductorPattern.m_Start;
    int     min_len = KiROUND( EuclideanNorm( pt ) );
    inductorPattern.m_length = min_len;

    // Enter the desired length.
    msg = StringFromValue( g_UserUnit, inductorPattern.m_length );
    wxTextEntryDialog dlg( nullptr, wxEmptyString, _( "Length of Trace:" ), msg );

    if( dlg.ShowModal() != wxID_OK )
        return nullptr; // canceled by user

    msg = dlg.GetValue();
    inductorPattern.m_length = ValueFromString( g_UserUnit, msg );

    // Control values (ii = minimum length)
    if( inductorPattern.m_length < min_len )
    {
        aErrorMessage = _( "Requested length < minimum length" );
        return nullptr;
    }

    // Calculate the elements.
    std::vector <wxPoint> buffer;
    ll = BuildCornersList_S_Shape( buffer, inductorPattern.m_Start,
                                   inductorPattern.m_End, inductorPattern.m_length,
                                   inductorPattern.m_Width );

    if( !ll )
    {
        aErrorMessage = _( "Requested length too large" );
        return nullptr;
    }

    // Generate footprint. the value is also used as footprint name.
    msg = "L";
    wxTextEntryDialog cmpdlg( nullptr, wxEmptyString, _( "Component Value:" ), msg );
    cmpdlg.SetTextValidator( FILE_NAME_CHAR_VALIDATOR( &msg ) );

    if( ( cmpdlg.ShowModal() != wxID_OK ) || msg.IsEmpty() )
        return nullptr;    //  Aborted by user

    MODULE* module = aPcbFrame->CreateNewModule( msg );

    // here the module is already in the BOARD, CreateNewModule() does that.
    module->SetFPID( LIB_ID( std::string( "mw_inductor" ) ) );
    module->SetAttributes( MOD_VIRTUAL | MOD_CMS );
    module->ClearFlags();
    module->SetPosition( inductorPattern.m_End );

    // Generate segments
    for( unsigned jj = 1; jj < buffer.size(); jj++ )
    {
        EDGE_MODULE* PtSegm;
        PtSegm = new EDGE_MODULE( module );
        PtSegm->SetStart( buffer[jj - 1] );
        PtSegm->SetEnd( buffer[jj] );
        PtSegm->SetWidth( inductorPattern.m_Width );
        PtSegm->SetLayer( module->GetLayer() );
        PtSegm->SetShape( S_SEGMENT );
        PtSegm->SetStart0( PtSegm->GetStart() - module->GetPosition() );
        PtSegm->SetEnd0( PtSegm->GetEnd() - module->GetPosition() );
        module->GraphicalItems().PushBack( PtSegm );
    }

    // Place a pad on each end of coil.
    pad = new D_PAD( module );

    module->Pads().PushFront( pad );

    pad->SetPadName( "1" );
    pad->SetPosition( inductorPattern.m_End );
    pad->SetPos0( pad->GetPosition() - module->GetPosition() );

    pad->SetSize( wxSize( inductorPattern.m_Width, inductorPattern.m_Width ) );

    pad->SetLayerSet( LSET( module->GetLayer() ) );
    pad->SetAttribute( PAD_ATTRIB_SMD );
    pad->SetShape( PAD_SHAPE_CIRCLE );

    D_PAD* newpad = new D_PAD( *pad );

    module->Pads().Insert( newpad, pad->Next() );

    pad = newpad;
    pad->SetPadName( "2" );
    pad->SetPosition( inductorPattern.m_Start );
    pad->SetPos0( pad->GetPosition() - module->GetPosition() );

    // Modify text positions.
    wxPoint refPos( ( inductorPattern.m_Start.x + inductorPattern.m_End.x ) / 2,
                    ( inductorPattern.m_Start.y + inductorPattern.m_End.y ) / 2 );

    wxPoint valPos = refPos;

    refPos.y -= module->Reference().GetTextSize().y;
    module->Reference().SetPosition( refPos );
    valPos.y += module->Value().GetTextSize().y;
    module->Value().SetPosition( valPos );

    module->CalculateBoundingBox();
    return module;
}
