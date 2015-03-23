#if !defined(_COMMON_H)
#define _COMMON_H
#include <windows.h>
#include <Setupapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <assert.h>
#include "args.h"

enum
{
    __START_OPT   = -1,
    ADD_FILTER_OPT,
    RM_FILTER_OPT,
    AFFINITY_OPT,
    DEVNAME_OPT,
    TOTAL_OPT
};

#define SAFE_REG_CLOSE(hKey)  \
{\
    if (hKey) \
    {\
        RegCloseKey(hKey); \
        hKey = NULL; \
    }\
}

void AddRmKeyForAllDevice(DWORD AffinityMask, BOOL fAdd, PCSTR deviceName);
void Warn(LPCSTR szFormat, ...);
void Error(const char *szFormat, ...);
#define Info printf

#define WARNING_COLOR   ( FOREGROUND_RED | FOREGROUND_GREEN )
#define ERROR_COLOR     ( FOREGROUND_RED )

#define REGKEY_INT_AFFINITY             "Interrupt Management\\Affinity Policy"
#define REGVAL_DEVICEPOLICY             "DevicePolicy"
#define REGVAL_ASSIGNMENTSETOVERRIDE    "AssignmentSetOverride"

#endif