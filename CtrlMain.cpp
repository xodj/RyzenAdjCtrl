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

QString fileName = "RyzenAdjCtrl - Service.log";

void messageHandler(QtMsgType, const QMessageLogContext &, const QString &msg) {
    QFile log(fileName);
    QDateTime dt = QDateTime::currentDateTime();
    log.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&log);
    ts << dt.toString("dd.MM.yyyy hh:mm:ss ") << msg << '\n';
    log.close();
    std::cout << msg.toStdString() << std::endl;
}

void exitCommand(QSharedMemory *bufferToService) {
        qDebug() << "Exit Message From CLI";
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
}

void checkLogsSize() {
    QFile log("RyzenAdjCtrl - Gui.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("RyzenAdjCtrl - Gui.log");
    log.setFileName("RyzenAdjCtrl - Service.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("RyzenAdjCtrl - Service.log");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    checkLogsSize();
    qInstallMessageHandler(messageHandler);

    QString qSharedMemoryKey = sharedMemoryKey;
    QSharedMemory *bufferToService = new QSharedMemory("bufferToService:" + qSharedMemoryKey);

    if(a.arguments().contains("exit")){ exitCommand(bufferToService); }

    QSharedMemory alreadyRunning("guiAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory serviceAlreadyRunning("serviceAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory *bufferToGui = new QSharedMemory("bufferToGui:" + qSharedMemoryKey);

    if(a.arguments().contains("startup")) {
        if(serviceAlreadyRunning.attach()){
            qDebug() << "Service Is Already Running.";
            qDebug() << "Exit.";
            return 1;
        } else {
            serviceAlreadyRunning.create(1);
            (new CtrlService(bufferToService, bufferToGui, new CtrlSettings));
        }
    } else {
        fileName = "RyzenAdjCtrl - Gui.log";
        if(alreadyRunning.attach()){
            qDebug() << "Application Is Already Running.";
            qDebug() << "Exit.";
            return 1;
        } else {
            alreadyRunning.create(1);
            new CtrlGui(bufferToService, bufferToGui, new CtrlSettings);
        }
    }
    return a.exec();
}
