#define _MBCS 

#include <windows.h>


extern "C" {
#include <ntddndis.h>

#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
#include <devguid.h>
#include <winsock2.h>
#include <winioctl.h>
#include <setupapi.h>
#include <ndisguid.h>
#include <string.h>
#include <mbstring.h>
}

#include <Commdlg.h>
#pragma comment(lib, "comdlg32.lib")

//

#define DOT11B_PLCP_DATA_RATE_1M     0x0A
#define DOT11B_PLCP_DATA_RATE_2M     0x14
#define DOT11B_PLCP_DATA_RATE_5P5M   0x37
#define DOT11B_PLCP_DATA_RATE_11M    0x6E

#define DOT11A_RATE_6M  0xB
#define DOT11A_RATE_9M  0XF
#define DOT11A_RATE_12M 0xA
#define DOT11A_RATE_18M 0xE
#define DOT11A_RATE_24M 0x9 // 1001
#define DOT11A_RATE_36M 0xD
#define DOT11A_RATE_48M 0x8
#define DOT11A_RATE_54M 0xC

#define SDR_PHY_MODE_DOT11A 1
#define SDR_PHY_MODE_DOT11B 2

#define DOT11B_PLCP_IS_LONG_PREAMBLE       0
#define DOT11B_PLCP_IS_SHORT_PREAMBLE      1

// define your control code here

#define IOCTL_START_MAC	        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STOP_MAC  		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_READ_REG          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_REG         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80C, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_TEST_MM_SPEED     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SHOW_STATISTICS   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x814, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DUMP_MODE         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x815, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_NOISE_TH      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x816, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_SDR_MODE      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x817, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_DUMMY_MODE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x818, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_CHANNEL       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x819, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LOAD_RAW_DATA     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x81A, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_POWER_CONTROL     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x81B, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_MAXIM       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x81C, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_DAC         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x81D, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// NSID Exp. IOCTL Code
#define IOCTL_TX_LARGE_PACKET   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x840, METHOD_BUFFERED, FILE_ANY_ACCESS)

void PrintHeader () {
    printf ("**********************************************\n");
    printf (" -- start                                     \n");
	printf (" -- stop                                      \n");
	printf (" -- memspeed                                  \n");
	printf (" -- dump [t | r | tr]                         \n");
	printf (" -- snt  ;Set Noise Threashold	               \n");
	printf (" -- mode b [0-long | 1-short] [1 | 2 | 5 | 11]\n");
	printf ("    mode a 0 [6 | 9 | 12 | 18 | 24 | 36 | 48 | 54] \n");
    printf (" -- load slot_number                          \n");
	printf (" -- dummy [0 | 1] slot_number                 \n");
	printf (" -- channel number /*1~14 for 2.4GHz or 36~161 for 5GHz*/\n");
    printf (" -- powerctrl [t | r] gain(hex)               \n");
	printf (" -- regr reg_addr                             \n");
	printf (" -- regw reg_addr reg_value                   \n");
    printf (" -- wmaxim reg_addr reg_value                 \n");
    printf (" -- wdac reg_addr reg_value                   \n");
	printf ("**********************************************\n");
}

void usage () 
{
}

INT
StrToHex(PCHAR pStr, INT Length)
{
    INT Hex  = 0;
    INT Unit = 1;
    
    for (INT i = Length - 1; i >= 0; i--)
    {
        if ((pStr[i] >= 'a' && pStr[i] <= 'f'))
        {
            Hex += (pStr[i] - 'a' + 10) * Unit;
        }
        else if (pStr[i] >= 'A' && pStr[i] <= 'F')
        {
            Hex += (pStr[i] - 'A' + 10) * Unit;
        }
        else if (pStr[i] >= '0' && pStr[i] <= '9')
        {
            Hex += (pStr[i] - '0') * Unit;
        }

        Unit *= 16;
    }

    return Hex;        
}

#define DEVNAME "\\\\.\\SDRHWDevice"

#define MAX_REQUEST_BUFFER 2048
char      RequestBuff [MAX_REQUEST_BUFFER];


