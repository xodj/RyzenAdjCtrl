#ifndef CTRLBUS_H
#define CTRLBUS_H

#include <QObject>
#include <QDebug>
#include "CtrlConfig.h"
#include "CtrlSettings.h"
#include <QSharedMemory>
#include <QTimer>

#define buffer_size 512
#define refresh_time 10

struct messageToServiceStr{
    bool exit = false;
    bool getSettings = false;
    bool saveSettings = false;
    settingsStr settings;
    bool savePreset = false;
    bool applyPreset = false;
    bool deletePreset = false;
    presetStr preset;
    bool ryzenAdjInfo = false;
    int ryzenAdjInfoTimeout = 0;
};

struct PMTable{
    char ryzenFamily[16] = {'U','n','k','n','o','w','n','\0'};
    int biosVersion = 0;
    uint32_t pmTableVersion = 0;
    int ryzenAdjVersion = 0;
    int ryzenAdjMajorVersion = 0;
    int ryzenAdjMinorVersion = 0;

    float stapm_limit = 0.f;
    float stapm_value = 0.f;
    float fast_limit = 0.f;
    float fast_value = 0.f;
    float slow_limit = 0.f;
    float slow_value = 0.f;
    float apu_slow_limit = 0.f;
    float apu_slow_value = 0.f;
    float vrm_current = 0.f;
    float vrm_current_value = 0.f;
    float vrmsoc_current = 0.f;
    float vrmsoc_current_value = 0.f;
    float vrmmax_current = 0.f;
    float vrmmax_current_value = 0.f;
    float vrmsocmax_current = 0.f;
    float vrmsocmax_current_value = 0.f;
    float tctl_temp = 0.f;
    float tctl_temp_value = 0.f;
    float apu_skin_temp_limit = 0.f;
    float apu_skin_temp_value = 0.f;
    float dgpu_skin_temp_limit = 0.f;
    float dgpu_skin_temp_value = 0.f;
    float stapm_time = 0.f;
    float slow_time = 0.f;
    float psi0_current = 0.f;
    float psi0soc_current = 0.f;
};

struct messageToGuiStr{
    int currentPresetId = -1;
    bool presetSaved = false;
    bool pmUpdated = false;
    PMTable pmTable;
};

struct settingsToGuiStr{
    settingsStr settings;
    presetStr preset;
    bool lastPreset = false;
};

enum busType{
    busTypeService = 0,
    busTypeGui
};

