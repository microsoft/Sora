#pragma once

#include "rxstream.h"
#include "diagnostics.h"

//
// 	TRxStream:: reads I/Q samples from a RxStream 
// 	downstream 
//
//	Required facade: CF_RxStream
//
DEFINE_LOCAL_CONTEXT(TRxStream, CF_RxStream, CF_Error);
template < TSOURCE_ARGS > 
class TRxStream : public TSource<TSOURCE_PARAMS>
{
protected:
	CTX_VAR_RO (PSORA_RADIO_RX_STREAM, rxstream_pointer );
	CTX_VAR_RW (FLAG, 				   rxstream_touched );
	CTX_VAR_RW (ulong, 				   error_code );

    // constance
    const static int YIELD_SIGNALBLOCK_COUNT = 40 * 8 * 1000 / 28; // number of SignalBlocks in 8ms (ie. sora scheduler switch time)

public:
	DEFINE_OPORT(COMPLEX16, 28);
	REFERENCE_LOCAL_CONTEXT(TRxStream);
	
	// brick interfaces
    STD_TSOURCE_CONSTRUCTOR(TRxStream)
    	BIND_CONTEXT (CF_RxStream::rxstream_pointer, rxstream_pointer)
		BIND_CONTEXT (CF_RxStream::rxstream_touched, rxstream_touched)
		BIND_CONTEXT (CF_Error::error_code,   error_code  )
    { }
	STD_TSOURCE_RESET() { }
	STD_TSOURCE_FLUSH() { }
	
    bool Process () {
        int yieldcounter = YIELD_SIGNALBLOCK_COUNT;
		HRESULT hr;
		do {
			//
    		// pump each signal block to downstream
			//
			SignalBlock * po = (SignalBlock *) opin().append();
		    hr = SoraRadioReadRxStream (rxstream_pointer, &rxstream_touched, *po);

			if ( SUCCEEDED (hr) ) {
				Next()->Process(opin());
			} else {
				error_code = BK_ERROR_HARDWARE_FAILED;
			}

            // Yield to caller to prevent OS hanging
            // The common cause is the logic error in brick graph, which take long time continuous processing without returning
            // control to the sora scheduler, so the scheduler doesn't have opportunity to schedule it to other cores.
            yieldcounter--;
            if (yieldcounter <= 0)
            {
                error_code = BK_ERROR_YIELD;
            }

		} while (!rxstream_touched && (error_code == BK_ERROR_SUCCESS));
		return true;
    }

    int Seek(int offset)
    {
		HRESULT hr;
        SignalBlock block;

        if (offset == END_POS)
        {
            // Seek to end of the stream, ie. flush all existing blocks in the stream
            int moved = 0;
            do {
                hr = SoraRadioReadRxStream (rxstream_pointer, &rxstream_touched, block);
                if (FAILED (hr)) break;
                moved += SignalBlock::size * vcs::size;
            } while (!rxstream_touched);
            return moved;
        }

        // Note: if the offset is large than available item in the stream currently,
        // wait until enough, and then seek
        int expected = offset;
        while(offset > 0)
        {
            hr = SoraRadioReadRxStream (rxstream_pointer, &rxstream_touched, block);
            if (FAILED (hr)) break;
            offset -= SignalBlock::size * vcs::size;
        }
        return expected - offset;
    }
};

struct RxStreams
{
protected:
    static const ULONG RX_STREAM_TIMESTAMP_UNIT    = 28;
    size_t nstream;
    SORA_RADIO_RX_STREAM *rxStream[MAX_RADIO_NUMBER];

    RxStreams() : nstream(0) { }

public:
    FINL void Init(size_t nstream_, SORA_RADIO_RX_STREAM rxStream_[])
    {
        nstream = nstream_;
        for (size_t i = 0; i < nstream; i++)
        {
		    rxStream[i] = &rxStream_[i];
        }
    }

