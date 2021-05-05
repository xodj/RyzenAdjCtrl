#ifndef CTRLEPMCALLBACK_H
#define CTRLEPMCALLBACK_H

#include <QObject>

enum epmMode{
    BatterySaver = 0,
    BetterBattery,
    Balanced,
    MaxPerformance,
    GameMode,
    MixedReality
};

class CtrlEPMCallback : public QObject
{
    Q_OBJECT
public:
    CtrlEPMCallback();
    ~CtrlEPMCallback();

signals:
    void epmIdChanged(epmMode currentEPM);

private:
    void **epmHandle;
};
#endif // CTRLEPMCALLBACK_H
