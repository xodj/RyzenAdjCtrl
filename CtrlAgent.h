#ifndef CTRLAGENT_H
#define CTRLAGENT_H

#include <QtWidgets/QSystemTrayIcon>
#include <QtWidgets/QMenu>
#include <QAction>
#include "CtrlSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CtrlPopupWidget; }
QT_END_NAMESPACE

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
        this->setToolTip("RyzenCtrl");

        checkIcon.addFile(QString::fromUtf8(":/main/unchecked.png"),
                          QSize(), QIcon::Selected, QIcon::Off);
        checkIcon.addFile(QString::fromUtf8(":/main/checked.png"),
                          QSize(), QIcon::Selected, QIcon::On);

        openAction = new QAction(icon, "Open RyzenCtrl", this);
        openAction->setObjectName(QString::fromUtf8("openAction"));
        showAction = new QAction(checkIcon, "Show Info Widget", this);
        showAction->setObjectName(QString::fromUtf8("showAction"));
        showAction->setCheckable(true);
        quitAction = new QAction(QIcon(":/main/application-exit.png"), "Quit", this);
        quitAction->setObjectName(QString::fromUtf8("quitAction"));
        trayMenu = new QMenu;
        trayMenu->setObjectName(QString::fromUtf8("trayMenu"));
        presetsMenu = new QMenu("Presets switch", trayMenu);
        presetsMenu->setObjectName(QString::fromUtf8("presetsMenu"));
        presetsMenu->setIcon(QIcon(":/main/settings.png"));

        this->setContextMenu(trayMenu);

        trayMenu->addAction(openAction);
        trayMenu->addAction(presetsMenu->menuAction());
        trayMenu->addAction(showAction);
        trayMenu->addAction(quitAction);

        presetActionList = new QList<QAction*>;
        QAction *presetAction;
        const QList<presetStr*> *presetsList = conf->getPresetsList();
        for(qsizetype i = 0; i < presetsList->count(); i++){
            const presetStr *presetBuffer = presetsList->at(i);

            presetAction = new QAction(checkIcon, presetBuffer->presetName, this);
            presetAction->setCheckable(true);
            presetAction->setProperty("idx", presetBuffer->presetId);

            presetsMenu->addAction(presetAction);

            presetActionList->append(presetAction);
        }

        connection();
        this->show();
    }

    ~CtrlAgent() {
        this->hide();

        disconnect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        disconnect(openAction, SIGNAL(triggered()),
                this, SIGNAL(showCtrlGui()));
        disconnect(showAction, &QAction::triggered,
                this, &CtrlAgent::showCtrlInfoWidget);
        disconnect(quitAction, SIGNAL(triggered()), this, SIGNAL(closeCtrlGui()));
        for(qsizetype i = 0; i < presetActionList->count(); i++){
            QAction *action = presetActionList->at(i);
            disconnect(action, &QAction::triggered,
                    this, &CtrlAgent::presetButtonClicked);
        }
    }

    void connection(){
        connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        connect(openAction, SIGNAL(triggered()),
                this, SIGNAL(showCtrlGui()));
        connect(showAction, &QAction::triggered,
                this, &CtrlAgent::showCtrlInfoWidget);
        connect(quitAction, SIGNAL(triggered()), this, SIGNAL(closeCtrlGui()));
        for(qsizetype i = 0; i < presetActionList->count(); i++){
            QAction *action = presetActionList->at(i);
            connect(action, &QAction::triggered,
                    this, &CtrlAgent::presetButtonClicked);
        }
    }

    void notificationToTray(QString message) {
        if(conf->getSettingsBuffer()->showNotifications)
            if(lastMessage != message) {
                lastMessage = message;
                this->showMessage("Preset Switch", message);
        }
    }

    void infoPushButtonClicked(bool activate){
        showAction->setChecked(activate);
    }

    void setCurrentPresetId(int idx){
        for(qsizetype i = 0; i < presetActionList->count(); i++){
            QAction *action = presetActionList->at(i);
            action->setChecked(false);
            if(action->property("idx").toInt() == idx)
                action->setChecked(true);
        }
    }

    void presetButtonClicked(){
        int idx = reinterpret_cast<QAction *>(sender())->property("idx").toInt();
        setCurrentPresetId(idx);
        emit changePreset(idx);
    }

    void addPresetButton(int idx){
        const QList<presetStr*> *presetsList = conf->getPresetsList();
        for(qsizetype i = 0; i < presetsList->count(); i++){
            const presetStr *presetBuffer = presetsList->at(i);
            if(presetBuffer->presetId == idx){
                QAction *presetAction = new QAction(checkIcon, presetBuffer->presetName, this);
                presetAction->setCheckable(true);
                presetAction->setProperty("idx", presetBuffer->presetId);

                presetsMenu->addAction(presetAction);

                presetActionList->append(presetAction);
                break;
            }
        }
    }

    void delPresetButton(int idx){
        for(qsizetype i = 0; i < presetActionList->count(); i++){
            QAction *presetAction = presetActionList->at(i);
            if(presetAction->property("idx").toInt() == idx){
                presetsMenu->removeAction(presetAction);
                presetActionList->removeOne(presetAction);
                delete presetAction;
                break;
            }
        }
    }

signals:
    void showCtrlGui();
    void closeCtrlGui();
    void showCtrlInfoWidget(bool checked);
    void changePreset(int idx);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason) {
        if(reason == ActivationReason::DoubleClick)
            emit showCtrlGui();
    }

private:
    QString lastMessage;

    QIcon checkIcon;
    QAction *openAction;
    QAction *showAction;
    QAction *quitAction;
    QList<QAction*> *presetActionList;
    QMenu *trayMenu;
    QMenu *presetsMenu;

    CtrlSettings *conf;
};

#endif // CTRLAGENT_H
