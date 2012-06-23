
include_directories(BEFORE ${INC_BEFORE})
include_directories(
    bitmaps
    dialogs
    ../polygon
    ../common
    ${INC_AFTER}
    )

set(PCAD2KICAD_SRCS
    LoadInputFile.cpp
    pcad2kicad.cpp
    pcad2kicad_frame.cpp
    PCB.cpp
    PCBArc.cpp
    PCBComponent.cpp
    PCBCopperPour.cpp
    PCBCutout.cpp
    PCBLine.cpp
    PCBModule.cpp
    PCBNet.cpp
    PCBPad.cpp
    PCBPadShape.cpp
    PCBPolygon.cpp
    PCBText.cpp
    PCBVia.cpp
    PCBViaShape.cpp
    Sch.cpp
    SchArc.cpp
    SchBus.cpp
    SchComponent.cpp
    SchJunction.cpp
    SchLine.cpp
    SchModule.cpp
    SchPin.cpp
    SchSymbol.cpp
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

target_link_libraries( pcad2kicad
    common
    bitmaps
    polygon
    kbool
    ${wxWidgets_LIBRARIES}
    )

install(TARGETS pcad2kicad
        DESTINATION ${KICAD_BIN}
        COMPONENT binary)

