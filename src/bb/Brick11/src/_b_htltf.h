#pragma once
#include "vector128.h"

struct HT_LTF
{
  static const int SIZE   = 160 * sizeof(COMPLEX16);
  static const int COUNT  = 160;
  static const int VCOUNT = 160 / 4;
  static const int csd    = 4;// 16samples = 400ns, 40Mhz

  //////////////////////////////////////////////////////////////////////////

  __forceinline void get_as_lltf(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);

    int i = 8; // start from data part
    int j = 16; // long cp, 16vcs=64COMPLEX16
    for (; i < VCOUNT; i++, j++)
    {
      pvout[j] = pvin[i];
    }
    // repeat
    for (i = 8; i < VCOUNT; i++, j++)
    {
      pvout[j] = pvin[i];
    }

    i = VCOUNT - 16;
    j = 0;
    for (; i < VCOUNT; i++, j++)
    {
      pvout[j] = pvin[i];
    }
  }
  // 1 -1 1 1, csd = 0ns, 0 vcs
  __forceinline void get_ltf_11(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);

    for (int i = 0; i < VCOUNT; i++)
    {
      pvout[i] = pvin[i];
    }
  }

  __forceinline void get_ltf_12(COMPLEX16* pout)
  {
    vcs msk;
    set_all_bits(msk);
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    
    for (int i = 0; i < VCOUNT; i++)
    {
      vcs temp;
      temp = xor(msk, pvin[i]);
      pvout[i] = sub(temp, msk);
    }
  }

  __forceinline void get_ltf_13(COMPLEX16* pout)
  {
    get_ltf_11(pout);
  }

  __forceinline void get_ltf_14(COMPLEX16* pout)
  {
    get_ltf_11(pout);
  }
  //////////////////////////////////////////////////////////////////////////
  // 1 1 -1 1, csd = 400ns, 4 vcs
  __forceinline void get_ltf_21(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 4;
    int i = 0;
    for (; i < 32; i++)
    {
      pvout[i + 8] = pvin[i + 8 - _csd];
    }

    for (i = 0; i < 8; i++)
    {
      pvout[i] = pvout[VCOUNT - 8 + i];
    }
  }
  __forceinline void get_ltf_22(COMPLEX16* pout)
  {
    get_ltf_21(pout);
  }
  __forceinline void get_ltf_23(COMPLEX16* pout)
  {
    vcs msk;
    set_all_bits(msk);
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 4;
    vcs temp;

    int i = 0;
    for (; i < 32; i++)
    {
      temp = xor(msk, pvin[i + 8 - _csd]);
      pvout[i + 8] = sub(temp, msk);
    }

    for (i = 0; i < 8; i++)
    {
      pvout[i] = pvout[VCOUNT - 8 + i];
    }
  }
  __forceinline void get_ltf_24(COMPLEX16* pout)
  {
    get_ltf_21(pout);
  }
  //////////////////////////////////////////////////////////////////////////
  // 1 1 1 -1, csd = 200ns, 2 vcs
  __forceinline void get_ltf_31(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 2;
    int i = 0;
    for (; i < 32; i++)
    {
      pvout[i + 8] = pvin[i + 8 - _csd];
    }

    for (i = 0; i < 8; i++)
    {
      pvout[i] = pvout[VCOUNT - 8 + i];
    }
  }
  __forceinline void get_ltf_32(COMPLEX16* pout)
  {
    get_ltf_31(pout);
  }
  __forceinline void get_ltf_33(COMPLEX16* pout)
  {
    get_ltf_31(pout);
  }
  __forceinline void get_ltf_34(COMPLEX16* pout)
  {
    vcs msk;
    set_all_bits(msk);
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 2;
    vcs temp;

    int i = 0;
    for (; i < 32; i++)
    {
      temp = xor(msk, pvin[i + 8 - _csd]);
      pvout[i + 8] = sub(temp, msk);
    }

    for (i = 0; i < 8; i++)
    {
      pvout[i] = pvout[VCOUNT - 8 + i];
    }    
  }
  //////////////////////////////////////////////////////////////////////////
  // -1 1 1 1, csd = 600ns, 6 vcs
  __forceinline void get_ltf_41(COMPLEX16* pout)
  {
    vcs msk;
    set_all_bits(msk);
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 6;
    vcs temp;

    int i = 0;
    for (; i < 32; i++)
    {
      temp = xor(msk, pvin[i + 8 - _csd]);
      pvout[i + 8] = sub(temp, msk);
    }

    for (i = 0; i < 8; i++)
    {
      pvout[i] = pvout[VCOUNT - 8 + i];
    }    
  }
  __forceinline void get_ltf_42(COMPLEX16* pout)
  {
    vcs *pvin  = reinterpret_cast<vcs*>(_ltf);
    vcs *pvout = reinterpret_cast<vcs*>(pout);
    static const int _csd = 6;
    int i = 0;
    for (; i < 32; i++)
    {
      pvout[i + 8] = pvin[i + 8 - _csd];
    }

    for (i = 0; i < 8; i++)
    {
      pvout[i] = pvout[VCOUNT - 8 + i];
    }
  }
  __forceinline void get_ltf_43(COMPLEX16* pout)
  {
    get_ltf_42(pout);
  }
  __forceinline void get_ltf_44(COMPLEX16* pout)
  {
    get_ltf_42(pout);
  }

