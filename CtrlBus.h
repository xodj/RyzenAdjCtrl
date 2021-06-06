#ifndef CTRLBUS_H
#define CTRLBUS_H

#include <QObject>
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
    CtrlBus(QString qSharedMemoryKey = sharedMemoryKey)
        : QObject(nullptr),
          bufferToService(new QSharedMemory("bufferToService:" + qSharedMemoryKey)),
          bufferToGui(new QSharedMemory("bufferToGui:" + qSharedMemoryKey)),
          guiAlreadyRunning(new QSharedMemory("guiAlreadyRunning:" + qSharedMemoryKey))
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
            }
            bufferToService->detach();
        }
    }
    bool isServiseRuning(){
        return bufferToService->attach();
    }
    void setServiseRuning(){
        bufferToService->create(buffer_size);
        bufferToGui->create(buffer_size);

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
        }
    }
    bool isGUIRuning(){
        return guiAlreadyRunning->attach();
    }
    void setGUIRuning(){
        guiAlreadyRunning->create(1);

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
        }
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
            }
            bufferToGui->detach();
        }
        if(data.size() > 0)
            emit messageFromServiceRecieved(data);
    }

    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;
    QSharedMemory *guiAlreadyRunning;

    QTimer *refresh_timer;
};
#else //BUILD_SERVICE
#ifdef WIN32
#include <QSharedMemory>
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QString qSharedMemoryKey = sharedMemoryKey)
        : QObject(nullptr),
          guiAlreadyRunning(new QSharedMemory("guiAlreadyRunning:" + qSharedMemoryKey))
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
#else //WIN32
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus() : QObject(nullptr){}
    ~CtrlBus(){}

    void sendMessageToService(QByteArray data){
        emit messageFromGUIRecieved(data);
    }
    void sendMessageToGui(QByteArray data){
        messageFromServiceRecieved(data);
    }
    bool isGUIRuning(){
        return true;
    }
    void setGUIRuning(){}

signals:
    void messageFromServiceRecieved(QByteArray data);
    void messageFromGUIRecieved(QByteArray data);
};
#endif //WIN32
#endif //BUILD_SERVICE
#endif // CTRLBUS_H
