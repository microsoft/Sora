#ifndef ATX_TPL_H
#define ATX_TPL_H

#define SERVICE_LEN_IN_BYTES 2

__forceinline
void Scramble11a(MDL * pmdlIn, char * pbOutput, 
        unsigned int uiOutTotal, unsigned int uiCRC32, ULONG /* ulRadom */)
{
    unsigned char * pbIn;
    unsigned char * pbOut = (unsigned char *)
        (pbOutput + SERVICE_LEN_IN_BYTES);
    unsigned char * pbOutEnd = (unsigned char *)
        (pbOutput + uiOutTotal);
    unsigned int uiLen;
    unsigned int i;
	
    unsigned char bReg = 0xFF; // no random
	
	// scramble the services bits
    pbOutput[0] = (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    pbOutput[1] = (bReg = *(SCRAMBLE_11A(bReg >> 1)));

    while (pmdlIn)
    {
        pbIn = (unsigned char *)(pmdlIn->StartVa) + pmdlIn->ByteOffset;
        uiLen = pmdlIn->ByteCount;

        for (i = 0; i < uiLen; i++)
        {
            *(pbOut++) = pbIn[i] ^ (bReg = *(SCRAMBLE_11A(bReg >> 1)));
        }

        pmdlIn = pmdlIn->Next;
    }

	#ifdef _DBG_PLOT_
		PlotText ( "MyLog", "CRC32 %08X\n", uiCRC32 );
	#endif
	
    // Scramble crc32
    pbIn = (unsigned char *)(&uiCRC32);
    *(pbOut++) = pbIn[0] ^ (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    *(pbOut++) = pbIn[1] ^ (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    *(pbOut++) = pbIn[2] ^ (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    *(pbOut++) = pbIn[3] ^ (bReg = *(SCRAMBLE_11A(bReg >> 1)));

    // Append the PSDU with at least 6 zero bits as tail field
    // ref: 802.11a-1999 17.3.2
    // We should replace the six scrambled zero bits following the message end with six
	// non-scrambled zero bits!
	*(pbOut++) = (bReg = *(SCRAMBLE_11A(bReg >> 1))) & 0xC0;

    // Append more zero bytes
    while (pbOut != pbOutEnd)
    {
        *(pbOut++) = (bReg = *(SCRAMBLE_11A(bReg >> 1)));
    }
	
	
}

__forceinline
void ClearWindow(COMPLEX16 * pcWindow)
{
	set_zero(*(vcs*)pcWindow);
}

__forceinline
ULONG GetSignalBytes(PBB11A_TX_VECTOR info, unsigned int uiSymbolCountData)
{
    unsigned int uiOutputSize = COMPLEX_COUNT_PREAMBLE + COMPLEX_COUNT_SIGNAL + uiSymbolCountData * COMPLEX_PER_SYMBOL;

    if (info->SampleRate == 44)
    {
        ASSERT(uiOutputSize % 10 == 0);
        uiOutputSize = uiOutputSize / 10 * 11;
    }

    uiOutputSize += 8; // last window, 16 align
    ULONG SignalBytes = uiOutputSize * sizeof(COMPLEX8);
    return SignalBytes;
}

#endif//ATX_TPL_H
