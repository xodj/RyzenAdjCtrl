#ifndef CTRLACCALLBACK_H
#define CTRLACCALLBACK_H

#include <QObject>

enum ACState {
    Battery = 0,
    ACPower,
    ACNone
};

#ifdef WIN32
#include <QTimer>
#include <Windows.h>

#define currentAc_refresh_time 300

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
        SYSTEM_POWER_STATUS sps;
        if (GetSystemPowerStatus(&sps))
            if(currentACState != ACState(sps.ACLineStatus)) {
                currentACState = ACState(sps.ACLineStatus);
                emit currentACStateChanged(currentACState);
            }
    }
    QTimer *currentAc_refresh_timer;

public slots:
    void emitCurrentACState(){
        if(currentACState != ACNone)
            emit currentACStateChanged(currentACState);
    }
};
#else
class CtrlACCallback : public QObject
{
    Q_OBJECT
public:
    CtrlACCallback() {}
    ~CtrlACCallback() {}

signals:
    void currentACStateChanged(ACState state);

public slots:
    void emitCurrentACState(){}
};
#endif
#endif // CTRLACCALLBACK_H
