#include "Library/LS3A6KPackageControlLib.h"

EFI_STATUS PackageControlInit() {
    if (sizeof(LS3A6000_FREQ_CONFIG_REG_1B0) == 8 &&
        sizeof(LS3A6000_FUNC_CONFIG_REG_180) == 8 &&
        sizeof(LS3A6000_OTHER_FUNC_CONFIG_REG_420) == 8 &&
        sizeof(LS3A6000_AVS_CSR_REG_160) == 4 &&
        sizeof(LS3A6000_AVS_MREG_REG_164) == 4 &&
        sizeof(LS3A6000_AVS_SREG_REG_168) == 4) {
        DebugPrint(EFI_D_INFO, "Package control init\n");
        return EFI_SUCCESS;
    }
    DebugPrint(EFI_D_INFO, "Wrong structure size\n");
    return EFI_WRITE_PROTECTED;
}

EFI_STATUS AvsRead(UINT16 cmd_type, UINT16 rail, UINT16 *Ret) {
    UINT16 cmd = LS3A6000_AVS_CMD_READ;
    LS3A6000_AVS_CSR_REG_160 AvsCsrReg;
    AvsCsrReg.Value = 0x70000; // manual page 170
    AsmIOCSRWrite32(LS3A6000_AVS_CSR_REG, AvsCsrReg.Value);

    LS3A6000_AVS_MREG_REG_164 AvsMregReg;
    AvsMregReg.Value = 0;
    AvsMregReg.TX_EN = 1;
    AvsMregReg.cmd = cmd;
    AvsMregReg.cmd_type = cmd_type;
    AvsMregReg.rail_sel = rail;
    // DebugPrint(EFI_D_INFO, "avs write32 REG_164=%08x\n", AvsMregReg.Value);
    AsmIOCSRWrite32(LS3A6000_AVS_MREG_REG, AvsMregReg.Value);

    LS3A6000_AVS_SREG_REG_168 AvsSregReg;
    // int i = AVS_RETRY_COUNT_MS;
    while(1) {
        AvsSregReg.Value = AsmIOCSRRead32(LS3A6000_AVS_SREG_REG);
        if (AvsSregReg.busy == 0) {
            // DebugPrint(EFI_D_INFO, "avs read32 REG_168=%08x time=%d ms\n", AvsSregReg.Value, AVS_RETRY_COUNT_MS - i);
            if (AvsSregReg.slave_ack == 0) {
                *Ret = AvsSregReg.sdata;
                return EFI_SUCCESS;
            }
            DebugPrint(EFI_D_ERROR, "avs read not ack\n");
            return EFI_DEVICE_ERROR;
        }
        // gBS->Stall(1000);
    }

    DebugPrint(EFI_D_ERROR, "avs read timeout, cmd %d cmd_type %d rail %d\n", cmd, cmd_type, rail);
    return EFI_TIMEOUT;
}

EFI_STATUS AvsWrite(UINT16 cmd_type, UINT16 rail, UINT16 data) {
    UINT16 cmd = LS3A6000_AVS_CMD_WRITE_COMMIT;
    LS3A6000_AVS_CSR_REG_160 AvsCsrReg;
    AvsCsrReg.Value = 0x70000; // manual page 170
    AsmIOCSRWrite32(LS3A6000_AVS_CSR_REG, AvsCsrReg.Value);

    LS3A6000_AVS_MREG_REG_164 AvsMregReg;
    AvsMregReg.Value = 0;
    AvsMregReg.TX_EN = 1;
    AvsMregReg.cmd = cmd;
    AvsMregReg.cmd_type = cmd_type;
    AvsMregReg.rail_sel = rail;
    AvsMregReg.cmd_data = data;
    // DebugPrint(EFI_D_INFO, "avs write32 REG_164=%08x\n", AvsMregReg.Value);
    AsmIOCSRWrite32(LS3A6000_AVS_MREG_REG, AvsMregReg.Value);

    LS3A6000_AVS_SREG_REG_168 AvsSregReg;
    // int i = AVS_RETRY_COUNT_MS;
    while(1) {
        AvsSregReg.Value = AsmIOCSRRead32(LS3A6000_AVS_SREG_REG);
        if (AvsSregReg.busy == 0) {
            // DebugPrint(EFI_D_INFO, "avs write32 REG_168=%08x time=%d ms\n", AvsSregReg.Value, AVS_RETRY_COUNT_MS - i);
            if (AvsSregReg.slave_ack == 0)
                return EFI_SUCCESS;
            DebugPrint(EFI_D_ERROR, "avs write not ack\n");
            return EFI_DEVICE_ERROR;
        }
        // gBS->Stall(1000);
    }

    DebugPrint(EFI_D_ERROR, "avs write timeout, cmd %d cmd_type %d rail %d data %d\n", cmd, cmd_type, rail, data);
    return EFI_TIMEOUT;
}

