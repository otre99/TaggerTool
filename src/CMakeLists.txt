cmake_minimum_required(VERSION 3.5)

project(TaggerTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

set(PROJECT_SOURCES
    custom_item.h custom_item.cpp
    point_item.cpp point_item.h
    bbox_item.cpp bbox_item.h
    line_item.cpp line_item.h
    polygon_item.cpp polygon_item.h
    #open_polygon_item.cpp open_polygon_item.h
    mainwindow.cpp mainwindow.h mainwindow.ui
    annimgmanager.cpp annimgmanager.h
    imagecanvas.cpp imagecanvas.h
    utils.h utils.cpp
    graphicsview.cpp graphicsview.h
    imgstringlistmodel.cpp imgstringlistmodel.h
    main.cpp
    editdialog.cpp editdialog.h editdialog.ui
    loadimganndialog.cpp loadimganndialog.h loadimganndialog.ui
    dialoglabels.cpp dialoglabels.h dialoglabels.ui
    undo_cmds.h  undo_cmds.cpp
    dialogexporter.h dialogexporter.cpp dialogexporter.ui
    heavytaskthread.h heavytaskthread.cpp
    resource.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TaggerTool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TaggerTool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TaggerTool SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TaggerTool
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TaggerTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(TaggerTool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS TaggerTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TaggerTool)
endif()

# Generate the deployment script for the target MyApp.
qt_generate_deploy_app_script(
    TARGET TaggerTool
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

# Call the deployment script on "cmake --install".
install(SCRIPT ${deploy_script})


set(CPACK_PACKAGE_NAME TaggerTool)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple tagger tool for computer vision annotations")
set(CPACK_PACKAGE_VENDOR "RCCR")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_VERBATIM_VARIABLES ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/TaggerTool")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Roberto Carlos Cruz Rodriguez <roberto.cruz.rdg@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6, libgcc-s1")
include(CPack)