#ifdef WIN32
#ifdef BUILD_SERVICE
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QSharedMemory *bufferToService,
            QSharedMemory *bufferToServiceFlag,
            QSharedMemory *bufferToGui,
            QSharedMemory *bufferToGuiFlag,
            QSharedMemory *guiAlreadyRunning,
            QSharedMemory *bufferSettingsToGui,
            QSharedMemory *bufferSettingsToGuiFlag)
        : QObject(nullptr),
          bufferToService(bufferToService),
          bufferToServiceFlag(bufferToServiceFlag),
          bufferToGui(bufferToGui),
          bufferToGuiFlag(bufferToGuiFlag),
          guiAlreadyRunning(guiAlreadyRunning),
          bufferSettingsToGui(bufferSettingsToGui),
          bufferSettingsToGuiFlag(bufferSettingsToGuiFlag),
          conf(new CtrlSettings)
    {}
    ~CtrlBus(){
        refresh_timer->stop();
    }

    CtrlSettings *getSettingsFromFile(){
        conf->checkSettings();
        return conf;
    }

    CtrlSettings *getSettingsFromService(){
        messageToServiceStr messageToService;
        messageToService.getSettings = true;
        sendMessageToService(messageToService);
        settingsToGuiStr settingsToGui;
        do{
            if(bufferSettingsToGuiFlag->attach(QSharedMemory::ReadWrite)){
                if (bufferSettingsToGuiFlag->lock()) {
                    bool flag = false;
                    memcpy(&flag, bufferSettingsToGuiFlag->data(), sizeof(bool));
                    if(flag){
                        /**/
                        if(bufferSettingsToGui->attach(QSharedMemory::ReadWrite)){
                            if (bufferSettingsToGui->lock()) {
                                memcpy(&settingsToGui, bufferSettingsToGui->data(), sizeof(settingsToGuiStr));
                                memcpy(conf->getSettingsBuffer(), &settingsToGui.settings, sizeof(settingsStr));
                                presetStr *preset = new presetStr;
                                memcpy(preset, &settingsToGui.preset, sizeof(presetStr));
                                conf->insertNewPreset(preset->presetId, preset);
                                for (int i=0;i < sizeof(settingsToGuiStr);i++)
                                    ((char*)bufferSettingsToGui->data())[i] = '\0';
                                bufferSettingsToGui->unlock();
                                flag = false;
                                memcpy(bufferSettingsToGuiFlag->data(), &flag, sizeof(bool));
                                qDebug()<<"Ctrl Bus - Recieved message from Service";
                            } else {
                                qDebug()<<"Ctrl Bus - Can't lock bufferSettingsToGui!";
                            }
                            /**/
                        } else {
                            qDebug()<<"Ctrl Bus - Can't attach bufferSettingsToGui!";
                        }
                        bufferSettingsToGui->detach();
                    }
                } else {
                    qDebug()<<"Ctrl Bus - Can't lock bufferSettingsToGuiFlag!";
                }
                bufferSettingsToGuiFlag->unlock();
            } else {
                qDebug()<<"Ctrl Bus - Can't attach bufferSettingsToGuiFlag!";
            }
            bufferSettingsToGuiFlag->detach();
        }
        while(!settingsToGui.lastPreset);
        return conf;
    }

    void sendMessageToService(messageToServiceStr messageToService){
        if(bufferToServiceFlag->attach(QSharedMemory::ReadWrite)){
            if (bufferToServiceFlag->lock()) {
                bool flag = false;
                memcpy(&flag, bufferToServiceFlag->data(), sizeof(bool));
                if(flag)
                    qDebug()<<"Ctrl Bus - Preveous message to service is not readed!";
                /**/
                if(bufferToService->attach(QSharedMemory::ReadWrite)){
                    if (bufferToService->lock()) {
                        memcpy(bufferToService->data(), &messageToService,
                               sizeof(messageToServiceStr));
                    } else {
                        qDebug()<<"Ctrl Bus - Can't lock bufferToService!";
                    }
                    bufferToService->unlock();
                } else {
                    qDebug()<<"Ctrl Bus - Can't attach bufferToService!";
                }
                bufferToService->detach();
                /**/
                flag = true;
                memcpy(bufferToServiceFlag->data(), &flag, sizeof(bool));
            } else {
                qDebug()<<"Ctrl Bus - Can't lock bufferToServiceFlag!";
            }
            bufferToServiceFlag->unlock();
        } else {
            qDebug()<<"Ctrl Bus - Can't attach bufferToServiceFlag!";
        }
        bufferToServiceFlag->detach();
    }

    bool isServiseRuning(){
        bool bReturn = false;
        if(bufferToService->attach()){
            bufferToService->detach();
            bReturn = true;
        }
        return bReturn;
    }
    void setServiseRuning(){
        if(!bufferToService->create(sizeof(messageToServiceStr)))
            qDebug()<<"Ctrl Bus - Can't create bufferToService!";
        if(!bufferToServiceFlag->create(sizeof(bool)))
            qDebug()<<"Ctrl Bus - Can't create bufferToServiceFlag!";
        if(!bufferToGui->create(buffer_size))
            qDebug()<<"Ctrl Bus - Can't create bufferToGui!";
        if(!bufferToGuiFlag->create(sizeof(bool)))
            qDebug()<<"Ctrl Bus - Can't create bufferToGuiFlag!";
        if(!bufferSettingsToGui->create(buffer_size))
            qDebug()<<"Ctrl Bus - Can't create bufferToGui!";
        if(!bufferSettingsToGuiFlag->create(sizeof(bool)))
            qDebug()<<"Ctrl Bus - Can't create bufferToGuiFlag!";

        refresh_timer = new QTimer;
        connect(refresh_timer, &QTimer::timeout,
                this, &CtrlBus::getMessageFromGui);
        refresh_timer->start(refresh_time);
    }

    void sendMessageToGui(messageToGuiStr messageToGui){
        if (bufferToGuiFlag->lock()) {
            bool flag = false;
            memcpy(&flag, bufferToGuiFlag->data(), sizeof(bool));
            if(flag)
                qDebug()<<"Ctrl Bus - Preveous message to GUI is not readed!";
            /**/
            if (bufferToGui->lock()) {
                memcpy(bufferToGui->data(), &messageToGui,
                       sizeof(messageToGuiStr));
            } else {
                qDebug()<<"Ctrl Bus - Can't lock bufferToGui!";
            }
            bufferToGui->unlock();
            /**/
            flag = true;
            memcpy(bufferToGuiFlag->data(), &flag, sizeof(bool));
        } else {
            qDebug()<<"Ctrl Bus - Can't lock bufferToGuiFlag!";
        }
        bufferToGuiFlag->unlock();
    }
    void sendSettingsToGui(){
        qDebug()<<"Ctrl Bus - sendSettingsToGui";
        settingsToGuiStr settingsToGui;
        settingsToGui.settings = *conf->getSettingsBuffer();
        for(size_t i=0;i<conf->getPresetsCount();){
            settingsToGui.preset = *(conf->getPresetsList()->at(i));
            if (bufferSettingsToGuiFlag->lock()) {
                bool flag = false;
                memcpy(&flag, bufferSettingsToGuiFlag->data(), sizeof(bool));
                if(flag){
                    qDebug()<<"Ctrl Bus - Preveous message to GUI is not readed!";
                } else {
                    /**/
                    if (bufferSettingsToGui->lock()) {
                        if(i + 1 == conf->getPresetsCount())
                            settingsToGui.lastPreset = true;
                        memcpy(bufferSettingsToGui->data(), &settingsToGui,
                               sizeof(settingsToGuiStr));
                        i++;
                    } else {
                        qDebug()<<"Ctrl Bus - Can't lock bufferSettingsToGui!";
                    }
                    bufferSettingsToGui->unlock();
                    /**/
                    flag = true;
                    memcpy(bufferSettingsToGuiFlag->data(), &flag, sizeof(bool));
                }
            } else {
                qDebug()<<"Ctrl Bus - Can't lock bufferSettingsToGuiFlag!";
            }
            bufferSettingsToGuiFlag->unlock();
        }
    }
    bool isGUIRuning(){
        if(guiAlreadyRunning->attach()){
            for(;!guiAlreadyRunning->lock();){}
            bool signal = true;
            memcpy(guiAlreadyRunning->data(), &signal, sizeof(bool));
            guiAlreadyRunning->unlock();
            guiAlreadyRunning->detach();
            return true;
        } else
            return false;
    }
    void setGUIRuning(){
        if(!guiAlreadyRunning->create(sizeof(bool)))
            qDebug()<<"Ctrl Bus - Can't create guiAlreadyRunning!";

        refresh_timer = new QTimer;
        connect(refresh_timer, &QTimer::timeout,
                this, &CtrlBus::getMessageFromService);
        connect(refresh_timer, &QTimer::timeout,
                this, &CtrlBus::guiAlreadyRunningCheck);
        refresh_timer->start(refresh_time);
    }

