// Do not modify this file, it was automatically generated by CMake.

#ifndef CONFIG_H_
#define CONFIG_H_

#cmakedefine HAVE_STRCASECMP

#cmakedefine HAVE_STRNCASECMP

// Handle platform differences in math.h
#cmakedefine HAVE_MATH_H

// Handle platform differences in C++ cmath.
#cmakedefine HAVE_CXX_CMATH

#cmakedefine HAVE_CMATH_ASINH

#cmakedefine HAVE_CMATH_ACOSH

#cmakedefine HAVE_CMATH_ATANH

#cmakedefine HAVE_CMATH_ISINF

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
#endif

#if defined( HAVE_STRNCASECMP )
#define strnicmp strncasecmp
#endif

// Use Posix getc_unlocked() instead of getc() when it's available.
#cmakedefine HAVE_FGETC_NOLOCK

// Warning!!!  Using wxGraphicContext for rendering is experimental.
#cmakedefine USE_WX_GRAPHICS_CONTEXT 1

#cmakedefine USE_IMAGES_IN_MENUS 1

/// The legacy file format revision of the *.brd file created by this build
#define LEGACY_BOARD_FILE_VERSION 2

/// Definition to compile with Pcbnew footprint library table implementation.
#cmakedefine USE_FP_LIB_TABLE

/// The install prefix defined in CMAKE_INSTALL_PREFIX.
#define DEFAULT_INSTALL_PATH               "@CMAKE_INSTALL_PREFIX"

/// Default footprint library install path when installed with `make install`.
#define DEFAULT_FP_LIB_PATH                "@KICAD_FP_LIB_INSTALL_PATH@"

/// When defined, build the GITHUB_PLUGIN for pcbnew.
#cmakedefine BUILD_GITHUB_PLUGIN


#endif  // CONFIG_H_
