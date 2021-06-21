#ifndef CTRLAGENT_H
#define CTRLAGENT_H

#include <QtWidgets/QSystemTrayIcon>
#include <QtWidgets/QMenu>
#include <QAction>
#include "CtrlSettings.h"

class CtrlAgent : public QSystemTrayIcon
{
    Q_OBJECT
public:
    CtrlAgent(CtrlSettings *conf)
        : QSystemTrayIcon(new QSystemTrayIcon),
          conf(conf)
    {
        QIcon icon(":/main/amd_icon.ico");
        this->setIcon(icon);
        this->setToolTip("RyzenAdjCtrl" "\n"
                         "");

        QMenu *trayMenu = new QMenu;

        QAction *action = new QAction(icon, "Open RyzenAdjCtrl", this);
        connect(action, SIGNAL(triggered()), this, SIGNAL(showCtrlGui()));
        trayMenu->addAction(action);

        showInfoWidgetAction = new QAction("Show Info Widget", this);
        showInfoWidgetAction->setCheckable(true);
        QIcon checkIcon;
        checkIcon.addFile(QString::fromUtf8(":/main/unchecked.png"), QSize(), QIcon::Selected, QIcon::Off);
        checkIcon.addFile(QString::fromUtf8(":/main/checked.png"), QSize(), QIcon::Selected, QIcon::On);
        showInfoWidgetAction->setIcon(checkIcon);
        connect(showInfoWidgetAction, &QAction::triggered, this, &CtrlAgent::showCtrlInfoWidget);
        trayMenu->addAction(showInfoWidgetAction);

        action = new QAction(QIcon(":/main/application-exit.png"), "Close", this);
        connect(action, SIGNAL(triggered()), this, SIGNAL(closeCtrlGui()));
        trayMenu->addAction(action);

        this->setContextMenu(trayMenu);
        this->show();

        connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    }
    ~CtrlAgent() {
        disconnect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

        disconnect(showInfoWidgetAction, &QAction::triggered, this, &CtrlAgent::showCtrlInfoWidget);
    }

    void notificationToTray(QString message) {
        if(conf->getSettingsBuffer()->showNotifications)
            if(lastMessage != message) {
                lastMessage = message;
                this->showMessage("Preset Switch", message);
        }
    }

    QAction *showInfoWidgetAction;

signals:
    void showCtrlGui();
    void closeCtrlGui();
    void showCtrlMiniGui();
    void showCtrlInfoWidget(bool checked);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason) {
        if(reason == ActivationReason::DoubleClick){
            emit showCtrlGui();
        } else if(reason == ActivationReason::Trigger){
            emit showCtrlMiniGui();
        }
    }

private:
    QString lastMessage;

    CtrlSettings *conf;
};

#endif // CTRLAGENT_H
