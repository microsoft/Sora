/*++
Copyright (c) Microsoft Corporation

Module Name: sdr_mac_precomp.h

Abstract: precompilation header for MAC

History: 

--*/
#define WIN9X_COMPAT_SPINLOCK
#include "ntifs.h"
#include <ntddk.h>
#pragma warning(disable:4201)  // nameless struct/union warning
#pragma warning(disable:4127) // avoid conditional expression is constant error with W4
#include <ndis.h>
#include <wdm.h>
#include <initguid.h> // required for GUID definitions
#include <wdmguid.h>  // required for WMILIB_CONTEXT

#include "sora.h"

//this file defines header format for ethnet packet and wlan packet
#include "dot11_pkt.h"

#include "sdr_mac_send_queue.h"
#include "sdr_mac_recv_queue.h"
#include "sdr_phy.h"
#include "sdr_mac.h"

