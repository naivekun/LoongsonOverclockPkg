#include "Overclock.h"

VOID WriteDefaultOCConfig(LS_OVERCLOCK_CONFIG *ocConfig) {
    SetMem(ocConfig, sizeof(ocConfig), 0);
    ocConfig->Magic = LS_OVERCLOCK_DEFAULT_MAGIC;
    ocConfig->VoltageCore = LS_OVERCLOCK_DEFAULT_VOLT_CORE;
    ocConfig->VoltageSA = LS_OVERCLOCK_DEFAULT_VOLT_SA;
    ocConfig->MainFreq = LS_OVERCLOCK_DEFAULT_FREQ;
    ocConfig->NodeFreqDiv = LS_OVERCLOCK_DEFAULT_NODE_FREQ_DIV;
    ocConfig->NodeFreqMode = LS_OVERCLOCK_DEFAULT_NODE_FREQ_MODE;
    ocConfig->HTFreqDiv = LS_OVERCLOCK_DEFAULT_HT_FREQ_DIV;
    ocConfig->HTFreqMode = LS_OVERCLOCK_DEFAULT_HT_FREQ_MODE;
    ocConfig->Enable = LS_OVERCLOCK_DEFAULT_ENABLE;
}


EFI_STATUS OverclockModuleInit() {
    EFI_PEI_READ_ONLY_VARIABLE2_PPI *Variable;
    UINTN VariableSize;
    DebugPrint(EFI_D_INFO, "ls overclock module init\n");

    EFI_STATUS Status = PeiServicesAllocatePool(sizeof(LS_OVERCLOCK_CONFIG), (VOID**)(UINTN)&ocConfig);
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "allocate memory for ocConfig failed: %d\n", Status);
        return Status;
    }

    // read overclock config from efi variable
    Status = PeiServicesLocatePpi(&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID**)(UINTN)&Variable);
    if (EFI_ERROR(Status))
    {
        DebugPrint(EFI_D_ERROR, "pei variable ppi not avaliable, use default oc config\n");
        WriteDefaultOCConfig(ocConfig);
        return EFI_SUCCESS;
    }
    VariableSize = sizeof(LS_OVERCLOCK_CONFIG);
    Status = Variable->GetVariable(
        Variable,
        L"LsOcConf",
        &LsBoostConfigGuid,
        NULL,
        &VariableSize,
        ocConfig
    );

    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_WARN, "read overclock config failed: %d\n", Status);
        WriteDefaultOCConfig(ocConfig);
    }

    DebugPrint(EFI_D_INFO, "---- overclock config -----\n");
    DebugPrint(EFI_D_INFO, "Enable: %d\n", (UINT32)ocConfig->Enable);
    DebugPrint(EFI_D_INFO, "Freq: %d\n", ocConfig->MainFreq);
    DebugPrint(EFI_D_INFO, "VoltageCore: %d\n", ocConfig->VoltageCore);
    DebugPrint(EFI_D_INFO, "VoltageSA: %d\n", ocConfig->VoltageSA);
    DebugPrint(EFI_D_INFO, "HT FreqDiv: %d\n", ocConfig->HTFreqDiv);
    DebugPrint(EFI_D_INFO, "HT FreqMode: %d\n", ocConfig->HTFreqMode);
    DebugPrint(EFI_D_INFO, "Node FreqDiv: %d\n", ocConfig->NodeFreqDiv);
    DebugPrint(EFI_D_INFO, "Node FreqMode: %d\n", ocConfig->NodeFreqMode);
    DebugPrint(EFI_D_INFO, "---------------------------\n");

    return EFI_SUCCESS;
}

EFI_STATUS ApplyOverclockSettings() {
    EFI_STATUS Status;

    if (ocConfig->Magic != LS_OVERCLOCK_DEFAULT_MAGIC || ocConfig->Enable == FALSE) {
        if (ocConfig->Magic != LS_OVERCLOCK_DEFAULT_MAGIC)
            DebugPrint(EFI_D_ERROR, "invalid ocConfig->Magic[%lld], expect %lld\n", ocConfig->Magic, LS_OVERCLOCK_DEFAULT_MAGIC);
        else
            DebugPrint(EFI_D_INFO, "overclock disabled\n");

        // we nopped the avs set voltage rail 0 in PlatformInitPei, so must set default voltage here
        DebugPrint(EFI_D_INFO, "set default vddn voltage 1150 mV\n");
        SetVoltage(0, LS_OVERCLOCK_DEFAULT_VOLT_CORE);
        DebugPrint(EFI_D_INFO, "set default vddp voltage 1050 mV\n");
        SetVoltage(1, LS_OVERCLOCK_DEFAULT_VOLT_SA);
        return EFI_INVALID_PARAMETER;
    }

    DebugPrint(EFI_D_INFO, "OVERCLOCK ENABLED !!!!!!!!!!!!!!!\n");

    DebugPrint(EFI_D_INFO, "apply overclock settings: voltageCore = %d mV, voltageSA = %d mV, freq = %d Mhz\n", ocConfig->VoltageCore, ocConfig->VoltageSA, ocConfig->MainFreq);
    DebugPrint(EFI_D_INFO, "apply overclock settings: HTFreqDiv = %d, HTFreqMode = %d, NodeFreqDiv = %d, NodeFreqMode = %d\n", ocConfig->HTFreqDiv, ocConfig->HTFreqMode, ocConfig->NodeFreqDiv, ocConfig->NodeFreqMode);
    if (ocConfig->VoltageCore >= LS_OVERCLOCK_LIMIT_VOLT_MIN && ocConfig->VoltageCore <= LS_OVERCLOCK_LIMIT_VOLT_MAX) {
        Status = SetVoltage(0, ocConfig->VoltageCore);
        if (EFI_ERROR(Status)) {
            DebugPrint(EFI_D_ERROR, "failed to set core voltage: %d\n", Status);
        }
    } else {
        DebugPrint(EFI_D_WARN, "invalid core voltage value: %d\n", ocConfig->VoltageCore);
    }

    if (ocConfig->VoltageSA >= LS_OVERCLOCK_LIMIT_VOLT_MIN && ocConfig->VoltageSA <= LS_OVERCLOCK_LIMIT_VOLT_MAX) {
        Status = SetVoltage(1, ocConfig->VoltageSA);
        if (EFI_ERROR(Status)) {
            DebugPrint(EFI_D_ERROR, "failed to set sa voltage: %d\n", Status);
        }
    } else {
        DebugPrint(EFI_D_WARN, "invalid sa voltage value: %d\n", ocConfig->VoltageCore);
    }

    if (ocConfig->MainFreq >= LS_OVERCLOCK_LIMIT_FREQ_MIN && ocConfig->MainFreq <= LS_OVERCLOCK_LIMIT_FREQ_MAX) {
        Status = SetMainClockFreq(ocConfig->MainFreq);
        if (EFI_ERROR(Status)) {
            DebugPrint(EFI_D_ERROR, "failed to set frequency: %d\n", Status);
        }
    } else {
        DebugPrint(EFI_D_WARN, "invalid frequency value: %d\n", ocConfig->MainFreq);
    }

    Status = SetNodeClockControl(ocConfig->NodeFreqDiv, ocConfig->NodeFreqMode);
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "failed to set node clock control: %d\n", Status);
    }

    Status = SetHTClockControl(ocConfig->HTFreqDiv, ocConfig->HTFreqMode);
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "failed to set HT clock control: %d\n", Status);
    }

    return EFI_SUCCESS;
}
