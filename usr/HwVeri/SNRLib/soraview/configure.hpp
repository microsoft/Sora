// define the pipe-line configuration

// Pin types
typedef CPinQueue<COMPLEX,1,1,4> CSQueue11;
typedef CPinQueue<COMPLEX,2,2,4> CSQueue22;
typedef CPinQueue<COMPLEX,64,64,160> CSQueue6464;
typedef CPinQueue<uchar, 48, 48, 120> CUCQueue4848;
typedef CPinQueue<uint, 1, 1, 2> CUIQueue11;


SINK_TYPE   (CContext, 
	         CUIQueue11, // input
	         TDiscard, CDiscard);

FILTER_TYPE (CContext,
			 CUIQueue11, 		    // input
			 CUIQueue11,  CDiscard,	// output
			 THDRParser, CHDRParser);

FILTER_TYPE (CContext,
			 CUCQueue4848, 			    // input
			 CUIQueue11,  CHDRParser,	// output
			 THDRViterbi, CHDRViterbi );

FILTER_TYPE (CContext,
			 CSQueue6464, 			    // input
			 CUCQueue4848,  CHDRViterbi,	// output
			 THDRSym, CHDRSym );

SINK_TYPE   (CContext, 
	         CSQueue6464, // input
	         TDiscard, CDiscard1);

FILTER_TYPE (CContext, 
	         CSQueue6464, // input
	         CSQueue6464, CDiscard1,
	         TSNREst,    CSNREst );

FILTER_TYPE (CContext, 
	         CSQueue6464, // input
	         CSQueue6464, CSNREst,
	         TDataSym,    CDataSym );
	         

MUX2_TYPE (  CContext,
			 CSQueue11, 		    // input
			 CSQueue6464,  CHDRSym,	 // output
			 CSQueue6464,  CDataSym, // output1
			 TSymbol, CSymbol );


/*
FILTER_TYPE( CContext,
			 CSQueue11, 			// input
			 CSQueue11,   CSymbol,	// output
			 TChannelEst, CChannelEst );
*/

FILTER_TYPE (CContext, 
             CSQueue11,
             CSQueue11, CSymbol,
             TFreqSync, CFreqSync );


FILTER_TYPE (CContext,
			 CSQueue11,				    // input
			 CSQueue11, CFreqSync,	// output
			 TTimeSync, CTimeSync );
/*
FILTER_TYPE (CContext,
			 CSQueue11,				// input
			 CSQueue11,  CTimeSync,	// output
			 TCoarseFOE, CCoarseFOE );
*/

FILTER_TYPE (CContext,
	         CSQueue11,				// input
	         CSQueue11, CTimeSync,	// output
	         TCarrierSense, CCarrierSense );

FILTER_TYPE (CContext,
			 CSQueue22,				// input
			 CSQueue11,  CCarrierSense,	// output
			 TDecimation, CDecimation );

SOURCE_TYPE (CContext, 
	         CSQueue22, CDecimation , // output 
	         TFileSource, CFileSource);

SOURCE_TYPE (CContext, 
	         CSQueue22, CDecimation , // output 
	         TBufferSource, CBufferSource);



// FileSource -> CDiscard
ADD_SINK (CDiscard, dd);

LINK_FILTER (dd, CHDRParser, fHdrParser );
LINK_FILTER (fHdrParser, CHDRViterbi, fHdrV );
LINK_FILTER (fHdrV, CHDRSym, fHDSym );

ADD_SINK (CDiscard1, dd1);

LINK_FILTER (dd1, CSNREst, fSnrEst );

LINK_FILTER (fSnrEst, CDataSym, fDataSym );
LINK_MUX2 (fHDSym, fDataSym, CSymbol, fSym );

// LINK_FILTER (fSym, CChannelEst, fEst );

LINK_FILTER (fSym,   CFreqSync, fFSync );
LINK_FILTER (fFSync, CTimeSync, fTSync );
// LINK_FILTER (fTSync, CCoarseFOE,    fCFOE );


LINK_FILTER (fTSync,  CCarrierSense, fSC );
// LINK_FILTER (dd,  CCarrierSense, fSC );

LINK_FILTER (fSC,    CDecimation,   fDeci );
LINK_SOURCE (fDeci,  CFileSource,   sFile);


LINK_SOURCE (fDeci,  CBufferSource,   sBuffer);



	
