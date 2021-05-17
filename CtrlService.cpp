#include "CtrlService.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QDateTime>
#include <iostream>
#include <Windows.h>

#define buffer_size 512
#define bufferToService_refresh_time 33

CtrlService::CtrlService(QSharedMemory *bufferToService, QSharedMemory *bufferToGui, CtrlSettings *conf)
    : QObject(nullptr),
      bufferToService(bufferToService),
      bufferToGui(bufferToGui),
      conf(conf),
      armour(new CtrlArmour)
{
    initPmTable();

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
    settingsStr *settingsBuffer = conf->getSettingsBuffer();
    if(settingsBuffer->autoPresetApplyDurationChecked)
        reapplyPresetTimer->start(settingsBuffer->autoPresetApplyDuration * 1000);

    acCallback = new CtrlACCallback;
    if(settingsBuffer->autoPresetSwitchAC)
        connect(acCallback, &CtrlACCallback::currentACStateChanged,
                this, &CtrlService::currentACStateChanged);
    acCallback->emitCurrentACState();

    epmCallback = new CtrlEPMCallback;
    if(settingsBuffer->epmAutoPresetSwitch)
        connect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                this, &CtrlService::epmIdChanged);
    epmCallback->emitCurrentEPMState();

    takeCurrentInfoTimer = new QTimer;
    takeCurrentInfoTimer->connect(takeCurrentInfoTimer, &QTimer::timeout, this, &CtrlService::takeCurrentInfo);

    qDebug() << "RyzenAdjCtrl Service started";
}

CtrlService::~CtrlService() {
    cleanup_ryzenadj(adjEntryPoint);
}

