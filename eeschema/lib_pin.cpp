/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2009 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file lib_pin.cpp
 */

#include "fctsys.h"
#include "appl_wxstruct.h"
#include "gr_basic.h"
#include "macros.h"
#include "trigo.h"
#include "class_drawpanel.h"
#include "drawtxt.h"
#include "plot_common.h"
#include "wxEeschemaStruct.h"
#include "richio.h"

#include "general.h"
#include "protos.h"
#include "libeditframe.h"
#include "class_libentry.h"
#include "lib_pin.h"
#include "transform.h"
#include "sch_component.h"


/**
 * Note: The following name lists are sentence capitalized per the GNOME UI
 *       standards for list controls.  Please do not change the capitalization
 *       of these strings unless the GNOME UI standards are changed.
 */
static const wxString pin_orientation_names[] =
{
    _( "Right" ),
    _( "Left" ),
    _( "Up" ),
    _( "Down" )
};

// bitmaps to show pins orientations in dialog editor
// must have same order than pin_orientation_names
static const BITMAP_DEF s_icons_Pins_Orientations[] =
{
    pinorient_right_xpm,
    pinorient_left_xpm,
    pinorient_up_xpm,
    pinorient_down_xpm,
};

static const int    pin_orientation_codes[] =
{
    PIN_RIGHT,
    PIN_LEFT,
    PIN_UP,
    PIN_DOWN
};


#define PIN_ORIENTATION_CNT ( sizeof( pin_orientation_names ) / \
                             sizeof( wxString ) )


static const wxString pin_style_names[] =
{
    _( "Line" ),
    _( "Inverted" ),
    _( "Clock" ),
    _( "Inverted clock" ),
    _( "Input low" ),
    _( "Clock low" ),
    _( "Output low" ),
    _( "Falling edge clock" ),
    _( "NonLogic" )
};

// bitmaps to show pins shapes in dialog editor
// must have same order than pin_style_names
static BITMAP_DEF s_icons_Pins_Shapes[] =
{
    pinshape_normal_xpm,
    pinshape_invert_xpm,
    pinshape_clock_normal_xpm,
    pinshape_clock_invert_xpm,
    pinshape_active_low_input_xpm,
    pinshape_clock_active_low_xpm,
    pinshape_active_low_output_xpm,
    pinshape_clock_fall_xpm,
    pinshape_nonlogic_xpm
};


#define PIN_STYLE_CNT ( sizeof( pin_style_names ) / sizeof( wxString ) )


static const int pin_style_codes[] =
{
    NONE,
    INVERT,
    CLOCK,
    CLOCK | INVERT,
    LOWLEVEL_IN,
    LOWLEVEL_IN | CLOCK,
    LOWLEVEL_OUT,
    CLOCK_FALL,
    NONLOGIC
};


static const wxString pin_electrical_type_names[] =
{
    _( "Input" ),
    _( "Output" ),
    _( "Bidirectional" ),
    _( "Tri-state" ),
    _( "Passive" ),
    _( "Unspecified" ),
    _( "Power input" ),
    _( "Power output" ),
    _( "Open collector" ),
    _( "Open emitter" ),
    _( "Not connected" )
};

// bitmaps to show pins electrical type in dialog editor
// must have same order than pin_electrical_type_names
static const BITMAP_DEF s_icons_Pins_Electrical_Type[] =
{
    pintype_input_xpm,
    pintype_output_xpm,
    pintype_bidi_xpm,
    pintype_3states_xpm,
    pintype_passive_xpm,
    pintype_notspecif_xpm,
    pintype_powerinput_xpm,
    pintype_poweroutput_xpm,
    pintype_opencoll_xpm,
    pintype_openemit_xpm,
    pintype_noconnect_xpm
};


#define PIN_ELECTRICAL_TYPE_CNT ( sizeof( pin_electrical_type_names ) / sizeof( wxString ) )


const wxChar* MsgPinElectricType[] =
{
    wxT( "input" ),
    wxT( "output" ),
    wxT( "BiDi" ),
    wxT( "3state" ),
    wxT( "passive" ),
    wxT( "unspc" ),
    wxT( "power_in" ),
    wxT( "power_out" ),
    wxT( "openCol" ),
    wxT( "openEm" ),
    wxT( "NotConnected" ),
    wxT( "?????" )
};


LIB_PIN::LIB_PIN( LIB_COMPONENT* aParent ) :
    LIB_ITEM( LIB_PIN_T, aParent )
{
    m_length = 300;                             /* default Pin len */
    m_orientation = PIN_RIGHT;                  /* Pin orient: Up, Down, Left, Right */
    m_shape = NONE;                             /* Pin shape, bitwise. */
    m_type  = PIN_UNSPECIFIED;                  /* electrical type of pin */
    m_attributes  = 0;                          /* bit 0 != 0: pin invisible */
    m_number      = 0;                          /* pin number ( i.e. 4 codes ASCII ) */
    m_PinNumSize  = 50;
    m_PinNameSize = 50;                         /* Default size for pin name and num */
    m_width    = 0;
    m_typeName = _( "Pin" );
    m_PinNumShapeOpt     = 0;
    m_PinNameShapeOpt    = 0;
    m_PinNumPositionOpt  = 0;
    m_PinNamePositionOpt = 0;
}


LIB_PIN::LIB_PIN( const LIB_PIN& pin ) : LIB_ITEM( pin )
{
    m_position    = pin.m_position;
    m_length      = pin.m_length;
    m_orientation = pin.m_orientation;
    m_shape = pin.m_shape;
    m_type  = pin.m_type;
    m_attributes         = pin.m_attributes;
    m_number             = pin.m_number;
    m_PinNumSize         = pin.m_PinNumSize;
    m_PinNameSize        = pin.m_PinNameSize;
    m_PinNumShapeOpt     = pin.m_PinNumShapeOpt;
    m_PinNameShapeOpt    = pin.m_PinNameShapeOpt;
    m_PinNumPositionOpt  = pin.m_PinNumPositionOpt;
    m_PinNamePositionOpt = pin.m_PinNamePositionOpt;
    m_width = pin.m_width;
    m_name  = pin.m_name;
}


void LIB_PIN::SetName( const wxString& aName )
{
    wxString tmp = ( aName.IsEmpty() ) ? wxT( "~" ) : aName;

    tmp.Replace( wxT( " " ), wxT( "_" ) );

    if( m_name != tmp )
    {
        m_name = tmp;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_name == m_name )
            continue;

        pinList[i]->m_name = m_name;
        SetModified();
    }
}


void LIB_PIN::SetNameTextSize( int size )
{
    if( size != m_PinNameSize )
    {
        m_PinNameSize = size;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_PinNameSize == size )
            continue;

        pinList[i]->m_PinNameSize = size;
        SetModified();
    }
}


void LIB_PIN::SetNumber( const wxString& number )
{
    wxString tmp = ( number.IsEmpty() ) ? wxT( "~" ) : number;

    tmp.Replace( wxT( " " ), wxT( "_" ) );
    long     oldNumber = m_number;
    SetPinNumFromString( tmp );

    if( m_number != oldNumber )
    {
        m_Flags |= IS_CHANGED;
    }

    /* Others pin numbers marked by EnableEditMode() are not modified
     * because each pin has its own number
     */
}


