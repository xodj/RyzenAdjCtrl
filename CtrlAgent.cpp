#include "CtrlAgent.h"
#include <QProcess>
#include <QMessageBox>

CtrlAgent::CtrlAgent(QSharedMemory *bufferToService, CtrlSettings *conf)
    : QSystemTrayIcon(new QSystemTrayIcon),
      bufferToService(bufferToService),
      conf(conf)
{
    QIcon icon(":/main/amd_icon.ico");
    this->setIcon(icon);
    this->setToolTip("RyzenAdjCtrl" "\n"
                     "");
    trayMenu = new QMenu;

    QAction *action = new QAction("Open RyzenAdjCtrl", this);
    action->setIcon(icon);
    connect(action, SIGNAL(triggered()), this, SLOT(openCtrl()));
    trayMenu->addAction(action);

    action = new QAction("Close Agent", this);
    QIcon exitIcon(":/main/application-exit.png");
    action->setIcon(exitIcon);
    connect(action, SIGNAL(triggered()), this, SLOT(closeAgent()));
    trayMenu->addAction(action);

    this->setContextMenu(trayMenu);
    this->show();

    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    if(bufferToService->attach(QSharedMemory::ReadWrite))
        bufferToService->detach();
    else {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("RyzenAdjCtrl Service is not runing!\nTry to run...");
        msgBox.exec();
        QProcess process;
        QString runas = ("" + qApp->arguments().value(0) + " startup");
        process.startDetached("powershell", QStringList({"start-process", runas, "-verb", "runas"}));
    }
}

CtrlAgent::~CtrlAgent(){}

void CtrlAgent::openCtrl(){
    if (gui == nullptr) {
        gui = new CtrlGuiX(bufferToService, conf);
    }
    gui->show();
}

void CtrlAgent::iconActivated(QSystemTrayIcon::ActivationReason reason){
    if(reason == ActivationReason::DoubleClick){
        openCtrl();
    }
}

void CtrlAgent::closeAgent(){ exit(0); }
