#include "args.h"

BOOLEAN BeginWith(const char *szStr, const char *szWith)
{
    int i;
    for (i = 0; szStr[i] != 0 && szWith[i] != 0; i++)
    {
        if (szStr[i] != szWith[i])
            return FALSE;
    }
    return (i > 0 && szWith[i] == 0);
}

HRESULT Match(PARG_BASE Base, const char *szOpt)
{
    if (BeginWith(szOpt, LONG_OPT_PREFIX))
    {
        if (Base->szLongOpts && strcmp(szOpt + LONG_OPT_PREFIX_LEN, Base->szLongOpts) == 0)
        {
            return S_OK;
        }
    }
    else
        if (BeginWith(szOpt, SHORT_OPT_PREFIX))
        {
            if (Base->szShortOpts && strcmp(szOpt + SHORT_OPT_PREFIX_LEN, Base->szShortOpts) == 0)
            {
                return S_OK;
            }
        }
        else
            if (Base->szLongOpts == NULL && Base->szShortOpts == NULL)
            {
                return S_UNTAGGED_OPT;
            }
    return E_FAIL;
}

VOID ArgBaseCtor(
        PARG_BASE   ArgBase,
        const char  *szShortOpts, 
        const char  *szLongOpts, 
        const char  *szValueHelp, 
        const char  *szOptHelp, 
        int         iMinOccurs, 
        int         iMaxOccurs)
{
    *(int*)(&ArgBase->iMinOccurs)   = iMinOccurs;
    *(int*)(&ArgBase->iMaxOccurs)   = iMaxOccurs;
    ArgBase->szShortOpts    = szShortOpts;
    ArgBase->szLongOpts     = szLongOpts;
    ArgBase->szValueHelp    = szValueHelp;
    ArgBase->szOptHelp      = szOptHelp;
    ArgBase->Match          = Match;
}

HRESULT ArgLiteralScan(PARG_BASE Base, const char *szOptValue)
{
    UNREFERENCED_PARAMETER(szOptValue);
    ((PARG_LITERAL)Base)->Count++;
    if (((PARG_LITERAL)Base)->Count > Base->iMaxOccurs)
        return E_FAIL;
    return S_NO_VALUE;
}

VOID ArgLiteralCtor(
        PARG_LITERAL    ArgLit, 
        const char      *szShortOpts, 
        const char      *szLongOpts, 
        const char      *szValueHelp, 
        const char      *szOptHelp, 
        int             iMinOccurs, 
        int             iMaxOccurs)
{
    ArgBaseCtor(
        &ArgLit->Base,
        szShortOpts,
        szLongOpts,
        szValueHelp,
        szOptHelp,
        iMinOccurs,
        iMaxOccurs);
    ArgLit->Base.Scan = ArgLiteralScan;
    ArgLit->Count = 0;

}

BOOLEAN ParseDecValue(IN const char *szDec, OUT int *p) 
{
    int i ;
    for (i = 0; szDec[i] != 0; i++)
    {
        if (i == 0 && szDec[i] == '-') continue;
        if (szDec[i] > '9' || szDec[i] < '0')
            return FALSE;
    }
    if (i > 0)
    {
        *p = atoi(szDec);
        return TRUE;
    }
    return FALSE;
}

BOOLEAN ParseHexValue(IN const char *szHex, OUT int *p) 
{
    int i;
    *p = 0;
    for (i = 0; szHex[i] != 0; i++)
    {
        if (szHex[i] >= '0' && szHex[i] <= '9') 
        {
            *p = ((*p) << 4) + (szHex[i] - '0');
            continue;
        }
        if (szHex[i] >= 'a' && szHex[i] <= 'f')
        {
            *p = ((*p) << 4) + (szHex[i] - 'a' + 10);
            continue;
        }
        if (szHex[i] >= 'A' && szHex[i] <= 'F')
        {
            *p = ((*p) << 4) + (szHex[i] - 'A' + 10);
            continue;
        }
        break;
    }
    
    return (i != 0  && szHex[i] == 0);
}

