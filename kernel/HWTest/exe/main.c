#include "dut.h"
#include "args.h"
#define USER_MODE
#include "..\\hwtest_ioctrl.h"

#define PRIVATE_FEATURE

typedef HRESULT (*FNOPTCHECKER)(PARG_BASE *OptTable, ULONG Count);

void __DutSet(HANDLE hDevice, ULONG RadioNo, DUT_CMD code, LONG Value);

typedef struct _CMDSTRCODE
{
    const char      *str;
    DUT_CMD         code;
    const char      *help;
    FNOPTCHECKER    Checker;
}CMDSTRCODE, *PCMDSTRCODE;

HRESULT MimoTxCheck(PARG_BASE *OptTable, ULONG Count) {

	HRESULT hr = E_FAIL;
	if (((PARG_INT)OptTable[DUT_OPT_RADIO_NO])->Count > 0 &&
		((PARG_INT)OptTable[DUT_OPT_RADIO_NO])->Count < 8 &&
		((PARG_INT)OptTable[DUT_OPT_SIG_ID])->Count > 0 &&
		((PARG_INT)OptTable[DUT_OPT_SIG_ID])->Count < 8 &&
		((PARG_INT)OptTable[DUT_OPT_RADIO_NO])->Count == ((PARG_INT)OptTable[DUT_OPT_SIG_ID])->Count)
		hr = S_OK;
	return hr;
}

HRESULT RadioCheck(PARG_BASE *OptTable, ULONG Count) {

	HRESULT hr = E_FAIL;
	if (((PARG_INT)OptTable[DUT_OPT_RADIO_NO])->Count == 1)
	{
		hr = S_OK;
	}
	return hr;
}

#ifdef PRIVATE_FEATURE
HRESULT RrChecker(PARG_BASE *OptTable, ULONG Count)
{
    return (((PARG_INT)OptTable[DUT_OPT_REG_OFFSET])->Count == 1) ? S_OK: E_FAIL;
    
}

HRESULT WrChecker(PARG_BASE *OptTable, ULONG Count)
{
    return (((PARG_INT)OptTable[DUT_OPT_VALUE])->Count == 1 &&
        ((PARG_INT)OptTable[DUT_OPT_REG_OFFSET])->Count == 1) ? S_OK : E_FAIL;
}

HRESULT RadioRrChecker(PARG_BASE *OptTable, ULONG Count) {

	HRESULT hr;
	hr = RadioCheck(OptTable, Count);
	if (hr != S_OK)
		return hr;
	return RrChecker(OptTable, Count);
}

HRESULT RadioWrChecker(PARG_BASE *OptTable, ULONG Count) {

	HRESULT hr;
	hr = RadioCheck(OptTable, Count);
	if (hr != S_OK)
		return hr;
	return WrChecker(OptTable, Count);
}
#endif

HRESULT TransChecker(PARG_BASE *OptTable, ULONG Count)
{
    HRESULT hr = E_FAIL;
    if (((PARG_STR)OptTable[DUT_OPT_FILE_NAME])->Count == 1)
    {
        size_t len = strlen(((PARG_STR)OptTable[DUT_OPT_FILE_NAME])->Values[0]);
        size_t prefixLen = strlen(FILE_OBJ_NAME_PREFIX);
        if (len + prefixLen < SYM_FILE_MAX_PATH_LENGTH)
            hr = S_OK;
    }
    return hr;
}

HRESULT TxCheck(PARG_BASE *OptTable, ULONG Count)
{
    HRESULT hr = E_FAIL;
    if (((PARG_INT)OptTable[DUT_OPT_SIG_ID])->Count == 1)
    {
    	hr = RadioCheck(OptTable, Count);
    }
    return hr;
}

HRESULT TxDoneCheck(PARG_BASE *OptTable, ULONG Count)
{
    HRESULT hr = E_FAIL;
    if (((PARG_INT)OptTable[DUT_OPT_SIG_ID])->Count == 1)
    {
    	hr = S_OK;
    }
    return hr;
}

