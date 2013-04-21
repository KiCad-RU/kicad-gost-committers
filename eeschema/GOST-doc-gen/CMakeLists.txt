
find_package(OpenOffice REQUIRED)
execute_process(COMMAND ${OOO_SDK_DIR}/bin/cppumaker -Gc -BUCR -O ${PROJECT_SOURCE_DIR}/include/openoffice ${OOO_BASIS_DIR}/ure-link/share/misc/types.rdb ${OOO_BASIS_DIR}/program/offapi.rdb)
execute_process(COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb)
execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regmerge ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb / "${OOO_BASIS_DIR}/ure-link/share/misc/types.rdb" "${OOO_BASIS_DIR}/program/offapi.rdb")
execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c connector.uno.so)
execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c remotebridge.uno.so)
execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c bridgefac.uno.so)
execute_process(COMMAND ${OOO_BASIS_DIR}/ure-link/bin/regcomp -register -r ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb -c uuresolver.uno.so)
#execute_process(COMMAND bash -c "echo #!/bin/sh > ${CMAKE_CURRENT_BINARY_DIR}/../env.sh")
execute_process(COMMAND bash -c "echo \"#!/bin/sh\n\nLD_LIBRARY_PATH=${OOO_BASIS_DIR}/ure-link/lib\nexport LD_LIBRARY_PATH\" > ${CMAKE_CURRENT_BINARY_DIR}/../env.sh")

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

set(GOST_DOC_GEN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../template_fieldnames_keywords.cpp
    common_funcs.cpp
    component.cpp
    component_db.cpp
    dictionaries.cpp
    GOST_comp_lister.cpp
    oo_common.cxx
    oo_component_index.cxx
    oo_specification.cxx
    )

add_library(GOST-doc-gen STATIC ${GOST_DOC_GEN_SRCS})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../GOST-doc-gen.rdb
        DESTINATION ${KICAD_DATA}/GOST-doc-gen
        COMPONENT resources)

install( DIRECTORY templates
        DESTINATION ${KICAD_DATA}/GOST-doc-gen
        COMPONENT resources )
