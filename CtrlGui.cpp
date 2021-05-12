#include "CtrlGui.h"
#include "ui_CtrlMainWindow.h"
#include "ui_CtrlAPUForm.h"
#include "ui_CtrlSettingsForm.h"
#include "ui_CtrlInfoWidget.h"
#include <QDebug>
#include <QFile>
#include <QDialogButtonBox>
#include <QXmlStreamWriter>
#include <QTimer>
#include <QMessageBox>
#include <QProcess>
#include <QObject>

#define bufferToGui_refresh_time 33

CtrlGui::CtrlGui(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf)
    : ui(new Ui::CtrlGui),
      ui_settings(new Ui::CtrlGuiSettings),
      ui_infoWidget(new Ui::CtrlInfoWidget),
      bufferToService(bufferToService),
      bufferToGui(bufferToGui),
      conf(conf)
{
    qtLanguageTranslator = new QTranslator;

    if(bufferToService->attach(QSharedMemory::ReadWrite))
        bufferToService->detach();
    else {
        qDebug() << "RyzenAdjCtrl Service is not runing!";
        startService();
    }

    setupUi();
    loadPresets();
    readSettings();
    setupConnections();
    loadStyleSheet();
    this->resize(824, 500);

    QTimer *bufferToService_refresh_timer = new QTimer;
    connect(bufferToService_refresh_timer, &QTimer::timeout,
            this, &CtrlGui::recieveArgs);
    bufferToService_refresh_timer->start(bufferToGui_refresh_time);
}

void CtrlGui::setupUi(){
    ui->setupUi(this);
    ui->comboBox->addItems(QStringList() << "en_US"<< "ru_RU");
    ui->comboBox->setHidden(true);

    for(int i = 0;i < 4;i++){
        apuForm[i] = new Ui::CtrlGuiAPUForm;
    }
    apuForm[0]->setupUi(ui->batteryTab);
    apuForm[1]->setupUi(ui->optimalTab);
    apuForm[2]->setupUi(ui->perfomanceTab);
    apuForm[3]->setupUi(ui->extremeTab);
    for(int i = 0;i < 4;i++){
        apuForm[i]->savePushButton->setProperty("idx",i);
        apuForm[i]->applyPushButton->setProperty("idx",i);
        apuForm[i]->cancelPushButton->setProperty("idx",i);

        apuForm[i]->fanComboBox->setProperty("idx",i);

        apuForm[i]->tempLimitSpinBox->setProperty("idx",i);
        apuForm[i]->tempLimitCheckBox->setProperty("idx",i);
        apuForm[i]->apuSkinSpinBox->setProperty("idx",i);
        apuForm[i]->apuSkinCheckBox->setProperty("idx",i);
        apuForm[i]->stampLimitSpinBox->setProperty("idx",i);
        apuForm[i]->stampLimitCheckBox->setProperty("idx",i);
        apuForm[i]->fastLimitSpinBox->setProperty("idx",i);
        apuForm[i]->fastLimitCheckBox->setProperty("idx",i);
        apuForm[i]->fastTimeSpinBox->setProperty("idx",i);
        apuForm[i]->fastTimeCheckBox->setProperty("idx",i);
        apuForm[i]->slowLimitSpinBox->setProperty("idx",i);
        apuForm[i]->slowLimitCheckBox->setProperty("idx",i);
        apuForm[i]->slowTimeSpinBox->setProperty("idx",i);
        apuForm[i]->slowTimeCheckBox->setProperty("idx",i);

        apuForm[i]->vrmCurrentSpinBox->setProperty("idx",i);
        apuForm[i]->vrmCurrentCheckBox->setProperty("idx",i);
        apuForm[i]->vrmMaxSpinBox->setProperty("idx",i);
        apuForm[i]->vrmMaxCheckBox->setProperty("idx",i);

        apuForm[i]->minFclkSpinBox->setProperty("idx",i);
        apuForm[i]->minFclkCheckBox->setProperty("idx",i);
        apuForm[i]->maxFclkSpinBox->setProperty("idx",i);
        apuForm[i]->maxFclkCheckBox->setProperty("idx",i);

        apuForm[i]->minGfxclkSpinBox->setProperty("idx",i);
        apuForm[i]->minGfxclkCheckBox->setProperty("idx",i);
        apuForm[i]->maxGfxclkSpinBox->setProperty("idx",i);
        apuForm[i]->maxGfxclkCheckBox->setProperty("idx",i);
        apuForm[i]->minSocclkSpinBox->setProperty("idx",i);
        apuForm[i]->minSocclkCheckBox->setProperty("idx",i);
        apuForm[i]->maxSocclkSpinBox->setProperty("idx",i);
        apuForm[i]->maxSocclkCheckBox->setProperty("idx",i);
        apuForm[i]->minVcnSpinBox->setProperty("idx",i);
        apuForm[i]->minVcnCheckBox->setProperty("idx",i);
        apuForm[i]->maxVcnSpinBox->setProperty("idx",i);
        apuForm[i]->maxVcnCheckBox->setProperty("idx",i);

        apuForm[i]->smuMaxPerformanceCheckBox->setProperty("idx",i);
        apuForm[i]->smuPowerSavingCheckBox->setProperty("idx",i);

        apuForm[i]->smuMaxPerformanceCheckBox->setProperty("idy",0);
        apuForm[i]->smuPowerSavingCheckBox->setProperty("idy",1);
    }

    ui_infoWidget = new Ui::CtrlInfoWidget;
    ui_infoWidget->setupUi(ui->infoWidget);
    ui->infoDockWidget->setHidden(true);

    settingFrame = new QFrame(nullptr, Qt::WindowType::Popup);
    settingFrame->setFrameStyle(QFrame::Panel);
    settingFrame->setAccessibleName("Settings");
    settingFrame->resize(1,1);
    ui_settings->setupUi(settingFrame);
    ui->tabWidget->setHidden(false);

    ui->batteryTab->setHidden(true);
    ui->extremeTab->setHidden(true);
    ui->perfomanceTab->setHidden(false);
    ui->optimalTab->setHidden(true);

    ui->batteryPushButton->setProperty("idx",0);
    ui->optimalPushButton->setProperty("idx",1);
    ui->perfomancePushButton->setProperty("idx",2);
    ui->extremePushButton->setProperty("idx",3);
}

