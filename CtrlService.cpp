#include "CtrlService.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QDateTime>
#include <iostream>
#include <Windows.h>

#define buffer_size 512
#define bufferToService_refresh_time 100

CtrlService::CtrlService(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf)
    : QObject(nullptr),
      bufferToService(bufferToService),
      bufferToGui(bufferToGui),
      conf(conf)
{
    settingsStr *settings = conf->getSettings();
    bufferToService->create(buffer_size);
    bufferToGui->create(buffer_size);
    bufferToService_refresh_timer = new QTimer;
    bufferToService_refresh_timer->connect(bufferToService_refresh_timer,
                                           &QTimer::timeout, this,
                                           &CtrlService::recieveArgs);
    bufferToService_refresh_timer->start(bufferToService_refresh_time);

    reapplyPresetTimer = new QTimer;
    connect(reapplyPresetTimer, &QTimer::timeout,
            this, &CtrlService::reapplyPresetTimeout);
    if(settings->autoPresetApplyDurationChecked)
        reapplyPresetTimer->start(settings->autoPresetApplyDuration * 1000);

    qDebug() << "RyzenAdj Service started";
    acCallback = new CtrlACCallback;
    if(settings->autoPresetSwitchAC)
        connect(acCallback, &CtrlACCallback::currentACStateChanged,
                this, &CtrlService::currentACStateChanged);
    acCallback->emitCurrentACState();

    epmCallback = new CtrlEPMCallback;
    if(settings->epmAutoPresetSwitch)
        connect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                this, &CtrlService::epmIdChanged);
    epmCallback->emitCurrentEPMState();
}

CtrlService::~CtrlService() {}

void CtrlService::recieveArgs(){
    char *iodata = (char*)bufferToService->data();
    QByteArray arguments;
    if (bufferToService->lock())
    {
      for (int i=0;iodata[i];i++) {
        arguments.append(iodata[i]);
        iodata[i] = '\0';
      }
      bufferToService->unlock();
    }
    if(arguments.size() > 0)
        decodeArgs(arguments);
}

