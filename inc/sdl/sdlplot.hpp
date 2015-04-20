#pragma once

#include <brick.h>
#include <strsafe.h>
#include <debugplotu.h>

DEFINE_LOCAL_CONTEXT(TPlotLine, CF_VOID);
template<size_t N = 1>
class TPlotLine {
public:
template<TSINK_ARGS>
class Sink : public TSink <TSINK_PARAMS>
{
static const uint name_max = 256;
    char name[name_max];

public:
    void SetName ( char* n) {
        size_t len = 0;
        StringCbLength (n, name_max, &len );
        StringCbCopyN ( name, name_max, n, len );
    };
public:
    DEFINE_IPORT(int, N);

public:    
    STD_TSINK_CONSTRUCTOR(Sink ) { 
        StringCbCopyN ( name, name_max, "Line", 4 );
    }
    
	STD_TSINK_RESET() {}
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin) {
        while ( ipin.check_read () ) {
            // plot a line with N points
            int * pi = ipin.peek ();
            PlotLine ( name, pi, N );
            ipin.pop ();
        }    
        return true;
    }
};
};

DEFINE_LOCAL_CONTEXT(TPlotSpec, CF_VOID);
template<size_t N>
class TPlotSpec {
public:
template<TSINK_ARGS>
class Sink : public TSink <TSINK_PARAMS>
{
static const uint name_max = 256;
    char name[name_max];

public:
    void SetName ( char* n) {
        size_t len = 0;
        StringCbLength (n, name_max, &len );
        StringCbCopyN ( name, name_max, n, len );
    };
public:
    DEFINE_IPORT(int, N);

public:    
    STD_TSINK_CONSTRUCTOR(Sink ) { 
        StringCbCopyN ( name, name_max, "Spec", 4 );
    }
    
	STD_TSINK_RESET() {}
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin) {
        while ( ipin.check_read () ) {
            // plot a line with N points
            int * pi = ipin.peek ();
            PlotSpectrum ( name, pi, N );
            ipin.pop ();
        }    
        return true;
    }
};
};

DEFINE_LOCAL_CONTEXT(TPlotDots, CF_VOID);
template<size_t N = 1>
class TPlotDots {
public:
template<TSINK_ARGS>
class Sink : public TSink <TSINK_PARAMS>
{
static const uint name_max = 256;
    char name[name_max];

public:
    void SetName ( char* n) {
        size_t len = 0;
        StringCbLength (n, name_max, &len );
        StringCbCopyN ( name, name_max, n, len );
    };
public:
    DEFINE_IPORT(COMPLEX16, N);

public:    
    STD_TSINK_CONSTRUCTOR(Sink ) { 
        StringCbCopyN ( name, name_max, "Dots", 4 );
    }
    
	STD_TSINK_RESET() {}
	STD_TSINK_FLUSH() {}

    BOOL_FUNC_PROCESS(ipin) {
        while ( ipin.check_read () ) {
            // plot a line with N points
            COMPLEX16 * pi = ipin.peek ();
            PlotDots ( name, pi, N );
            ipin.pop ();
        }    
        return true;
    }
};
};

