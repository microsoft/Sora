#pragma once

#include <sora.h>


typedef long SPINLOCK; 
FINL void acquire_lock ( SPINLOCK* lock ) {
	while (1) {
		if ( ! (*lock) ) {
			if ( !InterlockedCompareExchange ( lock, 1, 0 ) ) {
				return;
			}
		} else {
			_mm_pause ();
		}
	}
}

FINL void release_lock ( SPINLOCK* lock ) {
	InterlockedExchange ( lock, 0 );
}

