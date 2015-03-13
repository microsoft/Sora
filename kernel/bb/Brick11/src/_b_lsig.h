struct L_SIG 
{
    // Rate  Reserved  Length  Parity  Tail
    // 4     1         12      1       6
    static const int SIZE = 3;

    union
    {
        unsigned __int8  cdata[4];
        unsigned __int32 idata;
    };

    __forceinline void clear()
    {
        idata = 0;
    }

    __forceinline void zerotail()
    {
        idata &= 0x0003FFFF; // zero tailing bits
    }

    __forceinline void update(unsigned __int32 bRateCode, unsigned __int32 usLength)
    {
        unsigned __int32 uiParity;

        idata = bRateCode;
        idata |= ((unsigned __int32)(usLength)) << 5;

        uiParity = idata ^ (idata >> 16);
        uiParity ^= uiParity >> 8;
        uiParity ^= uiParity >> 4;
        uiParity ^= uiParity >> 2;
        uiParity ^= uiParity >> 1;
        uiParity &= 0x1;

        idata |= uiParity << 17;
    }
};