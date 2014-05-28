#pragma once
#include <assert.h>
#include "bbb.h"
#include "bba.h"

typedef struct _TXMODE
{
    // data rate number in kbps or Mbps
    unsigned int bps;
    // data rate code used in protocol
    unsigned int code;
} TXMODE;

const TXMODE __DataRateKbps_Dot11A[] =
{
    { 6000, DOT11A_RATE_6M },
    { 9000, DOT11A_RATE_9M },
    { 12000, DOT11A_RATE_12M },
    { 18000, DOT11A_RATE_18M },
    { 24000, DOT11A_RATE_24M },
    { 36000, DOT11A_RATE_36M },
    { 48000, DOT11A_RATE_48M },
    { 54000, DOT11A_RATE_54M },
};

const TXMODE __DataRateMbps_Dot11A[] =
{
    { 6, DOT11A_RATE_6M },
    { 9, DOT11A_RATE_9M },
    { 12, DOT11A_RATE_12M },
    { 18, DOT11A_RATE_18M },
    { 24, DOT11A_RATE_24M },
    { 36, DOT11A_RATE_36M },
    { 48, DOT11A_RATE_48M },
    { 54, DOT11A_RATE_54M },
};

const TXMODE __DataRateKbps_Dot11B[] =
{
    { 1000,    DOT11B_PLCP_DATA_RATE_1M },
    { 2000,    DOT11B_PLCP_DATA_RATE_2M },
    { 5500,    DOT11B_PLCP_DATA_RATE_5P5M },
    { 11000,   DOT11B_PLCP_DATA_RATE_11M },
};

template<size_t TXMODE_COUNT>
bool __TXMODE_bps_valid(const TXMODE (&txModes)[TXMODE_COUNT], unsigned int bps)
{
    int i;
    for (i = 0; i < TXMODE_COUNT; i++)
    {
        if (txModes[i].bps == bps)
        {
            return true;
        }
    }
    return false;
}

template<size_t TXMODE_COUNT>
unsigned int __TXMODE_bps2code(const TXMODE (&txModes)[TXMODE_COUNT], unsigned int bps)
{
    int i;
    for (i = 0; i < TXMODE_COUNT; i++)
    {
        if (txModes[i].bps == bps)
        {
            return txModes[i].code;
        }
    }

    assert(0);
    return txModes[0].code;
}

template<size_t TXMODE_COUNT>
int __TXMODE_bps2index(const TXMODE (&txModes)[TXMODE_COUNT], unsigned int bps)
{
    int i;
    for (i = 0; i < TXMODE_COUNT; i++)
    {
        if (txModes[i].bps == bps)
        {
            return i;
        }
    }

    assert(0);
    return -1;
}

template<size_t TXMODE_COUNT>
int __TXMODE_code2index(const TXMODE (&txModes)[TXMODE_COUNT], unsigned int code)
{
    int i;
    for (i = 0; i < TXMODE_COUNT; i++)
    {
        if (txModes[i].code == code)
        {
            return i;
        }
    }

    assert(0);
    return -1;
}

template<size_t TXMODE_COUNT>
unsigned int __TXMODE_code2bps(const TXMODE (&txModes)[TXMODE_COUNT], unsigned int nDataRate)
{
    int i;
    for (i = 0; i < TXMODE_COUNT; i++)
    {
        if (txModes[i].code == nDataRate)
        {
            return txModes[i].bps;
        }
    }

    assert(0);
    return txModes[0].bps;
}

inline bool Dot11BRate_KbpsValid(unsigned int kbps) { return __TXMODE_bps_valid(__DataRateKbps_Dot11B, kbps); }
inline bool Dot11ARate_KbpsValid(unsigned int kbps) { return __TXMODE_bps_valid(__DataRateKbps_Dot11A, kbps); }

inline unsigned char Dot11BDataRate_Kbps2Code(unsigned int kbps) { return (unsigned char)__TXMODE_bps2code(__DataRateKbps_Dot11B, kbps); }
inline unsigned int Dot11ADataRate_Kbps2Code(unsigned int kbps) { return __TXMODE_bps2code(__DataRateKbps_Dot11A, kbps); }

inline unsigned int Dot11BDataRate_Code2Kbps(unsigned int code) { return __TXMODE_code2bps(__DataRateKbps_Dot11B, code); }
inline unsigned int Dot11ADataRate_Code2Kbps(unsigned int code) { return __TXMODE_code2bps(__DataRateKbps_Dot11A, code); }
inline unsigned int Dot11ADataRate_Code2Mbps(unsigned int code) { return __TXMODE_code2bps(__DataRateMbps_Dot11A, code); }
inline int          Dot11ADataRate_Mbps2Index(unsigned int Mbps) { return __TXMODE_bps2index(__DataRateMbps_Dot11A, Mbps); }
inline int          Dot11ADataRate_Code2Index(unsigned int code) { return __TXMODE_code2index(__DataRateKbps_Dot11A, code); }
