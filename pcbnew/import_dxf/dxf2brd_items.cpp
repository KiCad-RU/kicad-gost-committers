/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2013 KiCad Developers, see AUTHORS.txt for contributors.
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

// The DXF reader lib (libdxfrw) comes from LibreCAD project, a 2D CAD program
// libdxfrw can be found on http://sourceforge.net/projects/libdxfrw/
// or (latest sources) on
// https://github.com/LibreCAD/LibreCAD/tree/master/libraries/libdxfrw/src
//
// There is no doc to use it, but have a look to
// https://github.com/LibreCAD/LibreCAD/blob/master/librecad/src/lib/filters/rs_filterdxf.cpp
// and https://github.com/LibreCAD/LibreCAD/blob/master/librecad/src/lib/filters/rs_filterdxf.h
// Each time a dxf entity is read, a "call back" fuction is called
// like void DXF2BRD_CONVERTER::addLine( const DRW_Line& data ) when a line is read.
// this function just add the BOARD entity from dxf parameters (start and end point ...)


#include "libdxfrw.h"
#include "dxf2brd_items.h"
#include <wx/arrstr.h>
#include <wx/regex.h>

#include <trigo.h>
#include <macros.h>
#include <class_board.h>
#include <class_drawsegment.h>
#include <class_pcb_text.h>
#include <convert_from_iu.h>

DXF2BRD_CONVERTER::DXF2BRD_CONVERTER() : DRW_Interface()
{
    m_xOffset   = 0.0;      // X coord offset for conversion (in mm)
    m_yOffset   = 0.0;      // Y coord offset for conversion (in mm)
    m_Dfx2mm    = 1.0;      // The scale factor to convert DXF units to mm
    m_version   = 0;
    m_defaultThickness = 0.1;
    m_brdLayer = Dwgs_User;
}


DXF2BRD_CONVERTER::~DXF2BRD_CONVERTER()
{
}


// coordinate conversions from dxf to internal units
int DXF2BRD_CONVERTER::mapX( double aDxfCoordX )
{
    return Millimeter2iu( m_xOffset + ( aDxfCoordX * m_Dfx2mm ) );
}


int DXF2BRD_CONVERTER::mapY( double aDxfCoordY )
{
    return Millimeter2iu( m_yOffset - ( aDxfCoordY * m_Dfx2mm ) );
}


int DXF2BRD_CONVERTER::mapDim( double aDxfValue )
{
    return Millimeter2iu( aDxfValue * m_Dfx2mm );
}


bool DXF2BRD_CONVERTER::ImportDxfFile( const wxString& aFile )
{
    dxfRW* dxf = new dxfRW( aFile.ToUTF8() );
    bool success = dxf->read( this, true );

    delete dxf;

    return success;
}


/*
 * Implementation of the method which handles layers.
 */
void DXF2BRD_CONVERTER::addLayer( const DRW_Layer& aData )
{
    // Not yet useful in Pcbnew.
#if 0
    wxString name = wxString::FromUTF8( aData.name.c_str() );
    wxLogMessage( name );
#endif
}


/*
 * Import line entities.
 */
void DXF2BRD_CONVERTER::addLine( const DRW_Line& aData )
{
    DRAWSEGMENT* segm = new DRAWSEGMENT;

    segm->SetLayer( ToLAYER_ID( m_brdLayer ) );
    wxPoint start( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );
    segm->SetStart( start );
    wxPoint end( mapX( aData.secPoint.x ), mapY( aData.secPoint.y ) );
    segm->SetEnd( end );
    segm->SetWidth( mapDim( aData.thickness == 0 ? m_defaultThickness : aData.thickness ) );
    m_newItemsList.push_back( segm );
}

