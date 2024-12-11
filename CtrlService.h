#ifndef CTRLSERVICE_H
#define CTRLSERVICE_H

#include <QObject>
#include <QTimer>
#include "CtrlSettings.h"
#ifdef WIN32
#include "CtrlEPMCallback.h"
#endif
#include "CtrlACCallback.h"
#include "CtrlArmour.h"
#include "ryzenadj.h"
#include "CtrlBus.h"

#include <QDebug>

class CtrlService : public QObject {
    Q_OBJECT
public:
    CtrlService(CtrlBus *bus);
    ~CtrlService();

private:
    void initPmTable();

    void recieveMessageToService(messageToServiceStr messageToService);

    void loadPreset(presetStr *preset);

    void sendCurrentPresetIdToGui(int presetId, bool saved);

    void currentInfoTimeoutChanged(int timeout);
    void takeCurrentInfo();
    void sendCurrentInfoToGui();

    ryzen_access adjEntryPoint;
    PMTable pmTable;

    QTimer *takeCurrentInfoTimer;

    ACState currentACState;
#ifdef WIN32
    epmMode currentEPMode;
    CtrlEPMCallback *epmCallback;
#endif
    CtrlArmour *armour;

    presetStr *lastPreset = nullptr;
    bool lastPresetSaved = true;

    CtrlBus *bus;
    CtrlACCallback *acCallback;
    QTimer *reapplyPresetTimer;

    CtrlSettings *conf;

public slots:
    void currentACStateChanged(ACState state);
#ifdef WIN32
    void epmIdChanged(epmMode EPMode);
#endif
    void reapplyPresetTimeout();

};

#endif // CTRLSERVICE_H
