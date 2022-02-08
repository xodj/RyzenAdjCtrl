#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <iostream>
#include "CtrlSettings.h"
#include "CtrlService.h"
#include "CtrlGui.h"
#include "CtrlBus.h"
#include "CtrlConfig.h"

#define logFileSizeTreshold 10000000
#ifdef WIN32
#define LOGFILE "Logs/RyzenCtrl.log"
#define LOGFILESERVICE "Logs/RyzenCtrl - Service.log"
#define LOGFILEGUI "Logs/RyzenCtrl - Gui.log"
#define CONFIGDIR "Config"
#define LOGDIR "Logs"
#else
#define LOGFILE "/tmp/RyzenCtrl.log"
#define LOGFILESERVICE "/tmp/RyzenCtrl - Service.log"
#define LOGFILEGUI "/tmp/RyzenCtrl - Gui.log"
#define CONFIGDIR "/etc/RyzenCtrl/"
#define LOGDIR "/tmp/RyzenCtr/"
#endif

QString fileName = LOGFILE;

void messageHandler(QtMsgType, const QMessageLogContext &, const QString &msg) {
    QFile log(fileName);
    QDateTime dt = QDateTime::currentDateTime();
    log.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&log);
    ts << dt.toString("dd.MM.yyyy hh:mm:ss ") << msg << '\n';
    log.close();
    std::cout << msg.toStdString() << std::endl;
}

void checkLogsSize() {
    QDir dir(CONFIGDIR);
    if (!dir.exists())
        dir.mkpath(".");
    dir.setPath(LOGDIR);
    if (!dir.exists())
        dir.mkpath(".");

    QFile log(LOGFILEGUI);
    if(log.size() > logFileSizeTreshold)
        log.remove(LOGFILEGUI);
    log.setFileName(LOGFILESERVICE);
    if(log.size() > logFileSizeTreshold)
        log.remove(LOGFILESERVICE);
    log.setFileName(LOGFILE);
    if(log.size() > logFileSizeTreshold)
        log.remove(LOGFILE);

}

#ifdef BUILD_SERVICE

int exitCommand(CtrlBus *bus) {
        qDebug() << "Ctrl Main - Exit Message From CLI";
        messageToServiceStr messageToService;
        messageToService.exit = true;
        bus->sendMessageToService(messageToService);
        return 0;
}

bool checkService(){
    QProcess process;
    QStringList powerShellCLI = {
        "Get-ScheduledTask -TaskName \"Startup RyzenCtrl\"\n",
        "exit\n"
                                };
    qDebug()<< "Ctrl Main - " << powerShellCLI;
    process.start("powershell", powerShellCLI);
    for(;!process.waitForStarted();){}
    for(;!process.waitForFinished();){}
    QString error = process.readAllStandardError();
    QString output = process.readAllStandardOutput();
    qDebug() << "Ctrl Main - RyzenCtrl Check Service error:" << error;
    qDebug() << "Ctrl Main - RyzenCtrl Check Service output:" << output;

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
        "Register-ScheduledTask -TaskName \"Startup RyzenCtrl\" -Trigger $Trigger -Action $Action -RunLevel Highest -Force  -Settings $task_settings\n",
        "Start-ScheduledTask \"Startup RyzenCtrl\"\n",
        "exit\n", "-verb", "runas"
                                };
    qDebug()<< "Ctrl Main - " << powerShellCLI;
    process.start("powershell", powerShellCLI);
    for(;!process.waitForStarted();){}
    for(;!process.waitForFinished();){}
    QString error = process.readAllStandardError();
    QString output = process.readAllStandardOutput();
    qDebug() << "Ctrl Main - RyzenCtrl Install Service error:" << error;
    qDebug() << "Ctrl Main - RyzenCtrl Install Service output:" << output;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("RyzenCtrl Service Installed\n");
    msgBox.exec();
}

