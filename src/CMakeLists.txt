cmake_minimum_required(VERSION 3.5)

project(TaggerTool VERSION 1.1.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(PROJECT_SOURCES
    custom_item.h custom_item.cpp
    point_item.cpp point_item.h
    bbox_item.cpp bbox_item.h
    line_item.cpp line_item.h
    polygon_item.cpp polygon_item.h
    circle_item.cpp circle_item.h
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
    labeltreemodel.cpp labeltreemodel.h
    import_export/yolo.cpp     import_export/yolo.h
    import_export/coco.h import_export/coco.cpp
    import_export/native.h import_export/native.cpp
    resource.qrc
)


qt_add_executable(TaggerTool
    WIN32 MACOSX_BUNDLE
    ${PROJECT_SOURCES}
)

target_link_libraries(TaggerTool PRIVATE Qt::Core Qt::Widgets)


include(GNUInstallDirs)

install(TARGETS TaggerTool
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET TaggerTool
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