void CtrlGui::setupConnections(){
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &CtrlGui::languageChange);
    connect(ui->infoPushButton, &QPushButton::clicked, this, &CtrlGui::infoPushButtonClicked);
    connect(ui->settingsPushButton, &QPushButton::clicked, this, &CtrlGui::settingsPushButtonClicked);
    connect(ui->rssPushButton, &QPushButton::clicked, this, &CtrlGui::loadStyleSheet);

    connect(ui->batteryPushButton, &QPushButton::clicked, this, &CtrlGui::presetPushButtonClicked);
    connect(ui->extremePushButton, &QPushButton::clicked, this, &CtrlGui::presetPushButtonClicked);
    connect(ui->perfomancePushButton, &QPushButton::clicked, this, &CtrlGui::presetPushButtonClicked);
    connect(ui->optimalPushButton, &QPushButton::clicked, this, &CtrlGui::presetPushButtonClicked);

    for(int i = 0;i < 4;i++){
        connect(apuForm[i]->savePushButton, &QPushButton::clicked, this, &CtrlGui::savePreset);
        connect(apuForm[i]->applyPushButton, &QPushButton::clicked, this, &CtrlGui::applyPreset);
        connect(apuForm[i]->cancelPushButton, &QPushButton::clicked, this, &CtrlGui::cancelPreset);

        connect(apuForm[i]->smuMaxPerformanceCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);
        connect(apuForm[i]->smuPowerSavingCheckBox, &QCheckBox::stateChanged, this, &CtrlGui::smuCheckBoxClicked);
    }

    connect(ui_settings->savePushButton, &QPushButton::clicked, this, &CtrlGui::saveSettings);
    connect(ui_settings->cancelPushButton, &QPushButton::clicked, this, &CtrlGui::cancelSettings);

    connect(ui_settings->epmAutoPresetSwitchGroupBox, &QGroupBox::clicked, this, &CtrlGui::settingsAutomaticPresetSwitchClicked);
    connect(ui_settings->acAutoPresetSwitchGroupBox, &QGroupBox::clicked, this, &CtrlGui::settingsAutomaticPresetSwitchClicked);
}

void CtrlGui::loadPresets(){
    presetStr **presetsBuffer = conf->presetsBuffer;
    for(int i = 0;i < 4;i++){
        apuForm[i]->fanComboBox->setCurrentIndex(presetsBuffer[i]->fanPresetId);

        apuForm[i]->tempLimitSpinBox->setValue(presetsBuffer[i]->tempLimitValue);
        apuForm[i]->tempLimitCheckBox->setChecked(presetsBuffer[i]->tempLimitChecked);
        apuForm[i]->apuSkinSpinBox->setValue(presetsBuffer[i]->apuSkinValue);
        apuForm[i]->apuSkinCheckBox->setChecked(presetsBuffer[i]->apuSkinChecked);
        apuForm[i]->stampLimitSpinBox->setValue(presetsBuffer[i]->stampLimitValue);
        apuForm[i]->stampLimitCheckBox->setChecked(presetsBuffer[i]->stampLimitChecked);
        apuForm[i]->fastLimitSpinBox->setValue(presetsBuffer[i]->fastLimitValue);
        apuForm[i]->fastLimitCheckBox->setChecked(presetsBuffer[i]->fastLimitChecked);
        apuForm[i]->fastTimeSpinBox->setValue(presetsBuffer[i]->fastTimeValue);
        apuForm[i]->fastTimeCheckBox->setChecked(presetsBuffer[i]->fastTimeChecked);
        apuForm[i]->slowLimitSpinBox->setValue(presetsBuffer[i]->slowLimitValue);
        apuForm[i]->slowLimitCheckBox->setChecked(presetsBuffer[i]->slowLimitChecked);
        apuForm[i]->slowTimeSpinBox->setValue(presetsBuffer[i]->slowTimeValue);
        apuForm[i]->slowTimeCheckBox->setChecked(presetsBuffer[i]->slowTimeChecked);

        apuForm[i]->vrmCurrentSpinBox->setValue(presetsBuffer[i]->vrmCurrentValue);
        apuForm[i]->vrmCurrentCheckBox->setChecked(presetsBuffer[i]->vrmCurrentChecked);
        apuForm[i]->vrmMaxSpinBox->setValue(presetsBuffer[i]->vrmMaxValue);
        apuForm[i]->vrmMaxCheckBox->setChecked(presetsBuffer[i]->vrmMaxChecked);

        apuForm[i]->minFclkSpinBox->setValue(presetsBuffer[i]->minFclkValue);
        apuForm[i]->minFclkCheckBox->setChecked(presetsBuffer[i]->minFclkChecked);
        apuForm[i]->maxFclkSpinBox->setValue(presetsBuffer[i]->maxFclkValue);
        apuForm[i]->maxFclkCheckBox->setChecked(presetsBuffer[i]->maxFclkChecked);

        apuForm[i]->minGfxclkSpinBox->setValue(presetsBuffer[i]->minGfxclkValue);
        apuForm[i]->minGfxclkCheckBox->setChecked(presetsBuffer[i]->minGfxclkChecked);
        apuForm[i]->maxGfxclkSpinBox->setValue(presetsBuffer[i]->maxGfxclkValue);
        apuForm[i]->maxGfxclkCheckBox->setChecked(presetsBuffer[i]->maxGfxclkChecked);
        apuForm[i]->minSocclkSpinBox->setValue(presetsBuffer[i]->minSocclkValue);
        apuForm[i]->minSocclkCheckBox->setChecked(presetsBuffer[i]->minSocclkChecked);
        apuForm[i]->maxSocclkSpinBox->setValue(presetsBuffer[i]->maxSocclkValue);
        apuForm[i]->maxSocclkCheckBox->setChecked(presetsBuffer[i]->maxSocclkChecked);
        apuForm[i]->minVcnSpinBox->setValue(presetsBuffer[i]->minVcnValue);
        apuForm[i]->minVcnCheckBox->setChecked(presetsBuffer[i]->minVcnChecked);
        apuForm[i]->maxVcnSpinBox->setValue(presetsBuffer[i]->maxVcnValue);
        apuForm[i]->maxVcnCheckBox->setChecked(presetsBuffer[i]->maxVcnChecked);

        apuForm[i]->smuMaxPerformanceCheckBox->setChecked(presetsBuffer[i]->smuMaxPerfomance);
        apuForm[i]->smuPowerSavingCheckBox->setChecked(presetsBuffer[i]->smuPowerSaving);
    }
}

