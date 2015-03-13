#pragma once
#include "brick.h"

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveBPSK_S0, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveBPSK_S0 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52);
    DEFINE_OPORT(uchar, 52); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveBPSK_S0);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveBPSK_S0)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[0];
        pbOutput[1] = pbInput[4];
        pbOutput[2] = pbInput[8];
        pbOutput[3] = pbInput[12];
        pbOutput[4] = pbInput[16];
        pbOutput[5] = pbInput[20];
        pbOutput[6] = pbInput[24];
        pbOutput[7] = pbInput[28];
        pbOutput[8] = pbInput[32];
        pbOutput[9] = pbInput[36];
        pbOutput[10] = pbInput[40];
        pbOutput[11] = pbInput[44];
        pbOutput[12] = pbInput[48];
        pbOutput[13] = pbInput[1];
        pbOutput[14] = pbInput[5];
        pbOutput[15] = pbInput[9];
        pbOutput[16] = pbInput[13];
        pbOutput[17] = pbInput[17];
        pbOutput[18] = pbInput[21];
        pbOutput[19] = pbInput[25];
        pbOutput[20] = pbInput[29];
        pbOutput[21] = pbInput[33];
        pbOutput[22] = pbInput[37];
        pbOutput[23] = pbInput[41];
        pbOutput[24] = pbInput[45];
        pbOutput[25] = pbInput[49];
        pbOutput[26] = pbInput[2];
        pbOutput[27] = pbInput[6];
        pbOutput[28] = pbInput[10];
        pbOutput[29] = pbInput[14];
        pbOutput[30] = pbInput[18];
        pbOutput[31] = pbInput[22];
        pbOutput[32] = pbInput[26];
        pbOutput[33] = pbInput[30];
        pbOutput[34] = pbInput[34];
        pbOutput[35] = pbInput[38];
        pbOutput[36] = pbInput[42];
        pbOutput[37] = pbInput[46];
        pbOutput[38] = pbInput[50];
        pbOutput[39] = pbInput[3];
        pbOutput[40] = pbInput[7];
        pbOutput[41] = pbInput[11];
        pbOutput[42] = pbInput[15];
        pbOutput[43] = pbInput[19];
        pbOutput[44] = pbInput[23];
        pbOutput[45] = pbInput[27];
        pbOutput[46] = pbInput[31];
        pbOutput[47] = pbInput[35];
        pbOutput[48] = pbInput[39];
        pbOutput[49] = pbInput[43];
        pbOutput[50] = pbInput[47];
        pbOutput[51] = pbInput[51];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveBPSK_S1, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveBPSK_S1 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52);
    DEFINE_OPORT(uchar, 52); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveBPSK_S1);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveBPSK_S1)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[30];
        pbOutput[1] = pbInput[34];
        pbOutput[2] = pbInput[38];
        pbOutput[3] = pbInput[42];
        pbOutput[4] = pbInput[46];
        pbOutput[5] = pbInput[50];
        pbOutput[6] = pbInput[2];
        pbOutput[7] = pbInput[6];
        pbOutput[8] = pbInput[10];
        pbOutput[9] = pbInput[14];
        pbOutput[10] = pbInput[18];
        pbOutput[11] = pbInput[22];
        pbOutput[12] = pbInput[26];
        pbOutput[13] = pbInput[31];
        pbOutput[14] = pbInput[35];
        pbOutput[15] = pbInput[39];
        pbOutput[16] = pbInput[43];
        pbOutput[17] = pbInput[47];
        pbOutput[18] = pbInput[51];
        pbOutput[19] = pbInput[3];
        pbOutput[20] = pbInput[7];
        pbOutput[21] = pbInput[11];
        pbOutput[22] = pbInput[15];
        pbOutput[23] = pbInput[19];
        pbOutput[24] = pbInput[23];
        pbOutput[25] = pbInput[27];
        pbOutput[26] = pbInput[32];
        pbOutput[27] = pbInput[36];
        pbOutput[28] = pbInput[40];
        pbOutput[29] = pbInput[44];
        pbOutput[30] = pbInput[48];
        pbOutput[31] = pbInput[0];
        pbOutput[32] = pbInput[4];
        pbOutput[33] = pbInput[8];
        pbOutput[34] = pbInput[12];
        pbOutput[35] = pbInput[16];
        pbOutput[36] = pbInput[20];
        pbOutput[37] = pbInput[24];
        pbOutput[38] = pbInput[28];
        pbOutput[39] = pbInput[33];
        pbOutput[40] = pbInput[37];
        pbOutput[41] = pbInput[41];
        pbOutput[42] = pbInput[45];
        pbOutput[43] = pbInput[49];
        pbOutput[44] = pbInput[1];
        pbOutput[45] = pbInput[5];
        pbOutput[46] = pbInput[9];
        pbOutput[47] = pbInput[13];
        pbOutput[48] = pbInput[17];
        pbOutput[49] = pbInput[21];
        pbOutput[50] = pbInput[25];
        pbOutput[51] = pbInput[29];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveQPSK_S0, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveQPSK_S0 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52*2);
    DEFINE_OPORT(uchar, 52*2); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveQPSK_S0);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveQPSK_S0)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[0];
        pbOutput[1] = pbInput[8];
        pbOutput[2] = pbInput[16];
        pbOutput[3] = pbInput[24];
        pbOutput[4] = pbInput[32];
        pbOutput[5] = pbInput[40];
        pbOutput[6] = pbInput[48];
        pbOutput[7] = pbInput[56];
        pbOutput[8] = pbInput[64];
        pbOutput[9] = pbInput[72];
        pbOutput[10] = pbInput[80];
        pbOutput[11] = pbInput[88];
        pbOutput[12] = pbInput[96];
        pbOutput[13] = pbInput[1];
        pbOutput[14] = pbInput[9];
        pbOutput[15] = pbInput[17];
        pbOutput[16] = pbInput[25];
        pbOutput[17] = pbInput[33];
        pbOutput[18] = pbInput[41];
        pbOutput[19] = pbInput[49];
        pbOutput[20] = pbInput[57];
        pbOutput[21] = pbInput[65];
        pbOutput[22] = pbInput[73];
        pbOutput[23] = pbInput[81];
        pbOutput[24] = pbInput[89];
        pbOutput[25] = pbInput[97];
        pbOutput[26] = pbInput[2];
        pbOutput[27] = pbInput[10];
        pbOutput[28] = pbInput[18];
        pbOutput[29] = pbInput[26];
        pbOutput[30] = pbInput[34];
        pbOutput[31] = pbInput[42];
        pbOutput[32] = pbInput[50];
        pbOutput[33] = pbInput[58];
        pbOutput[34] = pbInput[66];
        pbOutput[35] = pbInput[74];
        pbOutput[36] = pbInput[82];
        pbOutput[37] = pbInput[90];
        pbOutput[38] = pbInput[98];
        pbOutput[39] = pbInput[3];
        pbOutput[40] = pbInput[11];
        pbOutput[41] = pbInput[19];
        pbOutput[42] = pbInput[27];
        pbOutput[43] = pbInput[35];
        pbOutput[44] = pbInput[43];
        pbOutput[45] = pbInput[51];
        pbOutput[46] = pbInput[59];
        pbOutput[47] = pbInput[67];
        pbOutput[48] = pbInput[75];
        pbOutput[49] = pbInput[83];
        pbOutput[50] = pbInput[91];
        pbOutput[51] = pbInput[99];
        pbOutput[52] = pbInput[4];
        pbOutput[53] = pbInput[12];
        pbOutput[54] = pbInput[20];
        pbOutput[55] = pbInput[28];
        pbOutput[56] = pbInput[36];
        pbOutput[57] = pbInput[44];
        pbOutput[58] = pbInput[52];
        pbOutput[59] = pbInput[60];
        pbOutput[60] = pbInput[68];
        pbOutput[61] = pbInput[76];
        pbOutput[62] = pbInput[84];
        pbOutput[63] = pbInput[92];
        pbOutput[64] = pbInput[100];
        pbOutput[65] = pbInput[5];
        pbOutput[66] = pbInput[13];
        pbOutput[67] = pbInput[21];
        pbOutput[68] = pbInput[29];
        pbOutput[69] = pbInput[37];
        pbOutput[70] = pbInput[45];
        pbOutput[71] = pbInput[53];
        pbOutput[72] = pbInput[61];
        pbOutput[73] = pbInput[69];
        pbOutput[74] = pbInput[77];
        pbOutput[75] = pbInput[85];
        pbOutput[76] = pbInput[93];
        pbOutput[77] = pbInput[101];
        pbOutput[78] = pbInput[6];
        pbOutput[79] = pbInput[14];
        pbOutput[80] = pbInput[22];
        pbOutput[81] = pbInput[30];
        pbOutput[82] = pbInput[38];
        pbOutput[83] = pbInput[46];
        pbOutput[84] = pbInput[54];
        pbOutput[85] = pbInput[62];
        pbOutput[86] = pbInput[70];
        pbOutput[87] = pbInput[78];
        pbOutput[88] = pbInput[86];
        pbOutput[89] = pbInput[94];
        pbOutput[90] = pbInput[102];
        pbOutput[91] = pbInput[7];
        pbOutput[92] = pbInput[15];
        pbOutput[93] = pbInput[23];
        pbOutput[94] = pbInput[31];
        pbOutput[95] = pbInput[39];
        pbOutput[96] = pbInput[47];
        pbOutput[97] = pbInput[55];
        pbOutput[98] = pbInput[63];
        pbOutput[99] = pbInput[71];
        pbOutput[100] = pbInput[79];
        pbOutput[101] = pbInput[87];
        pbOutput[102] = pbInput[95];
        pbOutput[103] = pbInput[103];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveQPSK_S1, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveQPSK_S1 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52*2);
    DEFINE_OPORT(uchar, 52*2); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveQPSK_S1);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveQPSK_S1)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[60];
        pbOutput[1] = pbInput[68];
        pbOutput[2] = pbInput[76];
        pbOutput[3] = pbInput[84];
        pbOutput[4] = pbInput[92];
        pbOutput[5] = pbInput[100];
        pbOutput[6] = pbInput[4];
        pbOutput[7] = pbInput[12];
        pbOutput[8] = pbInput[20];
        pbOutput[9] = pbInput[28];
        pbOutput[10] = pbInput[36];
        pbOutput[11] = pbInput[44];
        pbOutput[12] = pbInput[52];
        pbOutput[13] = pbInput[61];
        pbOutput[14] = pbInput[69];
        pbOutput[15] = pbInput[77];
        pbOutput[16] = pbInput[85];
        pbOutput[17] = pbInput[93];
        pbOutput[18] = pbInput[101];
        pbOutput[19] = pbInput[5];
        pbOutput[20] = pbInput[13];
        pbOutput[21] = pbInput[21];
        pbOutput[22] = pbInput[29];
        pbOutput[23] = pbInput[37];
        pbOutput[24] = pbInput[45];
        pbOutput[25] = pbInput[53];
        pbOutput[26] = pbInput[62];
        pbOutput[27] = pbInput[70];
        pbOutput[28] = pbInput[78];
        pbOutput[29] = pbInput[86];
        pbOutput[30] = pbInput[94];
        pbOutput[31] = pbInput[102];
        pbOutput[32] = pbInput[6];
        pbOutput[33] = pbInput[14];
        pbOutput[34] = pbInput[22];
        pbOutput[35] = pbInput[30];
        pbOutput[36] = pbInput[38];
        pbOutput[37] = pbInput[46];
        pbOutput[38] = pbInput[54];
        pbOutput[39] = pbInput[63];
        pbOutput[40] = pbInput[71];
        pbOutput[41] = pbInput[79];
        pbOutput[42] = pbInput[87];
        pbOutput[43] = pbInput[95];
        pbOutput[44] = pbInput[103];
        pbOutput[45] = pbInput[7];
        pbOutput[46] = pbInput[15];
        pbOutput[47] = pbInput[23];
        pbOutput[48] = pbInput[31];
        pbOutput[49] = pbInput[39];
        pbOutput[50] = pbInput[47];
        pbOutput[51] = pbInput[55];
        pbOutput[52] = pbInput[64];
        pbOutput[53] = pbInput[72];
        pbOutput[54] = pbInput[80];
        pbOutput[55] = pbInput[88];
        pbOutput[56] = pbInput[96];
        pbOutput[57] = pbInput[0];
        pbOutput[58] = pbInput[8];
        pbOutput[59] = pbInput[16];
        pbOutput[60] = pbInput[24];
        pbOutput[61] = pbInput[32];
        pbOutput[62] = pbInput[40];
        pbOutput[63] = pbInput[48];
        pbOutput[64] = pbInput[56];
        pbOutput[65] = pbInput[65];
        pbOutput[66] = pbInput[73];
        pbOutput[67] = pbInput[81];
        pbOutput[68] = pbInput[89];
        pbOutput[69] = pbInput[97];
        pbOutput[70] = pbInput[1];
        pbOutput[71] = pbInput[9];
        pbOutput[72] = pbInput[17];
        pbOutput[73] = pbInput[25];
        pbOutput[74] = pbInput[33];
        pbOutput[75] = pbInput[41];
        pbOutput[76] = pbInput[49];
        pbOutput[77] = pbInput[57];
        pbOutput[78] = pbInput[66];
        pbOutput[79] = pbInput[74];
        pbOutput[80] = pbInput[82];
        pbOutput[81] = pbInput[90];
        pbOutput[82] = pbInput[98];
        pbOutput[83] = pbInput[2];
        pbOutput[84] = pbInput[10];
        pbOutput[85] = pbInput[18];
        pbOutput[86] = pbInput[26];
        pbOutput[87] = pbInput[34];
        pbOutput[88] = pbInput[42];
        pbOutput[89] = pbInput[50];
        pbOutput[90] = pbInput[58];
        pbOutput[91] = pbInput[67];
        pbOutput[92] = pbInput[75];
        pbOutput[93] = pbInput[83];
        pbOutput[94] = pbInput[91];
        pbOutput[95] = pbInput[99];
        pbOutput[96] = pbInput[3];
        pbOutput[97] = pbInput[11];
        pbOutput[98] = pbInput[19];
        pbOutput[99] = pbInput[27];
        pbOutput[100] = pbInput[35];
        pbOutput[101] = pbInput[43];
        pbOutput[102] = pbInput[51];
        pbOutput[103] = pbInput[59];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveQAM16_S0, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveQAM16_S0 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52*4);
    DEFINE_OPORT(uchar, 52*4); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveQAM16_S0);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveQAM16_S0)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[0];
        pbOutput[1] = pbInput[17];
        pbOutput[2] = pbInput[32];
        pbOutput[3] = pbInput[49];
        pbOutput[4] = pbInput[64];
        pbOutput[5] = pbInput[81];
        pbOutput[6] = pbInput[96];
        pbOutput[7] = pbInput[113];
        pbOutput[8] = pbInput[128];
        pbOutput[9] = pbInput[145];
        pbOutput[10] = pbInput[160];
        pbOutput[11] = pbInput[177];
        pbOutput[12] = pbInput[192];
        pbOutput[13] = pbInput[1];
        pbOutput[14] = pbInput[16];
        pbOutput[15] = pbInput[33];
        pbOutput[16] = pbInput[48];
        pbOutput[17] = pbInput[65];
        pbOutput[18] = pbInput[80];
        pbOutput[19] = pbInput[97];
        pbOutput[20] = pbInput[112];
        pbOutput[21] = pbInput[129];
        pbOutput[22] = pbInput[144];
        pbOutput[23] = pbInput[161];
        pbOutput[24] = pbInput[176];
        pbOutput[25] = pbInput[193];
        pbOutput[26] = pbInput[2];
        pbOutput[27] = pbInput[19];
        pbOutput[28] = pbInput[34];
        pbOutput[29] = pbInput[51];
        pbOutput[30] = pbInput[66];
        pbOutput[31] = pbInput[83];
        pbOutput[32] = pbInput[98];
        pbOutput[33] = pbInput[115];
        pbOutput[34] = pbInput[130];
        pbOutput[35] = pbInput[147];
        pbOutput[36] = pbInput[162];
        pbOutput[37] = pbInput[179];
        pbOutput[38] = pbInput[194];
        pbOutput[39] = pbInput[3];
        pbOutput[40] = pbInput[18];
        pbOutput[41] = pbInput[35];
        pbOutput[42] = pbInput[50];
        pbOutput[43] = pbInput[67];
        pbOutput[44] = pbInput[82];
        pbOutput[45] = pbInput[99];
        pbOutput[46] = pbInput[114];
        pbOutput[47] = pbInput[131];
        pbOutput[48] = pbInput[146];
        pbOutput[49] = pbInput[163];
        pbOutput[50] = pbInput[178];
        pbOutput[51] = pbInput[195];
        pbOutput[52] = pbInput[4];
        pbOutput[53] = pbInput[21];
        pbOutput[54] = pbInput[36];
        pbOutput[55] = pbInput[53];
        pbOutput[56] = pbInput[68];
        pbOutput[57] = pbInput[85];
        pbOutput[58] = pbInput[100];
        pbOutput[59] = pbInput[117];
        pbOutput[60] = pbInput[132];
        pbOutput[61] = pbInput[149];
        pbOutput[62] = pbInput[164];
        pbOutput[63] = pbInput[181];
        pbOutput[64] = pbInput[196];
        pbOutput[65] = pbInput[5];
        pbOutput[66] = pbInput[20];
        pbOutput[67] = pbInput[37];
        pbOutput[68] = pbInput[52];
        pbOutput[69] = pbInput[69];
        pbOutput[70] = pbInput[84];
        pbOutput[71] = pbInput[101];
        pbOutput[72] = pbInput[116];
        pbOutput[73] = pbInput[133];
        pbOutput[74] = pbInput[148];
        pbOutput[75] = pbInput[165];
        pbOutput[76] = pbInput[180];
        pbOutput[77] = pbInput[197];
        pbOutput[78] = pbInput[6];
        pbOutput[79] = pbInput[23];
        pbOutput[80] = pbInput[38];
        pbOutput[81] = pbInput[55];
        pbOutput[82] = pbInput[70];
        pbOutput[83] = pbInput[87];
        pbOutput[84] = pbInput[102];
        pbOutput[85] = pbInput[119];
        pbOutput[86] = pbInput[134];
        pbOutput[87] = pbInput[151];
        pbOutput[88] = pbInput[166];
        pbOutput[89] = pbInput[183];
        pbOutput[90] = pbInput[198];
        pbOutput[91] = pbInput[7];
        pbOutput[92] = pbInput[22];
        pbOutput[93] = pbInput[39];
        pbOutput[94] = pbInput[54];
        pbOutput[95] = pbInput[71];
        pbOutput[96] = pbInput[86];
        pbOutput[97] = pbInput[103];
        pbOutput[98] = pbInput[118];
        pbOutput[99] = pbInput[135];
        pbOutput[100] = pbInput[150];
        pbOutput[101] = pbInput[167];
        pbOutput[102] = pbInput[182];
        pbOutput[103] = pbInput[199];
        pbOutput[104] = pbInput[8];
        pbOutput[105] = pbInput[25];
        pbOutput[106] = pbInput[40];
        pbOutput[107] = pbInput[57];
        pbOutput[108] = pbInput[72];
        pbOutput[109] = pbInput[89];
        pbOutput[110] = pbInput[104];
        pbOutput[111] = pbInput[121];
        pbOutput[112] = pbInput[136];
        pbOutput[113] = pbInput[153];
        pbOutput[114] = pbInput[168];
        pbOutput[115] = pbInput[185];
        pbOutput[116] = pbInput[200];
        pbOutput[117] = pbInput[9];
        pbOutput[118] = pbInput[24];
        pbOutput[119] = pbInput[41];
        pbOutput[120] = pbInput[56];
        pbOutput[121] = pbInput[73];
        pbOutput[122] = pbInput[88];
        pbOutput[123] = pbInput[105];
        pbOutput[124] = pbInput[120];
        pbOutput[125] = pbInput[137];
        pbOutput[126] = pbInput[152];
        pbOutput[127] = pbInput[169];
        pbOutput[128] = pbInput[184];
        pbOutput[129] = pbInput[201];
        pbOutput[130] = pbInput[10];
        pbOutput[131] = pbInput[27];
        pbOutput[132] = pbInput[42];
        pbOutput[133] = pbInput[59];
        pbOutput[134] = pbInput[74];
        pbOutput[135] = pbInput[91];
        pbOutput[136] = pbInput[106];
        pbOutput[137] = pbInput[123];
        pbOutput[138] = pbInput[138];
        pbOutput[139] = pbInput[155];
        pbOutput[140] = pbInput[170];
        pbOutput[141] = pbInput[187];
        pbOutput[142] = pbInput[202];
        pbOutput[143] = pbInput[11];
        pbOutput[144] = pbInput[26];
        pbOutput[145] = pbInput[43];
        pbOutput[146] = pbInput[58];
        pbOutput[147] = pbInput[75];
        pbOutput[148] = pbInput[90];
        pbOutput[149] = pbInput[107];
        pbOutput[150] = pbInput[122];
        pbOutput[151] = pbInput[139];
        pbOutput[152] = pbInput[154];
        pbOutput[153] = pbInput[171];
        pbOutput[154] = pbInput[186];
        pbOutput[155] = pbInput[203];
        pbOutput[156] = pbInput[12];
        pbOutput[157] = pbInput[29];
        pbOutput[158] = pbInput[44];
        pbOutput[159] = pbInput[61];
        pbOutput[160] = pbInput[76];
        pbOutput[161] = pbInput[93];
        pbOutput[162] = pbInput[108];
        pbOutput[163] = pbInput[125];
        pbOutput[164] = pbInput[140];
        pbOutput[165] = pbInput[157];
        pbOutput[166] = pbInput[172];
        pbOutput[167] = pbInput[189];
        pbOutput[168] = pbInput[204];
        pbOutput[169] = pbInput[13];
        pbOutput[170] = pbInput[28];
        pbOutput[171] = pbInput[45];
        pbOutput[172] = pbInput[60];
        pbOutput[173] = pbInput[77];
        pbOutput[174] = pbInput[92];
        pbOutput[175] = pbInput[109];
        pbOutput[176] = pbInput[124];
        pbOutput[177] = pbInput[141];
        pbOutput[178] = pbInput[156];
        pbOutput[179] = pbInput[173];
        pbOutput[180] = pbInput[188];
        pbOutput[181] = pbInput[205];
        pbOutput[182] = pbInput[14];
        pbOutput[183] = pbInput[31];
        pbOutput[184] = pbInput[46];
        pbOutput[185] = pbInput[63];
        pbOutput[186] = pbInput[78];
        pbOutput[187] = pbInput[95];
        pbOutput[188] = pbInput[110];
        pbOutput[189] = pbInput[127];
        pbOutput[190] = pbInput[142];
        pbOutput[191] = pbInput[159];
        pbOutput[192] = pbInput[174];
        pbOutput[193] = pbInput[191];
        pbOutput[194] = pbInput[206];
        pbOutput[195] = pbInput[15];
        pbOutput[196] = pbInput[30];
        pbOutput[197] = pbInput[47];
        pbOutput[198] = pbInput[62];
        pbOutput[199] = pbInput[79];
        pbOutput[200] = pbInput[94];
        pbOutput[201] = pbInput[111];
        pbOutput[202] = pbInput[126];
        pbOutput[203] = pbInput[143];
        pbOutput[204] = pbInput[158];
        pbOutput[205] = pbInput[175];
        pbOutput[206] = pbInput[190];
        pbOutput[207] = pbInput[207];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveQAM16_S1, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveQAM16_S1 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52*4);
    DEFINE_OPORT(uchar, 52*4); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveQAM16_S1);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveQAM16_S1)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[120];
        pbOutput[1] = pbInput[137];
        pbOutput[2] = pbInput[152];
        pbOutput[3] = pbInput[169];
        pbOutput[4] = pbInput[184];
        pbOutput[5] = pbInput[201];
        pbOutput[6] = pbInput[8];
        pbOutput[7] = pbInput[25];
        pbOutput[8] = pbInput[40];
        pbOutput[9] = pbInput[57];
        pbOutput[10] = pbInput[72];
        pbOutput[11] = pbInput[89];
        pbOutput[12] = pbInput[104];
        pbOutput[13] = pbInput[121];
        pbOutput[14] = pbInput[136];
        pbOutput[15] = pbInput[153];
        pbOutput[16] = pbInput[168];
        pbOutput[17] = pbInput[185];
        pbOutput[18] = pbInput[200];
        pbOutput[19] = pbInput[9];
        pbOutput[20] = pbInput[24];
        pbOutput[21] = pbInput[41];
        pbOutput[22] = pbInput[56];
        pbOutput[23] = pbInput[73];
        pbOutput[24] = pbInput[88];
        pbOutput[25] = pbInput[105];
        pbOutput[26] = pbInput[122];
        pbOutput[27] = pbInput[139];
        pbOutput[28] = pbInput[154];
        pbOutput[29] = pbInput[171];
        pbOutput[30] = pbInput[186];
        pbOutput[31] = pbInput[203];
        pbOutput[32] = pbInput[10];
        pbOutput[33] = pbInput[27];
        pbOutput[34] = pbInput[42];
        pbOutput[35] = pbInput[59];
        pbOutput[36] = pbInput[74];
        pbOutput[37] = pbInput[91];
        pbOutput[38] = pbInput[106];
        pbOutput[39] = pbInput[123];
        pbOutput[40] = pbInput[138];
        pbOutput[41] = pbInput[155];
        pbOutput[42] = pbInput[170];
        pbOutput[43] = pbInput[187];
        pbOutput[44] = pbInput[202];
        pbOutput[45] = pbInput[11];
        pbOutput[46] = pbInput[26];
        pbOutput[47] = pbInput[43];
        pbOutput[48] = pbInput[58];
        pbOutput[49] = pbInput[75];
        pbOutput[50] = pbInput[90];
        pbOutput[51] = pbInput[107];
        pbOutput[52] = pbInput[124];
        pbOutput[53] = pbInput[141];
        pbOutput[54] = pbInput[156];
        pbOutput[55] = pbInput[173];
        pbOutput[56] = pbInput[188];
        pbOutput[57] = pbInput[205];
        pbOutput[58] = pbInput[12];
        pbOutput[59] = pbInput[29];
        pbOutput[60] = pbInput[44];
        pbOutput[61] = pbInput[61];
        pbOutput[62] = pbInput[76];
        pbOutput[63] = pbInput[93];
        pbOutput[64] = pbInput[108];
        pbOutput[65] = pbInput[125];
        pbOutput[66] = pbInput[140];
        pbOutput[67] = pbInput[157];
        pbOutput[68] = pbInput[172];
        pbOutput[69] = pbInput[189];
        pbOutput[70] = pbInput[204];
        pbOutput[71] = pbInput[13];
        pbOutput[72] = pbInput[28];
        pbOutput[73] = pbInput[45];
        pbOutput[74] = pbInput[60];
        pbOutput[75] = pbInput[77];
        pbOutput[76] = pbInput[92];
        pbOutput[77] = pbInput[109];
        pbOutput[78] = pbInput[126];
        pbOutput[79] = pbInput[143];
        pbOutput[80] = pbInput[158];
        pbOutput[81] = pbInput[175];
        pbOutput[82] = pbInput[190];
        pbOutput[83] = pbInput[207];
        pbOutput[84] = pbInput[14];
        pbOutput[85] = pbInput[31];
        pbOutput[86] = pbInput[46];
        pbOutput[87] = pbInput[63];
        pbOutput[88] = pbInput[78];
        pbOutput[89] = pbInput[95];
        pbOutput[90] = pbInput[110];
        pbOutput[91] = pbInput[127];
        pbOutput[92] = pbInput[142];
        pbOutput[93] = pbInput[159];
        pbOutput[94] = pbInput[174];
        pbOutput[95] = pbInput[191];
        pbOutput[96] = pbInput[206];
        pbOutput[97] = pbInput[15];
        pbOutput[98] = pbInput[30];
        pbOutput[99] = pbInput[47];
        pbOutput[100] = pbInput[62];
        pbOutput[101] = pbInput[79];
        pbOutput[102] = pbInput[94];
        pbOutput[103] = pbInput[111];
        pbOutput[104] = pbInput[128];
        pbOutput[105] = pbInput[145];
        pbOutput[106] = pbInput[160];
        pbOutput[107] = pbInput[177];
        pbOutput[108] = pbInput[192];
        pbOutput[109] = pbInput[1];
        pbOutput[110] = pbInput[16];
        pbOutput[111] = pbInput[33];
        pbOutput[112] = pbInput[48];
        pbOutput[113] = pbInput[65];
        pbOutput[114] = pbInput[80];
        pbOutput[115] = pbInput[97];
        pbOutput[116] = pbInput[112];
        pbOutput[117] = pbInput[129];
        pbOutput[118] = pbInput[144];
        pbOutput[119] = pbInput[161];
        pbOutput[120] = pbInput[176];
        pbOutput[121] = pbInput[193];
        pbOutput[122] = pbInput[0];
        pbOutput[123] = pbInput[17];
        pbOutput[124] = pbInput[32];
        pbOutput[125] = pbInput[49];
        pbOutput[126] = pbInput[64];
        pbOutput[127] = pbInput[81];
        pbOutput[128] = pbInput[96];
        pbOutput[129] = pbInput[113];
        pbOutput[130] = pbInput[130];
        pbOutput[131] = pbInput[147];
        pbOutput[132] = pbInput[162];
        pbOutput[133] = pbInput[179];
        pbOutput[134] = pbInput[194];
        pbOutput[135] = pbInput[3];
        pbOutput[136] = pbInput[18];
        pbOutput[137] = pbInput[35];
        pbOutput[138] = pbInput[50];
        pbOutput[139] = pbInput[67];
        pbOutput[140] = pbInput[82];
        pbOutput[141] = pbInput[99];
        pbOutput[142] = pbInput[114];
        pbOutput[143] = pbInput[131];
        pbOutput[144] = pbInput[146];
        pbOutput[145] = pbInput[163];
        pbOutput[146] = pbInput[178];
        pbOutput[147] = pbInput[195];
        pbOutput[148] = pbInput[2];
        pbOutput[149] = pbInput[19];
        pbOutput[150] = pbInput[34];
        pbOutput[151] = pbInput[51];
        pbOutput[152] = pbInput[66];
        pbOutput[153] = pbInput[83];
        pbOutput[154] = pbInput[98];
        pbOutput[155] = pbInput[115];
        pbOutput[156] = pbInput[132];
        pbOutput[157] = pbInput[149];
        pbOutput[158] = pbInput[164];
        pbOutput[159] = pbInput[181];
        pbOutput[160] = pbInput[196];
        pbOutput[161] = pbInput[5];
        pbOutput[162] = pbInput[20];
        pbOutput[163] = pbInput[37];
        pbOutput[164] = pbInput[52];
        pbOutput[165] = pbInput[69];
        pbOutput[166] = pbInput[84];
        pbOutput[167] = pbInput[101];
        pbOutput[168] = pbInput[116];
        pbOutput[169] = pbInput[133];
        pbOutput[170] = pbInput[148];
        pbOutput[171] = pbInput[165];
        pbOutput[172] = pbInput[180];
        pbOutput[173] = pbInput[197];
        pbOutput[174] = pbInput[4];
        pbOutput[175] = pbInput[21];
        pbOutput[176] = pbInput[36];
        pbOutput[177] = pbInput[53];
        pbOutput[178] = pbInput[68];
        pbOutput[179] = pbInput[85];
        pbOutput[180] = pbInput[100];
        pbOutput[181] = pbInput[117];
        pbOutput[182] = pbInput[134];
        pbOutput[183] = pbInput[151];
        pbOutput[184] = pbInput[166];
        pbOutput[185] = pbInput[183];
        pbOutput[186] = pbInput[198];
        pbOutput[187] = pbInput[7];
        pbOutput[188] = pbInput[22];
        pbOutput[189] = pbInput[39];
        pbOutput[190] = pbInput[54];
        pbOutput[191] = pbInput[71];
        pbOutput[192] = pbInput[86];
        pbOutput[193] = pbInput[103];
        pbOutput[194] = pbInput[118];
        pbOutput[195] = pbInput[135];
        pbOutput[196] = pbInput[150];
        pbOutput[197] = pbInput[167];
        pbOutput[198] = pbInput[182];
        pbOutput[199] = pbInput[199];
        pbOutput[200] = pbInput[6];
        pbOutput[201] = pbInput[23];
        pbOutput[202] = pbInput[38];
        pbOutput[203] = pbInput[55];
        pbOutput[204] = pbInput[70];
        pbOutput[205] = pbInput[87];
        pbOutput[206] = pbInput[102];
        pbOutput[207] = pbInput[119];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveQAM64_S0, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveQAM64_S0 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52*6);
    DEFINE_OPORT(uchar, 52*6); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveQAM64_S0);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveQAM64_S0)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[0];
        pbOutput[1] = pbInput[26];
        pbOutput[2] = pbInput[49];
        pbOutput[3] = pbInput[72];
        pbOutput[4] = pbInput[98];
        pbOutput[5] = pbInput[121];
        pbOutput[6] = pbInput[144];
        pbOutput[7] = pbInput[170];
        pbOutput[8] = pbInput[193];
        pbOutput[9] = pbInput[216];
        pbOutput[10] = pbInput[242];
        pbOutput[11] = pbInput[265];
        pbOutput[12] = pbInput[288];
        pbOutput[13] = pbInput[1];
        pbOutput[14] = pbInput[24];
        pbOutput[15] = pbInput[50];
        pbOutput[16] = pbInput[73];
        pbOutput[17] = pbInput[96];
        pbOutput[18] = pbInput[122];
        pbOutput[19] = pbInput[145];
        pbOutput[20] = pbInput[168];
        pbOutput[21] = pbInput[194];
        pbOutput[22] = pbInput[217];
        pbOutput[23] = pbInput[240];
        pbOutput[24] = pbInput[266];
        pbOutput[25] = pbInput[289];
        pbOutput[26] = pbInput[2];
        pbOutput[27] = pbInput[25];
        pbOutput[28] = pbInput[48];
        pbOutput[29] = pbInput[74];
        pbOutput[30] = pbInput[97];
        pbOutput[31] = pbInput[120];
        pbOutput[32] = pbInput[146];
        pbOutput[33] = pbInput[169];
        pbOutput[34] = pbInput[192];
        pbOutput[35] = pbInput[218];
        pbOutput[36] = pbInput[241];
        pbOutput[37] = pbInput[264];
        pbOutput[38] = pbInput[290];
        pbOutput[39] = pbInput[3];
        pbOutput[40] = pbInput[29];
        pbOutput[41] = pbInput[52];
        pbOutput[42] = pbInput[75];
        pbOutput[43] = pbInput[101];
        pbOutput[44] = pbInput[124];
        pbOutput[45] = pbInput[147];
        pbOutput[46] = pbInput[173];
        pbOutput[47] = pbInput[196];
        pbOutput[48] = pbInput[219];
        pbOutput[49] = pbInput[245];
        pbOutput[50] = pbInput[268];
        pbOutput[51] = pbInput[291];
        pbOutput[52] = pbInput[4];
        pbOutput[53] = pbInput[27];
        pbOutput[54] = pbInput[53];
        pbOutput[55] = pbInput[76];
        pbOutput[56] = pbInput[99];
        pbOutput[57] = pbInput[125];
        pbOutput[58] = pbInput[148];
        pbOutput[59] = pbInput[171];
        pbOutput[60] = pbInput[197];
        pbOutput[61] = pbInput[220];
        pbOutput[62] = pbInput[243];
        pbOutput[63] = pbInput[269];
        pbOutput[64] = pbInput[292];
        pbOutput[65] = pbInput[5];
        pbOutput[66] = pbInput[28];
        pbOutput[67] = pbInput[51];
        pbOutput[68] = pbInput[77];
        pbOutput[69] = pbInput[100];
        pbOutput[70] = pbInput[123];
        pbOutput[71] = pbInput[149];
        pbOutput[72] = pbInput[172];
        pbOutput[73] = pbInput[195];
        pbOutput[74] = pbInput[221];
        pbOutput[75] = pbInput[244];
        pbOutput[76] = pbInput[267];
        pbOutput[77] = pbInput[293];
        pbOutput[78] = pbInput[6];
        pbOutput[79] = pbInput[32];
        pbOutput[80] = pbInput[55];
        pbOutput[81] = pbInput[78];
        pbOutput[82] = pbInput[104];
        pbOutput[83] = pbInput[127];
        pbOutput[84] = pbInput[150];
        pbOutput[85] = pbInput[176];
        pbOutput[86] = pbInput[199];
        pbOutput[87] = pbInput[222];
        pbOutput[88] = pbInput[248];
        pbOutput[89] = pbInput[271];
        pbOutput[90] = pbInput[294];
        pbOutput[91] = pbInput[7];
        pbOutput[92] = pbInput[30];
        pbOutput[93] = pbInput[56];
        pbOutput[94] = pbInput[79];
        pbOutput[95] = pbInput[102];
        pbOutput[96] = pbInput[128];
        pbOutput[97] = pbInput[151];
        pbOutput[98] = pbInput[174];
        pbOutput[99] = pbInput[200];
        pbOutput[100] = pbInput[223];
        pbOutput[101] = pbInput[246];
        pbOutput[102] = pbInput[272];
        pbOutput[103] = pbInput[295];
        pbOutput[104] = pbInput[8];
        pbOutput[105] = pbInput[31];
        pbOutput[106] = pbInput[54];
        pbOutput[107] = pbInput[80];
        pbOutput[108] = pbInput[103];
        pbOutput[109] = pbInput[126];
        pbOutput[110] = pbInput[152];
        pbOutput[111] = pbInput[175];
        pbOutput[112] = pbInput[198];
        pbOutput[113] = pbInput[224];
        pbOutput[114] = pbInput[247];
        pbOutput[115] = pbInput[270];
        pbOutput[116] = pbInput[296];
        pbOutput[117] = pbInput[9];
        pbOutput[118] = pbInput[35];
        pbOutput[119] = pbInput[58];
        pbOutput[120] = pbInput[81];
        pbOutput[121] = pbInput[107];
        pbOutput[122] = pbInput[130];
        pbOutput[123] = pbInput[153];
        pbOutput[124] = pbInput[179];
        pbOutput[125] = pbInput[202];
        pbOutput[126] = pbInput[225];
        pbOutput[127] = pbInput[251];
        pbOutput[128] = pbInput[274];
        pbOutput[129] = pbInput[297];
        pbOutput[130] = pbInput[10];
        pbOutput[131] = pbInput[33];
        pbOutput[132] = pbInput[59];
        pbOutput[133] = pbInput[82];
        pbOutput[134] = pbInput[105];
        pbOutput[135] = pbInput[131];
        pbOutput[136] = pbInput[154];
        pbOutput[137] = pbInput[177];
        pbOutput[138] = pbInput[203];
        pbOutput[139] = pbInput[226];
        pbOutput[140] = pbInput[249];
        pbOutput[141] = pbInput[275];
        pbOutput[142] = pbInput[298];
        pbOutput[143] = pbInput[11];
        pbOutput[144] = pbInput[34];
        pbOutput[145] = pbInput[57];
        pbOutput[146] = pbInput[83];
        pbOutput[147] = pbInput[106];
        pbOutput[148] = pbInput[129];
        pbOutput[149] = pbInput[155];
        pbOutput[150] = pbInput[178];
        pbOutput[151] = pbInput[201];
        pbOutput[152] = pbInput[227];
        pbOutput[153] = pbInput[250];
        pbOutput[154] = pbInput[273];
        pbOutput[155] = pbInput[299];
        pbOutput[156] = pbInput[12];
        pbOutput[157] = pbInput[38];
        pbOutput[158] = pbInput[61];
        pbOutput[159] = pbInput[84];
        pbOutput[160] = pbInput[110];
        pbOutput[161] = pbInput[133];
        pbOutput[162] = pbInput[156];
        pbOutput[163] = pbInput[182];
        pbOutput[164] = pbInput[205];
        pbOutput[165] = pbInput[228];
        pbOutput[166] = pbInput[254];
        pbOutput[167] = pbInput[277];
        pbOutput[168] = pbInput[300];
        pbOutput[169] = pbInput[13];
        pbOutput[170] = pbInput[36];
        pbOutput[171] = pbInput[62];
        pbOutput[172] = pbInput[85];
        pbOutput[173] = pbInput[108];
        pbOutput[174] = pbInput[134];
        pbOutput[175] = pbInput[157];
        pbOutput[176] = pbInput[180];
        pbOutput[177] = pbInput[206];
        pbOutput[178] = pbInput[229];
        pbOutput[179] = pbInput[252];
        pbOutput[180] = pbInput[278];
        pbOutput[181] = pbInput[301];
        pbOutput[182] = pbInput[14];
        pbOutput[183] = pbInput[37];
        pbOutput[184] = pbInput[60];
        pbOutput[185] = pbInput[86];
        pbOutput[186] = pbInput[109];
        pbOutput[187] = pbInput[132];
        pbOutput[188] = pbInput[158];
        pbOutput[189] = pbInput[181];
        pbOutput[190] = pbInput[204];
        pbOutput[191] = pbInput[230];
        pbOutput[192] = pbInput[253];
        pbOutput[193] = pbInput[276];
        pbOutput[194] = pbInput[302];
        pbOutput[195] = pbInput[15];
        pbOutput[196] = pbInput[41];
        pbOutput[197] = pbInput[64];
        pbOutput[198] = pbInput[87];
        pbOutput[199] = pbInput[113];
        pbOutput[200] = pbInput[136];
        pbOutput[201] = pbInput[159];
        pbOutput[202] = pbInput[185];
        pbOutput[203] = pbInput[208];
        pbOutput[204] = pbInput[231];
        pbOutput[205] = pbInput[257];
        pbOutput[206] = pbInput[280];
        pbOutput[207] = pbInput[303];
        pbOutput[208] = pbInput[16];
        pbOutput[209] = pbInput[39];
        pbOutput[210] = pbInput[65];
        pbOutput[211] = pbInput[88];
        pbOutput[212] = pbInput[111];
        pbOutput[213] = pbInput[137];
        pbOutput[214] = pbInput[160];
        pbOutput[215] = pbInput[183];
        pbOutput[216] = pbInput[209];
        pbOutput[217] = pbInput[232];
        pbOutput[218] = pbInput[255];
        pbOutput[219] = pbInput[281];
        pbOutput[220] = pbInput[304];
        pbOutput[221] = pbInput[17];
        pbOutput[222] = pbInput[40];
        pbOutput[223] = pbInput[63];
        pbOutput[224] = pbInput[89];
        pbOutput[225] = pbInput[112];
        pbOutput[226] = pbInput[135];
        pbOutput[227] = pbInput[161];
        pbOutput[228] = pbInput[184];
        pbOutput[229] = pbInput[207];
        pbOutput[230] = pbInput[233];
        pbOutput[231] = pbInput[256];
        pbOutput[232] = pbInput[279];
        pbOutput[233] = pbInput[305];
        pbOutput[234] = pbInput[18];
        pbOutput[235] = pbInput[44];
        pbOutput[236] = pbInput[67];
        pbOutput[237] = pbInput[90];
        pbOutput[238] = pbInput[116];
        pbOutput[239] = pbInput[139];
        pbOutput[240] = pbInput[162];
        pbOutput[241] = pbInput[188];
        pbOutput[242] = pbInput[211];
        pbOutput[243] = pbInput[234];
        pbOutput[244] = pbInput[260];
        pbOutput[245] = pbInput[283];
        pbOutput[246] = pbInput[306];
        pbOutput[247] = pbInput[19];
        pbOutput[248] = pbInput[42];
        pbOutput[249] = pbInput[68];
        pbOutput[250] = pbInput[91];
        pbOutput[251] = pbInput[114];
        pbOutput[252] = pbInput[140];
        pbOutput[253] = pbInput[163];
        pbOutput[254] = pbInput[186];
        pbOutput[255] = pbInput[212];
        pbOutput[256] = pbInput[235];
        pbOutput[257] = pbInput[258];
        pbOutput[258] = pbInput[284];
        pbOutput[259] = pbInput[307];
        pbOutput[260] = pbInput[20];
        pbOutput[261] = pbInput[43];
        pbOutput[262] = pbInput[66];
        pbOutput[263] = pbInput[92];
        pbOutput[264] = pbInput[115];
        pbOutput[265] = pbInput[138];
        pbOutput[266] = pbInput[164];
        pbOutput[267] = pbInput[187];
        pbOutput[268] = pbInput[210];
        pbOutput[269] = pbInput[236];
        pbOutput[270] = pbInput[259];
        pbOutput[271] = pbInput[282];
        pbOutput[272] = pbInput[308];
        pbOutput[273] = pbInput[21];
        pbOutput[274] = pbInput[47];
        pbOutput[275] = pbInput[70];
        pbOutput[276] = pbInput[93];
        pbOutput[277] = pbInput[119];
        pbOutput[278] = pbInput[142];
        pbOutput[279] = pbInput[165];
        pbOutput[280] = pbInput[191];
        pbOutput[281] = pbInput[214];
        pbOutput[282] = pbInput[237];
        pbOutput[283] = pbInput[263];
        pbOutput[284] = pbInput[286];
        pbOutput[285] = pbInput[309];
        pbOutput[286] = pbInput[22];
        pbOutput[287] = pbInput[45];
        pbOutput[288] = pbInput[71];
        pbOutput[289] = pbInput[94];
        pbOutput[290] = pbInput[117];
        pbOutput[291] = pbInput[143];
        pbOutput[292] = pbInput[166];
        pbOutput[293] = pbInput[189];
        pbOutput[294] = pbInput[215];
        pbOutput[295] = pbInput[238];
        pbOutput[296] = pbInput[261];
        pbOutput[297] = pbInput[287];
        pbOutput[298] = pbInput[310];
        pbOutput[299] = pbInput[23];
        pbOutput[300] = pbInput[46];
        pbOutput[301] = pbInput[69];
        pbOutput[302] = pbInput[95];
        pbOutput[303] = pbInput[118];
        pbOutput[304] = pbInput[141];
        pbOutput[305] = pbInput[167];
        pbOutput[306] = pbInput[190];
        pbOutput[307] = pbInput[213];
        pbOutput[308] = pbInput[239];
        pbOutput[309] = pbInput[262];
        pbOutput[310] = pbInput[285];
        pbOutput[311] = pbInput[311];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};

