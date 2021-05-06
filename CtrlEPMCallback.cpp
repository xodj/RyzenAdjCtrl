#include "CtrlEPMCallback.h"

#define _WIN32_WINNT 0x1809

#include <Windows.h>
#include <PowrProf.h>

#pragma comment(lib, "PowrProf.lib")

CtrlEPMCallback *epmCallbackMainClass;

void epmCallback(EFFECTIVE_POWER_MODE EPM, void*) {
    epmMode epm;
    switch(int(EPM)){
    case 0:
        epm = BatterySaver;
        break;
    case 1:
        epm = BetterBattery;
        break;
    case 2:
        epm = Balanced;
        break;
    case 3:
        epm = MaxPerformance;
        break;
    case 4:
        epm = MaxPerformance;
        break;
    case 5:
        epm = GameMode;
        break;
    case 6:
        epm = MixedReality;
        break;
    }

    epmCallbackMainClass->emitEpmIdChanged(epm);
}

CtrlEPMCallback::CtrlEPMCallback() {
    epmHandle = new void*;
    PowerRegisterForEffectivePowerModeNotifications(EFFECTIVE_POWER_MODE_V1, epmCallback,
                                                    nullptr, epmHandle);
    // USE EFFECTIVE_POWER_MODE_V2 For Game and AR Modes
    epmCallbackMainClass = this;
}

CtrlEPMCallback::~CtrlEPMCallback() {
    PowerUnregisterFromEffectivePowerModeNotifications(epmHandle);
}

void CtrlEPMCallback::emitEpmIdChanged(epmMode epm){
    currentEpm = epm;
    emit epmIdChanged(epm);
}

void CtrlEPMCallback::emitCurrentEPMState(){
    if(currentEpm != epmNone)
        emit epmIdChanged(currentEpm);
}

#include <QTimer>

#define currentAc_refresh_time 300

CtrlACCallback::CtrlACCallback(){
    currentAc_refresh_timer = new QTimer;
    connect(currentAc_refresh_timer, &QTimer::timeout,
            this, &CtrlACCallback::checkCurrentACState);
    currentAc_refresh_timer->start(currentAc_refresh_time);
}

CtrlACCallback::~CtrlACCallback(){
    disconnect(currentAc_refresh_timer, &QTimer::timeout,
               this, &CtrlACCallback::checkCurrentACState);
    currentAc_refresh_timer->stop();
}

void CtrlACCallback::checkCurrentACState(){
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps))
        if(currentACState != ACState(sps.ACLineStatus)) {
            currentACState = ACState(sps.ACLineStatus);
            emit currentACStateChanged(currentACState);
        }
}

void CtrlACCallback::emitCurrentACState(){
    if(currentACState != ACNone)
        emit currentACStateChanged(currentACState);
}
