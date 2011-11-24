/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
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
 * @file common.cpp
 */

#include "fctsys.h"
#include "gr_basic.h"
#include "trigo.h"
#include "wxstruct.h"
#include "base_struct.h"
#include "common.h"
#include "macros.h"
#include "build_version.h"
#include "confirm.h"
#include <wx/process.h>

/**
 * Global variables definitions.
 *
 * TODO: All if these variables should be moved into the class were they
 *       are defined and used.  Most of them probably belong in the
 *       application class.
 */

/* Standard page sizes in 1/1000 inch */
#if defined(KICAD_GOST)
Ki_PageDescr  g_Sheet_A4( wxSize( 8283, 11700 ), wxPoint( 0, 0 ), wxT( "A4" ) );
#else
Ki_PageDescr  g_Sheet_A4( wxSize( 11700, 8267 ), wxPoint( 0, 0 ), wxT( "A4" ) );
#endif
Ki_PageDescr  g_Sheet_A3( wxSize( 16535, 11700 ), wxPoint( 0, 0 ), wxT( "A3" ) );
Ki_PageDescr  g_Sheet_A2( wxSize( 23400, 16535 ), wxPoint( 0, 0 ), wxT( "A2" ) );
Ki_PageDescr  g_Sheet_A1( wxSize( 33070, 23400 ), wxPoint( 0, 0 ), wxT( "A1" ) );
Ki_PageDescr  g_Sheet_A0( wxSize( 46800, 33070 ), wxPoint( 0, 0 ), wxT( "A0" ) );
Ki_PageDescr  g_Sheet_A( wxSize( 11000, 8500 ), wxPoint( 0, 0 ), wxT( "A" ) );
Ki_PageDescr  g_Sheet_B( wxSize( 17000, 11000 ), wxPoint( 0, 0 ), wxT( "B" ) );
Ki_PageDescr  g_Sheet_C( wxSize( 22000, 17000 ), wxPoint( 0, 0 ), wxT( "C" ) );
Ki_PageDescr  g_Sheet_D( wxSize( 34000, 22000 ), wxPoint( 0, 0 ), wxT( "D" ) );
Ki_PageDescr  g_Sheet_E( wxSize( 44000, 34000 ), wxPoint( 0, 0 ), wxT( "E" ) );
Ki_PageDescr  g_Sheet_GERBER( wxSize( 32000, 32000 ), wxPoint( 0, 0 ), wxT( "GERBER" ) );
Ki_PageDescr  g_Sheet_user( wxSize( 17000, 11000 ), wxPoint( 0, 0 ), wxT( "User" ) );

Ki_PageDescr* g_SheetSizeList[NB_ITEMS + 1] =
{
    &g_Sheet_A4,   &g_Sheet_A3, &g_Sheet_A2, &g_Sheet_A1, &g_Sheet_A0,
    &g_Sheet_A,    &g_Sheet_B,  &g_Sheet_C,  &g_Sheet_D,  &g_Sheet_E,
    &g_Sheet_user, NULL
};


const wxString ProjectFileExtension( wxT( "pro" ) );
const wxString SchematicFileExtension( wxT( "sch" ) );
const wxString NetlistFileExtension( wxT( "net" ) );
const wxString GerberFileExtension( wxT( "pho" ) );
const wxString PcbFileExtension( wxT( "brd" ) );
const wxString PdfFileExtension( wxT( "pdf" ) );
const wxString MacrosFileExtension( wxT( "mcr" ) );

const wxString ProjectFileWildcard( _( "KiCad project files (*.pro)|*.pro" ) );
const wxString SchematicFileWildcard( _( "KiCad schematic files (*.sch)|*.sch" ) );
const wxString NetlistFileWildcard( _( "KiCad netlist files (*.net)|*.net" ) );
const wxString GerberFileWildcard( _( "Gerber files (*.pho)|*.pho" ) );
const wxString PcbFileWildcard( _( "KiCad printed circuit board files (*.brd)|*.brd" ) );
const wxString PdfFileWildcard( _( "Portable document format files (*.pdf)|*.pdf" ) );
const wxString MacrosFileWildcard( _( "KiCad recorded macros (*.mcr)|*.mcr" ) );
const wxString AllFilesWildcard( _( "All files (*)|*" ) );


