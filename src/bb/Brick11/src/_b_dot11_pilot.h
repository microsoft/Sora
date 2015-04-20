#pragma once

struct dot11_ofdm_pilot
{
  static const int pilot_size = 127;

  __forceinline void operator()(int iss, int n, __int16 pilots[4])
  {
    int pilot_sign_idx = (n + 3) % 127;

    int pilot_idx = n & 0x3;

    pilots[0] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][0]);
    pilots[1] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][1]);
    pilots[2] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][2]);
    pilots[3] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][3]);
  }

  __forceinline __int16 operator()(int n)
  {
    return _pilot_sign[n];
  }

private:
  static __int16 _pilot_sign[pilot_size];
  static __int16 _pilot[4][2][4];
};

__declspec(selectany) __int16 dot11_ofdm_pilot::_pilot[4][2][4]=
{
  { {1, 1, -1, -1}, {1, -1, -1, 1} },
  { {1, -1, -1, 1}, {-1, -1, 1, 1} },
  { {-1, -1, 1, 1}, {-1, 1, 1, -1} },
  { {-1, 1, 1, -1}, {1, 1, -1, -1} },
};

__declspec(selectany) __int16 dot11_ofdm_pilot::_pilot_sign[pilot_size] =
{
  1,  1,  1, -1, -1, -1,  1, -1,         -1, -1, -1,  1,  1, -1,  1, -1,         -1,  1,  1, -1,  1,  1, -1,  1,         1,  1,  1,  1,  1, -1,  1,  1, 
  1, -1,  1,  1, -1, -1,  1,  1,         1, -1,  1, -1, -1, -1,  1, -1,         1, -1, -1,  1, -1, -1,  1,  1,         1,  1,  1, -1, -1,  1,  1, -1, 
  -1,  1, -1,  1, -1,  1,  1, -1,         -1, -1,  1,  1, -1, -1, -1, -1,         1, -1, -1,  1, -1,  1,  1,  1,         1, -1,  1, -1,  1, -1,  1, -1, 
  -1, -1, -1, -1,  1, -1,  1,  1,         -1,  1, -1,  1,  1,  1, -1, -1,         1, -1, -1, -1,  1,  1,  1, -1,         -1, -1, -1, -1, -1, -1,  1, 
};

//////////////////////////////////////////////////////////////////////////
struct dot11n_4x4_pilot
{
  static const int pilot_size = 127;

  __forceinline void operator()(int iss, int n, __int16 pilots[4])
  {
    int pilot_sign_idx = (n + 3) % 127;

    int pilot_idx = n & 0x3;

    pilots[0] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][0]);
    pilots[1] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][1]);
    pilots[2] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][2]);
    pilots[3] = (_pilot_sign[pilot_sign_idx] * _pilot[pilot_idx][iss][3]);
  }

  __forceinline __int16 operator()(int n)
  {
    return _pilot_sign[n];
  }

private:
  static __int16 _pilot_sign[pilot_size];
  static __int16 _pilot[4][4][4];
};

__declspec(selectany) __int16 dot11n_4x4_pilot::_pilot[4][4][4]=
{
  { {1, 1, 1, -1}, {1, 1, -1, 1}, {1, -1, 1, 1}, {-1, 1, 1, 1} },
  { {1, 1, -1, 1}, {1, -1, 1, 1}, {-1, 1, 1, 1}, {1, 1, 1, -1} },
  { {1, -1, 1, 1}, {-1, 1, 1, 1}, {1, 1, 1, -1}, {1, 1, -1, 1} },
  { {-1, 1, 1, 1}, {1, 1, 1, -1}, {1, 1, -1, 1}, {1, -1, 1, 1} }
};

__declspec(selectany) __int16 dot11n_4x4_pilot::_pilot_sign[pilot_size] =
{
  1,  1,  1, -1, -1, -1,  1, -1,         -1, -1, -1,  1,  1, -1,  1, -1,         -1,  1,  1, -1,  1,  1, -1,  1,         1,  1,  1,  1,  1, -1,  1,  1, 
  1, -1,  1,  1, -1, -1,  1,  1,         1, -1,  1, -1, -1, -1,  1, -1,         1, -1, -1,  1, -1, -1,  1,  1,         1,  1,  1, -1, -1,  1,  1, -1, 
  -1,  1, -1,  1, -1,  1,  1, -1,         -1, -1,  1,  1, -1, -1, -1, -1,         1, -1, -1,  1, -1,  1,  1,  1,         1, -1,  1, -1,  1, -1,  1, -1, 
  -1, -1, -1, -1,  1, -1,  1,  1,         -1,  1, -1,  1,  1,  1, -1, -1,         1, -1, -1, -1,  1,  1,  1, -1,         -1, -1, -1, -1, -1, -1,  1, 
};