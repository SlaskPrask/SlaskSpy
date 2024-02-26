cmake_minimum_required(VERSION 3.5)

project(SlaskSpy VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(INCLUDE_COMMON "include/common")
set(SRC_COMMON "src/common")

set(INCLUDE_QT "include/qt")
set(SRC_QT "src/qt")

set(QT_PROJECT_SOURCES
        ${SRC_QT}/main.cpp
        ${SRC_QT}/mainwindow.cpp
        ${SRC_QT}/viewerwindow.cpp
        ${SRC_QT}/window_manager.cpp
        ${SRC_QT}/qt_graphics_wrapper.cpp
        ${SRC_QT}/qt_input_items.cpp

        ${SRC_QT}/mainwindow.ui
        ${SRC_QT}/viewerwindow.ui

        ${INCLUDE_QT}/mainwindow.h
        ${INCLUDE_QT}/viewerwindow.h
        ${INCLUDE_QT}/window_manager.h
        ${INCLUDE_QT}/qt_graphics_wrapper.h
        ${INCLUDE_QT}/qt_input_items.h
        ${INCLUDE_COMMON}/graphics_wrapper.h
        ${INCLUDE_COMMON}/skin_settings.h
        ${SRC_COMMON}/skin_settings.cpp
        ${INCLUDE_COMMON}/input_items.h
        ${INCLUDE_COMMON}/viewer.h
        ${INCLUDE_COMMON}/viewers/n64_viewer.h
        ${SRC_COMMON}/viewers/n64_viewer.cpp
        ${INCLUDE_COMMON}/com_ports.h
        ${SRC_COMMON}/com_ports.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SlaskSpy
        MANUAL_FINALIZATION
        ${QT_PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SlaskSpy APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(SlaskSpy SHARED
            ${QT_PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SlaskSpy
            ${QT_PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(SlaskSpy PRIVATE ${INCLUDE_QT} ${INCLUDE_COMMON})
target_link_libraries(SlaskSpy PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Setupapi)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SlaskSpy)
endif()
set_target_properties(SlaskSpy PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SlaskSpy
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SlaskSpy)
endif()
