QT       += core gui widgets

CONFIG += c++11

TARGET = RyzenAdjCtrl
RC_ICONS = ./amd_icon.ico
VERSION = 0.3.0.334
QMAKE_TARGET_COMPANY = "xo.dj@ya.ru"
QMAKE_TARGET_DESCRIPTION = "GUI for RyzenAdj"
QMAKE_TARGET_COPYRIGHT = "GPL-3.0 License"
QMAKE_TARGET_PRODUCT = "RyzenAdjCtrl"
QMAKE_TARGET_ORIGINAL_FILENAME = RyzenAdjCtrl.exe

RESOURCES += \
    RyzenAdjRes.qrc

SOURCES += \
    CtrlAgent.cpp \
    CtrlEPMCallback.cpp \
    CtrlGui.cpp \
    CtrlMain.cpp \
    CtrlService.cpp \
    CtrlSettings.cpp

HEADERS += \
    CtrlAgent.h \
    CtrlConfig.h \
    CtrlEPMCallback.h \
    CtrlGui.h \
    CtrlService.h \
    CtrlSettings.h

FORMS += \
    CtrlAPUForm.ui \
    CtrlInfoWidget.ui \
    CtrlMainWindow.ui \
    CtrlSettingsForm.ui

TRANSLATIONS += \
    Appfolder/Language/RyzenAdjCtrl_ru_RU.ts

CODECFORSRC     = UTF-8

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/RyzenAdj/build/release/ -llibryzenadj
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/RyzenAdj/build/debug/ -llibryzenadj

INCLUDEPATH += $$PWD/RyzenAdj/
DEPENDPATH += $$PWD/RyzenAdj/