void DXF2BRD_CONVERTER::addPolyline(const DRW_Polyline& aData )
{
    // Currently, Pcbnew does not know polylines, for boards.
    // So we have to convert a polyline to a set of segments.
    // Obviously, the z coordinate is ignored

    wxPoint polyline_startpoint;
    wxPoint segment_startpoint;

    for( unsigned ii = 0; ii < aData.vertlist.size(); ii++ )
    {
        DRW_Vertex* vertex = aData.vertlist[ii];

        if( ii == 0 )
        {
            segment_startpoint.x = mapX( vertex->basePoint.x );
            segment_startpoint.y = mapY( vertex->basePoint.y );
            polyline_startpoint  = segment_startpoint;
            continue;
        }

        DRAWSEGMENT*    segm = new DRAWSEGMENT( NULL );

        segm->SetLayer( ToLAYER_ID( m_brdLayer ) );
        segm->SetStart( segment_startpoint );
        wxPoint segment_endpoint( mapX( vertex->basePoint.x ), mapY( vertex->basePoint.y ) );
        segm->SetEnd( segment_endpoint );
        segm->SetWidth( mapDim( aData.thickness == 0 ? m_defaultThickness
                                : aData.thickness ) );
        m_newItemsList.push_back( segm );
        segment_startpoint = segment_endpoint;
    }

    // Polyline flags bit 0 indicates closed (1) or open (0) polyline
    if( aData.flags & 1 )
    {
        DRAWSEGMENT*    closing_segm = new DRAWSEGMENT( NULL );

        closing_segm->SetLayer( ToLAYER_ID( m_brdLayer ) );
        closing_segm->SetStart( segment_startpoint );
        closing_segm->SetEnd( polyline_startpoint );
        closing_segm->SetWidth( mapDim( aData.thickness == 0 ? m_defaultThickness
                                : aData.thickness ) );
        m_newItemsList.push_back( closing_segm );
    }
}

void DXF2BRD_CONVERTER::addLWPolyline(const DRW_LWPolyline& aData )
{
    // Currently, Pcbnew does not know polylines, for boards.
    // So we have to convert a polyline to a set of segments.
    // The import is a simplified import: the width of segment is
    // (obviously constant and is the width of the DRW_LWPolyline.
    // the variable width of each vertex (when exists) is not used.
    wxPoint lwpolyline_startpoint;
    wxPoint segment_startpoint;

    for( unsigned ii = 0; ii < aData.vertlist.size(); ii++ )
    {
        DRW_Vertex2D* vertex = aData.vertlist[ii];

        if( ii == 0 )
        {
            segment_startpoint.x  = mapX( vertex->x );
            segment_startpoint.y  = mapY( vertex->y );
            lwpolyline_startpoint = segment_startpoint;
            continue;
        }

        DRAWSEGMENT*    segm = new DRAWSEGMENT( NULL );

        segm->SetLayer( ToLAYER_ID( m_brdLayer ) );
        segm->SetStart( segment_startpoint );
        wxPoint segment_endpoint( mapX( vertex->x ), mapY( vertex->y ) );
        segm->SetEnd( segment_endpoint );
        segm->SetWidth( mapDim( aData.thickness == 0 ? m_defaultThickness
                                : aData.thickness ) );
        m_newItemsList.push_back( segm );
        segment_startpoint = segment_endpoint;
    }

    // LWPolyline flags bit 0 indicates closed (1) or open (0) polyline
    if( aData.flags & 1 )
    {
        DRAWSEGMENT*    closing_segm = new DRAWSEGMENT( NULL );

        closing_segm->SetLayer( ToLAYER_ID( m_brdLayer ) );
        closing_segm->SetStart( segment_startpoint );
        closing_segm->SetEnd( lwpolyline_startpoint );
        closing_segm->SetWidth( mapDim( aData.thickness == 0 ? m_defaultThickness
                                : aData.thickness ) );
        m_newItemsList.push_back( closing_segm );
    }
}

/*
 * Import Circle entities.
 */
