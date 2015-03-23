#pragma once

#include "wintaps.h"

class TFIR{
public:
    //Each Windows filter will redefine this part
    virtual bool CompTap( uint Filter_type,         
                          uint* Cutoff_freq,        // The second element is just for BPF
                          uint  Sample_rate,        // The unit is MHz
                          uint  Tap_num,  
                          COMPLEX16* Taps,          //The computed taps 
						  int beta                   
    ) const 
    {
        return true;
    }
  
    // perform convolution with the taps   
    FINL 
    vcs Convolute ( const vcs& input,  // The input data 
                    const vcs *Taps,  
                    vcs *temp,   // The Temp Table for next convolution
                    int Tap_num  )
    {
        register vcs ans[4];
        register vcs * pTap = (vcs*)Taps;
        register vcs * pTemp1 = temp;
        register vcs * pTemp2 = temp;

		for ( int i=0; i<4; i ++ ) {
			ans[i] = add((vcs)(mul_high((vs)(input),(vs)(*pTap))),*pTemp1);
			pTap ++; pTemp1 ++;
		}
        
		vcs ret = hadd4 (ans[0], ans[1], ans[2], ans[3] );

		for (int i = 4; i < Tap_num; i++)
		{ 
			*pTemp2 = add((vcs)(mul_high((vs)(input),(vs)(*pTap))), *pTemp1);
			pTemp1 ++; pTap ++; pTemp2++;
		}

		return ret;	
    }

};


DEFINE_LOCAL_CONTEXT(THanningWin,CF_VOID);
template<int N_TAPS>
class THanningWin{
public:
template<TFILTER_ARGS>
class Filter: public TFilter<TFILTER_PARAMS>, public TFIR
{
public:
    uint  filter_type;
    uint* cutoff_freq;
    uint  sample_rate;

protected:  
	static const int tapnum = (N_TAPS%2==0)?(N_TAPS+1):(N_TAPS);    //add 1 to make the taps_num an odd
	static const int inputnum = 4;
	
	A16 COMPLEX16 Taps[tapnum];            
    A16 COMPLEX16 Tap_Table[tapnum+3][4];
	A16 COMPLEX16 Temp_Table[tapnum+3][4];
	
public:
	bool config (uint type, uint fc[2], uint fs)
	{
	    filter_type = type;  //0: LPF; 1: HPF; 2: BPF; 3: BSF 
	    cutoff_freq = fc;
	    sample_rate = fs;
		_init();
		_init_taps();
		return true;
	}

protected:	
	FINL void _init() 
	{
		memset(Taps,0,sizeof(Taps));
		memset(Tap_Table,0,sizeof(Tap_Table));
		memset(Temp_Table,0,sizeof(Temp_Table));
	}

	FINL bool CompTap( uint  Filter_type, 
                       uint* Cutoff_freq, 
                       uint  Sample_rate, 
                       uint  Tap_num,                       
                       COMPLEX16* Taps,
					   int beta
    )const
    {    
		 HanCompTap(Filter_type,Cutoff_freq,Sample_rate,Tap_num,Taps); 
		 return true;
		 
    }

    FINL void _init_taps() 
    { 

        //call the CompTap to Computer the filter taps
        uint* pfc= cutoff_freq;
        CompTap(filter_type,pfc,sample_rate,tapnum,Taps, 0); 

        //allocate the Tabs Table 

        for(int i=0;i<(tapnum);i++)
            Tap_Table[i][0]= Tap_Table[i+1][1]= Tap_Table[i+2][2]= Tap_Table[i+3][3]= Taps[i];
    }

public:
    DEFINE_IPORT (COMPLEX16, 4);
    DEFINE_OPORT (COMPLEX16, 4);

public:	
    
    REFERENCE_LOCAL_CONTEXT(THanningWin);
    STD_TFILTER_CONSTRUCTOR(Filter)
    {
        // default configuration 
        // Low pass with cutoff freq at 4 MHz
        uint cutoff[2] = {4, 0};
		config(TYPE_LOWPASS, cutoff, 40);
    }

	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
    	{
    	    const vcs& input= cast_ref<vcs> (ipin.peek());
            
			vcs* po = (vcs*)opin().append();
            *po = Convolute (input, (vcs*)Tap_Table, (vcs*)Temp_Table, tapnum+3);
            
			Next() -> Process(opin());		
            ipin.pop();						
    	}        
		return true;
    }
};
};

DEFINE_LOCAL_CONTEXT(TKaiserWin,CF_VOID);
template<int N_TAPS>
class TKaiserWin{
public:
template<TFILTER_ARGS>
class Filter: public TFilter<TFILTER_PARAMS>, public TFIR
{
public:
    uint  filter_type;
    uint* cutoff_freq;
    uint  sample_rate;
	int   k_beta;

protected:  
	static const int tapnum = (N_TAPS%2==0)?(N_TAPS+1):(N_TAPS);    //add 1 to make the taps_num an odd
	static const int inputnum = 4;
	
	A16 COMPLEX16 Taps[tapnum];            
    A16 COMPLEX16 Tap_Table[tapnum+3][4];
	A16 COMPLEX16 Temp_Table[tapnum+3][4];
	
public:
	bool config (uint type, uint fc[2], uint fs, uint beta)
	{
	    filter_type = type;  //0: LPF; 1: HPF; 2: BPF; 3: BSF 
	    cutoff_freq = fc;
	    sample_rate = fs;
		k_beta = beta;
		_init();
		_init_taps();
		return true;
	}

protected:	
	FINL void _init() 
	{
		memset(Taps,0,sizeof(Taps));
		memset(Tap_Table,0,sizeof(Tap_Table));
		memset(Temp_Table,0,sizeof(Temp_Table));
	}

	FINL bool CompTap( uint  Filter_type, 
                       uint* Cutoff_freq, 
                       uint  Sample_rate, 
                       uint  Tap_num,                       
                       COMPLEX16* Taps,
					   int beta
    )const
    {    
		 KSCompTap(Filter_type,Cutoff_freq,Sample_rate,Tap_num,Taps,beta); 
		 return true;
		 
    }

    FINL void _init_taps() 
    { 

        //call the CompTap to Computer the filter taps
        uint* pfc= cutoff_freq;
        CompTap(filter_type,pfc,sample_rate,tapnum,Taps,k_beta); 

        //allocate the Tabs Table 

        for(int i=0;i<(tapnum);i++)
            Tap_Table[i][0]= Tap_Table[i+1][1]= Tap_Table[i+2][2]= Tap_Table[i+3][3]= Taps[i];
    }

public:
    DEFINE_IPORT (COMPLEX16, 4);
    DEFINE_OPORT (COMPLEX16, 4);

public:	
    
    REFERENCE_LOCAL_CONTEXT(TKaiserWin);
    STD_TFILTER_CONSTRUCTOR(Filter)
    {
        // default configuration 
        // Low pass with cutoff freq at 4 MHz
        uint cutoff[2] = {4, 0};
		config(TYPE_LOWPASS, cutoff, 40, 4);
    }

	STD_TFILTER_RESET() {}
	STD_TFILTER_FLUSH() {}
	
    BOOL_FUNC_PROCESS(ipin)
    {
        while(ipin.check_read())
    	{
    	    const vcs& input= cast_ref<vcs> (ipin.peek());
            
			vcs* po = (vcs*)opin().append();
            *po = Convolute (input, (vcs*)Tap_Table, (vcs*)Temp_Table, tapnum+3);
            
			Next() -> Process(opin());		
            ipin.pop();						
    	}        
		return true;
    }
};
};