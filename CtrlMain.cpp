#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QXmlStreamWriter>
#include "CtrlConfig.h"
#include "CtrlSettings.h"
#include "CtrlService.h"
#include "CtrlGui.h"
#include "CtrlAgent.h"

int errorMessage(QString message)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(message);
    msgBox.exec();
    return 1;
}

int infoMessage(QString message)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(message);
    msgBox.exec();
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString qSharedMemoryKey = sharedMemoryKey;
    QSharedMemory alreadyRunning("guiAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory serviceAlreadyRunning("serviceAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory *bufferToService = new QSharedMemory("bufferToService:" + qSharedMemoryKey);
    QSharedMemory *bufferToGui = new QSharedMemory("bufferToGui:" + qSharedMemoryKey);

    CtrlSettings *conf = new CtrlSettings;

    if(a.arguments().contains("startup")) {
        if(serviceAlreadyRunning.attach()){
            return errorMessage("The service is already running.\n"
                                "Allowed to run only one instance of the application.");
        } else {
            serviceAlreadyRunning.create(1);
            (new CtrlService(bufferToService, bufferToGui, conf));
        }
    } else if(a.arguments().contains("exit")){
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
        if(alreadyRunning.attach()){
            return errorMessage("The application is already running.\n"
                                "Allowed to run only one instance of the application.");
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