HRESULT ArgStrScan(PARG_BASE Base, const char *szOptValue)
{
    if (((PARG_STR)Base)->Count < MAX_OPT_OCCURS)
    {
        if (szOptValue)
        {
            ((PARG_STR)Base)->Values[((PARG_STR)Base)->Count] = szOptValue;
            ((PARG_STR)Base)->Count++;
        }
        else
        {
            return E_FAIL;
        }
    }
    else
    {
        return E_FAIL;
    }
    return S_OK;
}
HRESULT ArgIntScan(PARG_BASE Base, const char *szOptValue)
{
    int iValue;
    HRESULT hr = S_OK;
    if (!szOptValue)
        return E_FAIL;
    if (BeginWith(szOptValue, HEX_LPREFIX) || BeginWith(szOptValue, HEX_UPREFIX))
    {
        if (!ParseHexValue(szOptValue + HEX_PREFIX_LEN, &iValue))
        {
            hr = E_FAIL;
        }
    }
    else
    {
        if (!ParseDecValue(szOptValue, &iValue))
        {
            hr = E_FAIL;
        }
    }
    if (SUCCEEDED(hr))
    {
        if (((PARG_INT)Base)->Count >= Base->iMaxOccurs)
        {
            hr = E_FAIL;
        }
        else
        {
            ((PARG_INT)Base)->Values[((PARG_INT)Base)->Count] = iValue;
            ((PARG_INT)Base)->Count++;
        }
    }
    return hr;
}

VOID ArgStrCtor(
        PARG_STR        ArgStr,
        const char      *szShortOpts,
        const char      *szLongOpts,
        const char      *szValueHelp,
        const char      *szOptHelp,
        int             iMinOccurs,
        int             iMaxOccurs)
{
    int i;
    ArgBaseCtor(
        &ArgStr->Base,
        szShortOpts,
        szLongOpts,
        szValueHelp,
        szOptHelp,
        iMinOccurs,
        iMaxOccurs);
    
    ArgStr->Base.Scan = ArgStrScan;
    ArgStr->Count = 0;
    for (i = 0; i < MAX_OPT_OCCURS; i++)
    {
        ArgStr->Values[i] = NULL;
    }
}

VOID ArgIntCtor(
        PARG_INT        ArgInt, 
        const char      *szShortOpts, 
        const char      *szLongOpts, 
        const char      *szValueHelp, 
        const char      *szOptHelp, 
        int             iMinOccurs, 
        int             iMaxOccurs)
{
    ArgBaseCtor(
        &ArgInt->Base,
        szShortOpts,
        szLongOpts,
        szValueHelp,
        szOptHelp,
        iMinOccurs,
        iMaxOccurs);
    ArgInt->Base.Scan = ArgIntScan;
    ArgInt->Count = 0;
}

HRESULT 
ArgParse(
    IN OUT PARG_BASE *Options, 
    IN int count, 
    IN int argc, 
    IN const char **argv)
{
    HRESULT hr = E_FAIL; 
    int i, j;

    for (i = 0; i < argc; i++)
    {
        for (j = 0; j < count; j++)
        {
            hr = Match(Options[j], argv[i]);
            if (FAILED(hr)) continue;
            if (hr == S_UNTAGGED_OPT)
            {
                hr = Options[j]->Scan(Options[j], argv[i]);
            }
            else
            {
                if (i + 1 >= argc)
                    hr = Options[j]->Scan(Options[j], NULL);
                else
                    hr = Options[j]->Scan(Options[j], argv[i+1]);
                if (hr != S_NO_VALUE) 
                {
                    i++;
                }
                
            }
            break;
        }
        if (FAILED(hr)) break;
    }
    return hr;
}

VOID ArgHelp(PARG_BASE Option)
{

    if (Option)
    {
        if (Option->szShortOpts)
        {
            printf("%s%s", SHORT_OPT_PREFIX, Option->szShortOpts);
            if (Option->szLongOpts)
            {
                printf(" | ");
            }
        }
        if (Option->szLongOpts)
        {
            printf("%s%s", LONG_OPT_PREFIX, Option->szLongOpts);
        }

        if (Option->szValueHelp)
        {
            printf(" %s", Option->szValueHelp);
        }
        
        printf(" :\n");
        if (Option->szOptHelp)
        {
            printf("%s\n\n", Option->szOptHelp);
        }
    }
}

VOID ArgsHelp(
    IN OUT PARG_BASE *Options, 
    IN int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        ArgHelp(Options[i]);
    }
}