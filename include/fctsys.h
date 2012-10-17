/********************/
/* System includes. */
/********************/
#ifndef FCTSYS_H_
#define FCTSYS_H_

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgetss headers
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

/**
 * @note This appears to already be included in the OSX build of wxWidgets.
 *       Will someone with OSX please remove this and see if it compiles?
 */
#ifdef __WXMAC__
#include <Carbon/Carbon.h>
#endif

/**
 * @note Shouldn't we be using wxFileName::GetPathSeparator() instead of the following code
 *       for improved portability?
 */
#ifdef __WINDOWS__
#define DIR_SEP        '\\'
#define STRING_DIR_SEP wxT( "\\" )
#else
#define DIR_SEP        '/'
#define STRING_DIR_SEP wxT( "/" )
#endif

/**
 * @note Do we really need these defined again?
 */
#define UNIX_STRING_DIR_SEP wxT( "/" )
#define WIN_STRING_DIR_SEP  wxT( "\\" )

#ifdef DEBUG
#define D(x)        x
#else
#define D(x)        // nothing
#endif

/**
 * Function Clamp
 * limits @a value within the range @a lower <= @a value <= @a upper.  It will work
 * on temporary expressions, since they are evaluated only once, and it should work
 * on most if not all numeric types, string types, or any type for which "operator < ()"
 * is present. The arguments are accepted in this order so you can remember the
 * expression as a memory aid:
 * <p>
 * result is:  lower <= value <= upper
 */
template <typename T> inline const T& Clamp( const T& lower, const T& value, const T& upper )
{
    wxASSERT( lower <= upper );
    if( value < lower )
        return lower;
    else if( upper < value )
        return upper;
    return value;
}

#define USE_RESIZE_BORDER
#if defined(__UNIX__) || defined(USE_RESIZE_BORDER)
#define MAYBE_RESIZE_BORDER wxRESIZE_BORDER   // Linux users like resizeable borders
#else
#define MAYBE_RESIZE_BORDER 0                 // no resizeable border
#endif

// wxNullPtr is not defined prior to wxWidgets 2.9.0.
#if !wxCHECK_VERSION( 2, 9, 0 )
#define wxNullPtr ((void *)NULL)
#endif

#include <config.h>

#endif // FCTSYS_H__
