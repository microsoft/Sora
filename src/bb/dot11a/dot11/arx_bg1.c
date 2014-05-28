 
#include "bb/bba.h"
#include <bb/mod.h>

void VitDesCRC6(PBB11A_RX_CONTEXT pRxContextA);
void VitDesCRC9(PBB11A_RX_CONTEXT pRxContextA);
void VitDesCRC12(PBB11A_RX_CONTEXT pRxContextA);
void VitDesCRC18(PBB11A_RX_CONTEXT pRxContextA);
void VitDesCRC24(PBB11A_RX_CONTEXT pRxContextA);
void VitDesCRC36(PBB11A_RX_CONTEXT pRxContextA);
void VitDesCRC48(PBB11A_RX_CONTEXT pRxContextA);
void VitDesCRC54(PBB11A_RX_CONTEXT pRxContextA);

BOOLEAN BB11ARxViterbiWorker(PVOID pContext)
{
    PBB11A_RX_CONTEXT pRxContextA = (PBB11A_RX_CONTEXT)pContext;

    if (BB11A_VITERBIRUN_WAIT_EVENT(pRxContextA))
    {
        BB11A_VITERBIRUN_CLEAR_EVENT(pRxContextA);
        pRxContextA->bCRCCorrect = FALSE;

        switch (pRxContextA->bRate & 0x7)
        {
            case 0x3:
                VitDesCRC6(pRxContextA);
                break;
            case 0x7:
                VitDesCRC9(pRxContextA);
                break;

            case 0x2:
                VitDesCRC12(pRxContextA);
                break;
            case 0x6:
                VitDesCRC18(pRxContextA);
                break;

            case 0x1:
                VitDesCRC24(pRxContextA);
                break;
            case 0x5:
                VitDesCRC36(pRxContextA);
                break;

            case 0x0:
                VitDesCRC48(pRxContextA);
                break;
            case 0x4:
                VitDesCRC54(pRxContextA);
                break;
        }

        _mm_mfence();
        BB11A_VITERBIDONE_SET_EVENT(pRxContextA);
    }
    return TRUE;
}
