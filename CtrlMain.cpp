#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QXmlStreamWriter>
#include "CtrlConfig.h"
#include "CtrlSettings.h"
#include "CtrlService.h"
#include "CtrlGui.h"
#include "CtrlAgent.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <iostream>

#define logFileSizeTreshold 10000000

void serviceMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg){
    QFile log("RyzenAdjCtrl - Service.log");
    QDateTime dt = QDateTime::currentDateTime();
    log.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&log);
    ts << dt.toString("dd.MM.yyyy hh:mm:ss ") << msg << '\n';
    log.close();
    std::cout << msg.toStdString() << std::endl;
}

void guiMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg){
    QFile log("RyzenAdjCtrl - Gui.log");
    QDateTime dt = QDateTime::currentDateTime();
    log.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&log);
    ts << dt.toString("dd.MM.yyyy hh:mm:ss ") << msg << '\n';
    log.close();
    std::cout << msg.toStdString() << std::endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile log("RyzenAdjCtrl - Gui.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("RyzenAdjCtrl - Gui.log");
    log.setFileName("RyzenAdjCtrl - Service.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("RyzenAdjCtrl - Service.log");

    QString qSharedMemoryKey = sharedMemoryKey;
    QSharedMemory alreadyRunning("guiAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory serviceAlreadyRunning("serviceAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory *bufferToService = new QSharedMemory("bufferToService:" + qSharedMemoryKey);
    QSharedMemory *bufferToGui = new QSharedMemory("bufferToGui:" + qSharedMemoryKey);

    CtrlSettings *conf = new CtrlSettings;

    if(a.arguments().contains("startup")) {
        qInstallMessageHandler(serviceMessageHandler);
        if(serviceAlreadyRunning.attach()){
            qDebug() << "The service is already running.";
            return 1;
        } else {
            serviceAlreadyRunning.create(1);
            (new CtrlService(bufferToService, bufferToGui, conf));
        }
    } else if(a.arguments().contains("exit")){
        qInstallMessageHandler(serviceMessageHandler);
        qDebug() << "Exit message from cli";
        QByteArray data;
        QXmlStreamWriter argsWriter(&data);
        argsWriter.setAutoFormatting(true);
        argsWriter.writeStartDocument();
        argsWriter.writeStartElement("bufferToService");
        //
            argsWriter.writeStartElement("exit");
            argsWriter.writeEndElement();
        //
        argsWriter.writeEndElement();
        argsWriter.writeEndDocument();

        if(bufferToService->attach(QSharedMemory::ReadWrite)) {
            char *iodata = (char*)bufferToService->data();
            if (bufferToService->lock()) {
                for (int i=0;i<data.size();i++)
                    iodata[i] = data[i];
                bufferToService->unlock();
            }
            bufferToService->detach();
        }
        exit(0);
    } else {
        qInstallMessageHandler(guiMessageHandler);
        if(alreadyRunning.attach()){
            qDebug() << "The application is already running.";
            return 1;
        } else {
            alreadyRunning.create(1);
            if (conf->getSettings()->useAgent)
                (new CtrlAgent(bufferToService, bufferToGui, conf))->show();
            else
                (new CtrlGui(bufferToService, bufferToGui, conf))->show();
        }
    }
    return a.exec();
}
