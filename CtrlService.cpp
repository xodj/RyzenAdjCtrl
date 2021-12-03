#include "CtrlService.h"

#define buffer_size 512
#define bufferToService_refresh_time 33

CtrlService::CtrlService(CtrlBus *bus, CtrlSettings *conf)
    : QObject(nullptr),
      bus(bus),
      conf(conf)
{
    initPmTable();

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

#ifdef WIN32
    armour = new CtrlArmour;
    epmCallback = new CtrlEPMCallback;
    if(settingsBuffer->epmAutoPresetSwitch)
        connect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                this, &CtrlService::epmIdChanged);
    epmCallback->emitCurrentEPMState();
#endif

    takeCurrentInfoTimer = new QTimer;
    takeCurrentInfoTimer->connect(takeCurrentInfoTimer, &QTimer::timeout, this, &CtrlService::takeCurrentInfo);

    qDebug() << "Ctrl Service - started";
    connect(bus, &CtrlBus::messageFromGUIRecieved, this, &CtrlService::recieveMessageToService);
#ifdef BUILD_SERVICE
    bus->setServiseRuning();
#endif
}

CtrlService::~CtrlService() {
    cleanup_ryzenadj(adjEntryPoint);
}

static inline char* charFromString(std::string str, char *cstr = NULL){
    int charLength = str.length() + 1;
    if(cstr == NULL)
        cstr = new char[charLength];
    strcpy_s(cstr, charLength, str.c_str());
    cstr[str.length()] = '\0';
    return cstr;
}

void CtrlService::initPmTable(){
    qDebug() << "Ctrl Service - initPmTable";
    adjEntryPoint = init_ryzenadj();
    init_table(adjEntryPoint);
    refresh_table(adjEntryPoint);

    switch(get_cpu_family(adjEntryPoint)){
    case 0:
        charFromString("Raven", pmTable.ryzenFamily);
        break;
    case 1:
        charFromString("Picasso", pmTable.ryzenFamily);
        break;
    case 2:
        charFromString("Renoir", pmTable.ryzenFamily);
        break;
    case 3:
        charFromString("Cezanne", pmTable.ryzenFamily);
        break;
    case 4:
        charFromString("Dali", pmTable.ryzenFamily);
        break;
    case 5:
        charFromString("Lucienne", pmTable.ryzenFamily);
        break;
    default:
        break;
    }
    pmTable.biosVersion = get_bios_if_ver(adjEntryPoint);
    pmTable.pmTableVersion = get_table_ver(adjEntryPoint);
    pmTable.ryzenAdjVersion = RYZENADJ_REVISION_VER;
    pmTable.ryzenAdjMajorVersion =  RYZENADJ_MAJOR_VER;
    pmTable.ryzenAdjMinorVersion = RYZENADJ_MINIOR_VER;
    qDebug() << "Ctrl Service - pmTable.ryzenFamily" << pmTable.ryzenFamily;
    qDebug() << "Ctrl Service - pmTable.biosVersion" << pmTable.biosVersion;
    qDebug() << "Ctrl Service - pmTable.pmTableVersion" << pmTable.pmTableVersion;
    qDebug() << "Ctrl Service - pmTable.ryzenAdjVersion" << pmTable.ryzenAdjVersion;
}

