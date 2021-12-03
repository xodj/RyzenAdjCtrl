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

#ifdef BUILD_SERVICE
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QSharedMemory *bufferToService,
            QSharedMemory *bufferToServiceFlag,
            QSharedMemory *bufferToGui,
            QSharedMemory *bufferToGuiFlag,
            QSharedMemory *guiAlreadyRunning)
        : QObject(nullptr),
          bufferToService(bufferToService),
          bufferToServiceFlag(bufferToServiceFlag),
          bufferToGui(bufferToGui),
          bufferToGuiFlag(bufferToGuiFlag),
          guiAlreadyRunning(guiAlreadyRunning)
    {}
    ~CtrlBus(){
        refresh_timer->stop();
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
                    memcpy(&messageToService, bufferToService->data(), sizeof(messageToServiceStr));
                    emit messageFromGUIRecieved(messageToService);
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
            qDebug()<<"Ctrl Bus - Can't attach bufferToGuiFlag!";
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

    QTimer *refresh_timer;
};
#else //BUILD_SERVICE
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QSharedMemory *guiAlreadyRunning)
        : QObject(nullptr),
          guiAlreadyRunning(guiAlreadyRunning){}
    ~CtrlBus(){}

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
};
#endif //BUILD_SERVICE
#endif // CTRLBUS_H