signals:
    void messageFromServiceRecieved(messageToGuiStr messageToGui);
    void messageFromGUIRecieved(messageToServiceStr messageToService);
    void messageFromAnotherGui();

private:
    void getMessageFromGui(){
        if (bufferToServiceFlag->lock()) {
            bool flag = false;
            memcpy(&flag, bufferToServiceFlag->data(), sizeof(bool));
            if(flag){
                /**/
                messageToServiceStr messageToService;
                if (bufferToService->lock()) {
                    memcpy(&messageToService, bufferToService->data(),
                           sizeof(messageToServiceStr));
                    if(messageToService.getSettings){
                        sendSettingsToGui();
                    } else {
                        emit messageFromGUIRecieved(messageToService);
                    }
                    for (int i=0;i < sizeof(messageToServiceStr);i++)
                        ((char*)bufferToService->data())[i] = '\0';
                    bufferToService->unlock();
                    flag = false;
                    memcpy(bufferToServiceFlag->data(), &flag, sizeof(bool));
                    qDebug()<<"Ctrl Bus - Recieved message from GUI";
                } else {
                    qDebug()<<"Ctrl Bus - Can't lock bufferToService!";
                }
                /**/
            }
            bufferToServiceFlag->unlock();
        } else {
            qDebug()<<"Ctrl Bus - Can't lock bufferToServiceFlag!";
        }
    }

    void getMessageFromService(){
        if(bufferToGuiFlag->attach(QSharedMemory::ReadWrite)){
            if (bufferToGuiFlag->lock()) {
                bool flag = false;
                memcpy(&flag, bufferToGuiFlag->data(), sizeof(bool));
                if(flag){
                    /**/
                    if(bufferToGui->attach(QSharedMemory::ReadWrite)){
                        messageToGuiStr messageToGui;
                        if (bufferToGui->lock()) {
                            memcpy(&messageToGui, bufferToGui->data(), sizeof(messageToGuiStr));
                            emit messageFromServiceRecieved(messageToGui);
                            for (int i=0;i < sizeof(messageToGuiStr);i++)
                                ((char*)bufferToGui->data())[i] = '\0';
                            bufferToGui->unlock();
                            flag = false;
                            memcpy(bufferToGuiFlag->data(), &flag, sizeof(bool));
                            qDebug()<<"Ctrl Bus - Recieved message from Service";
                        } else {
                            qDebug()<<"Ctrl Bus - Can't lock bufferToGui!";
                        }
                        /**/
                    } else {
                        qDebug()<<"Ctrl Bus - Can't attach bufferToGui!";
                    }
                    bufferToGui->detach();
                }
            } else {
                qDebug()<<"Ctrl Bus - Can't lock bufferToGuiFlag!";
            }
            bufferToGuiFlag->unlock();
        } else {
            qDebug()<<"Ctrl Bus - Can't attach bufferToGuiFlag: "<<errorCount + 1;
            errorCount++;
            if(errorCount == 100){
                qDebug()<<"Ctrl Bus - To much errors, exiting...";
                exit(1);
            }
        }
        bufferToGuiFlag->detach();
    }

    void guiAlreadyRunningCheck(){
        if (guiAlreadyRunning->lock()) {
            bool signal = false;
            memcpy(&signal, guiAlreadyRunning->data(), sizeof(bool));
            if(signal) {
                qDebug()<<"Ctrl Bus - Recieved message from another unit of GUI";
                signal = false;
                memcpy(guiAlreadyRunning->data(), &signal, sizeof(bool));
                emit messageFromAnotherGui();
            }
            guiAlreadyRunning->unlock();
        } else
            qDebug()<<"Ctrl Bus - Can't lock guiAlreadyRunning!";
    }

    QSharedMemory *bufferToService;
    QSharedMemory *bufferToServiceFlag;
    QSharedMemory *bufferToGui;
    QSharedMemory *bufferToGuiFlag;
    QSharedMemory *guiAlreadyRunning;
    QSharedMemory *bufferSettingsToGui;
    QSharedMemory *bufferSettingsToGuiFlag;

    QTimer *refresh_timer;
    CtrlSettings *conf;
    int errorCount = 0;
};
#else //NOT BUILD_SERVICE
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QSharedMemory *guiAlreadyRunning)
        : QObject(nullptr),
          guiAlreadyRunning(guiAlreadyRunning),
          conf(new CtrlSettings)
    {
        conf->checkSettings();
    }
    ~CtrlBus(){}

    CtrlSettings *getSettingsFromFile(){
        return conf;
    }
    CtrlSettings *getSettingsFromService(){
        return conf;
    }

    void sendMessageToService(messageToServiceStr data){
        emit messageFromGUIRecieved(data);
    }
    void sendMessageToGui(messageToGuiStr data){
        messageFromServiceRecieved(data);
    }
    bool isGUIRuning(){
        if(guiAlreadyRunning->attach()){
            for(;!guiAlreadyRunning->lock();){}
            bool signal = true;
            memcpy(guiAlreadyRunning->data(), &signal, sizeof(bool));
            guiAlreadyRunning->unlock();
            guiAlreadyRunning->detach();
            return true;
        } else
            return false;
    }
    void setGUIRuning(){
        guiAlreadyRunning->create(sizeof(bool));
        refresh_timer = new QTimer;
        connect(refresh_timer, &QTimer::timeout,
                this, &CtrlBus::guiAlreadyRunningCheck);
        refresh_timer->start(refresh_time);
    }

