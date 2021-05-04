#include "RyzenAdjGui.h"
#include "ui_RyzenAdjGui.h"
#include "ui_RyzenAdjGuiAPUForm.h"
#include "ui_RyzenAdjGuiSettings.h"
#include "RyzenAdjMain.cpp"
#include <QDebug>
#include <QFile>
#include <QDialogButtonBox>

RyzenAdjGui::RyzenAdjGui(QSharedMemory *bufferToService, RyzenAdjCfg *conf)
    : ui(new Ui::RyzenAdjGui),
      ui_settings(new Ui::RyzenAdjGuiSettings),
      bufferToService(bufferToService),
      conf(conf)
{
    qtLanguageTranslator = new QTranslator;

    if(bufferToService->attach(QSharedMemory::ReadWrite))
        bufferToService->detach();
    else {
        errorMessage("The service is not running.");
    }

    setupUi();
    loadPresets();
    readSettings();
    setupConnections();
    loadStyleSheet();
}

void RyzenAdjGui::setupUi(){
    ui->setupUi(this);
    ui->comboBox->addItems(QStringList() << "en_US"<< "ru_RU");
    ui->comboBox->setHidden(true);

    for(int i = 0;i < 4;i++){
        apuForm[i] = new Ui::RyzenAdjGuiAPUForm;
    }
    apuForm[0]->setupUi(ui->batteryTab);
    apuForm[1]->setupUi(ui->optimalTab);
    apuForm[2]->setupUi(ui->perfomanceTab);
    apuForm[3]->setupUi(ui->extremeTab);
    for(int i = 0;i < 4;i++){
        apuForm[i]->saveButton->setProperty("idx",i);
        apuForm[i]->tryButton->setProperty("idx",i);

        apuForm[i]->cmdOutputLineEdit->setProperty("idx",i);

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

    ui_settings->setupUi(ui->settingsWidget);
    ui->settingsWidget->setHidden(true);
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

void RyzenAdjGui::setupConnections(){
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &RyzenAdjGui::languageChange);
    connect(ui->settingsPushButton, &QPushButton::clicked, this, &RyzenAdjGui::settingsPushButtonClicked);
    connect(ui->rssPushButton, &QPushButton::clicked, this, &RyzenAdjGui::loadStyleSheet);

    connect(ui->batteryPushButton, &QPushButton::clicked, this, &RyzenAdjGui::presetPushButtonClicked);
    connect(ui->extremePushButton, &QPushButton::clicked, this, &RyzenAdjGui::presetPushButtonClicked);
    connect(ui->perfomancePushButton, &QPushButton::clicked, this, &RyzenAdjGui::presetPushButtonClicked);
    connect(ui->optimalPushButton, &QPushButton::clicked, this, &RyzenAdjGui::presetPushButtonClicked);

    for(int i = 0;i < 4;i++){
        connect(apuForm[i]->saveButton, &QPushButton::clicked, this, &RyzenAdjGui::savePreset);
        connect(apuForm[i]->tryButton, &QPushButton::clicked, this, &RyzenAdjGui::tryPreset);

        connect(apuForm[i]->fanComboBox, &QComboBox::currentIndexChanged, this, &RyzenAdjGui::presetVariableChanged);

        connect(apuForm[i]->apuSkinSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);

        connect(apuForm[i]->apuSkinSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->apuSkinCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);

        connect(apuForm[i]->tempLimitSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->tempLimitCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->apuSkinSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->apuSkinCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->stampLimitSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->stampLimitCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->fastLimitSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->fastLimitCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->fastTimeSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->fastTimeCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->slowLimitSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->slowLimitCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->slowTimeSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->slowTimeCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);

        connect(apuForm[i]->vrmCurrentSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->vrmCurrentCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->vrmMaxSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->vrmMaxCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);

        connect(apuForm[i]->minFclkSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->minFclkCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxFclkSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxFclkCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);

        connect(apuForm[i]->minGfxclkSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->minGfxclkCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxGfxclkSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxGfxclkCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->minSocclkSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->minSocclkCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxSocclkSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxSocclkCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->minVcnSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->minVcnCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxVcnSpinBox, &QSpinBox::valueChanged, this, &RyzenAdjGui::presetVariableChanged);
        connect(apuForm[i]->maxVcnCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::presetVariableChanged);

        connect(apuForm[i]->smuMaxPerformanceCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::smuCheckBoxClicked);
        connect(apuForm[i]->smuPowerSavingCheckBox, &QCheckBox::stateChanged, this, &RyzenAdjGui::smuCheckBoxClicked);
    }

    connect(ui_settings->sStartPushButton, &QPushButton::clicked, this, &RyzenAdjGui::startService);
    connect(ui_settings->sStopPushButton, &QPushButton::clicked, this, &RyzenAdjGui::stopService);

    connect(ui_settings->savePushButton, &QPushButton::clicked, this, &RyzenAdjGui::saveSettings);
    connect(ui_settings->cancelPushButton, &QPushButton::clicked, this, &RyzenAdjGui::readSettings);
}

void RyzenAdjGui::loadPresets(){
    presetStr *presetsBuffer = conf->getPresets();
    for(int i = 0;i < 4;i++){
        apuForm[i]->cmdOutputLineEdit->setText(presetsBuffer[i].cmdOutputValue);

        apuForm[i]->fanComboBox->setCurrentIndex(presetsBuffer[i].fanPresetId);

        apuForm[i]->tempLimitSpinBox->setValue(presetsBuffer[i].tempLimitValue);
        apuForm[i]->tempLimitCheckBox->setChecked(presetsBuffer[i].tempLimitChecked);
        apuForm[i]->apuSkinSpinBox->setValue(presetsBuffer[i].apuSkinValue);
        apuForm[i]->apuSkinCheckBox->setChecked(presetsBuffer[i].apuSkinChecked);
        apuForm[i]->stampLimitSpinBox->setValue(presetsBuffer[i].stampLimitValue);
        apuForm[i]->stampLimitCheckBox->setChecked(presetsBuffer[i].stampLimitChecked);
        apuForm[i]->fastLimitSpinBox->setValue(presetsBuffer[i].fastLimitValue);
        apuForm[i]->fastLimitCheckBox->setChecked(presetsBuffer[i].fastLimitChecked);
        apuForm[i]->fastTimeSpinBox->setValue(presetsBuffer[i].fastTimeValue);
        apuForm[i]->fastTimeCheckBox->setChecked(presetsBuffer[i].fastTimeChecked);
        apuForm[i]->slowLimitSpinBox->setValue(presetsBuffer[i].slowLimitValue);
        apuForm[i]->slowLimitCheckBox->setChecked(presetsBuffer[i].slowLimitChecked);
        apuForm[i]->slowTimeSpinBox->setValue(presetsBuffer[i].slowTimeValue);
        apuForm[i]->slowTimeCheckBox->setChecked(presetsBuffer[i].slowTimeChecked);

        apuForm[i]->vrmCurrentSpinBox->setValue(presetsBuffer[i].vrmCurrentValue);
        apuForm[i]->vrmCurrentCheckBox->setChecked(presetsBuffer[i].vrmCurrentChecked);
        apuForm[i]->vrmMaxSpinBox->setValue(presetsBuffer[i].vrmMaxValue);
        apuForm[i]->vrmMaxCheckBox->setChecked(presetsBuffer[i].vrmMaxChecked);

        apuForm[i]->minFclkSpinBox->setValue(presetsBuffer[i].minFclkValue);
        apuForm[i]->minFclkCheckBox->setChecked(presetsBuffer[i].minFclkChecked);
        apuForm[i]->maxFclkSpinBox->setValue(presetsBuffer[i].maxFclkValue);
        apuForm[i]->maxFclkCheckBox->setChecked(presetsBuffer[i].maxFclkChecked);

        apuForm[i]->minGfxclkSpinBox->setValue(presetsBuffer[i].minGfxclkValue);
        apuForm[i]->minGfxclkCheckBox->setChecked(presetsBuffer[i].minGfxclkChecked);
        apuForm[i]->maxGfxclkSpinBox->setValue(presetsBuffer[i].maxGfxclkValue);
        apuForm[i]->maxGfxclkCheckBox->setChecked(presetsBuffer[i].maxGfxclkChecked);
        apuForm[i]->minSocclkSpinBox->setValue(presetsBuffer[i].minSocclkValue);
        apuForm[i]->minSocclkCheckBox->setChecked(presetsBuffer[i].minSocclkChecked);
        apuForm[i]->maxSocclkSpinBox->setValue(presetsBuffer[i].maxSocclkValue);
        apuForm[i]->maxSocclkCheckBox->setChecked(presetsBuffer[i].maxSocclkChecked);
        apuForm[i]->minVcnSpinBox->setValue(presetsBuffer[i].minVcnValue);
        apuForm[i]->minVcnCheckBox->setChecked(presetsBuffer[i].minVcnChecked);
        apuForm[i]->maxVcnSpinBox->setValue(presetsBuffer[i].maxVcnValue);
        apuForm[i]->maxVcnCheckBox->setChecked(presetsBuffer[i].maxVcnChecked);

        apuForm[i]->smuMaxPerformanceCheckBox->setChecked(presetsBuffer[i].smuMaxPerfomance);
        apuForm[i]->smuPowerSavingCheckBox->setChecked(presetsBuffer[i].smuPowerSaving);
    }
}

void RyzenAdjGui::loadStyleSheet(){
    QFile configQFile("StyleSheet.xml");
    configQFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader configReader;
    configReader.setDevice(&configQFile);
    configReader.readNext();
    while(!configReader.atEnd())
    {
        //
        if (configReader.name() == QString("MainWindow"))
            foreach(const QXmlStreamAttribute &attr, configReader.attributes())
                if (attr.name().toString() == "value")
                    this->setStyleSheet(attr.value().toString());

        if (configReader.name() == QString("TopWidget"))
            foreach(const QXmlStreamAttribute &attr, configReader.attributes())
                if (attr.name().toString() == "value")
                    ui->topwidget->setStyleSheet(attr.value().toString());;

        if (configReader.name() == QString("TabWidget"))
            foreach(const QXmlStreamAttribute &attr, configReader.attributes())
                if (attr.name().toString() == "value")
                    ui->tabbtnwidget->setStyleSheet(attr.value().toString());
        //
        configReader.readNext();
    }
    configQFile.close();
}

void RyzenAdjGui::savePreset(){
    int i = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();
    presetStr *presetsBuffer = conf->getPresets();

    presetsBuffer[i].cmdOutputValue = apuForm[i]->cmdOutputLineEdit->text();

    presetsBuffer[i].fanPresetId = apuForm[i]->fanComboBox->currentIndex();

    presetsBuffer[i].tempLimitValue = apuForm[i]->tempLimitSpinBox->value();
    presetsBuffer[i].tempLimitChecked = apuForm[i]->tempLimitCheckBox->isChecked();
    presetsBuffer[i].apuSkinValue = apuForm[i]->apuSkinSpinBox->value();
    presetsBuffer[i].apuSkinChecked = apuForm[i]->apuSkinCheckBox->isChecked();
    presetsBuffer[i].stampLimitValue = apuForm[i]->stampLimitSpinBox->value();
    presetsBuffer[i].stampLimitChecked = apuForm[i]->stampLimitCheckBox->isChecked();
    presetsBuffer[i].fastLimitValue = apuForm[i]->fastLimitSpinBox->value();
    presetsBuffer[i].fastLimitChecked = apuForm[i]->fastLimitCheckBox->isChecked();
    presetsBuffer[i].fastTimeValue = apuForm[i]->fastTimeSpinBox->value();
    presetsBuffer[i].fastTimeChecked = apuForm[i]->fastTimeCheckBox->isChecked();
    presetsBuffer[i].slowLimitValue = apuForm[i]->slowLimitSpinBox->value();
    presetsBuffer[i].slowLimitChecked = apuForm[i]->slowLimitCheckBox->isChecked();
    presetsBuffer[i].slowTimeValue = apuForm[i]->slowTimeSpinBox->value();
    presetsBuffer[i].slowTimeChecked = apuForm[i]->slowTimeCheckBox->isChecked();

    presetsBuffer[i].vrmCurrentValue = apuForm[i]->vrmCurrentSpinBox->value();
    presetsBuffer[i].vrmCurrentChecked = apuForm[i]->vrmCurrentCheckBox->isChecked();
    presetsBuffer[i].vrmMaxValue = apuForm[i]->vrmMaxSpinBox->value();
    presetsBuffer[i].vrmMaxChecked = apuForm[i]->vrmMaxCheckBox->isChecked();

    presetsBuffer[i].minFclkValue = apuForm[i]->minFclkSpinBox->value();
    presetsBuffer[i].minFclkChecked = apuForm[i]->minFclkCheckBox->isChecked();
    presetsBuffer[i].maxFclkValue = apuForm[i]->maxFclkSpinBox->value();
    presetsBuffer[i].maxFclkChecked = apuForm[i]->maxFclkCheckBox->isChecked();

    presetsBuffer[i].minGfxclkValue = apuForm[i]->minGfxclkSpinBox->value();
    presetsBuffer[i].minGfxclkChecked = apuForm[i]->minGfxclkCheckBox->isChecked();
    presetsBuffer[i].maxGfxclkValue = apuForm[i]->maxGfxclkSpinBox->value();
    presetsBuffer[i].maxGfxclkChecked = apuForm[i]->maxGfxclkCheckBox->isChecked();

    presetsBuffer[i].minSocclkValue = apuForm[i]->minSocclkSpinBox->value();
    presetsBuffer[i].minSocclkChecked = apuForm[i]->minSocclkCheckBox->isChecked();
    presetsBuffer[i].maxSocclkValue = apuForm[i]->maxSocclkSpinBox->value();
    presetsBuffer[i].maxSocclkChecked = apuForm[i]->maxSocclkCheckBox->isChecked();

    presetsBuffer[i].minVcnValue = apuForm[i]->minVcnSpinBox->value();
    presetsBuffer[i].minVcnChecked = apuForm[i]->minVcnCheckBox->isChecked();
    presetsBuffer[i].maxVcnValue = apuForm[i]->maxVcnSpinBox->value();
    presetsBuffer[i].maxVcnChecked = apuForm[i]->maxVcnCheckBox->isChecked();

    presetsBuffer[i].smuMaxPerfomance = apuForm[i]->smuMaxPerformanceCheckBox->isChecked();
    presetsBuffer[i].smuPowerSaving = apuForm[i]->smuPowerSavingCheckBox->isChecked();

    conf->savePresets();
}

void RyzenAdjGui::tryPreset(){
    int i = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();

    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToService");
    //
        argsWriter.writeStartElement("id");
            argsWriter.writeAttribute("value", QString::number(i));
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("ryzenAdjCmdLine");
            argsWriter.writeAttribute("value", apuForm[i]->cmdOutputLineEdit->text());
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("fanPresetId");
            argsWriter.writeAttribute("value", QString::number(apuForm[i]->fanComboBox->currentIndex()));
        argsWriter.writeEndElement();
    //
    argsWriter.writeEndElement();
    argsWriter.writeEndDocument();

    sendArgs(data);
}

void RyzenAdjGui::presetVariableChanged(){
    int i = reinterpret_cast<QObject *>(sender())->property("idx").toInt();
    apuForm[i]->cmdOutputLineEdit->clear();

    if(apuForm[i]->tempLimitCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--tctl-temp=" +
                        QString::number(apuForm[i]->tempLimitSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->apuSkinCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--apu-skin-temp=" +
                        QString::number(apuForm[i]->apuSkinSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->stampLimitCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--stapm-limit=" +
                        QString::number(apuForm[i]->stampLimitSpinBox->value() * 1000)
                         + " "
                        )
                    );
    if(apuForm[i]->fastLimitCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--fast-limit=" +
                        QString::number(apuForm[i]->fastLimitSpinBox->value() * 1000)
                         + " "
                        )
                    );
    if(apuForm[i]->fastTimeCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--stapm-time=" +
                        QString::number(apuForm[i]->fastTimeSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->slowLimitCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--slow-limit=" +
                        QString::number(apuForm[i]->slowLimitSpinBox->value() * 1000)
                         + " "
                        )
                    );
    if(apuForm[i]->slowTimeCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--slow-time=" +
                        QString::number(apuForm[i]->slowTimeSpinBox->value())
                         + " "
                        )
                    );

    if(apuForm[i]->vrmCurrentCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--vrm-current=" +
                        QString::number(apuForm[i]->vrmCurrentSpinBox->value() * 1000)
                         + " "
                        )
                    );
    if(apuForm[i]->vrmMaxCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--vrmmax-current=" +
                        QString::number(apuForm[i]->vrmMaxSpinBox->value() * 1000)
                         + " "
                        )
                    );

    if(apuForm[i]->minFclkCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--min-fclk-frequency=" +
                        QString::number(apuForm[i]->minFclkSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->maxFclkCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--max-fclk-frequency=" +
                        QString::number(apuForm[i]->maxFclkSpinBox->value())
                         + " "
                        )
                    );

    if(apuForm[i]->minGfxclkCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--min-gfxclk=" +
                        QString::number(apuForm[i]->minGfxclkSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->maxGfxclkCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--max-gfxclk=" +
                        QString::number(apuForm[i]->maxGfxclkSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->minSocclkCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--min-socclk-frequency=" +
                        QString::number(apuForm[i]->minSocclkSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->maxSocclkCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--max-socclk-frequency=" +
                        QString::number(apuForm[i]->maxSocclkSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->maxVcnCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--max-vcn=" +
                        QString::number(apuForm[i]->maxVcnSpinBox->value())
                         + " "
                        )
                    );
    if(apuForm[i]->minVcnCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append(
                        "--min-vcn=" +
                        QString::number(apuForm[i]->minVcnSpinBox->value())
                         + " "
                        )
                    );

    if(apuForm[i]->smuMaxPerformanceCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append("--max-performance "));
    if(apuForm[i]->smuPowerSavingCheckBox->isChecked())
        apuForm[i]->cmdOutputLineEdit->setText(
                    apuForm[i]->cmdOutputLineEdit->text().append("--power-saving"));
}

void RyzenAdjGui::smuCheckBoxClicked(){
    int idx = reinterpret_cast<QCheckBox *>(sender())->property("idx").toInt();
    int idy = reinterpret_cast<QCheckBox *>(sender())->property("idy").toInt();

    if(idy == 0) {
        if(apuForm[idx]->smuMaxPerformanceCheckBox->isChecked())
            apuForm[idx]->smuPowerSavingCheckBox->setChecked(false);
    } else {
        if(apuForm[idx]->smuPowerSavingCheckBox->isChecked())
            apuForm[idx]->smuMaxPerformanceCheckBox->setChecked(false);
    }

    presetVariableChanged();
}

RyzenAdjGui::~RyzenAdjGui()
{
    disconnect(ui->comboBox, &QComboBox::currentIndexChanged, this, &RyzenAdjGui::languageChange);
    delete ui;
}

void RyzenAdjGui::languageChange()
{
    QString idx = reinterpret_cast<QComboBox *>(sender())->currentText();
    if(qtLanguageTranslator->load(QString("Language/RyzenAdjGui_") + idx, ".")){
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

void RyzenAdjGui::saveSettings(){
    settingsStr* settings = conf->getSettings();

    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToService");

    //
    if(settings->autoPresetApplyDurationChecked != ui_settings->autoPresetApplyDurationCheckBox->isChecked()){
        settings->autoPresetApplyDurationChecked = ui_settings->autoPresetApplyDurationCheckBox->isChecked();
        argsWriter.writeStartElement("autoPresetApplyDurationChecked");
            argsWriter.writeAttribute("value", QString::number(settings->autoPresetApplyDurationChecked));
        argsWriter.writeEndElement();
    }
    if(settings->autoPresetApplyDuration != ui_settings->autoPresetApplyDurationSpinBox->value()){
        settings->autoPresetApplyDuration = ui_settings->autoPresetApplyDurationSpinBox->value();
        argsWriter.writeStartElement("autoPresetApplyDuration");
            argsWriter.writeAttribute("value", QString::number(settings->autoPresetApplyDuration));
        argsWriter.writeEndElement();
    }

    if(settings->settingsReloadDurationChecked != ui_settings->settingsReloadDurationCheckBox->isChecked()){
        settings->settingsReloadDurationChecked = ui_settings->settingsReloadDurationCheckBox->isChecked();
        argsWriter.writeStartElement("settingsReloadDurationChecked");
            argsWriter.writeAttribute("value", QString::number(settings->settingsReloadDurationChecked));
        argsWriter.writeEndElement();
    }
    if(settings->settingsReloadDuration != ui_settings->settingsReloadDurationSpinBox->value()){
        settings->settingsReloadDuration = ui_settings->settingsReloadDurationSpinBox->value();
        argsWriter.writeStartElement("settingsReloadDuration");
            argsWriter.writeAttribute("value", QString::number(settings->settingsReloadDuration));
        argsWriter.writeEndElement();
    }

    if(settings->autoPresetSwitchAC != ui_settings->autoPresetSwitchCheckBox->isChecked()){
        settings->autoPresetSwitchAC = ui_settings->autoPresetSwitchCheckBox->isChecked();
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
    //

    argsWriter.writeEndElement();
    argsWriter.writeEndDocument();

    sendArgs(data);

    conf->saveSettings();
}

void RyzenAdjGui::readSettings(){
    settingsStr *settings = conf->getSettings();

    ui->rssPushButton->setVisible(settings->showReloadStyleSheetButton);

    ui_settings->autoPresetApplyDurationCheckBox->setChecked(settings->autoPresetApplyDurationChecked);
    ui_settings->autoPresetApplyDurationSpinBox->setValue(settings->autoPresetApplyDuration);

    ui_settings->settingsReloadDurationCheckBox->setChecked(settings->settingsReloadDurationChecked);
    ui_settings->settingsReloadDurationSpinBox->setValue(settings->settingsReloadDuration);

    ui_settings->autoPresetSwitchCheckBox->setChecked(settings->autoPresetSwitchAC);
    ui_settings->dcStateComboBox->setCurrentIndex(settings->dcStatePresetId);
    ui_settings->acStateComboBox->setCurrentIndex(settings->acStatePresetId);
}

void RyzenAdjGui::installService(){}

void RyzenAdjGui::removeService(){}

void RyzenAdjGui::startService(){
    QProcess process;
    QString runas = ("" + qApp->arguments().value(0) + " startup");
    qDebug()<<runas;
    process.startDetached("powershell", QStringList({"start-process", runas, "-verb", "runas"}));
}

void RyzenAdjGui::stopService(){
    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToService");
    //
        argsWriter.writeStartElement("exit");
        argsWriter.writeEndElement();
    //
    argsWriter.writeEndElement();
    argsWriter.writeEndDocument();
    sendArgs(data);
}

void RyzenAdjGui::sendArgs(QByteArray arguments){
    if(bufferToService->attach(QSharedMemory::ReadWrite)){
        char *iodata = (char*)bufferToService->data();
        if (bufferToService->lock()) {
            for (int i=0;i<arguments.size();i++)
                iodata[i] = arguments[i];
            bufferToService->unlock();
        }
        bufferToService->detach();
    } else
        errorMessage("The service is not running.");
}

void RyzenAdjGui::settingsPushButtonClicked() {
    if(!ui->settingsWidget->isVisible()){
        ui->tabWidget->setHidden(true);
        ui->settingsWidget->setHidden(false);
        ui->settingsPushButton->setChecked(true);
    } else {
        ui->tabWidget->setHidden(false);
        ui->settingsWidget->setHidden(true);
        ui->settingsPushButton->setChecked(false);
    }
}

void RyzenAdjGui::presetPushButtonClicked(){
    int idx = reinterpret_cast<QPushButton *>(sender())->property("idx").toInt();
    switch (idx) {
    case 0:
        ui->batteryPushButton->setChecked(true);
        ui->optimalPushButton->setChecked(false);
        ui->perfomancePushButton->setChecked(false);
        ui->extremePushButton->setChecked(false);

        ui->batteryTab->setHidden(false);
        ui->optimalTab->setHidden(true);
        ui->perfomanceTab->setHidden(true);
        ui->extremeTab->setHidden(true);
        break;
    case 1:
        ui->batteryPushButton->setChecked(false);
        ui->optimalPushButton->setChecked(true);
        ui->perfomancePushButton->setChecked(false);
        ui->extremePushButton->setChecked(false);

        ui->batteryTab->setHidden(true);
        ui->optimalTab->setHidden(false);
        ui->perfomanceTab->setHidden(true);
        ui->extremeTab->setHidden(true);
        break;
    case 2:
        ui->batteryPushButton->setChecked(false);
        ui->optimalPushButton->setChecked(false);
        ui->perfomancePushButton->setChecked(true);
        ui->extremePushButton->setChecked(false);

        ui->batteryTab->setHidden(true);
        ui->optimalTab->setHidden(true);
        ui->perfomanceTab->setHidden(false);
        ui->extremeTab->setHidden(true);
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

        ui->batteryTab->setHidden(false);
        ui->optimalTab->setHidden(true);
        ui->perfomanceTab->setHidden(true);
        ui->extremeTab->setHidden(true);
        break;
        break;
    }
}
