#include "common.h"

#include <stdarg.h>

#define NO_LOG_FILE_FLAG 1

static FILE * flog = (NO_LOG_FILE_FLAG)?NULL:fopen("sdromalog.txt", "w");
static FILE * fsta = (NO_LOG_FILE_FLAG)?NULL:fopen("conslog.txt",   "w");

static const int LOG_MAXLEN = 4096;
static const int LOG_MAXCOUNT = 20;
static char logLines[LOG_MAXCOUNT][LOG_MAXLEN];
static int logLengths[LOG_MAXCOUNT] = { 0 };
static int currentLine = 0;

void fdprintf ( int id, const char * format, ...) {
	/*
    FILE *ff;
    switch ( id ) {
    case 1:
        ff = fsta;
        break;
    default:
        ff = flog;
        
    }
    va_list arg;
    va_start(arg, format);
    vsnprintf_s(logLines[currentLine], LOG_MAXLEN, LOG_MAXLEN, format, arg);
    va_end(arg);

    int len = strlen(logLines[currentLine]);
    logLengths[currentLine] = len;

    if (flog)
    {
        fwrite(logLines[currentLine], sizeof(char), len, ff);
        fflush(ff);
    }
    
    currentLine++;
    currentLine %= LOG_MAXCOUNT;
	*/
}

void
dprintf(const char * format, ...)
{
	/*
    va_list arg;
    va_start(arg, format);
    vsnprintf_s(logLines[currentLine], LOG_MAXLEN, LOG_MAXLEN, format, arg);
    va_end(arg);

    int len = strlen(logLines[currentLine]);
    logLengths[currentLine] = len;

    if (flog)
    {
        fwrite(logLines[currentLine], sizeof(char), len, flog);
       // fprintf(flog, "\n");
        fflush(flog);
    }
    
    currentLine++;
    currentLine %= LOG_MAXCOUNT;
	*/
}

void
pntDebugLog(HDC hdc)
{
    const int PER_LINE_HEIGHT = 16;
    const int X = 0;
    // const int Y = 100;
    const int Y = getWinHeight() - PER_LINE_HEIGHT * (LOG_MAXCOUNT + 2);
    // TODO: why this??

    // pntText(hdc, 0, 18, "hello");
  /*  int index = currentLine;
    for (int i = 0; i < LOG_MAXCOUNT; i++)
    {
        if (logLengths[index])
            pntText(hdc, X, Y + i * PER_LINE_HEIGHT, 
                    logLines[index], logLengths[index]);
        
        index++;
        index %= LOG_MAXCOUNT;
    }
    */
}
