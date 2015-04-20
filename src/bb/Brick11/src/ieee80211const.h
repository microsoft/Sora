#pragma once 

#define DOT11A_RATE_6M  0xB // 1-011
#define DOT11A_RATE_9M  0XF // 1-111
#define DOT11A_RATE_12M 0xA // 1-010
#define DOT11A_RATE_18M 0xE // 1-110
#define DOT11A_RATE_24M 0x9 // 1-001
#define DOT11A_RATE_36M 0xD // 1-101
#define DOT11A_RATE_48M 0x8 // 1-000
#define DOT11A_RATE_54M 0xC // 1-100

// here defines the coding rate
// CR_12 = 1/2 code
enum CodingRateEnum
{
    CR_12 = 0,
    CR_23,
    CR_34,
    CR_56,
};

static const char LTS_Positive_table [64] = {
    0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 
    1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1
};

struct dot11n_rate_param
{
    int cdbps;
    int ndbps;
};

__declspec(selectany) dot11n_rate_param DOT11N_RATE_PARAMS[16] =
{
    /* MCS 0~7: for single spatial stream*/
    {52, 26},
    {104, 52},
    {104, 78},
    {208, 104},
    {208, 156},
    {312, 208},
    {312, 234},
    {312, 260},
    /* MCS 8~15: for two spatial streams*/
    {104, 52},
    {208, 104},
    {208, 156},
    {416, 208},
    {416, 312},
    {624, 416},
    {624, 468},
    {624, 520},
};

const int service_length = 16;
const int padding = 6;