    // Returns:
    //  false if hardware error
    bool EstablishSync()
    {
        ULONG i;
        const ULONG Count = nstream;

        // raise to realtime to sync RxStream read position
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        ULONG max_timestamp;
        ULONG last_timestamp = 0;
        ULONG clear_count = 0;
        for(clear_count=0; clear_count < 2; clear_count++) {
            for(i=0; i < Count; i++) {
                last_timestamp = ClearRxStream(rxStream[i]);
                if (last_timestamp == ULONG_MAX) return false;
                //printf("radio: %d, last_timestamp: 0x%08x\n", i, last_timestamp);
                if (i) {
                    if (last_timestamp > max_timestamp) {
                        if (last_timestamp - max_timestamp > LONG_MAX) {
                            // timestamp overflow !!! max_timestamp is still the max, do nothing
                        }
                        else 
                            max_timestamp = last_timestamp;
                    }
                    else
                        if (max_timestamp > last_timestamp) {
                            if (max_timestamp - last_timestamp > LONG_MAX) {
                                // timestamp overflow !!!
                                max_timestamp = last_timestamp;
                            }
                        }
                }
                else
                    max_timestamp = last_timestamp;
            }
        }
        //printf("max_timestamp: 0x%08x\n", max_timestamp);
        ULONG final_timestamp =  max_timestamp + DUMP_BUFFER_DELAY_TIMESTAMP;

        bool rc = true;
        for (i = 0; i < Count; i++)
        {
            ULONG stop = ClearRxStreamsToTimestamp(rxStream[i], final_timestamp);
            if (stop != final_timestamp)
            {
                rc = false;
                break;
            }
        }
        
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
        return rc;
    }

    // Returns:
    //  false if hardware error
    bool SeekEnd()
    {
        ULONG target_timestamp = ClearRxStream(rxStream[0]);
        if (target_timestamp == ULONG_MAX) return false;

        size_t iss;
        for (iss = 1; iss < nstream; iss++)
        {
            ULONG stop = ClearRxStreamsToTimestamp(rxStream[iss], target_timestamp);
            if (stop != target_timestamp) return false;
        }
        return true;
    }

private:
    static const ULONG DUMP_BUFFER_DELAY_TIMESTAMP = 56*10;
    static const ULONG DUMP_BUFFER_SIZE			   = 16*1024*1024;

    // Returns:
    //  the last timestamp met, if anything unexpected happens, return ULONG_MAX (not a valid timestamp)
    ULONG ClearRxStream(PSORA_RADIO_RX_STREAM pRxStream)
    {
        PRX_BLOCK rx_block;
        FLAG fReachEnd = 0;
        HRESULT hr;	
        ULONG last_timestamp = 0;
        ULONG skip_number = 0;
        rx_block = SoraRadioGetRxStreamPos(pRxStream);	
        while (1) {
            hr = SoraCheckSignalBlock(
                rx_block,
                SoraGetStreamVStreamMask(pRxStream),
                1,
                &fReachEnd);
            if (hr == E_FETCH_SIGNAL_HW_TIMEOUT)
                break;
            else if (FAILED(hr))
            {
                last_timestamp = ULONG_MAX;
                break;
            }

            skip_number ++;
            last_timestamp = rx_block->blocks->Desc.TimeStamp;
            rx_block = __SoraRadioIncRxStreamPointer(pRxStream, rx_block);
        }
        SoraRadioSetRxStreamPos(pRxStream, rx_block);
        return last_timestamp;
    }

    // Returns:
    //  the last timestamp met, if anything unexpected happens, return ULONG_MAX (not a valid timestamp)
    ULONG ClearRxStreamsToTimestamp(PSORA_RADIO_RX_STREAM pRxStream, ULONG timestamp)
    {
	    HRESULT hr;
	    FLAG touch;
	    PRX_BLOCK rxBlock;

        const int nblocks = SoraRadioGetRxStreamSize(pRxStream);
        int i;
        for (i = 0; i < nblocks; i++)
        {
			rxBlock = SoraRadioGetRxStreamPos(pRxStream);
			hr = SoraCheckSignalBlock(
				rxBlock,
				SoraGetStreamVStreamMask(pRxStream),
				8000,
				&touch);
			if (FAILED(hr)) return ULONG_MAX;

			__SoraRadioAdvanceRxStreamPos(pRxStream);

            if (rxBlock->u.Desc.TimeStamp >= timestamp)
            {
    			return rxBlock->u.Desc.TimeStamp;
            }
		}
        // Clear RX stream for one round and still cannot exceed expected timestamp, possible hardware error
        return ULONG_MAX;
    }
};

