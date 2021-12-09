#include "CtrlGui.h"
#include "ui_CtrlMainWindow.h"
#include "ui_CtrlAPUForm.h"
#include "ui_CtrlSettingsForm.h"
#include "ui_CtrlInfoWidget.h"
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QProcess>
#include <QtWidgets/QScroller>
#include <QDesktopServices>
#include <QUrl>
#include "CtrlConfig.h"
#include <QThread>

CtrlGui::CtrlGui(CtrlBus *bus, CtrlSettings *conf)
    : ui(new Ui::CtrlGui),
      ui_settings(new Ui::CtrlGuiSettings),
      ui_infoWidget(new Ui::CtrlInfoWidget),
      bus(bus),
      conf(conf)
{
    qtLanguageTranslator = new QTranslator;

#ifdef BUILD_SERVICE
    for(;!bus->isServiseRuning();){
        qDebug() << "Ctrl Gui - RyzenCtrl Service is not runing!";
        QMessageBox *dialog =
                new QMessageBox(QMessageBox::Warning,
                                "RyzenCtrl Service is not runing!",
                                "RyzenCtrl Service is not runing!"
                                "\nNeed to start service.");
        dialog->addButton(QString::fromUtf8("&Start"),
                                QMessageBox::AcceptRole);
        dialog->addButton(QString::fromUtf8("&Retry"),
                                QMessageBox::ActionRole);
        dialog->addButton(QString::fromUtf8("&Cancel"),
                                QMessageBox::RejectRole);
        dialog->setModal(true);
        int role = dialog->exec();
        if (role == 0){
            startService();
            for(int i = 0;i < 100;i++){
                if(bus->isServiseRuning()) break;
                QThread::msleep(100);
            }
        }
        if (role == 1)
            QThread::msleep(333);
        if (role == 2)
            exit(0);
        delete dialog;
    }
#endif

    setupUi();
    loadPresets();
    readSettings();
    setupConnections();
    loadStyleSheet();

    if(!conf->getSettingsBuffer()->useAgent)
        this->show();

    bus->setGUIRuning();
}

void CtrlGui::setupUi(){
    ui->setupUi(this);
    ui->comboBox->addItems(QStringList() << "en_US"<< "ru_RU");
    ui->comboBox->setHidden(true);

    verticalLayout = new QVBoxLayout;
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    ui->tabWidget->setLayout(verticalLayout);

    QFont font;
    font.setPointSize(8);
    font.setBold(true);
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    tabWidgetsList = new QList<QWidget*>;
    QWidget *widget;

    presetFormList = new QList<Ui::CtrlGuiAPUForm*>;
    Ui::CtrlGuiAPUForm *presetForm;

    tabButtonList = new QList<QPushButton*>;
    QPushButton *button;

    for(qsizetype i = 0;i < conf->getPresetsCount();i++){
        const presetStr *preset = conf->getPresetsList()->at(i);
        int idx = preset->presetId;

        widget = new QWidget;
        widget->setProperty("idx",idx);
        tabWidgetsList->append(widget);
        verticalLayout->addWidget(widget);
        presetForm = new Ui::CtrlGuiAPUForm;
        presetForm->setupUi(widget);
        widget->setHidden(true);
        if(i == 0)
            widget->setHidden(false);

        presetForm->deletePushButton->setProperty("idx",idx);
        presetForm->presetNameEdit->setText(preset->presetName);
        presetForm->presetNameEdit->setProperty("idx",idx);

        presetForm->saveApplyPushButton->setProperty("idx",idx);
        presetForm->saveOnlyPushButton->setProperty("idx",idx);
        presetForm->applyPushButton->setProperty("idx",idx);
        presetForm->cancelPushButton->setProperty("idx",idx);

        presetForm->fanComboBox->setProperty("idx",idx);

        presetForm->smuMaxPerformanceCheckBox->setProperty("idx",idx);
        presetForm->smuPowerSavingCheckBox->setProperty("idx",idx);

        presetForm->smuMaxPerformanceCheckBox->setProperty("idy",0);
        presetForm->smuPowerSavingCheckBox->setProperty("idy",1);

        presetFormList->append(presetForm);

        button = new QPushButton(preset->presetName);

        button->setObjectName(QString::fromUtf8("tabPushButton") + QString::number(i));
        button->setMinimumSize(QSize(105, 23));
        button->setFont(font);
        button->setStyleSheet(QString::fromUtf8(""));
        button->setCheckable(true);
        button->setProperty("idx", idx);
        button->setSizePolicy(sizePolicy);
        if(i == 0)
            button->setChecked(true);

        ui->scrollAreaWidgetContents->layout()->addWidget(button);

        tabButtonList->append(button);
    }

    tabPlusButton = new QPushButton("+");
    tabPlusButton->setObjectName(QString::fromUtf8("tabPlusPushButton"));
    tabPlusButton->setMinimumSize(QSize(23, 23));
    tabPlusButton->setMaximumSize(QSize(23, 23));
    tabPlusButton->setFont(font);
    tabPlusButton->setStyleSheet(QString::fromUtf8(""));
    tabPlusButton->setSizePolicy(sizePolicy);
    ui->scrollAreaWidgetContents->layout()->addWidget(tabPlusButton);

    spacer = new QSpacerItem(23, 23, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->scrollAreaWidgetContents->layout()->addItem(spacer);

    QScroller::grabGesture(ui->scrollArea, QScroller::LeftMouseButtonGesture);

    infoFrame = new CtrlFrame(this, Qt::Window);
    ui_infoWidget->setupUi(infoFrame);
    infoFrame->setWindowFlag(Qt::WindowMinMaxButtonsHint, false);
    infoFrame->setAccessibleName("CtrlInfo");
    infoFrame->resize(1,1);
    infoFrame->setWindowIcon(QIcon(":/main/amd_icon.ico"));
    if (infoFrame->geometry().width()<10) {
        infoFrame->show();
        infoFrame->hide();
    }

    settingFrame = new QFrame(this, Qt::WindowType::Popup);
    settingFrame->setFrameStyle(QFrame::Panel);
    settingFrame->setAccessibleName("CtrlSettings");
    settingFrame->resize(1,1);
    ui_settings->setupUi(settingFrame);
    ui->tabWidget->setHidden(false);
    if (settingFrame->geometry().width()<10) {
        settingFrame->show();
        settingFrame->hide();
    }

#ifndef WIN32
    ui_settings->epmAutoPresetSwitchGroupBox->setHidden(true);
    ui_settings->showArmourCheckBox->setHidden(true);
#endif
#ifndef BUILD_SERVICE
    ui_settings->installPushButton->setHidden(true);
#endif

    resize(600, 450);
    setWindowIcon(QIcon(":/main/amd_icon.ico"));
}

void CtrlGui::setupConnections(){
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &CtrlGui::languageChange);
    connect(ui->infoPushButton, &QPushButton::clicked, this, &CtrlGui::infoPushButtonClicked);
    connect(infoFrame, &CtrlFrame::closeClicked, this, &CtrlGui::infoPushButtonClicked);
    connect(ui->settingsPushButton, &QPushButton::clicked, this, &CtrlGui::settingsPushButtonClicked);

    connect(tabPlusButton, &QPushButton::clicked, this, &CtrlGui::presetPlusPushButtonClicked);

    for(qsizetype i = 0;i < conf->getPresetsCount();i++){
        connect(tabButtonList->at(i), &QPushButton::clicked, this, &CtrlGui::presetPushButtonClicked);

        connect(presetFormList->at(i)->deletePushButton, &QPushButton::clicked, this, &CtrlGui::presetDeletePushButtonClicked);
        connect(presetFormList->at(i)->presetNameEdit, &QLineEdit::textChanged, this, &CtrlGui::presetNameEditChanged);

        connect(presetFormList->at(i)->saveApplyPushButton, &QPushButton::clicked, this, &CtrlGui::saveApplyPreset);
        connect(presetFormList->at(i)->saveOnlyPushButton, &QPushButton::clicked, this, &CtrlGui::savePreset);
        connect(presetFormList->at(i)->applyPushButton, &QPushButton::clicked, this, &CtrlGui::applyPreset);
        connect(presetFormList->at(i)->cancelPushButton, &QPushButton::clicked, this, &CtrlGui::cancelPreset);

        connect(presetFormList->at(i)->smuMaxPerformanceCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);
        connect(presetFormList->at(i)->smuPowerSavingCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);
    }

    connect(ui_settings->savePushButton, &QPushButton::clicked, this, &CtrlGui::saveSettings);
    connect(ui_settings->cancelPushButton, &QPushButton::clicked, this, &CtrlGui::cancelSettings);

    connect(ui_settings->epmAutoPresetSwitchGroupBox, &QGroupBox::clicked, this, &CtrlGui::settingsAutomaticPresetSwitchClicked);
    connect(ui_settings->acAutoPresetSwitchGroupBox, &QGroupBox::clicked, this, &CtrlGui::settingsAutomaticPresetSwitchClicked);
    connect(ui_infoWidget->spinBox, &QSpinBox::textChanged, this, &CtrlGui::sendRyzenAdjInfo);

#ifdef BUILD_SERVICE
    connect(ui_settings->installPushButton, &QPushButton::clicked, this, &CtrlGui::installService);
#endif
    connect(ui_settings->openAdvancedInfoUrlPushButton, &QPushButton::clicked, this, &CtrlGui::openAdvancedInfoUrl);

    connect(bus, &CtrlBus::messageFromServiceRecieved, this, &CtrlGui::recieveMessageToGui);
    connect(bus, &CtrlBus::messageFromAnotherGui, this, &CtrlGui::showWindow);
}

