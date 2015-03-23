#include "common.h"
#include "args.h"

//HDEVINFO g_hDevInfo = INVALID_HANDLE_VALUE;

BOOL SetFilterAffinityMask( HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData,
                            DWORD affinityMask )
{
    HKEY hkeyDeviceParams;
    LONG lRetVal;
    BOOL fToReturn = TRUE;  // success


    //
    // Get a handle to the device's "Device Parameters" registry subkey
    //
    hkeyDeviceParams = SetupDiOpenDevRegKey( hDevInfo,
                                             pDevInfoData,
                                             DICS_FLAG_GLOBAL,  //CPRINCE: SHOULD (CAN?) USE 'DICS_FLAG_CONFIGSPECIFIC' INSTEAD ???
                                             0,
                                             DIREG_DEV,
                                             KEY_WRITE  // desired access
                                           );

    if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
    {
        // Error opening device registry key...
        //
        // If error occurred because "Device Parameters" sub-key does
        // not exist, then try to create that sub-key.

        // NOTE: when we call GetLastError() here, we're getting an invalid
        // error code.  So let's just _assume_ (yeah, I know) that the error
        // was because the key does not exist, and try to create it here.

        hkeyDeviceParams = SetupDiCreateDevRegKey( hDevInfo,
                                                   pDevInfoData,
                                                   DICS_FLAG_GLOBAL,  //CPRINCE: SHOULD (CAN?) USE 'DICS_FLAG_CONFIGSPECIFIC' INSTEAD ???
                                                   0,
                                                   DIREG_DEV,
                                                   NULL,
                                                   NULL
                                                 );
        if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
        {
            // OK, we can't open and can't create the key.  Let's
            // face it, we've failed, so return now.
            //MessageBox_FromErrorCode( GetLastError() );
            return FALSE;
        }
        //ELSE: we were able to create the key, so keep going...
    }


    //
    // Set the desired registry value
    //
    lRetVal = RegSetValueEx( hkeyDeviceParams,
                             FILTER_REGISTRY_VALUE,
                             0,
                             REG_DWORD,
                             (BYTE*)&affinityMask,
                             sizeof(DWORD)
                           );

    if( ERROR_SUCCESS != lRetVal )
    {
        //MessageBox_FromErrorCode( lRetVal );
        fToReturn = FALSE;  // failure
    }


    //
    // Close the registry key(s) we opened
    //
    lRetVal = RegCloseKey( hkeyDeviceParams );
    if( ERROR_SUCCESS != lRetVal )
    {
        //MessageBox_FromErrorCode( lRetVal );
        fToReturn = FALSE;  // failure
    }


    return fToReturn;
}

PBYTE
GetDeviceRegistryProperty(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD Property,
    IN DWORD    ExpectedRegDataType,
    OUT PDWORD pPropertyRegDataType
    )
{
    DWORD length = 0;
    PBYTE buffer = NULL;

    //
    // Get the required length of the buffer
    //
    if( SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
                                          DeviceInfoData,
                                          Property,
                                          NULL,   // registry data type
                                          NULL,   // buffer
                                          0,      // buffer size
                                          &length // [OUT] required size
        ) )
    {
        // We should not be successful at this point (since we passed in a
        // zero-length buffer), so this call succeeding is an error condition
        return NULL;
    }


    if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
    {
        // This is an error condition we didn't expect.  Something must have
        // gone wrong when trying to read the desired device property, so...
        //
        // NOTE: caller can use GetLastError() for more info
        return NULL;
    }


    // 
    // We didn't have a buffer before (hence the INSUFFICIENT_BUFFER error).
    // Now that we know required size, let's allocate a buffer and try again.
    //
    buffer = malloc( length );
    if( NULL == buffer )
    {
/* CPRINCE: SHOULD WE INDICATE EXACTLY _WHAT_ ERROR WAS VIA A RETURN CODE ??? (IE INFO THAT'S MORE USEFUL) */
        // Error: not enough memory
        return NULL;
    }
    if( !SetupDiGetDeviceRegistryProperty( DeviceInfoSet,
                                           DeviceInfoData,
                                           Property,
                                           pPropertyRegDataType,
                                           buffer,
                                           length, // buffer size
                                           NULL
        ) )
    {
        // Oops, error when trying to read the device property
        free( buffer );
        return NULL;
    }


    //
    // Successfully retrieved the device registry property.  Let's make
    // sure it has the right type.
    //
    if( ExpectedRegDataType != REG_NONE
        &&  ExpectedRegDataType != (*pPropertyRegDataType)  )
    {
        // Registry property has a type different from what caller expected.
        // So an error has occurred somewhere.
        free( buffer );
        return NULL;
    }


    //
    // OK, got device registry property.  Return ptr to buffer containing it.
    //
    return buffer;
}

