#include "Library/LS3A6KOverclockCommonLib.h"

LS_OVERCLOCK_CONFIG *ocConfig;
EFI_GUID LsBoostConfigGuid = {0x1d3f4738, 0x78d5, 0x3ac8, {0x99, 0x8a, 0x32, 0x01, 0x9c, 0x55, 0x2a, 0xf0}};
const CHAR16 *freqControlModes[2] = {
    L"(n+1)/8",
    L"1/(n+1)"
};