DEFINE_LOCAL_CONTEXT(TRxStream2, CF_Error);
template < TSOURCE_ARGS > 
class TRxStream2 : public TSource<TSOURCE_PARAMS>, public RxStreams
{
protected:
	CTX_VAR_RW (ulong, 				   error_code );

    // constance
    const static int YIELD_SIGNALBLOCK_COUNT = 40 * 8 * 1000 / 28; // number of SignalBlocks in 8ms (ie. sora scheduler switch time)

	// local state
    SignalBlock sigblk;

public:
    static const size_t NSTREAM = 2;
    DEFINE_OPORT(COMPLEX16, 28, NSTREAM);
	REFERENCE_LOCAL_CONTEXT(TRxStream2);
	
	// brick interfaces
    STD_TSOURCE_CONSTRUCTOR(TRxStream2)
		BIND_CONTEXT (CF_Error::error_code,   error_code  )
    {
        _reset();
    }
	STD_TSOURCE_RESET() { _reset(); }
	STD_TSOURCE_FLUSH() { }
	
    bool Process () {
        int yieldcounter = YIELD_SIGNALBLOCK_COUNT;
        FLAG rxstream_touched = 0; // Any radio stream touched? ie. read to the latest block
		HRESULT hr;
        ULONG timestamp[2];
		do {
    		// Pump signal block from all streams to downstream
            FLAG touch;
            size_t iss;
            for (iss = 0; iss < NSTREAM; iss++)
            {
			    SignalBlock * po = (SignalBlock *) opin().write(iss);
                hr = SoraRadioReadRxStreamEx(rxStream[iss], &touch, *po, timestamp[iss]);
                if (touch == 1) rxstream_touched = 1;
                if (FAILED(hr))
                {
				    error_code = BK_ERROR_HARDWARE_FAILED;
                    return true;
                }

                // Note: arrange branches for better runtime performance
                if (timestamp[iss] == expected_timestamp)
                {
                    // Do nothing
                }
                else if (timestamp[iss] == expected_timestamp + RxStreams::RX_STREAM_TIMESTAMP_UNIT)
                {
                    // Meet single timestamp drop
                    TraceOutput("TRxStream2: (%d) single timestamp drop: (%u) %u\n", iss, expected_timestamp, timestamp[iss]);
                    error_code = BK_ERROR_TIMESTAMP_DROP;
                    return true;
                }
                else if (expected_timestamp == ULONG_MAX)
                {
                    // Do nothing
                }
                else
                {
                    // Meet consequent timestamp drops, report as hardware failure
                    TraceOutput("TRxStream2: (%d) consequent timestamp drops: (%u) %u\n", iss, expected_timestamp, timestamp[iss]);
                    error_code = BK_ERROR_TIMESTAMP_DROPS;
                    return true;
                }
            }

            expected_timestamp = timestamp[0] + RxStreams::RX_STREAM_TIMESTAMP_UNIT;
            //TraceOutput("TIMESTAMP: %d, %d\n", timestamp[0], timestamp[1]);

            // Push to downstream
            opin().append();
			Next()->Process(opin());

            // Yield to caller to prevent OS hanging
            // The common cause is the logic error in brick graph, which take long time continuous processing without returning
            // control to the sora scheduler, so the scheduler doesn't have opportunity to schedule it to other cores.
            yieldcounter--;
            if (yieldcounter <= 0)
            {
                error_code = BK_ERROR_YIELD;
            }
		} while (!rxstream_touched && (error_code == BK_ERROR_SUCCESS));
		return true;
    }

    int Seek(int offset)
    {
        if (offset == ISource::END_POS)
        {
            expected_timestamp = ULONG_MAX;

            bool suc = RxStreams::SeekEnd();
            if (suc) return 0;

            // If not succeeded, possible reason is timestamp drop. Try again
            suc = RxStreams::SeekEnd();
            if (suc) return 0;

            // If not succeeded again, meet too much timestamp drops, report as hardware failure
            error_code = BK_ERROR_HARDWARE_FAILED;
        }

        return 0;
    }

private:
    ULONG expected_timestamp;
    FINL void _reset()
    {
        expected_timestamp = ULONG_MAX;
    }
};