HRESULT DefaultCheck(PARG_BASE *OptTable, ULONG Count)
{
    UNREFERENCED_PARAMETER(OptTable);
    UNREFERENCED_PARAMETER(Count);
    return S_OK;
}

#define FreqCheck GainCheck

HRESULT GainCheck(PARG_BASE *OptTable, ULONG Count)
{
    HRESULT hr = E_FAIL;
    if (((PARG_INT)OptTable[DUT_OPT_VALUE])->Count == 1)
    {
        hr = RadioCheck(OptTable, Count);
    }
    return hr;
}

HRESULT PACheck(PARG_BASE *OptTable, ULONG Count)
{
    HRESULT hr = E_FAIL;
    if (((PARG_INT)OptTable[DUT_OPT_VALUE])->Count == 1)
    {
        int v = ((PARG_INT)OptTable[DUT_OPT_VALUE])->Values[0];
        if (v == 0 || v == 0x1000 || v == 0x2000 || v == 0x3000)
        {
            hr = RadioCheck(OptTable, Count);
        }
    }
    return hr;
}

HRESULT DumpCheck(PARG_BASE *OptTable, ULONG Count) {

	HRESULT hr = E_FAIL;
	if (((PARG_INT)OptTable[DUT_OPT_RADIO_NO])->Count >= 0 &&
		((PARG_INT)OptTable[DUT_OPT_RADIO_NO])->Count <  8)
		hr = S_OK;
	return hr;
}


CMDSTRCODE g_SupportedCommands[] = 
{
#ifdef PRIVATE_FEATURE
    {"rr", CMD_RD_REG, "Read register", RrChecker}, 
    {"wr", CMD_WR_REG, "Write register", WrChecker}, 
	{"radrr", CMD_RADIO_RD_REG, "Radio read register", RadioRrChecker},
	{"radwr", CMD_RADIO_WR_REG, "Radio write register", RadioWrChecker},
#endif
    {"start", CMD_START_RADIO, "Start radio", RadioCheck},										// radio required
    {"stop", CMD_STOP_RADIO, "Stop radio", RadioCheck},											// radio required
    {"tx", CMD_TX, "Transmit signals by ID", TxCheck},											// radio required
    {"txdone", CMD_TX_DONE, "Complete TX by ID", TxDoneCheck}, 
    {"dump", CMD_DUMP, "Dump RX buffer", DumpCheck},											// radio required // special radio check, check for the radio mask
    {"txgain", CMD_TX_GAIN, "Set TX gain", GainCheck},											// radio required
    {"rxgain", CMD_RX_GAIN, "Set RX gain", GainCheck},											// radio required
    {"rxpa", CMD_RX_PA, "Set RX PA, can only be 0, 0x1000, 0x2000, 0x3000", PACheck},			// radio required
    {"info", CMD_INFO, "Get kernel information", DefaultCheck}, 
    {"transfer", CMD_TRANSFER, "Transfer signals in a file into hardware", TransChecker},
    {"centralfreq", CMD_CENTRAL_FREQ, "Set Central Frequency in MHz", FreqCheck},				// radio required
    {"freqoffset", CMD_FREQ_OFFSET, "Set frequency compensation in Hz, i.e, -5Hz", FreqCheck},	// radio required 
    {"stoptx", CMD_STOP_TX, "Force stop TX", DefaultCheck},										// radio required
//    {"dma", CMD_DMA, "Get Rx Buffer user space address", DefaultCheck}, 
    {"samplerate", CMD_SAMPLE_RATE, "change sample rate", RadioCheck},							// radio required
	{"fwver", CMD_FW_VERSION, "Get frimware version", DefaultCheck},
	{"mimotx", CMD_MIMO_TX, "MIMO version tx", MimoTxCheck},
};

PCMDSTRCODE CmdStr2CmdCode(const char *cmd, PCMDSTRCODE SupportedCommands, ULONG Count)
{
    ULONG i;
    for (i = 0; i < Count; i++)
    {
        if (strcmp(cmd, SupportedCommands[i].str) == 0)
            return SupportedCommands + i;
    }
    return NULL;
}

