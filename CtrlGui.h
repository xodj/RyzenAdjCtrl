#ifndef CTRLGUI_H
#define CTRLGUI_H

#include <QMainWindow>
#include <QTranslator>
#include <QSharedMemory>
#include <QFrame>
#include "CtrlSettings.h"
#include "CtrlInfoWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CtrlGui;
               class CtrlGuiAPUForm;
               class CtrlGuiSettings; }
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

    void presetVariableChanged();
    void smuCheckBoxClicked();
    void sendArgsToService(QByteArray arguments);

    void saveSettings();
    void readSettings();
    void cancelSettings();

    void startService();

    void infoPushButtonClicked();
    void settingsPushButtonClicked();
    void presetPushButtonClicked();
    void settingsAutomaticPresetSwitchClicked();

    void recieveArgs();
    void decodeArgs(QByteArray args);

    bool infoMessageShowed = false;

    Ui::CtrlGui *ui;
    Ui::CtrlGuiAPUForm *apuForm[4];
    Ui::CtrlGuiSettings *ui_settings;
    QFrame *settingFrame;
    QTranslator *qtLanguageTranslator;
    QSharedMemory *bufferToService;
    QSharedMemory *bufferToGui;
    CtrlSettings *conf;
    CtrlInfoWidget *ciw;
};
#endif // CTRLGUI_H
