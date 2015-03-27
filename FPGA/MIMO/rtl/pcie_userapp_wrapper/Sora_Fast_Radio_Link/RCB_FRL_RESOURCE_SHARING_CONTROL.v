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
// The RESOURCE_SHARING_CONTROL module allocates the BIT_ALIGN_MACHINE
// module to each of the 16 data channels of the interface.  Each channel 
// must be aligned one at a time, such that the RESOURCE_SHARING_CONTROL module
// must determine when training on a given channel is complete, and then 
// switch the context to the next channel. 
//
//----------------------------------------------------------------

module RCB_FRL_RESOURCE_SHARING_CONTROL (
		input		CLK,
		input		RST,
		input		DATA_ALIGNED,
		output [3:0]	CHAN_SEL,
		output reg	START_ALIGN,
		output	ALL_CHANNELS_ALIGNED
	);
	

wire	[6:0]	COUNT_VALUE;

reg		UD_DELAY;
reg		COUNT_DELAY;
reg		COUNT_CHAN;
reg	[2:0]	CS;
reg	[2:0]	NS;

parameter	INIT				= 3'b000;
parameter	INC_CHAN_SEL	= 3'b001;
parameter	WAIT_8			= 3'b010;
parameter	START_NEXT		= 3'b011;
parameter	LAST_CHAN		= 3'b100;
parameter	TRAIN_DONE		= 3'b101;
parameter	IDLE				= 3'b110;
parameter	START_LAST		= 3'b111;

assign	ALL_CHANNELS_ALIGNED	= CS[2] & ~CS[1] & CS[0];

RCB_FRL_count_to_128 delay_counter(
		.clk(CLK), 
		.rst(RST), 
		.count(COUNT_DELAY), 
		.ud(UD_DELAY), 
		.counter_value(COUNT_VALUE)
	);

RCB_FRL_count_to_16x channel_counter(
		.clk(CLK), 
		.rst(RST), 
		.count(COUNT_CHAN), 
		.counter_value(CHAN_SEL)
	);

//CURRENT STATE LOGIC
always@(posedge CLK) begin
	if (RST == 1'b1)
		CS <= INIT;
	else
		CS <= NS;
end

//NEXT_STATE LOGIC
always @(CS or DATA_ALIGNED or COUNT_VALUE or CHAN_SEL) begin
	case (CS)
		INIT:	begin	
				if (COUNT_VALUE < 7'h08 || DATA_ALIGNED == 1'b0)
					NS <= INIT;
				else
					NS <= IDLE;
			end
		
		IDLE:	NS <= INC_CHAN_SEL;
		
		INC_CHAN_SEL:	NS <= WAIT_8;
		
		WAIT_8:	begin
				if (COUNT_VALUE < 7'h08)
					NS <= WAIT_8;
				else if (CHAN_SEL == 4'b0100)
					NS <= START_LAST;
				else
					NS <= START_NEXT;
			end
		
		START_NEXT:	NS <= INIT;
		
		START_LAST:	NS <= LAST_CHAN;
		
		LAST_CHAN: begin
				if (COUNT_VALUE < 7'h08 || DATA_ALIGNED == 1'b0)
					NS <= LAST_CHAN;
				else
					NS <= TRAIN_DONE;
			end
		
		TRAIN_DONE:	NS <= TRAIN_DONE;
		
		default:	NS <= INIT;
	endcase
end

//OUTPUT LOGIC
always @(CS) begin
	case (CS)
		INIT:	begin
				COUNT_CHAN	<= 1'b0;
				COUNT_DELAY 	<= 1'b1;
				UD_DELAY	<= 1'b1;
				START_ALIGN	<= 1'b0;
			end
		
		IDLE:	begin
				COUNT_CHAN	<= 1'b0;
				COUNT_DELAY 	<= 1'b0;
				UD_DELAY	<= 1'b0;
				START_ALIGN	<= 1'b0;
			end
		
		INC_CHAN_SEL: begin
				COUNT_CHAN	<= 1'b1;
				COUNT_DELAY 	<= 1'b0;
				UD_DELAY	<= 1'b0;
				START_ALIGN	<= 1'b0;
			end
		
		WAIT_8:	begin
				COUNT_CHAN	<= 1'b0;
				COUNT_DELAY 	<= 1'b1;
				UD_DELAY	<= 1'b1;
				START_ALIGN	<= 1'b0;
			end
		
		START_NEXT:	begin
				COUNT_CHAN	<= 1'b0;
				COUNT_DELAY 	<= 1'b0;
				UD_DELAY	<= 1'b0;
				START_ALIGN	<= 1'b1;
			end
		
		START_LAST:	begin
				COUNT_CHAN	<= 1'b0;
				COUNT_DELAY 	<= 1'b0;
				UD_DELAY	<= 1'b0;
				START_ALIGN	<= 1'b1;
			end
		
		LAST_CHAN:	begin
				COUNT_CHAN	<= 1'b0;
				COUNT_DELAY 	<= 1'b1;
				UD_DELAY	<= 1'b1;
				START_ALIGN	<= 1'b0;
			end
		
		TRAIN_DONE:	begin
				COUNT_CHAN	<= 1'b0;
				COUNT_DELAY 	<= 1'b0;
				UD_DELAY	<= 1'b0;
				START_ALIGN	<= 1'b0;
			end
		
		default:	begin
				COUNT_CHAN	<= 1'b1; 
				COUNT_DELAY 	<= 1'b0;
				UD_DELAY	<= 1'b0;
				START_ALIGN	<= 1'b0;
			end
	endcase
end			

endmodule

		
	