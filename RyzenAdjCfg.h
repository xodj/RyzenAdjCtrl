#ifndef RYZENADJCFG_H
#define RYZENADJCFG_H

#include <QObject>

enum enumFanPresetId {
    None = 0,
    Windows,
    Silent,
    Perfomance,
    Turbo,
    Manual
};

enum ACState {
    Battery = 0,
    ACPower
};

struct presetStr {
    int presetId = 0;
    QString presetName = "Preset Name";
    QString cmdOutputValue = "";

    int fanPresetId = Perfomance;

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
    bool showReloadStyleSheetButton = false;

    bool autoPresetApplyDurationChecked = true;
    int autoPresetApplyDuration = 180;

    bool autoPresetSwitchAC = true;
    int dcStatePresetId = 1;
    int acStatePresetId = 2;
};

class RyzenAdjCfg : public QObject
{
    Q_OBJECT

public:
    RyzenAdjCfg();
    ~RyzenAdjCfg();
    presetStr* getPresets(){ return presetsBuffer; }
    settingsStr* getSettings(){ return &settingsBuffer; }
    bool saveSettings();
    bool openSettings();
    bool savePresets();
    bool openPresets();

private:
    presetStr *presetsBuffer;
    settingsStr settingsBuffer;
};

#endif // RYZENADJCFG_H
