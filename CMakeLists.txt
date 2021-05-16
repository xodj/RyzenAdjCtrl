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
    ${CMAKE_CURRENT_SOURCE_DIR}/RyzenAdj/build/${CMAKE_BUILD_TYPE}/libryzenadj.lib
    )
#Set show or not CLI
set_target_properties(RyzenAdjCtrl
    PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/RyzenAdj)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
