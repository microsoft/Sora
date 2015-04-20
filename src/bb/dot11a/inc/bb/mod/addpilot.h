#ifndef BB_MOD_ADDPILOT_H
#define BB_MOD_ADDPILOT_H
 
#define OFDM_ONE (32 * IFFT_LUT_FACTOR16) 

__forceinline
void AddPilot(COMPLEX16 * pcInput, COMPLEX16 * pcOutput, unsigned char bPilotSgn)
{
    unsigned int i;
    for (i = 64 - 26; i < 64; i++)
    {
        if (i == 64 - 7 || i == 64 - 21)
            continue;
        pcOutput[i] = *pcInput;
        pcInput++;
    }

    for (i = 1; i <= 26; i++)
    {
        if (i == 7 || i == 21)
            continue;
        pcOutput[i] = *pcInput;
        pcInput++;
    }

    if (!bPilotSgn)
    {
        pcOutput[7].re = OFDM_ONE;
        pcOutput[7].im = 0;
        pcOutput[21].re = -OFDM_ONE;
        pcOutput[21].im = 0;
        pcOutput[64 - 7].re = OFDM_ONE;
        pcOutput[64 - 7].im = 0;
        pcOutput[64 - 21].re = OFDM_ONE;
        pcOutput[64 - 21].im = 0;
    }
    else
    {        
        pcOutput[7].re = -OFDM_ONE;
        pcOutput[7].im = 0;
        pcOutput[21].re = OFDM_ONE;
        pcOutput[21].im = 0;
        pcOutput[64 - 7].re = -OFDM_ONE;
        pcOutput[64 - 7].im = 0;
        pcOutput[64 - 21].re = -OFDM_ONE;
        pcOutput[64 - 21].im = 0;
    }
}

#endif//BB_MOD_ADDPILOT_H
