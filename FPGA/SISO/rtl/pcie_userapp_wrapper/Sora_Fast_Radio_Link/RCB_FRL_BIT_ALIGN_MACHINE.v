/*+--------------------------------------------------------------------------
Copyright (c) 2015, Microsoft Corporation 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

`timescale 1ns / 1ps
///////////////////////////////////////////////////////////////////////////////
// 
// Summary:
//
// The BIT_ALIGN_MACHINE module analyzes the data input of a single channel
// to determine the optimal clock/data relationship for that channel.  By
// dynamically changing the delay of the data channel (with respect to the 
// sampling clock), the machine places the sampling point at the center of
// the data eye.
//  
//----------------------------------------------------------------

module RCB_FRL_BIT_ALIGN_MACHINE
	(
	RXCLKDIV,
	RXDATA,
	RST,
	USE_BITSLIP,
	SAP,
	INC,
	ICE,
	BITSLIP,
	DATA_ALIGNED
	);
	
input		RXCLKDIV;	//RX PARALLEL SIDE CLOCK
input	[7:0]	RXDATA;		//DATA FROM ONE CHANNEL ONLY
input		RST;		//RESET ALL CIRCUITRY IN MACHINE
input		USE_BITSLIP;	//OPTION TO BYPASS/INCLUDE BITSLIP FUNCTION
input		SAP;		//INDICATES THAT DATA IS STABLE AFTER CHANNEL TRANSITION
				//E.G. MACHINE FINISHES CHANNEL 12 AND GOES ON TO 13
output		INC;		//MACHINE ISSUES DELAY INCREMENT TO APPROPRIATE DATA CHANNEL
output		ICE;		//MACHINE ISSUES DELAY DECREMENT TO APPROPRIATE DATA CHANNEL
output		BITSLIP;	//MACHINE ISSUES BITSLIP COMMAND TO APPROPRIATE DATA CHANNEL
output		DATA_ALIGNED;	//FLAG INDICATING ALIGNMENT COMPLETE ON THIS CHANNEL

reg		INC; 
reg		ICE; 
reg		BITSLIP;
reg		COUNT; 
reg		UD; 
reg		STORE;
reg		STORE_DATA_PREV; 
reg		COUNT_SAMPLE;
reg		UD_SAMPLE;
reg	[4:0] 	CURRENT_STATE;
reg	[4:0]	NEXT_STATE;

wire	[6:0]	COUNT_VALUE; 
wire	[6:0]	HALF_DATA_EYE;
wire	[7:0]	RXDATA_PREV;
wire	[6:0]	COUNT_VALUE_SAMPLE;
wire	[6:0]	CVS;
wire	[6:0]	CVS_ADJUSTED;
wire	[7:0]	CHECK_PATTERN;

RCB_FRL_count_to_128 machine_counter_total(.clk(RXCLKDIV), .rst(RST), .count(COUNT_SAMPLE), .ud(UD_SAMPLE), .counter_value(COUNT_VALUE_SAMPLE));
RCB_FRL_count_to_128 machine_counter(.clk(RXCLKDIV), .rst(RST), .count(COUNT), .ud(UD), .counter_value(COUNT_VALUE));
RCB_FRL_seven_bit_reg_w_ce tap_reserve(.Q(CVS), .CLK(RXCLKDIV), .CE(STORE), .D(COUNT_VALUE), .RST(RST));  

FDR count_reg(.Q(DATA_ALIGNED), .C(RXCLKDIV), .D(DATA_ALIGNEDx), .R(RST));

//STORE ENTIRE DATA BUS FOR COMPARISON AFTER CHANGING DELAY
FDRE bit0(.Q(RXDATA_PREV[0]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[0]), .R(RST));
FDRE bit1(.Q(RXDATA_PREV[1]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[1]), .R(RST));
FDRE bit2(.Q(RXDATA_PREV[2]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[2]), .R(RST));
FDRE bit3(.Q(RXDATA_PREV[3]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[3]), .R(RST));
FDRE bit4(.Q(RXDATA_PREV[4]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[4]), .R(RST));
FDRE bit5(.Q(RXDATA_PREV[5]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[5]), .R(RST));
FDRE bit6(.Q(RXDATA_PREV[6]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[6]), .R(RST));
FDRE bit7(.Q(RXDATA_PREV[7]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[7]), .R(RST));
//FDRE bit8(.Q(RXDATA_PREV[8]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[8]), .R(RST));
//FDRE bit9(.Q(RXDATA_PREV[9]), .C(RXCLKDIV), .CE(STORE_DATA_PREV), .D(RXDATA[9]), .R(RST));

assign DATA_ALIGNEDx = (~CURRENT_STATE[4] & ~CURRENT_STATE[3] & CURRENT_STATE[2] & CURRENT_STATE[1] & CURRENT_STATE[0]);
//assign CHECK_PATTERN = 8'b11001010;
assign CHECK_PATTERN = 8'b11010010;
//CVS IS A SNAPSHOT OF THE TAP COUNTER.  IT'S VALUE IS THE SIZE OF THE DATA VALID WINDOW
//OUR INTENTION IS TO DECREMENT THE DELAY TO 1/2 THIS VALUE TO BE AT THE CENTER OF THE EYE.
//SINCE IT MAY BE POSSIBLE TO HAVE AN ODD COUNTER VALUE, THE HALVED VALUE WILL BE A DECIMAL.
//IN CASES WHERE CVS IS A DECIMAL, WE WILL ROUND UP.  E.G CVS = 4.5, SO DECREMENT 5 TAPS.
//CVS_ADJUSTED AND HALF_DATA_EYE ARE FINE TUNED ADJUSTMENTS FOR OPTIMAL OPERATION AT HIGH RATES

assign CVS_ADJUSTED	= CVS - 1;			//THE ALGORITHM COUNTS ONE TAP BEYOND EYE, MUST BE REMOVED
assign HALF_DATA_EYE	= {1'b0,CVS_ADJUSTED[6:1]} + CVS_ADJUSTED[0];	//THE CVS[0] FACTOR CAUSES A ROUND-UP

//CURRENT STATE LOGIC
always@(posedge RXCLKDIV or posedge RST)
begin
if(RST == 1'b1) begin
	CURRENT_STATE = 5'b00000;
end
else begin
	CURRENT_STATE = NEXT_STATE;
end
end

//NEXT_STATE LOGIC
always @(CURRENT_STATE or COUNT_VALUE or USE_BITSLIP or RXDATA or CHECK_PATTERN or RXDATA_PREV or COUNT_VALUE_SAMPLE or SAP or HALF_DATA_EYE)
begin
	case(CURRENT_STATE)
	5'b00000:	if (SAP == 1'b0)	//RST STATE
				NEXT_STATE <= 5'b00001;
			else
				NEXT_STATE <= 5'b00000; 	
			
	5'b00001:	begin	//INITIAL STATE, SAMPLE TRAINING BIT
			if (RXDATA_PREV != RXDATA)
				NEXT_STATE <= 5'b01111;
			else
				NEXT_STATE <= 5'b01000;
			end
			
	5'b01000:	begin	//CHECK SAMPLE TO SEE IF IT IS ON A TRANSITION
			if (RXDATA_PREV != RXDATA)
				NEXT_STATE <= 5'b01111;
			else
			if (COUNT_VALUE_SAMPLE > 7'b0001111)
				NEXT_STATE <= 5'b01011;
			else
				NEXT_STATE <= 5'b01000;
			end
			
	5'b01111:	begin	//IF SAMPLED POINT IS TRANSITION, EDGE IS FOUND, SO INC DELAY TO EXIT TRANSITION
				NEXT_STATE <= 5'b01101;
			end

	5'b01101:	begin	//WAIT 16 CYCLES WHILE APPLYING BITSLIP TO FIND CHECK_PATTERN
			if (COUNT_VALUE_SAMPLE > 7'b0001110)
				NEXT_STATE <= 5'b01111;
			else 
			if (RXDATA == CHECK_PATTERN) //IF CHECK_PATTERN IS FOUND, WE ARE CLOSE TO END OF TRANSITION
				NEXT_STATE <= 5'b01100;
			else
				NEXT_STATE <= 5'b01101;
			end
	
	5'b01100:	begin	//IDLE (NEEDED FOR COUNTER RESET BEFORE NEXT STATE)
				NEXT_STATE <= 5'b10000;
			end
			
	5'b10000:	begin	//IDLE (NEEDED FOR STABILIZATION)
				NEXT_STATE <= 5'b00010;
			end
			
	5'b00010:	begin	//CHECK SAMPLE AGAIN TO SEE IF WE HAVE EXITED TRANSITION
			if (COUNT_VALUE_SAMPLE < 7'b0000011) //ALLOW TIME FOR BITSLIP OP TO STABILIZE
				NEXT_STATE <= 5'b00010;
			else
			if (RXDATA_PREV != RXDATA)
				NEXT_STATE <= 5'b01111;
			else
			if (COUNT_VALUE_SAMPLE > 7'b1111110) //SCAN FOR STABILITY FOR 128 CYCLES
				NEXT_STATE <= 5'b01110;
			else
				NEXT_STATE <= 5'b00010;
			end
			
	5'b01011:	begin	//INITIAL STATE WAS STABLE, SO INC ONCE TO SEARCH FOR TRANS
				NEXT_STATE <= 5'b00100;
			end
			
	5'b00100:	begin	//WAIT 8 CYCLES, COMPARE RXDATA WITH PREVIOUS DATA
			if (COUNT_VALUE_SAMPLE < 7'b0000111)
				NEXT_STATE <= 5'b00100;
			else
			if(RXDATA_PREV != RXDATA)
				NEXT_STATE <= 5'b01111;
			else
				NEXT_STATE <= 5'b01011;
			end
	
	5'b01110:	begin	//DATA IS STABLE AFTER FINDING 1ST TRANS, COUNT 1 TO INCLUDE LAST INC
				NEXT_STATE <= 5'b01001;
			end
			
	5'b01001:	begin	//INC ONCE TO LOOK FOR 2ND TRANS
				NEXT_STATE <= 5'b00011;
			end

	5'b00011:	begin	//WAIT 8 CYCLES, COMPARE RXDATA WITH PREVIOUS DATA
			if (COUNT_VALUE_SAMPLE < 7'b0000111)
				NEXT_STATE <= 5'b00011;
			else
			if(RXDATA_PREV != RXDATA)
				NEXT_STATE <= 5'b10010;
			else
				NEXT_STATE <= 5'b01001;
			end
			
	5'b10010:	begin	//IDLE (NEEDED FOR COUNTER RESET BEFORE NEXT STATE)
				NEXT_STATE <= 5'b01010;
			end
			
	5'b01010:	begin	//DECREMENT TO MIDDLE OF DATA EYE
			if (COUNT_VALUE_SAMPLE < HALF_DATA_EYE-1)
				NEXT_STATE <= 5'b01010;
			else
				NEXT_STATE <= 5'b00101;
			end
	
	5'b00101:	begin	//SAMPLE PATTERN 16 TIMES TO SEE IF WORD ALIGNMENT NEEDED
			if(USE_BITSLIP == 1'b0)
    				NEXT_STATE <= 5'b00111;
    			else
    			if(COUNT_VALUE < 7'h0F)
    				NEXT_STATE <= 5'b00101;
    			else
    			if (RXDATA == CHECK_PATTERN)
    				NEXT_STATE <= 5'b00111;
   			else
    				NEXT_STATE <= 5'b00110;
			end
	
	5'b00110:	begin	//INITIATE 1 BITSLIP
			NEXT_STATE <= 5'b00101;
			end
	
	5'b00111:	if (SAP == 1'b0)	//TRAINING COMPLETE FOR THIS CHANNEL
				NEXT_STATE <= 5'b00111;  			
    			else
    				NEXT_STATE <= 5'b00000;  			
    				
    	default:	NEXT_STATE <= 5'b00000;
	endcase
end

//OUTPUT LOGIC

always @(CURRENT_STATE)
begin
	case(CURRENT_STATE)
	5'b00000:	begin	//RST STATE
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
	
	5'b00001:	begin	//INITIAL STATE, SAMPLE TRAINING BIT
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
			
	5'b01000:	begin	//CHECK SAMPLE TO SEE IF IT IS ON A TRANSITION
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b1;
			UD_SAMPLE = 1'b1;
			end
			
	5'b01111:	begin	//IF SAMPLED POINT IS TRANSITION, EDGE IS FOUND, SO INC DELAY TO EXIT TRANSITION
			INC = 1'b1;
  			ICE = 1'b1;
			COUNT = 1'b0;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0; 
			UD_SAMPLE = 1'b0;
			end
	
	5'b01101:	begin	//WAIT 16 CYCLES WHILE APPLYING BITSLIP TO FIND CHECK_PATTERN
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b1;
			COUNT_SAMPLE = 1'b1;
			UD_SAMPLE = 1'b1;
			end
			
	5'b01100:	begin	//IDLE (NEEDED FOR COUNTER RESET BEFORE NEXT STATE)
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
	
	5'b10000:	begin	//IDLE (NEEDED FOR STABILIZATION)
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
	
	5'b00010:	begin	//CHECK SAMPLE AGAIN TO SEE IF WE HAVE EXITED TRANSITION
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b1;
			UD_SAMPLE = 1'b1;
			end

	5'b01011:	begin	//INITIAL STATE WAS STABLE, SO INC ONCE TO SEARCH FOR TRANS
			INC = 1'b1;
  			ICE = 1'b1;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
	
	5'b00100:	begin	//WAIT 8 CYCLES, COMPARE RXDATA WITH PREVIOUS DATA
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b0;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b1;
			UD_SAMPLE = 1'b1;
			end
	
	5'b01110:	begin	//DATA IS STABLE AFTER FINDING 1ST TRANS, COUNT 1 TO INCLUDE LAST INC
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b1;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
				
	5'b01001:	begin	//INC ONCE TO LOOK FOR 2ND TRANS
			INC = 1'b1;
  			ICE = 1'b1;
			COUNT = 1'b1;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end

	5'b00011:	begin	//WAIT 8 CYCLES, COMPARE RXDATA WITH PREVIOUS DATA
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b1;			
			STORE = 1'b1;
			STORE_DATA_PREV = 1'b0;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b1;
			UD_SAMPLE = 1'b1;
			end	
			
	5'b10010:	begin	//IDLE (NEEDED FOR COUNTER RESET BEFORE NEXT STATE)
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
			
	5'b01010:	begin	//DECREMENT TO CENTER OF DATA EYE
			INC = 1'b0;
  			ICE = 1'b1;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b1;
			UD_SAMPLE = 1'b1;
			end	
	
	5'b00101:	begin	//SAMPLE PATTERN 16 TIMES TO SEE IF WORD ALIGNMENT NEEDED
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b1;
			UD = 1'b1;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
			
	5'b00110:	begin	//INITIATE 1 BITSLIP
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b1;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
			
	5'b00111:	begin	//TRAINING COMPLETE ON THIS CHANNEL
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
			
	default:	begin	
			INC = 1'b0;
  			ICE = 1'b0;
			COUNT = 1'b0;
			UD = 1'b0;			
			STORE = 1'b0;
			STORE_DATA_PREV = 1'b1;
			BITSLIP = 1'b0;
			COUNT_SAMPLE = 1'b0;
			UD_SAMPLE = 1'b0;
			end
	endcase
end

endmodule


`timescale  1 ns / 10 ps
module RCB_FRL_seven_bit_reg_w_ce(Q, CLK, CE, D, RST);

input	[6:0]	D;
input		CLK, CE, RST;

output	[6:0]	Q;	

FDRE bit0(.Q(Q[0]), .C(CLK), .CE(CE), .D(D[0]), .R(RST));
FDRE bit1(.Q(Q[1]), .C(CLK), .CE(CE), .D(D[1]), .R(RST));
FDRE bit2(.Q(Q[2]), .C(CLK), .CE(CE), .D(D[2]), .R(RST));
FDRE bit3(.Q(Q[3]), .C(CLK), .CE(CE), .D(D[3]), .R(RST));
FDRE bit4(.Q(Q[4]), .C(CLK), .CE(CE), .D(D[4]), .R(RST));
FDRE bit5(.Q(Q[5]), .C(CLK), .CE(CE), .D(D[5]), .R(RST));
FDRE bit6(.Q(Q[6]), .C(CLK), .CE(CE), .D(D[6]), .R(RST));

endmodule
