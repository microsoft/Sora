
struct stream_paser_bpsk_2ss
{
  static const int NINPUT  = 13;
  static const int NOUTPUT = 7;

  stream_paser_bpsk_2ss()
  {
    create_lookuptable();
  }

  __forceinline void operator()(const unsigned __int8 pInput[13], unsigned __int8 pOutput1[], unsigned __int8 pOutput2[])
  {
    int i = 0, j = 0;

    for (; i < 12; i += 2)
    {
      unsigned __int8 b1 = pInput[i];
      unsigned __int8 b2 = pInput[i + 1];
      pOutput1[j] = lookuptable[b1][b2][0];
      pOutput2[j] = lookuptable[b1][b2][1];
      ++j;
    }
    pOutput1[j] = lookuptable2[pInput[i]][0];
    pOutput2[j] = lookuptable2[pInput[i]][1];
  }

private:
  static const int lut_size_x = 256;
  static const int lut_size_y = 256;
  unsigned __int8 lookuptable[lut_size_x][lut_size_y][2];
  unsigned __int8 lookuptable2[lut_size_x][2];// lookup table for half byte

    void split(unsigned __int8 &b, unsigned __int8 &lb, unsigned __int8 &hb)
    {
        lb  = b & 0x01;
        lb |= ((b & 0x4) >> 1);
        lb |= ((b & 0x10) >> 2);
        lb |= ((b & 0x40) >> 3);

        hb  = ((b & 0x02) >> 1);
        hb |= ((b & 0x8) >> 2);
        hb |= ((b & 0x20) >> 3);
        hb |= ((b & 0x80) >> 4);

        //printf("%02X : h = %02X, l = %02X\n", b, hb, lb);
    };

  void create_lookuptable()
  {
    // generate the lookuptable for BPSK, 2 spatial stream
    for ( unsigned __int16 i = 0; i <= 255; i++)
    {
      for ( unsigned __int16 j = 0; j <= 255; j++)
      {
        unsigned __int8 b1 = static_cast<unsigned __int8>(i & 0xFF);
        unsigned __int8 b2 = static_cast<unsigned __int8>(j & 0xFF);
        unsigned __int8 lb1 = 0, lb2 = 0, hb1 = 0, hb2 = 0;

        split(b1, lb1, hb1);
        split(b2, lb2, hb2);

        lookuptable2[b1][0] = lb1;
        lookuptable2[b1][1] = hb1;

        unsigned __int8 vb1 = ((lb2 << 4) | lb1);
        unsigned __int8 vb2 = ((hb2 << 4) | hb1);

        lookuptable[b1][b2][0] = vb1;
        lookuptable[b1][b2][1] = vb2;
      }
    }
  }
};

struct stream_paser_qpsk_2ss
{
  static const int NINPUT  = 26;
  static const int NOUTPUT = 13;

  stream_paser_qpsk_2ss()
  {
    create_lookuptable();
  }

  __forceinline void operator()(const unsigned __int8 pInput[26], unsigned __int8 pOutput1[], unsigned __int8 pOutput2[])
  {
    int i = 0, j = 0;

    for (; i < 26; i += 2)
    {
      pOutput1[j] = lookuptable[pInput[i]][pInput[i + 1]][0];
      pOutput2[j] = lookuptable[pInput[i]][pInput[i + 1]][1];
      ++j;
    }
  }

private:
  static const int lut_size_x = 256;
  static const int lut_size_y = 256;
  unsigned __int8 lookuptable[lut_size_x][lut_size_y][2];

    void split(unsigned __int8 &b, unsigned __int8 &lb, unsigned __int8 &hb)
    {
      lb = b & 0x01;
      lb |= ((b & 0x4) >> 1);
      lb |= ((b & 0x10) >> 2);
      lb |= ((b & 0x40) >> 3);

      hb = ((b & 0x02) >> 1);
      hb |= ((b & 0x8) >> 2);
      hb |= ((b & 0x20) >> 3);
      hb |= ((b & 0x80) >> 4);
    };

  void create_lookuptable()
  {
    // generate the lookuptable for QPSK, 2 spatial stream
    for ( unsigned __int16 i = 0; i <= 255; i++)
    {
      for ( unsigned __int16 j = 0; j <= 255; j++)
      {
        unsigned __int8 b1 = static_cast<unsigned __int8>(i & 0xFF);
        unsigned __int8 b2 = static_cast<unsigned __int8>(j & 0xFF);
        unsigned __int8 lb1 = 0, lb2 = 0, hb1 = 0, hb2 = 0;

        split(b1, lb1, hb1);
        split(b2, lb2, hb2);

        unsigned __int8 vb1 = ((lb2 << 4) | lb1);
        unsigned __int8 vb2 = ((hb2 << 4) | hb1);

        lookuptable[b1][b2][0] = vb1;
        lookuptable[b1][b2][1] = vb2;
      }
    }
  }
};

