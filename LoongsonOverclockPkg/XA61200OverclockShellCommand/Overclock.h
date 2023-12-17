#include <Uefi.h>

#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>

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

extern LS_OVERCLOCK_CONFIG *ocConfig;

EFI_STATUS OverclockModuleInit();
EFI_STATUS WriteOverclockConfig();