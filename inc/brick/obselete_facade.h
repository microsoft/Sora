#pragma once 

class CF_VecDC {
	FACADE_FIELD (vcs, direct_current );
public:
	FINL void Reset () {
		set_zero (direct_current());
	}
};

class CF_AvePower {
	FACADE_FIELD (uint,  average_power );
};

class CF_DCRemove {
	FACADE_FIELD (COMPLEX16,  average_DC );
	FACADE_FIELD (uint,  DC_update_lock );
};
// end of facade definition on the stock bricks

class CF_MacState
{
public:
    enum MAC_STATE
    {
        MAC_CS,
        MAC_RX,
    };

    FACADE_FIELD(MAC_STATE, MacState);

    FINL void Reset()
    {
        MacState() = CF_MacState::MAC_CS;
    }

    FINL void MoveState_CS_RX()
    {
        MacState() = CF_MacState::MAC_RX;
    }

    FINL void MoveState_RX_CS(bool finished, bool good)
    {
        UNREFERENCED_PARAMETER((finished, good));
        MacState() = CF_MacState::MAC_CS;
    }
};

class CF_CRC32
{
    FACADE_FIELD(unsigned long, CurCrc32);

    // down counter for CRC calculating, should be initialized to the expected frame length,
    // including the CRC field.
    FACADE_FIELD(unsigned int, DownCounter);
public:
    FINL void Reset()
    {
        CurCrc32() = 0xFFFFFFFF;
    }

    // The comparison target is known as "magic sequence" for CRC32
    // ref: Martin Stigge, et al. Reversing CRC ¨C Theory and Practice
    static const unsigned long Crc32Target = ~ 0x2144DF1Cul;

    FINL void OnCrc32Match() { }
    FINL void OnCrc32Mismatch() { }
};
