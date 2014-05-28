#include <string>
#include "soratime.h"
#include "args.h"
#include "bb_test.h"

TIMESTAMPINFO tsinfo;

int Demod11B(PDOT11_DEMOD_ARGS pArgs);
int TestMod11B(PDOT11_MOD_ARGS pArgs);
int Demod11B_Brick(PDOT11_DEMOD_ARGS pArgs);
int TestMod11B_Brick(PDOT11_MOD_ARGS pArgs);
int TestMod11A(PDOT11_MOD_ARGS pArgs);
BOOLEAN ConvertModFile2DumpFile_8b(const char * inputFilename, const char * outputFilename);
BOOLEAN ConvertModFile2DumpFile_16b(const char * inputFilename, const char * outputFilename);
void CsFrameDemod(PDOT11_DEMOD_ARGS pArgs);
void CompareACK();
BOOLEAN Test11AACK(PDOT11_MOD_ARGS pArgs);

int Test11B_FB_Demod (PDOT11_DEMOD_ARGS pArgs);
int Test11B_FB_Mod   (PDOT11_MOD_ARGS   pArgs);
int Test11A_FB_Demod (PDOT11_DEMOD_ARGS pArgs);
int Test11A_FB_Mod   (PDOT11_MOD_ARGS   pArgs);
int Test11N_FB_Demod (PDOT11_DEMOD_ARGS pArgs);
int Test11N_FB_Mod   (PDOT11_MOD_ARGS   pArgs);

enum
{
    //begin
    BB_OPT_TABLE_BEGIN = -1,
    //literal
    BB_OPT_MODE_11A,
    BB_OPT_MODE_11B,
    BB_OPT_MODE_11A_BRICK,
    BB_OPT_MODE_11B_BRICK,
    BB_OPT_MODE_11N_BRICK,
    BB_OPT_MOD,
    BB_OPT_CONV,
    BB_OPT_DEMOD,
    BB_OPT_ACK,

    BB_OPT_LITERAL_END,
    //str
    BB_OPT_FILE = BB_OPT_LITERAL_END,
    BB_OPT_OUTFILE,
    BB_OPT_STR_END,
    //int
    BB_OPT_11B_THRESHOLD = BB_OPT_STR_END,
    BB_OPT_11B_THRESHOLD_LH,
    BB_OPT_11B_THRESHOLD_HL,
    BB_OPT_11B_SHIFT_RIGHT,
    BB_OPT_11B_START_DESC,
    BB_OPT_11A_BITRATE,
    BB_OPT_SAMPLERATE,

    //end
    BB_OPT_TABLE_END
};

