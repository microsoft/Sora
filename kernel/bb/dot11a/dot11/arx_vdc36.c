#include "bb/bba.h"
#include <bb/mod.h>
#include "arx_vdc.h"

#define TB_DEPTH        72
#define TB_OUTPUT       144

#define NOR_MASK        0x3

void VitDesCRC36(PBB11A_RX_CONTEXT pRxContextA)
{
    VitDesCRC<36, NOR_MASK, TB_DEPTH, TB_OUTPUT>
        (pRxContextA, pRxContextA->rxFifos->vb3);
}