void CtrlGui::loadStyleSheet(){
    QFile configQFile("StyleSheet.xml");
    configQFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader configReader;
    configReader.setDevice(&configQFile);
    configReader.readNext();
    while(!configReader.atEnd())
    {
        //
        if (configReader.name() == QString("MainWindow"))
            foreach(const QXmlStreamAttribute &attr, configReader.attributes()){
                if (attr.name().toString() == "value") {
                    QString strStyleSheet = attr.value().toString();
                    this->setStyleSheet(strStyleSheet);
                    settingFrame->setStyleSheet(strStyleSheet);
                    //ui_infoWidget->setStyleSheet(strStyleSheet);
                }
            }else{}

        if (configReader.name() == QString("TopWidget"))
            foreach(const QXmlStreamAttribute &attr, configReader.attributes()){
                if (attr.name().toString() == "value")
                    ui->topwidget->setStyleSheet(attr.value().toString());
            }else{}

        if (configReader.name() == QString("TabWidget"))
            foreach(const QXmlStreamAttribute &attr, configReader.attributes()){
                if (attr.name().toString() == "value")
                    ui->tabbtnwidget->setStyleSheet(attr.value().toString());
            }else{}
        //
        configReader.readNext();
    }
    configQFile.close();
}

void CtrlGui::savePreset(){
    ui->label->setText("RyzenAdjCtrl - Applying...");
    int i = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    if(!infoMessageShowed){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("For better experience:"
                       "\nDisable auto switcher in settings.");
        msgBox.exec();
        infoMessageShowed = true;
        conf->settingsBuffer->showNotificationToDisableAutoSwitcher = true;
        conf->saveSettings();
    }

    presetStr **presetsBuffer = conf->presetsBuffer;

    presetsBuffer[i]->fanPresetId = apuForm[i]->fanComboBox->currentIndex();

    presetsBuffer[i]->tempLimitValue = apuForm[i]->tempLimitSpinBox->value();
    presetsBuffer[i]->tempLimitChecked = apuForm[i]->tempLimitCheckBox->isChecked();
    presetsBuffer[i]->apuSkinValue = apuForm[i]->apuSkinSpinBox->value();
    presetsBuffer[i]->apuSkinChecked = apuForm[i]->apuSkinCheckBox->isChecked();
    presetsBuffer[i]->stampLimitValue = apuForm[i]->stampLimitSpinBox->value();
    presetsBuffer[i]->stampLimitChecked = apuForm[i]->stampLimitCheckBox->isChecked();
    presetsBuffer[i]->fastLimitValue = apuForm[i]->fastLimitSpinBox->value();
    presetsBuffer[i]->fastLimitChecked = apuForm[i]->fastLimitCheckBox->isChecked();
    presetsBuffer[i]->fastTimeValue = apuForm[i]->fastTimeSpinBox->value();
    presetsBuffer[i]->fastTimeChecked = apuForm[i]->fastTimeCheckBox->isChecked();
    presetsBuffer[i]->slowLimitValue = apuForm[i]->slowLimitSpinBox->value();
    presetsBuffer[i]->slowLimitChecked = apuForm[i]->slowLimitCheckBox->isChecked();
    presetsBuffer[i]->slowTimeValue = apuForm[i]->slowTimeSpinBox->value();
    presetsBuffer[i]->slowTimeChecked = apuForm[i]->slowTimeCheckBox->isChecked();

    presetsBuffer[i]->vrmCurrentValue = apuForm[i]->vrmCurrentSpinBox->value();
    presetsBuffer[i]->vrmCurrentChecked = apuForm[i]->vrmCurrentCheckBox->isChecked();
    presetsBuffer[i]->vrmMaxValue = apuForm[i]->vrmMaxSpinBox->value();
    presetsBuffer[i]->vrmMaxChecked = apuForm[i]->vrmMaxCheckBox->isChecked();

    presetsBuffer[i]->minFclkValue = apuForm[i]->minFclkSpinBox->value();
    presetsBuffer[i]->minFclkChecked = apuForm[i]->minFclkCheckBox->isChecked();
    presetsBuffer[i]->maxFclkValue = apuForm[i]->maxFclkSpinBox->value();
    presetsBuffer[i]->maxFclkChecked = apuForm[i]->maxFclkCheckBox->isChecked();

    presetsBuffer[i]->minGfxclkValue = apuForm[i]->minGfxclkSpinBox->value();
    presetsBuffer[i]->minGfxclkChecked = apuForm[i]->minGfxclkCheckBox->isChecked();
    presetsBuffer[i]->maxGfxclkValue = apuForm[i]->maxGfxclkSpinBox->value();
    presetsBuffer[i]->maxGfxclkChecked = apuForm[i]->maxGfxclkCheckBox->isChecked();

    presetsBuffer[i]->minSocclkValue = apuForm[i]->minSocclkSpinBox->value();
    presetsBuffer[i]->minSocclkChecked = apuForm[i]->minSocclkCheckBox->isChecked();
    presetsBuffer[i]->maxSocclkValue = apuForm[i]->maxSocclkSpinBox->value();
    presetsBuffer[i]->maxSocclkChecked = apuForm[i]->maxSocclkCheckBox->isChecked();

    presetsBuffer[i]->minVcnValue = apuForm[i]->minVcnSpinBox->value();
    presetsBuffer[i]->minVcnChecked = apuForm[i]->minVcnCheckBox->isChecked();
    presetsBuffer[i]->maxVcnValue = apuForm[i]->maxVcnSpinBox->value();
    presetsBuffer[i]->maxVcnChecked = apuForm[i]->maxVcnCheckBox->isChecked();

    presetsBuffer[i]->smuMaxPerfomance = apuForm[i]->smuMaxPerformanceCheckBox->isChecked();
    presetsBuffer[i]->smuPowerSaving = apuForm[i]->smuPowerSavingCheckBox->isChecked();

    conf->savePresets();

    sendPreset(i, true);
}