void DXF2BRD_CONVERTER::addCircle( const DRW_Circle& aData )
{
    DRAWSEGMENT* segm = new DRAWSEGMENT;

    segm->SetLayer( ToLAYER_ID( m_brdLayer ) );
    segm->SetShape( S_CIRCLE );
    wxPoint center( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );
    segm->SetCenter( center );
    wxPoint circle_start( mapX( aData.basePoint.x + aData.radious ), mapY( aData.basePoint.y ) );
    segm->SetArcStart( circle_start );
    segm->SetWidth( mapDim( aData.thickness == 0 ? m_defaultThickness : aData.thickness ) );
    m_newItemsList.push_back( segm );
}


/*
 * Import Arc entities.
 */
void DXF2BRD_CONVERTER::addArc( const DRW_Arc& data )
{
    DRAWSEGMENT* segm = new DRAWSEGMENT;

    segm->SetLayer( ToLAYER_ID( m_brdLayer ) );
    segm->SetShape( S_ARC );

    // Init arc centre:
    wxPoint center( mapX( data.basePoint.x ), mapY( data.basePoint.y ) );
    segm->SetCenter( center );

    // Init arc start point
    double  arcStartx   = data.radious;
    double  arcStarty   = 0;
    double  startangle = data.staangle;
    double  endangle = data.endangle;

    RotatePoint( &arcStartx, &arcStarty, -RAD2DECIDEG( startangle ) );
    wxPoint arcStart( mapX( arcStartx + data.basePoint.x ),
                      mapY( arcStarty + data.basePoint.y ) );
    segm->SetArcStart( arcStart );

    // calculate arc angle (arcs are CCW, and should be < 0 in Pcbnew)
    double angle = -RAD2DECIDEG( endangle - startangle );

    if( angle > 0.0 )
        angle -= 3600.0;

    segm->SetAngle( angle );

    segm->SetWidth( mapDim( data.thickness == 0 ? m_defaultThickness : data.thickness ) );
    m_newItemsList.push_back( segm );
}

/**
 * Import texts (TEXT).
 */
void DXF2BRD_CONVERTER::addText( const DRW_Text& aData )
{
    TEXTE_PCB* pcb_text = new TEXTE_PCB( NULL );
    pcb_text->SetLayer( ToLAYER_ID( m_brdLayer ) );

    wxPoint refPoint( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );
    wxPoint secPoint( mapX( aData.secPoint.x ), mapY( aData.secPoint.y ) );

    if( aData.alignV != 0 || aData.alignH != 0 || aData.alignH == DRW_Text::HMiddle )
    {
        if( aData.alignH != DRW_Text::HAligned && aData.alignH != DRW_Text::HFit )
        {
            wxPoint tmp = secPoint;
            secPoint = refPoint;
            refPoint = tmp;
        }
    }

    switch( aData.alignV )
    {
        case DRW_Text::VBaseLine:
            pcb_text->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
            break;

        case DRW_Text::VBottom:
            pcb_text->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
            break;

        case DRW_Text::VMiddle:
            pcb_text->SetVertJustify( GR_TEXT_VJUSTIFY_CENTER );
            break;

        case DRW_Text::VTop:
            pcb_text->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
            break;
    }

    switch( aData.alignH )
    {
        case DRW_Text::HLeft:
            pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
            break;

        case DRW_Text::HCenter:
            pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_CENTER );
            break;

        case DRW_Text::HRight:
            pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
            break;

        case DRW_Text::HAligned:
            // no equivalent options in text pcb.
            pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
            break;

        case DRW_Text::HMiddle:
            // no equivalent options in text pcb.
            pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_CENTER );
            break;

        case DRW_Text::HFit:
            // no equivalent options in text pcb.
            pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
            break;
    }

#if 0
    wxString sty = wxString::FromUTF8(aData.style.c_str());
    sty=sty.ToLower();

    if (aData.textgen==2)
    {
        // Text dir = left to right;
    } else if (aData.textgen==4)
    {
        / Text dir = top to bottom;
    } else
    {
    }
