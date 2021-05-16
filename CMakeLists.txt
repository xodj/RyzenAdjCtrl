cmake_minimum_required(VERSION 3.16.0)
#Project name and version
PROJECT(RyzenAdjCtrl VERSION 0.3.3.487 LANGUAGES CXX)
#Set version number in this  ^^^^^^^^^
#Generate key for shared memory
string(RANDOM LENGTH 512 RANDOM_DATA)
configure_file(CtrlConfig.h.in ${PROJECT_BINARY_DIR}/CtrlConfig.h)
#Place version to .rc
set(STR_CTRL_VERSION ${RyzenAdjCtrl_VERSION})
string(REPLACE "." "." RC_SPLIT ${STR_CTRL_VERSION})
string(REPLACE "." ";" RC_SPLIT ${RC_SPLIT})
message("\nRyzenAdjCtrl is now: ${STR_CTRL_VERSION}\n")
list(GET RC_SPLIT 0 CTRL_VERSION)
list(GET RC_SPLIT 1 CTRL_MAJOR_VERSION)
list(GET RC_SPLIT 2 CTRL_MINOR_VERSION)
list(GET RC_SPLIT 3 CTRL_PATCH_VERSION)
configure_file(RyzenAdjCtrl.rc.in ${PROJECT_BINARY_DIR}/RyzenAdjCtrl.rc)
#Place version to Inno Setup Project
configure_file(RyzenAdjCtrl.iss.in ${PROJECT_BINARY_DIR}/RyzenAdjCtrl.iss)
#Cmake+Qt default args
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#Check Qt PATH
if(NOT CMAKE_PREFIX_PATH)
    message(Please add QTDIR to CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/Qt/6.0.4/msvc2019_64")
endif()
#Find Qt Libraries
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
#Source Files:
set(RyzenAdjCtrl_HEADERS
    CtrlAgent.h
    ${PROJECT_BINARY_DIR}/CtrlConfig.h
    CtrlConfig.h.in
    CtrlEPMCallback.h
    CtrlGui.h
    CtrlService.h
    CtrlSettings.h
    )
set(RyzenAdjCtrl_SOURCES
    CtrlAgent.cpp
    CtrlEPMCallback.cpp
    CtrlGui.cpp
    CtrlMain.cpp
    CtrlService.cpp
    CtrlSettings.cpp
    ${PROJECT_BINARY_DIR}/RyzenAdjCtrl.rc
    RyzenAdjCtrl.rc.in
    ${PROJECT_BINARY_DIR}/RyzenAdjCtrl.iss
    RyzenAdjCtrl.iss.in
    )
set(RyzenAdjCtrl_QRC
    RyzenAdjRes.qrc
    )
set(RyzenAdjCtrl_UI
    CtrlAPUForm.ui
    CtrlInfoWidget.ui
    CtrlMainWindow.ui
    CtrlSettingsForm.ui
    )
set(RyzenAdjCtrl_TS_FILES
    Appfolder/Language/RyzenAdjCtrl_ru_RU.ts
    )
#Add project exec
add_executable(RyzenAdjCtrl
        ${RyzenAdjCtrl_SOURCES}
        ${RyzenAdjCtrl_HEADERS}
        ${RyzenAdjCtrl_QRC}
        ${RyzenAdjCtrl_UI}
        ${RyzenAdjCtrl_TS_FILES}
        )
#Link libraries
target_link_libraries(RyzenAdjCtrl PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/build/libryzenadj.lib
    )
#Set show or not CLI
set_target_properties(RyzenAdjCtrl
    PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
set_target_properties(RyzenAdjCtrl
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/APPFOLDER")
#Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
#COPY DEPS FOR RELEASE BUILD AND MAKE INSTALL
if(CMAKE_BUILD_TYPE MATCHES "Release")
message("\nIt's Release.\nCopy deps...")
configure_file(Appfolder/Config/StyleSheet.xml ${PROJECT_BINARY_DIR}/Appfolder/Config/StyleSheet.xml COPYONLY)
configure_file(Appfolder/Theme/checkbox_checked.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/checkbox_checked.png COPYONLY)
configure_file(Appfolder/Theme/checkbox_checked_disabled.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/checkbox_checked_disabled.png COPYONLY)
configure_file(Appfolder/Theme/checkbox_checked_focus.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/checkbox_checked_focus.png COPYONLY)
configure_file(Appfolder/Theme/checkbox_unchecked.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/checkbox_unchecked.png COPYONLY)
configure_file(Appfolder/Theme/checkbox_unchecked_disabled.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/checkbox_unchecked_disabled.png COPYONLY)
configure_file(Appfolder/Theme/checkbox_unchecked_focus.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/checkbox_unchecked_focus.png COPYONLY)
configure_file(Appfolder/Theme/down_arrow.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/down_arrow.png COPYONLY)
configure_file(Appfolder/Theme/down_arrow_disabled.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/down_arrow_disabled.png COPYONLY)
configure_file(Appfolder/Theme/up_arrow.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/up_arrow.png COPYONLY)
configure_file(Appfolder/Theme/up_arrow_disabled.png ${PROJECT_BINARY_DIR}/Appfolder/Theme/up_arrow_disabled.png COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../atrofac/bin/atrofac-cli.exe ${PROJECT_BINARY_DIR}/Appfolder/Binaries/atrofac-cli.exe COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/build/libryzenadj.dll ${PROJECT_BINARY_DIR}/Appfolder/libryzenadj.dll COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/win32/inpoutx64.dll ${PROJECT_BINARY_DIR}/Appfolder/inpoutx64.dll COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/win32/WinRing0x64.dll ${PROJECT_BINARY_DIR}/Appfolder/WinRing0x64.dll COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/win32/WinRing0x64.sys ${PROJECT_BINARY_DIR}/Appfolder/WinRing0x64.sys COPYONLY)
IF(${QT_VERSION_MAJOR} MATCHES 6)
message("Copy Qt6 deps...\n")
configure_file(${CMAKE_PREFIX_PATH}/bin/Qt6Core.dll ${PROJECT_BINARY_DIR}/Appfolder/Qt6Core.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/bin/Qt6Gui.dll ${PROJECT_BINARY_DIR}/Appfolder/Qt6Gui.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/bin/Qt6Widgets.dll ${PROJECT_BINARY_DIR}/Appfolder/Qt6Widgets.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/plugins/imageformats/qico.dll ${PROJECT_BINARY_DIR}/Appfolder/ImageFormats/qico.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll ${PROJECT_BINARY_DIR}/Appfolder/Platforms/qwindows.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/plugins/styles/qwindowsvistastyle.dll ${PROJECT_BINARY_DIR}/Appfolder/Styles/qwindowsvistastyle.dll COPYONLY)
ENDIF(${QT_VERSION_MAJOR} MATCHES 6)
IF(${QT_VERSION_MAJOR} MATCHES 5)
message("Copy Qt5 deps...\n")
configure_file(${CMAKE_PREFIX_PATH}/bin/Qt5Core.dll ${PROJECT_BINARY_DIR}/Appfolder/Qt5Core.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/bin/Qt5Gui.dll ${PROJECT_BINARY_DIR}/Appfolder/Qt5Gui.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/bin/Qt5Widgets.dll ${PROJECT_BINARY_DIR}/Appfolder/Qt5Widgets.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/plugins/imageformats/qico.dll ${PROJECT_BINARY_DIR}/Appfolder/ImageFormats/qico.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll ${PROJECT_BINARY_DIR}/Appfolder/Platforms/qwindows.dll COPYONLY)
configure_file(${CMAKE_PREFIX_PATH}/plugins/styles/qwindowsvistastyle.dll ${PROJECT_BINARY_DIR}/Appfolder/Styles/qwindowsvistastyle.dll COPYONLY)
ENDIF(${QT_VERSION_MAJOR} MATCHES 5)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${PROJECT_BINARY_DIR}/LICENSE COPYONLY)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")
