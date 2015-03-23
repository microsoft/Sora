#include "mac.h"


// MAC states
#pragma warning(disable:4345)

ISource* pCSGraph = NULL;
ISource* pRxGraph = NULL;

MAC_STATE 			current_state;


// Handler pointer
BOOLEAN MAC_CarrierSense   (void*);
BOOLEAN MAC_Send           (void*);
BOOLEAN MAC_Receive        (void*);

static MAC_STATE_HANDLER* STATE[] = {
	MAC_CarrierSense,
	MAC_Receive,
	MAC_Send
};

void CreateBasebandGraph () {
	pCSGraph = ConfigureCSGraph ();
	pRxGraph = ConfigureRxGraph ();
}

void ReleaseBasebandGraph () {
	if ( pCSGraph ) {
		IReferenceCounting::Release (pCSGraph);
		pCSGraph = NULL;
	}
	
	if ( pRxGraph ) {
		IReferenceCounting::Release (pRxGraph);
		pRxGraph = NULL;
	}
}

extern TIMESTAMPINFO tsinfo;


BOOLEAN MAC_CarrierSense (void*) {
	pCSGraph->Process ();
	if ( csContext.CF_EnergyDetect::high_energy_flag() ) {

		rxContext.CF_DCRemove::average_DC() = csContext.CF_DCRemove::average_DC();
		rxContext.CF_EnergyDetect::high_energy_flag() = TRUE;
		
		// carrier sense true
		current_state = MAC_STATE_RX;
		pRxGraph->Reset ();
		
		ULONGLONG us = SoraGetTimeofDay ( &tsinfo );
		SORATIMEOFDAYINFO info;
		
		SoraParseTime ( &info, us, &tsinfo );
		printf ( "%0d:%02d:%02d:%03d.%03d:High power detected! (DC %d %d)\n", 
			info.hh, info.mm, info.ss, info.ms, info.us, 
			csContext.CF_DCRemove::average_DC().re,
			csContext.CF_DCRemove::average_DC().im
			);
	}
	return TRUE;
}

BOOLEAN MAC_Receive (void*) {
	pRxGraph->Process ();
	if ( !rxContext.CF_EnergyDetect::high_energy_flag() ) {
		current_state = MAC_STATE_CS;
		
		ULONGLONG us = SoraGetTimeofDay ( &tsinfo );
		SORATIMEOFDAYINFO info;
		
		SoraParseTime ( &info, us, &tsinfo );
		printf ( "%0d:%02d:%02d:%03d.%03d:High power lost!\n", info.hh, info.mm, info.ss, info.ms, info.us );
	}
	
	return TRUE;
}


BOOLEAN MAC_Send (void*) {

	current_state = MAC_STATE_CS;
	return TRUE;
}

BOOLEAN RxProc ( void * args ) 
{
	return STATE[current_state](args);

	/*

	pGraph->Process ();
//	int data = graphContext.CF_AvePower::average_power();
//	PlotLine ( "Mon::Power", &data, 1 );

	COMPLEX16 dc = graphContext.CF_DCRemove::average_DC();
//	printf ( "DC (%d, %d) - DC power %d \n", dc.re, dc.im, norm2(dc));
	
	return true;
	*/
}



