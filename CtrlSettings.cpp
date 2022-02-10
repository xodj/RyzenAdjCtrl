#include "CtrlSettings.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

CtrlSettings::CtrlSettings()
    : presets(new QList<presetStr*>),
#ifdef WIN32
      configQFile(new QFile("Config/Config.xml")),
      presetsQFile(new QFile("Config/Presets.xml"))
#else //WIN32
      configQFile(new QFile("/etc/RyzenCtrl/Config.xml")),
      presetsQFile(new QFile("/etc/RyzenCtrl/Presets.xml"))
#endif //WIN32
{
    qDebug() << "Ctrl Settings - Started";
}

CtrlSettings::~CtrlSettings() {
    qDebug() << "Ctrl Settings - Desroyed";
}

void CtrlSettings::checkSettings() {
    if (!configQFile->exists()){
        qDebug() << "Ctrl Settings - Create New Settings File.";
        saveSettings();
    } else
        openSettings();

    if (!presetsQFile->exists()) {
        qDebug() << "Ctrl Settings - Create New Presets File.";
        char presetNames[5][PresetNameLenth] = {
            {'B', 'a', 't', 't', 'e', 'r', 'y', ' ', 'S', 'a', 'v', 'e', 'r', '\0'},
            {'B', 'e', 't', 't', 'e', 'r', ' ', 'B', 'a', 't', 't', 'e', 'r', 'y', '\0'},
            {'B', 'a', 'l', 'a', 'n', 'c', 'e', 'd', '\0'},
            {'P', 'e', 'r', 'f', 'o', 'm', 'a', 'n', 'c', 'e', '\0'},
            {'G', 'a', 'm', 'i', 'n', 'g', '\0'}
        };
        presetStr *preset;
        for(int i = 0;i < 5; i++){
            preset = new presetStr;
            preset->presetId = i;
            for(int y = 0;y <= sizeof(presetNames[i]); y++)
                preset->presetName[y] = presetNames[i][y];
            presets->append(preset);
        }
        savePresets();
    }
    else openPresets();
}

bool CtrlSettings::saveSettings() {
    configQFile->open(QIODevice::WriteOnly);
    QXmlStreamWriter xmlWriter(configQFile);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("Settings");
    //
        xmlWriter.writeStartElement("useAgent");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.useAgent));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("showNotifications");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.showNotifications));
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("lastUsedPMTableUpdateInterval");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.lastUsedPMTableUpdateInterval));
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("showNotificationToDisableAutoSwitcher");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.showNotificationToDisableAutoSwitcher));
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("autoPresetApplyDurationChecked");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.autoPresetApplyDurationChecked));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("autoPresetApplyDuration");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.autoPresetApplyDuration));
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("autoPresetSwitchAC");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.autoPresetSwitchAC));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("dcStatePresetId");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.dcStatePresetId));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("acStatePresetId");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.acStatePresetId));
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("epmAutoPresetSwitch");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.epmAutoPresetSwitch));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("epmBatterySaverPresetId");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.epmBatterySaverPresetId));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("epmBetterBatteryPresetId");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.epmBetterBatteryPresetId));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("epmBalancedPresetId");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.epmBalancedPresetId));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("epmMaximumPerfomancePresetId");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.epmMaximumPerfomancePresetId));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("epmGamingPresetId");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.epmGamingPresetId));
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("hideNotSupportedVariables");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.hideNotSupportedVariables));
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("apuFamilyIdx");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.apuFamilyIdx));
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("showArmourPlugin");
            xmlWriter.writeAttribute("value", QString::number(settingsBuffer.showArmourPlugin));
        xmlWriter.writeEndElement();
    //
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    configQFile->close();
    qDebug() << "Ctrl Settings - Saved";
    return true;
}