void LIB_PIN::SetNumberTextSize( int size )
{
    if( size != m_PinNumSize )
    {
        m_PinNumSize = size;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_PinNumSize == size )
            continue;

        pinList[i]->m_PinNumSize = size;
        SetModified();
    }
}


void LIB_PIN::SetOrientation( int orientation )
{
    if( m_orientation != orientation )
    {
        m_orientation = orientation;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_orientation == orientation )
            continue;

        pinList[i]->m_orientation = orientation;
        SetModified();
    }
}


void LIB_PIN::SetShape( int aShape )
{
    if( m_shape != aShape )
    {
        m_shape = aShape;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0
           || pinList[i]->m_Convert != m_Convert
           || pinList[i]->m_shape == aShape )
            continue;

        pinList[i]->m_shape = aShape;
        SetModified();
    }
}


void LIB_PIN::SetType( int aType )
{
    if( m_type != aType )
    {
        m_type = aType;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->m_type == aType )
            continue;

        pinList[i]->m_type = aType;
        SetModified();
    }
}


void LIB_PIN::SetLength( int length )
{
    if( m_length != length )
    {
        m_length = length;
        SetModified();
    }

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0
           || pinList[i]->m_Convert != m_Convert
           || pinList[i]->m_length == length )
            continue;

        pinList[i]->m_length = length;
        SetModified();
    }
}


void LIB_PIN::SetPartNumber( int part )
{
    if( m_Unit == part )
        return;

    m_Unit = part;
    SetModified();

    if( m_Unit == 0 )
    {
        LIB_PIN* pin;
        LIB_PIN* tmp = GetParent()->GetNextPin();

        while( tmp != NULL )
        {
            pin = tmp;
            tmp = GetParent()->GetNextPin( pin );

            if( pin->m_Flags == 0 || pin == this
               || ( m_Convert && ( m_Convert != pin->m_Convert ) )
               || ( m_position != pin->m_position )
               || ( pin->m_orientation != m_orientation ) )
                continue;

            GetParent()->RemoveDrawItem( (LIB_ITEM*) pin );
        }
    }
}


void LIB_PIN::SetConversion( int style )
{
    if( m_Convert == style )
        return;

    m_Convert = style;
    m_Flags  |= IS_CHANGED;

    if( style == 0 )
    {
        LIB_PIN* pin;
        LIB_PIN* tmp = GetParent()->GetNextPin();

        while( tmp != NULL )
        {
            pin = tmp;
            tmp = GetParent()->GetNextPin( pin );

            if( ( pin->m_Flags & IS_LINKED ) == 0
               || ( pin == this )
               || ( m_Unit && ( m_Unit != pin->m_Unit ) )
               || ( m_position != pin->m_position )
               || ( pin->m_orientation != m_orientation ) )
                continue;

            GetParent()->RemoveDrawItem( (LIB_ITEM*) pin );
        }
    }
}


void LIB_PIN::SetVisible( bool visible )
{
    if( visible == IsVisible() )
        return;

    if( visible )
        m_attributes &= ~PIN_INVISIBLE;
    else
        m_attributes |= PIN_INVISIBLE;

    SetModified();

    if( GetParent() == NULL )
        return;

    LIB_PINS pinList;
    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( ( pinList[i]->m_Flags & IS_LINKED ) == 0 || pinList[i]->IsVisible() == visible )
            continue;

        if( visible )
            pinList[i]->m_attributes &= ~PIN_INVISIBLE;
        else
            pinList[i]->m_attributes |= PIN_INVISIBLE;

        SetModified();
    }
}


void LIB_PIN::EnableEditMode( bool enable, bool editPinByPin )
{
    LIB_PINS pinList;

    if( GetParent() == NULL )
        return;

    GetParent()->GetPins( pinList );

    for( size_t i = 0; i < pinList.size(); i++ )
    {
        if( pinList[i] == this )
            continue;

        if( ( pinList[i]->m_position == m_position )
           && ( pinList[i]->m_orientation == m_orientation )
           && !IsNew()
           && editPinByPin == false
           && enable )
            pinList[i]->m_Flags |= IS_LINKED | IN_EDIT;
        else
            pinList[i]->m_Flags &= ~( IS_LINKED | IN_EDIT );
    }
}


bool LIB_PIN::HitTest( const wxPoint& aPosition )
{
    return HitTest( aPosition, 0, DefaultTransform );
}


bool LIB_PIN::HitTest( wxPoint aPosition, int aThreshold, const TRANSFORM& aTransform )
{
    if( aThreshold < 0 )
        aThreshold = 0;

    TRANSFORM transform = DefaultTransform;
    DefaultTransform = aTransform;

    EDA_RECT rect = GetBoundingBox();
    rect.Inflate( aThreshold );

    //Restore matrix
    DefaultTransform = transform;

    return rect.Contains( aPosition );
}


bool LIB_PIN::Save( OUTPUTFORMATTER& aFormatter )
{
    wxString StringPinNum;
    int      Etype;

    switch( m_type )
    {
    default:
    case PIN_INPUT:
        Etype = 'I';
        break;

    case PIN_OUTPUT:
        Etype = 'O';
        break;

    case PIN_BIDI:
        Etype = 'B';
        break;

    case PIN_TRISTATE:
        Etype = 'T';
        break;

    case PIN_PASSIVE:
        Etype = 'P';
        break;

    case PIN_UNSPECIFIED:
        Etype = 'U';
        break;

    case PIN_POWER_IN:
        Etype = 'W';
        break;

    case PIN_POWER_OUT:
        Etype = 'w';
        break;

    case PIN_OPENCOLLECTOR:
        Etype = 'C';
        break;

    case PIN_OPENEMITTER:
        Etype = 'E';
        break;

    case PIN_NC:
        Etype = 'N';
        break;
    }

    ReturnPinStringNum( StringPinNum );

    if( StringPinNum.IsEmpty() )
        StringPinNum = wxT( "~" );

    if( !m_name.IsEmpty() )
    {
        if( aFormatter.Print( 0, "X %s", TO_UTF8( m_name ) ) < 0 )
            return false;
    }
    else
    {
        if( aFormatter.Print( 0, "X ~" ) < 0 )
            return false;
    }

    if( aFormatter.Print( 0, " %s %d %d %d %c %d %d %d %d %c",
                          TO_UTF8( StringPinNum ), m_position.x, m_position.y,
                          (int) m_length, (int) m_orientation, m_PinNumSize, m_PinNameSize,
                          m_Unit, m_Convert, Etype ) < 0 )
        return false;

    if( m_shape || !IsVisible() )
    {
        if( aFormatter.Print( 0, " " ) < 0 )
            return false;
    }

    if( !IsVisible() && aFormatter.Print( 0, "N" ) < 0 )
        return false;

    if( m_shape & INVERT && aFormatter.Print( 0, "I" ) < 0 )
        return false;

    if( m_shape & CLOCK && aFormatter.Print( 0, "C" ) < 0 )
        return false;

    if( m_shape & LOWLEVEL_IN && aFormatter.Print( 0, "L" ) < 0 )
        return false;

    if( m_shape & LOWLEVEL_OUT && aFormatter.Print( 0, "V" ) < 0 )
        return false;

    if( m_shape & CLOCK_FALL && aFormatter.Print( 0, "F" ) < 0 )
        return false;

    if( m_shape & NONLOGIC && aFormatter.Print( 0, "X" ) < 0 )
        return false;

    if( aFormatter.Print( 0, "\n" ) < 0 )
        return false;

    m_Flags &= ~IS_CHANGED;

    return true;
}


