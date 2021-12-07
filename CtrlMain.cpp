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

QString fileName = "Logs/RyzenCtrl - Service.log";

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
#ifdef WIN32
    QDir dir("Config");
    if (!dir.exists())
        dir.mkpath(".");
    dir.setPath("Logs");
    if (!dir.exists())
        dir.mkpath(".");

    QFile log("Logs/RyzenCtrl - Gui.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("Logs/RyzenCtrl - Gui.log");
    log.setFileName("Logs/RyzenCtrl - Service.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("Logs/RyzenCtrl - Service.log");
    log.setFileName("Logs/RyzenCtrl.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("Logs/RyzenCtrl.log");
#else //WIN32
    QDir dir("/etc/RyzenCtrl/");
    if (!dir.exists())
        dir.mkpath(".");

    QFile log("/etc/RyzenCtrl/RyzenCtrl.log");
    if(log.size() > logFileSizeTreshold)
        log.remove("/etc/RyzenCtrl/RyzenCtrl.log");
#endif //WIN32
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

    CtrlBus *bus = new CtrlBus(bufferToService,
                               bufferToServiceFlag,
                               bufferToGui,
                               bufferToGuiFlag,
                               guiAlreadyRunning);

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
        fileName = "Logs/RyzenCtrl - Gui.log";
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
        /*QProcess process;
        process.start("pkexec", QStringList({"env", "DISPLAY=$DISPLAY", "XAUTHORITY=$XAUTHORITY", qApp->arguments().value(0)}));
        for(;!process.waitForStarted();){}
        for(;!process.waitForFinished();){}
        QString error = process.readAllStandardError();
        QString output = process.readAllStandardOutput();
        qDebug() << "Ctrl Main - error:" << error;
        qDebug() << "Ctrl Main - output:" << output;
        for(;true;){}*/
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
#ifdef WIN32
    fileName = "Logs/RyzenCtrl.log";
#else //Linux
    fileName = "/etc/RyzenCtrl/RyzenCtrl.log";
#endif

    if(!sudoersCheck())
        return 1;

    QSharedMemory *guiAlreadyRunning = new QSharedMemory("guiAlreadyRunning:" + QString(sharedMemoryKey));
    CtrlBus *bus = new CtrlBus(guiAlreadyRunning);
    if(bus->isGUIRuning()){
        qDebug() << "Ctrl Main - Application Is Already Running.";
        qDebug() << "Ctrl Main - Exit.";
        return 1;
    } else {
        new CtrlService(bus, new CtrlSettings);
        new CtrlGui(bus, new CtrlSettings);
    }

    return a.exec();
}
#endif //BUILD_SERVICE