#endif

    wxString text = toNativeString( wxString::FromUTF8( aData.text.c_str() ) );

    pcb_text->SetTextPosition( refPoint );
    pcb_text->SetOrientation( aData.angle * 10 );
    // The 0.8 factor gives a better height/width ratio with our font
    pcb_text->SetWidth( mapDim( aData.height * 0.8 ) );
    pcb_text->SetHeight( mapDim( aData.height ) );
    pcb_text->SetThickness( mapDim( aData.thickness == 0 ? m_defaultThickness : aData.thickness ) );
    pcb_text->SetText( text );

    m_newItemsList.push_back( pcb_text );
}


/**
 * Import multi line texts (MTEXT).
 */
void DXF2BRD_CONVERTER::addMText( const DRW_MText& aData )
{
    wxString    text = toNativeString( wxString::FromUTF8( aData.text.c_str() ) );
    wxString    attrib, tmp;

    /* Some texts start by '\' and have formating chars (font name, font option...)
     *  ending with ';'
     *  Here are some mtext formatting codes:
     *  Format code        Purpose
     * \0...\o            Turns overline on and off
     *  \L...\l            Turns underline on and off
     * \~                 Inserts a nonbreaking space
     \\                 Inserts a backslash
     \\\{...\}            Inserts an opening and closing brace
     \\ \File name;        Changes to the specified font file
     \\ \Hvalue;           Changes to the text height specified in drawing units
     \\ \Hvaluex;          Changes the text height to a multiple of the current text height
     \\ \S...^...;         Stacks the subsequent text at the \, #, or ^ symbol
     \\ \Tvalue;           Adjusts the space between characters, from.75 to 4 times
     \\ \Qangle;           Changes obliquing angle
     \\ \Wvalue;           Changes width factor to produce wide text
     \\ \A                 Sets the alignment value; valid values: 0, 1, 2 (bottom, center, top)    while( text.StartsWith( wxT("\\") ) )
     */
    while( text.StartsWith( wxT( "\\" ) ) )
    {
        attrib << text.BeforeFirst( ';' );
        tmp     = text.AfterFirst( ';' );
        text    = tmp;
    }

    TEXTE_PCB*  pcb_text = new TEXTE_PCB( NULL );
    pcb_text->SetLayer( ToLAYER_ID( m_brdLayer ) );
    wxPoint     textpos( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );
    pcb_text->SetTextPosition( textpos );
    pcb_text->SetOrientation( aData.angle * 10 );
    // The 0.8 factor gives a better height/width ratio with our font
    pcb_text->SetWidth( mapDim( aData.height * 0.8 ) );
    pcb_text->SetHeight( mapDim( aData.height ) );
    pcb_text->SetThickness( mapDim( aData.thickness == 0 ? m_defaultThickness : aData.thickness ) );
    pcb_text->SetText( text );

    // Initialize text justifications:
    if( aData.textgen <= 3 )
    {
        pcb_text->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
    }
    else if( aData.textgen <= 6 )
    {
        pcb_text->SetVertJustify( GR_TEXT_VJUSTIFY_CENTER );
    }
    else
    {
        pcb_text->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
    }

    if( aData.textgen % 3 == 1 )
    {
        pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
    }
    else if( aData.textgen % 3 == 2 )
    {
        pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_CENTER );
    }
    else
    {
        pcb_text->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
    }

#if 0   // These setting have no mening in Pcbnew
    if( data.alignH==1 )
    {
        // Text is left to right;
    }
    else if( data.alignH == 3 )
    {
        // Text is top to bottom;
    }
    else
    {
        // use ByStyle;
    }

    if( aData.alignV==1 )
    {
        // use AtLeast;
    }
    else
    {
        // useExact;
    }
#endif

    m_newItemsList.push_back( pcb_text );
}


/**
 * Sets the header variables from the DXF file.
 */