signals:
    void messageFromServiceRecieved(messageToGuiStr messageToGui);
    void messageFromGUIRecieved(messageToServiceStr messageToService);
    void messageFromAnotherGui();

private:
    void guiAlreadyRunningCheck(){
        if (guiAlreadyRunning->lock()) {
            bool signal = false;
            memcpy(&signal, guiAlreadyRunning->data(), sizeof(bool));
            if(signal) {
                qDebug()<<"Ctrl Bus - Recieved message from another unit of GUI";
                signal = false;
                memcpy(guiAlreadyRunning->data(), &signal, sizeof(bool));
                emit messageFromAnotherGui();
            }
            guiAlreadyRunning->unlock();
        } else
            qDebug()<<"Ctrl Bus - Can't lock guiAlreadyRunning!";
    }

    QSharedMemory *guiAlreadyRunning;

    QTimer *refresh_timer;
    CtrlSettings *conf;
};
#endif //BUILD_SERVICE
#else //NOT WIN32
#ifdef BUILD_SERVICE
#include <QtDBus/QtDBus>
#include <QObject>

#define INTERFACE_NAME "ru.ryzenctrl.service.ctrl"
#define SERVICE_NAME "ru.ryzenctrl.service"
#define GUI_NAME "ru.ryzenctrl.gui"
#define OBJECT_NAME "/control"
#define DBUS_CONFIG_FILE_NAME "ru.ryzenctrl.service.conf"
#define DBUS_CONFIG_FILE_PATH "/etc/dbus-1/system.d/ru.ryzenctrl.service.conf"

class CtrlBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", INTERFACE_NAME)
public:
    explicit CtrlBus(QObject *parent = nullptr) :
        QObject(parent),
        conf(new CtrlSettings)
    { qDebug() << __PRETTY_FUNCTION__; }
    ~CtrlBus(){
        qDebug() << __PRETTY_FUNCTION__;
        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.unregisterService(SERVICE_NAME);
        dbus.unregisterObject(OBJECT_NAME);
        dbus = QDBusConnection::systemBus();
        dbus.unregisterService(SERVICE_NAME);
        dbus.unregisterObject(OBJECT_NAME);
    }
    CtrlSettings *getSettingsFromFile(){
        qDebug() << __PRETTY_FUNCTION__;
        conf->checkSettings();
        return conf;
    }
    CtrlSettings *getSettingsFromService(){
        qDebug() << __PRETTY_FUNCTION__;
        QDBusConnection dbus = QDBusConnection::systemBus();
        if (!dbus.isConnected())
            qCritical() << "Cannot connect to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "connect to the D-Bus system bus succesful";
        QDBusInterface iface(SERVICE_NAME, OBJECT_NAME, INTERFACE_NAME, dbus);
        if (iface.isValid()) {
            QDBusReply<QByteArray> configReply = iface.call("getConfigFile");
            if (configReply.isValid()) {
                qDebug() << "Reply from getConfigFile was: "
                         << configReply.value();
                memcpy(conf->getSettingsBuffer(),
                       configReply.value().constData(),
                       sizeof(settingsStr));
            } else
                qCritical() << "Call to getConfigFile failed:" <<
                               qPrintable(configReply.error().message());

            int presetsCount = 0;
            QDBusReply<int> presetsCountReply = iface.call("getPresetsCount");
            if (presetsCountReply.isValid()) {
                qDebug() << "Reply from getPresetsCount was: "
                         << presetsCountReply.value();
                presetsCount = presetsCountReply.value();
            } else
                qCritical() << "Call to getPresetsCount failed:" <<
                               qPrintable(presetsCountReply.error().message());

            if(presetsCount != 0)
                for(int n = 0;n < presetsCount;n++){
                    QDBusReply<QByteArray> presetReply = iface.call("getPresetFile", n);
                    if (presetReply.isValid()) {
                        qDebug() << "Reply from getPresetFile was: "
                                 << presetReply.value();
                        presetStr *preset = new presetStr;
                        memcpy(preset,
                               presetReply.value().constData(),
                               sizeof(presetStr));
                        conf->insertNewPreset(preset->presetId, preset);
                    } else
                        qCritical() << "Call to getPresetFile failed:" <<
                                       qPrintable(presetReply.error().message());
                }
        } else
            qCritical() << "QDBusInterface is invalid.\n";
        return conf;
    }
    void sendMessageToService(messageToServiceStr data){
        qDebug() << __PRETTY_FUNCTION__;
        QDBusConnection dbus = QDBusConnection::systemBus();
        if (!dbus.isConnected())
            qCritical() << "Cannot connect to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "connect to the D-Bus system bus succesful";
        QDBusInterface iface(SERVICE_NAME, OBJECT_NAME, INTERFACE_NAME, dbus);
        if (iface.isValid()) {
            QByteArray var(sizeof(messageToServiceStr),
                           Qt::Initialization::Uninitialized);
            memcpy(var.data(), &data,
                   sizeof(messageToServiceStr));
            iface.call("sendMessageToService", var);
        } else
            qCritical() << "QDBusInterface is invalid.\n";
    }
    void sendMessageToGui(messageToGuiStr data){
        qDebug() << __PRETTY_FUNCTION__;
        QByteArray var(sizeof(messageToGuiStr),
                       Qt::Initialization::Uninitialized);
        memcpy(var.data(), &data,
               sizeof(messageToGuiStr));
        emit messageFromServiceSignal();
    }
    bool isGUIRuning(){
        qDebug() << __PRETTY_FUNCTION__;
        bool answer = false;
        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (!dbus.isConnected())
            qCritical() << "Cannot connect to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "connect to the D-Bus system bus succesful";
        QDBusInterface iface(GUI_NAME, OBJECT_NAME, INTERFACE_NAME, dbus);
        if (iface.isValid()) {
            QDBusReply<bool> reply = iface.call("checkGuiRunning");
            if (reply.isValid()) {
                qDebug() << "Reply from checkGuiRunning was: "
                         << reply.value();
                answer = reply.value();
            } else
                qCritical() << "Call to checkGuiRunning failed:"
                            << qPrintable(reply.error().message());
        } else
            qCritical() << "QDBusInterface is invalid.\n";
        return answer;
    }
    void setGUIRuning(){
        qDebug() << __PRETTY_FUNCTION__;
        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (!dbus.isConnected())
            qCritical() << "Cannot connect to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "connect to the D-Bus system bus succesful";

        if (!dbus.registerService(GUI_NAME))
            qCritical() << "Cannot registerService to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "registerService succesful";

        if (!dbus.registerObject(OBJECT_NAME, this,
                                 QDBusConnection::ExportAllSlots))
            qCritical() << "Cannot registerObject to the D-Bus system bus."
                        << dbus.lastError().message();
        else
            qDebug() << "registerObject succesful";

        bool cnt = dbus.connect(SERVICE_NAME, OBJECT_NAME,
                     INTERFACE_NAME, "messageFromServiceSignal",
                     this, SLOT(messageFromServiceSlot()));
        if (!cnt)
            qCritical() << "Cannot connect to the D-Bus system bus."
                        << dbus.lastError().message();
        else
            qDebug() << "connect succesful";
    }
    bool isServiseRuning(){
        qDebug() << __PRETTY_FUNCTION__;
        bool answer = false;
        QDBusConnection dbus = QDBusConnection::systemBus();
        if (!dbus.isConnected())
            qCritical() << "Cannot connect to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "connect to the D-Bus system bus succesful";
        QDBusInterface iface(SERVICE_NAME, OBJECT_NAME, INTERFACE_NAME, dbus);
        if (iface.isValid()) {
            QDBusReply<bool> reply = iface.call("checkServiceRunning");
            if (reply.isValid()) {
                qDebug() << "Reply from checkServiceRunning was: "
                         << reply.value();
                answer = reply.value();
            } else
                qCritical() << "Call to checkServiceRunning failed:"
                            << qPrintable(reply.error().message());
        } else
            qCritical() << "QDBusInterface is invalid.\n";
        return answer;
    }
    void setServiseRuning(){
        qDebug() << __PRETTY_FUNCTION__;
        QDBusConnection dbus = QDBusConnection::systemBus();
        if (!dbus.isConnected())
            qCritical() << "Cannot connect to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "connect to the D-Bus system bus succesful";

        if (!dbus.registerService(SERVICE_NAME))
            qCritical() << "Cannot registerService to the D-Bus system bus.\n"
                        << dbus.lastError().message();
        else
            qDebug() << "registerService succesful";

        if (!dbus.registerObject(OBJECT_NAME, this,
                                 QDBusConnection::ExportAllSlots |
                                 QDBusConnection::ExportAllSignals))
            qCritical() << "Cannot registerObject to the D-Bus system bus."
                        << dbus.lastError().message();
        else
            qDebug() << "registerObject succesful";
    }

