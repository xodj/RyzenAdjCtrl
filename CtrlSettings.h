#ifndef CTRLSETTINGS_H
#define CTRLSETTINGS_H

#include <QObject>
#include <QFile>
#include "CtrlConfig.h"

class CtrlSettings : public QObject
{
    Q_OBJECT

public:
    CtrlSettings();
    ~CtrlSettings();

    void checkSettings();
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

    hideShow hideShowWarnPresetVariable(int idx){
        switch(idx){
        case 0:
            return hideShow{SHOW_ONLY_RAVEN};
        case 1:
            return hideShow{SHOW_ONLY_PICASSO};
        case 2:
            return hideShow{SHOW_ONLY_RENOIR};
        case 3:
            return hideShow{SHOW_ONLY_CEZANNE};
        case 4:
            return hideShow{SHOW_ONLY_DALI};
        case 5:
            return hideShow{SHOW_ONLY_LUCIENNE};
        case 6:
            return hideShow();
        case 7:
            return hideShow();
        default:
            return hideShow();
        }
    }

private:
    settingsStr settingsBuffer;
    QList<presetStr*> *presets;

    QFile *configQFile;
    QFile *presetsQFile;
};

#endif // CTRLSETTINGS_H