bool LIB_PIN::Load( LINE_READER& aLineReader, wxString& aErrorMsg )
{
    int  i, j;
    char pinAttrs[64];
    char pinName[256];
    char pinNum[64];
    char pinOrient[64];
    char pinType[64];
    char* line = (char*) aLineReader;

    *pinAttrs = 0;

    i = sscanf( line + 2, "%s %s %d %d %d %s %d %d %d %d %s %s", pinName,
                pinNum, &m_position.x, &m_position.y, &m_length, pinOrient, &m_PinNumSize,
                &m_PinNameSize, &m_Unit, &m_Convert, pinType, pinAttrs );

    if( i < 11 )
    {
        aErrorMsg.Printf( wxT( "pin only had %d parameters of the required 11 or 12" ), i );
        return false;
    }

    m_orientation = pinOrient[0] & 255;
    strncpy( (char*) &m_number, pinNum, 4 );
    m_name = FROM_UTF8( pinName );

    switch( *pinType & 255 )
    {
    case 'I':
        m_type = PIN_INPUT;
        break;

    case 'O':
        m_type = PIN_OUTPUT;
        break;

    case 'B':
        m_type = PIN_BIDI;
        break;

    case 'T':
        m_type = PIN_TRISTATE;
        break;

    case 'P':
        m_type = PIN_PASSIVE;
        break;

    case 'U':
        m_type = PIN_UNSPECIFIED;
        break;

    case 'W':
        m_type = PIN_POWER_IN;
        break;

    case 'w':
        m_type = PIN_POWER_OUT;
        break;

    case 'C':
        m_type = PIN_OPENCOLLECTOR;
        break;

    case 'E':
        m_type = PIN_OPENEMITTER;
        break;

    case 'N':
        m_type = PIN_NC;
        break;

    default:
        aErrorMsg.Printf( wxT( "unknown pin type [%c]" ), *pinType & 255 );
        return false;
    }

    if( i == 12 )       /* Special Symbol defined */
    {
        for( j = strlen( pinAttrs ); j > 0; )
        {
            switch( pinAttrs[--j] )
            {
            case '~':
                break;

            case 'N':
                m_attributes |= PIN_INVISIBLE;
                break;

            case 'I':
                m_shape |= INVERT;
                break;

            case 'C':
                m_shape |= CLOCK;
                break;

            case 'L':
                m_shape |= LOWLEVEL_IN;
                break;

            case 'V':
                m_shape |= LOWLEVEL_OUT;
                break;

            case 'F':
                m_shape |= CLOCK_FALL;
                break;

            case 'X':
                m_shape |= NONLOGIC;
                break;

            default:
                aErrorMsg.Printf( wxT( "unknown pin attribute [%c]" ), pinAttrs[j] );
                return false;
            }
        }
    }

    return true;
}


int LIB_PIN::GetPenSize() const
{
    return ( m_width == 0 ) ? g_DrawDefaultLineThickness : m_width;
}


void LIB_PIN::drawGraphic( EDA_DRAW_PANEL*  aPanel,
                           wxDC*            aDC,
                           const wxPoint&   aOffset,
                           int              aColor,
                           int              aDrawMode,
                           void*            aData,
                           const TRANSFORM& aTransform )
{
    // Invisible pins are only drawn on request.
    // They are drawn in g_InvisibleItemColor.
    // in schematic, they are drawn only if m_ShowAllPins is true.
    // In other windows, they are always drawn because we must see them.
    if( ! IsVisible() )
    {
        EDA_DRAW_FRAME* frame = NULL;
        if( aPanel && aPanel->GetParent() )
            frame = (EDA_DRAW_FRAME*)aPanel->GetParent();

        if( frame && frame->IsType( SCHEMATIC_FRAME ) &&
            ! ((SCH_EDIT_FRAME*)frame)->m_ShowAllPins )
            return;

        aColor = g_InvisibleItemColor;
    }

    LIB_COMPONENT* Entry = GetParent();
    bool           DrawPinText = true;

    if( ( aData != NULL ) && ( (bool*) aData == false ) )
        DrawPinText = false;

    /* Calculate pin orient taking in account the component orientation. */
    int     orient = ReturnPinDrawOrient( aTransform );

    /* Calculate the pin position */
    wxPoint pos1 = aTransform.TransformCoordinate( m_position ) + aOffset;

    /* Drawing from the pin and the special symbol combination */
    DrawPinSymbol( aPanel, aDC, pos1, orient, aDrawMode, aColor );

    if( DrawPinText )
    {
        DrawPinTexts( aPanel, aDC, pos1, orient, Entry->GetPinNameOffset(),
                      Entry->ShowPinNumbers(), Entry->ShowPinNames(),
                      aColor, aDrawMode );
    }

    /* Set to one (1) to draw bounding box around pin to validate bounding
     * box calculation. */
#if 0
    EDA_RECT* clipbox = aPanel ? &aPanel->m_ClipBox : NULL;
    TRANSFORM transform = DefaultTransform;
    DefaultTransform = aTransform;
    EDA_RECT  bBox    = GetBoundingBox();
    bBox.Move( aOffset );
    //Restore matrix
    DefaultTransform = transform;
    GRRect( clipbox, aDC, bBox, 0, LIGHTMAGENTA );
#endif
}