wxString       g_ProductName    = wxT( "KiCad E.D.A.  " );
bool           g_ShowPageLimits = true;
wxString       g_UserLibDirBuffer;

wxString       g_Prj_Default_Config_FullFilename;
wxString       g_Prj_Config_LocalFilename;

EDA_UNITS_T    g_UserUnit;

int            g_GhostColor;

/**
 * The predefined colors used in KiCad.
 * Please: if you change a value, remember these values are carefully chosen
 * to have good results in Pcbnew, that uses the ORed value of basic colors
 * when displaying superimposed objects
 * This list must have exactly NBCOLOR items
 */
StructColors ColorRefs[NBCOLOR] =
{
    { 0,    0,   0,   BLACK,        wxT( "BLACK" ),        DARKDARKGRAY          },
    { 192,  0,   0,   BLUE,         wxT( "BLUE" ),         LIGHTBLUE             },
    { 0,    160, 0,   GREEN,        wxT( "GREEN" ),        LIGHTGREEN            },
    { 160,  160, 0,   CYAN,         wxT( "CYAN" ),         LIGHTCYAN             },
    { 0,    0,   160, RED,          wxT( "RED" ),          LIGHTRED              },
    { 160,  0,   160, MAGENTA,      wxT( "MAGENTA" ),      LIGHTMAGENTA          },
    { 0,    128, 128, BROWN,        wxT( "BROWN" ),        YELLOW                },
    { 192,  192, 192, LIGHTGRAY,    wxT( "GRAY" ),         WHITE                 },
    { 128,  128, 128, DARKGRAY,     wxT( "DARKGRAY" ),     LIGHTGRAY             },
    { 255,  0,   0,   LIGHTBLUE,    wxT( "LIGHTBLUE" ),    LIGHTBLUE             },
    { 0,    255, 0,   LIGHTGREEN,   wxT( "LIGHTGREEN" ),   LIGHTGREEN            },
    { 255,  255, 0,   LIGHTCYAN,    wxT( "LIGHTCYAN" ),    LIGHTCYAN             },
    { 0,    0,   255, LIGHTRED,     wxT( "LIGHTRED" ),     LIGHTRED              },
    { 255,  0,   255, LIGHTMAGENTA, wxT( "LIGHTMAGENTA" ), LIGHTMAGENTA          },
    { 0,    255, 255, YELLOW,       wxT( "YELLOW" ),       YELLOW                },
    { 255,  255, 255, WHITE,        wxT( "WHITE" ),        WHITE                 },
    {  64,  64,  64,  DARKDARKGRAY, wxT( "DARKDARKGRAY" ), DARKGRAY              },
    {  64,  0,   0,   DARKBLUE,     wxT( "DARKBLUE" ),     BLUE                  },
    {    0, 64,  0,   DARKGREEN,    wxT( "DARKGREEN" ),    GREEN                 },
    {  64,  64,  0,   DARKCYAN,     wxT( "DARKCYAN" ),     CYAN                  },
    {    0, 0,   80,  DARKRED,      wxT( "DARKRED" ),      RED                   },
    {  64,  0,   64,  DARKMAGENTA,  wxT( "DARKMAGENTA" ),  MAGENTA               },
    {    0, 64,  64,  DARKBROWN,    wxT( "DARKBROWN" ),    BROWN                 },
    {  128, 255, 255, LIGHTYELLOW,  wxT( "LIGHTYELLOW" ),  LIGHTYELLOW           }
};


/**
 * Function to use local notation or C standard notation for floating point numbers
 * some countries use 1,5 and others (and C) 1.5
 * so we switch from local to C and C to local when reading or writing files
 * And other problem is a bug when cross compiling under linux:
 * a printf print 1,5 and the read functions expects 1.5
 * (depending on version print = 1.5 and read = 1,5
 * Very annoying and we detect this and use a stupid but necessary workarount
*/
bool g_DisableFloatingPointLocalNotation = false;


