#include "common.h"

LPTSTR
GetUpperFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    DWORD  regDataType;
    LPTSTR buffer = (LPTSTR) GetDeviceRegistryProperty( DeviceInfoSet,
                                                        DeviceInfoData,
                                                        SPDRP_UPPERFILTERS,
                                                        REG_MULTI_SZ,
                                                        &regDataType );

    return buffer;
}

BOOL FilterIsInstalledOnDevice( HDEVINFO hDevInfo,
                                PSP_DEVINFO_DATA pDevInfoData )
{
    LPTSTR mszUpperFilterList;
    BOOL ret = FALSE;
    // Get MultiSz list of upper filters installed on device
    mszUpperFilterList = GetUpperFilters( hDevInfo, pDevInfoData );

    if( NULL == mszUpperFilterList )
    {
        return FALSE; // failure
    }

    // Search the list to see if our filter is there
    // (NOTE: filter names are case-INsensitive)
    if( MultiSzSearch(FILTER_SERVICE_NAME, mszUpperFilterList, FALSE, NULL) )
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    
    free(mszUpperFilterList);
    return ret;  // not found, or error occurred
}

BOOLEAN
AddUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    )
{
    size_t length = 0; // character length
    size_t size   = 0; // buffer size
    LPTSTR buffer = NULL;
    
    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    buffer = GetUpperFilters( DeviceInfoSet, DeviceInfoData );

    if( NULL == buffer )
    {
        // Some error occurred while trying to read the 'UpperFilters'
        // registry value.  So maybe no such value exists yet, or it's
        // invalid, or some other error occurred.
        //
        // In any case, let's just try to install a new registry value for
        // 'UpperFilters'

        // make room for the string, string null terminator, and multisz null
        // terminator
        length = _tcslen(Filter) + 2;
        size   = length*sizeof(_TCHAR);
        buffer = malloc( size );
        if( NULL == buffer )
        {
            // Error: Unable to allocate memory
            Error("Heap corrupted\n");
            return FALSE ;
        }

        // copy the string into the new buffer
        _tcscpy(buffer, Filter);
        // make the buffer a properly formed multisz
        buffer[length-1]=_T('\0');
    }
    else
    {
        // add the driver to the driver list
        PrependSzToMultiSz(Filter, &buffer);
    }
    Info("Set upper filters to %s\n", buffer); 
    length = MultiSzLength(buffer);
    size   = length*sizeof(_TCHAR);

    // set the new list of filters in place
    if( !SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                           DeviceInfoData,
                                           SPDRP_UPPERFILTERS,
                                           (PBYTE)buffer,
                                           size )
        )
    {
        // Error: couldn't set device registry property
        free( buffer );
        return FALSE;
    }

    // no need for buffer anymore
    free( buffer );

    return TRUE;
}

BOOLEAN
RemoveUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    )
{
    size_t length = 0; // character length
    size_t size   = 0; // buffer size
    LPTSTR buffer;
    BOOL   success = FALSE;

    ASSERT(DeviceInfoData != NULL);
    ASSERT(Filter != NULL);

    buffer = GetUpperFilters( DeviceInfoSet, DeviceInfoData );

    if( NULL == buffer )
    {
        // if there is no such value in the registry, then there are no upper
        // filter drivers loaded, and we are done
        return TRUE;
    }
    else
    {
        // remove all instances of filter from driver list
        MultiSzSearchAndDeleteCaseInsensitive( Filter, buffer, &length );
    }

    length = MultiSzLength(buffer);

    ASSERT ( length > 0 );

    if( 1 == length )
    {
        // if the length of the list is 1, the return value from
        // MultiSzLength() was just accounting for the trailing '\0', so we can
        // delete the registry key, by setting it to NULL.
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_UPPERFILTERS,
                                                    NULL,
                                                    0 );
    }
    else
    {
        // set the new list of drivers into the registry
        size = length*sizeof(_TCHAR);
        success = SetupDiSetDeviceRegistryProperty( DeviceInfoSet,
                                                    DeviceInfoData,
                                                    SPDRP_UPPERFILTERS,
                                                    (PBYTE)buffer,
                                                    size );
    }

    // no need for buffer anymore
    free( buffer );

    if( !success )
    {
        // Error: couldn't set device registry property
        return FALSE;
    }

    return TRUE;
}
