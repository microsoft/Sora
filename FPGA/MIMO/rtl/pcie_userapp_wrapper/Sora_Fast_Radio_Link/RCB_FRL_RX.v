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
// Create Date:    11:19:11 02/11/2009 
// Design Name: 
// Module Name:    RCB_FRL_RX
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
////////////////////
module RCB_FRL_RX(
		input CLKDIV, 
		input [31:0]	DATA_IN,
		output [31:0]	DATA_OUT, 
		input RST, 
//		input RDCLK, 
		input RDEN,
		output	ALMOSTEMPTY
//		output	fifo_WREN
	);
	
	wire [7:0] data_channel1;
	wire [7:0] data_channel2;
	wire [7:0] data_channel3;
	wire [7:0] data_channel4;
	
	wire	wren_channel1;
	wire	wren_channel2;
	wire	wren_channel3;
	wire	wren_channel4;
	
	wire	pempty_channel1;
	wire	pempty_channel2;
	wire	pempty_channel3;
	wire	pempty_channel4;

								
	assign ALMOSTEMPTY = pempty_channel1 | pempty_channel2 | pempty_channel3 | pempty_channel4;

	
	// four data channels, to do byte alignment and Sora-FRL packet decapsulation
	RCB_FRL_RX_OneDataChannel RCB_FRL_RX_Decapsulation_01_inst (
			.CLKDIV(CLKDIV), .DATA_IN(DATA_IN[7:0]), .RST(RST), .data_valid(wren_channel1), 
			.data_out(data_channel1[7:0])
		);
		
	RCB_FRL_RX_OneDataChannel RCB_FRL_RX_Decapsulation_02_inst (
			.CLKDIV(CLKDIV), .DATA_IN(DATA_IN[15:8]), .RST(RST), .data_valid(wren_channel2), 
			.data_out(data_channel2[7:0])
		);
		
	RCB_FRL_RX_OneDataChannel RCB_FRL_RX_Decapsulation_03_inst (
			.CLKDIV(CLKDIV), .DATA_IN(DATA_IN[23:16]), .RST(RST), .data_valid(wren_channel3), 
			.data_out(data_channel3[7:0])
		);
		
	RCB_FRL_RX_OneDataChannel RCB_FRL_RX_Decapsulation_04_inst (
			.CLKDIV(CLKDIV), .DATA_IN(DATA_IN[31:24]), .RST(RST), .data_valid(wren_channel4), 
			.data_out(data_channel4[7:0])
		);
	
//	assign fifo_WREN = wren_channel1 & wren_channel2 & wren_channel3 & wren_channel4;
	
	// Four 8-bit data fifos for four data channels
	RCB_FRL_RX_Data_FIFO_onechannel	RCB_FRL_RX_Data_FIFO_onechannel_inst1(
		.clk			(CLKDIV),
		.rst			(RST),
		.din			(data_channel1[7:0]), // Bus [7 : 0] 
		.wr_en		(wren_channel1),
		.rd_en		(RDEN),
		.dout			(DATA_OUT[7:0]), // Bus [7 : 0] 
		.full			(),
		.empty		(),
		.prog_empty	(pempty_channel1)
	);
	RCB_FRL_RX_Data_FIFO_onechannel	RCB_FRL_RX_Data_FIFO_onechannel_inst2(
		.clk			(CLKDIV),
		.rst			(RST),
		.din			(data_channel2[7:0]), // Bus [7 : 0] 
		.wr_en		(wren_channel2),
		.rd_en		(RDEN),
		.dout			(DATA_OUT[15:8]), // Bus [7 : 0] 
		.full			(),
		.empty		(),
		.prog_empty	(pempty_channel2)
	);
	RCB_FRL_RX_Data_FIFO_onechannel	RCB_FRL_RX_Data_FIFO_onechannel_inst3(
		.clk			(CLKDIV),
		.rst			(RST),
		.din			(data_channel3[7:0]), // Bus [7 : 0] 
		.wr_en		(wren_channel3),
		.rd_en		(RDEN),
		.dout			(DATA_OUT[23:16]), // Bus [7 : 0] 
		.full			(),
		.empty		(),
		.prog_empty	(pempty_channel3)
	);
	RCB_FRL_RX_Data_FIFO_onechannel	RCB_FRL_RX_Data_FIFO_onechannel_inst4(
		.clk			(CLKDIV),
		.rst			(RST),
		.din			(data_channel4[7:0]), // Bus [7 : 0] 
		.wr_en		(wren_channel4),
		.rd_en		(RDEN),
		.dout			(DATA_OUT[31:24]), // Bus [7 : 0] 
		.full			(),
		.empty		(),
		.prog_empty	(pempty_channel4)
	);
//	RCB_FRL_RX_Data_FIFO_8bit RCB_FRL_RX_Data_FIFO_01_inst(
//			.ALMOSTEMPTY(pempty_channel1), .ALMOSTFULL(), .DO(DATA_OUT [7:0]), .EMPTY(), .FULL(),
//			.DI(data_channel1[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(wren_channel1), .RST(RST)
//		);
//		
//	RCB_FRL_RX_Data_FIFO_8bit RCB_FRL_RX_Data_FIFO_02_inst(
//			.ALMOSTEMPTY(pempty_channel2), .ALMOSTFULL(), .DO(DATA_OUT [15:8]), .EMPTY(), .FULL(),
//			.DI(data_channel2[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(wren_channel2), .RST(RST)
//		);
//		
//	RCB_FRL_RX_Data_FIFO_8bit RCB_FRL_RX_Data_FIFO_03_inst(
//			.ALMOSTEMPTY(pempty_channel3), .ALMOSTFULL(), .DO(DATA_OUT [23:16]), .EMPTY(), .FULL(),
//			.DI(data_channel3[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(wren_channel3), .RST(RST)
//		);
//		
//	RCB_FRL_RX_Data_FIFO_8bit RCB_FRL_RX_Data_FIFO_04_inst(
//			.ALMOSTEMPTY(pempty_channel4), .ALMOSTFULL(), .DO(DATA_OUT [31:24]), .EMPTY(), .FULL(),
//			.DI(data_channel4[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(wren_channel4), .RST(RST)
//		);

endmodule
