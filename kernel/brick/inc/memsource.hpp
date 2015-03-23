#pragma once

#include "brick.h"
#include <sbuf.h>

//
// 	TMemSamples:: Loads I/Q samples from a memory buffer and pushs them 
// 	downstream 
//
//	Required facade: CF_MemSamples
//

//  to do: review the implementation once more
//
DEFINE_LOCAL_CONTEXT(TMemSamples, CF_MemSamples, CF_Error);
template < TSOURCE_ARGS > 
class TMemSamples : public TSource<TSOURCE_PARAMS>
{
protected:
	CTX_VAR_RO (COMPLEX16*, sample_buf );
	CTX_VAR_RO (uint,       sample_buf_size );	
	CTX_VAR_RO (uint,       sample_start_pos );
	CTX_VAR_RO (uint,       sample_count );
    CTX_VAR_RW (uint,       sample_index);

	CTX_VAR_RW (ulong, 	    error_code );

	// local state
	uint 		remain_sample;
	COMPLEX16*  sample_pointer;
	
public:
	DEFINE_OPORT(COMPLEX16, 28);

protected:
	FINL void __init () {
		if (   (sample_start_pos >= sample_count ) 
			|| (sample_count * sizeof(COMPLEX16) > sample_buf_size ) )
		{
			error_code = BK_ERROR_FAILED;
			return;
		}

        if (sample_buf == NULL)
        {
            error_code = BK_ERROR_FAILED;
            return;
        }

        sample_index = 0;
		remain_sample = sample_count;
		sample_pointer = sample_buf + sample_start_pos;

        // sample_pointer must point to the address 16 bytes aligned
        if ((size_t)sample_pointer % 16 != 0)
        {
            error_code = BK_ERROR_FAILED;
            return;
        }
	}
	
public:
	REFERENCE_LOCAL_CONTEXT(TMemSamples);
	
	// brick interfaces
    STD_TSOURCE_CONSTRUCTOR(TMemSamples)
    	BIND_CONTEXT (CF_MemSamples::mem_sample_buf, sample_buf)
    	BIND_CONTEXT (CF_MemSamples::mem_sample_buf_size, sample_buf_size)
    	BIND_CONTEXT (CF_MemSamples::mem_sample_start_pos, sample_start_pos)    	
		BIND_CONTEXT (CF_MemSamples::mem_sample_count, sample_count)
		BIND_CONTEXT (CF_MemSamples::mem_sample_index, sample_index)    	
		BIND_CONTEXT (CF_Error::error_code, error_code)
    {
    	__init ();
	}
	
	STD_TSOURCE_RESET() { 
#if 0		
		// Kun: rewind memsample source to the front
		// This behavior is for testing purpose only
		__init (); 		
#endif 
	}
	
	STD_TSOURCE_FLUSH() { }
	
    bool Process () {
		bool ret = true;

		if ( remain_sample > 28 ) {
			COMPLEX16* po = opin().append ();
			rep_memcpy<7>((vcs*)po, (vcs*) sample_pointer);
			
			sample_pointer += 28;
            sample_index   += 28;
			remain_sample  -= 28;
		} else if (remain_sample <= 0) {
            ret = false;
		} else {
			COMPLEX16* po = opin().append ();
			for ( uint i=0; i<remain_sample; i++) {
				po[i] = sample_pointer[i];
			}

			sample_pointer += remain_sample;
            sample_index   += remain_sample;
			remain_sample  = 0;
		}
		ret = ret && Next()->Process(opin());
		// break the process
		// if ( error_code != BK_ERROR_SUCCESS )
        //    ret = false;
		return ret;
    }

