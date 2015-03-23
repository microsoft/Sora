#ifndef _IO_CTRL_H
#define _IO_CTRL_H
#pragma once
#include "sora.h"
#define HWT_MAX_CACHED_SIGS     256

#define FILE_OBJ_NAME_PREFIX    "\\DosDevices\\"

typedef enum __DUT_CMD
{
    CMD_INVALID = -1, 
    CMD_RD_REG,
    CMD_WR_REG, 
    CMD_START_RADIO, 
    CMD_STOP_RADIO,
    CMD_TX, 
    CMD_TX_DONE, 
    CMD_INFO, 
    CMD_TRANSFER, 
    CMD_RX_GAIN, 
    CMD_RX_PA,
    CMD_TX_GAIN, 
    CMD_SAMPLE_RATE, 
    CMD_CENTRAL_FREQ,
    CMD_FREQ_OFFSET,
    CMD_DUMP,
    CMD_STOP_TX,
    CMD_DMA,
    CMD_FW_VERSION,
    CMD_MIMO_TX,
	CMD_RADIO_RD_REG,
	CMD_RADIO_WR_REG
}DUT_CMD;

#define DUT_IO_CODE(Cmd) CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x800 + (Cmd),\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

typedef struct _OFFSET_VALUE
{
    ULONG Offset;
    ULONG Value;
} OFFSET_VALUE, *POFFSET_VALUE;

typedef struct _START_RADIO {

	ULONG RadioNo;

} START_RADIO, *PSTART_RADIO;

typedef struct _STOP_RADIO {

	ULONG RadioNo;
} STOP_RADIO, *PSTOP_RADIO;

typedef struct _DEV_RET
{
    HRESULT hResult;
    UCHAR   data[0];
}DEV_RET, *PDEV_RET;

typedef struct _HWT_INFO
{
    ULONGLONG               ullBlocksLag;
    ULONGLONG               ullBlocksInTime;
} HWT_INFO, *PHWT_INFO;

typedef struct _ID_FILE_PAIR
{
    ULONG       PacketID;
    WCHAR       SymFileName[SYM_FILE_MAX_PATH_LENGTH];
} ID_FILE_PAIR, *PID_FILE_PAIR;

typedef struct _TIMES_ID_PAIR
{
	ULONG		RadioNo;
    ULONG       Times;
    ULONG       PacketID;
} TIMES_ID_PAIR, *PTIMES_ID_PAIR;

typedef struct _RADIO_PARAMETER {

	ULONG		RadioNo;
	ULONG		Value;
} RADIO_PARAMETER, *PRADIO_PARAMETER;

typedef struct _HWT_DETAIL_INFO
{
    HWT_INFO;
    ULONG			RadioMask;
    ID_FILE_PAIR    CachedSignals[HWT_MAX_CACHED_SIGS];
    ULONG           Count;

} HWT_DETAIL_INFO, *PHWT_DETAIL_INFO; 

typedef struct _HWT_DMA_RET
{
    PVOID UserVA;
    ULONG Size;
} HWT_DMA_RET, *PHWT_DMA_RET;

typedef struct _HWT_FW_VERSION
{
	s_uint32 m_fw_version;
	
} HWT_FW_VERSION, *PHWT_FW_VERSION;

#endif
