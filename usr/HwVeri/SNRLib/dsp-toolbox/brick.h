#pragma once

#include "templtrick.h"
#include "slackq.h"

// Definition of Bricks
// Interface
//	 	Reset : reset all states
//      Flush : flushing its output pin - clear all buffered data
//		Process: operating on the input pin
//

// void base class
class CBrick {}; 
class CVoid {};

template<class T_CTX, class T_IN> 
class TSink : public CBrick {
public:
	FINL bool Reset ( T_CTX & ctx ) {return true; };	
	FINL bool Flush ( T_CTX & ctx ) {return true; };	
	FINL bool Process (T_IN & ipin, T_CTX & ctx ) {return true; };
};

template<class T_CTX, class T_IN, class T_OUT, class T_NEXT> // two PinQueue
class TFilter : public CBrick {
public:
	T_NEXT*  next_;	// point to next brick
	T_OUT    opin_;	// output pin

	TFilter () : next_ (NULL) {};
	TFilter ( T_NEXT* n ) { next_ = n; };

	FINL bool Reset ( T_CTX & ctx ) {return true; };	
	FINL bool Flush ( T_CTX & ctx ) {return true; };	
    FINL bool Process ( T_IN & ipin, T_CTX & ctx ) {return true;};
};

//
// Mux output pin is up to 10
//
#include "mux.hpp"

// TMux2 is obsoleted
//
template<class T_CTX, class T_IN, 
		 class T_OUT,  class T_NEXT,
		 class T_OUT1, class T_NEXT1 > // two PinQueue
class TMux2 : public CBrick {
public:
	T_NEXT*  next_;	// point to next brick
	T_OUT    opin_;		// output pin
	T_NEXT1*  next1_;	// point to next brick
	T_OUT1    opin1_;		// output pin

	TMux2 () : next_ (NULL), next1_(NULL) {};
	TMux2 ( T_NEXT* n, T_NEXT1 * n1 ) { 
		next_ = n;
		next1_ = n1;
	};

	
	FINL bool Reset ( T_CTX & ctx ) {return true; };	
	FINL bool Flush ( T_CTX & ctx ) {return true; };	
    FINL bool Process ( T_IN & ipin, T_CTX & ctx ) {return true;};
};


template<class T_CTX, class T_OUT, class T_NEXT> 
class TSource : public CBrick {
public:
	T_NEXT * next_;
	T_OUT    opin_;
	T_CTX    ctx_;
	
	TSource () : next_ (NULL){};
	TSource ( T_NEXT* n ) { next_ = n; };

	FINL bool Reset ( T_CTX & ctx ) {return true; };	
	FINL bool Flush ( T_CTX & ctx ) {return true; };	
    FINL bool Process () {return true;};
};

template<class T_CTX, int K, class T_OUT, class T_NEXT> 
class TSourceN {
public:
	T_NEXT * next_[K];
	T_OUT    opin_;
	T_CTX    ctx_;
	
	TSourceN () : next_ (NULL){};
	TSourceN ( T_NEXT* n ) { next_[0] = n; };

	FINL bool LinkOutput ( int idx, T_NEXT * p_n ) {
		if ( idx < K ) {
			next_[K] = p_n;
		}
	};

	FINL bool Reset ( T_CTX & ctx ) {return true; };	
	FINL bool Flush ( T_CTX & ctx ) {return true; };	
    FINL bool Process () {return true;};
};

#define SOURCE(xx) class xx : public TSource<T_CTX, T_OUT, T_NEXT> { \
                   public: 
                   
#define INIT_SOURCE(xx)	xx(T_NEXT * n) : TSource<T_CTX, T_OUT, T_NEXT>(n)  


#define FILTER(xx) class xx : public TFilter<T_CTX, T_IN, T_OU, T_NEXT> { \
				   public: 
				   
#define INIT_FILTER(xx)	xx ( T_NEXT * n ) : TFilter<T_CTX, T_IN, T_OU, T_NEXT>(n)

#define SINK(xx)  class xx : public TSink<T_CTX, T_Input > { \
					public: 
#define INIT_SINK(xx) xx () : TSink<T_CTX, T_Input >

#define END_SINK   };	
#define END_SOURCE };	
#define END_FILTER };

#define SINK_TYPE(ctx, ii, stype, ss) typedef stype<ctx, ii> ss
	
#define FILTER_TYPE(ctx, ii, oo, nn, stype, ss) \
	typedef stype<ctx, ii,oo,nn> ss
	
#define MUX2_TYPE(ctx, ii, oo, nn, oo1, nn1, stype, ss) \
	typedef stype<ctx,ii, oo, nn, oo1, nn1> ss

#define MUX_TYPE(stype) stype<
#define CONTEXT(ctx) ctx
#define IN_PIN(ii) ,ii
#define OUT_PIN(oo,nn) , oo, nn
#define DEF_NAME(ss) > ss

	
#define SOURCE_TYPE(ctx, oo, nn, stype, ss) \
	typedef stype<ctx,oo,nn> ss

#define ADD_SINK(stype, ss) stype ss
#define LINK_FILTER(nn, stype, ss) stype ss(&nn)
#define LINK_SOURCE(nn, stype, ss) stype ss(&nn)
#define LINK_MUX2(nn, nn1, stype, ss) stype ss(&nn, &nn1)


