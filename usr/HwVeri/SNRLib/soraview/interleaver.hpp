#pragma once

FINL void  DeinterleaveBPSK(unsigned char * pbInput, unsigned char * pbOutput)
{
    pbOutput[0] = pbInput[0];
    pbOutput[1] = pbInput[3];
    pbOutput[2] = pbInput[6];
    pbOutput[3] = pbInput[9];
    pbOutput[4] = pbInput[12];
    pbOutput[5] = pbInput[15];
    pbOutput[6] = pbInput[18];
    pbOutput[7] = pbInput[21];
    pbOutput[8] = pbInput[24];
    pbOutput[9] = pbInput[27];
    pbOutput[10] = pbInput[30];
    pbOutput[11] = pbInput[33];
    pbOutput[12] = pbInput[36];
    pbOutput[13] = pbInput[39];
    pbOutput[14] = pbInput[42];
    pbOutput[15] = pbInput[45];
    pbOutput[16] = pbInput[1];
    pbOutput[17] = pbInput[4];
    pbOutput[18] = pbInput[7];
    pbOutput[19] = pbInput[10];
    pbOutput[20] = pbInput[13];
    pbOutput[21] = pbInput[16];
    pbOutput[22] = pbInput[19];
    pbOutput[23] = pbInput[22];
    pbOutput[24] = pbInput[25];
    pbOutput[25] = pbInput[28];
    pbOutput[26] = pbInput[31];
    pbOutput[27] = pbInput[34];
    pbOutput[28] = pbInput[37];
    pbOutput[29] = pbInput[40];
    pbOutput[30] = pbInput[43];
    pbOutput[31] = pbInput[46];
    pbOutput[32] = pbInput[2];
    pbOutput[33] = pbInput[5];
    pbOutput[34] = pbInput[8];
    pbOutput[35] = pbInput[11];
    pbOutput[36] = pbInput[14];
    pbOutput[37] = pbInput[17];
    pbOutput[38] = pbInput[20];
    pbOutput[39] = pbInput[23];
    pbOutput[40] = pbInput[26];
    pbOutput[41] = pbInput[29];
    pbOutput[42] = pbInput[32];
    pbOutput[43] = pbInput[35];
    pbOutput[44] = pbInput[38];
    pbOutput[45] = pbInput[41];
    pbOutput[46] = pbInput[44];
    pbOutput[47] = pbInput[47];
}

