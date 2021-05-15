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
#include <QDir>

#define logFileSizeTreshold 10000000

QString fileName = "Logs/RyzenAdjCtrl - Service.log";

void messageHandler(QtMsgType, const QMessageLogContext &, const QString &msg) {
    QFile log(fileName);
    QDateTime dt = QDateTime::currentDateTime();
    log.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&log);
    ts << dt.toString("dd.MM.yyyy hh:mm:ss ") << msg << '\n';
    log.close();
    std::cout << msg.toStdString() << std::endl;
}

int exitCommand(QSharedMemory *bufferToService) {
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
            return 0;
        } else {
            qDebug()<<"Service is not started.";
            return 1;
        }
}

void checkLogsSize() {
    QDir dir("Config");
    if (!dir.exists())
        dir.mkpath(".");
    dir.setPath("Logs");
    if (!dir.exists())
        dir.mkpath(".");

    QFile log("Logs/RyzenAdjCtrl - Gui.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("Logs/RyzenAdjCtrl - Gui.log");
    log.setFileName("Logs/RyzenAdjCtrl - Service.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("Logs/RyzenAdjCtrl - Service.log");
}

bool checkService(){
    QProcess process;
    QStringList powerShellCLI = {
        "Get-ScheduledTask -TaskName \"Startup RyzenAdjCtrl\"\n",
        "exit\n"
                                };
    qDebug()<<powerShellCLI;
    process.start("powershell", powerShellCLI);
    for(;!process.waitForStarted();){}
    for(;!process.waitForFinished();){}
    QString error = process.readAllStandardError();
    QString output = process.readAllStandardOutput();
    qDebug() << "\nRyzenAdjCtrl Check Service error:\n\n" << error;
    qDebug() << "\nRyzenAdjCtrl Check Service output:\n\n" << output;

    return (error.size() > 1);
}

void installService(){
    QString exePath = qApp->arguments().value(0);

    QByteArray workDir = exePath.toLatin1();
    qsizetype lastSlash = 0;
    for(qsizetype i = 0;i < workDir.size();i++){
        if(workDir[i] == QByteArray("\\")[0])
            lastSlash = i + 1;
    }
    workDir.remove(lastSlash, workDir.size() - lastSlash);

    QProcess process;
    QStringList powerShellCLI = {
        "$Trigger= New-ScheduledTaskTrigger -AtLogon\n",
        "$Action= New-ScheduledTaskAction -Execute \"" + exePath + "\" -Argument \"startup\" -WorkingDirectory \"" + QString(workDir) + "\"\n",
        "$task_settings= New-ScheduledTaskSettingsSet -DontStopIfGoingOnBatteries "
        "-AllowStartIfOnBatteries -DontStopOnIdleEnd  -ExecutionTimeLimit  (New-TimeSpan) "
        "-RestartCount 10 -RestartInterval (New-TimeSpan -Minutes 1)\n",
        "Register-ScheduledTask -TaskName \"Startup RyzenAdjCtrl\" -Trigger $Trigger -Action $Action -RunLevel Highest -Force  -Settings $task_settings\n",
        "Start-ScheduledTask \"Startup RyzenAdjCtrl\"\n",
        "exit\n", "-verb", "runas"
                                };
    qDebug()<<powerShellCLI;
    process.start("powershell", powerShellCLI);
    for(;!process.waitForStarted();){}
    for(;!process.waitForFinished();){}
    QString error = process.readAllStandardError();
    QString output = process.readAllStandardOutput();
    qDebug() << "\nRyzenAdjCtrl Install Service error:\n\n" << error;
    qDebug() << "\nRyzenAdjCtrl Install Service output:\n\n" << output;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("RyzenAdjCtrl Service Installed\n");
    msgBox.exec();
}

void uninstallService(){
    QProcess process;
    QStringList powerShellCLI = {
     /* "Stop-ScheduledTask -TaskName \"Startup RyzenAdjCtrl\"", */
        "Unregister-ScheduledTask \"Startup RyzenAdjCtrl\" -Confirm:$false\n",
        "exit\n"
                                };
    qDebug()<<powerShellCLI;
    process.start("powershell", powerShellCLI);
    for(;!process.waitForStarted();){}
    for(;!process.waitForFinished();){}
    QString error = process.readAllStandardError();
    QString output = process.readAllStandardOutput();
    qDebug() << "\nRyzenAdjCtrl Uninstall Service error:\n\n" << error;
    qDebug() << "\nRyzenAdjCtrl Uninstall Service output:\n\n" << output;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("RyzenAdjCtrl Service Uninstalled\n");
    msgBox.exec();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    checkLogsSize();
    qInstallMessageHandler(messageHandler);

    QString qSharedMemoryKey = sharedMemoryKey;
    QSharedMemory *bufferToService = new QSharedMemory("bufferToService:" + qSharedMemoryKey);

    if(a.arguments().contains("exit"))
        return exitCommand(bufferToService);
    if(a.arguments().contains("install")){
        exitCommand(bufferToService);
        if(checkService()) installService();
        else uninstallService();
        return 0;
    }

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
        fileName = "Logs/RyzenAdjCtrl - Gui.log";
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
