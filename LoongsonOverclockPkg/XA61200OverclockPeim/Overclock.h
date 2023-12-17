#ifndef _OVERCLOCK_H
#define _OVERCLOCK_H

#include <PiPei.h>

#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/LS3A6KPackageControlLib.h>

#include <Ppi/ReadOnlyVariable2.h>

#define LS_OVERCLOCK_LIMIT_VOLT_MAX 1350
#define LS_OVERCLOCK_LIMIT_VOLT_MIN 1000

#define LS_OVERCLOCK_LIMIT_FREQ_MAX 3200
#define LS_OVERCLOCK_LIMIT_FREQ_MIN 2500

#define LS_OVERCLOCK_DEFAULT_MAGIC      0x55aa114514191981
#define LS_OVERCLOCK_DEFAULT_VOLT_CORE  1150
#define LS_OVERCLOCK_DEFAULT_VOLT_SA    1050
#define LS_OVERCLOCK_DEFAULT_FREQ       2500
#define LS_OVERCLOCK_DEFAULT_ENABLE FALSE

#pragma pack(1)
typedef struct _LS_OVERCLOCK_CONFIG {
    UINT64 Magic;
    UINT32 VoltageCore;
    UINT32 VoltageSA;
    UINT32 MainFreq;
    BOOLEAN Enable;
} LS_OVERCLOCK_CONFIG;
#pragma pack()

EFI_STATUS OverclockModuleInit();
EFI_STATUS ApplyOverclockSettings();

#endif