void CtrlGui::loadPresets(){
    for(qsizetype i = 0;i < conf->getPresetsCount();i++){
        const presetStr *presetBuffer = conf->getPresetsList()->at(i);
        int idx = presetBuffer->presetId;
        Ui::CtrlGuiAPUForm *presetForm = nullptr;

        for(qsizetype x = 0;x < presetFormList->count();x++)
            if(presetFormList->at(x)->applyPushButton->property("idx") == idx){
                presetForm = presetFormList->at(x);
                break;
            }

        presetForm->fanComboBox->setCurrentIndex(presetBuffer->fanPresetId);

        presetForm->tempLimitSpinBox->setValue(presetBuffer->tempLimitValue);
        presetForm->tempLimitCheckBox->setChecked(presetBuffer->tempLimitChecked);
        presetForm->apuSkinSpinBox->setValue(presetBuffer->apuSkinValue);
        presetForm->apuSkinCheckBox->setChecked(presetBuffer->apuSkinChecked);
        presetForm->stampLimitSpinBox->setValue(presetBuffer->stampLimitValue);
        presetForm->stampLimitCheckBox->setChecked(presetBuffer->stampLimitChecked);
        presetForm->fastLimitSpinBox->setValue(presetBuffer->fastLimitValue);
        presetForm->fastLimitCheckBox->setChecked(presetBuffer->fastLimitChecked);
        presetForm->fastTimeSpinBox->setValue(presetBuffer->fastTimeValue);
        presetForm->fastTimeCheckBox->setChecked(presetBuffer->fastTimeChecked);
        presetForm->slowLimitSpinBox->setValue(presetBuffer->slowLimitValue);
        presetForm->slowLimitCheckBox->setChecked(presetBuffer->slowLimitChecked);
        presetForm->slowTimeSpinBox->setValue(presetBuffer->slowTimeValue);
        presetForm->slowTimeCheckBox->setChecked(presetBuffer->slowTimeChecked);

        presetForm->vrmCurrentSpinBox->setValue(presetBuffer->vrmCurrentValue);
        presetForm->vrmCurrentCheckBox->setChecked(presetBuffer->vrmCurrentChecked);
        presetForm->vrmMaxSpinBox->setValue(presetBuffer->vrmMaxValue);
        presetForm->vrmMaxCheckBox->setChecked(presetBuffer->vrmMaxChecked);

        presetForm->minFclkSpinBox->setValue(presetBuffer->minFclkValue);
        presetForm->minFclkCheckBox->setChecked(presetBuffer->minFclkChecked);
        presetForm->maxFclkSpinBox->setValue(presetBuffer->maxFclkValue);
        presetForm->maxFclkCheckBox->setChecked(presetBuffer->maxFclkChecked);

        presetForm->minGfxclkSpinBox->setValue(presetBuffer->minGfxclkValue);
        presetForm->minGfxclkCheckBox->setChecked(presetBuffer->minGfxclkChecked);
        presetForm->maxGfxclkSpinBox->setValue(presetBuffer->maxGfxclkValue);
        presetForm->maxGfxclkCheckBox->setChecked(presetBuffer->maxGfxclkChecked);
        presetForm->minSocclkSpinBox->setValue(presetBuffer->minSocclkValue);
        presetForm->minSocclkCheckBox->setChecked(presetBuffer->minSocclkChecked);
        presetForm->maxSocclkSpinBox->setValue(presetBuffer->maxSocclkValue);
        presetForm->maxSocclkCheckBox->setChecked(presetBuffer->maxSocclkChecked);
        presetForm->minVcnSpinBox->setValue(presetBuffer->minVcnValue);
        presetForm->minVcnCheckBox->setChecked(presetBuffer->minVcnChecked);
        presetForm->maxVcnSpinBox->setValue(presetBuffer->maxVcnValue);
        presetForm->maxVcnCheckBox->setChecked(presetBuffer->maxVcnChecked);

        presetForm->smuMaxPerformanceCheckBox->setChecked(presetBuffer->smuMaxPerfomance);
        presetForm->smuPowerSavingCheckBox->setChecked(presetBuffer->smuPowerSaving);

        //NEW VARS
        presetForm->vrmSocCurrentSpinBox->setValue(presetBuffer->vrmSocCurrent);
        presetForm->vrmSocCurrentCheckBox->setChecked(presetBuffer->vrmSocCurrentChecked);
        presetForm->vrmSocMaxSpinBox->setValue(presetBuffer->vrmSocMax);
        presetForm->vrmSocMaxCheckBox->setChecked(presetBuffer->vrmSocMaxChecked);

        presetForm->psi0CurrentSpinBox->setValue(presetBuffer->psi0Current);
        presetForm->psi0CurrentCheckBox->setChecked(presetBuffer->psi0CurrentChecked);
        presetForm->psi0SocCurrentSpinBox->setValue(presetBuffer->psi0SocCurrent);
        presetForm->psi0SocCurrentCheckBox->setChecked(presetBuffer->psi0SocCurrentChecked);

        presetForm->maxLclkSpinBox->setValue(presetBuffer->maxLclk);
        presetForm->maxLclkCheckBox->setChecked(presetBuffer->maxLclkChecked);
        presetForm->minLclkSpinBox->setValue(presetBuffer->minLclk);
        presetForm->minLclkCheckBox->setChecked(presetBuffer->minLclkChecked);

        presetForm->prochotDeassertionRampSpinBox->setValue(presetBuffer->prochotDeassertionRamp);
        presetForm->prochotDeassertionRampCheckBox->setChecked(presetBuffer->prochotDeassertionRampChecked);

        presetForm->dgpuSkinTempLimitSpinBox->setValue(presetBuffer->dgpuSkinTempLimit);
        presetForm->dgpuSkinTempLimitCheckBox->setChecked(presetBuffer->dgpuSkinTempLimitChecked);
        presetForm->apuSlowLimitSpinBox->setValue(presetBuffer->apuSlowLimit);
        presetForm->apuSlowLimitCheckBox->setChecked(presetBuffer->apuSlowLimitChecked);
        presetForm->skinTempPowerLimitSpinBox->setValue(presetBuffer->skinTempPowerLimit);
        presetForm->skinTempPowerLimitCheckBox->setChecked(presetBuffer->skinTempPowerLimitChecked);

        //Add idxes to settings
        ui_settings->dcStateComboBox->insertItem(idx, presetBuffer->presetName, idx);
        ui_settings->acStateComboBox->insertItem(idx, presetBuffer->presetName, idx);
        ui_settings->epmBatterySaverComboBox->insertItem(idx, presetBuffer->presetName, idx);
        ui_settings->epmBetterBatteryComboBox->insertItem(idx, presetBuffer->presetName, idx);
        ui_settings->epmBalancedComboBox->insertItem(idx, presetBuffer->presetName, idx);
        ui_settings->epmMaximumPerfomanceComboBox->insertItem(idx, presetBuffer->presetName, idx);
        ui_settings->epmGamingComboBox->insertItem(idx, presetBuffer->presetName, idx);
    }
}

void CtrlGui::loadStyleSheet(){
    QFile configQFile;
    configQFile.setFileName(":/theme/mainwindow.qss");
    configQFile.open(QIODevice::ReadOnly);
    QString strStyleSheet = configQFile.readAll();
    this->setStyleSheet(strStyleSheet);
    settingFrame->setStyleSheet(strStyleSheet);
    configQFile.close();

    configQFile.setFileName(":/theme/topwidget.qss");
    configQFile.open(QIODevice::ReadOnly);
    strStyleSheet = configQFile.readAll();
    ui->topwidget->setStyleSheet(strStyleSheet);
    configQFile.close();

    configQFile.setFileName(":/theme/tabwidget.qss");
    configQFile.open(QIODevice::ReadOnly);
    strStyleSheet = configQFile.readAll();
    ui->scrollAreaWidgetContents->setStyleSheet(strStyleSheet);
    ui->scrollArea->setStyleSheet(strStyleSheet);
    configQFile.close();
}

void CtrlGui::savePreset(){
    int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    if(!infoMessageShowed){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("For better experience:"
                       "\nDisable auto switcher in settings.");
        msgBox.exec();
        infoMessageShowed = true;
        settingsStr* settings = conf->getSettingsBuffer();
        settings->showNotificationToDisableAutoSwitcher = true;
        messageToServiceStr messageToService;
        messageToService.saveSettings = true;
        messageToService.settings = *settings;
        bus->sendMessageToService(messageToService);
    }

    presetStr *presetBuffer = conf->getPresetBuffer(idx);

    Ui::CtrlGuiAPUForm *presetForm;
    for(qsizetype i = 0;i < presetFormList->count();i++)
        if(presetFormList->at(i)->applyPushButton->property("idx").toInt() == idx)
            presetForm = presetFormList->at(i);

    QString presetName = presetForm->presetNameEdit->text();
    for(qsizetype i = 0;i < presetName.size() && i < (sizeof(presetBuffer->presetName) - 1); i++)
        presetBuffer->presetName[i] = presetName.at(i).toLatin1();
    presetBuffer->presetName[presetName.size()] = '\0';

    presetBuffer->fanPresetId = presetForm->fanComboBox->currentIndex();

    presetBuffer->tempLimitValue = presetForm->tempLimitSpinBox->value();
    presetBuffer->tempLimitChecked = presetForm->tempLimitCheckBox->isChecked();
    presetBuffer->apuSkinValue = presetForm->apuSkinSpinBox->value();
    presetBuffer->apuSkinChecked = presetForm->apuSkinCheckBox->isChecked();
    presetBuffer->stampLimitValue = presetForm->stampLimitSpinBox->value();
    presetBuffer->stampLimitChecked = presetForm->stampLimitCheckBox->isChecked();
    presetBuffer->fastLimitValue = presetForm->fastLimitSpinBox->value();
    presetBuffer->fastLimitChecked = presetForm->fastLimitCheckBox->isChecked();
    presetBuffer->fastTimeValue = presetForm->fastTimeSpinBox->value();
    presetBuffer->fastTimeChecked = presetForm->fastTimeCheckBox->isChecked();
    presetBuffer->slowLimitValue = presetForm->slowLimitSpinBox->value();
    presetBuffer->slowLimitChecked = presetForm->slowLimitCheckBox->isChecked();
    presetBuffer->slowTimeValue = presetForm->slowTimeSpinBox->value();
    presetBuffer->slowTimeChecked = presetForm->slowTimeCheckBox->isChecked();

    presetBuffer->vrmCurrentValue = presetForm->vrmCurrentSpinBox->value();
    presetBuffer->vrmCurrentChecked = presetForm->vrmCurrentCheckBox->isChecked();
    presetBuffer->vrmMaxValue = presetForm->vrmMaxSpinBox->value();
    presetBuffer->vrmMaxChecked = presetForm->vrmMaxCheckBox->isChecked();

    presetBuffer->minFclkValue = presetForm->minFclkSpinBox->value();
    presetBuffer->minFclkChecked = presetForm->minFclkCheckBox->isChecked();
    presetBuffer->maxFclkValue = presetForm->maxFclkSpinBox->value();
    presetBuffer->maxFclkChecked = presetForm->maxFclkCheckBox->isChecked();

    presetBuffer->minGfxclkValue = presetForm->minGfxclkSpinBox->value();
    presetBuffer->minGfxclkChecked = presetForm->minGfxclkCheckBox->isChecked();
    presetBuffer->maxGfxclkValue = presetForm->maxGfxclkSpinBox->value();
    presetBuffer->maxGfxclkChecked = presetForm->maxGfxclkCheckBox->isChecked();

    presetBuffer->minSocclkValue = presetForm->minSocclkSpinBox->value();
    presetBuffer->minSocclkChecked = presetForm->minSocclkCheckBox->isChecked();
    presetBuffer->maxSocclkValue = presetForm->maxSocclkSpinBox->value();
    presetBuffer->maxSocclkChecked = presetForm->maxSocclkCheckBox->isChecked();

    presetBuffer->minVcnValue = presetForm->minVcnSpinBox->value();
    presetBuffer->minVcnChecked = presetForm->minVcnCheckBox->isChecked();
    presetBuffer->maxVcnValue = presetForm->maxVcnSpinBox->value();
    presetBuffer->maxVcnChecked = presetForm->maxVcnCheckBox->isChecked();

    presetBuffer->smuMaxPerfomance = presetForm->smuMaxPerformanceCheckBox->isChecked();
    presetBuffer->smuPowerSaving = presetForm->smuPowerSavingCheckBox->isChecked();

    //NEW VARS
    presetBuffer->vrmSocCurrent = presetForm->vrmSocCurrentSpinBox->value();
    presetBuffer->vrmSocCurrentChecked = presetForm->vrmSocCurrentCheckBox->isChecked();
    presetBuffer->vrmSocMax = presetForm->vrmSocMaxSpinBox->value();
    presetBuffer->vrmSocMaxChecked = presetForm->vrmSocMaxCheckBox->isChecked();

    presetBuffer->psi0Current = presetForm->psi0CurrentSpinBox->value();
    presetBuffer->psi0CurrentChecked = presetForm->psi0CurrentCheckBox->isChecked();
    presetBuffer->psi0SocCurrent = presetForm->psi0SocCurrentSpinBox->value();
    presetBuffer->psi0SocCurrentChecked = presetForm->psi0SocCurrentCheckBox->isChecked();

    presetBuffer->maxLclk = presetForm->maxLclkSpinBox->value();
    presetBuffer->maxLclkChecked = presetForm->maxLclkCheckBox->isChecked();
    presetBuffer->minLclk = presetForm->minLclkSpinBox->value();
    presetBuffer->minLclkChecked = presetForm->minLclkCheckBox->isChecked();

    presetBuffer->prochotDeassertionRamp = presetForm->prochotDeassertionRampSpinBox->value();
    presetBuffer->prochotDeassertionRampChecked = presetForm->prochotDeassertionRampCheckBox->isChecked();

    presetBuffer->dgpuSkinTempLimit = presetForm->dgpuSkinTempLimitSpinBox->value();
    presetBuffer->dgpuSkinTempLimitChecked = presetForm->dgpuSkinTempLimitCheckBox->isChecked();
    presetBuffer->apuSlowLimit = presetForm->apuSlowLimitSpinBox->value();
    presetBuffer->apuSlowLimitChecked = presetForm->apuSlowLimitCheckBox->isChecked();
    presetBuffer->skinTempPowerLimit = presetForm->skinTempPowerLimitSpinBox->value();
    presetBuffer->skinTempPowerLimitChecked = presetForm->skinTempPowerLimitCheckBox->isChecked();

    sendPreset(idx, true, false);
}

