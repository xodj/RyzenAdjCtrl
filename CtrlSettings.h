#ifndef CTRLSETTINGS_H
#define CTRLSETTINGS_H

#include <QObject>

enum enumFanPresetId {
    None = 0,
    Windows,
    Silent,
    Perfomance,
    Turbo,
    Manual
};

struct presetStr {
    int presetId = 0;
    QString presetName = "Preset Name";

    int fanPresetId = None;

    int tempLimitValue = 85;
    bool tempLimitChecked = false;
    int apuSkinValue = 105;
    bool apuSkinChecked = false;

    int stampLimitValue = 25;
    bool stampLimitChecked = false;
    int fastLimitValue = 25;
    bool fastLimitChecked = false;
    int fastTimeValue = 900;
    bool fastTimeChecked = false;
    int slowLimitValue = 10;
    bool slowLimitChecked = false;
    int slowTimeValue = 60;
    bool slowTimeChecked = false;

    int vrmCurrentValue = 25;
    bool vrmCurrentChecked = false;
    int vrmMaxValue = 45;
    bool vrmMaxChecked = false;

    int minFclkValue = 800;
    bool minFclkChecked = false;
    int maxFclkValue = 1200;
    bool maxFclkChecked = false;

    int minGfxclkValue = 400;
    bool minGfxclkChecked = false;
    int maxGfxclkValue = 2200;
    bool maxGfxclkChecked = false;
    int minSocclkValue = 800;
    bool minSocclkChecked = false;
    int maxSocclkValue = 1200;
    bool maxSocclkChecked = false;
    int minVcnValue = 400;
    bool minVcnChecked = false;
    int maxVcnValue = 1200;
    bool maxVcnChecked = false;

    bool smuMaxPerfomance = false;
    bool smuPowerSaving = false;
};

struct settingsStr {
    bool useAgent = false;
    bool showNotifications = true;

    bool showReloadStyleSheetButton = false;
    bool showNotificationToDisableAutoSwitcher = false;

    bool autoPresetApplyDurationChecked = false;
    int autoPresetApplyDuration = 180;

    bool autoPresetSwitchAC = false;
    int dcStatePresetId = 1;
    int acStatePresetId = 2;

    bool epmAutoPresetSwitch = false;
    int epmBatterySaverPresetId = 0;
    int epmBetterBatteryPresetId = 1;
    int epmBalancedPresetId = 2;
    int epmMaximumPerfomancePresetId = 3;
};

class CtrlSettings : public QObject
{
    Q_OBJECT

public:
    CtrlSettings();
    ~CtrlSettings();
    bool saveSettings();
    bool openSettings();
    bool savePresets();
    bool openPresets();
    presetStr **presetsBuffer;
    settingsStr *settingsBuffer;

};

#endif // CTRLSETTINGS_H
