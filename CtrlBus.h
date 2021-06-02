#ifndef CTRLBUS_H
#define CTRLBUS_H

#include <QObject>
#include <QSharedMemory>
#include <QTimer>
#include "CtrlConfig.h"

//LINUX DBUS
#include <QtDBus/QtDBus>

#define buffer_size 512
#define refresh_time 33

#ifdef WIN32
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

    bool sendMessageToService(QByteArray data){
        if(bufferToService->attach(QSharedMemory::ReadWrite)){
            char *iodata = (char*)bufferToService->data();
            if (bufferToService->lock()) {
                for (int i=0;i<data.size();i++)
                    iodata[i] = data[i];
                bufferToService->unlock();
            }
            bufferToService->detach();
            return true;
        } else
            return false;
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

    bool sendMessageToGui(QByteArray data){
        char *iodata = (char*)bufferToGui->data();
        if (bufferToGui->lock()) {
            for (int i=0;i<data.size();i++)
                iodata[i] = data[i];
            bufferToGui->unlock();
            return true;
        } else
            return false;
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
#else //WIN32
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QString qSharedMemoryKey = sharedMemoryKey)
        : QObject(nullptr),
          bufferToService(new QSharedMemory("bufferToService:" + qSharedMemoryKey)),
          bufferToGui(new QSharedMemory("bufferToGui:" + qSharedMemoryKey)),
          guiAlreadyRunning(new QSharedMemory("guiAlreadyRunning:" + qSharedMemoryKey))
    {

        if (!QDBusConnection::sessionBus().isConnected()) {
            fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                    "To start it, run:\n"
                    "\teval `dbus-launch --auto-syntax`\n");
        }

        if (!QDBusConnection::sessionBus().registerService("by.xodj.ryzenadjctrl")) {
            fprintf(stderr, "%s\n",
                    qPrintable(QDBusConnection::sessionBus().lastError().message()));
            exit(1);
        }

        //QDBusConnection::sessionBus().registerObject("/toGui","by.xodj.ryzenadjctrl",&QObject);


        busInterface = new QDBusInterface("by.xodj.ryzenadjctrl", "/toGui", "", QDBusConnection::sessionBus());
        if (busInterface->isValid())
            QDBusReply<QString> reply = busInterface->call("ping", "");
    }
    ~CtrlBus(){
        refresh_timer->stop();
    }

    bool sendMessageToService(QByteArray data){
        if(bufferToService->attach(QSharedMemory::ReadWrite)){
            char *iodata = (char*)bufferToService->data();
            if (bufferToService->lock()) {
                for (int i=0;i<data.size();i++)
                    iodata[i] = data[i];
                bufferToService->unlock();
            }
            bufferToService->detach();
            return true;
        } else
            return false;
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

    bool sendMessageToGui(QByteArray data){
        char *iodata = (char*)bufferToGui->data();
        if (bufferToGui->lock()) {
            for (int i=0;i<data.size();i++)
                iodata[i] = data[i];
            bufferToGui->unlock();
            return true;
        } else
            return false;
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

    QDBusInterface *busInterface;

    QTimer *refresh_timer;
};
#endif //WIN32
#endif // CTRLBUS_H
