#include "CtrlEPMCallback.h"

#define _WIN32_WINNT 0x1809

#include <Windows.h>
#include <PowrProf.h>

#pragma comment(lib, "PowrProf.lib")

CtrlEPMCallback *epmCallbackMainClass;

void epmCallback(EFFECTIVE_POWER_MODE EPM, void*) {
    epmMode currentEPM;
    switch(int(EPM)){
    case 0:
        currentEPM = BatterySaver;
        break;
    case 1:
        currentEPM = BetterBattery;
        break;
    case 2:
        currentEPM = Balanced;
        break;
    case 3:
        currentEPM = MaxPerformance;
        break;
    case 4:
        currentEPM = MaxPerformance;
        break;
    case 5:
        currentEPM = GameMode;
        break;
    case 6:
        currentEPM = MixedReality;
        break;
    }

    emit epmCallbackMainClass->epmIdChanged(currentEPM);
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
