#ifndef _OVERCLOCK_H
#define _OVERCLOCK_H

#include <Uefi.h>

#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/LS3A6KOverclockCommonLib.h>

#define FREQ_DIV(mode, base, div) (mode)%2==0?((base)*(div+1)/8):((base)/(div+1))

EFI_STATUS OverclockModuleInit();
EFI_STATUS WriteOverclockConfig();

#endif