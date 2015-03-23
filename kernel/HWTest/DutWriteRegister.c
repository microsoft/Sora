#include "sora.h"

HRESULT DutWriteRegisterByOffset(PVOID SoraSysReg, ULONG Offset, ULONG Value)
{
//    if (Offset > 0x200 || (Offset % 4 != 0)) return E_FAIL;
    WRITE_REGISTER_ULONG(
        (PULONG)(((PUCHAR)SoraSysReg) + Offset), Value);
    return STATUS_SUCCESS;
}