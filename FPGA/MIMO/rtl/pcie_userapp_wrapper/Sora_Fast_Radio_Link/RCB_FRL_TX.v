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
// Create Date:    11:07:49 02/11/2009 
// Design Name: 
// Module Name:    utils 
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

module RCB_FRL_TX (
		input				CLK,
		input				CLKDIV,
		input				RST,
		input [31:0]	DATA_IN,
		input				SEND_EN,
		input				TRAINING_DONE,
		output [3:0]	OSER_OQ,
		output			RDEN
	);

	reg  		[31:0] frame_data;
		
	reg [8:0] count;
	parameter NUM = 10'h008;    // packet payload size on a single LVDS channel is 8-byte
	
	reg RDEN_REG;		
	assign 	RDEN = RDEN_REG;
	
	wire [7:0] PATTERN;
	wire [31:0] data_to_oserdes;	
//	assign data_to_oserdes = TRAINING_DONE ? frame_data : {PATTERN,PATTERN,PATTERN,PATTERN};
	assign data_to_oserdes = TRAINING_DONE ? frame_data : {8'h5c,8'h5c,8'h5c,8'h5c};				// training pattern for Spartan6

	
	// using counter to implement the state machine, state transition 
	always @ (posedge CLKDIV) begin
		if (RST == 1'b1) begin
			count <= 9'h000;
		end else begin
			
			if (count == 9'h000) begin
				if (SEND_EN == 1'b1) 
					count <= 9'h001;
				else
					count <= 9'h000;
			
			//end else if (count == (NUM+9'h004) ) begin // determine how many 00 will be inserted in two successive packets
			end else if (count == (NUM+9'h002) ) begin // no 00 will be inserted in two successive packets
				if (SEND_EN == 1'b1) begin
					count <= 9'h001;
				end else begin
					count <= 9'h000;
				end				
			
			end else begin
				count <= count + 9'h001;
			end
			
		end			
	end
	
	// RDEN_REG
	always @ (posedge CLKDIV) begin
		if (RST == 1'b1) begin
			RDEN_REG <= 1'b0;
		end
		else if (count == 9'h001) begin
			RDEN_REG <= 1'b1;
		end
		else if (count == NUM+9'h001) begin
			RDEN_REG <= 1'b0;
		end
	end
		
	// training pattern generator
	RCB_FRL_TrainingPattern RCB_FRL_TrainingPattern_inst(
		.clk			(CLKDIV),
		.rst			(RST),
		.trainingpattern	(PATTERN)
	);			
		
	// frame encapsulation
	always @ (posedge CLKDIV) begin
		if ( RST == 1'b1 ) begin
			frame_data[31:0] <= 32'h00000000;
		end else if (count == 9'h001) begin		// frame header
			frame_data[31:0] <= 32'hF5F5F5F5;
		end else if (count == 9'h002) begin
			frame_data[31:0] <= {NUM[7:0],NUM[7:0],NUM[7:0],NUM[7:0]};	// frame size
		end else if (count == 9'h000) begin
			frame_data[31:0] <= 32'h44444444;	// dummy
		end else if (count > NUM+9'h002) begin
			frame_data[31:0] <= 32'h00000000;	// error, should never in this state
		end else begin
			frame_data[31:0] <= DATA_IN[31:0];	// frame payload
		end
	end
	
	
	// output serdes
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst1 (
		.OQ(OSER_OQ[0]), 
		.CLK(CLK), 
		.CLKDIV(CLKDIV), 
		.DI(data_to_oserdes[7:0]), 
		.OCE(1'b1), 
		.SR(RST)
	);
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst2 (
		.OQ(OSER_OQ[1]), 
		.CLK(CLK), 
		.CLKDIV(CLKDIV), 
		.DI(data_to_oserdes[15:8]), 
		.OCE(1'b1), 
		.SR(RST)
	);
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst3 (
		.OQ(OSER_OQ[2]), 
		.CLK(CLK), 
		.CLKDIV(CLKDIV), 
		.DI(data_to_oserdes[23:16]), 
		.OCE(1'b1), 
		.SR(RST)
	);
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst4 (
		.OQ(OSER_OQ[3]), 
		.CLK(CLK), 
		.CLKDIV(CLKDIV), 
		.DI(data_to_oserdes[31:24]), 
		.OCE(1'b1), 
		.SR(RST)
	);
	
endmodule 