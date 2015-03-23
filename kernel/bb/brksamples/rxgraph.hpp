#pragma once

#include <sora.h>
#include <brick.h>
#include <stockbrk.hpp>
#include <soratime.h>

#include <timing_recovery.hpp>

extern ISource* pGraph;
extern SORA_RADIO_RX_STREAM	RxStream;	// RxStream
extern TIMESTAMPINFO tsinfo;

SELECTANY	
struct _tagCSContext:
	  LOCAL_CONTEXT(TDropAny)
	, LOCAL_CONTEXT(TEnergyDetect)	  	
	, LOCAL_CONTEXT(TDCRemove)	  
	, LOCAL_CONTEXT(TRxStream)
{
} csContext; // Carrier Sense context

FINL
ISource* ConfigureCSGraph () {
	CREATE_BRICK_SINK	( drop,   TDropAny, csContext );
	CREATE_BRICK_FILTER ( energy, TEnergyDetect, csContext, drop );
	CREATE_BRICK_FILTER ( dc,   TDCRemove, csContext, energy  );	
	CREATE_BRICK_SOURCE ( ssrc, TRxStream, csContext, dc );

	// configure the parameters
	csContext.CF_RxStream::rxstream_pointer() = &RxStream;
	csContext.CF_RxStream::rxstream_touched() = 0;
	csContext.CF_RxStream::minimal_blocks  () = 31; // ~20 us with 44 sampling rate

	csContext.CF_DCRemove::DC_update_lock() = 0;

	csContext.CF_EnergyDetect::energy_threshold() = 1000*1000 * 4;
	csContext.CF_EnergyDetect::high_energy_flag() = FALSE;

	ssrc->Reset ();

	return ssrc;
}

SELECTANY	
struct _tagRxContext :
	  LOCAL_CONTEXT(TDropAny)
	, LOCAL_CONTEXT(TEnergyDetect)	  		  	
	, LOCAL_CONTEXT(TDCRemove)	  
	, LOCAL_CONTEXT(TRxStream)
{
} rxContext; // Carrier Sense context

FINL
ISource* ConfigureRxGraph () {
	CREATE_BRICK_SINK	( drop, TDropAny, rxContext );
	CREATE_BRICK_FILTER ( symtiming, TSymTiming, rxContext, drop  );	
	CREATE_BRICK_FILTER ( energy, TEnergyDetect, csContext, symtiming );
	CREATE_BRICK_FILTER ( dc,   TDCRemove, rxContext, symtiming  );	
	CREATE_BRICK_SOURCE ( ssrc, TRxStream, rxContext, dc );

	// configure the parameters
	rxContext.CF_RxStream::rxstream_pointer() = &RxStream;
	rxContext.CF_RxStream::rxstream_touched() = 0;
	rxContext.CF_RxStream::minimal_blocks  () = 31; // ~20 us with 44 sampling rate

	rxContext.CF_DCRemove::DC_update_lock() = 1;

	csContext.CF_EnergyDetect::energy_threshold() = 1000*1000 * 4;
	csContext.CF_EnergyDetect::high_energy_flag() = FALSE;

	ssrc->Reset ();

	return ssrc;
}


