#ifndef CTRLAGENT_H
#define CTRLAGENT_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "CtrlSettings.h"

class CtrlAgent : public QSystemTrayIcon
{
    Q_OBJECT
public:
    CtrlAgent(CtrlSettings *conf);
    ~CtrlAgent();

    void notificationToTray(QString message);

signals:
    void showCtrlGui();
    void closeCtrlGui();
    void showCtrlMiniGui();

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QString lastMessage;

    QMenu *trayMenu;
    CtrlSettings *conf;
};

#endif // CTRLAGENT_H