void CtrlService::decodeArgs(QByteArray args){
    qDebug()<<"Recieved args from GUI";
    bool save = false;
    int id = -1;
    QString ryzenAdjCmdLine;
    int fanPresetId = 0;
    settingsStr* settings = conf->getSettings();
    presetStr* presets = conf->getPresets();

    QXmlStreamReader argsReader(args);
    argsReader.readNext();
    while(!argsReader.atEnd())
    {
        //
        if (argsReader.name() == QString("save"))
            save = true;
        if (argsReader.name() == QString("id"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                if (attr.name().toString() == "value")
                    id = attr.value().toString().toInt();
        if (argsReader.name() == QString("ryzenAdjCmdLine"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                if (attr.name().toString() == "value"){
                    ryzenAdjCmdLine = attr.value().toString();
                    if(save)
                        presets[id].cmdOutputValue = ryzenAdjCmdLine;
                }
        if (argsReader.name() == QString("fanPresetId"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                if (attr.name().toString() == "value") {
                    fanPresetId = attr.value().toString().toInt();
                    if(save)
                        presets[id].fanPresetId = fanPresetId;
                }
        if (argsReader.name() == QString("exit")){
            qDebug() << "Ricieved exit command";
            qDebug() << "RyzenAdj Service stoped";
            exit(0);
        }

        if (argsReader.name() == QString("autoPresetApplyDurationChecked"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                if (attr.name().toString() == "value"){
                        settings->autoPresetApplyDurationChecked
                                = attr.value().toString().toInt();
                        qDebug() << "autoPresetApplyDurationChecked set to "
                                 << settings->autoPresetApplyDurationChecked;
                        reapplyPresetTimer->stop();
                        if(conf->getSettings()->autoPresetApplyDurationChecked)
                            reapplyPresetTimer->start(conf ->getSettings()
                                                        ->autoPresetApplyDuration * 1000);
                    }
        if (argsReader.name() == QString("autoPresetApplyDuration"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                if (attr.name().toString() == "value"){
                        settings->autoPresetApplyDuration
                                = attr.value().toString().toInt();
                        qDebug() << "autoPresetApplyDuration set to "
                                 << settings->autoPresetApplyDuration;
                        reapplyPresetTimer->stop();
                        if(conf->getSettings()->autoPresetApplyDurationChecked)
                            reapplyPresetTimer->start(conf->getSettings()
                                                        ->autoPresetApplyDuration * 1000);
                    }

            if (argsReader.name() == QString("autoPresetSwitchAC"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->autoPresetSwitchAC
                                    = attr.value().toString().toInt();
                            qDebug() << "autoPresetSwitchAC set to "
                                     << settings->autoPresetSwitchAC;
                            disconnect(acCallback,&CtrlACCallback::currentACStateChanged,
                                       this, &CtrlService::currentACStateChanged);
                            if(conf->getSettings()->autoPresetSwitchAC)
                                connect(acCallback, &CtrlACCallback::currentACStateChanged,
                                        this, &CtrlService::currentACStateChanged);
                            reapplyPresetTimeout();
                        }
            if (argsReader.name() == QString("dcStatePresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->dcStatePresetId = attr.value().toString().toInt();
                            qDebug() << "dcStatePresetId set to "
                                     << settings->dcStatePresetId;
                            reapplyPresetTimeout();
                        }
            if (argsReader.name() == QString("acStatePresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->acStatePresetId = attr.value().toString().toInt();
                            qDebug() << "acStatePresetId set to "
                                     << settings->acStatePresetId;
                            reapplyPresetTimeout();
                        }

            if (argsReader.name() == QString("epmAutoPresetSwitch"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->epmAutoPresetSwitch
                                    = attr.value().toString().toInt();
                            qDebug() << "epmAutoPresetSwitch set to "
                                     << settings->epmAutoPresetSwitch;
                            disconnect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                                    this, &CtrlService::epmIdChanged);
                            if(conf->getSettings()->epmAutoPresetSwitch)
                                connect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                                        this, &CtrlService::epmIdChanged);
                            reapplyPresetTimeout();
                        }
            if (argsReader.name() == QString("epmBatterySaverPresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->epmBatterySaverPresetId = attr.value().toString().toInt();
                            qDebug() << "epmBatterySaverPresetId set to "
                                     << settings->epmBatterySaverPresetId;
                            reapplyPresetTimeout();
                        }
            if (argsReader.name() == QString("epmBetterBatteryPresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->epmBetterBatteryPresetId = attr.value().toString().toInt();
                            qDebug() << "epmBetterBatteryPresetId set to "
                                     << settings->epmBetterBatteryPresetId;
                            reapplyPresetTimeout();
                        }
            if (argsReader.name() == QString("epmBalancedPresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->epmBalancedPresetId = attr.value().toString().toInt();
                            qDebug() << "epmBalancedPresetId set to "
                                     << settings->epmBalancedPresetId;
                            reapplyPresetTimeout();
                        }
            if (argsReader.name() == QString("epmMaximumPerfomancePresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes())
                    if (attr.name().toString() == "value"){
                            settings->epmMaximumPerfomancePresetId = attr.value().toString().toInt();
                            qDebug() << "epmMaximumPerfomancePresetId set to "
                                     << settings->epmMaximumPerfomancePresetId;
                            reapplyPresetTimeout();
                        }
        //

        argsReader.readNext();
    }

    if(id != -1) {
        qDebug()<<"Preset ID: "<<id;
        sendCurrentPresetIdToGui(id, save);
    }
    if(ryzenAdjCmdLine.size() > 0) {
        RyzenAdjSendCommand(ryzenAdjCmdLine);

    }
    if(fanPresetId > 0) {
        QString fanArguments;
        switch(fanPresetId){
        case 1:
            fanArguments = "plan windows";
            break;
        case 2:
            fanArguments = "plan silent";
            break;
        case 3:
            fanArguments = "plan performance";
            break;
        case 4:
            fanArguments = "plan turbo";
            break;
        case 5:
            fanArguments = "fan --plan windows --cpu 30c:0%,40c:5%,50c:10%,"
                           "60c:20%,70c:35%,80c:55%,90c:65%,100c:65% --gpu "
                           "30c:0%,40c:5%,50c:10%,60c:20%,70c:35%,80c:55%,"
                           "90c:65%,100c:65%";
            break;
        }
        atrofacSendCommand(fanArguments);
    }
}

void CtrlService::currentACStateChanged(ACState state){
    currentACState = state;
    qDebug() << "Current state:"
             << ((state == Battery) ? "DC State" : "AC State");

    if(conf->getSettings()->autoPresetSwitchAC)
        loadPreset((state == Battery)
                   ? (conf->getSettings()->dcStatePresetId)
                   : (conf->getSettings()->acStatePresetId));
}

void CtrlService::epmIdChanged(epmMode EPMode){
    currentEPMode = EPMode;
    QString strEPMode;
    int epmPresetId = -1;
    switch(EPMode){
    case BatterySaver:
        strEPMode = "Battery Saver Mode";
        epmPresetId = conf->getSettings()->epmBatterySaverPresetId;
        break;
    case BetterBattery:
        strEPMode = "Better Battery Effective Power Mode";
        epmPresetId = conf->getSettings()->epmBetterBatteryPresetId;
        break;
    case Balanced:
        strEPMode = "Balanced Effective Power Mode";
        epmPresetId = conf->getSettings()->epmBalancedPresetId;
        break;
    case MaxPerformance:
        strEPMode = "Maximum Performance Effective Power Mode";
        epmPresetId = conf->getSettings()->epmMaximumPerfomancePresetId;
        break;
    default:
        strEPMode = "ERROR!";
        break;
    }
    qDebug() << "Current EPM:" << strEPMode;

    if(conf->getSettings()->epmAutoPresetSwitch && epmPresetId != -1)
        loadPreset(epmPresetId);
}

void CtrlService::reapplyPresetTimeout(){
    qDebug() << "Reapply Preset Timeout";
    acCallback->emitCurrentACState();
    epmCallback->emitCurrentEPMState();
    if(!conf->getSettings()->autoPresetSwitchAC
       && !conf->getSettings()->epmAutoPresetSwitch
       && lastUsedPresetId != -1)
        loadPreset(lastUsedPresetId);
}

void CtrlService::loadPreset(int currentPresetId){
    qDebug()<<"Load preset ID:" << currentPresetId << "...";
    sendCurrentPresetIdToGui(currentPresetId, true);
    if(conf->getPresets()[currentPresetId].cmdOutputValue.size() > 0)
        RyzenAdjSendCommand(conf->getPresets()[currentPresetId].cmdOutputValue);

    if(conf->getPresets()[currentPresetId].fanPresetId > 0) {
        QString fanArguments;
        switch(conf->getPresets()[currentPresetId].fanPresetId){
        case 1:
            fanArguments = "plan windows";
            break;
        case 2:
            fanArguments = "plan silent";
            break;
        case 3:
            fanArguments = "plan performance";
            break;
        case 4:
            fanArguments = "plan turbo";
            break;
        case 5:
            fanArguments = "fan --plan windows --cpu 30c:0%,40c:5%,50c:10%,60c:20%,70c:35%,80c:55%,90c:65%,100c:65% --gpu 30c:0%,40c:5%,50c:10%,60c:20%,70c:35%,80c:55%,90c:65%,100c:65%";
            break;
        }
        atrofacSendCommand(fanArguments);
    }
}

#include <QProcess>
#include <QDebug>
#include <QtCore/QVariant>

void CtrlService::RyzenAdjSendCommand(QString arguments) {
    qDebug()<<"RyzenAdj Commandline: "<<arguments;
    QProcess process;
    QString output, error;
    QStringList argumentsList = arguments.split(" ",Qt::SkipEmptyParts);
    for(int i = 0; i < 3; i++){
        qDebug() << "ryzenadj.exe exec with" << argumentsList;
        process.start("Binaries/ryzenadj.exe", argumentsList);
        if( !process.waitForStarted() || !process.waitForFinished())
            return;
        output = process.readAllStandardOutput();
        error = process.readAllStandardError();
        qDebug() << "RyzenAdj output:" << output;
        qDebug() << "RyzenAdj error:" << error;
        if (!output.contains("Err"))
            break;
    }
}

void CtrlService::atrofacSendCommand(QString arguments) {
    qDebug()<<"atrofac Commandline: "<<arguments;
    QStringList argumentsList = arguments.split(" ",Qt::SkipEmptyParts);
    QProcess process;
    QString output, error;
    for(int i = 0; i < 3; i++){
        qDebug() << "atrofac-cli.exe exec with" << argumentsList;
        process.start("Binaries/atrofac-cli.exe", argumentsList);
        if( !process.waitForStarted() || !process.waitForFinished())
            return;
        output = process.readAllStandardOutput();
        error = process.readAllStandardError();
        qDebug() << "atrofac output:" << output;
        qDebug() << "atrofac error:" << error;
        if (!error.contains("Err"))
            break;
    }
}

void CtrlService::sendCurrentPresetIdToGui(int presetId, bool saved = true){
    lastUsedPresetId = presetId;
    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToGui");
    //
        argsWriter.writeStartElement("currentPresetId");
            argsWriter.writeAttribute("value", QString::number(presetId));
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("saved");
            argsWriter.writeAttribute("value", QString::number(saved));
        argsWriter.writeEndElement();
    //
    argsWriter.writeEndElement();
    argsWriter.writeEndDocument();
    sendArgsToGui(data);
}

void CtrlService::sendArgsToGui(QByteArray arguments){
    char *iodata = (char*)bufferToGui->data();
    if (bufferToGui->lock()) {
        for (int i=0;i<arguments.size();i++)
            iodata[i] = arguments[i];
        bufferToGui->unlock();
    }
}