void LIB_PIN::DrawPinSymbol( EDA_DRAW_PANEL* aPanel,
                             wxDC*           aDC,
                             const wxPoint&  aPinPos,
                             int             aOrient,
                             int             aDrawMode,
                             int             aColor )
{
    int       MapX1, MapY1, x1, y1;
    int       color;
    int       width   = GetPenSize();
    int       posX    = aPinPos.x, posY = aPinPos.y, len = m_length;
    EDA_RECT* clipbox = aPanel ? &aPanel->m_ClipBox : NULL;

    color = ReturnLayerColor( LAYER_PIN );

    if( aColor < 0 )       // Used normal color or selected color
    {
        if( (m_Selected & IS_SELECTED) )
            color = g_ItemSelectetColor;
    }
    else
        color = aColor;

    GRSetDrawMode( aDC, aDrawMode );

    MapX1 = MapY1 = 0;
    x1    = posX;
    y1    = posY;

    switch( aOrient )
    {
    case PIN_UP:
        y1    = posY - len;
        MapY1 = 1;
        break;

    case PIN_DOWN:
        y1    = posY + len;
        MapY1 = -1;
        break;

    case PIN_LEFT:
        x1    = posX - len;
        MapX1 = 1;
        break;

    case PIN_RIGHT:
        x1    = posX + len;
        MapX1 = -1;
        break;
    }

    if( m_shape & INVERT )
    {
        GRCircle( clipbox, aDC, MapX1 * INVERT_PIN_RADIUS + x1,
                  MapY1 * INVERT_PIN_RADIUS + y1,
                  INVERT_PIN_RADIUS, width, color );

        GRMoveTo( MapX1 * INVERT_PIN_RADIUS * 2 + x1,
                  MapY1 * INVERT_PIN_RADIUS * 2 + y1 );
        GRLineTo( clipbox, aDC, posX, posY, width, color );
    }
    else if( m_shape & CLOCK_FALL ) /* an alternative for Inverted Clock */
    {
        GRMoveTo( x1 + MapY1 * CLOCK_PIN_DIM,
                  y1 - MapX1 * CLOCK_PIN_DIM );
        GRLineTo( clipbox,
                  aDC,
                  x1 + MapX1 * CLOCK_PIN_DIM,
                  y1 + MapY1 * CLOCK_PIN_DIM,
                  width,
                  color );
        GRLineTo( clipbox,
                  aDC,
                  x1 - MapY1 * CLOCK_PIN_DIM,
                  y1 + MapX1 * CLOCK_PIN_DIM,
                  width,
                  color );
        GRMoveTo( MapX1 * CLOCK_PIN_DIM + x1,
                  MapY1 * CLOCK_PIN_DIM + y1 );
        GRLineTo( clipbox, aDC, posX, posY, width, color );
    }
    else
    {
        GRMoveTo( x1, y1 );
        GRLineTo( clipbox, aDC, posX, posY, width, color );
    }

    if( m_shape & CLOCK )
    {
        if( MapY1 == 0 ) /* MapX1 = +- 1 */
        {
            GRMoveTo( x1, y1 + CLOCK_PIN_DIM );
            GRLineTo( clipbox,
                      aDC,
                      x1 - MapX1 * CLOCK_PIN_DIM,
                      y1,
                      width,
                      color );
            GRLineTo( clipbox,
                      aDC,
                      x1,
                      y1 - CLOCK_PIN_DIM,
                      width,
                      color );
        }
        else    /* MapX1 = 0 */
        {
            GRMoveTo( x1 + CLOCK_PIN_DIM, y1 );
            GRLineTo( clipbox,
                      aDC,
                      x1,
                      y1 - MapY1 * CLOCK_PIN_DIM,
                      width,
                      color );
            GRLineTo( clipbox,
                      aDC,
                      x1 - CLOCK_PIN_DIM,
                      y1,
                      width,
                      color );
        }
    }

    if( m_shape & LOWLEVEL_IN )     /* IEEE symbol "Active Low Input" */
    {
        if( MapY1 == 0 )            /* MapX1 = +- 1 */
        {
            GRMoveTo( x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2, y1 );
            GRLineTo( clipbox,
                      aDC,
                      x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2,
                      y1 - IEEE_SYMBOL_PIN_DIM,
                      width,
                      color );
            GRLineTo( clipbox, aDC, x1, y1, width, color );
        }
        else    /* MapX1 = 0 */
        {
            GRMoveTo( x1, y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2 );
            GRLineTo( clipbox, aDC, x1 - IEEE_SYMBOL_PIN_DIM,
                      y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2, width, color );
            GRLineTo( clipbox, aDC, x1, y1, width, color );
        }
    }


    if( m_shape & LOWLEVEL_OUT )    /* IEEE symbol "Active Low Output" */
    {
        if( MapY1 == 0 )            /* MapX1 = +- 1 */
        {
            GRMoveTo( x1, y1 - IEEE_SYMBOL_PIN_DIM );
            GRLineTo( clipbox,
                      aDC,
                      x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2,
                      y1,
                      width,
                      color );
        }
        else    /* MapX1 = 0 */
        {
            GRMoveTo( x1 - IEEE_SYMBOL_PIN_DIM, y1 );
            GRLineTo( clipbox,
                      aDC,
                      x1,
                      y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2,
                      width,
                      color );
        }
    }
    else if( m_shape & NONLOGIC ) /* NonLogic pin symbol */
    {
        GRMoveTo( x1 - (MapX1 + MapY1) * NONLOGIC_PIN_DIM,
                  y1 - (MapY1 - MapX1) * NONLOGIC_PIN_DIM );
        GRLineTo( clipbox,
                  aDC,
                  x1 + (MapX1 + MapY1) * NONLOGIC_PIN_DIM,
                  y1 + (MapY1 - MapX1) * NONLOGIC_PIN_DIM,
                  width,
                  color );
        GRMoveTo( x1 - (MapX1 - MapY1) * NONLOGIC_PIN_DIM,
                  y1 - (MapY1 + MapX1) * NONLOGIC_PIN_DIM );
        GRLineTo( clipbox,
                  aDC,
                  x1 + (MapX1 - MapY1) * NONLOGIC_PIN_DIM,
                  y1 + (MapY1 + MapX1) * NONLOGIC_PIN_DIM,
                  width,
                  color );
    }

    // Draw the pin end target (active end of the pin)
    BASE_SCREEN* screen = aPanel ? aPanel->GetScreen() : NULL;
    #define NCSYMB_PIN_DIM TARGET_PIN_RADIUS

    if( m_type == PIN_NC )   // Draw a N.C. symbol
    {
        GRLine( clipbox, aDC,
                posX - NCSYMB_PIN_DIM, posY - NCSYMB_PIN_DIM,
                posX + NCSYMB_PIN_DIM, posY + NCSYMB_PIN_DIM,
                width, color );
        GRLine( clipbox, aDC,
                posX + NCSYMB_PIN_DIM, posY - NCSYMB_PIN_DIM,
                posX - NCSYMB_PIN_DIM, posY + NCSYMB_PIN_DIM,
                width, color );
    }
    // Draw but do not print the pin end target 1 pixel width
    else if( screen == NULL || !screen->m_IsPrinting )
    {
        GRCircle( clipbox, aDC, posX, posY, TARGET_PIN_RADIUS, 0, color );
    }
}


