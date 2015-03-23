#pragma once


struct dsp_demapper
{
  /**
  * The constellation uses the gray code
  */
  static const int lut_bpsk_size  = 256;
  static const int lut_qpsk_size  = 256;
  static const int lut_16qam_size = 256;
  static const int lut_64qam_size = 288;

  static const vcs::data_type DemapMin;
  static const vcs::data_type DemapMax;

  static const vcs::data_type Demap64qamMin;
  static const vcs::data_type Demap64qamMax;

  static const unsigned __int8 lookup_table_bpsk[lut_bpsk_size];

  static const unsigned __int8 lookup_table_qpsk[lut_qpsk_size];

  static const unsigned __int8 lookup_table_16qam1[lut_16qam_size];
  static const unsigned __int8 lookup_table_16qam2[lut_16qam_size];

  static const unsigned __int8 lookup_table_64qam1[lut_64qam_size];
  static const unsigned __int8 lookup_table_64qam2[lut_64qam_size];
  static const unsigned __int8 lookup_table_64qam3[lut_64qam_size];

  static const unsigned __int8 *p_lookup_table_64qam1;
  static const unsigned __int8 *p_lookup_table_64qam2;
  static const unsigned __int8 *p_lookup_table_64qam3;

  dsp_demapper(){}

  __forceinline void demap_limit(const vcs* pvcs, vcs* pout, int vcount)
  {
    for (int i = 0; i < vcount; i++)
    {
      pout[i] = smin(smax((vs&)pvcs[i], (vs&)DemapMin), (vs&)DemapMax);
    }
  }

  __forceinline void demap_bpsk_i(const COMPLEX16 &cinput, unsigned __int8* pOutput)
  {
    *pOutput = lookup_table_bpsk[(unsigned __int8)cinput.re];
  }
  __forceinline void demap_bpsk_q(const COMPLEX16 &cinput, unsigned __int8* pOutput)
  {
    *pOutput = lookup_table_bpsk[(unsigned __int8)cinput.im];
  }

  __forceinline void demap_qpsk(const COMPLEX16 &cinput, unsigned __int8* pOutput)
  {
    pOutput[0] = lookup_table_qpsk[(unsigned __int8)cinput.re];
    pOutput[1] = lookup_table_qpsk[(unsigned __int8)cinput.im];
  }

  __forceinline void demap_16qam(const COMPLEX16 &cinput, unsigned __int8* pOutput)
  {
    pOutput[0] = lookup_table_16qam1[(unsigned __int8)cinput.re];
    pOutput[1] = lookup_table_16qam2[(unsigned __int8)cinput.re];
    pOutput[2] = lookup_table_16qam1[(unsigned __int8)cinput.im];
    pOutput[3] = lookup_table_16qam2[(unsigned __int8)cinput.im];
  }

  __forceinline void demap_64qam(const COMPLEX16 &cinput, unsigned __int8* pOutput)
  {
    pOutput[0] = p_lookup_table_64qam1[cinput.re];
    pOutput[1] = p_lookup_table_64qam2[cinput.re];
    pOutput[2] = p_lookup_table_64qam3[cinput.re];

    pOutput[3] = p_lookup_table_64qam1[cinput.im];
    pOutput[4] = p_lookup_table_64qam2[cinput.im];
    pOutput[5] = p_lookup_table_64qam3[cinput.im];
  }
};

SELECTANY const vcs::data_type dsp_demapper::DemapMin      = {-128, -128, -128, -128, -128, -128, -128, -128};
SELECTANY const vcs::data_type dsp_demapper::DemapMax      = {127, 127, 127, 127, 127, 127, 127, 127};
SELECTANY const vcs::data_type dsp_demapper::Demap64qamMin = {-144, -144, -144, -144, -144, -144, -144, -144};
SELECTANY const vcs::data_type dsp_demapper::Demap64qamMax = {143, 143, 143, 143, 143, 143, 143, 143};


// soft range : 0 ~~ 7
SELECTANY const unsigned __int8 *dsp_demapper::p_lookup_table_64qam1 = &lookup_table_64qam1[144];
SELECTANY const unsigned __int8 *dsp_demapper::p_lookup_table_64qam2 = &lookup_table_64qam2[144];
SELECTANY const unsigned __int8 *dsp_demapper::p_lookup_table_64qam3 = &lookup_table_64qam3[144];

// How to generate LLR?
// Ref: http://home.netcom.com/~chip.f/viterbi/ccode/sdvd.html
// This LUT is constructed at Eb/N0 about 4dB
SELECTANY const unsigned __int8 dsp_demapper::lookup_table_bpsk[lut_bpsk_size] =
{
  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 5,  5, 5, 5, 5,
  5, 5, 5, 5,  5, 6, 6, 6,  6, 6, 6, 6,  6, 6, 6, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,

  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 2,  2, 2, 2, 2,
  2, 2, 2, 2,  2, 3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3,
};

SELECTANY const unsigned __int8 dsp_demapper::lookup_table_qpsk[lut_qpsk_size] =
{
  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 5,  5, 5, 5, 5,
  5, 5, 5, 5,  5, 6, 6, 6,  6, 6, 6, 6,  6, 6, 6, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,
  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,  7, 7, 7, 7,

  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 2,  2, 2, 2, 2,
  2, 2, 2, 2,  2, 3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3,
};

SELECTANY const unsigned __int8 dsp_demapper::lookup_table_16qam1[lut_16qam_size] =
{
  4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 
};

SELECTANY const unsigned __int8 dsp_demapper::lookup_table_16qam2[lut_16qam_size] =
{
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 
  3, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 
  3, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
};


SELECTANY const unsigned __int8 dsp_demapper::lookup_table_64qam1[lut_64qam_size] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 

  4, 4, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};

SELECTANY const unsigned __int8 dsp_demapper::lookup_table_64qam2[lut_64qam_size] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 
  2, 3, 3, 4, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 


  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 4, 3, 3, 
  2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};
SELECTANY const unsigned __int8 dsp_demapper::lookup_table_64qam3[lut_64qam_size] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
  2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5,
  4, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4,
  4, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 5, 4, 4, 3, 3, 2,
  2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
