#include "Overclock.h"

LS_OVERCLOCK_CONFIG *ocConfig;
EFI_GUID LsBoostConfigGuid = {0x1d3f4738, 0x78d5, 0x3ac8, {0x99, 0x8a, 0x32, 0x01, 0x9c, 0x55, 0x2a, 0xf0}};

VOID WriteDefaultOCConfig(LS_OVERCLOCK_CONFIG *ocConfig) {
    SetMem(ocConfig, sizeof(ocConfig), 0);
    ocConfig->Magic = LS_OVERCLOCK_DEFAULT_MAGIC;
    ocConfig->VoltageCore = LS_OVERCLOCK_DEFAULT_VOLT_CORE;
    ocConfig->VoltageSA = LS_OVERCLOCK_DEFAULT_VOLT_SA;
    ocConfig->MainFreq = LS_OVERCLOCK_DEFAULT_FREQ;
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
    DebugPrint(EFI_D_INFO, "---------------------------\n");

    return EFI_SUCCESS;
}

EFI_STATUS ApplyOverclockSettings() {
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

    DebugPrint(EFI_D_INFO, "apply overclock settings: voltageCore = %d V, voltageSA = %d V, freq = %d Mhz\n", ocConfig->VoltageCore, ocConfig->VoltageSA, ocConfig->MainFreq);
    if (ocConfig->VoltageCore >= LS_OVERCLOCK_LIMIT_VOLT_MIN && ocConfig->VoltageCore <= LS_OVERCLOCK_LIMIT_VOLT_MAX) {
        EFI_STATUS Status = SetVoltage(0, ocConfig->VoltageCore);
        if (EFI_ERROR(Status)) {
            DebugPrint(EFI_D_ERROR, "failed to set core voltage: %d\n", Status);
        }
    } else {
        DebugPrint(EFI_D_WARN, "invalid core voltage value: %d\n", ocConfig->VoltageCore);
    }

    if (ocConfig->VoltageSA >= LS_OVERCLOCK_LIMIT_VOLT_MIN && ocConfig->VoltageSA <= LS_OVERCLOCK_LIMIT_VOLT_MAX) {
        EFI_STATUS Status = SetVoltage(1, ocConfig->VoltageSA);
        if (EFI_ERROR(Status)) {
            DebugPrint(EFI_D_ERROR, "failed to set sa voltage: %d\n", Status);
        }
    } else {
        DebugPrint(EFI_D_WARN, "invalid sa voltage value: %d\n", ocConfig->VoltageCore);
    }

    if (ocConfig->MainFreq >= LS_OVERCLOCK_LIMIT_FREQ_MIN && ocConfig->MainFreq <= LS_OVERCLOCK_LIMIT_FREQ_MAX) {
        EFI_STATUS Status = SetMainClockFreq(ocConfig->MainFreq);
        if (EFI_ERROR(Status)) {
            DebugPrint(EFI_D_ERROR, "failed to set frequency: %d\n", Status);
        }
    } else {
        DebugPrint(EFI_D_WARN, "invalid frequency value: %d\n", ocConfig->MainFreq);
    }

    return EFI_SUCCESS;
}