void LIB_PIN::DrawPinTexts( EDA_DRAW_PANEL* panel,
                            wxDC*           DC,
                            wxPoint&        pin_pos,
                            int             orient,
                            int             TextInside,
                            bool            DrawPinNum,
                            bool            DrawPinName,
                            int             Color,
                            int             DrawMode )
{
    int        x, y, x1, y1;
    wxString   StringPinNum;
    EDA_Colors NameColor, NumColor;

    wxSize     PinNameSize( m_PinNameSize, m_PinNameSize );
    wxSize     PinNumSize( m_PinNumSize, m_PinNumSize );

    int        nameLineWidth = GetPenSize();

    nameLineWidth = Clamp_Text_PenSize( nameLineWidth, m_PinNameSize, false );
    int        numLineWidth = GetPenSize();
    numLineWidth = Clamp_Text_PenSize( numLineWidth, m_PinNumSize, false );

    GRSetDrawMode( DC, DrawMode );

    /* Get the num and name colors */
    if( (Color < 0) && (m_Selected & IS_SELECTED) )
        Color = g_ItemSelectetColor;

    NameColor = (EDA_Colors) ( Color == -1 ? ReturnLayerColor( LAYER_PINNAM ) : Color );
    NumColor  = (EDA_Colors) ( Color == -1 ? ReturnLayerColor( LAYER_PINNUM ) : Color );

    /* Create the pin num string */
    ReturnPinStringNum( StringPinNum );

    x1 = pin_pos.x;
    y1 = pin_pos.y;

    switch( orient )
    {
    case PIN_UP:
        y1 -= m_length;
        break;

    case PIN_DOWN:
        y1 += m_length;
        break;

    case PIN_LEFT:
        x1 -= m_length;
        break;

    case PIN_RIGHT:
        x1 += m_length;
        break;
    }

    if( m_name.IsEmpty() )
        DrawPinName = FALSE;

    if( TextInside )  /* Draw the text inside, but the pin numbers outside. */
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) )
        {
            // It is an horizontal line
            if( DrawPinName )
            {
                if( orient == PIN_RIGHT )
                {
                    x = x1 + TextInside;
                    DrawGraphicText( panel, DC, wxPoint( x, y1 ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_HORIZ,
                                     PinNameSize,
                                     GR_TEXT_HJUSTIFY_LEFT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );
                }
                else    // Orient == PIN_LEFT
                {
                    x = x1 - TextInside;
                    DrawGraphicText( panel, DC, wxPoint( x, y1 ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_HORIZ,
                                     PinNameSize,
                                     GR_TEXT_HJUSTIFY_RIGHT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );
                }
            }

            if( DrawPinNum )
            {
                DrawGraphicText( panel, DC,
                                 wxPoint( (x1 + pin_pos.x) / 2,
                                         y1 - TXTMARGE ), NumColor,
                                 StringPinNum,
                                 TEXT_ORIENT_HORIZ, PinNumSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_BOTTOM, numLineWidth,
                                 false, false );
            }
        }
        else            /* Its a vertical line. */
        {
            // Text is drawn from bottom to top (i.e. to negative value for Y axis)
            if( orient == PIN_DOWN )
            {
                y = y1 + TextInside;

                if( DrawPinName )
                    DrawGraphicText( panel, DC, wxPoint( x1, y ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_VERT, PinNameSize,
                                     GR_TEXT_HJUSTIFY_RIGHT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );

                if( DrawPinNum )
                    DrawGraphicText( panel, DC,
                                     wxPoint( x1 - TXTMARGE,
                                              (y1 + pin_pos.y) / 2 ), NumColor,
                                     StringPinNum,
                                     TEXT_ORIENT_VERT, PinNumSize,
                                     GR_TEXT_HJUSTIFY_CENTER,
                                     GR_TEXT_VJUSTIFY_BOTTOM, numLineWidth,
                                     false, false );
            }
            else        /* PIN_UP */
            {
                y = y1 - TextInside;

                if( DrawPinName )
                    DrawGraphicText( panel, DC, wxPoint( x1, y ), NameColor,
                                     m_name,
                                     TEXT_ORIENT_VERT, PinNameSize,
                                     GR_TEXT_HJUSTIFY_LEFT,
                                     GR_TEXT_VJUSTIFY_CENTER, nameLineWidth,
                                     false, false );

                if( DrawPinNum )
                    DrawGraphicText( panel, DC,
                                     wxPoint( x1 - TXTMARGE,
                                              (y1 + pin_pos.y) / 2 ), NumColor,
                                     StringPinNum,
                                     TEXT_ORIENT_VERT, PinNumSize,
                                     GR_TEXT_HJUSTIFY_CENTER,
                                     GR_TEXT_VJUSTIFY_BOTTOM, numLineWidth,
                                     false, false );
            }
        }
    }
    else     /**** Draw num & text pin outside  ****/
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) )
        {
            /* Its an horizontal line. */
            if( DrawPinName )
            {
                x = (x1 + pin_pos.x) / 2;
                DrawGraphicText( panel, DC, wxPoint( x, y1 - TXTMARGE ),
                                 NameColor, m_name,
                                 TEXT_ORIENT_HORIZ, PinNameSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_BOTTOM, nameLineWidth,
                                 false, false );
            }
            if( DrawPinNum )
            {
                x = (x1 + pin_pos.x) / 2;
                DrawGraphicText( panel, DC, wxPoint( x, y1 + TXTMARGE ),
                                 NumColor, StringPinNum,
                                 TEXT_ORIENT_HORIZ, PinNumSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_TOP, numLineWidth,
                                 false, false );
            }
        }
        else     /* Its a vertical line. */
        {
            if( DrawPinName )
            {
                y = (y1 + pin_pos.y) / 2;
                DrawGraphicText( panel, DC, wxPoint( x1 - TXTMARGE, y ),
                                 NameColor, m_name,
                                 TEXT_ORIENT_VERT, PinNameSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_BOTTOM, nameLineWidth,
                                 false, false );
            }

            if( DrawPinNum )
            {
                DrawGraphicText( panel, DC,
                                 wxPoint( x1 + TXTMARGE, (y1 + pin_pos.y) / 2 ),
                                 NumColor, StringPinNum,
                                 TEXT_ORIENT_VERT, PinNumSize,
                                 GR_TEXT_HJUSTIFY_CENTER,
                                 GR_TEXT_VJUSTIFY_TOP, numLineWidth,
                                 false, false );
            }
        }
    }
}


void LIB_PIN::PlotSymbol( PLOTTER* aPlotter, const wxPoint& aPosition, int aOrientation )
{
    int        MapX1, MapY1, x1, y1;
    EDA_Colors color = UNSPECIFIED_COLOR;

    color = ReturnLayerColor( LAYER_PIN );

    aPlotter->set_color( color );

    MapX1 = MapY1 = 0;
    x1 = aPosition.x; y1 = aPosition.y;

    switch( aOrientation )
    {
    case PIN_UP:
        y1 = aPosition.y - m_length;
        MapY1 = 1;
        break;

    case PIN_DOWN:
        y1 = aPosition.y + m_length;
        MapY1 = -1;
        break;

    case PIN_LEFT:
        x1 = aPosition.x - m_length;
        MapX1 = 1;
        break;

    case PIN_RIGHT:
        x1 = aPosition.x + m_length;
        MapX1 = -1;
        break;
    }

    if( m_shape & INVERT )
    {
        aPlotter->circle( wxPoint( MapX1 * INVERT_PIN_RADIUS + x1,
                                   MapY1 * INVERT_PIN_RADIUS + y1 ),
                          INVERT_PIN_RADIUS * 2, // diameter
                          NO_FILL,               // fill
                          -1 );                  // width

        aPlotter->move_to( wxPoint( MapX1 * INVERT_PIN_RADIUS * 2 + x1,
                                    MapY1 * INVERT_PIN_RADIUS * 2 + y1 ) );
        aPlotter->finish_to( aPosition );
    }
    else
    {
        aPlotter->move_to( wxPoint( x1, y1 ) );
        aPlotter->finish_to( aPosition );
    }

    if( m_shape & CLOCK )
    {
        if( MapY1 == 0 ) /* MapX1 = +- 1 */
        {
            aPlotter->move_to( wxPoint( x1, y1 + CLOCK_PIN_DIM ) );
            aPlotter->line_to( wxPoint( x1 - MapX1 * CLOCK_PIN_DIM, y1 ) );
            aPlotter->finish_to( wxPoint( x1, y1 - CLOCK_PIN_DIM ) );
        }
        else    /* MapX1 = 0 */
        {
            aPlotter->move_to( wxPoint( x1 + CLOCK_PIN_DIM, y1 ) );
            aPlotter->line_to( wxPoint( x1, y1 - MapY1 * CLOCK_PIN_DIM ) );
            aPlotter->finish_to( wxPoint( x1 - CLOCK_PIN_DIM, y1 ) );
        }
    }

    if( m_shape & LOWLEVEL_IN )   /* IEEE symbol "Active Low Input" */
    {
        if( MapY1 == 0 )        /* MapX1 = +- 1 */
        {
            aPlotter->move_to( wxPoint( x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2, y1 ) );
            aPlotter->line_to( wxPoint( x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2,
                                        y1 - IEEE_SYMBOL_PIN_DIM ) );
            aPlotter->finish_to( wxPoint( x1, y1 ) );
        }
        else    /* MapX1 = 0 */
        {
            aPlotter->move_to( wxPoint( x1, y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2 ) );
            aPlotter->line_to( wxPoint( x1 - IEEE_SYMBOL_PIN_DIM,
                                        y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2 ) );
            aPlotter->finish_to( wxPoint( x1, y1 ) );
        }
    }


    if( m_shape & LOWLEVEL_OUT )  /* IEEE symbol "Active Low Output" */
    {
        if( MapY1 == 0 )        /* MapX1 = +- 1 */
        {
            aPlotter->move_to( wxPoint( x1, y1 - IEEE_SYMBOL_PIN_DIM ) );
            aPlotter->finish_to( wxPoint( x1 + MapX1 * IEEE_SYMBOL_PIN_DIM * 2, y1 ) );
        }
        else    /* MapX1 = 0 */
        {
            aPlotter->move_to( wxPoint( x1 - IEEE_SYMBOL_PIN_DIM, y1 ) );
            aPlotter->finish_to( wxPoint( x1, y1 + MapY1 * IEEE_SYMBOL_PIN_DIM * 2 ) );
        }
    }
}


