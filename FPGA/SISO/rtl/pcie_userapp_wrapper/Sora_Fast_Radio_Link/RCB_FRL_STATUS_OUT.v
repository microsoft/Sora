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
// Create Date:    14:54:18 08/11/2009 
// Design Name: 
// Module Name:    STATUS_OUT 
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

module RCB_FRL_STATUS_OUT(	CLK,
							RESET,				// input indicating whether channel is reset, reset pulse of the whole lvds channel
							MODULE_RST,			// reset pulse for this module only, should be shorter than RESET
							FIFO_FULL,			// input indicating whether FIFO is FULL
							TRAINING_DONE,		// input indicating whether TRAINING is done
							STATUS,
							INT_SAT				// coded status output
							);
	/////////////////////////////////////////////////////////////////////////////////						
	input		CLK;
	input		MODULE_RST;
	input		RESET,
				FIFO_FULL,
				TRAINING_DONE;
	output	STATUS;
	/////////////////////////////////////////////////////////////////////////////////
	reg		STATUS;
	
	output reg		[1:0] INT_SAT;
	parameter RST = 2'b00;
	parameter FULL = 2'b01;
	parameter DONE = 2'b10;
	parameter IDLE = 2'b11;
	
	reg		[2:0] counter;
	
	wire MODULE_RST_one;
	
	rising_edge_detect MODULE_RESET_one_inst(
						 .clk(CLK),
						 .rst(1'b0),
						 .in(MODULE_RST),
						 .one_shot_out(MODULE_RST_one)
						 );
	
	/////////////////////////////////////////////////////////////////////////////////
	/// Determine which state it is
	
	always @ ( posedge CLK ) begin
		if ( counter == 3'b000 ) begin
			if ( RESET == 1'b1 ) begin
				INT_SAT <= RST;
			end
			else if ( FIFO_FULL == 1'b1 & TRAINING_DONE == 1'b1 ) begin
				INT_SAT <= FULL;
			end
			else if ( TRAINING_DONE == 1'b1 ) begin
				INT_SAT <= DONE;
			end
			else begin
				INT_SAT <= IDLE;
			end
		end
	end
	
	/////////////////////////////////////////////////////////////////////////////////
	/// Counter runs
	
	always @ ( posedge CLK ) begin
//		if ( MODULE_RST == 1'b1 ) begin			// Jiansong: how can it send out reset status
		if ( MODULE_RST_one == 1'b1 ) begin
			counter <= 3'b000;
		end
		else begin
			counter <= counter + 3'b001;
		end
	end
	
	/////////////////////////////////////////////////////////////////////////////////
	/// pattern encode
	/// Idle		00000000
	/// Reset 		01010101
	/// FIFO_full 	00001111
	/// Train Done	00110011
	
	always @ ( posedge CLK) begin
		if ( INT_SAT == RST ) begin
			if ( counter == 3'b000 | counter == 3'b010 | counter == 3'b100 | counter == 3'b110 ) begin
				STATUS <= 1'b0;
			end
			else if (counter == 3'b001 | counter == 3'b011 | counter == 3'b101 | counter == 3'b111 ) begin
				STATUS <= 1'b1;
			end
		end
		else if ( INT_SAT == FULL) begin
			if (counter == 3'b000 | counter == 3'b001 | counter == 3'b010 | counter == 3'b011 ) begin
				STATUS <= 1'b0;
			end
			else if ( counter == 3'b100 | counter == 3'b101 | counter == 3'b110 | counter == 3'b111  ) begin
				STATUS <= 1'b1;
			end
		end
		else if ( INT_SAT == DONE) begin
			if ( counter == 3'b000 | counter == 3'b001 | counter == 3'b100 | counter == 3'b101  ) begin
				STATUS <= 1'b0;
			end
			else if ( counter == 3'b010 | counter == 3'b011 | counter == 3'b110 | counter == 3'b111  )begin
				STATUS <= 1'b1;
			end
		end
		else if ( INT_SAT == IDLE) begin
			STATUS <= 1'b0;
		end
	end
	
endmodule