void CtrlService::recieveMessageToService(messageToServiceStr messageToService){
    qDebug()<<"Ctrl Service - Recieved args from GUI";
    if (messageToService.exit){
        qDebug() << "Ctrl Service - Recieved Exit Command";
        qDebug() << "Ctrl Service - Stoped";
        cleanup_ryzenadj(adjEntryPoint);
        exit(0);
    }
    if(messageToService.ryzenAdjInfo)
        currentInfoTimeoutChanged(messageToService.ryzenAdjInfoTimeout);
    if(messageToService.applyPreset) {
        qDebug() << "Ctrl Service - Recieved Apply Preset" <<
                    messageToService.preset.presetId;
        sendCurrentPresetIdToGui(messageToService.preset.presetId, messageToService.savePreset);
        lastPresetSaved = messageToService.savePreset;
        loadPreset(&messageToService.preset);
    }
    if (messageToService.savePreset){
        qDebug() << "Ctrl Service - Recieved Save Preset" << messageToService.preset.presetId;
        conf->setPresetBuffer(messageToService.preset.presetId, &messageToService.preset);
        if(lastPreset != nullptr)
            if(messageToService.preset.presetId == lastPreset->presetId)
                lastPresetSaved = messageToService.savePreset;
    }
    if (messageToService.deletePreset){
        qDebug() << "Ctrl Service - Recieved Delete Preset"
                 << messageToService.preset.presetId;
        conf->deletePreset(messageToService.preset.presetId);
    }
    if (messageToService.saveSettings){
        qDebug() << "Ctrl Service - Recieved Save Settings";
        conf->getSettingsBuffer()->useAgent =
                messageToService.settings.useAgent;
        conf->getSettingsBuffer()->showNotifications =
                messageToService.settings.showNotifications;
        conf->getSettingsBuffer()->showNotificationToDisableAutoSwitcher =
                messageToService.settings.showNotificationToDisableAutoSwitcher;
        conf->getSettingsBuffer()->autoPresetApplyDurationChecked =
                messageToService.settings.autoPresetApplyDurationChecked;
        conf->getSettingsBuffer()->autoPresetApplyDuration =
                messageToService.settings.autoPresetApplyDuration;
        conf->getSettingsBuffer()->autoPresetSwitchAC =
                messageToService.settings.autoPresetSwitchAC;
        conf->getSettingsBuffer()->dcStatePresetId =
                messageToService.settings.dcStatePresetId;
        conf->getSettingsBuffer()->acStatePresetId =
                messageToService.settings.acStatePresetId;
        conf->getSettingsBuffer()->epmAutoPresetSwitch =
                messageToService.settings.epmAutoPresetSwitch;
        conf->getSettingsBuffer()->epmBatterySaverPresetId =
                messageToService.settings.epmBatterySaverPresetId;
        conf->getSettingsBuffer()->epmBetterBatteryPresetId =
                messageToService.settings.epmBetterBatteryPresetId;
        conf->getSettingsBuffer()->epmBalancedPresetId =
                messageToService.settings.epmBalancedPresetId;
        conf->getSettingsBuffer()->epmMaximumPerfomancePresetId =
                messageToService.settings.epmMaximumPerfomancePresetId;
        conf->getSettingsBuffer()->epmGamingPresetId =
                messageToService.settings.epmGamingPresetId;
        conf->getSettingsBuffer()->hideNotSupportedVariables =
                messageToService.settings.hideNotSupportedVariables;
        conf->getSettingsBuffer()->apuFamilyIdx =
                messageToService.settings.apuFamilyIdx;
        conf->getSettingsBuffer()->showArmourPlugin =
                messageToService.settings.showArmourPlugin;
        conf->saveSettings();

        reapplyPresetTimer->stop();
        disconnect(acCallback,&CtrlACCallback::currentACStateChanged,
                   this, &CtrlService::currentACStateChanged);
#ifdef WIN32
        disconnect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                   this, &CtrlService::epmIdChanged);
        if(conf->getSettingsBuffer()->epmAutoPresetSwitch) {
            connect(epmCallback, &CtrlEPMCallback::epmIdChanged,
                    this, &CtrlService::epmIdChanged);
            epmCallback->emitCurrentEPMState();
        }
#endif
        if(conf->getSettingsBuffer()->autoPresetSwitchAC) {
            connect(acCallback, &CtrlACCallback::currentACStateChanged,
                    this, &CtrlService::currentACStateChanged);
            acCallback->emitCurrentACState();
        }
        if(conf->getSettingsBuffer()->autoPresetApplyDurationChecked){
            reapplyPresetTimer->start(conf->getSettingsBuffer()
                                      ->autoPresetApplyDuration * 1000);
            reapplyPresetTimeout();
        }
    }
}

void CtrlService::currentACStateChanged(ACState state){
    settingsStr *settingsBuffer = conf->getSettingsBuffer();
    currentACState = state;
    qDebug() << "Ctrl Service - Current state:"
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

#ifdef WIN32
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
    case GameMode:
        strEPMode = "Gaming Effective Power Mode";
        epmPresetId = settingsBuffer->epmGamingPresetId;
        break;
    case MixedReality:
        strEPMode = "Gaming (MixedReality) Effective Power Mode";
        epmPresetId = settingsBuffer->epmGamingPresetId;
        break;
    default:
        strEPMode = "ERROR!";
        break;
    }
    qDebug() << "Ctrl Service - Current EPM" << strEPMode;

    if(settingsBuffer->epmAutoPresetSwitch && epmPresetId != -1){
        lastPresetSaved = true;
        sendCurrentPresetIdToGui(epmPresetId, true);
        loadPreset(conf->getPresetBuffer(epmPresetId));
    }
}
#endif

void CtrlService::reapplyPresetTimeout(){
    qDebug() << "Ctrl Service - Reapply Preset Timeout";
    if(lastPreset != nullptr) {
        sendCurrentPresetIdToGui(lastPreset->presetId, lastPresetSaved);
        loadPreset(lastPreset);
    }
}