int __cdecl
main(int argc, char **argv)
{
    DWORD BufferSize = sizeof(RequestBuff );
    BOOL bRet ;
    BOOL bQuery = FALSE;
    PrintHeader ();


    if ( argc < 2 ) 
    {
        usage ();
        return 0;
    }

    // Create a handle communicate to the driver
    HANDLE hDrv = CreateFile ( DEVNAME, 
        GENERIC_READ | GENERIC_WRITE, 
        0,  
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );
    if ( hDrv == INVALID_HANDLE_VALUE ) 
    {
        printf ( "Fail to create handle to the device!\n"
            "Please check if the device is properly installed!\n" );
        return 0;
    }

    do {
        if ( strcmp ( argv[1], "start" ) ==0 ) 
        {
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_START_MAC,
                NULL,
                0,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
        }
        else if ( strcmp (argv[1], "stop" ) == 0  ) 
        {
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_STOP_MAC,
                NULL,
                0,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
        }
        else if ( strcmp (argv[1], "memspeed" ) == 0  ) 
        {
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_TEST_MM_SPEED,
                NULL,
                0,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
        }else if ( strcmp (argv[1], "tlp" ) == 0  ) 
        {
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_TX_LARGE_PACKET,
                NULL,
                0,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
        }else if ( strcmp (argv[1], "dump" ) == 0  ) 
        {
            ULONG DumpBuffer[2] = {0};
            
            if(argc < 3) printf("Not enough parameters! dump [t | r | tr]");
            
            printf("%s %s\n", argv[1], argv[2]);
            
            if (strcmp (argv[2], "t" ) == 0) 
            { 
              DumpBuffer[0] = 1;
            }
            else if(strcmp (argv[2], "r" ) == 0) 
            { 
              DumpBuffer[1] = 1;
            }
            else if(strcmp (argv[2], "tr" ) == 0)
            { 
              DumpBuffer[0] = 1; DumpBuffer[1] = 1;
            }
            else
            {
              printf("Invalid parameters! dump [t | r | tr]"); 
            }
            
            printf("Buffer[0] = %d, Buffer[1] = %d\n", DumpBuffer[0], DumpBuffer[1]);
            
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_DUMP_MODE,
                DumpBuffer,
                2 * sizeof(ULONG),
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
                
            printf("Ret = %d, Buffer[0] = %d, Buffer[1] = %d\n", bRet, DumpBuffer[0], DumpBuffer[1]);
        }
        else if ( strcmp (argv[1], "sta" ) == 0  ) 
        {
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_SHOW_STATISTICS,
                NULL,
                0,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
        }
	    else if ( strcmp (argv[1], "snt" ) == 0  ) 
        {
            LONG NoiseTh = 150000;

            NoiseTh = atoi(argv[2]);            

            bRet = DeviceIoControl ( hDrv, 
                IOCTL_SET_NOISE_TH,
                &NoiseTh,
                4,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
            if(bRet){printf("Success!\n");} else {printf("Failed!\n");}
        }
     else if ( strcmp (argv[1], "dummy" ) == 0  ) 
        {
            LONG Buffer[3] = {0};
            
            if(argc < 3) {
                printf("Invalid arguments!  -- dummy {0 | 1} slot_number interval(unit - 1ms)\n");
                break;
            }

            Buffer[0] = atoi(argv[2]);

            if (Buffer[0] == 1)
            {
                if(argc < 4) {
                    printf("Invalid arguments!  -- dummy {0 | 1} slot_number interval(unit - 1ms)\n");
                    break;
                }

                Buffer[1] = atoi(argv[3]);


                if(argc < 5) {
                    printf("Invalid arguments!  -- dummy {0 | 1} slot_number interval(unit - 1ms)\n");
                    break;
                }

                Buffer[2] = atoi(argv[4]);
            }
            else
            {
                Buffer[1] = 0;
            }


            
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_SET_DUMMY_MODE,
                Buffer,
                12,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL 
                );
                
            if(bRet){ printf ("Dummy Success!\n");}else {printf("Dummy Failed!\n");}
        }
     else if ( strcmp (argv[1], "channel" ) == 0  ) 
        {
            LONG ChannelNumber = 0;
            
            if(argc < 3) {
                printf("Invalid arguments!  -- channel number\n");
                break;
            }

            ChannelNumber = atoi(argv[2]);
            
            
            if(ChannelNumber >= 1 && ChannelNumber <= 14);
            else if(ChannelNumber >= 36 && ChannelNumber <= 161);
            else{printf (" Invalid channel number! /*1~14 for 2.4GHz or 36~161 for 5GHz*/\n"); break;}
            
            bRet = DeviceIoControl ( hDrv, 
                IOCTL_SET_CHANNEL,
                &ChannelNumber,
                4,
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
                     
            if(bRet){ printf ("Success!\n");} else {printf("Failed!\n");}
        }        
	    else if ( strcmp (argv[1], "mode" ) == 0  ) 
        {
            ULONG Buffer[3] = {0};
            
            printf("arg2 = %s, arg3 = %s, arg4 = %s\n", argv[2], argv[3], argv[4]);

            //Buffer[0] = atoi(argv[2]);
            Buffer[1] = atoi(argv[3]);
	    Buffer[2] = atoi(argv[4]);
            
            // Default dot11b
            if(*argv[2] == 'a')
            {
               Buffer[0] = SDR_PHY_MODE_DOT11A;
            }
            else if (*argv[2] == 'b')
            {
                Buffer[0] = SDR_PHY_MODE_DOT11B;
            }
            else
            {
                Buffer[0] = SDR_PHY_MODE_DOT11B;
            }
	    
	    // Default long preamble
	    if(Buffer[1] != 1 && Buffer[1] != 0)
            {
                Buffer[1] = 0;
            }
            
	    if(Buffer[0] == SDR_PHY_MODE_DOT11B)
	    {		    
		    switch(Buffer[2])
		    {
		      case 1:
			Buffer[2] = DOT11B_PLCP_DATA_RATE_1M;
			break;
		      case 2:
			Buffer[2] = DOT11B_PLCP_DATA_RATE_2M;
			break;
		      case 5:
			Buffer[2] = DOT11B_PLCP_DATA_RATE_5P5M;
			break;
		      case 11:
			Buffer[2] = DOT11B_PLCP_DATA_RATE_11M;
			break;
		      default:
			Buffer[2] = DOT11B_PLCP_DATA_RATE_1M;
			break;
		    }
	    }
	    else if (Buffer[0] == SDR_PHY_MODE_DOT11A)
	    {
		    switch(Buffer[2])
		    {
		      case 6:
			Buffer[2] = DOT11A_RATE_6M;
			break;
		      case 9:
			Buffer[2] = DOT11A_RATE_9M;
			break;
		      case 12:
			Buffer[2] = DOT11A_RATE_12M;
			break;
		      case 18:
			Buffer[2] = DOT11A_RATE_18M;
			break;
		      case 24:
			Buffer[2] = DOT11A_RATE_24M;
			break;
		      case 36:
			Buffer[2] = DOT11A_RATE_36M;
			break;
		      case 48:
			Buffer[2] = DOT11A_RATE_48M;
			break;
		      case 54:
			Buffer[2] = DOT11A_RATE_54M;
			break;
		      default:
			Buffer[2] = DOT11A_RATE_6M;
			break;
		    }
	    }
            
            printf("PhyMode = %d, Preamble = %d DataRate = %x\n", Buffer[0], Buffer[1], Buffer[2]);

            bRet = DeviceIoControl ( hDrv, 
                IOCTL_SET_SDR_MODE,
                &Buffer,
		3 * sizeof(ULONG),
                RequestBuff,
                BufferSize,
                &BufferSize,
                NULL );
                
            if(bRet) printf("Success!\n"); else printf("Invalid Parameters!\n");
            
        }else if (strcmp(argv[1], "regr") == 0){
            if(argc < 3) {
                printf("Invalid arguments!  -- regr reg_addr\n");
                break;
            }
            
            ULONG Buffer[6] = {0};
            
            Buffer[0] = StrToHex(argv[2], strlen(argv[2]));
            Buffer[1] = 1;
                        
            bRet = DeviceIoControl ( hDrv, 
                    IOCTL_READ_REG,
                    Buffer,
                    2 * sizeof(ULONG),
                    RequestBuff,
                    2 * sizeof(ULONG),
                    &BufferSize,
                    NULL 
                    );
            
            if(bRet){
                printf("Read success! Reg[0x%x] = 0x%x Cycles=%d\n", Buffer[0], *((PULONG)RequestBuff), *((PULONG)RequestBuff + 1));
            }else{
                printf("Read failed!\n");
            }
        }else if (strcmp(argv[1], "regw") == 0){
            if(argc < 4) {
                printf("Invalid arguments!  -- regw reg_addr reg_value\n");
                break;
            }
            
            ULONG Buffer[6] = {0};
            
            Buffer[0] = StrToHex(argv[2], strlen(argv[2]));
            Buffer[1] = StrToHex(argv[3], strlen(argv[3]));
            Buffer[2] = 1;

            bRet = DeviceIoControl ( 
                    hDrv, 
                    IOCTL_WRITE_REG,
                    Buffer,
                    3 * sizeof(ULONG),
                    RequestBuff,
                    sizeof(ULONG),
                    &BufferSize,
                    NULL 
                    );

            if(bRet){
                printf("Write Success! Cycles=%d\n", *((PULONG)RequestBuff));
            }else{
                printf("Write failed!\n");
            }
        }else if (strcmp(argv[1], "powerctrl") == 0)
        {
            if(argc < 4) {
                printf("Invalid arguments!  -- powerctrl [t | r] gain\n");
                break;
            }

            ULONG Buffer[6] = {0};

            if (strcmp(argv[2], "t") == 0)
            {
                Buffer[0] = 1;
            }
            else if (strcmp(argv[2], "r") == 0)
            {
                Buffer[0] = 2;
            }
            else
            {
                printf("Invalid arguments!  -- powerctrl [t | r] gain\n");
                break;
            }

            Buffer[1] = StrToHex(argv[3], strlen(argv[3]));

            bRet = DeviceIoControl ( 
                hDrv, 
                IOCTL_POWER_CONTROL,
                Buffer,
                2 * sizeof(ULONG),
                RequestBuff,
                sizeof(ULONG),
                &BufferSize,
                NULL 
                );

            if(bRet){
                printf("Success! \n");
            }else{
                printf("Fail!\n");
            }

        }else if (strcmp(argv[1], "load") == 0){
            // Load a file to TX_SLOT
            if(argc < 3) {
                printf("Invalid arguments!  -- load slot_number\n");
                break;
            }

            SHORT  SlotNumber     = (SHORT)atoi(argv[2]);

            WCHAR  InputFileNameBuf[1024] = {0};

            InputFileNameBuf[0] = SlotNumber;

            OPENFILENAMEW OpenFileName;

            memset(&OpenFileName, 0, sizeof(OPENFILENAMEW));

            OpenFileName.lStructSize    = sizeof(OPENFILENAMEW);
            OpenFileName.lpstrFilter    = L"Dump file(*.tdmp)\0*.tdmp\0All file(*.*)\0*.*\0\0";
            OpenFileName.lpstrFile      = &InputFileNameBuf[1];
            OpenFileName.nMaxFile       = 1024;
            OpenFileName.lpstrTitle     = L"Open Dumped File...";
            OpenFileName.lpstrDefExt    = L"tdmp";
            OpenFileName.Flags          |= OFN_HIDEREADONLY;

            if (!GetOpenFileNameW(&OpenFileName))
            {
                break;
            }

            //
            // The first used to store the slot number
            // The following used to store path
            //

            bRet = DeviceIoControl ( 
                hDrv,
                IOCTL_LOAD_RAW_DATA,
                InputFileNameBuf,
                1024 * sizeof(WCHAR),
                RequestBuff,
                sizeof(ULONG),
                &BufferSize,
                NULL 
                );

            if ( bRet )
            {
                printf("Load Success!\n");
            }
            else
            {
                printf("Load failed!\n");
            }
        }else if (strcmp(argv[1], "wmaxim") == 0){
            // Load a file to TX_SLOT
            if(argc < 4) {
                printf("Invalid arguments!  -- wmaxim reg_addr reg_value\n");
                break;
            }

            ULONG Buffer[2] = {0};

            Buffer[0] = StrToHex(argv[2], strlen(argv[2]));
            Buffer[1] = StrToHex(argv[3], strlen(argv[3]));

            printf(" Maxim -- Reg = %x, Value = %x\n", Buffer[0], Buffer[1]);

            bRet = DeviceIoControl ( 
                hDrv,
                IOCTL_WRITE_MAXIM,
                Buffer,
                2 * sizeof(ULONG),
                RequestBuff,
                sizeof(ULONG),
                &BufferSize,
                NULL 
                );

            if ( bRet )
            {
                printf("Write Maxim Success!\n");
            }
            else
            {
                printf("Write Maxim failed!\n");
            }
        }
        else if (strcmp(argv[1], "wdac") == 0){
            // Load a file to TX_SLOT
            if(argc < 4) {
                printf("Invalid arguments!  -- wdac reg_addr reg_value\n");
                break;
            }

            ULONG Buffer[2] = {0};

            Buffer[0] = StrToHex(argv[2], strlen(argv[2]));
            Buffer[1] = StrToHex(argv[3], strlen(argv[3]));

            printf(" DAC -- Reg = %x, Value = %x\n", Buffer[0], Buffer[1]);

            bRet = DeviceIoControl ( 
                hDrv,
                IOCTL_WRITE_DAC,
                Buffer,
                2 * sizeof(ULONG),
                RequestBuff,
                sizeof(ULONG),
                &BufferSize,
                NULL 
                );

            if ( bRet )
            {
                printf("Write DAC Success!\n");
            }
            else
            {
                printf("Write DAC failed!\n");
            }
        }
        else
        {
            usage ();
            break;
        }
    }while (0);
    CloseHandle (hDrv);

    return 0;
}