void CtrlGui::saveApplyPreset(){
    ui->label->setText("RyzenCtrl - Applying...");
    int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    if(!infoMessageShowed){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("For better experience:"
                       "\nDisable auto switcher in settings.");
        msgBox.exec();
        infoMessageShowed = true;
        settingsStr* settings = conf->getSettingsBuffer();
        settings->showNotificationToDisableAutoSwitcher = true;
        messageToServiceStr messageToService;
        messageToService.saveSettings = true;
        messageToService.settings = *settings;
        bus->sendMessageToService(messageToService);
    }

    presetStr *presetBuffer = conf->getPresetBuffer(idx);

    Ui::CtrlGuiAPUForm *presetForm;
    for(qsizetype i = 0;i < presetFormList->count();i++)
        if(presetFormList->at(i)->applyPushButton->property("idx").toInt() == idx)
            presetForm = presetFormList->at(i);

    QString presetName = presetForm->presetNameEdit->text();
    for(qsizetype i = 0;i < presetName.size() && i < (sizeof(presetBuffer->presetName) - 1); i++)
        presetBuffer->presetName[i] = presetName.at(i).toLatin1();
    presetBuffer->presetName[presetName.size()] = '\0';

    presetBuffer->fanPresetId = presetForm->fanComboBox->currentIndex();

    presetBuffer->tempLimitValue = presetForm->tempLimitSpinBox->value();
    presetBuffer->tempLimitChecked = presetForm->tempLimitCheckBox->isChecked();
    presetBuffer->apuSkinValue = presetForm->apuSkinSpinBox->value();
    presetBuffer->apuSkinChecked = presetForm->apuSkinCheckBox->isChecked();
    presetBuffer->stampLimitValue = presetForm->stampLimitSpinBox->value();
    presetBuffer->stampLimitChecked = presetForm->stampLimitCheckBox->isChecked();
    presetBuffer->fastLimitValue = presetForm->fastLimitSpinBox->value();
    presetBuffer->fastLimitChecked = presetForm->fastLimitCheckBox->isChecked();
    presetBuffer->fastTimeValue = presetForm->fastTimeSpinBox->value();
    presetBuffer->fastTimeChecked = presetForm->fastTimeCheckBox->isChecked();
    presetBuffer->slowLimitValue = presetForm->slowLimitSpinBox->value();
    presetBuffer->slowLimitChecked = presetForm->slowLimitCheckBox->isChecked();
    presetBuffer->slowTimeValue = presetForm->slowTimeSpinBox->value();
    presetBuffer->slowTimeChecked = presetForm->slowTimeCheckBox->isChecked();

    presetBuffer->vrmCurrentValue = presetForm->vrmCurrentSpinBox->value();
    presetBuffer->vrmCurrentChecked = presetForm->vrmCurrentCheckBox->isChecked();
    presetBuffer->vrmMaxValue = presetForm->vrmMaxSpinBox->value();
    presetBuffer->vrmMaxChecked = presetForm->vrmMaxCheckBox->isChecked();

    presetBuffer->minFclkValue = presetForm->minFclkSpinBox->value();
    presetBuffer->minFclkChecked = presetForm->minFclkCheckBox->isChecked();
    presetBuffer->maxFclkValue = presetForm->maxFclkSpinBox->value();
    presetBuffer->maxFclkChecked = presetForm->maxFclkCheckBox->isChecked();

    presetBuffer->minGfxclkValue = presetForm->minGfxclkSpinBox->value();
    presetBuffer->minGfxclkChecked = presetForm->minGfxclkCheckBox->isChecked();
    presetBuffer->maxGfxclkValue = presetForm->maxGfxclkSpinBox->value();
    presetBuffer->maxGfxclkChecked = presetForm->maxGfxclkCheckBox->isChecked();

    presetBuffer->minSocclkValue = presetForm->minSocclkSpinBox->value();
    presetBuffer->minSocclkChecked = presetForm->minSocclkCheckBox->isChecked();
    presetBuffer->maxSocclkValue = presetForm->maxSocclkSpinBox->value();
    presetBuffer->maxSocclkChecked = presetForm->maxSocclkCheckBox->isChecked();

    presetBuffer->minVcnValue = presetForm->minVcnSpinBox->value();
    presetBuffer->minVcnChecked = presetForm->minVcnCheckBox->isChecked();
    presetBuffer->maxVcnValue = presetForm->maxVcnSpinBox->value();
    presetBuffer->maxVcnChecked = presetForm->maxVcnCheckBox->isChecked();

    presetBuffer->smuMaxPerfomance = presetForm->smuMaxPerformanceCheckBox->isChecked();
    presetBuffer->smuPowerSaving = presetForm->smuPowerSavingCheckBox->isChecked();

    //NEW VARS
    presetBuffer->vrmSocCurrent = presetForm->vrmSocCurrentSpinBox->value();
    presetBuffer->vrmSocCurrentChecked = presetForm->vrmSocCurrentCheckBox->isChecked();
    presetBuffer->vrmSocMax = presetForm->vrmSocMaxSpinBox->value();
    presetBuffer->vrmSocMaxChecked = presetForm->vrmSocMaxCheckBox->isChecked();

    presetBuffer->vrmSocMax = presetForm->psi0CurrentSpinBox->value();
    presetBuffer->psi0CurrentChecked = presetForm->psi0CurrentCheckBox->isChecked();
    presetBuffer->psi0SocCurrent = presetForm->psi0SocCurrentSpinBox->value();
    presetBuffer->psi0SocCurrentChecked = presetForm->psi0SocCurrentCheckBox->isChecked();

    presetBuffer->maxLclk = presetForm->maxLclkSpinBox->value();
    presetBuffer->maxLclkChecked = presetForm->maxLclkCheckBox->isChecked();
    presetBuffer->minLclk = presetForm->minLclkSpinBox->value();
    presetBuffer->minLclkChecked = presetForm->minLclkCheckBox->isChecked();

    presetBuffer->prochotDeassertionRamp = presetForm->prochotDeassertionRampSpinBox->value();
    presetBuffer->prochotDeassertionRampChecked = presetForm->prochotDeassertionRampCheckBox->isChecked();

    presetBuffer->dgpuSkinTempLimit = presetForm->dgpuSkinTempLimitSpinBox->value();
    presetBuffer->dgpuSkinTempLimitChecked = presetForm->dgpuSkinTempLimitCheckBox->isChecked();
    presetBuffer->apuSlowLimit = presetForm->apuSlowLimitSpinBox->value();
    presetBuffer->apuSlowLimitChecked = presetForm->apuSlowLimitCheckBox->isChecked();
    presetBuffer->skinTempPowerLimit = presetForm->skinTempPowerLimitSpinBox->value();
    presetBuffer->skinTempPowerLimitChecked = presetForm->skinTempPowerLimitCheckBox->isChecked();

    sendPreset(idx, true, true);
}

void CtrlGui::applyPreset(){
    ui->label->setText("RyzenCtrl - Applying...");
    int i = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    if(!infoMessageShowed){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("For better experience:"
                       "\nDisable auto switcher in settings.");
        msgBox.exec();
        infoMessageShowed = true;
        settingsStr* settings = conf->getSettingsBuffer();
        settings->showNotificationToDisableAutoSwitcher = true;
        messageToServiceStr messageToService;
        messageToService.saveSettings = true;
        messageToService.settings = *settings;
        bus->sendMessageToService(messageToService);
    }

    sendPreset(i, false, true);
}

void CtrlGui::cancelPreset(){
    ui->label->setText("RyzenCtrl - Applying...");
    int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    presetStr *presetBuffer = conf->getPresetBuffer(idx);

    Ui::CtrlGuiAPUForm *presetForm;
    for(qsizetype i = 0;i < presetFormList->count();i++)
        if(presetFormList->at(i)->applyPushButton->property("idx").toInt() == idx)
            presetForm = presetFormList->at(i);

    presetForm->fanComboBox->setCurrentIndex(presetBuffer->fanPresetId);

    presetForm->tempLimitSpinBox->setValue(presetBuffer->tempLimitValue);
    presetForm->tempLimitCheckBox->setChecked(presetBuffer->tempLimitChecked);
    presetForm->apuSkinSpinBox->setValue(presetBuffer->apuSkinValue);
    presetForm->apuSkinCheckBox->setChecked(presetBuffer->apuSkinChecked);
    presetForm->stampLimitSpinBox->setValue(presetBuffer->stampLimitValue);
    presetForm->stampLimitCheckBox->setChecked(presetBuffer->stampLimitChecked);
    presetForm->fastLimitSpinBox->setValue(presetBuffer->fastLimitValue);
    presetForm->fastLimitCheckBox->setChecked(presetBuffer->fastLimitChecked);
    presetForm->fastTimeSpinBox->setValue(presetBuffer->fastTimeValue);
    presetForm->fastTimeCheckBox->setChecked(presetBuffer->fastTimeChecked);
    presetForm->slowLimitSpinBox->setValue(presetBuffer->slowLimitValue);
    presetForm->slowLimitCheckBox->setChecked(presetBuffer->slowLimitChecked);
    presetForm->slowTimeSpinBox->setValue(presetBuffer->slowTimeValue);
    presetForm->slowTimeCheckBox->setChecked(presetBuffer->slowTimeChecked);

    presetForm->vrmCurrentSpinBox->setValue(presetBuffer->vrmCurrentValue);
    presetForm->vrmCurrentCheckBox->setChecked(presetBuffer->vrmCurrentChecked);
    presetForm->vrmMaxSpinBox->setValue(presetBuffer->vrmMaxValue);
    presetForm->vrmMaxCheckBox->setChecked(presetBuffer->vrmMaxChecked);

    presetForm->minFclkSpinBox->setValue(presetBuffer->minFclkValue);
    presetForm->minFclkCheckBox->setChecked(presetBuffer->minFclkChecked);
    presetForm->maxFclkSpinBox->setValue(presetBuffer->maxFclkValue);
    presetForm->maxFclkCheckBox->setChecked(presetBuffer->maxFclkChecked);

    presetForm->minGfxclkSpinBox->setValue(presetBuffer->minGfxclkValue);
    presetForm->minGfxclkCheckBox->setChecked(presetBuffer->minGfxclkChecked);
    presetForm->maxGfxclkSpinBox->setValue(presetBuffer->maxGfxclkValue);
    presetForm->maxGfxclkCheckBox->setChecked(presetBuffer->maxGfxclkChecked);
    presetForm->minSocclkSpinBox->setValue(presetBuffer->minSocclkValue);
    presetForm->minSocclkCheckBox->setChecked(presetBuffer->minSocclkChecked);
    presetForm->maxSocclkSpinBox->setValue(presetBuffer->maxSocclkValue);
    presetForm->maxSocclkCheckBox->setChecked(presetBuffer->maxSocclkChecked);
    presetForm->minVcnSpinBox->setValue(presetBuffer->minVcnValue);
    presetForm->minVcnCheckBox->setChecked(presetBuffer->minVcnChecked);
    presetForm->maxVcnSpinBox->setValue(presetBuffer->maxVcnValue);
    presetForm->maxVcnCheckBox->setChecked(presetBuffer->maxVcnChecked);

    presetForm->smuMaxPerformanceCheckBox->setChecked(presetBuffer->smuMaxPerfomance);
    presetForm->smuPowerSavingCheckBox->setChecked(presetBuffer->smuPowerSaving);
    //NEW VARS
    presetForm->vrmSocCurrentSpinBox->setValue(presetBuffer->vrmSocCurrent);
    presetForm->vrmSocCurrentCheckBox->setChecked(presetBuffer->vrmSocCurrentChecked);
    presetForm->vrmSocMaxSpinBox->setValue(presetBuffer->vrmSocMax);
    presetForm->vrmSocMaxCheckBox->setChecked(presetBuffer->vrmSocMaxChecked);

    presetForm->psi0CurrentSpinBox->setValue(presetBuffer->psi0Current);
    presetForm->psi0CurrentCheckBox->setChecked(presetBuffer->psi0CurrentChecked);
    presetForm->psi0SocCurrentSpinBox->setValue(presetBuffer->psi0SocCurrent);
    presetForm->psi0SocCurrentCheckBox->setChecked(presetBuffer->psi0SocCurrentChecked);

    presetForm->maxLclkSpinBox->setValue(presetBuffer->maxLclk);
    presetForm->maxLclkCheckBox->setChecked(presetBuffer->maxLclkChecked);
    presetForm->minLclkSpinBox->setValue(presetBuffer->minLclk);
    presetForm->minLclkCheckBox->setChecked(presetBuffer->minLclkChecked);

    presetForm->prochotDeassertionRampSpinBox->setValue(presetBuffer->prochotDeassertionRamp);
    presetForm->prochotDeassertionRampCheckBox->setChecked(presetBuffer->prochotDeassertionRampChecked);

    presetForm->dgpuSkinTempLimitSpinBox->setValue(presetBuffer->dgpuSkinTempLimit);
    presetForm->dgpuSkinTempLimitCheckBox->setChecked(presetBuffer->dgpuSkinTempLimitChecked);
    presetForm->apuSlowLimitSpinBox->setValue(presetBuffer->apuSlowLimit);
    presetForm->apuSlowLimitCheckBox->setChecked(presetBuffer->apuSlowLimitChecked);
    presetForm->skinTempPowerLimitSpinBox->setValue(presetBuffer->skinTempPowerLimit);
    presetForm->skinTempPowerLimitCheckBox->setChecked(presetBuffer->skinTempPowerLimitChecked);

    sendPreset(idx, false, true);
}

