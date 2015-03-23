#ifndef _ARGS_H
#define _ARGS_H
#pragma once

#ifdef __cplusplus
 extern "C" {
#endif 
#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>

#define MAX_OPT_OCCURS          8
#define LONG_OPT_PREFIX         "--"
#define LONG_OPT_PREFIX_LEN     2
#define SHORT_OPT_PREFIX        "-"
#define SHORT_OPT_PREFIX_LEN    1

#define HEX_LPREFIX             "0x"
#define HEX_UPREFIX             "0X"
#define HEX_PREFIX_LEN          2

enum 
{
    S_UNTAGGED_OPT  = 0x010,
    S_NO_VALUE      = 0x011,
};
typedef struct _ARG_BASE *PARG_BASE;

typedef HRESULT (*MATCH_FUNC)(PARG_BASE Base, const char *szOpt);
typedef HRESULT (*SCAN_FUNC)(PARG_BASE Base, const char *szOptValue);

typedef struct _ARG_BASE
{
    const char  *szShortOpts;
    const char  *szLongOpts;
    const char  *szValueHelp;
    const char  *szOptHelp;
    const int   iMinOccurs;
    const int   iMaxOccurs;
    MATCH_FUNC  Match;
    SCAN_FUNC   Scan;
} ARG_BASE, *PARG_BASE;

typedef struct _ARG_LITERAL
{
    ARG_BASE        Base;
    int             Count;
} ARG_LITERAL, *PARG_LITERAL;

typedef struct _ARG_INT
{
    ARG_BASE        Base;
    int             Values[MAX_OPT_OCCURS];
    int             Count;
} ARG_INT, *PARG_INT, **PPARG_INT;

typedef struct _ARG_STR
{
    ARG_BASE        Base;
    char            *Values[MAX_OPT_OCCURS];
    int             Count;
}ARG_STR, *PARG_STR;

VOID ArgLiteralCtor(
        PARG_LITERAL    ArgLit, 
        const char      *szShortOpts, 
        const char      *szLongOpts, 
        const char      *szValueHelp, 
        const char      *szOptHelp, 
        int             iMinOccurs, 
        int             iMaxOccurs);

VOID ArgIntCtor(
        PARG_INT        ArgInt, 
        const char      *szShortOpts, 
        const char      *szLongOpts, 
        const char      *szValueHelp, 
        const char      *szOptHelp, 
        int             iMinOccurs, 
        int             iMaxOccurs);

HRESULT 
ArgParse(
    IN OUT PARG_BASE *Options, 
    IN int count, 
    IN int argc, 
    IN const char **argv);

VOID ArgsHelp(
    IN OUT PARG_BASE *Options, 
    IN int count);


#ifdef __cplusplus
 } //end extern "C" {
#endif 

#endif