public slots:
    bool checkGuiRunning()
    {
        qDebug() << __PRETTY_FUNCTION__;
        emit messageFromAnotherGui();
        return true;
    }
    bool checkServiceRunning()
    {
        qDebug() << __PRETTY_FUNCTION__;
        return true;
    }
    void sendMessageToService(QByteArray data){
        qDebug() << __PRETTY_FUNCTION__;
        messageToServiceStr messageToService;
        memcpy(&messageToService, data.constData(),
               sizeof(messageToServiceStr));
        qDebug() << messageToService.getSettings;
        emit messageFromGUIRecieved(messageToService);
    }
    QByteArray getConfigFile(){
        QByteArray configArray(sizeof(settingsStr),
                                   Qt::Initialization::Uninitialized);
        memcpy(configArray.data(), conf->getSettingsBuffer(),
               sizeof(settingsStr));
        return configArray;
    }
    int getPresetsCount(){
        int presetsCount = static_cast<int>(conf->getPresetsCount());
        return presetsCount;
    }
    QByteArray getPresetFile(int number){
        QByteArray presetArray(sizeof(presetStr),
                                   Qt::Initialization::Uninitialized);
        memcpy(presetArray.data(), conf->getPresetsList()->at(number),
               sizeof(presetStr));
        return presetArray;
    }
    void messageFromServiceSlot(){
        qDebug() << "__PRETTY_FUNCTION__";
    }

