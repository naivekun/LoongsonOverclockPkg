#include <PiPei.h>

#include <Library/PeimEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>

#include "Overclock.h"
#include "Library/LS3A6KPackageControlLib.h"

EFI_STATUS
EFIAPI
XA61200OverclockModuleEntrypoint(
    IN EFI_PEI_FILE_HANDLE FileHandle,
    const IN EFI_PEI_SERVICES **PeiServices
) {
    DebugPrint(EFI_D_INFO, "********************************************\n");
    DebugPrint(EFI_D_INFO, "*Loongson 3A6000 overclock module @naivekun*\n");
    DebugPrint(EFI_D_INFO, "********************************************\n\n");

    EFI_STATUS Status = PackageControlInit();
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "package control init failed, check your compiler\n");
        return EFI_SUCCESS;
    }

    // PrintPackageStatus();

    // SetVoltage(0, 1200);
    // SetMainClockFreq(2700);

    // PrintPackageStatus();

    Status = OverclockModuleInit();
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "overclock module init failed: %d\n", Status);
        return EFI_SUCCESS;
    }

    Status = ApplyOverclockSettings();
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "apply overclock settings failed: %d\n", Status);
        return EFI_SUCCESS;
    }

    DebugPrint(EFI_D_INFO, "overclock module success\n");
    return EFI_SUCCESS;
}