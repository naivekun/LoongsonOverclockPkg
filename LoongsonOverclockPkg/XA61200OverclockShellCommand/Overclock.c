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
    UINTN VariableSize;
    DebugPrint(EFI_D_INFO, "ls overclock module init\n");

    EFI_STATUS Status = gBS->AllocatePool(EfiBootServicesData, sizeof(LS_OVERCLOCK_CONFIG), (VOID**)(UINTN)&ocConfig);
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "allocate memory for ocConfig failed: %d\n", Status);
        return Status;
    }

    // read overclock config from efi variable
    Status = gRT->GetVariable(
        L"LsOcConf",
        &LsBoostConfigGuid,
        NULL,
        &VariableSize,
        ocConfig
    );

    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_WARN, "read overclock config failed: %d\ncreate default\n", Status);
        WriteDefaultOCConfig(ocConfig);
    }

    if (ocConfig->Magic != LS_OVERCLOCK_DEFAULT_MAGIC) {
        DebugPrint(EFI_D_WARN, "invalid overclock config magic, use default config\n");
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

EFI_STATUS WriteOverclockConfig() {
    DebugPrint(EFI_D_INFO, "write overclock config to variable\n");
    EFI_STATUS Status = gRT->SetVariable(
        L"LsOcConf",
        &LsBoostConfigGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        sizeof(LS_OVERCLOCK_CONFIG),
        ocConfig
    );

    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "Failed to write overclock config: %d\n", Status);
        return Status;
    }
    return EFI_SUCCESS;
}