signals:
    void messageFromServiceSignal();
    void messageFromServiceRecieved(messageToGuiStr messageToGui);
    void messageFromGUIRecieved(messageToServiceStr messageToService);
    void messageFromAnotherGui();

private:
    CtrlSettings *conf;
};
#else //NOT BUILD_SERVICE and NOT WIN32
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus()
        : conf(new CtrlSettings)
    {
        qDebug() << __PRETTY_FUNCTION__;
        conf->checkSettings();
    }
    ~CtrlBus(){
        qDebug() << __PRETTY_FUNCTION__;
    }

    CtrlSettings *getSettingsFromFile(){
        return conf;
    }
    CtrlSettings *getSettingsFromService(){
        return conf;
    }

    void sendMessageToService(messageToServiceStr data){
        emit messageFromGUIRecieved(data);
    }
    void sendMessageToGui(messageToGuiStr data){
        emit messageFromServiceRecieved(data);
    }
    bool isGUIRuning(){
        return false;
    }
    void setGUIRuning(){}

signals:
    void messageFromServiceRecieved(messageToGuiStr messageToGui);
    void messageFromGUIRecieved(messageToServiceStr messageToService);
    void messageFromAnotherGui();

private:
    void guiAlreadyRunningCheck(){}

    CtrlSettings *conf;
};
#endif //BUILD_SERVICE
#endif //WIN32
#endif // CTRLBUS_H