void LIB_PIN::PlotPinTexts( PLOTTER* plotter,
                            wxPoint& pin_pos,
                            int      orient,
                            int      TextInside,
                            bool     DrawPinNum,
                            bool     DrawPinName,
                            int      aWidth )
{
    int        x, y, x1, y1;
    wxString   StringPinNum;
    EDA_Colors NameColor, NumColor;
    wxSize     PinNameSize = wxSize( m_PinNameSize, m_PinNameSize );
    wxSize     PinNumSize  = wxSize( m_PinNumSize, m_PinNumSize );

    /* Get the num and name colors */
    NameColor = ReturnLayerColor( LAYER_PINNAM );
    NumColor  = ReturnLayerColor( LAYER_PINNUM );

    /* Create the pin num string */
    ReturnPinStringNum( StringPinNum );
    x1 = pin_pos.x;
    y1 = pin_pos.y;

    switch( orient )
    {
    case PIN_UP:
        y1 -= m_length;
        break;

    case PIN_DOWN:
        y1 += m_length;
        break;

    case PIN_LEFT:
        x1 -= m_length;
        break;

    case PIN_RIGHT:
        x1 += m_length;
        break;
    }

    if( m_name.IsEmpty() )
        DrawPinName = FALSE;

    /* Draw the text inside, but the pin numbers outside. */
    if( TextInside )
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) ) /* Its an horizontal line. */
        {
            if( DrawPinName )
            {
                if( orient == PIN_RIGHT )
                {
                    x = x1 + TextInside;
                    plotter->text( wxPoint( x, y1 ), NameColor,
                                   m_name,
                                   TEXT_ORIENT_HORIZ,
                                   PinNameSize,
                                   GR_TEXT_HJUSTIFY_LEFT,
                                   GR_TEXT_VJUSTIFY_CENTER,
                                   aWidth, false, false );
                }
                else    // orient == PIN_LEFT
                {
                    x = x1 - TextInside;

                    if( DrawPinName )
                        plotter->text( wxPoint( x, y1 ),
                                       NameColor, m_name, TEXT_ORIENT_HORIZ,
                                       PinNameSize,
                                       GR_TEXT_HJUSTIFY_RIGHT,
                                       GR_TEXT_VJUSTIFY_CENTER,
                                       aWidth, false, false );
                }
            }
            if( DrawPinNum )
            {
                plotter->text( wxPoint( (x1 + pin_pos.x) / 2, y1 - TXTMARGE ),
                               NumColor, StringPinNum,
                               TEXT_ORIENT_HORIZ, PinNumSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_BOTTOM,
                               aWidth, false, false );
            }
        }
        else         /* Its a vertical line. */
        {
            if( orient == PIN_DOWN )
            {
                y = y1 + TextInside;

                if( DrawPinName )
                    plotter->text( wxPoint( x1, y ), NameColor,
                                   m_name,
                                   TEXT_ORIENT_VERT, PinNameSize,
                                   GR_TEXT_HJUSTIFY_RIGHT,
                                   GR_TEXT_VJUSTIFY_CENTER,
                                   aWidth, false, false );

                if( DrawPinNum )
                {
                    plotter->text( wxPoint( x1 - TXTMARGE, (y1 + pin_pos.y) / 2 ),
                                   NumColor, StringPinNum,
                                   TEXT_ORIENT_VERT, PinNumSize,
                                   GR_TEXT_HJUSTIFY_CENTER,
                                   GR_TEXT_VJUSTIFY_BOTTOM,
                                   aWidth, false, false );
                }
            }
            else        /* PIN_UP */
            {
                y = y1 - TextInside;

                if( DrawPinName )
                    plotter->text( wxPoint( x1, y ), NameColor,
                                   m_name,
                                   TEXT_ORIENT_VERT, PinNameSize,
                                   GR_TEXT_HJUSTIFY_LEFT,
                                   GR_TEXT_VJUSTIFY_CENTER,
                                   aWidth, false, false );

                if( DrawPinNum )
                {
                    plotter->text( wxPoint( x1 - TXTMARGE, (y1 + pin_pos.y) / 2 ),
                                   NumColor, StringPinNum,
                                   TEXT_ORIENT_VERT, PinNumSize,
                                   GR_TEXT_HJUSTIFY_CENTER,
                                   GR_TEXT_VJUSTIFY_BOTTOM,
                                   aWidth, false, false );
                }
            }
        }
    }
    else     /* Draw num & text pin outside */
    {
        if( (orient == PIN_LEFT) || (orient == PIN_RIGHT) )
        {
            /* Its an horizontal line. */
            if( DrawPinName )
            {
                x = (x1 + pin_pos.x) / 2;
                plotter->text( wxPoint( x, y1 - TXTMARGE ),
                               NameColor, m_name,
                               TEXT_ORIENT_HORIZ, PinNameSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_BOTTOM,
                               aWidth, false, false );
            }

            if( DrawPinNum )
            {
                x = ( x1 + pin_pos.x ) / 2;
                plotter->text( wxPoint( x, y1 + TXTMARGE ),
                               NumColor, StringPinNum,
                               TEXT_ORIENT_HORIZ, PinNumSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_TOP,
                               aWidth, false, false );
            }
        }
        else     /* Its a vertical line. */
        {
            if( DrawPinName )
            {
                y = ( y1 + pin_pos.y ) / 2;
                plotter->text( wxPoint( x1 - TXTMARGE, y ),
                               NameColor, m_name,
                               TEXT_ORIENT_VERT, PinNameSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_BOTTOM,
                               aWidth, false, false );
            }

            if( DrawPinNum )
            {
                plotter->text( wxPoint( x1 + TXTMARGE, ( y1 + pin_pos.y ) / 2 ),
                               NumColor, StringPinNum,
                               TEXT_ORIENT_VERT, PinNumSize,
                               GR_TEXT_HJUSTIFY_CENTER,
                               GR_TEXT_VJUSTIFY_TOP,
                               aWidth, false, false );
            }
        }
    }
}


