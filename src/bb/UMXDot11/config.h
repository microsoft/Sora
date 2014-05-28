// Functions for reading config from file (*.exe.ini)

#pragma once
#include <string>
#include <Windows.h>
#include "bb/DataRate.h"

// ref: 802.11a-1999 17.2.2.1
const unsigned int MAX_TXVECTOR_LENGTH = 4095;

enum PROTOCOL
{
    PROTOCOL_802_11A,
    PROTOCOL_802_11B,
    PROTOCOL_802_11A_BRICK,
    PROTOCOL_802_11B_BRICK,
};

class Config
{
    unsigned int DataRate;
    unsigned int PayloadLength;
    PROTOCOL Protocol;
    unsigned int SampleRate;
    unsigned int interval; // default 0 means infinit

public:
    // DataRate in kbps
    unsigned int GetDataRate() const { return DataRate; }

    // PayloadLength in bytes
    unsigned int GetPayloadLength() const { return PayloadLength; }

    // SampleRate in MHz
    unsigned int GetSampleRate() const { return SampleRate; }

    unsigned int& Interval() { return interval; }
    unsigned const int& Interval() const { return interval; }

    PROTOCOL GetProtocol() const { return Protocol; }

    Config(bool modeTX)
        : interval(0)
    {
        // Get name of this executable
        char buf[MAX_PATH];
        GetModuleFileName(NULL, buf, sizeof(buf)/sizeof(buf[0]));

        // Concat it with ".ini", get the config file name
        std::string fnConfig = std::string(buf) + ".ini";

        // Read config from file
        SampleRate = GetPrivateProfileInt("Hardware", "SampleRate", 0, fnConfig.c_str());
        if (SampleRate == 0) throw std::runtime_error("SampleRate error in config file " + fnConfig);

        char value[MAX_PATH];
        GetPrivateProfileString("Modulation", "Protocol", NULL, value, sizeof(value)/sizeof(value[0]), fnConfig.c_str());
        if (strcmp(value, "802.11a") == 0)
            Protocol = PROTOCOL_802_11A;
        else if (strcmp(value, "802.11b") == 0)
            Protocol = PROTOCOL_802_11B;
        else if (strcmp(value, "802.11a.brick") == 0)
            Protocol = PROTOCOL_802_11A_BRICK;
        else if (strcmp(value, "802.11b.brick") == 0)
            Protocol = PROTOCOL_802_11B_BRICK;
        else
            throw std::runtime_error("Protocol error in config file " + fnConfig);

        // Don't need other parameters in RX mode
        if (!modeTX) return;

        DataRate = GetPrivateProfileInt("Modulation", "DataRate", 0, fnConfig.c_str());
        if (DataRate == 0) throw std::runtime_error("DataRate error in config file " + fnConfig);

        PayloadLength = GetPrivateProfileInt("Modulation", "PayloadLength", 0, fnConfig.c_str());
        if (PayloadLength == 0) throw std::runtime_error("PayloadLength error in config file " + fnConfig);
        
        switch(Protocol)
        {
        case PROTOCOL_802_11A:
        case PROTOCOL_802_11A_BRICK:
            if (PayloadLength > MAX_TXVECTOR_LENGTH - 4)
                throw std::runtime_error("PayloadLength too large in config file " + fnConfig);

            if (!Dot11ARate_KbpsValid(DataRate)) throw std::runtime_error("DataRate invalid in config file " + fnConfig);
            break;
        case PROTOCOL_802_11B:
        case PROTOCOL_802_11B_BRICK:
            if (PayloadLength > 2048)
                throw std::runtime_error("PayloadLength too large in config file " + fnConfig);

            if (!Dot11BRate_KbpsValid(DataRate)) throw std::runtime_error("DataRate invalid in config file " + fnConfig);
            break;
        default:
            NODEFAULT;
        }
    }
};
