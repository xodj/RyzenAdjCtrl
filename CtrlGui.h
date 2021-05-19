#ifndef CTRLGUI_H
#define CTRLGUI_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QTranslator>
#include <QSharedMemory>
#include "CtrlSettings.h"
#include "CtrlAgent.h"

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

private:
    void setupUi();
    void setupConnections();
    void loadPresets();
    void loadStyleSheet();

    void languageChange(QString langid);

    void savePreset();
    void saveApplyPreset();
    void applyPreset();
    void cancelPreset();
    void sendPreset(int presetId, bool save, bool apply = true);

    void smuCheckBoxClicked();
    void sendArgsToService(QByteArray arguments);

    void saveSettings();
    void readSettings();
    void cancelSettings();

    void startService();
    void installService();

    void infoPushButtonClicked();
    void sendRyzenAdjInfo(QString value = "0");

    void settingsPushButtonClicked();
    void presetPushButtonClicked();
    void presetPlusPushButtonClicked();
    void presetDeletePushButtonClicked();
    void presetNameEditChanged(QString name);
    void settingsAutomaticPresetSwitchClicked();

    void openAdvancedInfoUrl();

    void recieveArgs();
    void decodeArgs(QByteArray args);

    void useAgent(bool use);
    void exitFromAgent();

    bool infoMessageShowed = false;

    Ui::CtrlGui *ui;
    Ui::CtrlGuiSettings *ui_settings;
    Ui::CtrlInfoWidget *ui_infoWidget;
    QFrame *settingFrame;
    QTranslator *qtLanguageTranslator;
    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;
    CtrlSettings *conf;
    CtrlAgent *ui_agent = nullptr;

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

protected:
    virtual void closeEvent(QCloseEvent *event);
};
#endif // CTRLGUI_H