wxPoint LIB_PIN::ReturnPinEndPoint() const
{
    wxPoint pos = m_position;

    switch( m_orientation )
    {
    case PIN_UP:
        pos.y += m_length;
        break;

    case PIN_DOWN:
        pos.y -= m_length;
        break;

    case PIN_LEFT:
        pos.x -= m_length;
        break;

    case PIN_RIGHT:
        pos.x += m_length;
        break;
    }

    return pos;
}


int LIB_PIN::ReturnPinDrawOrient( const TRANSFORM& aTransform ) const
{
    int     orient;
    wxPoint end;   // position of pin end starting at 0,0 according to its orientation, length = 1

    switch( m_orientation )
    {
    case PIN_UP:
        end.y = 1;
        break;

    case PIN_DOWN:
        end.y = -1;
        break;

    case PIN_LEFT:
        end.x = -1;
        break;

    case PIN_RIGHT:
        end.x = 1;
        break;
    }

    // = pos of end point, according to the component orientation
    end    = aTransform.TransformCoordinate( end );
    orient = PIN_UP;

    if( end.x == 0 )
    {
        if( end.y > 0 )
            orient = PIN_DOWN;
    }
    else
    {
        orient = PIN_RIGHT;

        if( end.x < 0 )
            orient = PIN_LEFT;
    }

    return orient;
}


void LIB_PIN::ReturnPinStringNum( wxString& aStringBuffer ) const
{
    aStringBuffer = ReturnPinStringNum( m_number );
}


wxString LIB_PIN::ReturnPinStringNum( long aPinNum )
{
    char ascii_buf[5];

    memcpy( ascii_buf, &aPinNum, 4 );
    ascii_buf[4] = 0;

    wxString buffer = FROM_UTF8( ascii_buf );

    return buffer;
}


void LIB_PIN::SetPinNumFromString( wxString& buffer )
{
    char     ascii_buf[4];
    unsigned ii, len = buffer.Len();

    ascii_buf[0] = ascii_buf[1] = ascii_buf[2] = ascii_buf[3] = 0;

    if( len > 4 )
        len = 4;

    for( ii = 0; ii < len; ii++ )
    {
        ascii_buf[ii]  = buffer.GetChar( ii );
        ascii_buf[ii] &= 0xFF;
    }

    strncpy( (char*) &m_number, ascii_buf, 4 );
}


EDA_ITEM* LIB_PIN::doClone() const
{
    return new LIB_PIN( *this );
}


int LIB_PIN::DoCompare( const LIB_ITEM& other ) const
{
    wxASSERT( other.Type() == LIB_PIN_T );

    const LIB_PIN* tmp = (LIB_PIN*) &other;

    if( m_number != tmp->m_number )
        return m_number - tmp->m_number;

    int result = m_name.CmpNoCase( tmp->m_name );

    if( result != 0 )
        return result;

    if( m_position.x != tmp->m_position.x )
        return m_position.x - tmp->m_position.x;

    if( m_position.y != tmp->m_position.y )
        return m_position.y - tmp->m_position.y;

    return 0;
}


void LIB_PIN::DoOffset( const wxPoint& offset )
{
    m_position += offset;
}


bool LIB_PIN::DoTestInside( EDA_RECT& rect ) const
{
    wxPoint end = ReturnPinEndPoint();

    return rect.Contains( m_position.x, -m_position.y ) || rect.Contains( end.x, -end.y );
}


void LIB_PIN::DoMove( const wxPoint& newPosition )
{
    if( m_position != newPosition )
    {
        m_position = newPosition;
        SetModified();
    }
}


void LIB_PIN::DoMirrorHorizontal( const wxPoint& center )
{
    m_position.x -= center.x;
    m_position.x *= -1;
    m_position.x += center.x;

    if( m_orientation == PIN_RIGHT )
        m_orientation = PIN_LEFT;
    else if( m_orientation == PIN_LEFT )
        m_orientation = PIN_RIGHT;
}

void LIB_PIN::DoMirrorVertical( const wxPoint& center )
{
    m_position.y -= center.y;
    m_position.y *= -1;
    m_position.y += center.y;

    if( m_orientation == PIN_UP )
        m_orientation = PIN_DOWN;
    else if( m_orientation == PIN_DOWN )
        m_orientation = PIN_UP;
}

void LIB_PIN::DoRotate( const wxPoint& center, bool aRotateCCW )
{
    int rot_angle = aRotateCCW ? -900 : 900;

    RotatePoint( &m_position, center, rot_angle );

    if( aRotateCCW )
    {
        switch( m_orientation )
        {
            case PIN_RIGHT:
                m_orientation = PIN_UP;
                break;

            case PIN_UP:
                m_orientation = PIN_LEFT;
                break;
            case PIN_LEFT:
                m_orientation = PIN_DOWN;
                break;

            case PIN_DOWN:
                m_orientation = PIN_RIGHT;
                break;
        }
    }
    else
    {
        switch( m_orientation )
        {
            case PIN_RIGHT:
                m_orientation = PIN_DOWN;
                break;

            case PIN_UP:
                m_orientation = PIN_RIGHT;
                break;
            case PIN_LEFT:
                m_orientation = PIN_UP;
                break;

            case PIN_DOWN:
                m_orientation = PIN_LEFT;
                break;
        }
    }
}


void LIB_PIN::DoPlot( PLOTTER* plotter, const wxPoint& offset, bool fill,
                      const TRANSFORM& aTransform )
{
    if( ! IsVisible() )
        return;

    int     orient = ReturnPinDrawOrient( aTransform );

    wxPoint pos = aTransform.TransformCoordinate( m_position ) + offset;

    plotter->set_current_line_width( GetPenSize() );
    PlotSymbol( plotter, pos, orient );
    PlotPinTexts( plotter, pos, orient, GetParent()->GetPinNameOffset(),
                  GetParent()->ShowPinNumbers(), GetParent()->ShowPinNames(),
                  GetPenSize() );
}


void LIB_PIN::DoSetWidth( int aWidth )
{
    if( m_width != aWidth )
    {
        m_width = aWidth;
        SetModified();
    }
}


void LIB_PIN::DisplayInfo( EDA_DRAW_FRAME* aFrame )
{
    wxString Text;

    LIB_ITEM::DisplayInfo( aFrame );

    aFrame->AppendMsgPanel( _( "Name" ), m_name, DARKCYAN );

    if( m_number == 0 )
        Text = wxT( "?" );
    else
        ReturnPinStringNum( Text );

    aFrame->AppendMsgPanel( _( "Number" ), Text, DARKCYAN );

    aFrame->AppendMsgPanel( _( "Type" ),
                           wxGetTranslation( pin_electrical_type_names[ m_type ] ),
                           RED );
    Text = wxGetTranslation( pin_style_names[ GetStyleCodeIndex( m_shape ) ] );
    aFrame->AppendMsgPanel( _( "Style" ), Text, BLUE );

    if( IsVisible() )
        Text = _( "Yes" );
    else
        Text = _( "No" );

    aFrame->AppendMsgPanel( _( "Visible" ), Text, DARKGREEN );

    /* Display pin length */
    Text = ReturnStringFromValue( g_UserUnit, m_length, EESCHEMA_INTERNAL_UNIT, true );
    aFrame->AppendMsgPanel( _( "Length" ), Text, MAGENTA );

    Text = wxGetTranslation( pin_orientation_names[ GetOrientationCodeIndex( m_orientation ) ] );
    aFrame->AppendMsgPanel( _( "Orientation" ), Text, DARKMAGENTA );
}


