
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
    pcad2kicad.cpp
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
        set(PCAD2KICADSCH_RESOURCES pcad2kicad.rc)
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


add_executable(pcad2kicadsch WIN32 MACOSX_BUNDLE
    ${PCAD2KICADSCH_SRCS}
    ${PCAD2KICADSCH_RESOURCES}
    )


if(APPLE)
    set_target_properties(pcad2kicadsch PROPERTIES MACOSX_BUNDLE_INFO_PLIST
        ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif(APPLE)

target_link_libraries(pcad2kicadsch
    common
    bitmaps
    polygon
    ${wxWidgets_LIBRARIES}
    )

install(TARGETS pcad2kicadsch
        DESTINATION ${KICAD_BIN}
        COMPONENT binary)