void CtrlGui::sendPreset(int idx, bool save, bool apply){
    Ui::CtrlGuiAPUForm *presetForm;
    for(qsizetype i = 0;i < presetFormList->count();i++) {
        if(presetFormList->at(i)->applyPushButton->property("idx").toInt() == idx)
            presetForm = presetFormList->at(i);
    }
    //NEW WAY
    presetStr presetBuffer;
    presetBuffer.presetId = idx;
    QString presetName = presetForm->presetNameEdit->text();
    for(qsizetype i = 0;i < presetName.size() && i < (sizeof(presetBuffer.presetName) - 1); i++)
        presetBuffer.presetName[i] = presetName.at(i).toLatin1();
    presetBuffer.presetName[presetName.size()] = '\0';

    presetBuffer.fanPresetId = presetForm->fanComboBox->currentIndex();

    presetBuffer.tempLimitValue = presetForm->tempLimitSpinBox->value();
    presetBuffer.tempLimitChecked = presetForm->tempLimitCheckBox->isChecked();
    presetBuffer.apuSkinValue = presetForm->apuSkinSpinBox->value();
    presetBuffer.apuSkinChecked = presetForm->apuSkinCheckBox->isChecked();

    presetBuffer.stampLimitValue = presetForm->stampLimitSpinBox->value();
    presetBuffer.stampLimitChecked = presetForm->stampLimitCheckBox->isChecked();
    presetBuffer.fastLimitValue = presetForm->fastLimitSpinBox->value();
    presetBuffer.fastLimitChecked = presetForm->fastLimitCheckBox->isChecked();
    presetBuffer.fastTimeValue = presetForm->fastTimeSpinBox->value();
    presetBuffer.fastTimeChecked = presetForm->fastTimeCheckBox->isChecked();
    presetBuffer.slowLimitValue = presetForm->slowLimitSpinBox->value();
    presetBuffer.slowLimitChecked = presetForm->slowLimitCheckBox->isChecked();
    presetBuffer.slowTimeValue = presetForm->slowTimeSpinBox->value();
    presetBuffer.slowTimeChecked = presetForm->slowTimeCheckBox->isChecked();

    presetBuffer.vrmCurrentValue = presetForm->vrmCurrentSpinBox->value();
    presetBuffer.vrmCurrentChecked = presetForm->vrmCurrentCheckBox->isChecked();
    presetBuffer.vrmMaxValue = presetForm->vrmMaxSpinBox->value();
    presetBuffer.vrmMaxChecked = presetForm->vrmMaxCheckBox->isChecked();

    presetBuffer.minFclkValue = presetForm->minFclkSpinBox->value();
    presetBuffer.minFclkChecked = presetForm->minFclkCheckBox->isChecked();
    presetBuffer.maxFclkValue = presetForm->maxFclkSpinBox->value();
    presetBuffer.maxFclkChecked = presetForm->maxFclkCheckBox->isChecked();

    presetBuffer.minGfxclkValue = presetForm->minGfxclkSpinBox->value();
    presetBuffer.minGfxclkChecked = presetForm->minGfxclkCheckBox->isChecked();
    presetBuffer.maxGfxclkValue = presetForm->maxGfxclkSpinBox->value();
    presetBuffer.maxGfxclkChecked = presetForm->maxGfxclkCheckBox->isChecked();
    presetBuffer.minSocclkValue = presetForm->minSocclkSpinBox->value();
    presetBuffer.minSocclkChecked = presetForm->minSocclkCheckBox->isChecked();
    presetBuffer.maxSocclkValue = presetForm->maxSocclkSpinBox->value();
    presetBuffer.maxSocclkChecked = presetForm->maxSocclkCheckBox->isChecked();
    presetBuffer.minVcnValue = presetForm->minVcnSpinBox->value();
    presetBuffer.minVcnChecked = presetForm->minVcnCheckBox->isChecked();
    presetBuffer.maxVcnValue = presetForm->maxVcnSpinBox->value();
    presetBuffer.maxVcnChecked = presetForm->maxVcnCheckBox->isChecked();

    presetBuffer.smuMaxPerfomance = presetForm->smuMaxPerformanceCheckBox->isChecked();
    presetBuffer.smuPowerSaving = presetForm->smuPowerSavingCheckBox->isChecked();
    //NEW VARS
    presetBuffer.vrmSocCurrent = presetForm->vrmSocCurrentSpinBox->value();
    presetBuffer.vrmSocCurrentChecked = presetForm->vrmSocCurrentCheckBox->isChecked();
    presetBuffer.vrmSocMax = presetForm->vrmSocMaxSpinBox->value();
    presetBuffer.vrmSocMaxChecked = presetForm->vrmSocMaxCheckBox->isChecked();

    presetBuffer.psi0Current = presetForm->psi0CurrentSpinBox->value();
    presetBuffer.psi0CurrentChecked = presetForm->psi0CurrentCheckBox->isChecked();
    presetBuffer.psi0SocCurrent = presetForm->psi0SocCurrentSpinBox->value();
    presetBuffer.psi0SocCurrentChecked = presetForm->psi0SocCurrentCheckBox->isChecked();

    presetBuffer.maxLclk = presetForm->maxLclkSpinBox->value();
    presetBuffer.maxLclkChecked = presetForm->maxLclkCheckBox->isChecked();
    presetBuffer.minLclk = presetForm->minLclkSpinBox->value();
    presetBuffer.minLclkChecked = presetForm->minLclkCheckBox->isChecked();

    presetBuffer.prochotDeassertionRamp = presetForm->prochotDeassertionRampSpinBox->value();
    presetBuffer.prochotDeassertionRampChecked = presetForm->prochotDeassertionRampCheckBox->isChecked();

    presetBuffer.dgpuSkinTempLimit = presetForm->dgpuSkinTempLimitSpinBox->value();
    presetBuffer.dgpuSkinTempLimitChecked = presetForm->dgpuSkinTempLimitCheckBox->isChecked();
    presetBuffer.apuSlowLimit = presetForm->apuSlowLimitSpinBox->value();
    presetBuffer.apuSlowLimitChecked = presetForm->apuSlowLimitCheckBox->isChecked();
    presetBuffer.skinTempPowerLimit = presetForm->skinTempPowerLimitSpinBox->value();
    presetBuffer.skinTempPowerLimitChecked = presetForm->skinTempPowerLimitCheckBox->isChecked();

    messageToServiceStr messageToService;
    messageToService.savePreset = save;
    messageToService.applyPreset = apply;
    messageToService.preset = presetBuffer;
    bus->sendMessageToService(messageToService);
}

void CtrlGui::smuCheckBoxClicked(){
    int idx = reinterpret_cast<QCheckBox *>(sender())->property("idx").toInt();
    int idy = reinterpret_cast<QCheckBox *>(sender())->property("idy").toInt();

    Ui::CtrlGuiAPUForm *presetForm;
    for(qsizetype i = 0;i < presetFormList->count();i++)
        if(presetFormList->at(i)->applyPushButton->property("idx").toInt() == idx)
            presetForm = presetFormList->at(i);

    if(idy == 0) {
        if(presetForm->smuMaxPerformanceCheckBox->isChecked())
            presetForm->smuPowerSavingCheckBox->setChecked(false);
    } else {
        if(presetForm->smuPowerSavingCheckBox->isChecked())
            presetForm->smuMaxPerformanceCheckBox->setChecked(false);
    }
}

CtrlGui::~CtrlGui()
{
    delete ui;
}

void CtrlGui::languageChange(QString langid){
    Ui::CtrlGuiAPUForm *presetForm;

    if(qtLanguageTranslator->load(QString("Language/CtrlGui_") + langid, ".")){
        qApp->installTranslator(qtLanguageTranslator);
        ui->retranslateUi(this);

        for(qsizetype i = 0;i < presetFormList->count();i++){
                presetForm = presetFormList->at(i);
                for(qsizetype i = 0;i < tabWidgetsList->count();i++)
                    if(tabWidgetsList->at(i)->property("idx").toInt()
                            == presetFormList->at(i)->applyPushButton
                            ->property("idx").toInt())
                        presetForm->retranslateUi(tabWidgetsList->at(i));
        }
    } else {
        qApp->removeTranslator(qtLanguageTranslator);
        ui->retranslateUi(this);

        for(qsizetype i = 0;i < presetFormList->count();i++){
                presetForm = presetFormList->at(i);
                for(qsizetype i = 0;i < tabWidgetsList->count();i++)
                    if(tabWidgetsList->at(i)->property("idx").toInt()
                            == presetFormList->at(i)->applyPushButton
                            ->property("idx").toInt())
                        presetForm->retranslateUi(tabWidgetsList->at(i));
        }
    }
}

