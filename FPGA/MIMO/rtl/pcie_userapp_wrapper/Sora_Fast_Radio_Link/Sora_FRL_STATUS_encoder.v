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
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    16:08:11 07/29/2011 
// Design Name: 
// Module Name:    Sora_FRL_STATUS_OUT 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
/////
/////			Generate four patterns for specific states
/////			Idle								00000000
/////			Reset 								01010101
/////			FIFO full 							00001111
/////			Training_done						00110011
/////
/////////////////////////////////////////////////////////////////////////////////

module Sora_FRL_STATUS_encoder(
		input	clk,
//		input	rst,
		
		// input from STATUS line
		output	status_out,
		
		// four output states
		input	RST_signal,
		input	TRAININGDONE_signal,
		input	IDLE_signal,
		input	FIFOFULL_signal
   );
	 
//	wire	rst_one;
	reg [2:0]	counter;
	reg	status_r;
	
	reg [1:0]	Status_state;
	parameter STATE_IDLE				= 2'b00;
	parameter STATE_TRAININGDONE	= 2'b01;
	parameter STATE_FIFOFULL		= 2'b10;
	parameter STATE_RST				= 2'b11;
	
	parameter STR_IDLE			= 8'b0000_0000;
	parameter STR_TRAININGDONE	= 8'b0011_0011;
	parameter STR_FIFOFULL		= 8'b0000_1111;
	parameter STR_RST				= 8'b0101_0101;

	// determine Status_state according to input signals
	initial 
		Status_state	<= STATE_IDLE;
	always@(posedge clk)begin
//		if (rst | RST_signal)
		if (RST_signal)
			Status_state	<= STATE_RST;
		else if (IDLE_signal)		// this signal is subtle, it's asserted when a RST state is received from Status line,
											// and in this case, FIFOFULL_signal & TRAININGDONE_signal should be deasserted outside of this module
			Status_state	<= STATE_IDLE;
		else if (FIFOFULL_signal)
			Status_state	<= STATE_FIFOFULL;
		else if (TRAININGDONE_signal)
			Status_state	<= STATE_TRAININGDONE;
		else
			Status_state	<= STATE_IDLE;
	end

	initial 
		counter <= 3'b000;
	always@(posedge clk)
		counter <= counter + 3'b001;
//	always@(posedge clk)begin
//		if (rst)
//			counter <= 3'b000;
//		else
//			counter <= counter + 3'b001;
//	end
	
	assign status_out = status_r;
	// Status encoder
	always@(posedge clk)begin
		case(Status_state)
			STATE_RST:
				status_r	<= STR_RST[counter];
			STATE_TRAININGDONE:
				status_r	<= STR_TRAININGDONE[counter];
			STATE_FIFOFULL:
				status_r	<= STR_FIFOFULL[counter];
			STATE_IDLE:
				status_r	<= STR_IDLE[counter];
		endcase
	end

endmodule