void SetLocaleTo_C_standard( void )
{
    setlocale( LC_NUMERIC, "C" );    // Switch the locale to standard C
}


void SetLocaleTo_Default( void )
{
    if( ! g_DisableFloatingPointLocalNotation )
        setlocale( LC_NUMERIC, "" );      // revert to the current locale
}


bool EnsureTextCtrlWidth( wxTextCtrl* aCtrl, const wxString* aString )
{
    wxWindow* window = aCtrl->GetParent();

    if( !window )
        window = aCtrl;

    wxString ctrlText;

    if( !aString )
    {
        ctrlText = aCtrl->GetValue();
        aString  = &ctrlText;
    }

    wxCoord width;
    wxCoord height;

    {
        wxClientDC dc( window );
        dc.SetFont( aCtrl->GetFont() );
        dc.GetTextExtent( *aString, &width, &height );
    }

    wxSize size = aCtrl->GetSize();

    if( size.GetWidth() < width + 10 )
    {
        size.SetWidth( width + 10 );
        aCtrl->SetSizeHints( size );
        return true;
    }

    return false;
}


Ki_PageDescr::Ki_PageDescr( const wxSize& size, const wxPoint& offset, const wxString& name )
{
    // All sizes are in 1/1000 inch
    m_Size   = size;
    m_Offset = offset;
    m_Name   = name;

    // Adjust the default value for margins to 400 mils (0,4 inch or 10 mm)
#if defined(KICAD_GOST)
    m_LeftMargin   = GOST_LEFTMARGIN;
    m_RightMargin  = GOST_RIGHTMARGIN;
    m_TopMargin    = GOST_TOPMARGIN;
    m_BottomMargin = GOST_BOTTOMMARGIN;
#else
    m_LeftMargin = m_RightMargin = m_TopMargin = m_BottomMargin = 400;
#endif
}


wxString ReturnUnitSymbol( EDA_UNITS_T aUnit, const wxString& formatString )
{
    wxString tmp;
    wxString label;

    switch( aUnit )
    {
    case INCHES:
        tmp = _( "\"" );
        break;

    case MILLIMETRES:
        tmp = _( "mm" );
        break;

    case UNSCALED_UNITS:
        break;
    }

    if( formatString.IsEmpty() )
        return tmp;

    label.Printf( formatString, GetChars( tmp ) );

    return label;
}


wxString GetUnitsLabel( EDA_UNITS_T aUnit )
{
    wxString label;

    switch( aUnit )
    {
    case INCHES:
        label = _( "inches" );
        break;

    case MILLIMETRES:
        label = _( "millimeters" );
        break;

    case UNSCALED_UNITS:
        label = _( "units" );
        break;
    }

    return label;
}


wxString GetAbbreviatedUnitsLabel( EDA_UNITS_T aUnit )
{
    wxString label;

    switch( aUnit )
    {
    case INCHES:
        label = _( "in" );
        break;

    case MILLIMETRES:
        label = _( "mm" );
        break;

    case UNSCALED_UNITS:
        break;
    }

    return label;
}


void AddUnitSymbol( wxStaticText& Stext, EDA_UNITS_T aUnit )
{
    wxString msg = Stext.GetLabel();

    msg += ReturnUnitSymbol( aUnit );

    Stext.SetLabel( msg );
}


void PutValueInLocalUnits( wxTextCtrl& TextCtr, int Value, int Internal_Unit )
{
    wxString msg = ReturnStringFromValue( g_UserUnit, Value, Internal_Unit );

    TextCtr.SetValue( msg );
}


int ReturnValueFromTextCtrl( const wxTextCtrl& TextCtr, int Internal_Unit )
{
    int      value;
    wxString msg = TextCtr.GetValue();

    value = ReturnValueFromString( g_UserUnit, msg, Internal_Unit );

    return value;
}