private:
  static A16 COMPLEX16 _ltf[160];
};

SELECTANY COMPLEX16 HT_LTF::_ltf[] =
{
    {948, 474},{1741, 735},{1809, 299},{761, -1033},{-341, -2365},{-222, -2147},{890, -165},{1416, 1677},{371, 1543},{-1358, 164},{-2075, -86},{-1265, 1422},{15, 2545},{667, 1422},{809, -697},{1125, -1064},
    {1480, 728},{972, 2226},{-581, 1654},{-1936, 309},{-1746, 409},{-268, 1746},{907, 2069},{842, 470},{320, -1335},{578, -1490},{1469, -346},{1659, 582},{603, 957},{-472, 1536},{-78, 2230},{1488, 1834},
    {2370, 0},{1488, -1834},{-78, -2230},{-472, -1536},{603, -957},{1659, -582},{1469, 346},{578, 1490},{320, 1335},{842, -470},{907, -2069},{-268, -1746},{-1746, -409},{-1936, -309},{-581, -1654},{972, -2226},
    {1480, -728},{1125, 1064},{809, 697},{667, -1422},{15, -2545},{-1265, -1422},{-2075, 86},{-1358, -164},{371, -1543},{1416, -1677},{890, 165},{-222, 2147},{-341, 2365},{761, 1033},{1809, -299},{1741, -735},
    {948, -474},{494, 127},{560, 892},{277, 1404},{-868, 1195},{-2058, 579},{-1991, 505},{-476, 1210},{1247, 1694},{1817, 1108},{1055, 143},{-146, 154},{-915, 1103},{-1064, 1371},{-856, -59},{-585, -2049},
    {-532, -2624},{-977, -1400},{-1849, 68},{-2444, 291},{-1931, -554},{-345, -1224},{1139, -986},{1247, -119},{-43, 780},{-1383, 1405},{-1394, 1713},{-36, 1751},{1391, 1665},{1531, 1595},{186, 1438},{-1577, 915},
    {-2370, 0},{-1577, -915},{186, -1438},{1531, -1595},{1391, -1665},{-36, -1751},{-1394, -1713},{-1383, -1405},{-43, -780},{1247, 119},{1139, 986},{-345, 1224},{-1931, 554},{-2444, -291},{-1849, -68},{-977, 1400},
    {-532, 2624},{-585, 2049},{-856, 59},{-1064, -1371},{-915, -1103},{-146, -154},{1055, -143},{1817, -1108},{1247, -1694},{-476, -1210},{-1991, -505},{-2058, -579},{-868, -1195},{277, -1404},{560, -892},{494, -127},
    {948, 474},{1741, 735},{1809, 299},{761, -1033},{-341, -2365},{-222, -2147},{890, -165},{1416, 1677},{371, 1543},{-1358, 164},{-2075, -86},{-1265, 1422},{15, 2545},{667, 1422},{809, -697},{1125, -1064},
    {1480, 728},{972, 2226},{-581, 1654},{-1936, 309},{-1746, 409},{-268, 1746},{907, 2069},{842, 470},{320, -1335},{578, -1490},{1469, -346},{1659, 582},{603, 957},{-472, 1536},{-78, 2230},{1488, 1834},
};
