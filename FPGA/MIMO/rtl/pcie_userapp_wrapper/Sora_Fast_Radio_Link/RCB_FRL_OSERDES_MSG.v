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

module RCB_FRL_OSERDES_MSG(OQ, clk, clkdiv, DI, OCE, SR);
	
	output OQ;
	input clk, clkdiv;
	input [7:0] DI;
	input OCE, SR;
	
	wire SHIFT1, SHIFT2;

	OSERDES OSERDES_inst1 (
		.OQ(OQ), // 1-bit data path output
		.SHIFTOUT1(), // 1-bit data expansion output
		.SHIFTOUT2(), // 1-bit data expansion output
		.TQ(), // 1-bit 3-state control output
		.CLK(clk), // 1-bit clock input
		.CLKDIV(clkdiv), // 1-bit divided clock input
		.D1(DI[7]), // 1-bit parallel data input
		.D2(DI[6]), // 1-bit parallel data input
		.D3(DI[5]), // 1-bit parallel data input
		.D4(DI[4]), // 1-bit parallel data input
		.D5(DI[3]), // 1-bit parallel data input
		.D6(DI[2]), // 1-bit parallel data input
		.OCE(OCE), // 1-bit clock enable input
		.REV(1'b0), // 1-bit reverse SR input
		.SHIFTIN1(SHIFT1), // 1-bit data expansion input
		.SHIFTIN2(SHIFT2), // 1-bit data expansion input
		.SR(SR), // 1-bit set/reset input
		.T1(), // 1-bit parallel 3-state input
		.T2(), // 1-bit parallel 3-state input
		.T3(), // 1-bit parallel 3-state input
		.T4(), // 1-bit parallel 3-state input
		.TCE(1'b1) // 1-bit 3-state signal clock enable input
	);
	defparam OSERDES_inst1.DATA_RATE_OQ = "DDR";
	defparam OSERDES_inst1.DATA_RATE_TQ = "DDR";
	defparam OSERDES_inst1.DATA_WIDTH = 8;
	defparam OSERDES_inst1.SERDES_MODE = "MASTER";
	defparam OSERDES_inst1.TRISTATE_WIDTH = 1;
	
	OSERDES OSERDES_inst2 (
		.OQ(), // 1-bit data path output
		.SHIFTOUT1(SHIFT1), // 1-bit data expansion output
		.SHIFTOUT2(SHIFT2), // 1-bit data expansion output
		.TQ(), // 1-bit 3-state control output
		.CLK(clk), // 1-bit clock input
		.CLKDIV(clkdiv), // 1-bit divided clock input
		.D1(), // 1-bit parallel data input
		.D2(), // 1-bit parallel data input
		.D3(DI[1]), // 1-bit parallel data input
		.D4(DI[0]), // 1-bit parallel data input
		.D5(), // 1-bit parallel data input
		.D6(), // 1-bit parallel data input
		.OCE(OCE), // 1-bit clock enable input
		.REV(1'b0), // 1-bit reverse SR input
		.SHIFTIN1(), // 1-bit data expansion input
		.SHIFTIN2(), // 1-bit data expansion input
		.SR(SR), // 1-bit set/reset input
		.T1(), // 1-bit parallel 3-state input
		.T2(), // 1-bit parallel 3-state input
		.T3(), // 1-bit parallel 3-state input
		.T4(), // 1-bit parallel 3-state input
		.TCE(1'b1) // 1-bit 3-state signal clock enable input
	);
	defparam OSERDES_inst2.DATA_RATE_OQ = "DDR";
	defparam OSERDES_inst2.DATA_RATE_TQ = "DDR";
	defparam OSERDES_inst2.DATA_WIDTH = 8;
	defparam OSERDES_inst2.SERDES_MODE = "SLAVE";
	defparam OSERDES_inst2.TRISTATE_WIDTH = 1;
	
endmodule