wxString ReturnStringFromValue( EDA_UNITS_T aUnit, int aValue, int aInternal_Unit,
                                bool aAdd_unit_symbol )
{
    wxString StringValue;
    double   value_to_print;

    value_to_print = To_User_Unit( aUnit, aValue, aInternal_Unit );

    /* Yet another 'if Pcbnew' :( */
    StringValue.Printf( ( aInternal_Unit > 1000 ) ? wxT( "%.4f" ) : wxT( "%.3f" ),
                        value_to_print );

    if( aAdd_unit_symbol )
        switch( aUnit )
        {
        case INCHES:
            StringValue += _( " \"" );
            break;

        case MILLIMETRES:
            StringValue += _( " mm" );
            break;

        case UNSCALED_UNITS:
            break;
        }

    return StringValue;
}


int ReturnValueFromString( EDA_UNITS_T aUnit, const wxString& TextValue, int Internal_Unit )
{
    int    Value;
    double dtmp = 0;

    /* Acquire the 'right' decimal point separator */
    const struct lconv* lc = localeconv();
    wxChar decimal_point = lc->decimal_point[0];
    wxString            buf( TextValue.Strip( wxString::both ) );

    /* Convert the period in decimal point */
    buf.Replace( wxT( "." ), wxString( decimal_point, 1 ) );

    // An ugly fix needed by WxWidgets 2.9.1 that sometimes
    // back to a point as separator, although the separator is the comma
    // TODO: remove this line if WxWidgets 2.9.2 fixes this issue
    buf.Replace( wxT( "," ), wxString( decimal_point, 1 ) );

    /* Find the end of the numeric part */
    unsigned brk_point = 0;

    while( brk_point < buf.Len() )
    {
        wxChar ch = buf[brk_point];

        if( !( (ch >= '0' && ch <='9') || (ch == decimal_point) || (ch == '-') || (ch == '+') ) )
        {
            break;
        }

        ++brk_point;
    }

    /* Extract the numeric part */
    buf.Left( brk_point ).ToDouble( &dtmp );

    /* Check the optional unit designator (2 ch significant) */
    wxString unit( buf.Mid( brk_point ).Strip( wxString::leading ).Left( 2 ).Lower() );

    if( unit == wxT( "in" ) || unit == wxT( "\"" ) )
    {
        aUnit = INCHES;
    }
    else if( unit == wxT( "mm" ) )
    {
        aUnit = MILLIMETRES;
    }
    else if( unit == wxT( "mi" ) || unit == wxT( "th" ) ) /* Mils or thous */
    {
        aUnit = INCHES;
        dtmp /= 1000;
    }

    Value = From_User_Unit( aUnit, dtmp, Internal_Unit );

    return Value;
}


/**
 * Function wxStringSplit
 * Split a String to a String List when founding 'splitter'
 * @return the list
 * @param txt : wxString : a String text
 * @param splitter : wxChar : the 'split' character
 */
wxArrayString* wxStringSplit( wxString txt, wxChar splitter )
{
    wxArrayString* list = new wxArrayString();

    while( 1 )
    {
        int index = txt.Find( splitter );
        if( index == wxNOT_FOUND )
            break;

        wxString tmp;
        tmp = txt.Mid( 0, index );
        txt = txt.Mid( index + 1, txt.size() - index );
        list->Add( tmp );
    }

    if( !txt.IsEmpty() )
    {
        list->Add( txt );
    }

    return list;
}



/**
 * Function To_User_Unit
 * Convert in inch or mm the variable "val" (double)given in internal units
 * @return the converted value, in double
 * @param aUnit : user measure unit
 * @param val : double : the given value
 * @param internal_unit_value = internal units per inch
 */
double To_User_Unit( EDA_UNITS_T aUnit, double val, int internal_unit_value )
{

    switch( aUnit )
    {
    case MILLIMETRES:
        return val * 25.4 / internal_unit_value;

    case INCHES:
        return val / internal_unit_value;

    default:
        return val;
    }
}


/*
 * Return in internal units the value "val" given in inch or mm
 */
