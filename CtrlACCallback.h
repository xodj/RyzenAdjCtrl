#ifndef CTRLACCALLBACK_H
#define CTRLACCALLBACK_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include "CtrlConfig.h"

#ifdef WIN32
#include <Windows.h>
#else // WIN32
#include <QFile>
#endif // WIN32

class CtrlACCallback : public QObject
{
    Q_OBJECT
public:
    CtrlACCallback() {
        AC_STATE_REFRESH_TIMEr = new QTimer;
        connect(AC_STATE_REFRESH_TIMEr, &QTimer::timeout,
                this, &CtrlACCallback::checkCurrentACState);
        AC_STATE_REFRESH_TIMEr->start(AC_STATE_REFRESH_TIME);
    }
    ~CtrlACCallback() {
        disconnect(AC_STATE_REFRESH_TIMEr, &QTimer::timeout,
                   this, &CtrlACCallback::checkCurrentACState);
        AC_STATE_REFRESH_TIMEr->stop();
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
    QTimer *AC_STATE_REFRESH_TIMEr;

public slots:
    void emitCurrentACState(){
        if(currentACState != ACNone)
            emit currentACStateChanged(currentACState);
    }
};
#endif // CTRLACCALLBACK_H