struct stream_paser_16qam_2ss
{
  static const int NINPUT  = 52;
  static const int NOUTPUT = 26;

  stream_paser_16qam_2ss()
  {
    create_lookuptable();
  }

  __forceinline void operator()(const unsigned __int8 pInput[52], unsigned __int8 pOutput1[], unsigned __int8 pOutput2[])
  {
    int i = 0, j = 0;

    for (; i < 52; i += 2)
    {
      pOutput1[j] = lookuptable[pInput[i]][pInput[i + 1]][0];
      pOutput2[j] = lookuptable[pInput[i]][pInput[i + 1]][1];
      ++j;
    }
  }

private:
  static const int lut_size_x = 256;
  static const int lut_size_y = 256;
  unsigned __int8 lookuptable[lut_size_x][lut_size_y][2];

    void split(unsigned __int8 &b, unsigned __int8 &lb, unsigned __int8 &hb)
    {
      lb  = b & 0x03;
      lb |= ((b & 0x30) >> 2);

      hb  = ((b & 0x0C) >> 2);
      hb |= ((b & 0xC0) >> 4);
    };

  void create_lookuptable()
  {
    // generate the lookuptable for 16-qam, 2 spatial stream
    for ( unsigned __int16 i = 0; i <= 255; i++)
    {
      for ( unsigned __int16 j = 0; j <= 255; j++)
      {
        unsigned __int8 b1  = static_cast<unsigned __int8>(i & 0xFF);
        unsigned __int8 b2  = static_cast<unsigned __int8>(j & 0xFF);
        unsigned __int8 lb1 = 0, lb2 = 0, hb1 = 0, hb2 = 0;

        split(b1, lb1, hb1);
        split(b2, lb2, hb2);

        unsigned __int8 vb1 = ((lb2 << 4) | lb1);
        unsigned __int8 vb2 = ((hb2 << 4) | hb1);

        lookuptable[b1][b2][0] = vb1;
        lookuptable[b1][b2][1] = vb2;
      }
    }
  }
};

struct stream_paser_64qam_2ss
{
  static const int NINPUT  = 78;
  static const int NOUTPUT = 39;

  stream_paser_64qam_2ss()
  {
    create_lookuptable();
  }

  __forceinline void operator()(const unsigned __int8 pInput[78], unsigned __int8 pOutput1[], unsigned __int8 pOutput2[])
  {
    const unsigned __int16 *psInput      = reinterpret_cast<const unsigned __int16*>(&pInput[0]);
    unsigned __int16 sInput        = 0;
    unsigned __int32 iOutput       = 0;

    for (int i = 0, j = 0; i < 39; i += 3, j += 3)
    {
      unsigned __int32 *piOutput1 = (unsigned __int32 *)&pOutput1[j];
      unsigned __int32 *piOutput2 = (unsigned __int32 *)&pOutput2[j];

      sInput        = psInput[i] & 0x0FFF;
      *piOutput1    = lookuptable[sInput][0];
      *piOutput2    = lookuptable[sInput][1];

      sInput        = (psInput[i] >> 12) | ((psInput[i + 1] & 0x00FF) << 4);
      iOutput       = lookuptable[sInput][0];
      *piOutput1   |= (iOutput << 6);
      iOutput       = lookuptable[sInput][1];
      *piOutput2   |= (iOutput << 6);

      sInput        = (psInput[i + 1] >> 8) | ((psInput[i + 2] & 0x000F) << 8);
      iOutput       = lookuptable[sInput][0];
      *piOutput1   |= (iOutput << 12);
      iOutput       = lookuptable[sInput][1];
      *piOutput2   |= (iOutput << 12);

      sInput        = (psInput[i + 2] >> 4);
      iOutput       = lookuptable[sInput][0];
      *piOutput1   |= (iOutput << 18);
      iOutput       = lookuptable[sInput][1];
      *piOutput2   |= (iOutput << 18);
    }
  }

private:
  static const int lut_size = 1024 * 4;
  unsigned __int8 lookuptable[lut_size][2];

    void split(unsigned __int16 &b, unsigned __int8 &lb, unsigned __int8 &hb)
    {
      lb  = b & 0x0007;
      lb |= ((b & 0x01C0) >> 3);

      hb  = ((b & 0x0038) >> 3);
      hb |= ((b & 0x0E00) >> 6);
    };

  void create_lookuptable()
  {
    // generate the lookuptable for 64-qam, 2 spatial stream
    for ( unsigned __int16 i = 0; i < lut_size; i++)
    {
      unsigned __int8 lb1 = 0, hb1 = 0;

      split(i, lb1, hb1);

      lookuptable[i][0] = lb1;
      lookuptable[i][1] = hb1;
    }
  }
};        