int From_User_Unit( EDA_UNITS_T aUnit, double val, int internal_unit_value )
{
    double value;

    switch( aUnit )
    {
    case MILLIMETRES:
        value = val * internal_unit_value / 25.4;
        break;

    case INCHES:
        value = val * internal_unit_value;
        break;

    default:
    case UNSCALED_UNITS:

        value = val;
    }

    return wxRound( value );
}


/*
 * Return the string date "day month year" like "23 jun 2005"
 */
wxString GenDate()
{
    static const wxString mois[12] =
    {
        wxT( "jan" ), wxT( "feb" ), wxT( "mar" ), wxT( "apr" ), wxT( "may" ), wxT( "jun" ),
        wxT( "jul" ), wxT( "aug" ), wxT( "sep" ), wxT( "oct" ), wxT( "nov" ), wxT( "dec" )
    };

    time_t     buftime;
    struct tm* Date;
    wxString   string_date;

    time( &buftime );

    Date = gmtime( &buftime );
    string_date.Printf( wxT( "%d %s %d" ), Date->tm_mday,
                        GetChars( mois[Date->tm_mon] ),
                        Date->tm_year + 1900 );
    return string_date;
}


bool ProcessExecute( const wxString& aCommandLine, int aFlags )
{
#ifdef __WINDOWS__
    int        pid = wxExecute( aCommandLine );
    return pid ? true : false;
#else
    wxProcess* process = wxProcess::Open( aCommandLine, aFlags );
    return (process != NULL) ? true : false;
#endif
}


int GetTimeStamp()
{
    static int OldTimeStamp, NewTimeStamp;

    NewTimeStamp = time( NULL );

    if( NewTimeStamp <= OldTimeStamp )
        NewTimeStamp = OldTimeStamp + 1;

    OldTimeStamp = NewTimeStamp;

    return NewTimeStamp;
}


const wxString& valeur_param( int valeur, wxString& buf_texte )
{
    switch( g_UserUnit )
    {
    case MILLIMETRES:
        buf_texte.Printf( _( "%3.3f mm" ), valeur * 0.00254 );
        break;

    case INCHES:
        buf_texte.Printf( wxT( "%2.4f \"" ), valeur * 0.0001 );
        break;

    case UNSCALED_UNITS:
        buf_texte.Printf( wxT( "%d" ), valeur );
        break;
    }

    return buf_texte;
}


wxString CoordinateToString( int aValue, int aInternalUnits, bool aConvertToMils )
{
    wxCHECK_MSG( (aInternalUnits == EESCHEMA_INTERNAL_UNIT)
                 || (aInternalUnits == PCB_INTERNAL_UNIT),
                 wxString( _( "*** Bad Internal Units ***" ) ),
                 wxT( "Invalid interanl units value." ) );

    wxString      text;
    const wxChar* format;
    double        value = To_User_Unit( g_UserUnit, aValue, aInternalUnits );

    if( g_UserUnit == INCHES )
    {
        if( aConvertToMils )
        {
            format = ( aInternalUnits == EESCHEMA_INTERNAL_UNIT ) ? wxT( "%.0f" ) : wxT( "%.1f" );
            value *= 1000;
        }
        else
        {
            format = ( aInternalUnits == EESCHEMA_INTERNAL_UNIT ) ? wxT( "%.3f" ) : wxT( "%.4f" );
        }
    }
    else
    {
        format = ( aInternalUnits == EESCHEMA_INTERNAL_UNIT ) ? wxT( "%.2f" ) : wxT( "%.3f" );
    }

    text.Printf( format, value );

    if( g_UserUnit == INCHES )
        text += ( aConvertToMils ) ? _( " mils" ) : _( " in" );
    else
        text += _( " mm" );

    return text;
}


wxString& operator <<( wxString& aString, const wxPoint& aPos )
{
    wxString temp;

    aString << wxT( "@ (" ) << valeur_param( aPos.x, temp );
    aString << wxT( "," ) << valeur_param( aPos.y, temp );
    aString << wxT( ")" );

    return aString;
}
