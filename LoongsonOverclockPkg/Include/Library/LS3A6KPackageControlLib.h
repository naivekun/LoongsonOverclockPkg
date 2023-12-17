#ifndef _PACKAGE_CONTROL_H
#define _PACKAGE_CONTROL_H

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/LS3A6KAsmIOCSRPeiLib.h>

#define LS3A6000_FREQ_CONFIG_REG       0x1fe001b0
#define LS3A6000_FUNC_CONFIG_REG       0x1fe00180
#define LS3A6000_OTHER_FUNC_CONFIG_REG 0x1fe00420

#define LS3A6000_AVS_CSR_REG     0x1fe00160
#define LS3A6000_AVS_MREG_REG    0x1fe00164
#define LS3A6000_AVS_SREG_REG    0x1fe00168

#define LS3A6000_AVS_CMD_WRITE_COMMIT 0x0
#define LS3A6000_AVS_CMD_WRITE        0x1
#define LS3A6000_AVS_CMD_RSVD         0x2
#define LS3A6000_AVS_CMD_READ         0x3

#define LS3A6000_AVS_CMD_TYPE_VOLT       0x0
#define LS3A6000_AVS_CMD_TYPE_SKEW       0x1
#define LS3A6000_AVS_CMD_TYPE_CURRENT    0x2
#define LS3A6000_AVS_CMD_TYPE_TEMP       0x3
#define LS3A6000_AVS_CMD_TYPE_RESET_VOLT 0x4
#define LS3A6000_AVS_CMD_TYPE_POWER      0x5
#define LS3A6000_AVS_CMD_TYPE_CLEAR      0xe
#define LS3A6000_AVS_CMD_TYPE_VERSION    0xf

#define LS3A6000_XA61200_AVS_TOTAL_RAIL 2

#define CLK_M 1000.f*1000.f
#define SYS_PLL_MHZ 100.f
#define SYS_PLL SYS_PLL_MHZ * CLK_M

#define AVS_RETRY_COUNT_MS 500
#define PLL_RETRY_COUNT_MS 100

#pragma pack(1)
typedef union{
    struct {
        UINT16 SEL_PLL_NODE : 1;
        UINT16 RSVD2 : 1;
        UINT16 SOFT_SET_PLL : 1;
        UINT16 DUMMY3 : 13;
        UINT16 LOCKED_L1 : 1;
        UINT16 DUMMY2 : 9;
        UINT16 L1_DIV_REFC : 6;
        UINT16 L1_DIV_LOOPC : 9;
        UINT16 RSVD1 : 1;
        UINT16 L1_DIV_OUT : 6;
        UINT16 DUMMY1 : 16;
    };
    UINT64 Value;
} LS3A6000_FREQ_CONFIG_REG_1B0;

typedef union {
    struct {
        UINT32 DUMMY3 : 24;
        UINT16 HT_freq_scale_ctrl : 3;
        UINT32 DUMMY2 : 13;
        UINT16 Node_freq_ctrl : 3;
        UINT32 DUMMY1 : 21;
    };
    UINT64 Value;
} LS3A6000_FUNC_CONFIG_REG_180;

typedef union {
    struct {
        UINT64 DUMMY3 : 36;
        UINT16 freqscale_mode_node : 1;
        UINT16 DUMMY2 : 1;
        UINT16 freqscale_mode_HT : 2;
        UINT32 DUMMY1 : 24;
    };
    UINT64 Value;
} LS3A6000_OTHER_FUNC_CONFIG_REG_420;

typedef union {
    struct {
        UINT16 RSVD1 : 16;
        UINT16 Dmux : 1;
        UINT16 clk_div : 3;
        UINT16 rx_delay : 4;
        UINT16 rx_ctrl : 1;
        UINT16 mask_a : 1;
        UINT16 mask_c : 1;
        UINT16 mask_s : 1;
        UINT16 mask_i : 1;
        UINT16 mask_ack : 2;
        UINT16 resyn : 1;
    };
    UINT32 Value;
} LS3A6000_AVS_CSR_REG_160;

typedef union {
    struct {
        UINT16 RSVD1 : 4;
        UINT16 cmd_data : 16;
        UINT16 rail_sel : 4;
        UINT16 cmd_type : 4;
        UINT16 group : 1;
        UINT16 cmd : 2;
        UINT16 TX_EN : 1;
    };
    UINT32 Value;
} LS3A6000_AVS_MREG_REG_164;

typedef union {
    struct {
        UINT16 sdata : 16;
        UINT16 RSVD1 : 9;
        UINT16 alert_a : 1;
        UINT16 alert_s : 1;
        UINT16 alert_c : 1;
        UINT16 alert_i : 1;
        UINT16 slave_ack : 2;
        UINT16 busy : 1;
    };
    UINT32 Value;
} LS3A6000_AVS_SREG_REG_168;

#pragma pack()

#define DBG_REG_PRINT(REG, OFFSET) DebugPrint(EFI_D_INFO, "REG[0x%08x] = 0x%016llx\n", REG, ((REG##_##OFFSET)(AsmIOCSRRead64(REG))).Value)

EFI_STATUS PackageControlInit();
EFI_STATUS SetVoltage(UINT32 rail, UINT16 Voltage);
EFI_STATUS AvsRead(UINT16 cmd_type, UINT16 rail, UINT16 *Ret);
EFI_STATUS AvsWrite(UINT16 cmd_type, UINT16 rail, UINT16 data);
VOID PrintPackageStatus();
EFI_STATUS SetMainClockFreq(UINT32 clockMhz);

#endif