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
// Create Date:    18:23:12 08/15/2011 
// Design Name: 
// Module Name:    RCB_FRL_RX_Data_FIFO_8bit 
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
module RCB_FRL_RX_Data_FIFO_8bit(
		input			RDCLK, 
		input			RDEN, 
		input			WRCLK, 
		input			WREN, 
		input			RST,
		input [7:0]	DI,
		output		ALMOSTEMPTY, 
		output		ALMOSTFULL, 
		output		EMPTY, 
		output		FULL,
		output [7:0]	DO
   );

	wire [7:0] temp1;

	FIFO18  FIFO18_inst (
		.ALMOSTEMPTY(ALMOSTEMPTY), // 1-bit almost empty output flag
		.ALMOSTFULL(ALMOSTFULL), // 1-bit almost full output flag
		.DO({temp1, DO[7:0]}), // 16-bit data output
		.DOP(), // 2-bit parity data output
		.EMPTY(EMPTY), // 1-bit empty output flag
		.FULL(FULL), // 1-bit full output flag
		.RDCOUNT(), // 12-bit read count output
		.RDERR(), // 1-bit read error output
		.WRCOUNT(), // 12-bit write count output
		.WRERR(), // 1-bit write error
		.DI({8'h0,DI[7:0]}), // 16-bit data input
		.DIP(), // 2-bit parity input
		.RDCLK(RDCLK), // 1-bit read clock input
		.RDEN(RDEN), // 1-bit read enable input
		.RST(RST), // 1-bit reset input
		.WRCLK(WRCLK), // 1-bit write clock input
		.WREN(WREN) // 1-bit write enable input
	);
	defparam FIFO18_inst.DATA_WIDTH = 9;
	defparam FIFO18_inst.ALMOST_EMPTY_OFFSET = 6;

endmodule
