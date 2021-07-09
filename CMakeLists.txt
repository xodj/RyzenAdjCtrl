cmake_minimum_required(VERSION 3.16.0)
#Project name and version
PROJECT(RyzenAdjCtrl VERSION 0.4.2.725 LANGUAGES CXX)
#Set version number in this  ^^^^^^^^^
#BUILD_STANDALONE on UNIX or choose on Windows
if(WIN32)
    set(BUILD_STANDALONE FALSE CACHE BOOL "Build without service")
    if(BUILD_STANDALONE)
        set(DEF_UNDEF_BUILD_SERVICE "#undef BUILD_SERVICE")
    else(BUILD_STANDALONE)
        set(DEF_UNDEF_BUILD_SERVICE "#define BUILD_SERVICE")
    endif(BUILD_STANDALONE)
else(WIN32)
    set(DEF_UNDEF_BUILD_SERVICE "#undef BUILD_SERVICE")
endif(WIN32)
#Generate and place key for shared memory in CtrlConfig.h
string(RANDOM LENGTH 512 RANDOM_DATA)
configure_file(CtrlConfig.h.in ${PROJECT_BINARY_DIR}/CtrlConfig.h)
#Cmake+Qt default args
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#Check Qt PATH
if(NOT CMAKE_PREFIX_PATH)
    message("Please add QTDIR to CMAKE_PREFIX_PATH")
    set(CMAKE_PREFIX_PATH "D:/Qt/6.0.4/msvc2019_64")
endif()
#Find Qt Libraries
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets DBus REQUIRED)
#Source Files:
set(RyzenAdjCtrl_HEADERS
    CtrlAgent.h
    ${PROJECT_BINARY_DIR}/CtrlConfig.h
    CtrlConfig.h.in
    CtrlGui.h
    CtrlService.h
    CtrlSettings.h
    CtrlSpinBox.h
    CtrlComboBox.h
    CtrlBus.h
    CtrlACCallback.h
    CtrlFrame.h
    )
set(RyzenAdjCtrl_SOURCES
    CtrlGui.cpp
    CtrlMain.cpp
    CtrlService.cpp
    CtrlSettings.cpp
    )
set(RyzenAdjCtrl_QRC
    RyzenAdjCtrl.qrc
    )
set(RyzenAdjCtrl_UI
    CtrlAPUForm.ui
    CtrlInfoWidget.ui
    CtrlMainWindow.ui
    CtrlSettingsForm.ui
    )
set(RyzenAdjCtrl_TS_FILES
    translate/RyzenAdjCtrl_ru_RU.ts
    )
#Only Windows Sources
if(WIN32)
    message("WINDOWS OS")
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
    set(RyzenAdjCtrl_SOURCES
        ${RyzenAdjCtrl_SOURCES}
        CtrlEPMCallback.cpp
        CtrlEPMCallback.h
        CtrlArmour.h
        RyzenAdjCtrl.rc.in
        RyzenAdjCtrl.iss.in
        ${PROJECT_BINARY_DIR}/RyzenAdjCtrl.rc
        ${PROJECT_BINARY_DIR}/RyzenAdjCtrl.iss
        )
    set(RyzenAdjLib
        ${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/build/libryzenadj.lib
        )
endif(WIN32)
#Linux
if(UNIX)
    message("UNIX OS")
    set(RyzenAdjLib
        ${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/build/libryzenadjd.so
        )
endif(UNIX)
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
    Qt${QT_VERSION_MAJOR}::DBus
    ${RyzenAdjLib}
    )
if(WIN32)
#Set show or not CLI
set_target_properties(RyzenAdjCtrl
    PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#COPY DEPS
include(CopyDependencies.cmake)
endif(WIN32)
#Set output file path
set_target_properties(RyzenAdjCtrl
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Appfolder")
#Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
