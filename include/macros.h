/**
 * @file macros.h
 * @brief This file contains miscellaneous helper definitions and functions.
 */

#ifndef MACROS_H
#define MACROS_H

#include <wx/wx.h>

/**
 * Macro TO_UTF8
 * converts a wxString to a UTF8 encoded C string for all wxWidgets build modes.
 * wxstring is a wxString, not a wxT() or _().  The scope of the return value
 * is very limited and volatile, but can be used with printf() style functions well.
 */
#define TO_UTF8( wxstring )  ( (const char*) (wxstring).utf8_str() )

/**
 * Macro FROM_UTF8
 * converts a UTF8 encoded C string to a wxString for all wxWidgets build modes.
 */
//#define FROM_UTF8( cstring )    wxString::FromUTF8( cstring )
static inline wxString FROM_UTF8( const char* cstring )
{
    wxString line = wxString::FromUTF8( cstring );

    if( line.IsEmpty() )  // happens when cstring is not a valid UTF8 sequence
        line = wxConvCurrent->cMB2WC( cstring );    // try to use locale conversion

    return line;
}

/**
 * Function GetChars
 * returns a wxChar* to the actual character data within a wxString, and is
 * helpful for passing strings to wxString::Printf(wxT("%s"), GetChars(wxString) )
 * <p>
 * wxChar is defined to be
 * <ul>
 * <li> standard C style char when wxUSE_UNICODE==0 </li>
 * <li> wchar_t when wxUSE_UNICODE==1 (the default). </li>
 * </ul>
 * i.e. it depends on how the wxWidgets library was compiled.  There was a period
 * during the development of wxWidgets 2.9 when GetData() was missing, so this
 * function was used to provide insulation from that design change.  It may
 * no longer be needed, and is harmless.  GetData() seems to be an acceptable
 * alternative in all cases now.
 */
static inline const wxChar* GetChars( const wxString& s )
{
#if wxCHECK_VERSION( 2, 9, 0 )
    return (const wxChar*) s.c_str();
#else
    return s.GetData();
#endif
}

#define NEGATE( x ) (x = -x)

/// # of elements in an array
#define DIM( x )    unsigned( sizeof(x) / sizeof( (x)[0] ) )    // not size_t


#define DEG2RAD( Deg ) ( (Deg) * M_PI / 180.0 )
#define RAD2DEG( Rad ) ( (Rad) * 180.0 / M_PI )

// Normalize angle to be in the -360.0 .. 360.0:
#define NORMALIZE_ANGLE_360( Angle ) {                        \
        while( Angle < -3600 )                                \
            Angle += 3600;                                    \
        while( Angle > 3600 )                                 \
            Angle -= 3600; }

/* Normalize angle to be in the 0.0 .. 360.0 range: */
#define NORMALIZE_ANGLE_POS( Angle ) {                        \
        while( Angle < 0 )                                    \
            Angle += 3600;                                    \
        while( Angle >= 3600 )                                \
            Angle -= 3600; }

#define NEGATE_AND_NORMALIZE_ANGLE_POS( Angle ) {             \
        Angle = -Angle;                                       \
        while( Angle < 0 )                                    \
            Angle += 3600;                                    \
        while( Angle >= 3600 )                                \
            Angle -= 3600; }

/* Normalize angle to be in the -90.0 .. 90.0 range */
#define NORMALIZE_ANGLE_90( Angle ) {                         \
        while( Angle < -900 )                                 \
            Angle += 1800;                                    \
        while( Angle > 900 )                                  \
            Angle -= 1800; }

/* Normalize angle to be in the -180.0 .. 180.0 range */
#define NORMALIZE_ANGLE_180( Angle ) {                        \
        while( Angle <= -1800 )                               \
            Angle += 3600;                                    \
        while( Angle > 1800 )                                 \
            Angle -= 3600; }

/*****************************/
/* macro to exchange 2 items */
/*****************************/

/*
 * The EXCHG macro uses BOOST_TYPEOF for compilers that do not have native
 * typeof support (MSVC).  Please do not attempt to qualify these macros
 * within #ifdef compiler definitions pragmas.  BOOST_TYPEOF is smart enough
 * to check for native typeof support and use it instead of it's own
 * implementation.  These macros effectively compile to nothing on platforms
 * with native typeof support.
 */

#include <boost/typeof/typeof.hpp>

// we have to register the types used with the typeof keyword with boost
BOOST_TYPEOF_REGISTER_TYPE( wxPoint )
BOOST_TYPEOF_REGISTER_TYPE( wxSize )
BOOST_TYPEOF_REGISTER_TYPE( wxString )
class DrawSheetLabelStruct;
BOOST_TYPEOF_REGISTER_TYPE( DrawSheetLabelStruct* )
class EDA_ITEM;
BOOST_TYPEOF_REGISTER_TYPE( EDA_ITEM* )
class D_PAD;
BOOST_TYPEOF_REGISTER_TYPE( D_PAD* )
BOOST_TYPEOF_REGISTER_TYPE( const D_PAD* )
class BOARD_ITEM;
BOOST_TYPEOF_REGISTER_TYPE( BOARD_ITEM* )

#define EXCHG( a, b ) { BOOST_TYPEOF( a ) __temp__ = (a);      \
                        (a) = (b);                             \
                        (b) = __temp__; }


#endif /* ifdef MACRO_H */
