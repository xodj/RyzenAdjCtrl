#ifndef CTRLSERVICE_H
#define CTRLSERVICE_H

#include <QObject>
#include <QSharedMemory>
#include <QProcess>
#include <QTimer>
#include "CtrlSettings.h"

#include <QDebug>

class CtrlService : public QObject {
    Q_OBJECT
public:
    CtrlService(QSharedMemory *bufferToService, CtrlSettings *conf);
    ~CtrlService();

private:
    void recieveArgs();
    void decodeArgs(QByteArray args);

    void checkCurrentACState();
    void currentACStateChanged();
    void loadPreset(int currentPresetId);

    void RyzenAdjSendCommand(QString arguments);
    void atrofacSendCommand(QString arguments);

    QSharedMemory *bufferToService;
    CtrlSettings *conf;

    ACState currentACState;

    QTimer *currentAc_refresh_timer;
    QTimer *bufferToService_refresh_timer;
    QTimer *autoPresetApplyTimer;
};

#endif // CTRLSERVICE_H
