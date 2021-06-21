#ifndef CTRLSERVICE_H
#define CTRLSERVICE_H

#include <QObject>
#include <QTimer>
#include "CtrlSettings.h"
#ifdef WIN32
#include "CtrlEPMCallback.h"
#include "CtrlArmour.h"
#endif
#include "CtrlACCallback.h"
#include "lib/ryzenadj.h"
#include "CtrlBus.h"

#include <QDebug>

struct PMTable{
    QString ryzenFamily;
    QString biosVersion;
    QString pmTableVersion;
    QString ryzenAdjVersion;

    QString stapm_limit;
    QString stapm_value;
    QString fast_limit;
    QString fast_value;
    QString slow_limit;
    QString slow_value;
    QString apu_slow_limit;
    QString apu_slow_value;
    QString vrm_current;
    QString vrm_current_value;
    QString vrmsoc_current;
    QString vrmsoc_current_value;
    QString vrmmax_current;
    QString vrmmax_current_value;
    QString vrmsocmax_current;
    QString vrmsocmax_current_value;
    QString tctl_temp;
    QString tctl_temp_value;
    QString apu_skin_temp_limit;
    QString apu_skin_temp_value;
    QString dgpu_skin_temp_limit;
    QString dgpu_skin_temp_value;
    QString stapm_time;
    QString slow_time;
    //NEW VARS
    QString psi0_current;
    QString psi0soc_current;
};

class CtrlService : public QObject {
    Q_OBJECT
public:
    CtrlService(CtrlBus *bus, CtrlSettings *conf);
    ~CtrlService();

private:
    void initPmTable();

    void decodeArgs(QByteArray args);

    void loadPreset(presetStr *preset);

    void sendCurrentPresetIdToGui(int presetId, bool saved);

    void currentInfoTimeoutChanged(int timeout);
    void takeCurrentInfo();
    void sendCurrentInfoToGui();

    ryzen_access adjEntryPoint;
    PMTable pmTable;

    QTimer *takeCurrentInfoTimer;

    ACState currentACState;
#ifdef WIN32
    epmMode currentEPMode;
    CtrlEPMCallback *epmCallback;
    CtrlArmour *armour;
#endif

    presetStr *lastPreset = nullptr;
    bool lastPresetSaved = true;

    CtrlBus *bus;
    CtrlACCallback *acCallback;
    QTimer *reapplyPresetTimer;

    CtrlSettings *conf;

public slots:
    void currentACStateChanged(ACState state);
#ifdef WIN32
    void epmIdChanged(epmMode EPMode);
#endif
    void reapplyPresetTimeout();

};

#endif // CTRLSERVICE_H
