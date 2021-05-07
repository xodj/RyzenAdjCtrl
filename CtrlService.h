#ifndef CTRLSERVICE_H
#define CTRLSERVICE_H

#include <QObject>
#include <QSharedMemory>
#include <QProcess>
#include <QTimer>
#include "CtrlSettings.h"
#include "CtrlEPMCallback.h"

#include <QDebug>

class CtrlService : public QObject {
    Q_OBJECT
public:
    CtrlService(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf);
    ~CtrlService();

private:
    void recieveArgs();
    void decodeArgs(QByteArray args);

    void loadPreset(int currentPresetId);

    void RyzenAdjSendCommand(QString arguments);
    void atrofacSendCommand(QString arguments);

    void sendCurrentPresetIdToGui(int presetId, bool saved);
    void sendArgsToGui(QByteArray arguments);

    void currentInfoTimeoutChanged(int timeout);
    void takeCurrentInfo();
    void sendCurrentInfoToGui(QString info);
    QTimer *takeCurrentInfoTimer;

    ACState currentACState;
    epmMode currentEPMode;
    int lastUsedPresetId = -1;

    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;

    CtrlSettings *conf;
    CtrlEPMCallback *epmCallback;
    CtrlACCallback *acCallback;

    QTimer *bufferToService_refresh_timer;
    QTimer *reapplyPresetTimer;

public slots:
    void currentACStateChanged(ACState state);
    void epmIdChanged(epmMode EPMode);
    void reapplyPresetTimeout();

};

#endif // CTRLSERVICE_H