EDA_RECT LIB_PIN::GetBoundingBox() const
{
    LIB_COMPONENT* entry = (LIB_COMPONENT*) m_Parent;
    EDA_RECT       bbox;
    wxPoint        begin;
    wxPoint        end;
    int            nameTextOffset = 0;
    bool           showName = !m_name.IsEmpty() && (m_name != wxT( "~" ));
    bool           showNum = m_number != 0;
    int            minsizeV = TARGET_PIN_RADIUS;


    if( entry )
    {
        if( entry->ShowPinNames() )
            nameTextOffset = entry->GetPinNameOffset();
        else
            showName = false;

        showNum = entry->ShowPinNumbers();
    }

    // First, calculate boundary box corners position
    int numberTextLength = showNum ? m_PinNumSize * GetNumberString().Len() : 0;

    // Actual text height is bigger than text size
    int numberTextHeight  = showNum ? wxRound( m_PinNumSize * 1.1 ) : 0;

    if( m_shape & INVERT )
        minsizeV = MAX( TARGET_PIN_RADIUS, INVERT_PIN_RADIUS );

    // calculate top left corner position
    // for the default pin orientation (PIN_RIGHT)
    begin.y = MAX( minsizeV, numberTextHeight + TXTMARGE );
    begin.x = MIN( -TARGET_PIN_RADIUS, m_length - (numberTextLength / 2) );

    // calculate bottom right corner position and adjust top left corner position
    int nameTextLength = 0;
    int nameTextHeight = 0;

    if( showName )
    {
        int length = m_name.Len();

        // Don't count the line over text symbol.
        if( m_name.Left( 1 ) == wxT( "~" ) )
            length -= 1;

        nameTextLength = ( m_PinNameSize * length ) + nameTextOffset;
        // Actual text height are bigger than text size
        nameTextHeight = wxRound( m_PinNameSize * 1.1 ) + TXTMARGE;
    }

    if( nameTextOffset )        // for values > 0, pin name is inside the body
    {
        end.x = m_length + nameTextLength;
        end.y = MIN( -minsizeV, -nameTextHeight / 2 );
    }
    else        // if value == 0:
                // pin name is outside the body, and above the pin line
                // pin num is below the pin line
    {
        end.x = MAX(m_length, nameTextLength);
        end.y = -begin.y;
        begin.y = MAX( minsizeV, nameTextHeight );
    }

    // Now, calculate boundary box corners position for the actual pin orientation
    int orient = ReturnPinDrawOrient( DefaultTransform );

    /* Calculate the pin position */
    switch( orient )
    {
    case PIN_UP:
        // Pin is rotated and texts positions are mirrored
        RotatePoint( &begin, wxPoint( 0, 0 ), -900 );
        RotatePoint( &end, wxPoint( 0, 0 ), -900 );
        break;

    case PIN_DOWN:
        RotatePoint( &begin, wxPoint( 0, 0 ), 900 );
        RotatePoint( &end, wxPoint( 0, 0 ), 900 );
        NEGATE( begin.x );
        NEGATE( end.x );
        break;

    case PIN_LEFT:
        NEGATE( begin.x );
        NEGATE( end.x );
        break;

    case PIN_RIGHT:
        break;
    }

    // Draw Y axis is reversed in schematic:
    NEGATE( begin.y );
    NEGATE( end.y );

    wxPoint pos1 = DefaultTransform.TransformCoordinate( m_position );
    begin += pos1;
    end += pos1;

    bbox.SetOrigin( begin );
    bbox.SetEnd( end );
    bbox.Normalize();
    bbox.Inflate( GetPenSize() / 2 );

    return bbox;
}


wxArrayString LIB_PIN::GetOrientationNames( void )
{
    wxArrayString tmp;

    for( unsigned ii = 0; ii < PIN_ORIENTATION_CNT; ii++ )
        tmp.Add( wxGetTranslation( pin_orientation_names[ii] ) );

    return tmp;
}


int LIB_PIN::GetOrientationCode( int index )
{
    if( index >= 0 && index < (int) PIN_ORIENTATION_CNT )
        return pin_orientation_codes[ index ];

    return PIN_RIGHT;
}


int LIB_PIN::GetOrientationCodeIndex( int code )
{
    size_t i;

    for( i = 0; i < PIN_ORIENTATION_CNT; i++ )
    {
        if( pin_orientation_codes[i] == code )
            return (int) i;
    }

    return wxNOT_FOUND;
}


void LIB_PIN::Rotate()
{
    int orient = PIN_RIGHT;

    switch( GetOrientation() )
    {
        case PIN_UP:
            orient = PIN_LEFT;
            break;

        case PIN_DOWN:
            orient = PIN_RIGHT;
            break;

        case PIN_LEFT:
            orient = PIN_DOWN;
           break;

        case PIN_RIGHT:
            orient = PIN_UP;
            break;
    }

    // Set the new orientation
    SetOrientation( orient );
}


wxArrayString LIB_PIN::GetStyleNames( void )
{
    wxArrayString tmp;

    for( unsigned ii = 0; ii < PIN_STYLE_CNT; ii++ )
        tmp.Add( wxGetTranslation( pin_style_names[ii] ) );

    return tmp;
}


int LIB_PIN::GetStyleCode( int index )
{
    if( index >= 0 && index < (int) PIN_STYLE_CNT )
        return pin_style_codes[ index ];

    return NONE;
}


int LIB_PIN::GetStyleCodeIndex( int code )
{
    size_t i;

    for( i = 0; i < PIN_STYLE_CNT; i++ )
    {
        if( pin_style_codes[i] == code )
            return (int) i;
    }

    return wxNOT_FOUND;
}


wxArrayString LIB_PIN::GetElectricalTypeNames( void )
{
    wxArrayString tmp;

    for( unsigned ii = 0; ii < PIN_ELECTRICAL_TYPE_CNT; ii++ )
        tmp.Add( wxGetTranslation( pin_electrical_type_names[ii] ) );

    return tmp;
}


const BITMAP_DEF* LIB_PIN::GetElectricalTypeSymbols()
{
    return s_icons_Pins_Electrical_Type;
}


const BITMAP_DEF* LIB_PIN::GetOrientationSymbols()
{
    return s_icons_Pins_Orientations;
}


const BITMAP_DEF* LIB_PIN::GetStyleSymbols()
{
    return s_icons_Pins_Shapes;
}


BITMAP_DEF LIB_PIN::GetMenuImage() const
{
    return s_icons_Pins_Electrical_Type[m_type];
}


wxString LIB_PIN::GetSelectMenuText() const
{
    wxString tmp;

    tmp.Printf( _( "Pin %s, %s, %s" ),
                GetChars( GetNumberString() ),
                GetChars( GetTypeString() ),
                GetChars( wxGetTranslation( pin_style_names[ GetStyleCodeIndex( m_shape ) ] ) )
                );
    return tmp;
}


#if defined(DEBUG)

void LIB_PIN::Show( int nestLevel, std::ostream& os )
{
    NestedSpace( nestLevel, os ) << '<' << GetClass().Lower().mb_str()
                                 << " num=\"" << GetNumberString().mb_str()
                                 << '"' << "/>\n";

//    NestedSpace( nestLevel, os ) << "</" << GetClass().Lower().mb_str() << ">\n";
}

#endif