void uninstallService(){
    QProcess process;
    QStringList powerShellCLI = {
     /* "Stop-ScheduledTask -TaskName \"Startup RyzenCtrl\"", */
        "Unregister-ScheduledTask \"Startup RyzenCtrl\" -Confirm:$false\n",
        "exit\n"
                                };
    qDebug() << "Ctrl Main - " << powerShellCLI;
    process.start("powershell", powerShellCLI);
    for(;!process.waitForStarted();){}
    for(;!process.waitForFinished();){}
    QString error = process.readAllStandardError();
    QString output = process.readAllStandardOutput();
    qDebug() << "Ctrl Main - RyzenCtrl Uninstall Service error:" << error;
    qDebug() << "Ctrl Main - RyzenCtrl Uninstall Service output:" << output;

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("RyzenCtrl Service Uninstalled\n");
    msgBox.exec();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    checkLogsSize();
    qInstallMessageHandler(messageHandler);

    if(!a.arguments().contains("startup"))
        fileName = LOGFILEGUI;
    else
        fileName = LOGFILESERVICE;

#ifdef WIN32
    QString qSharedMemoryKey = sharedMemoryKey;
    QSharedMemory *bufferToService =
            new QSharedMemory("bufferToService:" + qSharedMemoryKey);
    QSharedMemory *bufferToServiceFlag =
            new QSharedMemory("bufferToServiceFlag:" + qSharedMemoryKey);
    QSharedMemory *bufferToGui =
            new QSharedMemory("bufferToGui:" + qSharedMemoryKey);
    QSharedMemory *bufferToGuiFlag =
            new QSharedMemory("bufferToGuiFlag:" + qSharedMemoryKey);
    QSharedMemory *guiAlreadyRunning =
            new QSharedMemory("guiAlreadyRunning:" + qSharedMemoryKey);
    QSharedMemory *bufferSettingsToGui =
            new QSharedMemory("bufferSettingsToGui:" + qSharedMemoryKey);
    QSharedMemory *bufferSettingsToGuiFlag =
            new QSharedMemory("bufferSettingsToGuiFlag:" + qSharedMemoryKey);

    CtrlBus *bus = new CtrlBus(bufferToService,
                               bufferToServiceFlag,
                               bufferToGui,
                               bufferToGuiFlag,
                               guiAlreadyRunning,
                               bufferSettingsToGui,
                               bufferSettingsToGuiFlag);
#else
    CtrlBus *bus = new CtrlBus;
#endif //WIN32

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
            new CtrlService(bus);
        }
    } else {
        if(bus->isGUIRuning()){
            qDebug() << "Ctrl Main - Application Is Already Running.";
            qDebug() << "Ctrl Main - Exit.";
            return 1;
        } else {
            new CtrlGui(bus);
        }
    }
    return a.exec();
}
#else //BUILD_SERVICE
#ifdef WIN32
#include "lib/ryzenadj.h"
bool sudoersCheck(){
    qDebug() << "Ctrl Main - Check for Administator priviliges...";
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if( OpenProcessToken( GetCurrentProcess( ),TOKEN_QUERY,&hToken ) ) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof( TOKEN_ELEVATION );
        if( GetTokenInformation( hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize ) ) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if(hToken) {
        CloseHandle( hToken );
    }
    qDebug() << "Ctrl Main - Administator priviliges:" << (fRet ? "True" : "False");
    if(!fRet) {
        qDebug() << "Ctrl Main - Try to get pm tables without administator priviliges...";
        ryzen_access ry = init_ryzenadj();
        if(ry != NULL){
            qDebug() << "Ctrl Main - Have access without administator priviliges...";
            cleanup_ryzenadj(ry);
            fRet = true;
        }
    }
    if(!fRet) {
        qDebug() << "Ctrl Main - Try to run with Administator priviliges...";
        QProcess process;
        QString runas = ("\"" + qApp->arguments().value(0) + "\"");
        process.startDetached("powershell", QStringList({"start-process", runas, "-verb", "runas"}));
        qDebug() << "Ctrl Main - Exit.";
    }
    return fRet;
}
#else //WIN32
#include <unistd.h>
bool sudoersCheck(){
    if (int(getuid()) != 0){
        qDebug() << "Ctrl Main - Need sudo priviliges!";
        QMessageBox msgBox;
        msgBox.setWindowIcon(QIcon(":/main/amd_icon.ico"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Need sudo priviliges!\n");
        msgBox.exec();
        return false;
    } else
        return true;
}
#endif //WIN32
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    checkLogsSize();
    qInstallMessageHandler(messageHandler);
    if(!sudoersCheck())
        return 1;
#ifdef WIN32
    CtrlBus *bus =
            new CtrlBus(
                new QSharedMemory(
                    "guiAlreadyRunning:"
                    + QString(sharedMemoryKey)
                    )
                );

    if(bus->isGUIRuning()){
        qDebug() << "Ctrl Main - Application Is Already Running.";
        qDebug() << "Ctrl Main - Exit.";
        return 1;
    } else {
        new CtrlService(bus);
        new CtrlGui(bus);
    }
#else //Linux
    CtrlBus *bus = new CtrlBus;
    new CtrlService(bus);
    new CtrlGui(bus);
#endif

    return a.exec();
}
#endif //BUILD_SERVICE
