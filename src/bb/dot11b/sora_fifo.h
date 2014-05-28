/*++
Copyright (c) Microsoft Corporation

Module Name: sora_fifo.h

Abstract: FIFO used in Sora

History: 
          7/7/2009: Modified by senxiang.
--*/
#pragma once
#include <stdio.h>

// Counter-based FIFO
template<class TYPE, size_t SIZE>
class SoraCounterFifo
{
    TYPE data[SIZE];

    // reader and writer
    volatile unsigned int w_count;
    volatile unsigned int r_count;

    // writer only
    TYPE *w_pointer;
    unsigned int w_itCounter;

    // reader only
    TYPE *r_pointer;
    unsigned int r_itCounter;

    // Handle to dump file
    FILE *dump;
public:
    SoraCounterFifo(const char *dump_file = NULL) throw()
        : w_count(0), r_count(0), w_itCounter(0), r_itCounter(0)
        , w_pointer(data), r_pointer(data)
    {
#if defined(USER_MODE) && defined(BBB_VALIDATION)
        dump = dump_file ? fopen(dump_file, "w") : NULL;
#else
        UNREFERENCED_PARAMETER(dump_file);
        dump = NULL;
#endif
    }

    ~SoraCounterFifo()
    {
#if defined(USER_MODE) && defined(BBB_VALIDATION)
        if(dump) fclose(dump);
#endif
    }

    bool RCheck(size_t n)
    {
        return w_count - r_count >= n;
    }

    TYPE *RNextPt(size_t n)
    {
        TYPE *ret = r_pointer;
        r_itCounter += n;
        r_pointer += n;
        if (r_pointer >= data + SIZE)
        {
            r_pointer = data + (r_pointer - data) % SIZE;
        }
        return ret;
    }

    void RFlush()
    {
        r_count += r_itCounter;
        r_itCounter = 0;
    }

    TYPE Read()
    {
        ASSERT(r_itCounter == 0);
        TYPE ret = *r_pointer;
        r_count++;
        r_pointer++;
        if (r_pointer == data + SIZE)
        {
            r_pointer = data;
        }
        return ret;
    }

    bool WCheck(size_t n)
    {
        return SIZE - (w_count - r_count) >= n;
    }

    TYPE *WNextPt(size_t n)
    {
        TYPE *ret = w_pointer;
        w_itCounter += n;
        w_pointer += n;
        if (w_pointer >= data + SIZE)
        {
            w_pointer = data + (w_pointer - data) % SIZE;
        }
        return ret;
    }

    void WFlush()
    {
        w_count += w_itCounter;
        w_itCounter = 0;
    }

    void Write(const TYPE& p)
    {
        Dump(p);
        ASSERT(w_itCounter == 0);
        *w_pointer = p;
        w_count++;
        w_pointer++;
        if (w_pointer == data + SIZE)
        {
            w_pointer = data;
        }
    }

    void Clear()
    {
#if defined(USER_MODE) && defined(BBB_VALIDATION)
        if (dump) fprintf(dump, "\n");
#endif
        r_pointer = w_pointer = data;
        w_count = r_count = w_itCounter = r_itCounter = 0;
    }

private:
    template<class T> void Dump(const T&) { /* Dummy default func */ }
#if defined(USER_MODE) && defined(BBB_VALIDATION)
    void Dump(const unsigned char& p) { if (dump) fprintf(dump, "%2x ", p); }
    // Add more overloads here if required
#endif
};

// Linear FIFO, ie. all items in the FIFO are stored in sequential memory
template<class TYPE, size_t SIZE, size_t FIELD_SIZE = SIZE * 4096>
class SoraLinearFifo
{
    TYPE data[FIELD_SIZE];

    // reader and writer
    unsigned int w_count;
    unsigned int r_count;

    // writer only
    TYPE *w_pointer;
    unsigned int w_itCounter;

    // reader only
    TYPE *r_pointer;
    unsigned int r_itCounter;

    // Handle to dump file
    FILE *dump;
public:
    SoraLinearFifo(const char *dump_file = NULL) throw()
        : w_count(0), r_count(0), w_itCounter(0), r_itCounter(0)
        , w_pointer(data), r_pointer(data)
    {
#if defined(USER_MODE) && defined(BBB_VALIDATION)
        dump = dump_file ? fopen(dump_file, "w") : NULL;
#else
        UNREFERENCED_PARAMETER(dump_file);
        dump = NULL;
#endif
    }

    ~SoraLinearFifo()
    {
#if defined(USER_MODE) && defined(BBB_VALIDATION)
        if(dump) fclose(dump);
#endif
    }

    bool RCheck(size_t n)
    {
        return w_count - r_count >= n;
    }

    TYPE *RNextPt(size_t n)
    {
        TYPE *ret = r_pointer + r_itCounter;
        r_itCounter += n;
        return ret;
    }

    void RFlush()
    {
        r_count += r_itCounter;
        r_pointer += r_itCounter;
        r_itCounter = 0;
        if (r_pointer > data + FIELD_SIZE / 2)
        {
            memcpy(r_pointer - FIELD_SIZE / 2, r_pointer, (char *)w_pointer - (char *)r_pointer);
            r_pointer -= FIELD_SIZE / 2;
            w_pointer -= FIELD_SIZE / 2;
        }
    }

    bool WCheck(size_t n)
    {
        return SIZE - (w_count - r_count) >= n;
    }

    TYPE *WNextPt(size_t n)
    {
        TYPE *ret = w_pointer + w_itCounter;
        w_itCounter += n;
        return ret;
    }

    void WFlush()
    {
        TYPE *p;
        for (p = w_pointer; p < w_pointer + w_itCounter; p++) Dump(*p);

        w_count += w_itCounter;
        w_pointer += w_itCounter;
        w_itCounter = 0;
    }

    void WEnd()
    {
        if (w_itCounter > 0) { WFlush(b); }
    }

    void Clear()
    {
#if defined(USER_MODE) && defined(BBB_VALIDATION)
        if (dump) fprintf(dump, "\n");
#endif
        r_pointer = w_pointer = data;
        w_count = r_count = w_itCounter = r_itCounter = 0;
    }

private:
    template<class T> void Dump(const T&) { /* Dummy default func */ }
#if defined(USER_MODE) && defined(BBB_VALIDATION)
    void Dump(COMPLEX16& p) { fprintf(dump, "(%d,%d) ", p.re, p.im); }
    // Add more overloads here if required
#endif
};
