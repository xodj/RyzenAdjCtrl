#ifndef CTRLEPMCALLBACK_H
#define CTRLEPMCALLBACK_H

#include <QObject>
#include <QTimer>

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
#endif // CTRLEPMCALLBACK_H
