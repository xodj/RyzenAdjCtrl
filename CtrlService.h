#ifndef CTRLSERVICE_H
#define CTRLSERVICE_H

#include <QObject>
#include <QSharedMemory>
#include <QProcess>
#include <QTimer>
#include "CtrlSettings.h"
#include "CtrlEPMCallback.h"
#include "lib/ryzenadj.h"
#include "CtrlArmour.h"

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
};

class CtrlService : public QObject {
    Q_OBJECT
public:
    CtrlService(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf);
    ~CtrlService();

private:
    void initPmTable();

    void recieveArgs();
    void decodeArgs(QByteArray args);

    void loadPreset(presetStr *preset);

    void sendCurrentPresetIdToGui(int presetId, bool saved);
    void sendArgsToGui(QByteArray arguments);

    void currentInfoTimeoutChanged(int timeout);
    void takeCurrentInfo();
    void sendCurrentInfoToGui();

    ryzen_access adjEntryPoint;
    PMTable pmTable;
    CtrlArmour *armour;

    QTimer *takeCurrentInfoTimer;

    ACState currentACState;
    epmMode currentEPMode;

    presetStr *lastPreset = nullptr;
    bool lastPresetSaved = true;

    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;

    CtrlEPMCallback *epmCallback;
    CtrlACCallback *acCallback;

    QTimer *bufferToService_refresh_timer;
    QTimer *reapplyPresetTimer;

    CtrlSettings *conf;

public slots:
    void currentACStateChanged(ACState state);
    void epmIdChanged(epmMode EPMode);
    void reapplyPresetTimeout();

};

#endif // CTRLSERVICE_H
