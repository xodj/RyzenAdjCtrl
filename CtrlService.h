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
    CtrlService(QSharedMemory *bufferToService, CtrlSettings *conf);
    ~CtrlService();

private:
    void recieveArgs();
    void decodeArgs(QByteArray args);

    void loadPreset(int currentPresetId);

    void RyzenAdjSendCommand(QString arguments);
    void atrofacSendCommand(QString arguments);

    ACState currentACState;
    epmMode currentEPMode;

    QSharedMemory *bufferToService;

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
