#include <stdio.h>
#include "appext.h"

void usage()
{
	printf("UMXDot11.exe tx/rx [options]\n");

	printf("rx : receive packets until user terminates.\n");
	printf("tx : transmit packets until user terminates.\n\n");
    printf("Options:\n");
    printf("  -t INTERVAL       terminated after INTERVAL second, default is infinit\n");
    printf("\n");
}

int __cdecl main(int argc, const char *argv[])
{
    try
    {
        // Parse command line parameters
	    if (argc <= 1)
	    {
		    usage();
		    return -1;
	    }

        int argi = 1;
        const char *argMode = argv[argi++];
        bool modeTX  = (strcmp(argMode, "tx") == 0);
        bool modeRX  = (strcmp(argMode, "rx") == 0);
        if (!modeTX && !modeRX)
        {
            usage();
            return -1;
        }

        Config config(modeTX);
        while(argi < argc)
        {
            if (argv[argi][0] != '-') { usage(); return -1; }
            if (strcmp(&argv[argi][1], "t") == 0)
            {
                if (++argi >= argc) { usage(); return -1; }
                int intv = atoi(argv[argi]);
                if (intv < 0) { usage(); return -1; }
                config.Interval() = intv;
            }
            argi ++;
        }


        // Init UMX
	    BOOLEAN succ = SoraUInitUserExtension("\\\\.\\HWTest");
	    if (!succ)
	    {
		    printf("can't find device \n");
            return -2;
	    }

        InitializeTimestampInfo(&tsinfo, false);
        
        if (modeTX)
            printf("[INFO] Mode: TX\n");
        else
            printf("[INFO] Mode: RX\n");

	    // Begin receive or transmit packets
        switch(config.GetProtocol())
        {
        case PROTOCOL_802_11A:
            printf("[INFO] Protocol: 802.11a\n");
            if (modeTX)
            {
                SoraURadioSetTxGain(TARGET_RADIO, SORA_RADIO_DEFAULT_TX_GAIN);
                Dot11ATxApp(config);
            }
            else
            {
                Dot11ARxApp(config);
            }
            break;
        case PROTOCOL_802_11A_BRICK:
            printf("[INFO] Protocol: 802.11a.brick\n");
            if (modeTX)
            {
                SoraURadioSetTxGain(TARGET_RADIO, SORA_RADIO_DEFAULT_TX_GAIN);
                Dot11ATxApp_Brick(config);
            }
            else
            {
                Dot11ARxApp_Brick(config);
            }
            break;
        case PROTOCOL_802_11B:
            printf("[INFO] Protocol: 802.11b\n");
            if (modeTX)
            {
                Dot11BTxApp(config);
            }
            else
                Dot11BRxApp(config);
            break;
        case PROTOCOL_802_11B_BRICK:
            printf("[INFO] Protocol: 802.11b.brick\n");
            if (modeTX)
            {
                Dot11BTxApp_FineBrick(config);
            }
            else
                Dot11BRxApp_FineBrick(config);
            break;
        default:
            NODEFAULT;
        }

        // Close UMX
	    SoraUCleanUserExtension();
	    printf("Clean user ext\n");

        return 0;

    }
    catch(std::runtime_error& ex)
    {
        fprintf(stderr, "[ERROR] %s\n", ex.what());
    }
    catch(std::exception& ex)
    {
        fprintf(stderr, "[ERROR] caught exception in main(): %s\n", ex.what());
    }
    catch(...)
    {
        fprintf(stderr, "[ERROR] caught unknown exception in main()\n");
    }
}
