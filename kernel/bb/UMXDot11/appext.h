#pragma once

#include "sora.h"
#include "config.h"
#include "monitor.h"
#include "soratime.h"

#define IFFT_MAG_SHIFT	0
#define TARGET_RADIO	0

#define DataBufferSize      4096

#define _K(N)			(1024 * (N))
#define _M(N)			(1024 * 1024 * (N))

void Dot11ARxApp(const Config& config);
void Dot11ATxApp(const Config& config);
void Dot11BRxApp(const Config& config);
void Dot11BTxApp(const Config& config);
void Dot11ARxApp_Brick(const Config& config);
void Dot11ATxApp_Brick(const Config& config);
void Dot11BRxApp_Brick(const Config& config);
void Dot11BTxApp_Brick(const Config& config);
void Dot11BRxApp_FineBrick(const Config& config);
void Dot11BTxApp_FineBrick(const Config& config);

struct TxContext
{
    const Config& config;
    Monitor& monitor;

    TxContext(const Config& config, Monitor& monitor)
        : config(config)
        , monitor(monitor)
    { }

    TxContext& operator=(const TxContext&);
};

SELECTANY TIMESTAMPINFO        tsinfo;
