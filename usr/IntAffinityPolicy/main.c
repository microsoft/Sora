#include "common.h"

WORD __SetConsoleColor(WORD NewTextAttribute)
{
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO OldInfo;
    WORD TextAttr = -1;
    if (GetConsoleScreenBufferInfo(hStdOut, &OldInfo))
    {
        TextAttr = OldInfo.wAttributes;
        SetConsoleTextAttribute(hStdOut, NewTextAttribute);
    }
    
    return TextAttr;
}

void __RestoreConsoleColor(WORD OldAttribute)
{
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, OldAttribute);
}

void Warn(LPCSTR szFormat, ...)
{
    va_list ap;
    WORD OldColor;
    va_start(ap, szFormat);
    OldColor = __SetConsoleColor(WARNING_COLOR);
    vprintf(szFormat, ap);
    if (OldColor != -1) __RestoreConsoleColor(OldColor);
    va_end(ap);
}

void Error(const char *szFormat, ...)
{
    va_list args;
    WORD OldColor;
    va_start(args, szFormat);
    OldColor = __SetConsoleColor(ERROR_COLOR);
    vfprintf(stdout, szFormat, args);
    if (OldColor != -1) __RestoreConsoleColor(OldColor);
    va_end(args);
}

PARG_BASE * GenSupportedOptions()
{
    PARG_BASE *Options;
    PARG_LITERAL AddOpt;
    PARG_LITERAL RmOpt;
    PARG_INT AffinityOpt;
    PARG_STR DevNameOpt;

    Options = malloc(sizeof(PARG_BASE) * TOTAL_OPT);
    AddOpt = malloc(sizeof(ARG_LITERAL));
    RmOpt = malloc(sizeof(ARG_LITERAL));    
    AffinityOpt = malloc(sizeof(ARG_INT));
    DevNameOpt = malloc(sizeof(ARG_STR));

    ArgLiteralCtor(AddOpt, "a", NULL, NULL, "add interrupt affinity ", 0, 1);
    ArgLiteralCtor(RmOpt, "r", NULL, NULL, "Remove interrupt affinity", 0, 1);
    ArgIntCtor(AffinityOpt, "m", NULL, "affinity mask", "Specify the affinity mask", 0, 1);
    ArgStrCtor(DevNameOpt, "i", NULL, "device name", "Specify the target device name", 0, 1);
    
    Options[ADD_FILTER_OPT] = (PARG_BASE)AddOpt;
    Options[RM_FILTER_OPT] = (PARG_BASE)RmOpt;
    Options[AFFINITY_OPT] = (PARG_BASE)AffinityOpt;
    Options[DEVNAME_OPT] = (PARG_BASE)DevNameOpt;
    
    return Options;
}

void FreeSupportedOptions(PARG_BASE *Options)
{
    int i;
    for (i = 0; i < TOTAL_OPT; i++)
        free(Options[i]);
    
    free(Options);
}

int __cdecl main(int argc, const char **argv)
{
    DWORD Mask = 0x1;
    int IsAdd = -1;
    PCSTR devName = NULL;
    HRESULT hr;
    PARG_BASE *Options = GenSupportedOptions();
    
    hr = ArgParse(Options, TOTAL_OPT, argc - 1, argv + 1);

    do
    {
        if (FAILED(hr))
        {
            ArgsHelp(Options, TOTAL_OPT);
            break;
        }
        if (((PARG_LITERAL)Options[ADD_FILTER_OPT])->Count == 1)
        {    
            IsAdd = 1;
            if (((PARG_INT)Options[AFFINITY_OPT])->Count == 1)
            {
                Mask = ((PARG_INT)Options[AFFINITY_OPT])->Values[0];
            }
        }
        else
        {
            if (((PARG_LITERAL)Options[RM_FILTER_OPT])->Count == 1)
            {
                IsAdd = 0;
            }
        }
        if (((PARG_STR)Options[DEVNAME_OPT])->Count == 1)
        {
            devName = ((PARG_STR)Options[DEVNAME_OPT])->Values[0];
        }
    } while (FALSE);
    FreeSupportedOptions(Options);

    if (IsAdd != -1)
        AddRmKeyForAllDevice(Mask, IsAdd, devName);
    return 1;
}
