#include "CtrlAgent.h"

CtrlAgent::CtrlAgent(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf)
    : QSystemTrayIcon(new QSystemTrayIcon),
      bufferToService(bufferToService),
      bufferToGui(bufferToGui),
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

    action = new QAction("Close", this);
    QIcon exitIcon(":/main/application-exit.png");
    action->setIcon(exitIcon);
    connect(action, SIGNAL(triggered()), this, SLOT(closeAgent()));
    trayMenu->addAction(action);

    this->setContextMenu(trayMenu);
    this->show();

    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    gui = new CtrlGuiX(bufferToService, bufferToGui, conf);
    connect(gui, &CtrlGuiX::messageToAgent, this, &CtrlAgent::notificationToTray);
}

CtrlAgent::~CtrlAgent(){}

void CtrlAgent::openCtrl(){
    if (gui == nullptr) {
        gui = new CtrlGuiX(bufferToService, bufferToGui, conf);
    }
    gui->show();
}

void CtrlAgent::iconActivated(QSystemTrayIcon::ActivationReason reason){
    if(reason == ActivationReason::DoubleClick){
        openCtrl();
    }
}

void CtrlAgent::closeAgent(){ exit(0); }

void CtrlAgent::notificationToTray(QString message){
    if(conf->getSettings()->showNotifications)
        if(lastMessage != message) {
            lastMessage = message;
            this->showMessage("RyzenAdjCtrl", message);
    }
}