void DXF2BRD_CONVERTER::addHeader( const DRW_Header* data )
{
    std::map<std::string, DRW_Variant*>::const_iterator it;

    for( it = data->vars.begin(); it != data->vars.end(); ++it )
    {
        std::string key = ( (*it).first ).c_str();

        if( key == "$DWGCODEPAGE" )
        {
            DRW_Variant* var = (*it).second;
            m_codePage = ( *var->content.s );
        }
    }
}


/**
 * Converts a native unicode string into a DXF encoded string.
 *
 * DXF endoding includes the following special sequences:
 * - %%%c for a diameter sign
 * - %%%d for a degree sign
 * - %%%p for a plus/minus sign
 */
wxString DXF2BRD_CONVERTER::toDxfString( const wxString& aStr )
{
    wxString    res;
    int         j = 0;

    for( unsigned i = 0; i<aStr.length(); ++i )
    {
        int c = aStr[i];

        if( c > 175 || c < 11 )
        {
            res.append( aStr.Mid( j, i - j ) );
            j = i;

            switch( c )
            {
            case 0x0A:
                res += wxT( "\\P" );
                break;

                // diameter:
#ifdef __WINDOWS_
            // windows, as always, is special.
            case 0x00D8:
#else
            case 0x2205:
#endif
                res += wxT( "%%C" );
                break;

            // degree:
            case 0x00B0:
                res += wxT( "%%D" );
                break;

            // plus/minus
            case 0x00B1:
                res += wxT( "%%P" );
                break;

            default:
                j--;
                break;
            }

            j++;
        }
    }

    res.append( aStr.Mid( j ) );
    return res;
}


/**
 * Converts a DXF encoded string into a native Unicode string.
 */
wxString DXF2BRD_CONVERTER::toNativeString( const wxString& aData )
{
    wxString    res;

    // Ignore font tags:
    int         j = 0;

    for( unsigned i = 0; i < aData.length(); ++i )
    {
        if( aData[ i ] == 0x7B )                                     // is '{' ?
        {
            if( aData[ i + 1 ] == 0x5c && aData[ i + 2 ] == 0x66 )    // is "\f" ?
            {
                // found font tag, append parsed part
                res.append( aData.Mid( j, i - j ) );

                // skip to ';'
                for( unsigned k = i + 3; k < aData.length(); ++k )
                {
                    if( aData[ k ] == 0x3B )
                    {
                        i = j = ++k;
                        break;
                    }
                }

                // add to '}'
                for( unsigned k = i; k < aData.length(); ++k )
                {
                    if( aData[ k ] == 0x7D )
                    {
                        res.append( aData.Mid( i, k - i ) );
                        i = j = ++k;
                        break;
                    }
                }
            }
        }
    }

    res.append( aData.Mid( j ) );

#if 1
    wxRegEx regexp;
    // Line feed:
    regexp.Compile( wxT( "\\\\P" ) );
    regexp.Replace( &res, wxT( "\n" ) );

    // Space:
    regexp.Compile( wxT( "\\\\~" ) );
    regexp.Replace( &res, wxT( " " ) );

    // diameter:
    regexp.Compile( wxT( "%%[cC]" ) );
#ifdef __WINDOWS__
    // windows, as always, is special.
    regexp.Replace( &res, wxChar( 0xD8 ) );
#else
    // Empty_set, diameter is 0x2300
    regexp.Replace( &res, wxChar( 0x2205 ) );
#endif

    // degree:
    regexp.Compile( wxT( "%%[dD]" ) );
    regexp.Replace( &res, wxChar( 0x00B0 ) );
    // plus/minus
    regexp.Compile( wxT( "%%[pP]" ) );
    regexp.Replace( &res, wxChar( 0x00B1 ) );
#endif

    return res;
}


void DXF2BRD_CONVERTER::addTextStyle( const DRW_Textstyle& aData )
{
    // TODO
}
