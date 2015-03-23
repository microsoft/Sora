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

    FINL void Init(size_t nstream_, SORA_RADIO_RX_STREAM *rxStream_[])
    {
        nstream = nstream_;
        for (size_t i = 0; i < nstream; i++)
        {
		    rxStream[i] = rxStream_[i];
        }
    }

};

class SampleRateDesc
{
protected:
    ulong rxsample_rate;

    SampleRateDesc()
        : rxsample_rate(40 /* MHz */)
    {
    }

public:
    // Returns true if succeeded
    FINL bool Init (size_t rxsample_rate_MHz)
	{
        rxsample_rate = rxsample_rate_MHz;
        return true;
	}
};

//
// TRxMIMOStream
//
class CF_MIMOStreams {
	FACADE_FIELD (ULONG, blk_drops      );
public:
	CF_MIMOStreams () {
		blk_drops ()     = 0;
	}
};

// used to track the exact sample has been processed
class CF_RxTimestamp {
	FACADE_FIELD (ULONG, committed_timestamp );
};

DEFINE_LOCAL_CONTEXT(TRxMIMOStream, CF_Error, CF_RxTimestamp, CF_MIMOStreams);
template<int NSTREAMS>
class TRxMIMOStream {
public:
template < TSOURCE_ARGS > 
class Filter : public TSource<TSOURCE_PARAMS>, public SampleRateDesc, public RxStreams
{
private:
    // constance
    int YIELD_SIGNALBLOCK_COUNT; // number of SignalBlocks in 8ms (ie. sora scheduler switch time)
    ULONG ts_inc_per_blk;

    ULONG expected_ts;

protected:
	CTX_VAR_RW (ULONG, error_code );
	CTX_VAR_RW (ULONG, committed_timestamp );
	CTX_VAR_RW (ULONG, blk_drops );

	void _reset () {
		blk_drops = 0;

        if (rxsample_rate == 40)
        {
            YIELD_SIGNALBLOCK_COUNT = 40 * 8 * 1000 / 28 * NSTREAMS; // number of SignalBlocks in 8ms (ie. sora scheduler switch time)
            ts_inc_per_blk    = 28;
        }
        else if (rxsample_rate == 20)
        {
            YIELD_SIGNALBLOCK_COUNT = 20 * 8 * 1000 / 28 * NSTREAMS; // number of SignalBlocks in 8ms (ie. sora scheduler switch time)
            ts_inc_per_blk    = 28 * 2;
        }
        else
        {
            error_code = BK_ERROR_INVALID_PARAM;
        }
	}

public:
    DEFINE_OPORT(COMPLEX16, 28, NSTREAMS);
	REFERENCE_LOCAL_CONTEXT(TRxMIMOStream);

	// brick interfaces
    STD_TSOURCE_CONSTRUCTOR(Filter)
		BIND_CONTEXT (CF_Error::error_code,   error_code  )
		BIND_CONTEXT (CF_RxTimestamp::committed_timestamp, committed_timestamp)
		BIND_CONTEXT (CF_MIMOStreams::blk_drops,   blk_drops  )
    {
        _reset();
    }

	STD_TSOURCE_RESET() { _reset(); }
	STD_TSOURCE_FLUSH() { }
	
    bool Process () {
        int yieldcounter = YIELD_SIGNALBLOCK_COUNT;
		HRESULT hr;
		FLAG touch;
		SignalBlock *po;

        size_t iss = 0;
		po = (SignalBlock *) opin().write(iss);
		hr = SoraRadioReadRxStreamEx( rxStream[iss], &touch, *po, expected_ts );
		if ( FAILED(hr)) {
			error_code = BK_ERROR_HARDWARE_FAILED;
            return true;
		}

        iss++;
        size_t synced = 1;
        while (yieldcounter > 0 && synced < NSTREAMS)
        {
            ULONG t;
    		FLAG f;
    		po = (SignalBlock *) opin().write(iss);
            hr = ReadStreamUntil(rxStream[iss], expected_ts, t, f, *po, yieldcounter);
            if ( yieldcounter <= 0)
                break;
		    if ( FAILED(hr)) {
			    error_code = BK_ERROR_HARDWARE_FAILED;
                return true;
		    }

            // This stream has more recently timestamp, sync all others to this
			LONG delta = LONG(t - expected_ts);
            if (delta > 0)
            {
				// something dropped
                touch = f;
                synced = 1;
                expected_ts = t;
				blk_drops += delta;
				error_code = BK_ERROR_TIMESTAMP_DROPS;
            }
            else
            {
                touch |= f;
                synced ++;
            }

            iss ++;
            if (iss >= NSTREAMS) iss = 0;
        }

        if (synced == NSTREAMS)
        {
		    committed_timestamp = expected_ts;
		    expected_ts += ts_inc_per_blk;
			opin().append();
			Next()->Process (opin());
        }
        else
        {
            error_code = BK_ERROR_YIELD;
        }
        return true;
	}

    FINL int Seek (int offset)
    {
        int yieldcounter = YIELD_SIGNALBLOCK_COUNT;
		HRESULT hr;
		FLAG touch;
		SignalBlock block;

        size_t iss = 0;
		hr = SoraRadioReadRxStreamEx( rxStream[iss], &touch, block, expected_ts );
		if ( FAILED(hr)) {
			error_code = BK_ERROR_HARDWARE_FAILED;
            return ISource::START_POS;
		}

        iss++;
        size_t synced = 1;
        while (yieldcounter > 0 && synced < NSTREAMS)
        {
            ULONG t;
    		FLAG f;
            hr = ReadStreamUntil(rxStream[iss], expected_ts, t, f, yieldcounter);
            if ( yieldcounter <= 0)
                break;
		    if ( FAILED(hr)) {
			    error_code = BK_ERROR_HARDWARE_FAILED;
                return ISource::START_POS;
		    }

            // This stream has more recently timestamp, sync all others to this
			LONG delta = LONG(t - expected_ts);
            if (delta > 0)
            {
				// something dropped
                touch = f;
                synced = 1;
                expected_ts = t;
				blk_drops += delta;
            }
            else
            {
                touch |= f;
                synced ++;

                // We have sync read all streams, but still not catch up with latest data, continue sync reading
                if (synced == NSTREAMS && !touch)
                {
                    synced = 0;
                }
            }

            iss ++;
            if (iss >= NSTREAMS) iss = 0;
        }

        if (synced == NSTREAMS)
        {
		    committed_timestamp = expected_ts;
		    expected_ts += ts_inc_per_blk;
            return ISource::END_POS;
        }
        else
        {
            error_code = BK_ERROR_YIELD;
            return ISource::START_POS;
        }
    }

private:
    FINL HRESULT ReadStreamUntil(SORA_RADIO_RX_STREAM *stream, ULONG target_timestamp, ULONG& final_timestamp, FLAG& touch, int yieldcounter)
    {
		SignalBlock block;
        return ReadStreamUntil(stream, target_timestamp, final_timestamp, touch, block, yieldcounter);
    }

    FINL HRESULT ReadStreamUntil(SORA_RADIO_RX_STREAM *stream, ULONG target_timestamp, ULONG& final_timestamp, FLAG& touch, SignalBlock& block, int yieldcounter)
    {
		FLAG f;
        touch = 0;
        do
        {
    		HRESULT hr = SoraRadioReadRxStreamEx(stream, &f, block, final_timestamp);
            if (FAILED(hr)) return hr;
            touch |= f;
            yieldcounter --;
        } while(LONG(final_timestamp - target_timestamp) < 0 && yieldcounter > 0);
        return S_OK;
    }
};
};
