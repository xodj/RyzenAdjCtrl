#ifndef CTRLEPMCALLBACK_H
#define CTRLEPMCALLBACK_H

#include <QObject>

enum epmMode{
    BatterySaver = 0,
    BetterBattery,
    Balanced,
    MaxPerformance,
    GameMode,
    MixedReality,
    epmNone
};

class CtrlEPMCallback : public QObject
{
    Q_OBJECT
public:
    CtrlEPMCallback();
    ~CtrlEPMCallback();

signals:
    void epmIdChanged(epmMode currentEPM);

public slots:
    void emitEpmIdChanged(epmMode currentEPM);
    void emitCurrentEPMState();

private:
    void **epmHandle;
    epmMode currentEpm = epmNone;
};

enum ACState {
    Battery = 0,
    ACPower,
    ACNone
};

class CtrlACCallback : public QObject
{
    Q_OBJECT
public:
    CtrlACCallback();
    ~CtrlACCallback();

signals:
    void currentACStateChanged(ACState state);

private:
    ACState currentACState = ACNone;
    void checkCurrentACState();
    QTimer *currentAc_refresh_timer;

public slots:
    void emitCurrentACState();
};
#endif // CTRLEPMCALLBACK_H
