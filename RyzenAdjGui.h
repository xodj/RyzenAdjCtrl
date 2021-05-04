#ifndef RYZENADJGUI_H
#define RYZENADJGUI_H

#include <QMainWindow>
#include <QTranslator>
#include <QSharedMemory>
#include "RyzenAdjCfg.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RyzenAdjGui;
               class RyzenAdjGuiAPUForm;
               class RyzenAdjGuiSettings; }
QT_END_NAMESPACE

class RyzenAdjGui : public QMainWindow
{
    Q_OBJECT

public:
    RyzenAdjGui(QSharedMemory *bufferToService, RyzenAdjCfg *conf);
    ~RyzenAdjGui();

private:
    void setupUi();
    void setupConnections();
    void loadPresets();
    void loadStyleSheet();

    void languageChange();

    void savePreset();
    void tryPreset();
    void presetVariableChanged();
    void smuCheckBoxClicked();
    void sendArgs(QByteArray arguments);

    void saveSettings();
    void readSettings();

    void installService();
    void removeService();
    void startService();
    void stopService();

    void settingsPushButtonClicked();
    void presetPushButtonClicked();

    Ui::RyzenAdjGui *ui;
    Ui::RyzenAdjGuiAPUForm *apuForm[4];
    Ui::RyzenAdjGuiSettings *ui_settings;
    QTranslator *qtLanguageTranslator;
    QSharedMemory *bufferToService;
    RyzenAdjCfg *conf;
};
#endif // RYZENADJGUI_H
