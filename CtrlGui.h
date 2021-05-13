#ifndef CTRLGUI_H
#define CTRLGUI_H

#include <QMainWindow>
#include <QTranslator>
#include <QSharedMemory>
#include <QFrame>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include "CtrlSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CtrlGui;
               class CtrlGuiAPUForm;
               class CtrlGuiSettings;
               class CtrlInfoWidget; }
QT_END_NAMESPACE

class CtrlGui : public QMainWindow
{
    Q_OBJECT

public:
    CtrlGui(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf);
    ~CtrlGui();
signals:
    void messageToAgent(QString message);

private:
    void setupUi();
    void setupConnections();
    void loadPresets();
    void loadStyleSheet();

    void languageChange();

    void savePreset();
    void applyPreset();
    void cancelPreset();
    void sendPreset(int presetId, bool save);

    void smuCheckBoxClicked();
    void sendArgsToService(QByteArray arguments);

    void saveSettings();
    void readSettings();
    void cancelSettings();

    void startService();

    void infoPushButtonClicked();
    void sendRyzenAdjInfo(int value);

    void settingsPushButtonClicked();
    void presetPushButtonClicked();
    void presetPlusPushButtonClicked();
    void presetDeletePushButtonClicked();
    void presetNameEditChanged(QString name);
    void settingsAutomaticPresetSwitchClicked();

    void recieveArgs();
    void decodeArgs(QByteArray args);

    bool infoMessageShowed = false;

    Ui::CtrlGui *ui;
    Ui::CtrlGuiSettings *ui_settings;
    Ui::CtrlInfoWidget *ui_infoWidget;
    QFrame *settingFrame;
    QTranslator *qtLanguageTranslator;
    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;
    CtrlSettings *conf;

    QList<Ui::CtrlGuiAPUForm*> *presetFormList;
    QVBoxLayout *verticalLayout;
    QList<QWidget*> *tabWidgetsList;
    QList<QPushButton*> *tabButtonList;
    QPushButton *tabPlusButton;
    QSpacerItem *spacer;

    QString ryzenFamily;
    QString biosVersion;
    QString pmTableVersion;
    QString ryzenAdjVersion;
};
#endif // CTRLGUI_H