DEFINE_LOCAL_CONTEXT(T11nDeinterleaveQAM64_S1, CF_VOID);
template<TFILTER_ARGS>
class T11nDeinterleaveQAM64_S1 : public TFilter<TFILTER_PARAMS>
{
public:
    DEFINE_IPORT(uchar, 52*6);
    DEFINE_OPORT(uchar, 52*6); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDeinterleaveQAM64_S1);

    STD_TFILTER_CONSTRUCTOR(T11nDeinterleaveQAM64_S1)
    { }

	FINL void deinterleaver ( const uchar * pbInput, uchar* pbOutput ) {
        pbOutput[0] = pbInput[180];
        pbOutput[1] = pbInput[206];
        pbOutput[2] = pbInput[229];
        pbOutput[3] = pbInput[252];
        pbOutput[4] = pbInput[278];
        pbOutput[5] = pbInput[301];
        pbOutput[6] = pbInput[12];
        pbOutput[7] = pbInput[38];
        pbOutput[8] = pbInput[61];
        pbOutput[9] = pbInput[84];
        pbOutput[10] = pbInput[110];
        pbOutput[11] = pbInput[133];
        pbOutput[12] = pbInput[156];
        pbOutput[13] = pbInput[181];
        pbOutput[14] = pbInput[204];
        pbOutput[15] = pbInput[230];
        pbOutput[16] = pbInput[253];
        pbOutput[17] = pbInput[276];
        pbOutput[18] = pbInput[302];
        pbOutput[19] = pbInput[13];
        pbOutput[20] = pbInput[36];
        pbOutput[21] = pbInput[62];
        pbOutput[22] = pbInput[85];
        pbOutput[23] = pbInput[108];
        pbOutput[24] = pbInput[134];
        pbOutput[25] = pbInput[157];
        pbOutput[26] = pbInput[182];
        pbOutput[27] = pbInput[205];
        pbOutput[28] = pbInput[228];
        pbOutput[29] = pbInput[254];
        pbOutput[30] = pbInput[277];
        pbOutput[31] = pbInput[300];
        pbOutput[32] = pbInput[14];
        pbOutput[33] = pbInput[37];
        pbOutput[34] = pbInput[60];
        pbOutput[35] = pbInput[86];
        pbOutput[36] = pbInput[109];
        pbOutput[37] = pbInput[132];
        pbOutput[38] = pbInput[158];
        pbOutput[39] = pbInput[183];
        pbOutput[40] = pbInput[209];
        pbOutput[41] = pbInput[232];
        pbOutput[42] = pbInput[255];
        pbOutput[43] = pbInput[281];
        pbOutput[44] = pbInput[304];
        pbOutput[45] = pbInput[15];
        pbOutput[46] = pbInput[41];
        pbOutput[47] = pbInput[64];
        pbOutput[48] = pbInput[87];
        pbOutput[49] = pbInput[113];
        pbOutput[50] = pbInput[136];
        pbOutput[51] = pbInput[159];
        pbOutput[52] = pbInput[184];
        pbOutput[53] = pbInput[207];
        pbOutput[54] = pbInput[233];
        pbOutput[55] = pbInput[256];
        pbOutput[56] = pbInput[279];
        pbOutput[57] = pbInput[305];
        pbOutput[58] = pbInput[16];
        pbOutput[59] = pbInput[39];
        pbOutput[60] = pbInput[65];
        pbOutput[61] = pbInput[88];
        pbOutput[62] = pbInput[111];
        pbOutput[63] = pbInput[137];
        pbOutput[64] = pbInput[160];
        pbOutput[65] = pbInput[185];
        pbOutput[66] = pbInput[208];
        pbOutput[67] = pbInput[231];
        pbOutput[68] = pbInput[257];
        pbOutput[69] = pbInput[280];
        pbOutput[70] = pbInput[303];
        pbOutput[71] = pbInput[17];
        pbOutput[72] = pbInput[40];
        pbOutput[73] = pbInput[63];
        pbOutput[74] = pbInput[89];
        pbOutput[75] = pbInput[112];
        pbOutput[76] = pbInput[135];
        pbOutput[77] = pbInput[161];
        pbOutput[78] = pbInput[186];
        pbOutput[79] = pbInput[212];
        pbOutput[80] = pbInput[235];
        pbOutput[81] = pbInput[258];
        pbOutput[82] = pbInput[284];
        pbOutput[83] = pbInput[307];
        pbOutput[84] = pbInput[18];
        pbOutput[85] = pbInput[44];
        pbOutput[86] = pbInput[67];
        pbOutput[87] = pbInput[90];
        pbOutput[88] = pbInput[116];
        pbOutput[89] = pbInput[139];
        pbOutput[90] = pbInput[162];
        pbOutput[91] = pbInput[187];
        pbOutput[92] = pbInput[210];
        pbOutput[93] = pbInput[236];
        pbOutput[94] = pbInput[259];
        pbOutput[95] = pbInput[282];
        pbOutput[96] = pbInput[308];
        pbOutput[97] = pbInput[19];
        pbOutput[98] = pbInput[42];
        pbOutput[99] = pbInput[68];
        pbOutput[100] = pbInput[91];
        pbOutput[101] = pbInput[114];
        pbOutput[102] = pbInput[140];
        pbOutput[103] = pbInput[163];
        pbOutput[104] = pbInput[188];
        pbOutput[105] = pbInput[211];
        pbOutput[106] = pbInput[234];
        pbOutput[107] = pbInput[260];
        pbOutput[108] = pbInput[283];
        pbOutput[109] = pbInput[306];
        pbOutput[110] = pbInput[20];
        pbOutput[111] = pbInput[43];
        pbOutput[112] = pbInput[66];
        pbOutput[113] = pbInput[92];
        pbOutput[114] = pbInput[115];
        pbOutput[115] = pbInput[138];
        pbOutput[116] = pbInput[164];
        pbOutput[117] = pbInput[189];
        pbOutput[118] = pbInput[215];
        pbOutput[119] = pbInput[238];
        pbOutput[120] = pbInput[261];
        pbOutput[121] = pbInput[287];
        pbOutput[122] = pbInput[310];
        pbOutput[123] = pbInput[21];
        pbOutput[124] = pbInput[47];
        pbOutput[125] = pbInput[70];
        pbOutput[126] = pbInput[93];
        pbOutput[127] = pbInput[119];
        pbOutput[128] = pbInput[142];
        pbOutput[129] = pbInput[165];
        pbOutput[130] = pbInput[190];
        pbOutput[131] = pbInput[213];
        pbOutput[132] = pbInput[239];
        pbOutput[133] = pbInput[262];
        pbOutput[134] = pbInput[285];
        pbOutput[135] = pbInput[311];
        pbOutput[136] = pbInput[22];
        pbOutput[137] = pbInput[45];
        pbOutput[138] = pbInput[71];
        pbOutput[139] = pbInput[94];
        pbOutput[140] = pbInput[117];
        pbOutput[141] = pbInput[143];
        pbOutput[142] = pbInput[166];
        pbOutput[143] = pbInput[191];
        pbOutput[144] = pbInput[214];
        pbOutput[145] = pbInput[237];
        pbOutput[146] = pbInput[263];
        pbOutput[147] = pbInput[286];
        pbOutput[148] = pbInput[309];
        pbOutput[149] = pbInput[23];
        pbOutput[150] = pbInput[46];
        pbOutput[151] = pbInput[69];
        pbOutput[152] = pbInput[95];
        pbOutput[153] = pbInput[118];
        pbOutput[154] = pbInput[141];
        pbOutput[155] = pbInput[167];
        pbOutput[156] = pbInput[192];
        pbOutput[157] = pbInput[218];
        pbOutput[158] = pbInput[241];
        pbOutput[159] = pbInput[264];
        pbOutput[160] = pbInput[290];
        pbOutput[161] = pbInput[1];
        pbOutput[162] = pbInput[24];
        pbOutput[163] = pbInput[50];
        pbOutput[164] = pbInput[73];
        pbOutput[165] = pbInput[96];
        pbOutput[166] = pbInput[122];
        pbOutput[167] = pbInput[145];
        pbOutput[168] = pbInput[168];
        pbOutput[169] = pbInput[193];
        pbOutput[170] = pbInput[216];
        pbOutput[171] = pbInput[242];
        pbOutput[172] = pbInput[265];
        pbOutput[173] = pbInput[288];
        pbOutput[174] = pbInput[2];
        pbOutput[175] = pbInput[25];
        pbOutput[176] = pbInput[48];
        pbOutput[177] = pbInput[74];
        pbOutput[178] = pbInput[97];
        pbOutput[179] = pbInput[120];
        pbOutput[180] = pbInput[146];
        pbOutput[181] = pbInput[169];
        pbOutput[182] = pbInput[194];
        pbOutput[183] = pbInput[217];
        pbOutput[184] = pbInput[240];
        pbOutput[185] = pbInput[266];
        pbOutput[186] = pbInput[289];
        pbOutput[187] = pbInput[0];
        pbOutput[188] = pbInput[26];
        pbOutput[189] = pbInput[49];
        pbOutput[190] = pbInput[72];
        pbOutput[191] = pbInput[98];
        pbOutput[192] = pbInput[121];
        pbOutput[193] = pbInput[144];
        pbOutput[194] = pbInput[170];
        pbOutput[195] = pbInput[195];
        pbOutput[196] = pbInput[221];
        pbOutput[197] = pbInput[244];
        pbOutput[198] = pbInput[267];
        pbOutput[199] = pbInput[293];
        pbOutput[200] = pbInput[4];
        pbOutput[201] = pbInput[27];
        pbOutput[202] = pbInput[53];
        pbOutput[203] = pbInput[76];
        pbOutput[204] = pbInput[99];
        pbOutput[205] = pbInput[125];
        pbOutput[206] = pbInput[148];
        pbOutput[207] = pbInput[171];
        pbOutput[208] = pbInput[196];
        pbOutput[209] = pbInput[219];
        pbOutput[210] = pbInput[245];
        pbOutput[211] = pbInput[268];
        pbOutput[212] = pbInput[291];
        pbOutput[213] = pbInput[5];
        pbOutput[214] = pbInput[28];
        pbOutput[215] = pbInput[51];
        pbOutput[216] = pbInput[77];
        pbOutput[217] = pbInput[100];
        pbOutput[218] = pbInput[123];
        pbOutput[219] = pbInput[149];
        pbOutput[220] = pbInput[172];
        pbOutput[221] = pbInput[197];
        pbOutput[222] = pbInput[220];
        pbOutput[223] = pbInput[243];
        pbOutput[224] = pbInput[269];
        pbOutput[225] = pbInput[292];
        pbOutput[226] = pbInput[3];
        pbOutput[227] = pbInput[29];
        pbOutput[228] = pbInput[52];
        pbOutput[229] = pbInput[75];
        pbOutput[230] = pbInput[101];
        pbOutput[231] = pbInput[124];
        pbOutput[232] = pbInput[147];
        pbOutput[233] = pbInput[173];
        pbOutput[234] = pbInput[198];
        pbOutput[235] = pbInput[224];
        pbOutput[236] = pbInput[247];
        pbOutput[237] = pbInput[270];
        pbOutput[238] = pbInput[296];
        pbOutput[239] = pbInput[7];
        pbOutput[240] = pbInput[30];
        pbOutput[241] = pbInput[56];
        pbOutput[242] = pbInput[79];
        pbOutput[243] = pbInput[102];
        pbOutput[244] = pbInput[128];
        pbOutput[245] = pbInput[151];
        pbOutput[246] = pbInput[174];
        pbOutput[247] = pbInput[199];
        pbOutput[248] = pbInput[222];
        pbOutput[249] = pbInput[248];
        pbOutput[250] = pbInput[271];
        pbOutput[251] = pbInput[294];
        pbOutput[252] = pbInput[8];
        pbOutput[253] = pbInput[31];
        pbOutput[254] = pbInput[54];
        pbOutput[255] = pbInput[80];
        pbOutput[256] = pbInput[103];
        pbOutput[257] = pbInput[126];
        pbOutput[258] = pbInput[152];
        pbOutput[259] = pbInput[175];
        pbOutput[260] = pbInput[200];
        pbOutput[261] = pbInput[223];
        pbOutput[262] = pbInput[246];
        pbOutput[263] = pbInput[272];
        pbOutput[264] = pbInput[295];
        pbOutput[265] = pbInput[6];
        pbOutput[266] = pbInput[32];
        pbOutput[267] = pbInput[55];
        pbOutput[268] = pbInput[78];
        pbOutput[269] = pbInput[104];
        pbOutput[270] = pbInput[127];
        pbOutput[271] = pbInput[150];
        pbOutput[272] = pbInput[176];
        pbOutput[273] = pbInput[201];
        pbOutput[274] = pbInput[227];
        pbOutput[275] = pbInput[250];
        pbOutput[276] = pbInput[273];
        pbOutput[277] = pbInput[299];
        pbOutput[278] = pbInput[10];
        pbOutput[279] = pbInput[33];
        pbOutput[280] = pbInput[59];
        pbOutput[281] = pbInput[82];
        pbOutput[282] = pbInput[105];
        pbOutput[283] = pbInput[131];
        pbOutput[284] = pbInput[154];
        pbOutput[285] = pbInput[177];
        pbOutput[286] = pbInput[202];
        pbOutput[287] = pbInput[225];
        pbOutput[288] = pbInput[251];
        pbOutput[289] = pbInput[274];
        pbOutput[290] = pbInput[297];
        pbOutput[291] = pbInput[11];
        pbOutput[292] = pbInput[34];
        pbOutput[293] = pbInput[57];
        pbOutput[294] = pbInput[83];
        pbOutput[295] = pbInput[106];
        pbOutput[296] = pbInput[129];
        pbOutput[297] = pbInput[155];
        pbOutput[298] = pbInput[178];
        pbOutput[299] = pbInput[203];
        pbOutput[300] = pbInput[226];
        pbOutput[301] = pbInput[249];
        pbOutput[302] = pbInput[275];
        pbOutput[303] = pbInput[298];
        pbOutput[304] = pbInput[9];
        pbOutput[305] = pbInput[35];
        pbOutput[306] = pbInput[58];
        pbOutput[307] = pbInput[81];
        pbOutput[308] = pbInput[107];
        pbOutput[309] = pbInput[130];
        pbOutput[310] = pbInput[153];
        pbOutput[311] = pbInput[179];
	}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const uchar*  input = ipin.peek();
            uchar*  output = opin().append();

			deinterleaver ( input, output );

            ipin.pop();
			Next()->Process(opin());
        }
        return true;
    }
};