PARG_BASE *BBSupportedOptions()
{
	int i = 0;
    PARG_BASE* OptTable = (PARG_BASE*)malloc(sizeof(PARG_BASE) * BB_OPT_TABLE_END);
	for(i = 0; i < BB_OPT_LITERAL_END; i++)
	{
        OptTable[i] = (PARG_BASE) malloc(sizeof(ARG_LITERAL));
	}
	for(i = BB_OPT_LITERAL_END; i < BB_OPT_STR_END; i++)
	{
        OptTable[i] = (PARG_BASE) malloc(sizeof(ARG_STR));
	}
	for(i = BB_OPT_STR_END; i < BB_OPT_TABLE_END; i++)
	{
        OptTable[i] = (PARG_BASE) malloc(sizeof(ARG_INT));
	}

    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_MODE_11A], "a", "802.11a", NULL, "specify 802.11a mode", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_MODE_11B], "b", "802.11b", NULL, "specify 802.11b mode", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_MODE_11A_BRICK], NULL, "802.11a.brick", NULL, "specify 802.11a.brick mode", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_MODE_11B_BRICK], NULL, "802.11b.brick", NULL, "specify 802.11b.brick mode", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_MODE_11N_BRICK], NULL, "802.11n.brick", NULL, "specify 802.11n.brick mode", 0, 1);

    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_MOD], "m", "mod", NULL, "specify modulation mode", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_CONV], "c", "conv", NULL, "specify converting mode", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_DEMOD], "d", "demod", NULL, "specify demodulation mode", 0, 1);
    ArgLiteralCtor((PARG_LITERAL)OptTable[BB_OPT_ACK], "k", "ack", NULL, "specify ack test mode", 0, 1);
	
    ArgStrCtor((PARG_STR)OptTable[BB_OPT_FILE], "f", "file", "[file name]", "specify input file", 0, 1);
    ArgStrCtor((PARG_STR)OptTable[BB_OPT_OUTFILE], "o", "out", "[file name]", "specify output file(mod/conv only)", 0, 1);
    ArgIntCtor((PARG_INT)OptTable[BB_OPT_11B_THRESHOLD], "t", "spdthd", "[energy]", "set energy threshold for 802.11b power detection(802.11b demod only)", 0, 1);
    ((PARG_INT)OptTable[BB_OPT_11B_THRESHOLD])->Values[0] = DEFAULT_THRESHOLD;
    ArgIntCtor((PARG_INT)OptTable[BB_OPT_11B_THRESHOLD_LH], NULL, "spdthd_lh", "[energy]", "set energy threshold for 802.11b power detection(802.11b demod only)", 0, 1);
    ((PARG_INT)OptTable[BB_OPT_11B_THRESHOLD_LH])->Values[0] = DEFAULT_THRESHOLD_LH;
    ArgIntCtor((PARG_INT)OptTable[BB_OPT_11B_THRESHOLD_HL], NULL, "spdthd_hl", "[energy]", "set energy threshold for 802.11b power detection(802.11b demod only)", 0, 1);
    ((PARG_INT)OptTable[BB_OPT_11B_THRESHOLD_HL])->Values[0] = DEFAULT_THRESHOLD_HL;
    ArgIntCtor((PARG_INT)OptTable[BB_OPT_11B_SHIFT_RIGHT], "S", "shiftright", "[bits]", "set shift bits after downsampling(802.11b demod only)", 0, 1);
    ((PARG_INT)OptTable[BB_OPT_11B_SHIFT_RIGHT])->Values[0] = DEFAULT_SHIFT_RIGHT;
    ArgIntCtor((PARG_INT)OptTable[BB_OPT_11B_START_DESC], "s", NULL, NULL, "start processing from startDescCount(802.11b demod only)", 0, 1);
    ((PARG_INT)OptTable[BB_OPT_11B_START_DESC])->Values[0] = DEFAULT_START_DESC;
    ArgIntCtor((PARG_INT)OptTable[BB_OPT_11A_BITRATE], "r", "bitrate", "[kbps]", "bit rate for 802.11 modulation", 0, 1);
    ((PARG_INT)OptTable[BB_OPT_11A_BITRATE])->Values[0] = DEFAULT_BITRATE;
    ArgIntCtor((PARG_INT)OptTable[BB_OPT_SAMPLERATE], "p", "samplerate", "[MHz]", "sample rate of the radio PCB", 0, 1);
    ((PARG_INT)OptTable[BB_OPT_SAMPLERATE])->Values[0] = DEFAULT_SAMPLERATE;
    return OptTable;
}