EFI_STATUS SetVoltage(UINT32 rail, UINT16 Voltage) {
    DebugPrint(EFI_D_INFO, "setting rail %d to voltage %d\n", rail, Voltage);
    EFI_STATUS Status = AvsWrite(LS3A6000_AVS_CMD_TYPE_VOLT, rail, Voltage);
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "set rail %d voltage failed: %d\n", rail, Status);
        return EFI_DEVICE_ERROR;
    }

    UINT16 VoltageRead;
    Status = AvsRead(LS3A6000_AVS_CMD_TYPE_VOLT, rail, &VoltageRead);
    if (EFI_ERROR(Status))
        DebugPrint(EFI_D_WARN, "avs rail %d read voltage failed: %d\n", rail, Status);
    else
        DebugPrint(EFI_D_INFO, "AVS rail %d voltage: %d mV\n", rail, (UINT32)VoltageRead);
    return EFI_SUCCESS;
}

EFI_STATUS SetMainClockFreq(UINT32 clockMhz) {
    if (clockMhz > 2500) {
        // HT clock will exceed 1.25G
        LS3A6000_FUNC_CONFIG_REG_180 FuncConfigReg;
        FuncConfigReg.Value = AsmIOCSRRead64(LS3A6000_FUNC_CONFIG_REG);
        FuncConfigReg.HT_freq_scale_ctrl = 2;
        DebugPrint(EFI_D_INFO, "debug [0x180] = %016llx\n", FuncConfigReg.Value);
        AsmIOCSRWrite64(LS3A6000_FUNC_CONFIG_REG, FuncConfigReg.Value);
        DBG_REG_PRINT(LS3A6000_FUNC_CONFIG_REG, 180);
    }

    LS3A6000_FREQ_CONFIG_REG_1B0 FreqConfigReg;
    FreqConfigReg.Value = AsmIOCSRRead64(LS3A6000_FREQ_CONFIG_REG);
    FreqConfigReg.SEL_PLL_NODE = 0;
    FreqConfigReg.SOFT_SET_PLL = 0;

    UINT32 NewL1LoopC = clockMhz * FreqConfigReg.L1_DIV_REFC / 100 * FreqConfigReg.L1_DIV_OUT;
    DebugPrint(EFI_D_INFO, "L1_DIV_REFC=%d, L1_DIV_LOOPC=%d, L1_DIV_OUT=%d, new L1_DIV_LOOPC=%d\n", FreqConfigReg.L1_DIV_REFC, FreqConfigReg.L1_DIV_LOOPC, FreqConfigReg.L1_DIV_OUT, NewL1LoopC);
    FreqConfigReg.L1_DIV_LOOPC = NewL1LoopC;
    AsmIOCSRWrite64(LS3A6000_FREQ_CONFIG_REG, FreqConfigReg.Value);
    DBG_REG_PRINT(LS3A6000_FREQ_CONFIG_REG, 1B0);

    FreqConfigReg.Value = AsmIOCSRRead64(LS3A6000_FREQ_CONFIG_REG);
    FreqConfigReg.SEL_PLL_NODE = 0;
    FreqConfigReg.SOFT_SET_PLL = 1;
    AsmIOCSRWrite64(LS3A6000_FREQ_CONFIG_REG, FreqConfigReg.Value);
    DBG_REG_PRINT(LS3A6000_FREQ_CONFIG_REG, 1B0);
    // int retry = PLL_RETRY_COUNT_MS;
    while (1) {
        if (((LS3A6000_FREQ_CONFIG_REG_1B0)(AsmIOCSRRead64(LS3A6000_FREQ_CONFIG_REG))).LOCKED_L1 == 1) {
            break;
        }
        // gBS->Stall(1000);
    }
    DBG_REG_PRINT(LS3A6000_FREQ_CONFIG_REG, 1B0);
    // if (retry == 0) {
    //     DebugPrint(EFI_D_ERROR, "L1 PLL lock timeout\n");
    //     return EFI_TIMEOUT;
    // } else {
        // DebugPrint(EFI_D_INFO, "L1 PLL locked, time=%dms\n", PLL_RETRY_COUNT_MS-retry);
    // }
    DebugPrint(EFI_D_INFO, "L1 PLL locked\n");
    FreqConfigReg.Value = AsmIOCSRRead64(LS3A6000_FREQ_CONFIG_REG);
    FreqConfigReg.SEL_PLL_NODE = 1;
    AsmIOCSRWrite64(LS3A6000_FREQ_CONFIG_REG, FreqConfigReg.Value);
    DBG_REG_PRINT(LS3A6000_FREQ_CONFIG_REG, 1B0);

    return EFI_SUCCESS;
}

