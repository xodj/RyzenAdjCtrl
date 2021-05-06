#ifndef CTRLAGENT_H
#define CTRLAGENT_H

#include <QSystemTrayIcon>
#include <QApplication>
#include <QSharedMemory>
#include <QMenu>
#include <QAction>
#include "CtrlSettings.h"
#include "CtrlGui.h"

class CtrlGuiX : public CtrlGui
{
    Q_OBJECT
public:
    CtrlGuiX(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf)
        : CtrlGui(bufferToService, bufferToGui, conf)
    {}

protected:
    virtual void closeEvent(QCloseEvent *event){
        QEvent *ev = (QEvent*)event;
        ev->ignore();
        this->hide();
    }

};

class CtrlAgent : public QSystemTrayIcon
{
    Q_OBJECT
public:
    CtrlAgent(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf);
    ~CtrlAgent();

private slots:
    void openCtrl();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void closeAgent();

    void notificationToTray(QString message);

private:
    QString lastMessage;

    QMenu *trayMenu;
    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;
    CtrlSettings *conf;
    CtrlGuiX *gui = nullptr;
};

#endif // CTRLAGENT_H
