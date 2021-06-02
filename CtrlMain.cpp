#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QXmlStreamWriter>
#include <iostream>
#include "CtrlSettings.h"
#include "CtrlService.h"
#include "CtrlGui.h"
#include "CtrlBus.h"

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

int exitCommand(CtrlBus *bus) {
        qDebug() << "Ctrl Main - Exit Message From CLI";
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

        bus->sendMessageToService(data);

        return 0;
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
    qDebug() << "\nCtrl Main - RyzenAdjCtrl Check Service error:\n\n" << error;
    qDebug() << "\nCtrl Main - RyzenAdjCtrl Check Service output:\n\n" << output;

    return (error.size() > 1);
}

void installService(){
    QString exePath = qApp->arguments().value(0);

    QByteArray workDir = exePath.toLatin1();
    int lastSlash = 0;
    for(int i = 0;i < workDir.size();i++){
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
    qDebug() << "\nCtrl Main - RyzenAdjCtrl Install Service error:\n\n" << error;
    qDebug() << "\nCtrl Main - RyzenAdjCtrl Install Service output:\n\n" << output;

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
    qDebug() << "\nCtrl Main - RyzenAdjCtrl Uninstall Service error:\n\n" << error;
    qDebug() << "\nCtrl Main - RyzenAdjCtrl Uninstall Service output:\n\n" << output;

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

    CtrlBus *bus = new CtrlBus;

    if(a.arguments().contains("exit"))
        return exitCommand(bus);
    if(a.arguments().contains("check")){
        exitCommand(bus);
        if(checkService()) installService();
        else uninstallService();
        return 0;
    }
    if(a.arguments().contains("install")){
        exitCommand(bus);
        installService();
        return 0;
    }
    if(a.arguments().contains("uninstall")){
        exitCommand(bus);
        uninstallService();
        return 0;
    }

    if(a.arguments().contains("startup")) {
        if(bus->isServiseRuning()){
            qDebug() << "Ctrl Main - Service Is Already Running.";
            qDebug() << "Ctrl Main - Exit.";
            return 1;
        } else {
            (new CtrlService(bus, new CtrlSettings));
        }
    } else {
        fileName = "Logs/RyzenAdjCtrl - Gui.log";
        if(bus->isGUIRuning()){
            qDebug() << "Ctrl Main - Application Is Already Running.";
            qDebug() << "Ctrl Main - Exit.";
            return 1;
        } else {
            new CtrlGui(bus, new CtrlSettings);
        }
    }
    return a.exec();
}