void CtrlGui::saveSettings(){
    settingFrame->hide();
    settingsStr* settings = conf->getSettingsBuffer();

    //
    if(settings->useAgent != ui_settings->useAgentGroupBox->isChecked())
        settings->useAgent = ui_settings->useAgentGroupBox->isChecked();
    if(settings->showNotifications != ui_settings->showNotificationsCheckBox->isChecked())
        settings->showNotifications = ui_settings->showNotificationsCheckBox->isChecked();

    if(settings->autoPresetApplyDurationChecked != ui_settings->reapplyDurationGroupBox->isChecked())
        settings->autoPresetApplyDurationChecked = ui_settings->reapplyDurationGroupBox->isChecked();
    if(settings->autoPresetApplyDuration != ui_settings->reapplyDurationSpinBox->value())
        settings->autoPresetApplyDuration = ui_settings->reapplyDurationSpinBox->value();

    if(settings->autoPresetSwitchAC != ui_settings->acAutoPresetSwitchGroupBox->isChecked())
        settings->autoPresetSwitchAC = ui_settings->acAutoPresetSwitchGroupBox->isChecked();
    if(settings->dcStatePresetId != ui_settings->dcStateComboBox->currentData().toInt())
        settings->dcStatePresetId = ui_settings->dcStateComboBox->currentData().toInt();
    if(settings->acStatePresetId != ui_settings->acStateComboBox->currentData().toInt())
        settings->acStatePresetId = ui_settings->acStateComboBox->currentData().toInt();

    if(settings->epmAutoPresetSwitch != ui_settings->epmAutoPresetSwitchGroupBox->isChecked())
        settings->epmAutoPresetSwitch = ui_settings->epmAutoPresetSwitchGroupBox->isChecked();
    if(settings->epmBatterySaverPresetId != ui_settings->epmBatterySaverComboBox->currentData().toInt())
        settings->epmBatterySaverPresetId = ui_settings->epmBatterySaverComboBox->currentData().toInt();
    if(settings->epmBetterBatteryPresetId != ui_settings->epmBetterBatteryComboBox->currentData().toInt())
        settings->epmBetterBatteryPresetId = ui_settings->epmBetterBatteryComboBox->currentData().toInt();
    if(settings->epmBalancedPresetId != ui_settings->epmBalancedComboBox->currentData().toInt())
        settings->epmBalancedPresetId = ui_settings->epmBalancedComboBox->currentData().toInt();
    if(settings->epmMaximumPerfomancePresetId != ui_settings->epmMaximumPerfomanceComboBox->currentData().toInt())
        settings->epmMaximumPerfomancePresetId = ui_settings->epmMaximumPerfomanceComboBox->currentData().toInt();
    if(settings->epmGamingPresetId != ui_settings->epmGamingComboBox->currentData().toInt())
        settings->epmGamingPresetId = ui_settings->epmGamingComboBox->currentData().toInt();

    settings->hideNotSupportedVariables = ui_settings->hideVarsGroupBox->isChecked();
    settings->apuFamilyIdx = ui_settings->apuFamilyComboBox->currentIndex();

    settings->showArmourPlugin = ui_settings->showArmourCheckBox->isChecked();
    //

    messageToServiceStr messageToService;
    messageToService.saveSettings = true;
    messageToService.settings = *settings;
    bus->sendMessageToService(messageToService);

    useAgent(settings->useAgent);

    //Hide not supported variables
    Ui::CtrlGuiAPUForm *presetForm = nullptr;
    hideShow *var = conf->hideShowWarnPresetVariable(
                settings->hideNotSupportedVariables
                ? settings->apuFamilyIdx : -1);
    for(qsizetype i = 0;i < presetFormList->count();i++){
        presetForm = presetFormList->at(i);

        presetForm->tempLimitWidget->setVisible(var->shwTctlTemp);
        presetForm->apuSkinWidget->setVisible(var->shwApuSkinTemp);
        presetForm->stampLimitWidget->setVisible(var->shwStapmLimit);
        presetForm->fastLimitWidget->setVisible(var->shwFastLimit);
        presetForm->slowLimitWidget->setVisible(var->shwSlowLimit);
        presetForm->slowTimeWidget->setVisible(var->shwSlowTime);
        presetForm->fastTimeWidget->setVisible(var->shwStapmTime);

        presetForm->vrmCurrentWidget->setVisible(var->shwVrmCurrent);
        presetForm->vrmMaxWidget->setVisible(var->shwVrmMaxCurrent);

        presetForm->minFclkWidget->setVisible(var->shwMinFclkFrequency);
        presetForm->maxFclkWidget->setVisible(var->shwMaxFclkFrequency);

        presetForm->minGfxclkWidget->setVisible(var->shwMinGfxclk);
        presetForm->maxGfxclkWidget->setVisible(var->shwMaxGfxclk);
        presetForm->minSocclkWidget->setVisible(var->shwMinSocclkFrequency);
        presetForm->maxSocclkWidget->setVisible(var->shwMaxSocclkFrequency);
        presetForm->minVcnWidget->setVisible(var->shwMinVcn);
        presetForm->maxVcnWidget->setVisible(var->shwMaxVcn);

        presetForm->smuMaxPerformanceCheckBox->setVisible(var->shwMaxPerformance);
        presetForm->smuPowerSavingCheckBox->setVisible(var->shwPowerSaving);

        //NEW VARS
        presetForm->vrmSocCurrentWidget->setVisible(var->shwVrmSocCurrent);
        presetForm->vrmSocMaxWidget->setVisible(var->shwVrmSocMaxCurrent);

        presetForm->psi0CurrentWidget->setVisible(var->shwPsi0Current);
        presetForm->psi0SocCurrentWidget->setVisible(var->shwPsi0SocCurrent);

        presetForm->maxLclkWidget->setVisible(var->shwMaxLclk);
        presetForm->minLclkWidget->setVisible(var->shwMinLclk);

        presetForm->prochotDeassertionRampWidget->setVisible(var->shwProchotDeassertionRamp);

        presetForm->dgpuSkinTempLimitWidget->setVisible(var->shwDgpuSkinTemp);
        presetForm->apuSlowLimitWidget->setVisible(var->shwApuSlowLimit);
        presetForm->skinTempPowerLimitWidget->setVisible(var->shwSkinTempLimit);

        //Show Armour Plugin
        presetForm->armourGroupBox->setVisible(settings->showArmourPlugin);
    }
}

void CtrlGui::readSettings(){
    settingsStr *settings = conf->getSettingsBuffer();

    ui_settings->useAgentGroupBox->setChecked(settings->useAgent);
    ui_settings->showNotificationsCheckBox->setChecked(settings->showNotifications);

    infoMessageShowed = settings->showNotificationToDisableAutoSwitcher;

    ui_settings->reapplyDurationGroupBox->setChecked(settings->autoPresetApplyDurationChecked);
    ui_settings->reapplyDurationSpinBox->setValue(settings->autoPresetApplyDuration);

    ui_settings->acAutoPresetSwitchGroupBox->setChecked(settings->autoPresetSwitchAC);

    ui_settings->epmAutoPresetSwitchGroupBox->setChecked(settings->epmAutoPresetSwitch);
    ui_settings->epmBatterySaverComboBox->setCurrentIndex(settings->epmBatterySaverPresetId);
    ui_settings->epmBetterBatteryComboBox->setCurrentIndex(settings->epmBetterBatteryPresetId);
    ui_settings->epmBalancedComboBox->setCurrentIndex(settings->epmBalancedPresetId);
    ui_settings->epmMaximumPerfomanceComboBox->setCurrentIndex(settings->epmMaximumPerfomancePresetId);
    ui_settings->epmGamingComboBox->setCurrentIndex(settings->epmGamingPresetId);

    for(qsizetype i = 0;i < conf->getPresetsCount();i++){
        if(ui_settings->dcStateComboBox->itemData(i) == settings->dcStatePresetId)
            ui_settings->dcStateComboBox->setCurrentIndex(i);

        if(ui_settings->acStateComboBox->itemData(i) == settings->acStatePresetId)
            ui_settings->acStateComboBox->setCurrentIndex(i);

        if(ui_settings->epmBatterySaverComboBox->itemData(i)
                == settings->epmBatterySaverPresetId)
            ui_settings->epmBatterySaverComboBox->setCurrentIndex(i);

        if(ui_settings->epmBetterBatteryComboBox->itemData(i)
                == settings->epmBetterBatteryPresetId)
            ui_settings->epmBetterBatteryComboBox->setCurrentIndex(i);

        if(ui_settings->epmBalancedComboBox->itemData(i)
                == settings->epmBalancedPresetId)
            ui_settings->epmBalancedComboBox->setCurrentIndex(i);

        if(ui_settings->epmMaximumPerfomanceComboBox->itemData(i)
                == settings->epmMaximumPerfomancePresetId)
            ui_settings->epmMaximumPerfomanceComboBox->setCurrentIndex(i);

        if(ui_settings->epmGamingComboBox->itemData(i)
                == settings->epmGamingPresetId)
            ui_settings->epmGamingComboBox->setCurrentIndex(i);
    }

    ui_settings->hideVarsGroupBox->setChecked(settings->hideNotSupportedVariables);
    ui_settings->apuFamilyComboBox->setCurrentIndex(settings->apuFamilyIdx);

    ui_settings->showArmourCheckBox->setChecked(settings->showArmourPlugin);

    useAgent(settings->useAgent);

    //Hide not supported variables
    Ui::CtrlGuiAPUForm *presetForm = nullptr;
    hideShow *var = conf->hideShowWarnPresetVariable(
                settings->hideNotSupportedVariables
                ? settings->apuFamilyIdx : -1);
    for(qsizetype i = 0;i < presetFormList->count();i++){
        presetForm = presetFormList->at(i);

        presetForm->tempLimitWidget->setVisible(var->shwTctlTemp);
        presetForm->apuSkinWidget->setVisible(var->shwApuSkinTemp);
        presetForm->stampLimitWidget->setVisible(var->shwStapmLimit);
        presetForm->fastLimitWidget->setVisible(var->shwFastLimit);
        presetForm->slowLimitWidget->setVisible(var->shwSlowLimit);
        presetForm->slowTimeWidget->setVisible(var->shwSlowTime);
        presetForm->fastTimeWidget->setVisible(var->shwStapmTime);

        presetForm->vrmCurrentWidget->setVisible(var->shwVrmCurrent);
        presetForm->vrmMaxWidget->setVisible(var->shwVrmMaxCurrent);

        presetForm->minFclkWidget->setVisible(var->shwMinFclkFrequency);
        presetForm->maxFclkWidget->setVisible(var->shwMaxFclkFrequency);

        presetForm->minGfxclkWidget->setVisible(var->shwMinGfxclk);
        presetForm->maxGfxclkWidget->setVisible(var->shwMaxGfxclk);
        presetForm->minSocclkWidget->setVisible(var->shwMinSocclkFrequency);
        presetForm->maxSocclkWidget->setVisible(var->shwMaxSocclkFrequency);
        presetForm->minVcnWidget->setVisible(var->shwMinVcn);
        presetForm->maxVcnWidget->setVisible(var->shwMaxVcn);

        presetForm->smuMaxPerformanceCheckBox->setVisible(var->shwMaxPerformance);
        presetForm->smuPowerSavingCheckBox->setVisible(var->shwPowerSaving);

        //NEW VARS
        presetForm->vrmSocCurrentWidget->setVisible(var->shwVrmSocCurrent);
        presetForm->vrmSocMaxWidget->setVisible(var->shwVrmSocMaxCurrent);

        presetForm->psi0CurrentWidget->setVisible(var->shwPsi0Current);
        presetForm->psi0SocCurrentWidget->setVisible(var->shwPsi0SocCurrent);

        presetForm->maxLclkWidget->setVisible(var->shwMaxLclk);
        presetForm->minLclkWidget->setVisible(var->shwMinLclk);

        presetForm->prochotDeassertionRampWidget->setVisible(var->shwProchotDeassertionRamp);

        presetForm->dgpuSkinTempLimitWidget->setVisible(var->shwDgpuSkinTemp);
        presetForm->apuSlowLimitWidget->setVisible(var->shwApuSlowLimit);
        presetForm->skinTempPowerLimitWidget->setVisible(var->shwSkinTempLimit);

        //Show Armour Plugin
        presetForm->armourGroupBox->setVisible(settings->showArmourPlugin);
    }
}

