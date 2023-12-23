#ifndef _OVERCLOCK_H
#define _OVERCLOCK_H

#include <PiPei.h>

#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/LS3A6KPackageControlLib.h>
#include <Library/LS3A6KOverclockCommonLib.h>

#include <Ppi/ReadOnlyVariable2.h>

EFI_STATUS OverclockModuleInit();
EFI_STATUS ApplyOverclockSettings();

#endif