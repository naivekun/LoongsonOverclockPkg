#ifndef _IOCSR_H
#define _IOCSR_H

#include <PiPei.h>

UINT8 AsmIOCSRRead8(
    IN volatile UINTN csrAddr
);

UINT8  AsmIOCSRWrite8(
    IN volatile UINTN csrAddr,
    IN volatile UINT8 value
);

UINT32  AsmIOCSRRead32(
    IN volatile UINTN csrAddr
);

UINT64  AsmIOCSRRead64(
    IN volatile UINTN csrAddr
);

UINT32  AsmIOCSRWrite32(
    IN volatile UINTN csrAddr,
    IN volatile UINT32 value
);

UINT64  AsmIOCSRWrite64(
    IN volatile UINTN csrAddr,
    IN volatile UINT64 value
);

#endif