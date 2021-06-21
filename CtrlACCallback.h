#ifndef CTRLACCALLBACK_H
#define CTRLACCALLBACK_H

#include <QObject>
#include <QTimer>
#include <QDebug>

enum ACState {
    Battery = 0,
    ACPower,
    ACNone
};

#ifdef WIN32
#include <Windows.h>
#define currentAc_refresh_time 333
#else // WIN32
#include <QFile>
#define currentAc_refresh_time 1332
#endif // WIN32

class CtrlACCallback : public QObject
{
    Q_OBJECT
public:
    CtrlACCallback() {
        currentAc_refresh_timer = new QTimer;
        connect(currentAc_refresh_timer, &QTimer::timeout,
                this, &CtrlACCallback::checkCurrentACState);
        currentAc_refresh_timer->start(currentAc_refresh_time);
    }
    ~CtrlACCallback() {
        disconnect(currentAc_refresh_timer, &QTimer::timeout,
                   this, &CtrlACCallback::checkCurrentACState);
        currentAc_refresh_timer->stop();
    }

signals:
    void currentACStateChanged(ACState state);

private:
    ACState currentACState = ACNone;
    void checkCurrentACState() {
#ifdef WIN32
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps))
            if(currentACState != ACState(sps.ACLineStatus)) {
                currentACState = ACState(sps.ACLineStatus);
                emit currentACStateChanged(currentACState);
            }
#else // WIN32
        QFile acOnline("/sys/class/power_supply/AC0/online");
        if(acOnline.open(QIODevice::ReadOnly) && acOnline.size() > 0) {
            int state = acOnline.readAll().toInt();
            if(currentACState != ACState(state)) {
                currentACState = ACState(state);
                emit currentACStateChanged(currentACState);
            }
        }
#endif // WIN32
    }
    QTimer *currentAc_refresh_timer;

public slots:
    void emitCurrentACState(){
        if(currentACState != ACNone)
            emit currentACStateChanged(currentACState);
    }
};
#endif // CTRLACCALLBACK_H