void CmdHelp(PCMDSTRCODE SupportedCommands, ULONG Count)
{
    ULONG i;
    printf("Sora hardware device unit test tool 2.0\n");
    printf("dut.exe command [-|--options]\n\n");
    
    printf("Commands:\n\n");
    for (i = 0; i < Count; i++)
    {
        printf("%s: %s\n", SupportedCommands[i].str, SupportedCommands[i].help);
    }
    
}

PVOID __MALLOC(size_t size)
{
    PVOID ret = malloc(size);
    //printf("malloc size=%d, ret=%08x\n", size, ret);
    return ret;
}

void __FREE(PVOID p)
{
    //printf("free p = %08x\n", p);
    free(p);
    return;
}
PARG_BASE *SupportedOptions()
{
    int i;
    PARG_BASE *OptTable = __MALLOC(sizeof(PARG_BASE) * DUT_OPT_TABLE_SIZE);
    for ( i = 0; i < DUT_OPT_INT_END; i++)
    {
        OptTable[i] = (PARG_BASE) __MALLOC(sizeof(ARG_INT));
    }
    OptTable[DUT_OPT_FILE_NAME] = (PARG_BASE) __MALLOC(sizeof(ARG_STR));

    ArgIntCtor((PARG_INT)OptTable[DUT_OPT_SIG_ID], NULL, "sid", "[signal ID]", "specify a signal ID", 0, 8);
#ifdef PRIVATE_FEATURE
    ArgIntCtor((PARG_INT)OptTable[DUT_OPT_REG_OFFSET], NULL, "reg", "[register offset]", "specify register to operate", 0, 1);
#endif
    ArgIntCtor((PARG_INT)OptTable[DUT_OPT_VALUE], NULL, "value", "[value]", "specify a value for command", 0, 1);
	ArgIntCtor((PARG_INT)OptTable[DUT_OPT_RADIO_NO], NULL, "radio", "[radio number]", "specify a radio number", 0, 8);
    ArgStrCtor((PARG_STR)OptTable[DUT_OPT_FILE_NAME], NULL, "file", "[file name]", "specify filename", 0, 1);
    return OptTable;
}

void SupportedOptionsDtor(PARG_BASE *OptTable)
{
    int i;
    if (OptTable != NULL)
    {
        for ( i = 0; i < DUT_OPT_INT_END; i++)
        {
            free(OptTable[i]);
        }
        free(OptTable);
    }
}

