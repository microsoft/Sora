#pragma once
#include "vector128.h"

struct HT_STF
{
  static const int SIZE   = 160 * sizeof(COMPLEX16);
  static const int COUNT  = 160;
  static const int VCOUNT = 160 / 4;
  static const int csd    = 4;// 16samples = 400ns, 40Mhz

  __forceinline void get_stf_1(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_stf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);

    for (int i = 0; i < VCOUNT; i++)
    {
      pvout[i] = pvin[i];
    }
  }

  __forceinline void get_stf_2(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_stf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
        
    int i = 0;
    for (; i < VCOUNT - csd; i++)
    {
      pvout[i + csd] = pvin[i];
    }
    for (int j = 0; i < VCOUNT; i++, j++)
    {
      pvout[j] = pvin[i];
    }
  }

  //////////////////////////////////////////////////////////////////////////
  __forceinline void get_stf_1_of_4(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_stf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);

    for (int i = 0; i < VCOUNT; i++)
    {
      pvout[i] = pvin[i];
    }
  }

  __forceinline void get_stf_2_of_4(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_stf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 4;

    int i = 0;
    for (; i < VCOUNT - _csd; i++)
    {
      pvout[i + csd] = pvin[i];
    }
    for (int j = 0; i < VCOUNT; i++, j++)
    {
      pvout[j] = pvin[i];
    }
  }

  __forceinline void get_stf_3_of_4(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_stf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 2;

    int i = 0;
    for (; i < VCOUNT - _csd; i++)
    {
      pvout[i + csd] = pvin[i];
    }
    for (int j = 0; i < VCOUNT; i++, j++)
    {
      pvout[j] = pvin[i];
    }
  }

  __forceinline void get_stf_4_of_4(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_stf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 6;

    int i = 0;
    for (; i < VCOUNT - _csd; i++)
    {
      pvout[i + csd] = pvin[i];
    }
    for (int j = 0; i < VCOUNT; i++, j++)
    {
      pvout[j] = pvin[i];
    }
  }

private:
  static A16 COMPLEX16 _stf[160];
};

SELECTANY COMPLEX16 HT_STF::_stf[] =
{
    {724, 724},{-743, 983},{-2085, 37},{-1907, -1009},{-212, -1236},{1605, -737},{2247, -199},{1814, -6},{1448, 0},{1814, -6},{2247, -199},{1605, -737},{-212, -1236},{-1907, -1009},{-2085, 37},{-743, 983},
    {724, 724},{983, -743},{37, -2085},{-1009, -1907},{-1236, -212},{-737, 1605},{-199, 2247},{-6, 1814},{0, 1448},{-6, 1814},{-199, 2247},{-737, 1605},{-1236, -212},{-1009, -1907},{37, -2085},{983, -743},
    {724, 724},{-743, 983},{-2085, 37},{-1907, -1009},{-212, -1236},{1605, -737},{2247, -199},{1814, -6},{1448, 0},{1814, -6},{2247, -199},{1605, -737},{-212, -1236},{-1907, -1009},{-2085, 37},{-743, 983},
    {724, 724},{983, -743},{37, -2085},{-1009, -1907},{-1236, -212},{-737, 1605},{-199, 2247},{-6, 1814},{0, 1448},{-6, 1814},{-199, 2247},{-737, 1605},{-1236, -212},{-1009, -1907},{37, -2085},{983, -743},
    {724, 724},{-743, 983},{-2085, 37},{-1907, -1009},{-212, -1236},{1605, -737},{2247, -199},{1814, -6},{1448, 0},{1814, -6},{2247, -199},{1605, -737},{-212, -1236},{-1907, -1009},{-2085, 37},{-743, 983},
    {724, 724},{983, -743},{37, -2085},{-1009, -1907},{-1236, -212},{-737, 1605},{-199, 2247},{-6, 1814},{0, 1448},{-6, 1814},{-199, 2247},{-737, 1605},{-1236, -212},{-1009, -1907},{37, -2085},{983, -743},
    {724, 724},{-743, 983},{-2085, 37},{-1907, -1009},{-212, -1236},{1605, -737},{2247, -199},{1814, -6},{1448, 0},{1814, -6},{2247, -199},{1605, -737},{-212, -1236},{-1907, -1009},{-2085, 37},{-743, 983},
    {724, 724},{983, -743},{37, -2085},{-1009, -1907},{-1236, -212},{-737, 1605},{-199, 2247},{-6, 1814},{0, 1448},{-6, 1814},{-199, 2247},{-737, 1605},{-1236, -212},{-1009, -1907},{37, -2085},{983, -743},
    {724, 724},{-743, 983},{-2085, 37},{-1907, -1009},{-212, -1236},{1605, -737},{2247, -199},{1814, -6},{1448, 0},{1814, -6},{2247, -199},{1605, -737},{-212, -1236},{-1907, -1009},{-2085, 37},{-743, 983},
    {724, 724},{983, -743},{37, -2085},{-1009, -1907},{-1236, -212},{-737, 1605},{-199, 2247},{-6, 1814},{0, 1448},{-6, 1814},{-199, 2247},{-737, 1605},{-1236, -212},{-1009, -1907},{37, -2085},{983, -743},
};
