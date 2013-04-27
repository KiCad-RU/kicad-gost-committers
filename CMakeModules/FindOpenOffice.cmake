# - Try to find OpenOffice.org SDK, installed from repositories or from "vanilla" distribution
# Once done this will define
#
#  OpenOffice_FOUND - System has OpenOffice.org with SDK
#  OpenOffice_VERSION - Version of OpenOffice.org with SDK
#  OOO_PREFIX - Prefix of OpenOffice.org installation
#  OOO_BASIS_DIR - "basis" directory of OpenOffice.org
#  OOO_PROGRAM_DIR - "program" directory of OpenOffice.org
#  OOO_URE_DIR - "ure" directory of OpenOffice.org
#  OOO_SDK_DIR - "sdk" directory of OpenOffice.org
#  OOO_INCLUDE_DIR -  include directory of OpenOffice.org
#  UNOPKG_EXECUTABLE - Absolute path of unopkg tool
#
#  Copyright (c) 2010 Konstantin Tokarev <annulen@gmail.com>
#  Copyright (c) 2013 Alexander Lunev <al.lunev@yahoo.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# TODO: Find Go-oo from Novell

unset(OOO_PREFIX CACHE)
unset(OOO_BASIS_DIR CACHE)
unset(OOO_SDK_DIR CACHE)
unset(OOO_PROGRAM_DIR CACHE)
unset(UNOPKG_EXECUTABLE CACHE)
unset(OOO_URE_DIR CACHE)
unset(OOO_INCLUDE_DIR CACHE)

if(UNIX)
  set(OOO_PATH_HINTS
    /usr/lib/openoffice/*
    /usr/lib/ooo/*
    /usr/lib64/openoffice/*
    /usr/lib64/ooo/*
    /usr/local/openoffice/*
    /opt/openoffice.org3/*
    /opt/ooo-dev3/*
    /usr/lib/libreoffice
    /usr/lib64/libreoffice
  )
endif(UNIX)

if(OOO_PREFIX)
  set(OOO_PATH_HINTS 
  "${OOO_PREFIX}/*"
  ${OOO_PATH_HINTS}
)
endif(OOO_PREFIX)

find_path(OOO_BASIS_DIR
  NAMES
    sdk/bin/idlc
  HINTS
    ${OOO_PATH_HINTS}  
)
mark_as_advanced(OOO_BASIS_DIR)

if(NOT OOO_BASIS_DIR)
  if(OpenOffice_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OpenOffice.org SDK")
  endif(OpenOffice_FIND_REQUIRED)
else(NOT OOO_BASIS_DIR)
  set(OOO_PREFIX "${OOO_BASIS_DIR}" CACHE STRING "OpenOffice.org root directory")
  set(OOO_SDK_DIR "${OOO_BASIS_DIR}/sdk" CACHE STRING "OpenOffice.org SDK root directory")
  message(STATUS "Found OpenOffice.org SDK: ${OOO_SDK_DIR}")
endif(NOT OOO_BASIS_DIR)

find_path(OOO_PROGRAM_DIR
  NAMES
    soffice
  HINTS
    "${OOO_PREFIX}/program"
    "${OOO_PREFIX}/../program"
)
mark_as_advanced(OOO_PROGRAM_DIR)

if(NOT OOO_PROGRAM_DIR)
  if(OpenOffice_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OpenOffice.org program directory")
  endif(OpenOffice_FIND_REQUIRED)
else(NOT OOO_PROGRAM_DIR)
  message(STATUS "Found OpenOffice.org program directory: ${OOO_PROGRAM_DIR}")
endif(NOT OOO_PROGRAM_DIR)

find_file(UNOPKG_EXECUTABLE
  NAMES
    unopkg
  HINTS 
    "${OOO_PREFIX}/program"
    "${OOO_PREFIX}/../program"
  NO_DEFAULT_PATH
)
mark_as_advanced(UNOPKG_EXECUTABLE)

if(NOT UNOPKG_EXECUTABLE)
  if(OpenOffice_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find unopkg executable")
  endif(OpenOffice_FIND_REQUIRED)
else(NOT UNOPKG_EXECUTABLE)
  message(STATUS "Found unopkg executable: ${UNOPKG_EXECUTABLE}")
endif(NOT UNOPKG_EXECUTABLE)


find_path(OOO_URE_DIR
  NAMES
    share/java/java_uno.jar
  HINTS
    "${OOO_BASIS_DIR}/*"
)
mark_as_advanced(OOO_URE_DIR)

if(NOT OOO_URE_DIR)
  if(OpenOffice_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OpenOffice ure directory")
  endif(OpenOffice_FIND_REQUIRED)
else(NOT OOO_URE_DIR)
  set(OOO_URE_JAVA_DIR "${OOO_URE_DIR}/share/java")
  message(STATUS "Found URE Java path: ${OOO_URE_JAVA_DIR}")
endif(NOT OOO_URE_DIR)

find_path(OOO_INCLUDE_DIR 
  NAMES 
    com/sun/star/uno/Any.h
  HINTS
    "${OOO_SDK_DIR}/include"
    /usr/include/openoffice
)
mark_as_advanced(OOO_INCLUDE_DIR)

if(NOT OOO_INCLUDE_DIR)
  if(OpenOffice_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find OpenOffice.org SDK include directory")
  endif(OpenOffice_FIND_REQUIRED)
else(NOT OOO_INCLUDE_DIR)
  message(STATUS "Found OpenOffice.org SDK include directory: ${OOO_INCLUDE_DIR}")
endif(NOT OOO_INCLUDE_DIR)

if(OOO_PROGRAM_DIR AND OOO_SDK_DIR AND OOO_URE_DIR AND UNOPKG_EXECUTABLE AND OOO_INCLUDE_DIR)
  set(OpenOffice_FOUND 1)
  file(READ "${OOO_PROGRAM_DIR}/versionrc" _VERSION_RC_CONTENTS)
  string(REGEX REPLACE ".*ProductMajor=([0-9]+).*" "\\1" OpenOffice_VERSION "${_VERSION_RC_CONTENTS}")
else(OOO_PROGRAM_DIR AND OOO_SDK_DIR AND OOO_URE_DIR AND UNOPKG_EXECUTABLE AND OOO_INCLUDE_DIR)
  set(OpenOffice_FOUND 0)
endif(OOO_PROGRAM_DIR AND OOO_SDK_DIR AND OOO_URE_DIR AND UNOPKG_EXECUTABLE AND OOO_INCLUDE_DIR)
