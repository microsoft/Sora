#if !defined(_COMMON_H)
#define _COMMON_H
#include <windows.h>
#include <Setupapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <assert.h>

#if DBG
#include <assert.h>
#define ASSERT(condition) assert(condition)
#else
#define ASSERT(condition)
#endif

#define FILTER_SERVICE_NAME    "InterruptAffinityFilter" 
#define FILTER_REGISTRY_VALUE  "IntFiltr_AffinityMask"   

#define WARNING_COLOR   ( FOREGROUND_RED | FOREGROUND_GREEN )
#define ERROR_COLOR     ( FOREGROUND_RED )

enum
{
    __START_OPT   = -1,
    ADD_FILTER_OPT,
    RM_FILTER_OPT,
    AFFINITY_OPT,
    INDEX_OPT,
    TOTAL_OPT
};

BOOLEAN
AddUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    );

PBYTE
GetDeviceRegistryProperty(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD Property,
    IN DWORD    ExpectedRegDataType,
    OUT PDWORD pPropertyRegDataType
    );

BOOLEAN
PrependSzToMultiSz(
    IN     LPCTSTR  SzToPrepend,
    IN OUT LPTSTR  *MultiSz
    );

size_t
MultiSzLength(
    IN LPCTSTR MultiSz
    );

BOOL
MultiSzSearch( IN LPCTSTR szFindThis,
               IN LPCTSTR mszFindWithin,
               IN BOOL    fCaseSensitive,
               OUT LPCTSTR * ppszMatch OPTIONAL
             );

void Main(int IsAdd, DWORD AffinityMask, int devIndex);

BOOL 
FilterIsInstalledOnDevice( 
    HDEVINFO hDevInfo,
    PSP_DEVINFO_DATA pDevInfoData );

size_t
MultiSzSearchAndDeleteCaseInsensitive(
    IN  LPCTSTR  szFindThis,
    IN  LPTSTR   mszFindWithin,
    OUT size_t  *NewLength
    );

LPTSTR
GetUpperFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

BOOLEAN
RemoveUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    );

void Warn(LPCSTR szFormat, ...);
void Error(LPCSTR szFormat, ...);

#define Info printf

#endif