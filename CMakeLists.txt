cmake_minimum_required(VERSION 3.16.0)
#Project name and version
PROJECT(RyzenCtrl VERSION 0.5.2.924 LANGUAGES CXX)
#Set version number here  ^^^^^^^^^
#BUILD_STANDALONE
set(BUILD_STANDALONE FALSE CACHE BOOL "Build without service")
if(BUILD_STANDALONE)
    set(DEF_UNDEF_BUILD_SERVICE "#undef BUILD_SERVICE")
else(BUILD_STANDALONE)
    set(DEF_UNDEF_BUILD_SERVICE "#define BUILD_SERVICE")
endif(BUILD_STANDALONE)
#Generate and place key for shared memory in CtrlConfig.h
string(RANDOM LENGTH 32 RANDOM_DATA)
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
set(RyzenCtrl_HEADERS
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
set(RyzenCtrl_SOURCES
    CtrlGui.cpp
    CtrlMain.cpp
    CtrlService.cpp
    CtrlSettings.cpp
    )
set(RyzenCtrl_QRC
    RyzenCtrl.qrc
    )
set(RyzenCtrl_UI
    CtrlAPUForm.ui
    CtrlInfoWidget.ui
    CtrlMainWindow.ui
    CtrlSettingsForm.ui
    )
set(RyzenCtrl_TS_FILES
    translate/RyzenCtrl_ru_RU.ts
    )
#Only Windows Sources
if(WIN32)
    message("WINDOWS OS")
    #Place version to .rc
    set(STR_CTRL_VERSION ${RyzenCtrl_VERSION})
    string(REPLACE "." "." RC_SPLIT ${STR_CTRL_VERSION})
    string(REPLACE "." ";" RC_SPLIT ${RC_SPLIT})
    message("\nRyzenCtrl is now: ${STR_CTRL_VERSION}\n")
    list(GET RC_SPLIT 0 CTRL_VERSION)
    list(GET RC_SPLIT 1 CTRL_MAJOR_VERSION)
    list(GET RC_SPLIT 2 CTRL_MINOR_VERSION)
    list(GET RC_SPLIT 3 CTRL_PATCH_VERSION)
    configure_file(RyzenCtrl.rc.in ${PROJECT_BINARY_DIR}/RyzenCtrl.rc)
    #Place version to Inno Setup Project
    configure_file(RyzenCtrl.iss.in ${PROJECT_BINARY_DIR}/RyzenCtrl.iss)
    set(RyzenCtrl_SOURCES
        ${RyzenCtrl_SOURCES}
        CtrlEPMCallback.cpp
        CtrlEPMCallback.h
        CtrlArmour.h
        RyzenCtrl.rc.in
        RyzenCtrl.iss.in
        ${PROJECT_BINARY_DIR}/RyzenCtrl.rc
        ${PROJECT_BINARY_DIR}/RyzenCtrl.iss
        )
    set(RyzenAdjLib
        ${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj/build/libryzenadj.lib
        )
endif(WIN32)
#Linux
if(UNIX)
    message("UNIX OS")
    set(RyzenAdjLib
        /usr/lib/libryzenadj.so
        Qt${QT_VERSION_MAJOR}::DBus
        )
    set(RyzenCtrl_SOURCES
        ${RyzenCtrl_SOURCES}
        CtrlArmour.h
        ru.ryzenctrl.service.conf
        ryzenctrl-root.service
        )
endif(UNIX)
#Add project exec
add_executable(RyzenCtrl
        ${RyzenCtrl_SOURCES}
        ${RyzenCtrl_HEADERS}
        ${RyzenCtrl_QRC}
        ${RyzenCtrl_UI}
        ${RyzenCtrl_TS_FILES}
        )
#Link libraries
target_link_libraries(RyzenCtrl PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    ${RyzenAdjLib}
    )
if(WIN32)
#Set show or not CLI
set_target_properties(RyzenCtrl
    PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#COPY DEPS
include(CopyDependencies.cmake)
endif(WIN32)
#Set output file path
set_target_properties(RyzenCtrl
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Appfolder")
#Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RyzenAdj)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ryzenadj)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ryzenadj-git)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
