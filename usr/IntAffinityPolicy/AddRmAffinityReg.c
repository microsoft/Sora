#include "common.h"

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

//
// Open \Interrupt Management\Affinity Policy key
//
HKEY OpenAffinityPolicyKey(HDEVINFO hDevInfo, SP_DEVINFO_DATA *pDevInfoData)
{
    HKEY    hkeyDeviceParams = NULL;
    HKEY    hAffinityPolicy = NULL;
    LONG    lRetVal = ERROR_SUCCESS;

    do
    {
        hkeyDeviceParams = SetupDiOpenDevRegKey( 
                                    hDevInfo,
                                    pDevInfoData,
                                    DICS_FLAG_GLOBAL,  //CPRINCE: SHOULD (CAN?) USE 'DICS_FLAG_CONFIGSPECIFIC' INSTEAD ???
                                    0,
                                    DIREG_DEV,
                                    KEY_ALL_ACCESS 
                                    );
        if( INVALID_HANDLE_VALUE == hkeyDeviceParams )
        {
            DWORD err = GetLastError();
            Warn("Device Parameters open fails with code=%d, so try to create it\n", err);
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
                break;
            }
        }

        lRetVal = RegOpenKeyEx(
                    hkeyDeviceParams, 
                    REGKEY_INT_AFFINITY, 
                    0, 
                    KEY_ALL_ACCESS , &hAffinityPolicy);
        if (lRetVal !=  ERROR_SUCCESS)
        {
            DWORD err = GetLastError();
            Warn("\"Interrupt Management\\Affinity Policy\" key open fails with code %d, so try to create it\n", err);
            lRetVal = RegCreateKeyEx(
                hkeyDeviceParams, 
                REGKEY_INT_AFFINITY, 
                0, 
                NULL,
                REG_OPTION_NON_VOLATILE, 
                KEY_ALL_ACCESS ,
                NULL, 
                &hAffinityPolicy, 
                NULL);
            if(lRetVal !=  ERROR_SUCCESS)
            {
                break;
            }
        }

    } while(FALSE);

    SAFE_REG_CLOSE(hkeyDeviceParams);
    return hAffinityPolicy;
}

void AddAffinityPolicy(HKEY hAffinityPolicy, DWORD AffinityMask)
{
    DWORD   DevicePolicy = 4; //IrqPolicySpecifiedProcessors
    
    LONG    lRetVal;
    
    do
    {
        lRetVal = RegSetValueEx(
            hAffinityPolicy, 
            REGVAL_DEVICEPOLICY,
            0,
            REG_DWORD, 
            (BYTE *)&DevicePolicy, 
            sizeof(DevicePolicy));
        if (lRetVal != ERROR_SUCCESS)
        {
            Warn("DevicePolicy value can not be set\n");
            break;
        }
        else
        {
            Info("DevicePolicy value set to IrqPolicySpecifiedProcessors\n");
        }

        lRetVal = RegSetValueEx(
            hAffinityPolicy, 
            REGVAL_ASSIGNMENTSETOVERRIDE, 
            0,
            REG_BINARY, 
            (BYTE *)&AffinityMask, 
            sizeof(AffinityMask));
        if (lRetVal != ERROR_SUCCESS)
        {
            Warn("AssignmentSetOverride value can not be set\n");
            break;
        }
        else
        {
            Info("AssignmentSetOverride value set to 0x%08x\n", AffinityMask);
        }
    } while(FALSE);

    return;
}

void RemoveAffinityPolicy(HKEY hAffinityPolicy)
{
    LONG    lRetVal;
    do 
    {
        lRetVal = RegDeleteValue(hAffinityPolicy, REGVAL_DEVICEPOLICY);
        if (lRetVal != ERROR_SUCCESS)
        {
            Warn("DevicePolicy value can't be deleted with code %d\n", lRetVal);
            break;
        }
        else
        {
            Info("DevicePolicy value has been deleted\n");
        }

        lRetVal = RegDeleteValue(hAffinityPolicy, REGVAL_ASSIGNMENTSETOVERRIDE);
        if (lRetVal != ERROR_SUCCESS)
        {
            Warn("AssignmentSetOverride value can't be deleted with code %d\n", lRetVal);
            break;
        }
        else
        {
            Info("AssignmentSetOverride value has been deleted\n");
        }
    } while(FALSE);
    
    return;

}
//
//If targetDevName is empty, do for current device, otherwise only do 
//when current device equals to target
//
void DoForDevice(HDEVINFO hDevInfo, 
                 SP_DEVINFO_DATA *pDevInfoData, 
                 PCSTR targetDevName, 
                 DWORD AffinityMask, 
                 BOOL fAdd)
{
    LPTSTR  deviceName = NULL;
    DWORD   regDataType;
    HKEY    hAffinityPolicy = NULL;
    DWORD   regValueSize;
    LONG    lRetVal = ERROR_SUCCESS;
    DWORD   DevicePolicy;
    DWORD   regValueType;

    do
    {
        deviceName = GetDeviceRegistryProperty(
                        hDevInfo, 
                        pDevInfoData, 
                        SPDRP_DEVICEDESC, 
                        REG_SZ, 
                        &regDataType);
        if (NULL == deviceName)
        {
            Warn("Unknown Device: No action \n");
            break;
        }
        
        if (targetDevName && strcmp(targetDevName, deviceName) != 0) 
        {
            break; //skip
        }


        Info("%s:\n", deviceName);

        //
        // Get a handle to the device's "Device Parameters\interrupt management\affinity policy" registry subkey
        //
        hAffinityPolicy = OpenAffinityPolicyKey(hDevInfo, pDevInfoData);
        if (hAffinityPolicy == INVALID_HANDLE_VALUE)
            break;

        if (fAdd)
        {
            AddAffinityPolicy(hAffinityPolicy, AffinityMask);
        }
        else
        {
            RemoveAffinityPolicy(hAffinityPolicy);
        }

    } while (FALSE);

    if (deviceName)
    {
        free( deviceName );
        deviceName = NULL;
    }
    
    SAFE_REG_CLOSE(hAffinityPolicy);
    
}

void AddRmKeyForAllDevice(DWORD AffinityMask, BOOL fAdd, PCSTR deviceName)
{
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA  devInfoData;
    int deviceIndex;
   
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
            DoForDevice(hDevInfo, &devInfoData, deviceName, AffinityMask, fAdd);
        }
    } while (FALSE);

    return;
}