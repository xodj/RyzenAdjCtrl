#ifndef CTRLSETTINGS_H
#define CTRLSETTINGS_H

#include <QObject>
#include <QFile>
#include "CtrlConfig.h"

struct presetStr {
    int presetId = 0;
    char presetName[17] = {'P', 'r', 'e', 's', 'e', 't', ' ', 'N', 'a', 'm', 'e', '\0', '\0', '\0', '\0', '\0', '\0'};

    int fanPresetId = 0;

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
    int epmGamingPresetId = 4;

    bool hideNotSupportedVariables = false;
    int apuFamilyIdx = 0;

    bool showArmourPlugin = false;
};

struct hideShow {
    bool shwStapmLimit = true;
    bool shwFastLimit = true;
    bool shwSlowLimit = true;
    bool shwSlowTime = true;
    bool shwStapmTime = true;
    bool shwTctlTemp = true;
    bool shwVrmCurrent = true;
    bool shwVrmSocCurrent = true;
    bool shwVrmMaxCurrent = true;
    bool shwVrmSocMaxCurrent = true;
    bool shwPsi0Current = true;
    bool shwPsi0SocCurrent = true;
    bool shwMaxSocclkFrequency = true;
    bool shwMinSocclkFrequency = true;
    bool shwMaxFclkFrequency = true;
    bool shwMinFclkFrequency = true;
    bool shwMaxVcn = true;
    bool shwMinVcn = true;
    bool shwMaxLclk = true;
    bool shwMinLclk = true;
    bool shwMaxGfxclk = true;
    bool shwMinGfxclk = true;
    bool shwProchotDeassertionRamp = true;
    bool shwApuSkinTemp = true;
    bool shwDgpuSkinTemp = true;
    bool shwApuSlowLimit = true;
    bool shwSkinTempLimit = true;
    bool shwPowerSaving = true;
    bool shwMaxPerformance = true;
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
            return new hideShow;
        }
    }

private:
    settingsStr settingsBuffer;
    QList<presetStr*> *presets;

    QFile *configQFile;
    QFile *presetsQFile;

    hideShow shwpvRaven = {
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        true,
        true
    };
    hideShow shwpvPicasso = {
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        true,
        true
    };
    hideShow shwpvRenoir = {
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    };
    hideShow shwpvCezanne = {
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    };
    hideShow shwpvDali = {
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        true,
        true
    };
    hideShow shwpvLucienne = {
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    };

};

#endif // CTRLSETTINGS_H
