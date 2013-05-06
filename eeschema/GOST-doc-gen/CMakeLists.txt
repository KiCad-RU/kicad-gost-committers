if(USE_OPENOFFICE_SDK)
    if(${OpenOffice_VERSION} GREATER 340 OR ${OpenOffice_VERSION} EQUAL 340)
        set(NEW_OO_CONNECT_MODE TRUE)
        add_definitions(-DNEW_OO_CONNECT_MODE)
        message(STATUS "NEW_OO_CONNECT_MODE is active")
    endif(${OpenOffice_VERSION} GREATER 340 OR ${OpenOffice_VERSION} EQUAL 340)

    if(WIN32)
        set(TYPES_RDB_FILE ${OOO_BASIS_DIR}/URE/misc/types.rdb)
    else(WIN32)
        set(TYPES_RDB_FILE ${OOO_BASIS_DIR}/ure-link/share/misc/types.rdb)
    endif(WIN32)

    if(WIN32)
        set(OFFAPI_RDB_FILE ${OOO_BASIS_DIR}/Basis/program/offapi.rdb)
    elseif(NEW_OO_CONNECT_MODE)
        set(OFFAPI_RDB_FILE ${OOO_BASIS_DIR}/program/types/offapi.rdb)
    else()
        set(OFFAPI_RDB_FILE ${OOO_BASIS_DIR}/program/offapi.rdb)
    endif()

    execute_process(COMMAND ${OOO_SDK_DIR}/bin/cppumaker -Gc -BUCR -O ${PROJECT_SOURCE_DIR}/include/openoffice ${TYPES_RDB_FILE} ${OFFAPI_RDB_FILE})

    if(WIN32)
        execute_process(COMMAND del ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb)

        execute_process(COMMAND cmd /C "echo SET PATH=${OOO_BASIS_DIR}/URE/bin;^%PATH^% > ${CMAKE_CURRENT_BINARY_DIR}/../env.bat")

        if(NEW_OO_CONNECT_MODE)
            STRING(REGEX REPLACE " " "%20" OFFAPI_RDB_FILE ${OFFAPI_RDB_FILE})
            execute_process(COMMAND cmd /C "echo SET URE_MORE_TYPES=${OFFAPI_RDB_FILE} >> ${CMAKE_CURRENT_BINARY_DIR}/../env.bat")
        else(NEW_OO_CONNECT_MODE)
            execute_process(COMMAND ${OOO_BASIS_DIR}/URE/bin/regmerge ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb / "${TYPES_RDB_FILE}" "${OFFAPI_RDB_FILE}")
            execute_process(COMMAND ${OOO_BASIS_DIR}/URE/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c connector.uno.dll)
            execute_process(COMMAND ${OOO_BASIS_DIR}/URE/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c remotebridge.uno.dll)
            execute_process(COMMAND ${OOO_BASIS_DIR}/URE/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c bridgefac.uno.dll)
            execute_process(COMMAND ${OOO_BASIS_DIR}/URE/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c uuresolver.uno.dll)
        endif(NEW_OO_CONNECT_MODE)
    else(WIN32)
        execute_process(COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb)

        execute_process(COMMAND bash -c "echo \"#!/bin/sh\n\nLD_LIBRARY_PATH=${OOO_BASIS_DIR}/ure-link/lib\nexport LD_LIBRARY_PATH\" > ${CMAKE_CURRENT_BINARY_DIR}/../env.sh")

        if(NEW_OO_CONNECT_MODE)
            execute_process(COMMAND bash -c "echo \"\nURE_MORE_TYPES=file://${OFFAPI_RDB_FILE}\nexport URE_MORE_TYPES\" >> ${CMAKE_CURRENT_BINARY_DIR}/../env.sh")
        else(NEW_OO_CONNECT_MODE)
            execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regmerge ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb / "${TYPES_RDB_FILE}" "${OFFAPI_RDB_FILE}")
            execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c connector.uno.so)
            execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c remotebridge.uno.so)
            execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c bridgefac.uno.so)
            execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c uuresolver.uno.so)
        endif(NEW_OO_CONNECT_MODE)
    endif(WIN32)
endif(USE_OPENOFFICE_SDK)

add_definitions(-DEESCHEMA)

include_directories(BEFORE ${INC_BEFORE})
include_directories(
    ../.
    ${OOO_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include/openoffice
    ${INC_AFTER}
    )

make_lexer(
    ${CMAKE_CURRENT_SOURCE_DIR}/../template_fieldnames.keywords
    ${CMAKE_CURRENT_SOURCE_DIR}/../template_fieldnames_lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../template_fieldnames_keywords.cpp
    TFIELD_T
    )

if(USE_OPENOFFICE_SDK)
    set(OO_SDK_SRCS oo_iface.cxx)
endif(USE_OPENOFFICE_SDK)

set(GOST_DOC_GEN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../template_fieldnames_keywords.cpp
    ../../scripting/python_scripting.cpp
    ../../scripting/wx_python_helpers.cpp
    common_funcs.cpp
    component.cpp
    component_db.cpp
    dictionaries.cpp
    doc_common.cpp
    doc_component_index.cpp
    doc_specification.cpp
    GOST_comp_lister.cpp
    ${OO_SDK_SRCS}
    oo_python_uno_iface.cpp
    )

add_library(GOST-doc-gen STATIC ${GOST_DOC_GEN_SRCS})

if(USE_OPENOFFICE_SDK AND NOT NEW_OO_CONNECT_MODE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb
            DESTINATION ${KICAD_DATA}/GOST-doc-gen
            COMPONENT resources)
endif(USE_OPENOFFICE_SDK AND NOT NEW_OO_CONNECT_MODE)

install(FILES uno_iface.py
        DESTINATION ${KICAD_DATA}/GOST-doc-gen
        COMPONENT resources)

install( DIRECTORY templates
        DESTINATION ${KICAD_DATA}/GOST-doc-gen
        COMPONENT resources )
