#ifndef CTRLBUS_H
#define CTRLBUS_H

#include <QObject>
#include <QDebug>
#include "CtrlConfig.h"

#ifdef BUILD_SERVICE
#include <QSharedMemory>
#include <QTimer>

#define buffer_size 512
#define refresh_time 33

class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QSharedMemory *bufferToService,
            QSharedMemory *bufferToGui,
            QSharedMemory *guiAlreadyRunning)
        : QObject(nullptr),
          bufferToService(bufferToService),
          bufferToGui(bufferToGui),
          guiAlreadyRunning(guiAlreadyRunning)
    {}
    ~CtrlBus(){
        refresh_timer->stop();
    }

    void sendMessageToService(QByteArray data){
        if(bufferToService->attach(QSharedMemory::ReadWrite)){
            char *iodata = (char*)bufferToService->data();
            if (bufferToService->lock()) {
                for (int i=0;i<data.size();i++)
                    iodata[i] = data[i];
                bufferToService->unlock();
            } else
                qDebug()<<"Ctrl Bus - Can't lock bufferToService!";
            bufferToService->detach();
        } else
            qDebug()<<"Ctrl Bus - Can't attach bufferToService!";
        bufferToService->detach();
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
        if(!bufferToService->create(buffer_size))
            qDebug()<<"Ctrl Bus - Can't create bufferToService!";
        if(!bufferToGui->create(buffer_size))
            qDebug()<<"Ctrl Bus - Can't create bufferToGui!";

        refresh_timer = new QTimer;
        connect(refresh_timer, &QTimer::timeout,
                this, &CtrlBus::getMessageFromGui);
        refresh_timer->start(refresh_time);
    }

    void sendMessageToGui(QByteArray data){
        char *iodata = (char*)bufferToGui->data();
        if (bufferToGui->lock()) {
            for (int i=0;i<data.size();i++)
                iodata[i] = data[i];
            bufferToGui->unlock();
        } else
            qDebug()<<"Ctrl Bus - Can't lock bufferToGui!";
    }
    bool isGUIRuning(){
        bool bReturn = false;
        if(guiAlreadyRunning->attach()){
            guiAlreadyRunning->detach();
            bReturn = true;
        }
        return bReturn;
    }
    void setGUIRuning(){
        if(!guiAlreadyRunning->create(1))
            qDebug()<<"Ctrl Bus - Can't create guiAlreadyRunning!";

        refresh_timer = new QTimer;
        connect(refresh_timer, &QTimer::timeout,
                this, &CtrlBus::getMessageFromService);
        refresh_timer->start(refresh_time);
    }

signals:
    void messageFromServiceRecieved(QByteArray data);
    void messageFromGUIRecieved(QByteArray data);

private:
    void getMessageFromGui(){
        char *iodata = (char*)bufferToService->data();
        QByteArray data;
        if (bufferToService->lock())
        {
          for (int i=0;iodata[i];i++) {
            data.append(iodata[i]);
            iodata[i] = '\0';
          }
          bufferToService->unlock();
        } else
            qDebug()<<"Ctrl Bus - Can't lock bufferToService!";
        if(data.size() > 0)
            emit messageFromGUIRecieved(data);
    }
    void getMessageFromService(){
        QByteArray data;
        if(bufferToGui->attach(QSharedMemory::ReadWrite)){
            if (bufferToGui->lock())
            {
                char *iodata = (char*)bufferToGui->data();
                for (int i=0;iodata[i];i++) {
                    data.append(iodata[i]);
                    iodata[i] = '\0';
                }
                bufferToGui->unlock();
            } else
                qDebug()<<"Ctrl Bus - Can't lock bufferToGui!";
            bufferToGui->detach();
        } else
            qDebug()<<"Ctrl Bus - Can't attach bufferToGui!";
        if(data.size() > 0)
            emit messageFromServiceRecieved(data);
    }

    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;
    QSharedMemory *guiAlreadyRunning;

    QTimer *refresh_timer;
};
#else //BUILD_SERVICE
#include <QSharedMemory>
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QSharedMemory *guiAlreadyRunning)
        : QObject(nullptr),
          guiAlreadyRunning(guiAlreadyRunning)
    {}
    ~CtrlBus(){}

    void sendMessageToService(QByteArray data){
        emit messageFromGUIRecieved(data);
    }
    void sendMessageToGui(QByteArray data){
        messageFromServiceRecieved(data);
    }
    bool isGUIRuning(){
        return guiAlreadyRunning->attach();
    }
    void setGUIRuning(){
        guiAlreadyRunning->create(1);
    }

signals:
    void messageFromServiceRecieved(QByteArray data);
    void messageFromGUIRecieved(QByteArray data);

private:
    QSharedMemory *guiAlreadyRunning;
};
#endif //BUILD_SERVICE
#endif // CTRLBUS_H
