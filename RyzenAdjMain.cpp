#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include "RyzenAdjCtrl.h"
#include "RyzenAdjGui.h"
#include <QXmlStreamWriter>
#include "Config.h"

int errorMessage(QString message)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(message);
    msgBox.exec();
    return 1;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString qSharedMemoryKey = sharedMemoryKey;
    QSharedMemory guiAlreadyRunning("guiAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory ctrlAlreadyRunning("ctrlAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory *bufferToService = new QSharedMemory("bufferToService:" + qSharedMemoryKey);

    RyzenAdjCfg *conf = new RyzenAdjCfg;

    if(a.arguments().contains("startup")) {
        if(ctrlAlreadyRunning.attach()){
            return errorMessage("The service is already running.\n"
                                "Allowed to run only one instance of the application.");
        } else {
            ctrlAlreadyRunning.create(1);
            (new RyzenAdjCtrl(bufferToService, conf));
        }
    } else if(a.arguments().contains("agent")){
        return errorMessage("The application agent not working yet.");
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
        if(guiAlreadyRunning.attach()){
            return errorMessage("The application is already running.\n"
                                "Allowed to run only one instance of the application.");
        } else {
            guiAlreadyRunning.create(1);
            (new RyzenAdjGui(bufferToService, conf))->show();
        }
    }
    return a.exec();
}
