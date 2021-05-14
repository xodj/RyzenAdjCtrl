#include "CtrlAgent.h"

CtrlAgent::CtrlAgent(CtrlSettings *conf)
    : QSystemTrayIcon(new QSystemTrayIcon),
      conf(conf)
{
    QIcon icon(":/main/amd_icon.ico");
    this->setIcon(icon);
    this->setToolTip("RyzenAdjCtrl" "\n"
                     "");
    trayMenu = new QMenu;

    QAction *action = new QAction("Open RyzenAdjCtrl", this);
    action->setIcon(icon);
    connect(action, SIGNAL(triggered()), this, SIGNAL(showCtrlGui()));
    trayMenu->addAction(action);

    action = new QAction("Close", this);
    QIcon exitIcon(":/main/application-exit.png");
    action->setIcon(exitIcon);
    connect(action, SIGNAL(triggered()), this, SIGNAL(closeCtrlGui()));
    trayMenu->addAction(action);

    this->setContextMenu(trayMenu);
    this->show();

    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

CtrlAgent::~CtrlAgent(){
    disconnect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void CtrlAgent::iconActivated(QSystemTrayIcon::ActivationReason reason){
    if(reason == ActivationReason::DoubleClick){
        emit showCtrlGui();
    } else if(reason == ActivationReason::Trigger){
        emit showCtrlMiniGui();
    }
}

void CtrlAgent::notificationToTray(QString message){
    if(conf->getSettingsBuffer()->showNotifications)
        if(lastMessage != message) {
            lastMessage = message;
            this->showMessage("Preset Switch", message);
    }
}