void CtrlGui::applyPreset(){
    ui->label->setText("RyzenAdjCtrl - Applying...");
    int i = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    if(!infoMessageShowed){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("For better experience:"
                       "\nDisable auto switcher in settings.");
        msgBox.exec();
        infoMessageShowed = true;
        conf->settingsBuffer->showNotificationToDisableAutoSwitcher = true;
        conf->saveSettings();
    }

    sendPreset(i, false);
}

void CtrlGui::cancelPreset(){
    ui->label->setText("RyzenAdjCtrl - Applying...");
    int i = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();
    presetStr **presetsBuffer = conf->presetsBuffer;

    apuForm[i]->fanComboBox->setCurrentIndex(presetsBuffer[i]->fanPresetId);

    apuForm[i]->tempLimitSpinBox->setValue(presetsBuffer[i]->tempLimitValue);
    apuForm[i]->tempLimitCheckBox->setChecked(presetsBuffer[i]->tempLimitChecked);
    apuForm[i]->apuSkinSpinBox->setValue(presetsBuffer[i]->apuSkinValue);
    apuForm[i]->apuSkinCheckBox->setChecked(presetsBuffer[i]->apuSkinChecked);
    apuForm[i]->stampLimitSpinBox->setValue(presetsBuffer[i]->stampLimitValue);
    apuForm[i]->stampLimitCheckBox->setChecked(presetsBuffer[i]->stampLimitChecked);
    apuForm[i]->fastLimitSpinBox->setValue(presetsBuffer[i]->fastLimitValue);
    apuForm[i]->fastLimitCheckBox->setChecked(presetsBuffer[i]->fastLimitChecked);
    apuForm[i]->fastTimeSpinBox->setValue(presetsBuffer[i]->fastTimeValue);
    apuForm[i]->fastTimeCheckBox->setChecked(presetsBuffer[i]->fastTimeChecked);
    apuForm[i]->slowLimitSpinBox->setValue(presetsBuffer[i]->slowLimitValue);
    apuForm[i]->slowLimitCheckBox->setChecked(presetsBuffer[i]->slowLimitChecked);
    apuForm[i]->slowTimeSpinBox->setValue(presetsBuffer[i]->slowTimeValue);
    apuForm[i]->slowTimeCheckBox->setChecked(presetsBuffer[i]->slowTimeChecked);

    apuForm[i]->vrmCurrentSpinBox->setValue(presetsBuffer[i]->vrmCurrentValue);
    apuForm[i]->vrmCurrentCheckBox->setChecked(presetsBuffer[i]->vrmCurrentChecked);
    apuForm[i]->vrmMaxSpinBox->setValue(presetsBuffer[i]->vrmMaxValue);
    apuForm[i]->vrmMaxCheckBox->setChecked(presetsBuffer[i]->vrmMaxChecked);

    apuForm[i]->minFclkSpinBox->setValue(presetsBuffer[i]->minFclkValue);
    apuForm[i]->minFclkCheckBox->setChecked(presetsBuffer[i]->minFclkChecked);
    apuForm[i]->maxFclkSpinBox->setValue(presetsBuffer[i]->maxFclkValue);
    apuForm[i]->maxFclkCheckBox->setChecked(presetsBuffer[i]->maxFclkChecked);

    apuForm[i]->minGfxclkSpinBox->setValue(presetsBuffer[i]->minGfxclkValue);
    apuForm[i]->minGfxclkCheckBox->setChecked(presetsBuffer[i]->minGfxclkChecked);
    apuForm[i]->maxGfxclkSpinBox->setValue(presetsBuffer[i]->maxGfxclkValue);
    apuForm[i]->maxGfxclkCheckBox->setChecked(presetsBuffer[i]->maxGfxclkChecked);
    apuForm[i]->minSocclkSpinBox->setValue(presetsBuffer[i]->minSocclkValue);
    apuForm[i]->minSocclkCheckBox->setChecked(presetsBuffer[i]->minSocclkChecked);
    apuForm[i]->maxSocclkSpinBox->setValue(presetsBuffer[i]->maxSocclkValue);
    apuForm[i]->maxSocclkCheckBox->setChecked(presetsBuffer[i]->maxSocclkChecked);
    apuForm[i]->minVcnSpinBox->setValue(presetsBuffer[i]->minVcnValue);
    apuForm[i]->minVcnCheckBox->setChecked(presetsBuffer[i]->minVcnChecked);
    apuForm[i]->maxVcnSpinBox->setValue(presetsBuffer[i]->maxVcnValue);
    apuForm[i]->maxVcnCheckBox->setChecked(presetsBuffer[i]->maxVcnChecked);

    apuForm[i]->smuMaxPerformanceCheckBox->setChecked(presetsBuffer[i]->smuMaxPerfomance);
    apuForm[i]->smuPowerSavingCheckBox->setChecked(presetsBuffer[i]->smuPowerSaving);

    sendPreset(i, false);
}

