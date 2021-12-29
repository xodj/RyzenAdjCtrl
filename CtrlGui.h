#ifndef CTRLGUI_H
#define CTRLGUI_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QTranslator>
#include "CtrlSettings.h"
#include "CtrlAgent.h"
#include "CtrlBus.h"
#include "CtrlFrame.h"
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class CtrlGui;
class CtrlGuiAPUForm;
class CtrlGuiSettings;
class CtrlInfoWidget;
}
QT_END_NAMESPACE

class CtrlGui : public QMainWindow
{
    Q_OBJECT

public:
    CtrlGui(CtrlBus *bus);
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
    void sendPreset(int presetId = -1, bool save = false, bool apply = false);

    void smuCheckBoxClicked();

    void saveSettings();
    void readSettings();
    void cancelSettings();

#ifdef BUILD_SERVICE
    void startService();
    void installService();
#endif

    void infoPushButtonClicked();
    void sendRyzenAdjInfo(QString value = "0");
    bool infoWidgetHasBeenShowed = false;

    void settingsPushButtonClicked();
    void presetPushButtonClicked();
    void presetPlusPushButtonClicked();
    void presetDeletePushButtonClicked();
    void presetNameEditChanged(QString name);
    void settingsAutomaticPresetSwitchClicked();

    void openAdvancedInfoUrl();

    void recieveMessageToGui(messageToGuiStr messageToGui);

    void useAgent(bool use);
    void exitFromAgent();
    void presetChangeFromAgent(int idx);

    void showWindow(){ this->showNormal(); }

    bool infoMessageShowed = false;

    Ui::CtrlGui *ui;
    Ui::CtrlGuiSettings *ui_settings;
    Ui::CtrlInfoWidget *ui_infoWidget;
    QFrame *settingFrame;
    CtrlFrame *infoFrame;
    QTranslator *qtLanguageTranslator;
    CtrlBus *bus;
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

    int currentPresetId = -1;

protected:
    virtual void closeEvent(QCloseEvent *event);

};
#endif // CTRLGUI_H
