#ifndef CTRLGUI_H
#define CTRLGUI_H

#include <QMainWindow>
#include <QTranslator>
#include <QSharedMemory>
#include <QFrame>
#include "CtrlSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CtrlGui;
               class CtrlGuiAPUForm;
               class CtrlGuiSettings; }
QT_END_NAMESPACE

class CtrlGui : public QMainWindow
{
    Q_OBJECT

public:
    CtrlGui(QSharedMemory *bufferToService, CtrlSettings *conf);
    ~CtrlGui();

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
    void sendArgs(QByteArray arguments);

    void saveSettings();
    void readSettings();
    void cancelSettings();

    void installService();
    void removeService();
    void startService();
    void stopService();

    void settingsPushButtonClicked();
    void presetPushButtonClicked();
    void settingsAutomaticPresetSwitchClicked();

    Ui::CtrlGui *ui;
    Ui::CtrlGuiAPUForm *apuForm[4];
    Ui::CtrlGuiSettings *ui_settings;
    QFrame *settingFrame;
    QTranslator *qtLanguageTranslator;
    QSharedMemory *bufferToService;
    CtrlSettings *conf;
};
#endif // CTRLGUI_H