bool CtrlSettings::openSettings(){
    configQFile->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader;
    xmlReader.setDevice(configQFile);
    xmlReader.readNext();
    while(!xmlReader.atEnd())
    {
        //
        if (xmlReader.name() == QString("useAgent"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.useAgent =
                            attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("showNotifications"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.showNotifications =
                            attr.value().toString().toInt();
            }else{}


        if (xmlReader.name() == QString("lastUsedPMTableUpdateInterval"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.lastUsedPMTableUpdateInterval =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("showNotificationToDisableAutoSwitcher"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.showNotificationToDisableAutoSwitcher =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("autoPresetApplyDurationChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.autoPresetApplyDurationChecked =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("autoPresetApplyDuration"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.autoPresetApplyDuration =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("autoPresetSwitchAC"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.autoPresetSwitchAC =
                            attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("dcStatePresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.dcStatePresetId =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("acStatePresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.acStatePresetId =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("epmAutoPresetSwitch"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.epmAutoPresetSwitch =
                            attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("epmBatterySaverPresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.epmBatterySaverPresetId =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("epmBetterBatteryPresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.epmBetterBatteryPresetId =
                            attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("epmBalancedPresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.epmBalancedPresetId =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("epmMaximumPerfomancePresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.epmMaximumPerfomancePresetId =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("epmGamingPresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.epmGamingPresetId =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("hideNotSupportedVariables"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.hideNotSupportedVariables =
                            attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("apuFamilyIdx"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.apuFamilyIdx =
                            attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("showArmourPlugin"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    settingsBuffer.showArmourPlugin =
                            attr.value().toString().toInt();
            }else{}
        //
        xmlReader.readNext();
    }
    configQFile->close();
    qDebug() << "Ctrl Settings - Opened";
    return true;
}

bool CtrlSettings::savePresets() {
    presetsQFile->open(QIODevice::WriteOnly);
    QXmlStreamWriter xmlWriter(presetsQFile);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("Presets");
    //
    for(qsizetype i = 0;i < presets->count();i++){
        xmlWriter.writeStartElement("Preset");

            xmlWriter.writeStartElement("presetId");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->presetId));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("presetName");
                xmlWriter.writeAttribute("value", presets->at(i)->presetName);
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("fanPresetId");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->fanPresetId));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("tempLimitValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->tempLimitValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("tempLimitChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->tempLimitChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("apuSkinValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->apuSkinValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("apuSkinChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->apuSkinChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("stampLimitValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->stampLimitValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("stampLimitChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->stampLimitChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("fastLimitValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->fastLimitValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("fastLimitChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->fastLimitChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("fastTimeValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->fastTimeValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("fastTimeChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->fastTimeChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("slowLimitValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->slowLimitValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("slowLimitChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->slowLimitChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("slowTimeValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->slowTimeValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("slowTimeChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->slowTimeChecked));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("vrmCurrentValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->vrmCurrentValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("vrmCurrentChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->vrmCurrentChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("vrmMaxValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->vrmMaxValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("vrmMaxChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->vrmMaxChecked));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("minFclkValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minFclkValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("minFclkChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minFclkChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxFclkValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxFclkValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxFclkChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxFclkChecked));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("minGfxclkValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minGfxclkValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("minGfxclkChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minGfxclkChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxGfxclkValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxGfxclkValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxGfxclkChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxGfxclkChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("minSocclkValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minSocclkValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("minSocclkChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minSocclkChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxSocclkValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxSocclkValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxSocclkChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxSocclkChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("minVcnValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minVcnValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("minVcnChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->minVcnChecked));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxVcnValue");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxVcnValue));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("maxVcnChecked");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->maxVcnChecked));
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("smuMaxPerfomance");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->smuMaxPerfomance));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("smuPowerSaving");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->smuPowerSaving));
            xmlWriter.writeEndElement();
            //new 0.8.3
            xmlWriter.writeStartElement("gfx_clk");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->gfx_clk));
            xmlWriter.writeEndElement();
            //new 0.8.4
            xmlWriter.writeStartElement("vrmgfx_current");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->vrmgfx_current));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("vrmcvip_current");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->vrmcvip_current));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("vrmgfxmax_current");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->vrmgfxmax_current));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("psi3cpu_current");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->psi3cpu_current));
            xmlWriter.writeEndElement();
            xmlWriter.writeStartElement("psi3gfx_current");
                xmlWriter.writeAttribute("value", QString::number(presets->at(i)->psi3gfx_current));
            xmlWriter.writeEndElement();

        xmlWriter.writeEndElement();
    }
    //
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    presetsQFile->close();
    qDebug() << "Ctrl Settings - Presets Saved";
    return true;
}

