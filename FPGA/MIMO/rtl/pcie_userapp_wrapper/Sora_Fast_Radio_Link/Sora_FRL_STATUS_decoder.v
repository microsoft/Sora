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
// Create Date:    16:00:17 07/29/2011 
// Design Name: 
// Module Name:    Sora_FRL_STATUS_IN 
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
/////			Decode four patterns for specific states
/////			Idle									00000000
/////			Reset 								01010101
/////			FIFO full 							00001111
/////			Training_done						00110011
/////
/////////////////////////////////////////////////////////////////////////////////

module Sora_FRL_STATUS_decoder(
		input	clk,
//		input	rst,				// no need for a reset signal since these's no internal state
		
		// input from STATUS line
		input	status_in,
		
		// four output states
		output reg	RST_state,
		output reg	TRAININGDONE_state,
		output reg	IDLE_state,
		output reg	FIFOFULL_state,
		// error state
		output reg	error_state
	);
		
	reg			ambiguity;			// indicates ambiguous status
	reg [3:0]	error_cnt;
	
	reg [7:0]	shift_reg;
	parameter IDLE				= 4'b0001;
	parameter RST				= 4'b0010;
	parameter FIFOFULL		= 4'b0100;
	parameter TRAININGDONE	= 4'b1000;
	reg [3:0]	INT_SAT;
	
//	always@(posedge clk)begin
//		if(rst)
//			shift_reg <= 8'h00;
//		else
//			shift_reg <= {shift_reg[6:0], status_in};
//	end
	initial shift_reg	<= 8'h00;
	always@(posedge clk) shift_reg <= {shift_reg[6:0], status_in};
	
	// decoder
	initial begin
		ambiguity	<= 1'b0;
		INT_SAT		<= IDLE;
	end
	always@(posedge clk) begin
		ambiguity <= 1'b0;
	
		if ( shift_reg == 8'h55 | shift_reg == 8'hAA) begin
			INT_SAT <= RST;
		end else if ( shift_reg == 8'hF0 | shift_reg == 8'h87 | shift_reg == 8'hC3 | shift_reg == 8'hE1 | shift_reg == 8'h78 | shift_reg == 8'h3C | shift_reg == 8'h1E | shift_reg == 8'h0F ) begin
			INT_SAT <= FIFOFULL;
		end else if ( shift_reg == 8'h33 | shift_reg == 8'h66 | shift_reg == 8'hCC | shift_reg == 8'h99 ) begin
			INT_SAT <= TRAININGDONE;
		end else if ( shift_reg == 8'h00) begin
			INT_SAT <= IDLE;
		end else begin// by default, the previous INT_SAT remains, this normally happen when the status is changing			
			INT_SAT <= INT_SAT;
			ambiguity <= 1'b1;
		end
	end
	
	// detect error state
	initial
		error_cnt <= 4'b0000;
	always@(posedge clk) begin
//		if (rst) begin
//			error_cnt <= 4'b0000;
//		end else if(ambiguity) begin
		if (ambiguity) begin
			if (error_cnt != 4'b1000)
				error_cnt <= error_cnt + 4'b0001;
			else
				error_cnt <= error_cnt;
		end else begin
			error_cnt <= 4'b0000;
		end
	end
	always@(posedge clk) begin
		error_state <= (error_cnt == 4'b1000) ? 1'b1 : 1'b0;
	end
	
	// determine the output signals
	initial begin
		RST_state				<= 1'b0;
		TRAININGDONE_state	<= 1'b0;
		FIFOFULL_state			<= 1'b0;
		IDLE_state				<= 1'b1;
	end
	always@(posedge clk) begin
//		if (rst) begin
//			RST_state				<= 1'b0;
//			TRAININGDONE_state	<= 1'b0;
//			FIFOFULL_state			<= 1'b0;
//			IDLE_state				<= 1'b1;
//		end else if (INT_SAT == RST) begin
		if (INT_SAT == RST) begin
			RST_state				<= 1'b1;
			TRAININGDONE_state	<= 1'b0;
			FIFOFULL_state			<= 1'b0;
			IDLE_state				<= 1'b0;
		end else if ( INT_SAT == FIFOFULL ) begin
			RST_state				<= 1'b0;
			TRAININGDONE_state	<= 1'b1;
			FIFOFULL_state			<= 1'b1;
			IDLE_state				<= 1'b0;
		end else if ( INT_SAT == TRAININGDONE ) begin
			RST_state				<= 1'b0;
			TRAININGDONE_state	<= 1'b1;
			FIFOFULL_state			<= 1'b0;
			IDLE_state				<= 1'b0;
		end else if ( INT_SAT == IDLE ) begin
			RST_state				<= 1'b0;
			TRAININGDONE_state	<= 1'b0;
			FIFOFULL_state			<= 1'b0;
			IDLE_state				<= 1'b1;
		end else begin	// should not go into this branch
			RST_state				<= 1'b0;
			TRAININGDONE_state	<= 1'b0;
			FIFOFULL_state			<= 1'b0;
			IDLE_state				<= 1'b1;
		end
	end

endmodule