int __cdecl main(int argc, char *argv[])
{
    PCMDSTRCODE CmdCode;
    PARG_BASE *Options;
    HRESULT hrSyntax = S_OK;
    HANDLE hDevice = NULL;
    
    Options = SupportedOptions();

    if (argc < 2) 
    {
        hrSyntax = E_FAIL; 
        goto Help;
    }
    
    CmdCode = CmdStr2CmdCode(argv[1], g_SupportedCommands, sizeof(g_SupportedCommands)/sizeof(CMDSTRCODE));
    
    if (argc > 2)
        hrSyntax = ArgParse(Options, DUT_OPT_TABLE_SIZE, argc - 2,  argv + 2);
    if (FAILED(hrSyntax) || !CmdCode)
    {
        goto Help;
    }
    
    hrSyntax = CmdCode->Checker(Options, DUT_OPT_TABLE_SIZE);
    if (FAILED(hrSyntax)) goto Help;

    hDevice = GetDeviceHandle(DEVNAME);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Can't find device\n");
        goto Help;
    }

    switch (CmdCode->code)
    {
#ifdef PRIVATE_FEATURE
    case CMD_RD_REG:
        DutReadRegister(hDevice, ((PARG_INT)Options[DUT_OPT_REG_OFFSET])->Values[0]);
        break;
    case CMD_WR_REG:
        DutWriteRegister(hDevice,
            ((PARG_INT)Options[DUT_OPT_REG_OFFSET])->Values[0], 
            ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
        break;
	case CMD_RADIO_RD_REG:
		{
			if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
				printf("Can't find device\n");
				break;
			}
			if (((PARG_INT)Options[DUT_OPT_REG_OFFSET])->Values[0] > 0x3f) {
				SoraUCleanUserExtension();
				printf("Invalid radio address\n");
				break;
			}			
			{
				HRESULT hr = S_OK;
				ULONG value = 0;
				hr = SoraUReadRadioRegister(
					((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0],
					((PARG_INT)Options[DUT_OPT_REG_OFFSET])->Values[0], 
					&value);
				printf("Ret: 0x%08x\n", hr);
				printf("Content: 0x%08x\n", value);
			}
			SoraUCleanUserExtension();
		}
		break;
	case CMD_RADIO_WR_REG:
		if (!SoraUInitUserExtension("\\\\.\\HWTest")) {
			printf("Can't find device\n");
			break;
		}
		if (((PARG_INT)Options[DUT_OPT_REG_OFFSET])->Values[0] > 0x3f) {
			SoraUCleanUserExtension();
			printf("Invalid radio address\n");
			break;
		}
		SoraUWriteRadioRegister(
			((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0],
			((PARG_INT)Options[DUT_OPT_REG_OFFSET])->Values[0], 
            ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
		SoraUCleanUserExtension();
		break;
#endif
    case CMD_START_RADIO:
        DutStartRadio(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0]);
        break;
    case CMD_STOP_RADIO:
        DutStopRadio(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0]);
        break;
    case CMD_DUMP:
        DutDump(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Count,
            ((PARG_STR)Options[DUT_OPT_FILE_NAME])->Values[0]);
        break;
    case CMD_INFO:
        DutGetInfo(hDevice);
        break;
    case CMD_STOP_TX:
        DutStopTX(hDevice);
        break;
    case CMD_TRANSFER:
        DutTransferSignals(hDevice, ((PARG_STR)Options[DUT_OPT_FILE_NAME])->Values[0]);
        break;
    case CMD_TX:
        DutTxSignals(hDevice, 
			((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0],
            ((PARG_INT)Options[DUT_OPT_SIG_ID])->Values[0], 
            ((PARG_INT)Options[DUT_OPT_VALUE])->Count ? ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0] : 1
            );
        break;
    case CMD_TX_DONE:
        DutTxDone(hDevice, ((PARG_INT)Options[DUT_OPT_SIG_ID])->Values[0]);
        break;
    case CMD_TX_GAIN:
        DutSetTxGain(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0], ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
        break;
    case CMD_RX_GAIN:
        DutSetRxGain(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0], ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
        break;
    case CMD_CENTRAL_FREQ:
        DutSetCentralFreq(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0], ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
        break;
#if 0		
    case CMD_DMA:
        DutGetRxBuffer(hDevice);
        break;
#endif
    case CMD_FREQ_OFFSET:
        DutSetFreqOffset(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0], ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
        break;
    case CMD_RX_PA:
        __DutSet(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0], CMD_RX_PA, ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
        break;
    case CMD_SAMPLE_RATE:
        __DutSet(hDevice, ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values[0], CMD_SAMPLE_RATE, ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0]);
        break;
	case CMD_FW_VERSION:
		DutGetFwVersion(hDevice);
		break;
	case CMD_MIMO_TX:
		DutMimoTx(hDevice, 
			((PARG_INT)Options[DUT_OPT_RADIO_NO])->Values,
            ((PARG_INT)Options[DUT_OPT_SIG_ID])->Values, 
            ((PARG_INT)Options[DUT_OPT_RADIO_NO])->Count,
            ((PARG_INT)Options[DUT_OPT_VALUE])->Count ? ((PARG_INT)Options[DUT_OPT_VALUE])->Values[0] : 1);
		break;
    default:
        hrSyntax = E_FAIL;
        break;
    }

Help:
    if (!CmdCode || FAILED(hrSyntax))
    {
        CmdHelp(g_SupportedCommands, sizeof(g_SupportedCommands)/sizeof(CMDSTRCODE));
        printf("\nOptions:\n\n");
        ArgsHelp(Options, DUT_OPT_TABLE_SIZE);
    }
    
    if (hDevice)
        CloseHandle(hDevice);
    SupportedOptionsDtor(Options);
    return 0;
}