void CtrlGui::cancelSettings(){
    settingFrame->hide();
    readSettings();
}

#ifdef BUILD_SERVICE
void CtrlGui::startService(){
    QString runas = ("\"" + qApp->arguments().value(0) + "\" startup");
    QProcess process;
    process.startDetached("powershell", QStringList({"start-process", runas, "-verb", "runas"}));
}

void CtrlGui::installService(){
    QProcess process;
    QString runas = ("\"" + qApp->arguments().value(0) + "\" check");
    process.startDetached("powershell", QStringList({"start-process", runas, "-verb", "runas"}));
}
#endif

void CtrlGui::infoPushButtonClicked() {
    if(!infoWidgetHasBeenShowed){
        if(this->isVisible()){
            QRect rect = infoFrame->geometry();
            const QRect windowGeometry = this->geometry();
            int width = rect.width();
            rect.setX(windowGeometry.right() + 4);
            rect.setY(windowGeometry.top());
            rect.setWidth(width);
            rect.setHeight(windowGeometry.height());
            infoFrame->setGeometry(rect);
            infoWidgetHasBeenShowed = true;
        } else {
            QRect rect = infoFrame->geometry();
            const QRect primaryScreenGeometry = qApp->primaryScreen()->geometry();
            int width = 310;
            int height = 448;
            rect.setX((primaryScreenGeometry.right() / 2) - (width / 2));
            rect.setY((primaryScreenGeometry.bottom() / 2) - (height / 2));
            rect.setWidth(width);
            rect.setHeight(height);
            infoFrame->setGeometry(rect);
            infoWidgetHasBeenShowed = true;
        }
    }

    infoFrame->setHidden(infoFrame->isVisible());
    ui->infoPushButton->setChecked(infoFrame->isVisible());
    if(ui_agent != nullptr){
        ui_agent->infoPushButtonClicked(infoFrame->isVisible());
    }
    sendRyzenAdjInfo();
}

void CtrlGui::sendRyzenAdjInfo(QString value){
    if(infoFrame->isVisible())
        value = QString::number(ui_infoWidget->spinBox->value());

    settingsStr* settings = conf->getSettingsBuffer();
    settings->showNotificationToDisableAutoSwitcher = true;
    messageToServiceStr messageToService;
    messageToService.ryzenAdjInfo = true;
    messageToService.ryzenAdjInfoTimeout = value.toInt();
    bus->sendMessageToService(messageToService);
}

void CtrlGui::settingsPushButtonClicked() {
    QRect rect = settingFrame->geometry();
    const QRect buttonGeometry = ui->settingsPushButton->geometry();
    const QRect windowGeometry = this->geometry();
    int width = rect.width();
    int height = windowGeometry.height() - 8;
    rect.setX(buttonGeometry.left()+windowGeometry.left()+buttonGeometry.width()-width);
    rect.setY(buttonGeometry.top()+windowGeometry.top());
    rect.setWidth(width);
    rect.setHeight(height);
    settingFrame->setGeometry(rect);

    if (settingFrame->isHidden()) settingFrame->show();
}

void CtrlGui::presetPushButtonClicked(){
    int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    for(int i = 0;i < tabWidgetsList->count();i++)
        tabWidgetsList->at(i)->setHidden(true);
    for(int i = 0;i < tabButtonList->count();i++)
        tabButtonList->at(i)->setChecked(false);

    for(int i = 0;i < tabWidgetsList->count();i++)
        if(tabWidgetsList->at(i)->property("idx").toInt() == idx)
            tabWidgetsList->at(i)->setHidden(false);
    for(int i = 0;i < tabButtonList->count();i++)
        if(tabButtonList->at(i)->property("idx").toInt() == idx)
            tabButtonList->at(i)->setChecked(true);
}

void CtrlGui::presetPlusPushButtonClicked(){
    //Remove + button and spacer
    ui->scrollAreaWidgetContents->layout()->removeWidget(tabPlusButton);
    ui->scrollAreaWidgetContents->layout()->removeItem(spacer);
    //Create preset
    int idx = conf->insertNewPreset();
    presetStr *presetBuffer = conf->getPresetBuffer(idx);
    //Send new preset to service
    messageToServiceStr messageToService;
    messageToService.savePreset = true;
    messageToService.preset = *presetBuffer;
    bus->sendMessageToService(messageToService);
    //FONT
    QFont font;
    font.setPointSize(9);
    font.setBold(true);
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    //TAB WIDGET AND FORM
    QWidget *widget = new QWidget;
    widget->setProperty("idx",idx);
    tabWidgetsList->append(widget);
    verticalLayout->addWidget(widget);
    Ui::CtrlGuiAPUForm *presetForm = new Ui::CtrlGuiAPUForm;
    presetForm->setupUi(widget);
    presetFormList->append(presetForm);
    widget->setHidden(true);
    //Write idx-es
    presetForm->deletePushButton->setProperty("idx",idx);
    presetForm->presetNameEdit->setText(presetBuffer->presetName);
    presetForm->presetNameEdit->setProperty("idx",idx);
    presetForm->saveApplyPushButton->setProperty("idx",idx);
    presetForm->saveOnlyPushButton->setProperty("idx",idx);
    presetForm->applyPushButton->setProperty("idx",idx);
    presetForm->cancelPushButton->setProperty("idx",idx);
    presetForm->fanComboBox->setProperty("idx",idx);
    presetForm->smuMaxPerformanceCheckBox->setProperty("idx",idx);
    presetForm->smuPowerSavingCheckBox->setProperty("idx",idx);
    presetForm->smuMaxPerformanceCheckBox->setProperty("idy",0);
    presetForm->smuPowerSavingCheckBox->setProperty("idy",1);
    //TAB BUTTON
    QPushButton *button = new QPushButton(presetBuffer->presetName);
    button->setObjectName(QString::fromUtf8("tabPushButton") + QString::number(idx));
    button->setMinimumSize(QSize(105, 23));
    button->setFont(font);
    button->setStyleSheet(QString::fromUtf8(""));
    button->setCheckable(true);
    button->setProperty("idx",idx);
    button->setSizePolicy(sizePolicy);
    ui->scrollAreaWidgetContents->layout()->addWidget(button);
    tabButtonList->append(button);
    //Connections
    connect(button, &QPushButton::clicked, this, &CtrlGui::presetPushButtonClicked);
    connect(presetForm->deletePushButton, &QPushButton::clicked, this, &CtrlGui::presetDeletePushButtonClicked);
    connect(presetForm->presetNameEdit, &QLineEdit::textChanged, this, &CtrlGui::presetNameEditChanged);
    connect(presetForm->saveApplyPushButton, &QPushButton::clicked, this, &CtrlGui::saveApplyPreset);
    connect(presetForm->saveOnlyPushButton, &QPushButton::clicked, this, &CtrlGui::savePreset);
    connect(presetForm->applyPushButton, &QPushButton::clicked, this, &CtrlGui::applyPreset);
    connect(presetForm->cancelPushButton, &QPushButton::clicked, this, &CtrlGui::cancelPreset);
    connect(presetForm->smuMaxPerformanceCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);
    connect(presetForm->smuPowerSavingCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);
    //Replace + button and spacer
    ui->scrollAreaWidgetContents->layout()->addWidget(tabPlusButton);
    ui->scrollAreaWidgetContents->layout()->addItem(spacer);
    //Get values from conf
    presetForm->fanComboBox->setCurrentIndex(presetBuffer->fanPresetId);
    presetForm->tempLimitSpinBox->setValue(presetBuffer->tempLimitValue);
    presetForm->tempLimitCheckBox->setChecked(presetBuffer->tempLimitChecked);
    presetForm->apuSkinSpinBox->setValue(presetBuffer->apuSkinValue);
    presetForm->apuSkinCheckBox->setChecked(presetBuffer->apuSkinChecked);
    presetForm->stampLimitSpinBox->setValue(presetBuffer->stampLimitValue);
    presetForm->stampLimitCheckBox->setChecked(presetBuffer->stampLimitChecked);
    presetForm->fastLimitSpinBox->setValue(presetBuffer->fastLimitValue);
    presetForm->fastLimitCheckBox->setChecked(presetBuffer->fastLimitChecked);
    presetForm->fastTimeSpinBox->setValue(presetBuffer->fastTimeValue);
    presetForm->fastTimeCheckBox->setChecked(presetBuffer->fastTimeChecked);
    presetForm->slowLimitSpinBox->setValue(presetBuffer->slowLimitValue);
    presetForm->slowLimitCheckBox->setChecked(presetBuffer->slowLimitChecked);
    presetForm->slowTimeSpinBox->setValue(presetBuffer->slowTimeValue);
    presetForm->slowTimeCheckBox->setChecked(presetBuffer->slowTimeChecked);
    presetForm->vrmCurrentSpinBox->setValue(presetBuffer->vrmCurrentValue);
    presetForm->vrmCurrentCheckBox->setChecked(presetBuffer->vrmCurrentChecked);
    presetForm->vrmMaxSpinBox->setValue(presetBuffer->vrmMaxValue);
    presetForm->vrmMaxCheckBox->setChecked(presetBuffer->vrmMaxChecked);
    presetForm->minFclkSpinBox->setValue(presetBuffer->minFclkValue);
    presetForm->minFclkCheckBox->setChecked(presetBuffer->minFclkChecked);
    presetForm->maxFclkSpinBox->setValue(presetBuffer->maxFclkValue);
    presetForm->maxFclkCheckBox->setChecked(presetBuffer->maxFclkChecked);
    presetForm->minGfxclkSpinBox->setValue(presetBuffer->minGfxclkValue);
    presetForm->minGfxclkCheckBox->setChecked(presetBuffer->minGfxclkChecked);
    presetForm->maxGfxclkSpinBox->setValue(presetBuffer->maxGfxclkValue);
    presetForm->maxGfxclkCheckBox->setChecked(presetBuffer->maxGfxclkChecked);
    presetForm->minSocclkSpinBox->setValue(presetBuffer->minSocclkValue);
    presetForm->minSocclkCheckBox->setChecked(presetBuffer->minSocclkChecked);
    presetForm->maxSocclkSpinBox->setValue(presetBuffer->maxSocclkValue);
    presetForm->maxSocclkCheckBox->setChecked(presetBuffer->maxSocclkChecked);
    presetForm->minVcnSpinBox->setValue(presetBuffer->minVcnValue);
    presetForm->minVcnCheckBox->setChecked(presetBuffer->minVcnChecked);
    presetForm->maxVcnSpinBox->setValue(presetBuffer->maxVcnValue);
    presetForm->maxVcnCheckBox->setChecked(presetBuffer->maxVcnChecked);
    presetForm->smuMaxPerformanceCheckBox->setChecked(presetBuffer->smuMaxPerfomance);
    presetForm->smuPowerSavingCheckBox->setChecked(presetBuffer->smuPowerSaving);
    //NEW VARS
    presetForm->vrmSocCurrentSpinBox->setValue(presetBuffer->vrmSocCurrent);
    presetForm->vrmSocCurrentCheckBox->setChecked(presetBuffer->vrmSocCurrentChecked);
    presetForm->vrmSocMaxSpinBox->setValue(presetBuffer->vrmSocMax);
    presetForm->vrmSocMaxCheckBox->setChecked(presetBuffer->vrmSocMaxChecked);

    presetForm->psi0CurrentSpinBox->setValue(presetBuffer->psi0Current);
    presetForm->psi0CurrentCheckBox->setChecked(presetBuffer->psi0CurrentChecked);
    presetForm->psi0SocCurrentSpinBox->setValue(presetBuffer->psi0SocCurrent);
    presetForm->psi0SocCurrentCheckBox->setChecked(presetBuffer->psi0SocCurrentChecked);

    presetForm->maxLclkSpinBox->setValue(presetBuffer->maxLclk);
    presetForm->maxLclkCheckBox->setChecked(presetBuffer->maxLclkChecked);
    presetForm->minLclkSpinBox->setValue(presetBuffer->minLclk);
    presetForm->minLclkCheckBox->setChecked(presetBuffer->minLclkChecked);

    presetForm->prochotDeassertionRampSpinBox->setValue(presetBuffer->prochotDeassertionRamp);
    presetForm->prochotDeassertionRampCheckBox->setChecked(presetBuffer->prochotDeassertionRampChecked);

    presetForm->dgpuSkinTempLimitSpinBox->setValue(presetBuffer->dgpuSkinTempLimit);
    presetForm->dgpuSkinTempLimitCheckBox->setChecked(presetBuffer->dgpuSkinTempLimitChecked);
    presetForm->apuSlowLimitSpinBox->setValue(presetBuffer->apuSlowLimit);
    presetForm->apuSlowLimitCheckBox->setChecked(presetBuffer->apuSlowLimitChecked);
    presetForm->skinTempPowerLimitSpinBox->setValue(presetBuffer->skinTempPowerLimit);
    presetForm->skinTempPowerLimitCheckBox->setChecked(presetBuffer->skinTempPowerLimitChecked);
    //Set active
    for(int i = 0;i < tabWidgetsList->count();i++)
        tabWidgetsList->at(i)->setHidden(true);
    for(int i = 0;i < tabButtonList->count();i++)
        tabButtonList->at(i)->setChecked(false);
    widget->setHidden(false);
    button->setChecked(true);
    //Add items to settings
    ui_settings->dcStateComboBox->insertItem(idx, presetBuffer->presetName, idx);
    ui_settings->acStateComboBox->insertItem(idx, presetBuffer->presetName, idx);
    ui_settings->epmBatterySaverComboBox->insertItem(idx, presetBuffer->presetName, idx);
    ui_settings->epmBetterBatteryComboBox->insertItem(idx, presetBuffer->presetName, idx);
    ui_settings->epmBalancedComboBox->insertItem(idx, presetBuffer->presetName, idx);
    ui_settings->epmMaximumPerfomanceComboBox->insertItem(idx, presetBuffer->presetName, idx);
    ui_settings->epmGamingComboBox->insertItem(idx, presetBuffer->presetName, idx);
    //Add items to agent
    if(ui_agent != nullptr)
        ui_agent->addPresetButton(idx);
    /*
    Hide/Show elements by settings
    */
    settingsStr *settings = conf->getSettingsBuffer();
    hideShow *var = conf->hideShowWarnPresetVariable(
                settings->hideNotSupportedVariables
                ? settings->apuFamilyIdx : -1);

    presetForm->tempLimitWidget->setVisible(var->shwTctlTemp);
    presetForm->apuSkinWidget->setVisible(var->shwApuSkinTemp);
    presetForm->stampLimitWidget->setVisible(var->shwStapmLimit);
    presetForm->fastLimitWidget->setVisible(var->shwFastLimit);
    presetForm->slowLimitWidget->setVisible(var->shwSlowLimit);
    presetForm->slowTimeWidget->setVisible(var->shwSlowTime);
    presetForm->fastTimeWidget->setVisible(var->shwStapmTime);

    presetForm->vrmCurrentWidget->setVisible(var->shwVrmCurrent);
    presetForm->vrmMaxWidget->setVisible(var->shwVrmMaxCurrent);

    presetForm->minFclkWidget->setVisible(var->shwMinFclkFrequency);
    presetForm->maxFclkWidget->setVisible(var->shwMaxFclkFrequency);

    presetForm->minGfxclkWidget->setVisible(var->shwMinGfxclk);
    presetForm->maxGfxclkWidget->setVisible(var->shwMaxGfxclk);
    presetForm->minSocclkWidget->setVisible(var->shwMinSocclkFrequency);
    presetForm->maxSocclkWidget->setVisible(var->shwMaxSocclkFrequency);
    presetForm->minVcnWidget->setVisible(var->shwMinVcn);
    presetForm->maxVcnWidget->setVisible(var->shwMaxVcn);

    presetForm->smuMaxPerformanceCheckBox->setVisible(var->shwMaxPerformance);
    presetForm->smuPowerSavingCheckBox->setVisible(var->shwPowerSaving);

    presetForm->vrmSocCurrentWidget->setVisible(var->shwVrmSocCurrent);
    presetForm->vrmSocMaxWidget->setVisible(var->shwVrmSocMaxCurrent);

    presetForm->psi0CurrentWidget->setVisible(var->shwPsi0Current);
    presetForm->psi0SocCurrentWidget->setVisible(var->shwPsi0SocCurrent);

    presetForm->maxLclkWidget->setVisible(var->shwMaxLclk);
    presetForm->minLclkWidget->setVisible(var->shwMinLclk);

    presetForm->prochotDeassertionRampWidget->setVisible(var->shwProchotDeassertionRamp);

    presetForm->dgpuSkinTempLimitWidget->setVisible(var->shwDgpuSkinTemp);
    presetForm->apuSlowLimitWidget->setVisible(var->shwApuSlowLimit);
    presetForm->skinTempPowerLimitWidget->setVisible(var->shwSkinTempLimit);
    /*
    Hide/ShowShow Armour Plugin
    */
    presetForm->armourGroupBox->setVisible(settings->showArmourPlugin);
}

