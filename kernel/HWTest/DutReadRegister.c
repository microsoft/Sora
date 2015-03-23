#include "sora.h"

HRESULT DutReadRegisterByOffset(PVOID SoraSysReg, ULONG Offset, PULONG Value)
{
//    if (Offset > 0x200 || (Offset % 4 != 0)) return E_FAIL;
    *Value = READ_REGISTER_ULONG(
        (PULONG)(((PUCHAR)SoraSysReg) + Offset));
    return S_OK;
}