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
    CtrlGuiX(QSharedMemory *bufferToService, CtrlSettings *conf)
        : CtrlGui(bufferToService, conf)
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
    CtrlAgent(QSharedMemory *bufferToService, CtrlSettings *conf);
    ~CtrlAgent();

private slots:
    void openCtrl();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void closeAgent();

private:
    QMenu *trayMenu;
    QSharedMemory *bufferToService;
    CtrlSettings *conf;
    CtrlGuiX *gui = nullptr;
};

#endif // CTRLAGENT_H