void CtrlGui::presetDeletePushButtonClicked() {
    int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();
    QMessageBox *deleteDialog;
    if(conf->getPresetsCount() != 1) {
        deleteDialog =
                new QMessageBox(QMessageBox::Question,
                                "Delete Preset",
                                "Do you really want to delete preset?");
        deleteDialog->addButton(QString::fromUtf8("&Yes"),
                                QMessageBox::AcceptRole);
        deleteDialog->addButton(QString::fromUtf8("&No"),
                                QMessageBox::RejectRole);
        deleteDialog->setModal(true);
        if (!(deleteDialog->exec() == QDialog::Accepted)){
            Ui::CtrlGuiAPUForm *presetForm;
            for(qsizetype x = 0;x < presetFormList->count();x++)
                if(presetFormList->at(x)->applyPushButton->property("idx") == idx){
                    presetForm = presetFormList->at(x);
                    break;
                }
            QPushButton *button;
            for(qsizetype x = 0;x < tabButtonList->count();x++)
                if(tabButtonList->at(x)->property("idx") == idx){
                    button = tabButtonList->at(x);
                    break;
                }
            QWidget *widget = nullptr;
            for(qsizetype x = 0;x < tabWidgetsList->count();x++)
                if(tabWidgetsList->at(x)->property("idx") == idx){
                    widget = tabWidgetsList->at(x);
                    break;
                }

            disconnect(button, &QPushButton::clicked, this, &CtrlGui::presetPushButtonClicked);
            disconnect(presetForm->deletePushButton, &QPushButton::clicked, this, &CtrlGui::presetDeletePushButtonClicked);
            disconnect(presetForm->presetNameEdit, &QLineEdit::textChanged, this, &CtrlGui::presetNameEditChanged);

            disconnect(presetForm->saveApplyPushButton, &QPushButton::clicked, this, &CtrlGui::saveApplyPreset);
            disconnect(presetForm->saveOnlyPushButton, &QPushButton::clicked, this, &CtrlGui::savePreset);
            disconnect(presetForm->applyPushButton, &QPushButton::clicked, this, &CtrlGui::applyPreset);
            disconnect(presetForm->cancelPushButton, &QPushButton::clicked, this, &CtrlGui::cancelPreset);

            disconnect(presetForm->smuMaxPerformanceCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);
            disconnect(presetForm->smuPowerSavingCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);

            ui->scrollAreaWidgetContents->layout()->removeWidget(button);
            verticalLayout->removeWidget(widget);

            tabButtonList->removeOne(button);
            presetFormList->removeOne(presetForm);
            tabWidgetsList->removeOne(widget);

            //send del command
            messageToServiceStr messageToService;
            messageToService.deletePreset = true;
            messageToService.preset = *conf->getPresetBuffer(idx);
            bus->sendMessageToService(messageToService);

            conf->deletePreset(idx);

            delete button;
            delete presetForm;
            delete widget;
            //Set active
            if(conf->getPresetsCount()>0){
                for(int i = 0;i < tabWidgetsList->count();i++)
                    tabWidgetsList->at(i)->setHidden(true);
                for(int i = 0;i < tabButtonList->count();i++)
                    tabButtonList->at(i)->setChecked(false);
                tabWidgetsList->at(0)->setHidden(false);
                tabButtonList->at(0)->setChecked(true);
            }
            //delete items from settings
            for(qsizetype i = 0;i < ui_settings->dcStateComboBox->count();i++)
                if(ui_settings->dcStateComboBox->itemData(i) == idx) {
                    if(ui_settings->dcStateComboBox->currentIndex() == idx) {
                        ui_settings->dcStateComboBox->setCurrentIndex(0);
                        ui_settings->savePushButton->click();
                    }
                    ui_settings->dcStateComboBox->removeItem(i);
                }
            for(qsizetype i = 0;i < ui_settings->acStateComboBox->count();i++)
                if(ui_settings->acStateComboBox->itemData(i) == idx) {
                    if(ui_settings->acStateComboBox->currentIndex() == idx) {
                        ui_settings->acStateComboBox->setCurrentIndex(0);
                        ui_settings->savePushButton->click();
                    }
                    ui_settings->acStateComboBox->removeItem(i);
                }
            for(qsizetype i = 0;i < ui_settings->epmBatterySaverComboBox->count();i++)
                if(ui_settings->epmBatterySaverComboBox->itemData(i) == idx) {
                    if(ui_settings->epmBatterySaverComboBox->currentIndex() == idx) {
                        ui_settings->epmBatterySaverComboBox->setCurrentIndex(0);
                        ui_settings->savePushButton->click();
                    }
                    ui_settings->epmBatterySaverComboBox->removeItem(i);
                }
            for(qsizetype i = 0;i < ui_settings->epmBetterBatteryComboBox->count();i++)
                if(ui_settings->epmBetterBatteryComboBox->itemData(i) == idx) {
                    if(ui_settings->epmBetterBatteryComboBox->currentIndex() == idx) {
                        ui_settings->epmBetterBatteryComboBox->setCurrentIndex(0);
                        ui_settings->savePushButton->click();
                    }
                    ui_settings->epmBetterBatteryComboBox->removeItem(i);
                }
            for(qsizetype i = 0;i < ui_settings->epmBalancedComboBox->count();i++)
                if(ui_settings->epmBalancedComboBox->itemData(i) == idx) {
                    if(ui_settings->epmBalancedComboBox->currentIndex() == idx) {
                        ui_settings->epmBalancedComboBox->setCurrentIndex(0);
                        ui_settings->savePushButton->click();
                    }
                    ui_settings->epmBalancedComboBox->removeItem(i);
                }
            for(qsizetype i = 0;i < ui_settings->epmMaximumPerfomanceComboBox->count();i++)
                if(ui_settings->epmMaximumPerfomanceComboBox->itemData(i) == idx) {
                    if(ui_settings->epmMaximumPerfomanceComboBox->currentIndex() == idx) {
                        ui_settings->epmMaximumPerfomanceComboBox->setCurrentIndex(0);
                        ui_settings->savePushButton->click();
                    }
                    ui_settings->epmMaximumPerfomanceComboBox->removeItem(i);
                }
            for(qsizetype i = 0;i < ui_settings->epmGamingComboBox->count();i++)
                if(ui_settings->epmGamingComboBox->itemData(i) == idx) {
                    if(ui_settings->epmGamingComboBox->currentIndex() == idx) {
                        ui_settings->epmGamingComboBox->setCurrentIndex(0);
                        ui_settings->savePushButton->click();
                    }
                    ui_settings->epmGamingComboBox->removeItem(i);
                }

            //Add items to agent
            if(ui_agent != nullptr)
                ui_agent->delPresetButton(idx);

            //Remove preset namo from label
            if (currentPresetId == idx) {
                currentPresetId = -1;
                ui->label->setText("RyzenCtrl");
            }
        }
    } else {
        deleteDialog =
                new QMessageBox(QMessageBox::Warning,
                                "Delete Preset",
                                "Unable to delete the last preset!");
        deleteDialog->addButton(QString::fromUtf8("&OK"),
                                QMessageBox::AcceptRole);
        deleteDialog->setModal(true);
        deleteDialog->exec();
    }
    delete deleteDialog;
}

