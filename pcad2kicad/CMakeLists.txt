
include_directories(BEFORE ${INC_BEFORE})
include_directories(
    bitmaps
    dialogs
    ../polygon
    ../pcbnew
    ../common
    ${INC_AFTER}
    )

set(PCAD2KICAD_SRCS
    LoadInputFile.cpp
    pcad2kicad.cpp
    pcad2kicad_frame.cpp
#    pcb.cpp
#    pcb_arc.cpp
#    pcb_component.cpp
#    pcb_copper_pour.cpp
#    pcb_cutout.cpp
#    pcb_line.cpp
#    pcb_module.cpp
#    pcb_net.cpp
#    pcb_pad.cpp
#    pcb_pad_shape.cpp
#    pcb_polygon.cpp
#    pcb_text.cpp
#    pcb_via.cpp
#    pcb_via_shape.cpp
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
    TextToXMLUnit.cpp
    XMLtoObjectCommonProceduresUnit.cpp
    dialogs/pcad2kicad_frame_base.cpp
    )

if(WIN32)
    if(MINGW)
        # PCAD2KICAD_RESOURCES variable is set by the macro.
        mingw_resource_compiler(pcad2kicad)
    else(MINGW)
        set(PCAD2KICAD_RESOURCES pcad2kicad.rc)
    endif(MINGW)
endif(WIN32)


if(APPLE)
    set(PCAD2KICAD_RESOURCES pcad2kicad.icns)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/pcad2kicad.icns"
                                PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_ICON_FILE pcad2kicad.icns)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER org.kicad-eda.pcad2kicad)
    set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif(APPLE)


add_executable(pcad2kicad WIN32 MACOSX_BUNDLE
    ${PCAD2KICAD_SRCS}
    ${PCAD2KICAD_RESOURCES}
    )


if(APPLE)
    set_target_properties(pcad2kicad PROPERTIES MACOSX_BUNDLE_INFO_PLIST
        ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif(APPLE)

target_link_libraries(pcad2kicad
    common
    bitmaps
    polygon
    kbool
    ${wxWidgets_LIBRARIES}
    )

install(TARGETS pcad2kicad
        DESTINATION ${KICAD_BIN}
        COMPONENT binary)

