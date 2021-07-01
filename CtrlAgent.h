#ifndef CTRLAGENT_H
#define CTRLAGENT_H

#include <QtWidgets/QSystemTrayIcon>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFrame>
#include <QAction>
#include "CtrlSettings.h"
#include "ui_CtrlPopupWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CtrlPopupWidget; }
QT_END_NAMESPACE

class CtrlAgent : public QSystemTrayIcon
{
    Q_OBJECT
public:
    CtrlAgent(CtrlSettings *conf)
        : QSystemTrayIcon(new QSystemTrayIcon),
          conf(conf),
          ui_popupwidget(new Ui::CtrlPopupWidget)
    {
        QIcon icon(":/main/amd_icon.ico");
        this->setIcon(icon);
        this->setToolTip("RyzenAdjCtrl");
        this->show();

        popupMenu = new QFrame(nullptr, Qt::WindowType::Popup);
        popupMenu->setFrameStyle(QFrame::Shape::NoFrame);
        popupMenu->setAccessibleName("CtrlSettings");
        popupMenu->resize(1,1);
        ui_popupwidget->setupUi(popupMenu);
        if (popupMenu->geometry().width()<10) {
            popupMenu->show();
            popupMenu->hide();
        }

        presetButtonsList = new QList<QPushButton*>;
        QPushButton *button;
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        const QList<presetStr*> *presetsList = conf->getPresetsList();
        for(qsizetype i = 0; i < presetsList->count(); i++){
            const presetStr *presetBuffer = presetsList->at(i);

            button = new QPushButton(presetBuffer->presetName);
            button->setCheckable(true);
            button->setProperty("idx", presetBuffer->presetId);
            button->setMinimumSize(QSize(105, 23));
            button->setFont(font);

            ui_popupwidget->scrollAreaWidgetContents->layout()->addWidget(button);

            presetButtonsList->append(button);
        }

        spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
        ui_popupwidget->scrollAreaWidgetContents->layout()->addItem(spacer);

        QFile configQFile;
        configQFile.setFileName(":/theme/mainwindow.qss");
        configQFile.open(QIODevice::ReadOnly);
        QString strStyleSheet = configQFile.readAll();
        popupMenu->setStyleSheet(strStyleSheet);
        configQFile.close();

        connection();
    }

    ~CtrlAgent() {
        disconnect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        disconnect(ui_popupwidget->openPushButton, SIGNAL(clicked()),
                   this, SIGNAL(showCtrlGui()));
        disconnect(ui_popupwidget->showInfoPushButton, &QPushButton::clicked, this, &CtrlAgent::showCtrlInfoWidget);
        disconnect(ui_popupwidget->quitPushButton, SIGNAL(clicked()),
                   this, SIGNAL(closeCtrlGui()));
        for(qsizetype i = 0; i < presetButtonsList->count(); i++){
            QPushButton *button = presetButtonsList->at(i);
            disconnect(button, &QPushButton::clicked,
                    this, &CtrlAgent::presetButtonClicked);
        }
    }

    void connection(){
        connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
        connect(ui_popupwidget->openPushButton, SIGNAL(clicked()),
                this, SIGNAL(showCtrlGui()));
        connect(ui_popupwidget->showInfoPushButton, &QPushButton::clicked,
                this, &CtrlAgent::showCtrlInfoWidget);
        connect(ui_popupwidget->quitPushButton, SIGNAL(clicked()),
                this, SIGNAL(closeCtrlGui()));
        for(qsizetype i = 0; i < presetButtonsList->count(); i++){
            QPushButton *button = presetButtonsList->at(i);
            connect(button, &QPushButton::clicked,
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

    void showPopupFrame() {
        QRect rect = popupMenu->geometry();
        const QRect iconGeometry = this->geometry();
        int width = rect.width();
        int height = rect.height();
        rect.setX(iconGeometry.left() - (width / 2) + (iconGeometry.width() / 2));
        rect.setY(iconGeometry.top() - height - 10);
        rect.setWidth(width);
        rect.setHeight(height);
        popupMenu->setGeometry(rect);

        if (popupMenu->isHidden())
            popupMenu->show();
    }

    void infoPushButtonClicked(bool activate){
        ui_popupwidget->showInfoPushButton->setChecked(activate);
    }

    void setCurrentPresetId(int idx){
        for(qsizetype i = 0; i < presetButtonsList->count(); i++){
            QPushButton *button = presetButtonsList->at(i);
            button->setChecked(false);
            if(button->property("idx").toInt() == idx)
                button->setChecked(true);
        }
    }

    void presetButtonClicked(){
        int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();
        setCurrentPresetId(idx);
        emit changePreset(idx);
    }

    void addPresetButton(int idx){
        ui_popupwidget->scrollAreaWidgetContents->layout()->removeItem(spacer);
        QPushButton *button;
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        const QList<presetStr*> *presetsList = conf->getPresetsList();
        for(qsizetype i = 0; i < presetsList->count(); i++){
            const presetStr *presetBuffer = presetsList->at(i);
            if(presetBuffer->presetId == idx){
                button = new QPushButton(presetBuffer->presetName);
                button->setCheckable(true);
                button->setProperty("idx", presetBuffer->presetId);
                button->setMinimumSize(QSize(105, 23));
                button->setFont(font);
                ui_popupwidget->scrollAreaWidgetContents->layout()->addWidget(button);
                presetButtonsList->append(button);
                connect(button, &QPushButton::clicked,
                        this, &CtrlAgent::presetButtonClicked);
                break;
            }
        }
        ui_popupwidget->scrollAreaWidgetContents->layout()->addItem(spacer);
    }

    void delPresetButton(int idx){
        ui_popupwidget->scrollAreaWidgetContents->layout()->removeItem(spacer);
        for(qsizetype i = 0; i < presetButtonsList->count(); i++){
            QPushButton *button = presetButtonsList->at(i);
            if(button->property("idx").toInt() == idx){
                disconnect(button, &QPushButton::clicked,
                        this, &CtrlAgent::presetButtonClicked);
                ui_popupwidget->scrollAreaWidgetContents->layout()->removeWidget(button);
                presetButtonsList->removeOne(button);
                delete button;
            }
        }
        ui_popupwidget->scrollAreaWidgetContents->layout()->addItem(spacer);
    }

signals:
    void showCtrlGui();
    void closeCtrlGui();
    void showCtrlInfoWidget(bool checked);
    void changePreset(int idx);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason) {
        if(reason == ActivationReason::DoubleClick){
            emit showCtrlGui();
        } else if(reason == ActivationReason::Context){
            showPopupFrame();
        }
    }

private:
    QString lastMessage;
    QFrame *popupMenu;
    Ui::CtrlPopupWidget *ui_popupwidget;
    QList<QPushButton*> *presetButtonsList;
    QSpacerItem *spacer;

    CtrlSettings *conf;
};

#endif // CTRLAGENT_H
