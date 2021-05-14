#ifndef CTRLSETTINGS_H
#define CTRLSETTINGS_H

#include <QObject>
#include <QDebug>

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

    settingsStr *getSettingsBuffer(){
        qDebug()<<"RyzenAdjCtrl Settings Get Settings";
        return &settingsBuffer;
    }

    const QList<presetStr*> *getPresetsList(){
        //qDebug()<<"RyzenAdjCtrl Settings Get Presets List";
        return presets;
    }

    qsizetype getPresetsCount(){
        //qDebug()<<"RyzenAdjCtrl Settings Get Presets Count";
        return presets->count();
    }

    bool setPresetBuffer(int idx, presetStr *preset){
        qDebug()<<"RyzenAdjCtrl Settings Set Preset ID" << idx << preset->presetName;
        presetStr *presetBuffer = getPresetBuffer(idx);

        if(presetBuffer != nullptr)
            presets->removeOne(presetBuffer);

        insertNewPreset(idx, preset);
        return true;
    }

    presetStr *getPresetBuffer(int idx){
        qDebug()<<"RyzenAdjCtrl Settings Get Preset ID" << idx;
        presetStr *presetBuffer = nullptr;
        for(qsizetype i = 0;i < presets->count();i++)
            if(presets->at(i)->presetId == idx)
                presetBuffer = presets->at(i);
        return presetBuffer;
    }

    int insertNewPreset(int newidx = -1, presetStr *newPreset = nullptr){
        qDebug()<<"RyzenAdjCtrl Settings Insert New Preset ID" << newidx;
        if(newPreset == nullptr) {
            newPreset = new presetStr;
            newPreset->presetName = "New preset";
        }
        if(newidx == -1) {
            newidx = presets->count();
            for(;;){
                newidx++;
                if(getPresetBuffer(newidx) == nullptr)
                    break;
            }
        }
        newPreset->presetId = newidx;
        presets->emplaceBack(newPreset);
        return newidx;
    }

    bool deletePreset(int idx){
        qDebug()<<"RyzenAdjCtrl Settings Delete Preset ID" << idx;
        presetStr *preset = getPresetBuffer(idx);
        presets->removeOne(preset);
        delete preset;
        return true;
    }

private:
    settingsStr settingsBuffer;
    QList<presetStr*> *presets;

};

#endif // CTRLSETTINGS_H
