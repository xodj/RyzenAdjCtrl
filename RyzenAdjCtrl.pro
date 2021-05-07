QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = RyzenAdjCtrl
RC_ICONS = ./amd_icon.ico
VERSION = 0.2.4.181
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
    CtrlMainWindow.ui \
    CtrlSettingsForm.ui

TRANSLATIONS += \
    Appfolder/Language/RyzenAdjCtrl_ru_RU.ts

CODECFORSRC     = UTF-8

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