bool CtrlSettings::openPresets(){
    presetsQFile->open(QIODevice::ReadOnly | QIODevice::Text);

    QXmlStreamReader xmlReader;
    xmlReader.setDevice(presetsQFile);
    xmlReader.readNext();
    presetStr *presetReadBuffer = new presetStr;
    presetReadBuffer->presetId = -1;


    while(!xmlReader.atEnd()) {
        //
        if (xmlReader.name() == QString("presetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value") {
                    if(presetReadBuffer->presetId != -1)
                        presets->append(presetReadBuffer);
                    presetReadBuffer = new presetStr;
                    presetReadBuffer->presetId = attr.value().toString().toInt();
                }
            }else{}
        if (xmlReader.name() == QString("presetName"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value"){
                    QString presetName = attr.value().toString();
                    for(qsizetype i = 0;i < presetName.size() && i < (sizeof(presetReadBuffer->presetName) - 1); i++)
                        presetReadBuffer->presetName[i] = presetName.at(i).toLatin1();
                    presetReadBuffer->presetName[presetName.size()] = '\0';
                }
            }else{}

        if (xmlReader.name() == QString("fanPresetId"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->fanPresetId = attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("tempLimitValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->tempLimitValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("tempLimitChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->tempLimitChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("apuSkinValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->apuSkinValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("apuSkinChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->apuSkinChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("stampLimitValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->stampLimitValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("stampLimitChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->stampLimitChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("fastLimitValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->fastLimitValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("fastLimitChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->fastLimitChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("fastTimeValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->fastTimeValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("fastTimeChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->fastTimeChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("slowLimitValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->slowLimitValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("slowLimitChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->slowLimitChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("slowTimeValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->slowTimeValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("slowTimeChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->slowTimeChecked = attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("vrmCurrentValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->vrmCurrentValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("vrmCurrentChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->vrmCurrentChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("vrmMaxValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->vrmMaxValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("vrmMaxChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->vrmMaxChecked = attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("minFclkValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minFclkValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("minFclkChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minFclkChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxFclkValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxFclkValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxFclkChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxFclkChecked = attr.value().toString().toInt();
            }else{}

        if (xmlReader.name() == QString("minGfxclkValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minGfxclkValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("minGfxclkChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minGfxclkChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxGfxclkValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxGfxclkValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxGfxclkChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxGfxclkChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("minSocclkValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minSocclkValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("minSocclkChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minSocclkChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxSocclkValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxSocclkValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxSocclkChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxSocclkChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("minVcnValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minVcnValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("minVcnChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->minVcnChecked = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxVcnValue"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxVcnValue = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("maxVcnChecked"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->maxVcnChecked = attr.value().toString().toInt();
            }else{}


        if (xmlReader.name() == QString("smuMaxPerfomance"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->smuMaxPerfomance = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("smuPowerSaving"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->smuPowerSaving = attr.value().toString().toInt();
            }else{}
        //new 0.8.3
        if (xmlReader.name() == QString("gfx_clk"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->gfx_clk = attr.value().toString().toInt();
            }else{}
        //new 0.8.4
        if (xmlReader.name() == QString("vrmgfx_current"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->vrmgfx_current = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("vrmcvip_current"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->vrmcvip_current = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("vrmgfxmax_current"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->vrmgfxmax_current = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("psi3cpu_current"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->psi3cpu_current = attr.value().toString().toInt();
            }else{}
        if (xmlReader.name() == QString("psi3gfx_current"))
            foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()){
                if (attr.name().toString() == "value")
                    presetReadBuffer->psi3gfx_current = attr.value().toString().toInt();
            }else{}
        //
        xmlReader.readNext();
    }
    presets->append(presetReadBuffer);
    presetsQFile->close();
    qDebug() << "Ctrl Settings - Presets Opened";
    return true;
}

settingsStr* CtrlSettings::getSettingsBuffer() {
    qDebug() << "Ctrl Settings - Get Settings";
    return &settingsBuffer;
}

const QList<presetStr*>* CtrlSettings::getPresetsList() {
    qDebug() << "Ctrl Settings - Get Presets List";
    return presets;
}

qsizetype CtrlSettings::getPresetsCount() {
    qDebug() << "Ctrl Settings - Get Presets Count"
             << presets->count();
    return presets->count();
}

bool CtrlSettings::setPresetBuffer(int idx, presetStr* preset) {
    QString presetName = QByteArray(preset->presetName);
    qDebug() << "Ctrl Settings - Set Preset ID" << idx << presetName;
    presetStr* presetBuffer = getPresetBuffer(idx);
    if (presetBuffer != nullptr)
        memcpy(presetBuffer, preset, sizeof(presetStr));
    else
        insertNewPreset(idx, preset);
    return true;
}

presetStr* CtrlSettings::getPresetBuffer(int idx) {
    qDebug() << "Ctrl Settings - Get Preset ID" << idx;
    presetStr* presetBuffer = nullptr;
    for (qsizetype i = 0; i < presets->count(); i++)
        if (presets->at(i)->presetId == idx)
            presetBuffer = presets->at(i);
    return presetBuffer;
}

int CtrlSettings::insertNewPreset(int newidx, presetStr* newPreset) {
    qDebug() << "Ctrl Settings - Insert New Preset ID" << newidx;
    if (newPreset == nullptr) {
        newPreset = new presetStr;
        char newPresetName[] = {'N', 'e', 'w', ' ', 'p', 'r', 'e', 's', 'e', 't', '\0'};
        for(int i = 0;i < sizeof(newPresetName) + 1; i++)
            newPreset->presetName[i] = newPresetName[i];
    }
    if (newidx == -1) {
        newidx = presets->count();
        for (;;) {
            if (getPresetBuffer(newidx) == nullptr)
                break;
            newidx++;
        }
    }
    newPreset->presetId = newidx;
    presets->append(newPreset);
    return newidx;
}

bool CtrlSettings::deletePreset(int idx) {
    qDebug() << "Ctrl Settings - Delete Preset ID" << idx;
    presetStr* preset = getPresetBuffer(idx);
    presets->removeOne(preset);
    delete preset;
    return true;
}