BOOL GetFilterAffinityMask( HDEVINFO hDevInfo,
                            PSP_DEVINFO_DATA pDevInfoData,
                            DWORD* pAffinityMask )
{
    HKEY  hkeyDeviceParams;
    LONG  lRetVal;
    BOOL  fToReturn = TRUE;  // success
    DWORD regValueType;
    DWORD regValueSize;


    ASSERT( NULL != pAffinityMask );


    //
    // Get a handle to the device's "Device Parameters" registry subkey
    //
    hkeyDeviceParams = SetupDiOpenDevRegKey( hDevInfo,
                                             pDevInfoData,
                                             DICS_FLAG_GLOBAL,  //CPRINCE: SHOULD (CAN?) USE 'DICS_FLAG_CONFIGSPECIFIC' INSTEAD ???
                                             0,
                                             DIREG_DEV,
                                             KEY_QUERY_VALUE  // desired access
                                           );

    if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
    {
        // Probably just means that the "Device Parameters" subkey
        // does not exist, so return, but _don't_ display error message.
        return FALSE;  // failure
    }


    //
    // Get the desired registry value
    //
    regValueSize = sizeof(DWORD);
    lRetVal = RegQueryValueEx( hkeyDeviceParams,
                               FILTER_REGISTRY_VALUE,
                               0,
                               &regValueType,
                               (BYTE*)pAffinityMask,
                               &regValueSize
                             );

    if( ERROR_SUCCESS != lRetVal )
    {
        if( ERROR_FILE_NOT_FOUND == lRetVal )
        {
            // Just means key didn't already exist.
            // So don't display error message.
        }
        else
        {
            //MessageBox_FromErrorCode( lRetVal );
        }
        fToReturn = FALSE; // failure
    }
    else if( REG_DWORD != regValueType )
    {
        
        fToReturn = FALSE;  // failure
    }


    //
    // Close the registry key(s) we opened
    //
    lRetVal = RegCloseKey( hkeyDeviceParams );
    if( ERROR_SUCCESS != lRetVal )
    {
        //MessageBox_FromErrorCode( lRetVal );
        fToReturn = FALSE;  // failure
    }


    return fToReturn;
}

void RmFilterForDevice(HDEVINFO hDevInfo, SP_DEVINFO_DATA *pDevInfoData)
{
    LPTSTR buffer;
    LPTSTR  deviceName = NULL;
    DWORD   regDataType;
    do
    {
        deviceName =
            (LPTSTR) GetDeviceRegistryProperty( 
                        hDevInfo,
                        pDevInfoData,
                        SPDRP_DEVICEDESC,
                        REG_SZ,
                        &regDataType);
        if (NULL == deviceName)
        {
            Warn("Unknown Device: No action\n");
            break;
        }
        Info("%s:\n", deviceName);
        if( FilterIsInstalledOnDevice(hDevInfo, pDevInfoData) )
        {
            Info("Intfilter dettached from %s\n", deviceName);
            RemoveUpperFilterDriver(hDevInfo, pDevInfoData, FILTER_SERVICE_NAME);
        }
        else
        {
            Warn("Intfilter is not attached, can't remove\n");
        }
    }while (FALSE);

    free(deviceName);
    Info("\n");
}

void AddFilterForDevice(HDEVINFO hDevInfo, SP_DEVINFO_DATA *pDevInfoData, DWORD AffinityMask)
{
    LPTSTR  deviceName = NULL;
    DWORD   dwAffinityMask;
    DWORD   regDataType;

    do
    {
        deviceName =
            (LPTSTR) GetDeviceRegistryProperty( 
                        hDevInfo,
                        pDevInfoData,
                        SPDRP_DEVICEDESC,
                        REG_SZ,
                        &regDataType);
        if (NULL == deviceName)
        {
            Warn("Unknown Device: No action\n");
            break;
        }
        Info("%s:\n", deviceName);
        if (!GetFilterAffinityMask(hDevInfo, pDevInfoData, &dwAffinityMask))
        {
            dwAffinityMask = -1;
        }
        Info("Original Affinity Mask: 0X%08X\n", dwAffinityMask);
        if (SetFilterAffinityMask( hDevInfo, pDevInfoData, AffinityMask ))
        {
            Info("New Affinity Mask: 0X%08X\n", AffinityMask);
        }
        else
        {
            Error("Affinity Set Failed!\n");
        }

        if( FilterIsInstalledOnDevice(hDevInfo,pDevInfoData) )
        {
            Info("Intfilter already installed on %s\n", deviceName);
        }
        else
        {
            if ( AddUpperFilterDriver(hDevInfo, pDevInfoData, FILTER_SERVICE_NAME) )
            {
                Info("Intfilter attached to %s\n", deviceName);
            }
            else
            {
                Error("Intfilter can't be attached to %s\n", deviceName);
            }
        }
    } while (FALSE);
    
    if (deviceName)
    {
        free( deviceName );
        deviceName = NULL;
    }
    Info("\n");
    return;
}

void AddRmForAllDevice(DWORD AffinityMask, BOOL fAdd, int targetIndex)
{
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    int deviceIndex;
    SP_DEVINFO_DATA  devInfoData;
    
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);  //API required.
    do
    {
        hDevInfo = 
            SetupDiGetClassDevs( 
                NULL,
                NULL,
                NULL,
                DIGCF_ALLCLASSES
                | DIGCF_PRESENT
                | DIGCF_PROFILE
                );
        if (hDevInfo == INVALID_HANDLE_VALUE)
        {
            Warn("unable to get a list of devices.\n");
            break;
        }
        for ( deviceIndex = 0; 
              SetupDiEnumDeviceInfo(hDevInfo, deviceIndex,  &devInfoData);
              deviceIndex++)
        {
            if ((targetIndex < 0) || (targetIndex == deviceIndex))
            {
                if (fAdd)
                    AddFilterForDevice(hDevInfo, &devInfoData, AffinityMask);
                else
                    RmFilterForDevice(hDevInfo, &devInfoData);
            }
        }

    } while (FALSE);
    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
        hDevInfo = INVALID_HANDLE_VALUE;
    }
}


void Main(int IsAdd, DWORD AffinityMask, int devIndex)
{
    AddRmForAllDevice(AffinityMask, IsAdd, devIndex);
    return;
}
