#pragma once

#include "dspcomm.h"

//
// Light-weight input/output queue
//

template<class T, size_t N, size_t M, size_t SIZE> // input N, output M, buffer size SIZE
class CPinQueue {
public:
static const size_t rsize = M;
static const size_t wsize = N;
typedef T DataType; 

	T buf_ [SIZE];
    unsigned int w_cnt;
	unsigned int r_cnt;

	CPinQueue () {
		// an idea to put checks here
		STATIC_ASSERT (((SIZE >= lcm<M,N>::value)));
		w_cnt = r_cnt = 0;
	};
	
	FINL bool check_read () {
		//printf ( "queue(%d,%d).check == w %d, r %d\n", N, M, w_cnt, r_cnt );
		return (w_cnt - r_cnt >= M);
	}

	FINL void Pop () {
		r_cnt += M;

		if (r_cnt == w_cnt) {
			r_cnt = w_cnt = 0;
		} 

		/*
		else if ( r_cnt > SIZE / 2 ) {
			memcpy ( buf_ + r_cnt - SIZE /2, buf_ + r_cnt, (w_cnt - r_cnt )*sizeof(T));
			w_cnt -= SIZE / 2;
			r_cnt -= SIZE / 2;
		}
		*/
	}

	FINL T * Peek () {
		return (buf_ + r_cnt);
	}

	FINL T * Push () {
		T * ret = buf_ + w_cnt;
		w_cnt += N;
   		//printf ( "queue(%d,%d).push == w %d, r %d\n", N, M, w_cnt, r_cnt );
		return ret;
	}

	FINL void Clear () {
		w_cnt = r_cnt = 0;
	}
};

// specialize the 1:1
template<class T> // input N, output M, buffer size SIZE
class CPinQueue <T,1,1,1> {
public:
	static const size_t rsize = 1;
	static const size_t wsize = 1;

	T buf_;
	
	FINL void Pop () {
	}

	FINL T * Peek () {
		return (&buf_);
	}

	FINL T * Push () {
		return (&buf_);
	}

	FINL void Clear () {
	}
	
	FINL T & PopHead () {
		return buf_;
	}

	FINL T & PushHead () {
		return buf_;
	}

};

