/* Do not modify this file, it was automatically generated by CMake. */

#ifndef CONFIG_H_
#define CONFIG_H_

#cmakedefine HAVE_STRCASECMP

#cmakedefine HAVE_STRNCASECMP

#cmakedefine HAVE_ISO_STRICMP

#cmakedefine HAVE_ISO_STRNICMP

#cmakedefine HAVE_ISO_SNPRINTF

#if defined( HAVE_ISO_SNPRINTF )
#define snprintf _snprintf
#endif

#cmakedefine HAVE_ISO_HYPOT

#if defined( HAVE_ISO_HYPOT )
#define hypot _hypot
#endif

#cmakedefine HAVE_CLOCK_GETTIME
#cmakedefine HAVE_GETTIMEOFDAY_FUNC

#cmakedefine MALLOC_IN_STDLIB_H

#if !defined( MALLOC_IN_STDLIB_H )
#include <malloc.h>
#endif

#cmakedefine HAVE_ISO646_H

#if defined( HAVE_ISO646_H )
#include <iso646.h>
#endif

#if defined( HAVE_STRCASECMP )
#define stricmp strcasecmp
#elif defined( HAVE_ISO_STRICMP )
#define stricmp _stricmp
#endif

#if defined( HAVE_STRNCASECMP )
#define strnicmp strncasecmp
#elif defined( HAVE_ISO_STRNICMP )
#define strnicmp _strnicmp
#endif

/* Use Posix getc_unlocked() instead of getc() when it's available. */
#cmakedefine HAVE_GETC_UNLOCKED

/* Warning!!!  Using wxGraphicContext for rendering is experimental. */
#cmakedefine USE_WX_GRAPHICS_CONTEXT 1

#cmakedefine USE_IMAGES_IN_MENUS 1

/// Definitions to enable the s-expression file formats and nanometer units.
#cmakedefine USE_PCBNEW_NANOMETRES
#cmakedefine USE_PCBNEW_SEXPR_FILE_FORMAT
#cmakedefine USE_PCBNEW_SEXPR_FOOTPRINT_LIBS

/// The legacy file format revision of the *.brd file created by this build
#if defined(USE_PCBNEW_NANOMETRES)
#define LEGACY_BOARD_FILE_VERSION          2
#else
#define LEGACY_BOARD_FILE_VERSION          1
#endif

#endif  // CONFIG_H_
