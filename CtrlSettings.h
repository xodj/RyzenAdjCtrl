#ifndef CTRLSETTINGS_H
#define CTRLSETTINGS_H

#include <QObject>
#include <QFile>
#include "CtrlConfig.h"

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
    int apuSkinValue = 85;
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
    //NEW VARS
    int vrmSocCurrent = 10;
    bool vrmSocCurrentChecked = false;
    int vrmSocMax = 13;
    bool vrmSocMaxChecked = false;

    int psi0Current = 13;
    bool psi0CurrentChecked = false;
    int psi0SocCurrent = 5;
    bool psi0SocCurrentChecked = false;

    int maxLclk = 1200;
    bool maxLclkChecked = false;
    int minLclk = 400;
    bool minLclkChecked = false;

    int prochotDeassertionRamp = 2;
    bool prochotDeassertionRampChecked = false;

    int dgpuSkinTempLimit = 85;
    bool dgpuSkinTempLimitChecked = false;
    int apuSlowLimit = 25;
    bool apuSlowLimitChecked = false;
    int skinTempPowerLimit = 25;
    bool skinTempPowerLimitChecked = false;
};

struct settingsStr {
    bool useAgent = true;
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

    bool hideNotSupportedVariables = false;
    int apuFamilyIdx = 0;

    bool showArmourPlugin = false;
};

struct hideShow {
    int shwStapmLimit;
    int shwFastLimit;
    int shwSlowLimit;
    int shwSlowTime;
    int shwStapmTime;
    int shwTctlTemp;
    int shwVrmCurrent;
    int shwVrmSocCurrent;
    int shwVrmMaxCurrent;
    int shwVrmSocMaxCurrent;
    int shwPsi0Current;
    int shwPsi0SocCurrent;
    int shwMaxSocclkFrequency;
    int shwMinSocclkFrequency;
    int shwMaxFclkFrequency;
    int shwMinFclkFrequency;
    int shwMaxVcn;
    int shwMinVcn;
    int shwMaxLclk;
    int shwMinLclk;
    int shwMaxGfxclk;
    int shwMinGfxclk;
    int shwProchotDeassertionRamp;
    int shwApuSkinTemp;
    int shwDgpuSkinTemp;
    int shwApuSlowLimit;
    int shwSkinTempLimit;
    int shwPowerSaving;
    int shwMaxPerformance;
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

    settingsStr* getSettingsBuffer();
    const QList<presetStr*>* getPresetsList();
    qsizetype getPresetsCount();
    bool setPresetBuffer(int idx, presetStr* preset);
    presetStr* getPresetBuffer(int idx);
    int insertNewPreset(int newidx = -1, presetStr* newPreset = nullptr);
    bool deletePreset(int idx);

    hideShow *hideShowWarnPresetVariable(int idx){
        switch(idx){
        case 0:
            return &shwpvRaven;
        case 1:
            return &shwpvPicasso;
        case 2:
            return &shwpvRenoir;
        case 3:
            return &shwpvCezanne;
        case 4:
            return &shwpvDali;
        case 5:
            return &shwpvLucienne;
        default:
            return &shwpvShowAll;
        }
        return &shwpvShowAll;
    }

private:
    settingsStr settingsBuffer;
    QList<presetStr*> *presets;

    QFile *configQFile;
    QFile *presetsQFile;

    hideShow shwpvRaven = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        2,
        2,
        1,
        0,
        0,
        0,
        0,
        1,
        1
    };

    hideShow shwpvPicasso = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
        1,
        1
    };

    hideShow shwpvRenoir = {
        2,
        1,
        1,
        1,
        2,
        1,
        1,
        1,
        1,
        1,
        2,
        2,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        2,
        2,
        2,
        2,
        1,
        1
    };

    hideShow shwpvCezanne = {
        2,
        1,
        1,
        1,
        2,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        2,
        2,
        2,
        2,
        1,
        1
    };

    hideShow shwpvDali = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
        1,
        1
    };

    hideShow shwpvLucienne = {
        2,
        1,
        1,
        1,
        2,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        1,
        1,
        1,
        1,
        1,
        1
    };

    hideShow shwpvShowAll = {
        2,
        1,
        1,
        1,
        2,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        1,
        1,
        1,
        1,
        1,
        1,
        1
    };
};

#endif // CTRLSETTINGS_H