void CtrlService::loadPreset(presetStr *preset){
    if(preset != nullptr){
        qDebug() << "Ctrl Service - Load Preset"<<preset->presetId;
        lastPreset = preset;

#ifdef WIN32
        if(preset->fanPresetId > 0) {
            armour->sendArmourThrottlePlan((preset->fanPresetId) - 1);
        }
#endif


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

        //NEW VARS
        if(preset->vrmSocCurrentChecked)
            set_vrmsoc_current(adjEntryPoint, preset->vrmSocCurrent * 1000);
        if(preset->vrmSocMaxChecked)
            set_vrmsocmax_current(adjEntryPoint, preset->vrmSocMax * 1000);

        if(preset->psi0CurrentChecked)
            set_psi0_current(adjEntryPoint, preset->psi0Current * 1000);
        if(preset->psi0SocCurrentChecked)
            set_psi0soc_current(adjEntryPoint, preset->psi0SocCurrent * 1000);

        if(preset->maxLclkChecked)
            set_max_lclk(adjEntryPoint, preset->maxLclk);
        if(preset->minLclkChecked)
            set_min_lclk(adjEntryPoint, preset->minLclk);

        if(preset->prochotDeassertionRampChecked)
            set_prochot_deassertion_ramp(adjEntryPoint, preset->prochotDeassertionRamp);

        if(preset->dgpuSkinTempLimitChecked)
            set_dgpu_skin_temp_limit(adjEntryPoint, preset->dgpuSkinTempLimit);
        if(preset->apuSlowLimitChecked)
            set_apu_slow_limit(adjEntryPoint, preset->apuSlowLimit * 1000);
        if(preset->skinTempPowerLimitChecked)
            set_skin_temp_power_limit(adjEntryPoint, preset->skinTempPowerLimit);
    } else
        qDebug()<<"Ctrl Service - Try to load nullptr (deleted) preset!";
}

void CtrlService::sendCurrentPresetIdToGui(int presetId, bool saved = true){
    qDebug() << "Ctrl Service - Send Current Loaded Preset ID to GUI" << presetId << "Saved" << saved;

    messageToGuiStr messageToGui;
    messageToGui.currentPresetId = presetId;
    messageToGui.presetSaved = saved;
    bus->sendMessageToGui(messageToGui);
}

void CtrlService::currentInfoTimeoutChanged(int timeout){
    if(timeout <= 0)
        takeCurrentInfoTimer->stop();
    else {
        takeCurrentInfoTimer->stop();
        takeCurrentInfoTimer->start(timeout);
    }
}

void CtrlService::takeCurrentInfo() {
    refresh_table(adjEntryPoint);

    pmTable.stapm_limit = get_stapm_limit(adjEntryPoint);
    pmTable.stapm_value = get_stapm_value(adjEntryPoint);
    pmTable.fast_limit = get_fast_limit(adjEntryPoint);
    pmTable.fast_value = get_fast_value(adjEntryPoint);
    pmTable.slow_limit = get_slow_limit(adjEntryPoint);
    pmTable.slow_value = get_slow_value(adjEntryPoint);
    pmTable.apu_slow_limit = get_apu_slow_limit(adjEntryPoint);
    pmTable.apu_slow_value = get_apu_slow_value(adjEntryPoint);
    pmTable.vrm_current = get_vrm_current(adjEntryPoint);
    pmTable.vrm_current_value = get_vrm_current_value(adjEntryPoint);
    pmTable.vrmsoc_current = get_vrmsoc_current(adjEntryPoint);
    pmTable.vrmsoc_current_value = get_vrmsoc_current_value(adjEntryPoint);
    pmTable.vrmmax_current = get_vrmmax_current(adjEntryPoint);
    pmTable.vrmmax_current_value = get_vrmmax_current_value(adjEntryPoint);
    pmTable.vrmsocmax_current = get_vrmsocmax_current(adjEntryPoint);
    pmTable.vrmsocmax_current_value = get_vrmsocmax_current_value(adjEntryPoint);
    pmTable.tctl_temp = get_tctl_temp(adjEntryPoint);
    pmTable.tctl_temp_value = get_tctl_temp_value(adjEntryPoint);
    pmTable.apu_skin_temp_limit = get_apu_skin_temp_limit(adjEntryPoint);
    pmTable.apu_skin_temp_value = get_apu_skin_temp_value(adjEntryPoint);
    pmTable.dgpu_skin_temp_limit = get_dgpu_skin_temp_limit(adjEntryPoint);
    pmTable.dgpu_skin_temp_value = get_dgpu_skin_temp_value(adjEntryPoint);
    pmTable.stapm_time = get_stapm_time(adjEntryPoint);
    pmTable.slow_time = get_slow_time(adjEntryPoint);
    //NEW VARS
    pmTable.psi0_current = get_psi0_current(adjEntryPoint);
    pmTable.psi0soc_current = get_psi0soc_current(adjEntryPoint);

    sendCurrentInfoToGui();
}

void CtrlService::sendCurrentInfoToGui(){
    messageToGuiStr messageToGui;
    messageToGui.pmUpdated = true;
    messageToGui.pmTable = pmTable;
    bus->sendMessageToGui(messageToGui);
}