void CtrlService::initPmTable(){
    qDebug() << "RyzenAdjCtrl Service initPmTable";
    adjEntryPoint = init_ryzenadj();
    init_table(adjEntryPoint);
    refresh_table(adjEntryPoint);

    switch(get_cpu_family(adjEntryPoint)){
    case -1:
        pmTable.ryzenFamily = "Unknown";
        break;
    case 0:
        pmTable.ryzenFamily = "Raven";
        break;
    case 1:
        pmTable.ryzenFamily = "Picasso";
        break;
    case 2:
        pmTable.ryzenFamily = "Renoir";
        break;
    case 3:
        pmTable.ryzenFamily = "Cezanne";
        break;
    case 4:
        pmTable.ryzenFamily = "Dali";
        break;
    case 5:
        pmTable.ryzenFamily = "Lucienne";
        break;
    case 6:
        pmTable.ryzenFamily = "Unknown";
        break;
    default:
        pmTable.ryzenFamily = "Unknown";
        break;
    }
    pmTable.biosVersion = QString::number(get_bios_if_ver(adjEntryPoint));
    std::stringstream ss;
    ss<< std::hex << get_table_ver(adjEntryPoint);
    pmTable.pmTableVersion = QString::fromStdString(ss.str());
    pmTable.ryzenAdjVersion = QString::number(RYZENADJ_REVISION_VER) + "." + QString::number(RYZENADJ_MAJOR_VER) + "." + QString::number(RYZENADJ_MINIOR_VER);
    qDebug() << "RyzenAdjCtrl Service pmTable.ryzenFamily" << pmTable.ryzenFamily;
    qDebug() << "RyzenAdjCtrl Service pmTable.biosVersion" << pmTable.biosVersion;
    qDebug() << "RyzenAdjCtrl Service pmTable.pmTableVersion" << pmTable.pmTableVersion;
    qDebug() << "RyzenAdjCtrl Service pmTable.ryzenAdjVersion" << pmTable.ryzenAdjVersion;
}

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
    qDebug()<<"RyzenAdjCtrl Service Recieved args from GUI";
    bool save = false, apply = false, deletePreset = false;
    int presetId = -1;
    presetStr *recievedPreset = new presetStr;
    settingsStr *settingsBuffer = conf->getSettingsBuffer();

    QXmlStreamReader argsReader(args);
    argsReader.readNext();
    while(!argsReader.atEnd())
    {
        //
        if (argsReader.name() == QString("ryzenAdjInfoTimeout"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value") {
                    currentInfoTimeoutChanged(attr.value().toInt());
                    qDebug() << "RyzenAdjCtrl Service Recieved ryzenAdjInfoTimeout" << attr.value().toInt();
                }
            }else{}


        if (argsReader.name() == QString("id"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    presetId = attr.value().toInt();
                    recievedPreset->presetId = presetId;
                }
            }else{}
        if (argsReader.name() == QString("save"))
            save = true;
        if (argsReader.name() == QString("apply"))
            apply = true;
        if (argsReader.name() == QString("delete"))
            deletePreset = true;


        if (argsReader.name() == QString("tempLimitValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->tempLimitChecked = true;
                    recievedPreset->tempLimitValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("apuSkinValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->apuSkinChecked = true;
                    recievedPreset->apuSkinValue = attr.value().toInt();
                }
            }else{}


        if (argsReader.name() == QString("stampLimitValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->stampLimitChecked = true;
                    recievedPreset->stampLimitValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("fastLimitValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->fastLimitChecked = true;
                    recievedPreset->fastLimitValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("fastTimeValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->fastTimeChecked = true;
                    recievedPreset->fastTimeValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("slowLimitValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->slowLimitChecked = true;
                    recievedPreset->slowLimitValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("slowTimeValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->slowTimeChecked = true;
                    recievedPreset->slowTimeValue = attr.value().toInt();
                }
            }else{}


        if (argsReader.name() == QString("vrmCurrentValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->vrmCurrentChecked = true;
                    recievedPreset->vrmCurrentValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("vrmMaxValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->vrmMaxChecked = true;
                    recievedPreset->vrmMaxValue = attr.value().toInt();
                }
            }else{}


        if (argsReader.name() == QString("minFclkValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->minFclkChecked = true;
                    recievedPreset->minFclkValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("maxFclkValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->maxFclkChecked = true;
                    recievedPreset->maxFclkValue = attr.value().toInt();
                }
            }else{}


        if (argsReader.name() == QString("minGfxclkValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->minGfxclkChecked = true;
                    recievedPreset->minGfxclkValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("maxGfxclkValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->maxGfxclkChecked = true;
                    recievedPreset->maxGfxclkValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("minSocclkValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->minSocclkChecked = true;
                    recievedPreset->minSocclkValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("maxSocclkValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->maxSocclkChecked = true;
                    recievedPreset->maxSocclkValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("minVcnValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->minVcnChecked = true;
                    recievedPreset->minVcnValue = attr.value().toInt();
                }
            }else{}
        if (argsReader.name() == QString("maxVcnValue"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->maxVcnChecked = true;
                    recievedPreset->maxVcnValue = attr.value().toInt();
                }
            }else{}


        if (argsReader.name() == QString("smuMaxPerfomance"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->smuMaxPerfomance = true;
                }
            }else{}
        if (argsReader.name() == QString("smuPowerSaving"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                    recievedPreset->smuPowerSaving = true;
                }
            }else{}


        if (argsReader.name() == QString("fanPresetId"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value")
                    recievedPreset->fanPresetId = attr.value().toInt();
            }else{}


        if (argsReader.name() == QString("exit")){
            qDebug() << "RyzenAdjCtrl Service Recieved Exit Command";
            qDebug() << "RyzenAdjCtrl Service Stoped";
            cleanup_ryzenadj(adjEntryPoint);
            exit(0);
        }



        if (argsReader.name() == QString("autoPresetApplyDurationChecked"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                        settingsBuffer->autoPresetApplyDurationChecked
                                = attr.value().toInt();
                        qDebug() << "RyzenAdjCtrl Service Recieved autoPresetApplyDurationChecked set to "
                                 << settingsBuffer->autoPresetApplyDurationChecked;
                        reapplyPresetTimer->stop();
                        if(settingsBuffer->autoPresetApplyDurationChecked)
                            reapplyPresetTimer->start(settingsBuffer
                                                        ->autoPresetApplyDuration * 1000);
                    }
            }else{}
        if (argsReader.name() == QString("autoPresetApplyDuration"))
            foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                if (attr.name().toString() == "value"){
                        settingsBuffer->autoPresetApplyDuration
                                = attr.value().toInt();
                        qDebug() << "RyzenAdjCtrl Service Recieved autoPresetApplyDuration set to "
                                 << settingsBuffer->autoPresetApplyDuration;
                        reapplyPresetTimer->stop();
                        if(settingsBuffer->autoPresetApplyDurationChecked)
                            reapplyPresetTimer->start(settingsBuffer
                                                        ->autoPresetApplyDuration * 1000);
                    }
            }else{}

            if (argsReader.name() == QString("autoPresetSwitchAC"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->autoPresetSwitchAC
                                    = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved autoPresetSwitchAC set to "
                                     << settingsBuffer->autoPresetSwitchAC;
                            disconnect(acCallback,&CtrlACCallback::currentACStateChanged,
                                       this, &CtrlService::currentACStateChanged);
                            if(settingsBuffer->autoPresetSwitchAC)
                                connect(acCallback, &CtrlACCallback::currentACStateChanged,
                                        this, &CtrlService::currentACStateChanged);
                            reapplyPresetTimeout();
                        }
                }else{}
            if (argsReader.name() == QString("dcStatePresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->dcStatePresetId = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved dcStatePresetId set to "
                                     << settingsBuffer->dcStatePresetId;
                            reapplyPresetTimeout();
                        }
                }else{}
            if (argsReader.name() == QString("acStatePresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->acStatePresetId = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved acStatePresetId set to "
                                     << settingsBuffer->acStatePresetId;
                            reapplyPresetTimeout();
                        }
                }else{}

            if (argsReader.name() == QString("epmAutoPresetSwitch"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->epmAutoPresetSwitch
                                    = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved epmAutoPresetSwitch set to "
                                     << settingsBuffer->epmAutoPresetSwitch;
                            disconnect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                                    this, &CtrlService::epmIdChanged);
                            if(settingsBuffer->epmAutoPresetSwitch)
                                connect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                                        this, &CtrlService::epmIdChanged);
                            reapplyPresetTimeout();
                        }
                }else{}
            if (argsReader.name() == QString("epmBatterySaverPresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->epmBatterySaverPresetId = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved epmBatterySaverPresetId set to "
                                     << settingsBuffer->epmBatterySaverPresetId;
                            reapplyPresetTimeout();
                        }
                }else{}
            if (argsReader.name() == QString("epmBetterBatteryPresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->epmBetterBatteryPresetId = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved epmBetterBatteryPresetId set to "
                                     << settingsBuffer->epmBetterBatteryPresetId;
                            reapplyPresetTimeout();
                        }
                }else{}
            if (argsReader.name() == QString("epmBalancedPresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->epmBalancedPresetId = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved epmBalancedPresetId set to "
                                     << settingsBuffer->epmBalancedPresetId;
                            reapplyPresetTimeout();
                        }
                }else{}
            if (argsReader.name() == QString("epmMaximumPerfomancePresetId"))
                foreach(const QXmlStreamAttribute &attr, argsReader.attributes()){
                    if (attr.name().toString() == "value"){
                            settingsBuffer->epmMaximumPerfomancePresetId = attr.value().toInt();
                            qDebug() << "RyzenAdjCtrl Service Recieved epmMaximumPerfomancePresetId set to "
                                     << settingsBuffer->epmMaximumPerfomancePresetId;
                            reapplyPresetTimeout();
                        }
                }else{}
        //

        argsReader.readNext();
    }

    if(apply) {
        qDebug() << "RyzenAdjCtrl Service Recieved Apply Preset" << presetId;
        sendCurrentPresetIdToGui(presetId, save);
        lastPresetSaved = save;
        loadPreset(recievedPreset);
    }
    if (save){
        qDebug() << "RyzenAdjCtrl Service Recieved Save Preset" << presetId;
        conf->setPresetBuffer(presetId, recievedPreset);
    }
    if (deletePreset){
        qDebug() << "RyzenAdjCtrl Service Recieved Delete Preset" << presetId;
        conf->deletePreset(presetId);
    }
}

void CtrlService::currentACStateChanged(ACState state){
    settingsStr *settingsBuffer = conf->getSettingsBuffer();
    currentACState = state;
    qDebug() << "RyzenAdjCtrl Service Current state:"
             << ((state == Battery) ? "DC State" : "AC State");

    if(settingsBuffer->autoPresetSwitchAC){
        lastPresetSaved = true;
        sendCurrentPresetIdToGui((state == Battery)
                                 ? (settingsBuffer->dcStatePresetId)
                                 : (settingsBuffer->acStatePresetId), true);
        loadPreset(conf->getPresetBuffer((state == Battery)
                ? (settingsBuffer->dcStatePresetId)
                : (settingsBuffer->acStatePresetId)));
    }
}

void CtrlService::epmIdChanged(epmMode EPMode){
    settingsStr *settingsBuffer = conf->getSettingsBuffer();
    currentEPMode = EPMode;
    QString strEPMode;
    int epmPresetId = -1;
    switch(EPMode){
    case BatterySaver:
        strEPMode = "Battery Saver Mode";
        epmPresetId = settingsBuffer->epmBatterySaverPresetId;
        break;
    case BetterBattery:
        strEPMode = "Better Battery Effective Power Mode";
        epmPresetId = settingsBuffer->epmBetterBatteryPresetId;
        break;
    case Balanced:
        strEPMode = "Balanced Effective Power Mode";
        epmPresetId = settingsBuffer->epmBalancedPresetId;
        break;
    case MaxPerformance:
        strEPMode = "Maximum Performance Effective Power Mode";
        epmPresetId = settingsBuffer->epmMaximumPerfomancePresetId;
        break;
    default:
        strEPMode = "ERROR!";
        break;
    }
    qDebug() << "RyzenAdjCtrl Service Current EPM" << strEPMode;

    if(settingsBuffer->epmAutoPresetSwitch && epmPresetId != -1){
        lastPresetSaved = true;
        sendCurrentPresetIdToGui(epmPresetId, true);
        loadPreset(conf->getPresetBuffer(epmPresetId));
    }
}

void CtrlService::reapplyPresetTimeout(){
    qDebug() << "RyzenAdjCtrl Service Reapply Preset Timeout";
    if(lastPreset != nullptr) {
        sendCurrentPresetIdToGui(lastPreset->presetId, lastPresetSaved);
        loadPreset(lastPreset);
    }
}

void CtrlService::loadPreset(presetStr *preset){
    if(preset != nullptr){
        qDebug() << "RyzenAdjCtrl Service Load Preset"<<preset->presetId;
        lastPreset = preset;

        if(preset->fanPresetId > 0) {
            armour->sendArmourThrottlePlan((preset->fanPresetId) - 1);
        }


        if(preset->tempLimitChecked)
            set_tctl_temp(adjEntryPoint, preset->tempLimitValue);
        if(preset->apuSkinChecked)
            set_apu_skin_temp_limit(adjEntryPoint, preset->apuSkinValue);

        if(preset->stampLimitChecked)
            set_stapm_limit(adjEntryPoint, preset->stampLimitValue * 1000);
        if(preset->fastLimitChecked)
            set_fast_limit(adjEntryPoint, preset->fastLimitValue * 1000);
        if(preset->fastTimeChecked)
            set_stapm_time(adjEntryPoint, preset->fastTimeValue);
        if(preset->slowLimitChecked)
            set_slow_limit(adjEntryPoint, preset->slowLimitValue * 1000);
        if(preset->slowTimeChecked)
            set_slow_time(adjEntryPoint, preset->slowTimeValue);

        if(preset->vrmCurrentChecked)
            set_vrm_current(adjEntryPoint, preset->vrmCurrentValue * 1000);
        if(preset->vrmMaxChecked)
            set_vrmmax_current(adjEntryPoint, preset->vrmMaxValue * 1000);

        if(preset->maxFclkChecked)
            set_max_fclk_freq(adjEntryPoint, preset->maxFclkValue);
        if(preset->minFclkChecked)
            set_min_fclk_freq(adjEntryPoint, preset->minFclkValue);

        if(preset->minGfxclkChecked)
            set_max_gfxclk_freq(adjEntryPoint, preset->minGfxclkValue);
        if(preset->maxGfxclkChecked)
            set_min_gfxclk_freq(adjEntryPoint, preset->maxGfxclkValue);
        if(preset->maxSocclkChecked)
            set_max_socclk_freq(adjEntryPoint, preset->maxSocclkValue);
        if(preset->minSocclkChecked)
            set_min_socclk_freq(adjEntryPoint, preset->minSocclkValue);
        if(preset->maxVcnChecked)
            set_max_vcn(adjEntryPoint, preset->maxVcnValue);
        if(preset->minVcnChecked)
            set_min_vcn(adjEntryPoint, preset->minVcnValue);

        if(preset->smuPowerSaving)
            set_power_saving(adjEntryPoint);
        if(preset->smuMaxPerfomance)
            set_max_performance(adjEntryPoint);

        /*set_vrmsoc_current(adjEntryPoint, preset->);
    set_vrmsocmax_current(adjEntryPoint, preset->);
    set_psi0_current(adjEntryPoint, preset->);
    set_psi0soc_current(adjEntryPoint, preset->);
    set_max_lclk(adjEntryPoint, preset->);
    set_min_lclk(adjEntryPoint, preset->);
    set_prochot_deassertion_ramp(adjEntryPoint ry, preset->);
    set_dgpu_skin_temp_limit(adjEntryPoint, preset->);
    set_apu_slow_limit(adjEntryPoint, preset->);
    set_skin_temp_power_limit(adjEntryPoint ry, preset->);*/
    }
}

void CtrlService::sendCurrentPresetIdToGui(int presetId, bool saved = true){
    qDebug() << "RyzenAdjCtrl Service Send Current Loaded Preset ID to GUI" << presetId << "Saved" << saved;
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

void CtrlService::currentInfoTimeoutChanged(int timeout){
    if(timeout == 0)
        takeCurrentInfoTimer->stop();
    else {
        takeCurrentInfoTimer->stop();
        takeCurrentInfoTimer->start(timeout);
    }
}

void CtrlService::takeCurrentInfo() {
    refresh_table(adjEntryPoint);

    pmTable.stapm_limit = QString::number(get_stapm_limit(adjEntryPoint));
    pmTable.stapm_value = QString::number(get_stapm_value(adjEntryPoint));
    pmTable.fast_limit = QString::number(get_fast_limit(adjEntryPoint));
    pmTable.fast_value = QString::number(get_fast_value(adjEntryPoint));
    pmTable.slow_limit = QString::number(get_slow_limit(adjEntryPoint));
    pmTable.slow_value = QString::number(get_slow_value(adjEntryPoint));
    pmTable.apu_slow_limit = QString::number(get_apu_slow_limit(adjEntryPoint));
    pmTable.apu_slow_value = QString::number(get_apu_slow_value(adjEntryPoint));
    pmTable.vrm_current = QString::number(get_vrm_current(adjEntryPoint));
    pmTable.vrm_current_value = QString::number(get_vrm_current_value(adjEntryPoint));
    pmTable.vrmsoc_current = QString::number(get_vrmsoc_current(adjEntryPoint));
    pmTable.vrmsoc_current_value = QString::number(get_vrmsoc_current_value(adjEntryPoint));
    pmTable.vrmmax_current = QString::number(get_vrmmax_current(adjEntryPoint));
    pmTable.vrmmax_current_value = QString::number(get_vrmmax_current_value(adjEntryPoint));
    pmTable.vrmsocmax_current = QString::number(get_vrmsocmax_current(adjEntryPoint));
    pmTable.vrmsocmax_current_value = QString::number(get_vrmsocmax_current_value(adjEntryPoint));
    pmTable.tctl_temp = QString::number(get_tctl_temp(adjEntryPoint));
    pmTable.tctl_temp_value = QString::number(get_tctl_temp_value(adjEntryPoint));
    pmTable.apu_skin_temp_limit = QString::number(get_apu_skin_temp_limit(adjEntryPoint));
    pmTable.apu_skin_temp_value = QString::number(get_apu_skin_temp_value(adjEntryPoint));
    pmTable.dgpu_skin_temp_limit = QString::number(get_dgpu_skin_temp_limit(adjEntryPoint));
    pmTable.dgpu_skin_temp_value = QString::number(get_dgpu_skin_temp_value(adjEntryPoint));
    pmTable.stapm_time = QString::number(get_stapm_time(adjEntryPoint));
    pmTable.slow_time = QString::number(get_slow_time(adjEntryPoint));

    sendCurrentInfoToGui();
}

void CtrlService::sendCurrentInfoToGui(){
    QByteArray data;
    QXmlStreamWriter argsWriter(&data);
    argsWriter.setAutoFormatting(true);
    argsWriter.writeStartDocument();
    argsWriter.writeStartElement("bufferToGui");
    //
        argsWriter.writeStartElement("ryzenFamily");
            argsWriter.writeAttribute("value", pmTable.ryzenFamily);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("biosVersion");
            argsWriter.writeAttribute("value", pmTable.biosVersion);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("pmTableVersion");
            argsWriter.writeAttribute("value", pmTable.pmTableVersion);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("ryzenAdjVersion");
            argsWriter.writeAttribute("value", pmTable.ryzenAdjVersion);
        argsWriter.writeEndElement();

        argsWriter.writeStartElement("stapm_limit");
            argsWriter.writeAttribute("value", pmTable.stapm_limit);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("stapm_value");
            argsWriter.writeAttribute("value", pmTable.stapm_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("fast_limit");
            argsWriter.writeAttribute("value", pmTable.fast_limit);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("fast_value");
            argsWriter.writeAttribute("value", pmTable.fast_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("slow_limit");
            argsWriter.writeAttribute("value", pmTable.slow_limit);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("slow_value");
            argsWriter.writeAttribute("value", pmTable.slow_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("apu_slow_limit");
            argsWriter.writeAttribute("value", pmTable.apu_slow_limit);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("apu_slow_value");
            argsWriter.writeAttribute("value", pmTable.apu_slow_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrm_current");
            argsWriter.writeAttribute("value", pmTable.vrm_current);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrm_current_value");
            argsWriter.writeAttribute("value", pmTable.vrm_current_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrmsoc_current");
            argsWriter.writeAttribute("value", pmTable.vrmsoc_current);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrmsoc_current_value");
            argsWriter.writeAttribute("value", pmTable.vrmsoc_current_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrmmax_current");
            argsWriter.writeAttribute("value", pmTable.vrmmax_current);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrmmax_current_value");
            argsWriter.writeAttribute("value", pmTable.vrmmax_current_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrmsocmax_current");
            argsWriter.writeAttribute("value", pmTable.vrmsocmax_current);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("vrmsocmax_current_value");
            argsWriter.writeAttribute("value", pmTable.vrmsocmax_current_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("tctl_temp");
            argsWriter.writeAttribute("value", pmTable.tctl_temp);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("tctl_temp_value");
            argsWriter.writeAttribute("value", pmTable.tctl_temp_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("apu_skin_temp_limit");
            argsWriter.writeAttribute("value", pmTable.apu_skin_temp_limit);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("apu_skin_temp_value");
            argsWriter.writeAttribute("value", pmTable.apu_skin_temp_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("dgpu_skin_temp_limit");
            argsWriter.writeAttribute("value", pmTable.dgpu_skin_temp_limit);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("dgpu_skin_temp_value");
            argsWriter.writeAttribute("value", pmTable.dgpu_skin_temp_value);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("stapm_time");
            argsWriter.writeAttribute("value", pmTable.stapm_time);
        argsWriter.writeEndElement();
        argsWriter.writeStartElement("slow_time");
            argsWriter.writeAttribute("value", pmTable.slow_time);
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