int __cdecl main(int argc, const char *argv[])
{
    // Init sora timer
	InitializeTimestampInfo ( &tsinfo );
	tsinfo.use_rdtsc = 0;

    PARG_BASE *Options;
    HRESULT hrSyntax = S_OK;
    BOOL bMod = FALSE, bConv = FALSE, bDemod = FALSE, bACK = FALSE;
    DOT11_DEMOD_ARGS argsDemod11;
    DOT11_MOD_ARGS argsMod11;

    Options = BBSupportedOptions();

    hrSyntax = ArgParse(Options, BB_OPT_TABLE_END, argc - 1,  argv + 1);
    if(FAILED(hrSyntax))
    {
        ArgsHelp(Options, BB_OPT_TABLE_END);
        return 1;
    }

    BOOL b11A = ((PARG_LITERAL)Options[BB_OPT_MODE_11A])->Count;
    BOOL b11B = ((PARG_LITERAL)Options[BB_OPT_MODE_11B])->Count;
    BOOL b11A_Brick = ((PARG_LITERAL)Options[BB_OPT_MODE_11A_BRICK])->Count;
    BOOL b11B_Brick = ((PARG_LITERAL)Options[BB_OPT_MODE_11B_BRICK])->Count;
    BOOL b11N_Brick = ((PARG_LITERAL)Options[BB_OPT_MODE_11N_BRICK])->Count;
    
    bMod = ((PARG_LITERAL)Options[BB_OPT_MOD])->Count;
    bConv = ((PARG_LITERAL)Options[BB_OPT_CONV])->Count;
    bDemod = ((PARG_LITERAL)Options[BB_OPT_DEMOD])->Count;
    bACK = ((PARG_LITERAL)Options[BB_OPT_ACK])->Count;

	if(bMod + bConv + bDemod + bACK != 1)
	{
        ArgsHelp(Options, BB_OPT_TABLE_END);
        return 1;
	}

    argsMod11.nBitRate = ((PARG_INT)Options[BB_OPT_11A_BITRATE])->Values[0];
    argsMod11.SampleRate = ((PARG_INT)Options[BB_OPT_SAMPLERATE])->Values[0];
    argsDemod11.pcInFileName = argsMod11.pcInFileName = ((PARG_STR)Options[BB_OPT_FILE])->Values[0];
    argsDemod11.pcOutFileName = argsMod11.pcOutFileName = ((PARG_STR)Options[BB_OPT_OUTFILE])->Values[0];
	argsDemod11.nPowerThreshold = ((PARG_INT)Options[BB_OPT_11B_THRESHOLD])->Values[0];
	argsDemod11.nPowerThresholdLH = ((PARG_INT)Options[BB_OPT_11B_THRESHOLD_LH])->Values[0];
	argsDemod11.nPowerThresholdHL = ((PARG_INT)Options[BB_OPT_11B_THRESHOLD_HL])->Values[0];
	argsDemod11.nShiftRight = ((PARG_INT)Options[BB_OPT_11B_SHIFT_RIGHT])->Values[0];
	argsDemod11.nStartDesc = ((PARG_INT)Options[BB_OPT_11B_START_DESC])->Values[0];
    argsDemod11.SampleRate = ((PARG_INT)Options[BB_OPT_SAMPLERATE])->Values[0];

    if (bConv)
    {
        if (argsMod11.pcInFileName && argsMod11.pcOutFileName)
        {
            if (b11N_Brick)
            {
                ConvertModFile2DumpFile_16b(
                    std::string(argsMod11.pcInFileName).append("_0.dmp").c_str(),
                    std::string(argsMod11.pcOutFileName).append("_0.dmp").c_str());
                ConvertModFile2DumpFile_16b(
                    std::string(argsMod11.pcInFileName).append("_1.dmp").c_str(),
                    std::string(argsMod11.pcOutFileName).append("_1.dmp").c_str());
            }
            else
            {
                ConvertModFile2DumpFile_8b(argsMod11.pcInFileName, argsMod11.pcOutFileName);
            }
        }
        else
        {
            ArgsHelp(Options, BB_OPT_TABLE_END);
            return 1;
        }
        return 0;
    }

    if(b11A + b11B + b11A_Brick + b11B_Brick + b11N_Brick != 1)
	{
        ArgsHelp(Options, BB_OPT_TABLE_END);
        return 1;
	}

    if (b11A)
    {
        if (bMod)
        {
            if (argsMod11.pcInFileName == NULL || argsMod11.pcOutFileName == NULL)
            {
                ArgsHelp(Options, BB_OPT_TABLE_END);
                return 1;
            }
            TestMod11A(&argsMod11);
        }
        else if (bDemod)
        {
            if(argsDemod11.pcInFileName == NULL)
            {
                ArgsHelp(Options, BB_OPT_TABLE_END);
                return 1;
            }
			CsFrameDemod(&argsDemod11);
        }
        else if (bACK)
            Test11AACK(&argsMod11);
    }
    else if (b11A_Brick)
    {
        if (bMod)
        {
            if (argsMod11.pcInFileName  == NULL || argsMod11.pcOutFileName == NULL)
            {
                ArgsHelp(Options, BB_OPT_TABLE_END);
                return 1;
            }
            Test11A_FB_Mod(&argsMod11);
        }
        else if (bDemod)
        {
		    if(argsDemod11.pcInFileName == NULL)
		    {
			    ArgsHelp(Options, BB_OPT_TABLE_END);
			    return 1;
		    }
		    Test11A_FB_Demod(&argsDemod11);
        }
    }
    else if (b11B)
    {
        if (bMod)
        {
            if (argsMod11.pcInFileName  == NULL || argsMod11.pcOutFileName == NULL)
		    {
			    ArgsHelp(Options, BB_OPT_TABLE_END);
			    return 1;
		    }
            TestMod11B(&argsMod11);
        }
        else if (bDemod)
        {
		    if(argsDemod11.pcInFileName == NULL)
		    {
			    ArgsHelp(Options, BB_OPT_TABLE_END);
			    return 1;
		    }
		    Demod11B(&argsDemod11);
        }
        else if (bACK)
            CompareACK();
    }
    else if (b11B_Brick)
    {
        if (bMod)
        {
            if (argsMod11.pcInFileName  == NULL || argsMod11.pcOutFileName == NULL)
		    {
			    ArgsHelp(Options, BB_OPT_TABLE_END);
			    return 1;
		    }
            Test11B_FB_Mod(&argsMod11);
        }
        else if (bDemod)
        {
		    if(argsDemod11.pcInFileName == NULL)
		    {
			    ArgsHelp(Options, BB_OPT_TABLE_END);
			    return 1;
		    }
		    Test11B_FB_Demod(&argsDemod11);
        }
	}
    else if (b11N_Brick)
    {
        if (bMod)
        {
            if (argsMod11.pcInFileName  == NULL || argsMod11.pcOutFileName == NULL)
		    {
			    ArgsHelp(Options, BB_OPT_TABLE_END);
			    return 1;
		    }
            Test11N_FB_Mod(&argsMod11);
        }
        else
        if (bDemod)
        {
		    if(argsDemod11.pcInFileName == NULL)
		    {
			    ArgsHelp(Options, BB_OPT_TABLE_END);
			    return 1;
		    }
		    Test11N_FB_Demod(&argsDemod11);
        }
    }
    
	return 0;
}