void CtrlGui::sendPreset(int i, bool save){
    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToService");
    //
        if(save) {
            argsWriter.writeStartElement("save");
            argsWriter.writeEndElement();
        }
        argsWriter.writeStartElement("id");
            argsWriter.writeAttribute("value", QString::number(i));
        argsWriter.writeEndElement();

        if(apuForm[i]->tempLimitCheckBox->isChecked()) {
            argsWriter.writeStartElement("tempLimitValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->tempLimitSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->apuSkinCheckBox->isChecked()) {
            argsWriter.writeStartElement("apuSkinValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->apuSkinSpinBox->value()));
            argsWriter.writeEndElement();
        }

        if(apuForm[i]->stampLimitCheckBox->isChecked()) {
            argsWriter.writeStartElement("stampLimitValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->stampLimitSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->fastLimitCheckBox->isChecked()) {
            argsWriter.writeStartElement("fastLimitValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->fastLimitSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->fastTimeCheckBox->isChecked()) {
            argsWriter.writeStartElement("fastTimeValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->fastTimeSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->slowLimitCheckBox->isChecked()) {
            argsWriter.writeStartElement("slowLimitValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->slowLimitSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->slowTimeCheckBox->isChecked()) {
            argsWriter.writeStartElement("slowTimeValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->slowTimeSpinBox->value()));
            argsWriter.writeEndElement();
        }

        if(apuForm[i]->vrmCurrentCheckBox->isChecked()) {
            argsWriter.writeStartElement("vrmCurrentValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->vrmCurrentSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->vrmMaxCheckBox->isChecked()) {
            argsWriter.writeStartElement("vrmMaxValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->vrmMaxSpinBox->value()));
            argsWriter.writeEndElement();
        }

        if(apuForm[i]->minFclkCheckBox->isChecked()) {
            argsWriter.writeStartElement("minFclkValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->minFclkSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->maxFclkCheckBox->isChecked()) {
            argsWriter.writeStartElement("maxFclkValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->maxFclkSpinBox->value()));
            argsWriter.writeEndElement();
        }

        if(apuForm[i]->minGfxclkCheckBox->isChecked()) {
            argsWriter.writeStartElement("minGfxclkValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->minGfxclkSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->maxGfxclkCheckBox->isChecked()) {
            argsWriter.writeStartElement("maxGfxclkValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->maxGfxclkSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->minSocclkCheckBox->isChecked()) {
            argsWriter.writeStartElement("minSocclkValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->minSocclkSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->maxSocclkCheckBox->isChecked()) {
            argsWriter.writeStartElement("maxSocclkValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->maxSocclkSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->minVcnCheckBox->isChecked()) {
            argsWriter.writeStartElement("minVcnValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->minVcnSpinBox->value()));
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->maxVcnCheckBox->isChecked()) {
            argsWriter.writeStartElement("maxVcnValue");
                argsWriter.writeAttribute("value", QString::number(apuForm[i]->maxVcnSpinBox->value()));
            argsWriter.writeEndElement();
        }

        if(apuForm[i]->smuMaxPerformanceCheckBox->isChecked()) {
            argsWriter.writeStartElement("tempLimitValue");
            argsWriter.writeEndElement();
        }
        if(apuForm[i]->smuPowerSavingCheckBox->isChecked()) {
            argsWriter.writeStartElement("tempLimitValue");
            argsWriter.writeEndElement();
        }

        argsWriter.writeStartElement("fanPresetId");
            argsWriter.writeAttribute("value", QString::number(apuForm[i]->fanComboBox->currentIndex()));
        argsWriter.writeEndElement();
    //
    argsWriter.writeEndElement();
    argsWriter.writeEndDocument();

    sendArgsToService(data);
}

void CtrlGui::smuCheckBoxClicked(){
    int idx = reinterpret_cast<QCheckBox *>(sender())->property("idx").toInt();
    int idy = reinterpret_cast<QCheckBox *>(sender())->property("idy").toInt();

    if(idy == 0) {
        if(apuForm[idx]->smuMaxPerformanceCheckBox->isChecked())
            apuForm[idx]->smuPowerSavingCheckBox->setChecked(false);
    } else {
        if(apuForm[idx]->smuPowerSavingCheckBox->isChecked())
            apuForm[idx]->smuMaxPerformanceCheckBox->setChecked(false);
    }
}

CtrlGui::~CtrlGui()
{
    disconnect(ui->comboBox, &QComboBox::currentIndexChanged, this, &CtrlGui::languageChange);
    delete ui;
}

void CtrlGui::languageChange()
{
    QString idx = reinterpret_cast<QComboBox *>(sender())->currentText();
    if(qtLanguageTranslator->load(QString("Language/CtrlGui_") + idx, ".")){
        qApp->installTranslator(qtLanguageTranslator);
        ui->retranslateUi(this);
        apuForm[0]->retranslateUi(ui->batteryTab);
        apuForm[1]->retranslateUi(ui->optimalTab);
        apuForm[2]->retranslateUi(ui->perfomanceTab);
        apuForm[3]->retranslateUi(ui->extremeTab);
    } else {
        qApp->removeTranslator(qtLanguageTranslator);
        ui->retranslateUi(this);
        apuForm[0]->retranslateUi(ui->batteryTab);
        apuForm[1]->retranslateUi(ui->optimalTab);
        apuForm[2]->retranslateUi(ui->perfomanceTab);
        apuForm[3]->retranslateUi(ui->extremeTab);
    }
}

void CtrlGui::saveSettings(){
    settingFrame->hide();
    settingsStr* settings = conf->settingsBuffer;

    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToService");

    //
    if(settings->useAgent != ui_settings->useAgentGroupBox->isChecked())
        settings->useAgent = ui_settings->useAgentGroupBox->isChecked();
    if(settings->showNotifications != ui_settings->showNotificationsCheckBox->isChecked())
        settings->showNotifications = ui_settings->showNotificationsCheckBox->isChecked();

    if(settings->autoPresetApplyDurationChecked != ui_settings->reapplyDurationGroupBox->isChecked()){
        settings->autoPresetApplyDurationChecked = ui_settings->reapplyDurationGroupBox->isChecked();
        argsWriter.writeStartElement("autoPresetApplyDurationChecked");
            argsWriter.writeAttribute("value", QString::number(settings->autoPresetApplyDurationChecked));
        argsWriter.writeEndElement();
    }
    if(settings->autoPresetApplyDuration != ui_settings->reapplyDurationSpinBox->value()){
        settings->autoPresetApplyDuration = ui_settings->reapplyDurationSpinBox->value();
        argsWriter.writeStartElement("autoPresetApplyDuration");
            argsWriter.writeAttribute("value", QString::number(settings->autoPresetApplyDuration));
        argsWriter.writeEndElement();
    }

    if(settings->autoPresetSwitchAC != ui_settings->acAutoPresetSwitchGroupBox->isChecked()){
        settings->autoPresetSwitchAC = ui_settings->acAutoPresetSwitchGroupBox->isChecked();
        argsWriter.writeStartElement("autoPresetSwitchAC");
            argsWriter.writeAttribute("value", QString::number(settings->autoPresetSwitchAC));
        argsWriter.writeEndElement();
    }
    if(settings->dcStatePresetId != ui_settings->dcStateComboBox->currentIndex()){
        settings->dcStatePresetId = ui_settings->dcStateComboBox->currentIndex();
        argsWriter.writeStartElement("dcStatePresetId");
            argsWriter.writeAttribute("value", QString::number(settings->dcStatePresetId));
        argsWriter.writeEndElement();
    }
    if(settings->acStatePresetId != ui_settings->acStateComboBox->currentIndex()){
        settings->acStatePresetId = ui_settings->acStateComboBox->currentIndex();
        argsWriter.writeStartElement("acStatePresetId");
            argsWriter.writeAttribute("value", QString::number(settings->acStatePresetId));
        argsWriter.writeEndElement();
    }

    if(settings->epmAutoPresetSwitch != ui_settings->epmAutoPresetSwitchGroupBox->isChecked()){
        settings->epmAutoPresetSwitch = ui_settings->epmAutoPresetSwitchGroupBox->isChecked();
        argsWriter.writeStartElement("epmAutoPresetSwitch");
            argsWriter.writeAttribute("value", QString::number(settings->epmAutoPresetSwitch));
        argsWriter.writeEndElement();
    }
    if(settings->epmBatterySaverPresetId != ui_settings->epmBatterySaverComboBox->currentIndex()){
        settings->epmBatterySaverPresetId = ui_settings->epmBatterySaverComboBox->currentIndex();
        argsWriter.writeStartElement("epmBatterySaverPresetId");
            argsWriter.writeAttribute("value", QString::number(settings->epmBatterySaverPresetId));
        argsWriter.writeEndElement();
    }
    if(settings->epmBetterBatteryPresetId != ui_settings->epmBetterBatteryComboBox->currentIndex()){
        settings->epmBetterBatteryPresetId = ui_settings->epmBetterBatteryComboBox->currentIndex();
        argsWriter.writeStartElement("epmBetterBatteryPresetId");
            argsWriter.writeAttribute("value", QString::number(settings->epmBetterBatteryPresetId));
        argsWriter.writeEndElement();
    }
    if(settings->epmBalancedPresetId != ui_settings->epmBalancedComboBox->currentIndex()){
        settings->epmBalancedPresetId = ui_settings->epmBalancedComboBox->currentIndex();
        argsWriter.writeStartElement("epmBalancedPresetId");
            argsWriter.writeAttribute("value", QString::number(settings->epmBalancedPresetId));
        argsWriter.writeEndElement();
    }
    if(settings->epmMaximumPerfomancePresetId != ui_settings->epmMaximumPerfomanceComboBox->currentIndex()){
        settings->epmMaximumPerfomancePresetId = ui_settings->epmMaximumPerfomanceComboBox->currentIndex();
        argsWriter.writeStartElement("epmMaximumPerfomancePresetId");
            argsWriter.writeAttribute("value", QString::number(settings->epmMaximumPerfomancePresetId));
        argsWriter.writeEndElement();
    }
    //

    argsWriter.writeEndElement();
    argsWriter.writeEndDocument();

    sendArgsToService(data);

    conf->saveSettings();
}

void CtrlGui::readSettings(){
    settingsStr *settings = conf->settingsBuffer;

    ui_settings->useAgentGroupBox->setChecked(settings->useAgent);
    ui_settings->showNotificationsCheckBox->setChecked(settings->showNotifications);

    infoMessageShowed = settings->showNotificationToDisableAutoSwitcher;

    ui->rssPushButton->setVisible(settings->showReloadStyleSheetButton);

    ui_settings->reapplyDurationGroupBox->setChecked(settings->autoPresetApplyDurationChecked);
    ui_settings->reapplyDurationSpinBox->setValue(settings->autoPresetApplyDuration);

    ui_settings->acAutoPresetSwitchGroupBox->setChecked(settings->autoPresetSwitchAC);
    ui_settings->dcStateComboBox->setCurrentIndex(settings->dcStatePresetId);
    ui_settings->acStateComboBox->setCurrentIndex(settings->acStatePresetId);

    ui_settings->epmAutoPresetSwitchGroupBox->setChecked(settings->epmAutoPresetSwitch);
    ui_settings->epmBatterySaverComboBox->setCurrentIndex(settings->epmBatterySaverPresetId);
    ui_settings->epmBetterBatteryComboBox->setCurrentIndex(settings->epmBetterBatteryPresetId);
    ui_settings->epmBalancedComboBox->setCurrentIndex(settings->epmBalancedPresetId);
    ui_settings->epmMaximumPerfomanceComboBox->setCurrentIndex(settings->epmMaximumPerfomancePresetId);
}

void CtrlGui::cancelSettings(){
    settingFrame->hide();
    readSettings();
}

void CtrlGui::startService(){
    QProcess process;
    QString runas = ("" + qApp->arguments().value(0) + " startup");
    process.startDetached("powershell", QStringList({"start-process", runas, "-verb", "runas"}));
}

void CtrlGui::sendArgsToService(QByteArray arguments){
    if(bufferToService->attach(QSharedMemory::ReadWrite)){
        char *iodata = (char*)bufferToService->data();
        if (bufferToService->lock()) {
            for (int i=0;i<arguments.size();i++)
                iodata[i] = arguments[i];
            bufferToService->unlock();
        }
        bufferToService->detach();
    } else {
        ui->label->setText("RyzenAdjCtrl - The service is not running. Please run the service or restart RyzenAdjCtrl.");
        qDebug() << "The service is not running. Please run the service or restart RyzenAdjCtrl.";
    }
}

void CtrlGui::infoPushButtonClicked() {
    if(ui->infoDockWidget->isHidden()){
        ui->infoDockWidget->setHidden(false);
        sendRyzenAdjInfo(ui_infoWidget->spinBox->value());
        ui_infoWidget->spinBox->connect(ui_infoWidget->spinBox, &QSpinBox::valueChanged,
                                        this, &CtrlGui::sendRyzenAdjInfo);
    } else {
        ui->infoDockWidget->setHidden(true);
        sendRyzenAdjInfo(0);
    }
}

void CtrlGui::sendRyzenAdjInfo(int value){
    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToService");
    //
        argsWriter.writeStartElement("ryzenAdjInfoTimeout");
            argsWriter.writeAttribute("value", QString::number(value));
        argsWriter.writeEndElement();
    //
    argsWriter.writeEndElement();
    argsWriter.writeEndDocument();

    sendArgsToService(data);
}

void CtrlGui::settingsPushButtonClicked() {
    if (settingFrame->geometry().width()<10) {
        settingFrame->show();
        settingFrame->hide();
    }

    QRect rect = settingFrame->geometry();
    const QRect buttonGeometry = ui->settingsPushButton->geometry();
    const QRect windowGeometry = this->geometry();
    int height = rect.height();
    int width = rect.width();
    rect.setX(buttonGeometry.left()+windowGeometry.left()+buttonGeometry.width()-width);
    rect.setY(buttonGeometry.top()+windowGeometry.top());
    rect.setWidth(width);
    rect.setHeight(height);
    settingFrame->setGeometry(rect);

    if (settingFrame->isHidden()) settingFrame->show();
}

void CtrlGui::presetPushButtonClicked(){
    int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();
    switch (idx) {
    case 0:
        ui->batteryPushButton->setChecked(true);
        ui->optimalPushButton->setChecked(false);
        ui->perfomancePushButton->setChecked(false);
        ui->extremePushButton->setChecked(false);

        ui->optimalTab->setHidden(true);
        ui->perfomanceTab->setHidden(true);
        ui->extremeTab->setHidden(true);

        ui->batteryTab->setHidden(false);
        break;
    case 1:
        ui->batteryPushButton->setChecked(false);
        ui->optimalPushButton->setChecked(true);
        ui->perfomancePushButton->setChecked(false);
        ui->extremePushButton->setChecked(false);

        ui->batteryTab->setHidden(true);
        ui->perfomanceTab->setHidden(true);
        ui->extremeTab->setHidden(true);

        ui->optimalTab->setHidden(false);
        break;
    case 2:
        ui->batteryPushButton->setChecked(false);
        ui->optimalPushButton->setChecked(false);
        ui->perfomancePushButton->setChecked(true);
        ui->extremePushButton->setChecked(false);

        ui->batteryTab->setHidden(true);
        ui->optimalTab->setHidden(true);
        ui->extremeTab->setHidden(true);

        ui->perfomanceTab->setHidden(false);
        break;
    case 3:
        ui->batteryPushButton->setChecked(false);
        ui->optimalPushButton->setChecked(false);
        ui->perfomancePushButton->setChecked(false);
        ui->extremePushButton->setChecked(true);

        ui->batteryTab->setHidden(true);
        ui->optimalTab->setHidden(true);
        ui->perfomanceTab->setHidden(true);

        ui->extremeTab->setHidden(false);
        break;
    default:
        ui->batteryPushButton->setChecked(true);
        ui->optimalPushButton->setChecked(false);
        ui->perfomancePushButton->setChecked(false);
        ui->extremePushButton->setChecked(false);

        ui->optimalTab->setHidden(true);
        ui->perfomanceTab->setHidden(true);
        ui->extremeTab->setHidden(true);

        ui->batteryTab->setHidden(false);
        break;
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

void CtrlGui::recieveArgs(){
    QByteArray arguments;
    if(bufferToGui->attach(QSharedMemory::ReadWrite)){
        if (bufferToGui->lock())
        {
            char *iodata = (char*)bufferToGui->data();
            for (int i=0;iodata[i];i++) {
                arguments.append(iodata[i]);
                iodata[i] = '\0';
            }
            bufferToGui->unlock();
        }
        bufferToGui->detach();
    }
    if(arguments.size() > 0)
        decodeArgs(arguments);
}

void CtrlGui::decodeArgs(QByteArray args){
    int currentPresetId = -1;
    bool saved = false;
    QString ryzenAdjInfo;

    QXmlStreamReader argsReader(args);
    argsReader.readNext();
    while(!argsReader.atEnd())
    {
        //
        if (argsReader.name() == QString("currentPresetId"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    currentPresetId = attr.value().toString().toInt();
                    qDebug()<<"currentPresetId:"<<currentPresetId;
                }
            }else{}
        if (argsReader.name() == QString("saved"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    saved = attr.value().toString().toInt();
                    qDebug()<<"saved:"<<saved;
                }
            }else{}

        if (argsReader.name() == QString("ryzenFamily"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    if(ryzenFamily != attr.value().toString()){
                        ryzenFamily = attr.value().toString();
                        ui_infoWidget->ryzenFamily->setText("APU Ryzen " + ryzenFamily + " Family");
                    }
            }else{}
        if (argsReader.name() == QString("biosVersion"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    if(biosVersion != attr.value().toString()){
                        biosVersion = attr.value().toString();
                        ui_infoWidget->biosVersion->setText("BIOS Version: " + biosVersion);
                    }
            }else{}
        if (argsReader.name() == QString("pmTableVersion"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    if(pmTableVersion != attr.value().toString()){
                        pmTableVersion = attr.value().toString();
                        ui_infoWidget->pmTableVersion->setText("PM Table Version: " + pmTableVersion);
                    }
            }else{}
        if (argsReader.name() == QString("ryzenAdjVersion"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    if(ryzenAdjVersion != attr.value().toString()){
                        ryzenAdjVersion = attr.value().toString();
                        ui_infoWidget->ryzenAdjVersion->setText("RyzenAdj Version: " + ryzenAdjVersion);
                    }
            }else{}

        if (argsReader.name() == QString("stapm_limit"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->stapm_limit->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("stapm_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->stapm_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("fast_limit"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->fast_limit->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("fast_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->fast_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("slow_limit"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->slow_limit->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("slow_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->slow_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("apu_slow_limit"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->apu_slow_limit->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("apu_slow_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->apu_slow_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrm_current"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrm_current->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrm_current_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrm_current_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrmsoc_current"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrmsoc_current->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrmsoc_current_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrmsoc_current_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrmmax_current"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrmmax_current->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrmmax_current_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrmmax_current_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrmsocmax_current"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrmsocmax_current->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("vrmsocmax_current_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->vrmsocmax_current_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("tctl_temp"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->tctl_temp->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("tctl_temp_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->tctl_temp_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("apu_skin_temp_limit"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->apu_skin_temp_limit->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("apu_skin_temp_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->apu_skin_temp_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("dgpu_skin_temp_limit"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->dgpu_skin_temp_limit->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("dgpu_skin_temp_value"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->dgpu_skin_temp_value->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("stapm_time"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->stapm_time->setText(attr.value().toString());
            }else{}
        if (argsReader.name() == QString("slow_time"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    ui_infoWidget->slow_time->setText(attr.value().toString());
            }else{}
        //
        argsReader.readNext();
    }

    if(currentPresetId != -1){
        QString message;
        if (saved)
            message = (conf->presetsBuffer[currentPresetId]->presetName
                       + " is runing now.");
        else
            message = (conf->presetsBuffer[currentPresetId]->presetName
                       + " NOT SAVED! is runing now.");
        ui->label->setText("RyzenAdjCtrl - " + message);
        emit messageToAgent(message);
    }
}
