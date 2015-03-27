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
// Create Date:    15:18:35 08/11/2009 
// Design Name: 
// Module Name:    STATUS_IN 
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
/////			Decode four patterns for specific states\
/////			Idle								00000000
/////			Reset 								01010101
/////			FIFO full 							00001111
/////			Training_done						00110011
/////
/////////////////////////////////////////////////////////////////////////////////


module RCB_FRL_STATUS_IN(	
	input CLK,
	input MODULE_RST,
	output reg		RESET,				//output indicating reset state
	output reg		FIFO_FULL,			//output indicating full state
	output reg		TRAINING_DONE,		//output indicating done state
	input STATUS,
	output reg		status_error		// indicates error if ambiguous status lasts longer than 8 cycles
//	output reg		IDLE_RESET			// Jiansong: why we need IDLE_RESET?
							);
/////////////////////////////////////////////////////////////////////////////////							
//	input			CLK;
//	input			MODULE_RST;
//	input			STATUS;
//	output		RESET,
//					FIFO_FULL,
//					TRAINING_DONE,
//					IDLE_RESET;
/////////////////////////////////////////////////////////////////////////////////	
//	reg			RESET,
//					FIFO_FULL,
//					TRAINING_DONE,
	reg			IDLE_RESET;
	reg			IDLE_FLAG;					// why we need this flag?
	
	reg			ambiguity;			// indicates ambiguous status
	reg [2:0]		error_cnt;
	
	reg	[7:0] shift_reg;
	parameter RST = 2'b00;
	parameter FULL = 2'b01;
	parameter DONE = 2'b10;
	parameter IDLE = 2'b11;
	reg	[1:0] INT_SAT;
	
/////////////////////////////////////////////////////////////////////////////////	
	always @ ( negedge CLK ) begin
		if ( MODULE_RST == 1'b1 ) begin
			shift_reg <= 8'h00;
		end
		else begin
			shift_reg <= {shift_reg[6:0], STATUS};
		end
	end
	
	/////////////////////////////////////////////////////////////////////////////////
	/// Pattern Recognition
// Modified by Jiansong, 2010-5-25, remove ambiguity
	always @ ( negedge CLK ) begin
		ambiguity <= 1'b0;
	
		if ( shift_reg == 8'h55 | shift_reg == 8'hAA) begin
			INT_SAT <= RST;
		end
		else if ( shift_reg == 8'hF0 | shift_reg == 8'h87 | shift_reg == 8'hC3 | shift_reg == 8'hE1 | shift_reg == 8'h78 | shift_reg == 8'h3C | shift_reg == 8'h1E | shift_reg == 8'h0F ) begin
			INT_SAT <= FULL;
		end
		else if ( shift_reg == 8'h33 | shift_reg == 8'h66 | shift_reg == 8'hCC | shift_reg == 8'h99 ) begin
			INT_SAT <= DONE;
		end
		else if ( shift_reg == 8'h00) begin
			INT_SAT <= IDLE;
		end
		else begin// by default, the previous INT_SAT remains, this normally happen when the status is changing			
			INT_SAT <= INT_SAT;
			ambiguity <= 1'b1;
		end
	end
	
	always@ (negedge CLK) begin
		if (MODULE_RST) begin
			error_cnt <= 3'b000;
		end else if(ambiguity) begin
			if (error_cnt != 3'b111)
				error_cnt <= error_cnt + 3'b001;
			else
				error_cnt <= error_cnt;
		end else begin
			error_cnt <= 3'b000;
		end
	end
	
	always@ (negedge CLK) begin
		status_error <= (error_cnt == 3'b111) ? 1'b1 : 1'b0;
	end
	
	/////////////////////////////////////////////////////////////////////////////////
	/// States are exclusive of each other
	always @ (posedge CLK) begin
		if ( MODULE_RST == 1'b1 ) begin
			RESET <= 1'b0;
			TRAINING_DONE <= 1'b0;
			FIFO_FULL <= 1'b0;
			IDLE_RESET <= 0;
			IDLE_FLAG <= 0;
		end
		else if ( INT_SAT == RST) begin
			RESET <= 1'b1;
			TRAINING_DONE <= 1'b0;
			FIFO_FULL <= 1'b0;
			IDLE_RESET <= 0;
			IDLE_FLAG <= 0;
		end
		else if ( INT_SAT == DONE ) begin
			TRAINING_DONE <= 1'b1;
			FIFO_FULL <= 1'b0;
			RESET <= 1'b0;
			IDLE_RESET <= 0;
			IDLE_FLAG <= 0;
		end
		else if ( INT_SAT == FULL ) begin
			RESET <= 1'b0;
			FIFO_FULL <= 1'b1;
			TRAINING_DONE <= 1'b1;
			IDLE_RESET <= 0;
			IDLE_FLAG <= 0;
		end
		else if ( INT_SAT == IDLE ) begin
			if(IDLE_FLAG == 0)					// Jiansong: edge detection
			begin
				IDLE_FLAG <= 1;
				IDLE_RESET <= 1;
			end
			else
			begin
				IDLE_RESET <= 0;
			end
			RESET <= 1'b0;
			FIFO_FULL <= 1'b0;
			TRAINING_DONE <= 1'b0;
		end
	end
endmodule