void CtrlGui::presetNameEditChanged(QString name){
    int idx = reinterpret_cast<QLineEdit *>(sender())->property("idx").toInt();

    for(qsizetype i = 0;i < name.size() && i < (sizeof(conf->getPresetBuffer(idx)->presetName) - 1); i++)
        conf->getPresetBuffer(idx)->presetName[i] = name.at(i).toLatin1();
    conf->getPresetBuffer(idx)->presetName[name.size()] = '\0';

    for(int i = 0;i < tabButtonList->count();i++)
        if(tabButtonList->at(i)->property("idx").toInt() == idx)
            tabButtonList->at(i)->setText(name);

    for(qsizetype i = 0;i < conf->getPresetsCount();i++){
        if(ui_settings->dcStateComboBox->itemData(i) == idx)
            ui_settings->dcStateComboBox->setItemText(i, name);

        if(ui_settings->acStateComboBox->itemData(i) == idx)
            ui_settings->acStateComboBox->setItemText(i, name);

        if(ui_settings->epmBatterySaverComboBox->itemData(i) == idx)
            ui_settings->epmBatterySaverComboBox->setItemText(i, name);

        if(ui_settings->epmBetterBatteryComboBox->itemData(i) == idx)
            ui_settings->epmBetterBatteryComboBox->setItemText(i, name);

        if(ui_settings->epmBalancedComboBox->itemData(i) == idx)
            ui_settings->epmBalancedComboBox->setItemText(i, name);

        if(ui_settings->epmMaximumPerfomanceComboBox->itemData(i) == idx)
            ui_settings->epmMaximumPerfomanceComboBox->setItemText(i, name);

        if(ui_settings->epmGamingComboBox->itemData(i) == idx)
            ui_settings->epmGamingComboBox->setItemText(i, name);
    }
}

void CtrlGui::settingsAutomaticPresetSwitchClicked(){
    if (reinterpret_cast<QGroupBox *>(sender()) == ui_settings->epmAutoPresetSwitchGroupBox) {
        if(ui_settings->epmAutoPresetSwitchGroupBox->isChecked())
            ui_settings->acAutoPresetSwitchGroupBox->setChecked(false);
    }
    else {
        if(ui_settings->acAutoPresetSwitchGroupBox->isChecked())
            ui_settings->epmAutoPresetSwitchGroupBox->setChecked(false);
    }
}

void CtrlGui::openAdvancedInfoUrl(){
    QString link = "https://github.com/FlyGoat/RyzenAdj/wiki/Supported-Models";
    QDesktopServices::openUrl(QUrl(link));
}

void CtrlGui::recieveMessageToGui(messageToGuiStr messageToGui){
    currentPresetId = messageToGui.currentPresetId;
    bool saved = messageToGui.presetSaved;

    if(messageToGui.pmUpdated){
        ryzenFamily = messageToGui.pmTable.ryzenFamily;
        ui_infoWidget->ryzenFamily->setText("APU Ryzen " + ryzenFamily + " Family");
        biosVersion = QString::number(messageToGui.pmTable.biosVersion);
        ui_infoWidget->biosVersion->setText("BIOS Version: " + biosVersion);
        pmTableVersion = QString::number(messageToGui.pmTable.pmTableVersion, 16);
        ui_infoWidget->pmTableVersion->setText("PM Table Version: " + pmTableVersion);
        ryzenAdjVersion =
                QString::number(messageToGui.pmTable.ryzenAdjVersion) + "." +
                QString::number(messageToGui.pmTable.ryzenAdjMajorVersion) + "." +
                QString::number(messageToGui.pmTable.ryzenAdjMinorVersion);
        ui_infoWidget->ryzenAdjVersion->setText("RyzenAdj Version: " + ryzenAdjVersion);

        ui_infoWidget->stapm_limit->setText(QString::number(messageToGui.pmTable.stapm_limit) + " W");
        ui_infoWidget->stapm_value->setText(QString::number(messageToGui.pmTable.stapm_value) + " W");
        ui_infoWidget->fast_limit->setText(QString::number(messageToGui.pmTable.fast_limit) + " W");
        ui_infoWidget->fast_value->setText(QString::number(messageToGui.pmTable.fast_value) + " W");
        ui_infoWidget->slow_limit->setText(QString::number(messageToGui.pmTable.slow_limit) + " W");
        ui_infoWidget->slow_value->setText(QString::number(messageToGui.pmTable.slow_value) + " W");
        ui_infoWidget->apu_slow_limit->setText(QString::number(messageToGui.pmTable.apu_slow_limit) + " W");
        ui_infoWidget->apu_slow_value->setText(QString::number(messageToGui.pmTable.apu_slow_value) + " W");
        ui_infoWidget->vrm_current->setText(QString::number(messageToGui.pmTable.vrm_current) + " A");
        ui_infoWidget->vrm_current_value->setText(QString::number(messageToGui.pmTable.vrm_current_value) + " A");
        ui_infoWidget->vrmsoc_current->setText(QString::number(messageToGui.pmTable.vrmsoc_current) + " A");
        ui_infoWidget->vrmsoc_current_value->setText(QString::number(messageToGui.pmTable.vrmsoc_current_value) + " A");
        ui_infoWidget->vrmmax_current->setText(QString::number(messageToGui.pmTable.vrmmax_current) + " A");
        ui_infoWidget->vrmmax_current_value->setText(QString::number(messageToGui.pmTable.vrmmax_current_value) + " A");
        ui_infoWidget->vrmsocmax_current->setText(QString::number(messageToGui.pmTable.vrmsocmax_current) + " A");
        ui_infoWidget->vrmsocmax_current_value->setText(QString::number(messageToGui.pmTable.vrmsocmax_current_value) + " A");
        ui_infoWidget->tctl_temp->setText(QString::number(messageToGui.pmTable.tctl_temp) + " °C");
        ui_infoWidget->tctl_temp_value->setText(QString::number(messageToGui.pmTable.tctl_temp_value) + " °C");
        ui_infoWidget->apu_skin_temp_limit->setText(QString::number(messageToGui.pmTable.apu_skin_temp_limit) + " °C");
        ui_infoWidget->apu_skin_temp_value->setText(QString::number(messageToGui.pmTable.apu_skin_temp_value) + " °C");
        ui_infoWidget->dgpu_skin_temp_limit->setText(QString::number(messageToGui.pmTable.dgpu_skin_temp_limit) + " °C");
        ui_infoWidget->dgpu_skin_temp_value->setText(QString::number(messageToGui.pmTable.dgpu_skin_temp_value) + " °C");
        ui_infoWidget->stapm_time->setText(QString::number(messageToGui.pmTable.stapm_time) + " Sec.");
        ui_infoWidget->slow_time->setText(QString::number(messageToGui.pmTable.slow_time) + " Sec.");
        ui_infoWidget->psi0_current->setText(QString::number(messageToGui.pmTable.psi0_current) + " A");
        ui_infoWidget->psi0soc_current->setText(QString::number(messageToGui.pmTable.psi0soc_current) + " A");
    }

    if(currentPresetId != -1){
        qDebug()<<"Ctrl Gui - currentPresetId:"<<currentPresetId;
        QString message = conf->getPresetBuffer(currentPresetId)->presetName;
        if(saved)
            qDebug()<<"Ctrl Gui - saved:"<<saved;
        else
            message += (" NOT SAVED!");
        message += (" preset is runing now.");
        ui->label->setText("RyzenCtrl - " + message);

        if(ui_agent != nullptr
                && conf->getSettingsBuffer()->useAgent
                && conf->getSettingsBuffer()->showNotifications)
            ui_agent->notificationToTray(message);
        if(ui_agent != nullptr)
            ui_agent->setCurrentPresetId(currentPresetId);

    }
}

void CtrlGui::closeEvent(QCloseEvent *event) {
    if (conf->getSettingsBuffer()->useAgent){
        QEvent *ev = (QEvent*)event;
        ev->ignore();
        this->hide();
    }
}

void CtrlGui::useAgent(bool use){
    qDebug()<<"Ctrl Gui - CtrlAgent:"<<use;
    if(use){
        if(ui_agent == nullptr){
            qDebug()<<"Ctrl Gui - Create CtrlAgent";
            ui_agent = new CtrlAgent(conf);
            connect(ui_agent, &CtrlAgent::showCtrlGui, this, &CtrlGui::show);
            connect(ui_agent, &CtrlAgent::showCtrlInfoWidget, ui->infoPushButton, &QPushButton::clicked);
            connect(ui_agent, &CtrlAgent::closeCtrlGui, this, &CtrlGui::exitFromAgent);
            connect(ui_agent, &CtrlAgent::changePreset, this, &CtrlGui::presetChangeFromAgent);
        }
    } else {
        if(ui_agent != nullptr) {
            qDebug()<<"Ctrl Gui - Delete CtrlAgent";
            ui_agent->hide();
            disconnect(ui_agent, &CtrlAgent::showCtrlGui, this, &CtrlGui::show);
            disconnect(ui_agent, &CtrlAgent::showCtrlInfoWidget, ui->infoPushButton, &QPushButton::clicked);
            disconnect(ui_agent, &CtrlAgent::closeCtrlGui, this, &CtrlGui::exitFromAgent);
            disconnect(ui_agent, &CtrlAgent::changePreset, this, &CtrlGui::presetChangeFromAgent);
            delete ui_agent;
            ui_agent = nullptr;
        }
    }
}

void CtrlGui::exitFromAgent(){
    ui_agent->hide();
    disconnect(ui_agent, &CtrlAgent::showCtrlGui, this, &CtrlGui::show);
    disconnect(ui_agent, &CtrlAgent::showCtrlInfoWidget, ui->infoPushButton, &QPushButton::clicked);
    disconnect(ui_agent, &CtrlAgent::closeCtrlGui, this, &CtrlGui::exitFromAgent);
    disconnect(ui_agent, &CtrlAgent::changePreset, this, &CtrlGui::presetChangeFromAgent);
    delete ui_agent;
    exit(0);
}

void CtrlGui::presetChangeFromAgent(int idx){
    for(qsizetype x = 0;x < presetFormList->count();x++)
        if(presetFormList->at(x)->applyPushButton->property("idx") == idx){
            presetFormList->at(x)->saveApplyPushButton->click();
            break;
        }
}