VOID PrintPackageStatus() {
    LS3A6000_FREQ_CONFIG_REG_1B0 FreqConfigReg;
    FreqConfigReg.Value = AsmIOCSRRead64(LS3A6000_FREQ_CONFIG_REG);
    LS3A6000_FUNC_CONFIG_REG_180 FuncConfigReg;
    FuncConfigReg.Value = AsmIOCSRRead64(LS3A6000_FUNC_CONFIG_REG);
    LS3A6000_OTHER_FUNC_CONFIG_REG_420 OtherFuncConfigReg;
    OtherFuncConfigReg.Value = AsmIOCSRRead64(LS3A6000_OTHER_FUNC_CONFIG_REG);

    UINT32 mainClock = SYS_PLL_MHZ / FreqConfigReg.L1_DIV_REFC * FreqConfigReg.L1_DIV_LOOPC / FreqConfigReg.L1_DIV_OUT;
    UINT32 nodeClock;
    if (OtherFuncConfigReg.freqscale_mode_node == 0)
        nodeClock = mainClock * (FuncConfigReg.Node_freq_ctrl + 1) / 8;
    else
        nodeClock = mainClock / (FuncConfigReg.Node_freq_ctrl + 1);
    UINT32 htClock; 
    if (OtherFuncConfigReg.freqscale_mode_HT == 0)
        htClock = nodeClock * (FuncConfigReg.HT_freq_scale_ctrl+1)/8;
    else
        htClock = nodeClock / (FuncConfigReg.HT_freq_scale_ctrl+1);

    DebugPrint(EFI_D_INFO, "REG[0x%08x] = 0x%016llx\n", LS3A6000_FREQ_CONFIG_REG, FreqConfigReg.Value);
    DebugPrint(EFI_D_INFO, "REG[0x%08x] = 0x%016llx\n", LS3A6000_FUNC_CONFIG_REG, FuncConfigReg.Value);
    DebugPrint(EFI_D_INFO, "REG[0x%08x] = 0x%016llx\n", LS3A6000_OTHER_FUNC_CONFIG_REG, OtherFuncConfigReg.Value);

    DebugPrint(EFI_D_INFO, "Main clock: %u Mhz\n", (UINT32)mainClock);
    DebugPrint(EFI_D_INFO, "Node clock: %u Mhz\n", (UINT32)nodeClock);
    DebugPrint(EFI_D_INFO, "HT clock: %u Mhz\n", (UINT32)htClock);

    UINT16 Voltage;
    UINT16 Temp;
    UINT16 Current;
    EFI_STATUS Status;
    for(int i=0;i<LS3A6000_XA61200_AVS_TOTAL_RAIL;++i) {
        Status = AvsRead(LS3A6000_AVS_CMD_TYPE_CURRENT, i, &Current);
        if (EFI_ERROR(Status))
            DebugPrint(EFI_D_WARN, "avs rail %d read current failed: %d\n", i, Status);
        else
            DebugPrint(EFI_D_INFO, "AVS rail %d current: %d mA\n", i, (UINT32)Current * 20);

        Status = AvsRead(LS3A6000_AVS_CMD_TYPE_TEMP, i, &Temp);
        if (EFI_ERROR(Status))
            DebugPrint(EFI_D_WARN, "avs rail %d read temperature failed: %d\n", i, Status);
        else
            DebugPrint(EFI_D_INFO, "AVS rail %d temperature: %d C\n", i, (UINT32)Temp / 10);
        
        Status = AvsRead(LS3A6000_AVS_CMD_TYPE_VOLT, i, &Voltage);
        if (EFI_ERROR(Status))
            DebugPrint(EFI_D_WARN, "avs rail %d read voltage failed: %d\n", i, Status);
        else
            DebugPrint(EFI_D_INFO, "AVS rail %d voltage: %d mV\n", i, (UINT32)Voltage);
    }

}

