#pragma once

// define a brute force Mux

template< class T_CTX, class T_IN, 
class T_OUT0 = CVoid,  class T_NEXT0 = CBrick,
class T_OUT1 = CVoid,  class T_NEXT1 = CBrick,
class T_OUT2 = CVoid,  class T_NEXT2 = CBrick,
class T_OUT3 = CVoid,  class T_NEXT3 = CBrick,
class T_OUT4 = CVoid,  class T_NEXT4 = CBrick,
class T_OUT5 = CVoid,  class T_NEXT5 = CBrick,
class T_OUT6 = CVoid,  class T_NEXT6 = CBrick,
class T_OUT7 = CVoid,  class T_NEXT7 = CBrick,
class T_OUT8 = CVoid,  class T_NEXT8 = CBrick,
class T_OUT9 = CVoid,  class T_NEXT9 = CBrick>
class TMux : public CBrick { 
public: 
    T_NEXT0*  next_0;  
    T_OUT0    opin_0;  
    T_NEXT1*  next_1;  
    T_OUT1    opin_1;  
    T_NEXT2*  next_2;  
    T_OUT2    opin_2;  
    T_NEXT3*  next_3;  
    T_OUT3    opin_3;  
    T_NEXT4*  next_4;  
    T_OUT4    opin_4;  
    T_NEXT5*  next_5;  
    T_OUT5    opin_5;  
    T_NEXT6*  next_6;  
    T_OUT6    opin_6;  
    T_NEXT7*  next_7;  
    T_OUT7    opin_7;  
    T_NEXT8*  next_8;  
    T_OUT8    opin_8;  
    T_NEXT9*  next_9;  
    T_OUT9    opin_9;  
    TMux () : 
        next_0(NULL), 
        next_1(NULL), 
        next_2(NULL), 
        next_3(NULL), 
        next_4(NULL), 
        next_5(NULL), 
        next_6(NULL), 
        next_7(NULL), 
        next_8(NULL), 
        next_9(NULL) {}; 
		
	TMux (	
		T_NEXT0 * n0, 
		T_NEXT1 * n1, 
		T_NEXT2 * n2 = NULL, 
		T_NEXT3 * n3 = NULL, 
		T_NEXT4 * n4 = NULL, 
		T_NEXT5 * n5 = NULL, 
		T_NEXT6 * n6 = NULL, 
		T_NEXT7 * n7 = NULL, 
		T_NEXT8 * n8 = NULL, 
		T_NEXT9 * n9 = NULL ) : 
		next_0 (n0), 
		next_1 (n1), 
		next_2 (n2), 
		next_3 (n3), 
		next_4 (n4), 
		next_5 (n5), 
		next_6 (n6), 
		next_7 (n7), 
		next_8 (n8), 
		next_9 (n9) {}; 
		
    FINL bool InstallPin (int n, CBrick* p_n ) { 
		switch (n) { 
        case 0: 
            next_0 = static_cast<T_NEXT0*>( p_n); 
            break;
        case 1: 
            next_1 = static_cast<T_NEXT1*>( p_n); 
            break;
        case 2: 
            next_2 = static_cast<T_NEXT2*>( p_n); 
            break;
        case 3: 
            next_3 = static_cast<T_NEXT3*>( p_n); 
            break;
        case 4: 
            next_4 = static_cast<T_NEXT4*>( p_n); 
            break;
        case 5: 
            next_5 = static_cast<T_NEXT5*>( p_n); 
            break;
        case 6: 
            next_6 = static_cast<T_NEXT6*>( p_n); 
            break;
        case 7: 
            next_7 = static_cast<T_NEXT7*>( p_n); 
            break;
        case 8: 
            next_8 = static_cast<T_NEXT8*>( p_n); 
            break;
        case 9: 
            next_9 = static_cast<T_NEXT9*>( p_n); 
            break;
		default:
			return false;
        }; 
		return true;
    }; 
	
	
	FINL bool Reset ( T_CTX & ctx ) {return true; };	
	FINL bool Flush ( T_CTX & ctx ) {return true; };	
	FINL bool Process ( T_IN & ipin, T_CTX & ctx ) {return true;};
};