    FINL int Seek (int offset)
    {
		if ( offset == ISource::START_POS )
        {
			// rewind to the beginning
			sample_pointer = sample_buf;
			sample_index   = 0;
			remain_sample  = sample_count;
            return offset;
		}
        if ( offset == END_POS )
        {
            // forward to the end
            sample_pointer = sample_buf + sample_buf_size;
            sample_index   = sample_count;
            remain_sample  = 0;
            return offset;
        }

        // adjust offset to be integer multiple of 4, to assume data alignment
        offset = ceil_div(offset, 4) * 4;
        if (offset >= 0)
        {
            offset = min(remain_sample, (uint)offset);
        }
        else
        {
            // Note: be careful about the value range of singed and unsigned integer
            assert(offset != INT_MIN);
            offset = -(int)min(sample_index, (uint)-offset);
        }
		sample_pointer += offset;
        sample_index   += offset;
		remain_sample  -= offset;
        return offset;
    }
};

class MemSamplesDesc
{
protected:
    size_t     nstream;
	uint       mem_sample_count;                    // # of samples in buffer
    COMPLEX16* mem_sample_buf[MAX_RADIO_NUMBER];
	uint       mem_sample_index[MAX_RADIO_NUMBER];  // index of sample being processed

    MemSamplesDesc()
        : nstream(0)
        , mem_sample_count(0)
    {
    }

public:
    // Returns true if succeeded
    FINL bool Init ( size_t nstream_, COMPLEX16* buf[], uint bufsize )
	{
        nstream = nstream_;
        mem_sample_count = bufsize;
        for (size_t i = 0; i < nstream; i++)
        {
            assert(buf[i] != NULL);
            // sample_buf must point to the address 16 bytes aligned
            assert((size_t)buf[i] % 16 == 0);

		    mem_sample_buf[i] = buf[i];
		    mem_sample_index[i] = 0;
        }
        return true;
	}
};

DEFINE_LOCAL_CONTEXT(TMemSamples2, CF_MemSamples, CF_Error);
template < TSOURCE_ARGS >
class TMemSamples2 : public TSource<TSOURCE_PARAMS>, public MemSamplesDesc
{
protected:
	CTX_VAR_RW (ulong, 	    error_code );
    CTX_VAR_RW (uint,       sample_index);

public:
    static const size_t NSTREAM = 2;
	DEFINE_OPORT(COMPLEX16, 28, NSTREAM);

public:
	REFERENCE_LOCAL_CONTEXT(TMemSamples2);
	
	// brick interfaces
    STD_TSOURCE_CONSTRUCTOR(TMemSamples2)
		BIND_CONTEXT (CF_Error::error_code, error_code)
		BIND_CONTEXT (CF_MemSamples::mem_sample_index, sample_index)	
    { }
	
	STD_TSOURCE_RESET() { }
	
	STD_TSOURCE_FLUSH() { }
	
    bool Process () {
		bool ret = true;

        uint remain_sample = mem_sample_count - mem_sample_index[0];

        size_t iss;
        for (iss = 0; iss < NSTREAM; iss++)
        {
            COMPLEX16*  sample_pointer = mem_sample_buf[iss] + mem_sample_index[iss];

            if ( remain_sample > 28 ) {
			    rep_memcpy<7>((vcs*)opin().write(iss), (vcs*) sample_pointer);
                mem_sample_index[iss] += 28;
		    } else if (remain_sample == 0) {
                ret = false;
                FlushPort();
                return ret;
		    } else {
			    for ( uint i=0; i<remain_sample; i++) {
                    COMPLEX16 *po = opin().write(iss);
				    po[i] = sample_pointer[i];
			    }

                mem_sample_index[iss] += remain_sample;
		    }
        }
        sample_index = mem_sample_index[0];

        opin().append();
		ret = Next()->Process(opin());
		return ret;
    }

