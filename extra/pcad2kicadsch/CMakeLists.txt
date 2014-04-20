
include_directories(BEFORE ${INC_BEFORE})
include_directories(
    dialogs
    ../../polygon
    ../../pcbnew
    ../../common
    ../../pcbnew/pcad2kicadpcb_plugin
    ${INC_AFTER}
    )

set(PCAD2KICADSCH_SRCS
    pcad2kicad_frame.cpp
    ../../pcbnew/pcad2kicadpcb_plugin/s_expr_loader.cpp
    sch.cpp
    sch_arc.cpp
    sch_bus.cpp
    sch_component.cpp
    sch_junction.cpp
    sch_line.cpp
    sch_module.cpp
    sch_pin.cpp
    sch_port.cpp
    sch_sheet.cpp
    sch_symbol.cpp
    sch_text.cpp
    ../../pcbnew/pcad2kicadpcb_plugin/pcad2kicad_common.cpp
    dialogs/pcad2kicad_frame_base.cpp
    )

if(WIN32)
    if(MINGW)
        # PCAD2KICADSCH_RESOURCES variable is set by the macro.
        mingw_resource_compiler(pcad2kicadsch)
    else(MINGW)
        set(PCAD2KICADSCH_RESOURCES pcad2kicadsch.rc)
    endif(MINGW)
endif(WIN32)


if(APPLE)
    set(PCAD2KICADSCH_RESOURCES pcad2kicadsch.icns)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/pcad2kicadsch.icns"
                                PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_ICON_FILE pcad2kicadsch.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER org.kicad-eda.pcad2kicadsch)
    set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif(APPLE)


if( USE_KIWAY_DLLS )
    add_executable( pcad2kicadsch WIN32 MACOSX_BUNDLE
        ../../common/single_top.cpp
        ../../common/pgm_base.cpp
        ${PCAD2KICADSCH_RESOURCES}
        )
    set_source_files_properties( ../../common/single_top.cpp PROPERTIES
        COMPILE_DEFINITIONS "TOP_FRAME=0;BUILD_KIWAY_DLL"
        )
    target_link_libraries( pcad2kicadsch
        #singletop  # replaces common, giving us restrictive control and link warnings.
        # There's way too much crap coming in from common yet.
        common
        bitmaps
        ${wxWidgets_LIBRARIES}
        )
    if( MAKE_LINK_MAPS )
        set_target_properties( pcad2kicadsch PROPERTIES
            LINK_FLAGS "${TO_LINKER},-cref ${TO_LINKER},-Map=pcad2kicadsch.map" )
    endif()

    # the main pcad2kicadsch program, in DSO form.
    add_library( pcad2kicadsch_kiface MODULE
        pcad2kicad.cpp
        ${PCAD2KICADSCH_SRCS}
        #${PCAD2KICADSCH_RESOURCES}
        )
    set_target_properties( pcad2kicadsch_kiface PROPERTIES
        OUTPUT_NAME     pcad2kicadsch
        PREFIX          ${KIFACE_PREFIX}
        SUFFIX          ${KIFACE_SUFFIX}
        )
    target_link_libraries( pcad2kicadsch_kiface
        common
        bitmaps
        polygon
        ${wxWidgets_LIBRARIES}
        )
    set_source_files_properties( pcad2kicad.cpp PROPERTIES
        # The KIFACE is in pcad2kicad.cpp, export it:
        COMPILE_DEFINITIONS     "BUILD_KIWAY_DLL;COMPILING_DLL"
        )
    if( MAKE_LINK_MAPS )
        set_target_properties( pcad2kicadsch_kiface PROPERTIES
            LINK_FLAGS "${TO_LINKER},-cref ${TO_LINKER},-Map=_pcad2kicadsch.kiface.map" )
    endif()

    # if building pcad2kicadsch, then also build pcad2kicadsch_kiface if out of date.
    add_dependencies( pcad2kicadsch pcad2kicadsch_kiface )

    # these 2 binaries are a matched set, keep them together
    install( TARGETS pcad2kicadsch
        DESTINATION ${KICAD_BIN}
        COMPONENT binary
        )
    install( TARGETS pcad2kicadsch_kiface
        DESTINATION ${KICAD_BIN}
        COMPONENT binary
        )

    if( APPLE )
        set_target_properties( pcad2kicadsch PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
            )

        # copies kiface into the bundle
        add_custom_target( _pcad2kicadsch_kiface_copy ALL
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/extra/pcad2kicadsch/_pcad2kicadsch.kiface "${CMAKE_BINARY_DIR}/extra/pcad2kicadsch/pcad2kicadsch.app/Contents/MacOS/"
            DEPENDS pcad2kicadsch_kiface
            COMMENT "Copying kiface into pcad2kicadsch"
        )
    endif()
else()
    add_executable( pcad2kicadsch WIN32 MACOSX_BUNDLE
        ../../common/single_top.cpp
        pcad2kicad.cpp
        ${PCAD2KICADSCH_SRCS}
        ${PCAD2KICADSCH_RESOURCES}
        )
    set_source_files_properties( ../../common/single_top.cpp PROPERTIES
        COMPILE_DEFINITIONS     "TOP_FRAME=0"
        )
    set_source_files_properties( pcad2kicad.cpp PROPERTIES
        COMPILE_DEFINITIONS     "COMPILING_DLL"
        )

    if( APPLE )
        set_target_properties( pcad2kicadsch PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
            )
    endif()

    target_link_libraries( pcad2kicadsch
        common
        bitmaps
        polygon
        ${wxWidgets_LIBRARIES}
        )
    install( TARGETS pcad2kicadsch
        DESTINATION ${KICAD_BIN}
        COMPONENT binary
        )
endif()
