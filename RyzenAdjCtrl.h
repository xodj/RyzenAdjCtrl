#ifndef RYZENADJCTRL_H
#define RYZENADJCTRL_H

#include <QObject>
#include <QSharedMemory>
#include <QProcess>
#include <QTimer>
#include "RyzenAdjCfg.h"

#include <QDebug>

class RyzenAdjCtrl : public QObject {
    Q_OBJECT
public:
    RyzenAdjCtrl(QSharedMemory *bufferToService, RyzenAdjCfg *conf);
    ~RyzenAdjCtrl();

private:
    void recieveArgs();
    void decodeArgs(QByteArray args);

    void checkCurrentACState();
    void currentACStateChanged();
    void loadPreset(int currentPresetId);

    void RyzenAdjSendCommand(QString arguments);
    void atrofacSendCommand(QString arguments);

    QSharedMemory *bufferToService;
    RyzenAdjCfg *conf;

    ACState currentACState;

    QTimer *currentAc_refresh_timer;
    QTimer *bufferToService_refresh_timer;
    QTimer *autoPresetApplyTimer;
};

#endif // RYZENADJCTRL_H
