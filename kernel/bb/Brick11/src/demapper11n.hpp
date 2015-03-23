#pragma once

#include "brick.h"
#include "dsp_demap.h"

DEFINE_LOCAL_CONTEXT(T11nSigDemap, CF_VOID);
template<TFILTER_ARGS>
class T11nSigDemap : public TFilter<TFILTER_PARAMS>
{
public:
    static const int low = -26;
    static const int high = 26;
    static const int NbPS = high - low - 4;
    DEFINE_IPORT(COMPLEX16, 64 * 3); // 3 symbols
    DEFINE_OPORT(unsigned __int8,   NbPS * 3); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nSigDemap);

    STD_TFILTER_CONSTRUCTOR(T11nSigDemap)
    { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16* cip = ipin.peek();
            const vcs *ip = (const vcs *)cip;
            unsigned __int8 *op = opin().append();

            int i;
            int j = 0;

            demapper.demap_limit(ip, (vcs *)limited, 16);
            for (i = 64 + low; i < 64; i++)
            {
                if (i == 64 - 21 || i == 64 - 7)
                    continue;
                demapper.demap_bpsk_i(limited[i], &op[j++]);
            }
            for (i = 1; i <= high; i++)
            {
                if (i == 7 || i == 21)
                    continue;
                demapper.demap_bpsk_i(limited[i], &op[j++]);
            }

            ip += 16;
            demapper.demap_limit(ip, (vcs *)limited, 16);
            for (i = 64 + low; i < 64; i++)
            {
                if (i == 64 - 21 || i == 64 - 7)
                    continue;
                demapper.demap_bpsk_q(limited[i], &op[j++]);
            }
            for (i = 1; i <= high; i++)
            {
                if (i == 7 || i == 21)
                    continue;
                demapper.demap_bpsk_q(limited[i], &op[j++]);
            }

            ip += 16;
            demapper.demap_limit(ip, (vcs *)limited, 16);
            for (i = 64 + low; i < 64; i++)
            {
                if (i == 64 - 21 || i == 64 - 7)
                    continue;
                demapper.demap_bpsk_q(limited[i], &op[j++]);
            }
            for (i = 1; i <= high; i++)
            {
                if (i == 7 || i == 21)
                    continue;
                demapper.demap_bpsk_q(limited[i], &op[j++]);
            }

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    dsp_demapper demapper;
    A16 COMPLEX16 limited[64];
};

DEFINE_LOCAL_CONTEXT(T11nDemapBPSK, CF_VOID);
template<TFILTER_ARGS>
class T11nDemapBPSK : public TFilter<TFILTER_PARAMS>
{
public:
    static const int low = -28;
    static const int high = 28;
    static const int NbPS = high - low - 4;
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(unsigned __int8,   NbPS); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDemapBPSK);
    STD_TFILTER_CONSTRUCTOR(T11nDemapBPSK) { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16* cip = ipin.peek();
            const vcs *ip = (const vcs *)cip;
            unsigned __int8 *op = opin().append();

            demapper.demap_limit(ip, (vcs *)limited, 16);

            int i;
            int j = 0;

            for (i = 64 + low; i < 64; i++)
            {
                if (i == 64 - 21 || i == 64 - 7)
                    continue;
                demapper.demap_bpsk_i(limited[i], &op[j++]);
            }

            for (i = 1; i <= high; i++)
            {
                if (i == 7 || i == 21)
                    continue;

                demapper.demap_bpsk_i(limited[i], &op[j++]);
            }

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    dsp_demapper demapper;
    A16 COMPLEX16 limited[64];
};

DEFINE_LOCAL_CONTEXT(T11nDemapQPSK, CF_VOID);
template<TFILTER_ARGS>
class T11nDemapQPSK : public TFilter<TFILTER_PARAMS>
{
public:
    static const int low = -28;
    static const int high = 28;
    static const int NbPS = (high - low - 4) * 2;
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(unsigned __int8,   NbPS); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDemapQPSK);
    STD_TFILTER_CONSTRUCTOR(T11nDemapQPSK) { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16* cip = ipin.peek();
            vcs *ip = (vcs *)cip;
            unsigned __int8 *op = opin().append();

            demapper.demap_limit(ip, (vcs *)limited, 16);

            int i;
            int j = 0;

            for (i = 64 + low; i < 64; i++)
            {
                if (i == 64 - 21 || i == 64 - 7)
                    continue;
                demapper.demap_qpsk(limited[i], &op[j]);
                j += 2;
            }

            for (i = 1; i <= high; i++)
            {
                if (i == 7 || i == 21)
                    continue;

                demapper.demap_qpsk(limited[i], &op[j]);
                j += 2;
            }

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    dsp_demapper demapper;
    A16 COMPLEX16 limited[64];
};

DEFINE_LOCAL_CONTEXT(T11nDemapQAM16, CF_VOID);
template<TFILTER_ARGS>
class T11nDemapQAM16 : public TFilter<TFILTER_PARAMS>
{
public:
    static const int low = -28;
    static const int high = 28;
    static const int NbPS = (high - low - 4) * 4;
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(unsigned __int8,   NbPS); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDemapQAM16);
    STD_TFILTER_CONSTRUCTOR(T11nDemapQAM16) { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16* cip = ipin.peek();
            vcs *ip = (vcs *)cip;
            unsigned __int8 *op = opin().append();

            demapper.demap_limit(ip, (vcs *)limited, 16);

            int i;
            int j = 0;

            for (i = 64 + low; i < 64; i++)
            {
                if (i == 64 - 21 || i == 64 - 7)
                    continue;
                demapper.demap_16qam(limited[i], &op[j]);
                j += 4;
            }

            for (i = 1; i <= high; i++)
            {
                if (i == 7 || i == 21)
                    continue;

                demapper.demap_16qam(limited[i], &op[j]);
                j += 4;
            }

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    dsp_demapper demapper;
    A16 COMPLEX16 limited[64];
};

DEFINE_LOCAL_CONTEXT(T11nDemapQAM64, CF_VOID);
template<TFILTER_ARGS>
class T11nDemapQAM64 : public TFilter<TFILTER_PARAMS>
{
public:
    static const int low = -28;
    static const int high = 28;
    static const int NbPS = (high - low - 4) * 6;
    DEFINE_IPORT(COMPLEX16, 64);
    DEFINE_OPORT(unsigned __int8,   NbPS); // each bit is one 8-bit soft-value

public:
    REFERENCE_LOCAL_CONTEXT(T11nDemapQAM64);
    STD_TFILTER_CONSTRUCTOR(T11nDemapQAM64) { }

    BOOL_FUNC_PROCESS(ipin)
    {
        while (ipin.check_read())
        {
            const COMPLEX16* cip = ipin.peek();
            vcs *ip = (vcs *)cip;
            unsigned __int8 *op = opin().append();

            demapper.demap_limit(ip, (vcs *)limited, 16);

            int i;
            int j = 0;

            for (i = 64 + low; i < 64; i++)
            {
                if (i == 64 - 21 || i == 64 - 7)
                    continue;
                demapper.demap_64qam(limited[i], &op[j]);
                j += 6;
            }

            for (i = 1; i <= high; i++)
            {
                if (i == 7 || i == 21)
                    continue;

                demapper.demap_64qam(limited[i], &op[j]);
                j += 6;
            }

            ipin.pop();
            Next()->Process(opin());
        }
        return true;
    }

private:
    dsp_demapper demapper;
    A16 COMPLEX16 limited[64];
};