    FINL int Seek (int offset)
    {
        size_t iss;

		if ( offset == ISource::START_POS )
        {
			// rewind to the beginning
            for (iss = 0; iss < NSTREAM; iss++)
            {
			    mem_sample_index[iss] = 0;
                sample_index = 0;
            }
            return offset;
		}
        if ( offset == END_POS )
        {
            // forward to the end
            for (iss = 0; iss < NSTREAM; iss++)
            {
                mem_sample_index[iss] = mem_sample_count;
                sample_index = mem_sample_count;
            }
                
            return offset;
        }

        // adjust offset to be integer multiple of 4, to assume data alignment
        offset = ceil_div(offset, 4) * 4;
        if (offset >= 0)
        {
            uint remain_sample = mem_sample_count - mem_sample_index[0];
            offset = min(remain_sample, (uint)offset);
        }
        else
        {
            // Note: be careful about the value range of singed and unsigned integer
            assert(offset != INT_MIN);
            offset = -(int)min(mem_sample_index[0], (uint)-offset);
        }
        for (iss = 0; iss < NSTREAM; iss++)
        {
            mem_sample_index[0] += offset;
            sample_index += offset;
        }
        return offset;
    }
};

//
// TBufSource -- flexible memeory source 
//
class CF_Buffer {
	FACADE_FIELD(uchar*, buffer_pointer);
	FACADE_FIELD(ulong , buffer_size   );
	FACADE_FIELD(ulong , data_length   );
	FACADE_FIELD(uchar*, read_pointer  );

public:
	CF_Buffer () {
		read_pointer() = buffer_pointer() = NULL;
		buffer_size() = data_length() = 0;
	}
	template<int SIZE>
	void Init ( TSBuf<SIZE>& s) {
		buffer_pointer () = s.GetBuffer ();
		buffer_size()     = s.GetSize   ();
		data_length()     = s.GetDataLength ();
		read_pointer()    = buffer_pointer();
	}

	void Init ( TDBuf & s) {
		buffer_pointer () = s.GetBuffer ();
		buffer_size()     = s.GetSize   ();
		data_length()     = s.GetDataLength ();
		read_pointer()    = buffer_pointer();
	}
};

DEFINE_LOCAL_CONTEXT(TBufSink, CF_Error);
template<typename TData, int Burst>
class TBufSink {
public:
template<TSINK_ARGS>
class Sink: public CF_Buffer, public TSink<TSINK_PARAMS>
{
	CTX_VAR_RW (ulong,      error_code ); 

public:
	DEFINE_IPORT(TData, Burst);
	
public:
    REFERENCE_LOCAL_CONTEXT(TBufSink);

    STD_TSINK_CONSTRUCTOR(Sink)
        BIND_CONTEXT(CF_Error::error_code, error_code)
    {}

	STD_TSINK_RESET()  {}

	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
			int write_size = Burst * sizeof(TData);
            const TData* input = ipin.peek();
			assert ( data_length() + write_size <= buffer_size() );
			memcpy ( buffer_pointer() + data_length(), input, write_size );
			data_length() += write_size;

			ipin.pop();
        }
        return true;
    }
};
};

DEFINE_LOCAL_CONTEXT(TBufSource, CF_Error);
template<typename TData, int Burst>
class TBufSource {
public:
template < TSOURCE_ARGS > 
class Source : public TSource<TSOURCE_PARAMS>, public CF_Buffer
{
protected:
	CTX_VAR_RW (ulong, 	    error_code );

public:
	DEFINE_OPORT(TData, Burst);

protected:
	
public:
	REFERENCE_LOCAL_CONTEXT(TBufSource);
	
	// brick interfaces
    STD_TSOURCE_CONSTRUCTOR(Source)
		BIND_CONTEXT (CF_Error::error_code, error_code)
    {}
	
	STD_TSOURCE_RESET() {}
	STD_TSOURCE_FLUSH() {}
	
    bool Process () {
		bool ret = true;
		if ( buffer_pointer () == NULL ) 
			return false;

		int  read_size = Burst* sizeof(TData);
		while ( read_pointer() + read_size <= buffer_pointer() + data_length() )
		{
			TData* po = opin().append();
			memcpy ( po, read_pointer(), read_size );

			ret = Next()->Process(opin());
			read_pointer() += read_size;
		}

		return false;
    }
};
};
