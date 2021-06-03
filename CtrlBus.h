#ifndef CTRLBUS_H
#define CTRLBUS_H

#include <QObject>
#include "CtrlConfig.h"

#ifdef WIN32
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
#else //WIN32
#include <QtDBus/QtDBus>
class CtrlBus : public QObject
{
    Q_OBJECT
public:
    CtrlBus(QString qSharedMemoryKey = sharedMemoryKey)
        : QObject(nullptr)
    {
        if (!QDBusConnection::sessionBus().isConnected()) {
            fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                    "To start it, run:\n"
                    "\teval `dbus-launch --auto-syntax`\n");
            exit(1);
        }
    }
    ~CtrlBus(){}

    void sendMessageToService(QByteArray data){
        busInterface->call(QDBus::NoBlock, "messageFromGui", data.constData());
    }
    void sendMessageToGui(QByteArray data){
        busInterface->call(QDBus::NoBlock, "messageFromService", data.constData());
    }
    void setServiseRuning(){
        QDBusConnection::sessionBus().registerService("by.xodj.ryzenadjctrl.service");
        QDBusConnection::sessionBus().registerObject("/toService", this, QDBusConnection::ExportAllSlots);
        busInterface = new QDBusInterface("by.xodj.ryzenadjctrl.gui", "/toGui", "", QDBusConnection::sessionBus());
    }
    void setGUIRuning(){
        QDBusConnection::sessionBus().registerService("by.xodj.ryzenadjctrl.gui");
        QDBusConnection::sessionBus().registerObject("/toGui", this, QDBusConnection::ExportAllSlots);
        busInterface = new QDBusInterface("by.xodj.ryzenadjctrl.service", "/toService", "", QDBusConnection::sessionBus());
    }

    bool isServiseRuning(){
        bool return_ = true;
        if(QDBusConnection::sessionBus().registerService("by.xodj.ryzenadjctrl.service")){
            return_ = false;
            QDBusConnection::sessionBus().unregisterService("by.xodj.ryzenadjctrl.service");
        }
        return return_;
    }
    bool isGUIRuning(){
        bool return_ = true;
        if(QDBusConnection::sessionBus().registerService("by.xodj.ryzenadjctrl.gui")){
            return_ = false;
            QDBusConnection::sessionBus().unregisterService("by.xodj.ryzenadjctrl.gui");
        }
        return return_;
    }

signals:
    void messageFromServiceRecieved(QByteArray data);
    void messageFromGUIRecieved(QByteArray data);

public slots:
    void messageFromGui(const QString &arg){
        if(arg.size() > 0)
            emit messageFromGUIRecieved(arg.toLatin1());
    }
    void messageFromService(const QString &arg){
        if(arg.size() > 0)
            emit messageFromServiceRecieved(arg.toLatin1());
    }

private:
    QDBusInterface *busInterface;
};
#endif //WIN32
#endif // CTRLBUS_H
