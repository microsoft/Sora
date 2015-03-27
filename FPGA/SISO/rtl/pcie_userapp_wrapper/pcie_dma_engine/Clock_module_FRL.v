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
// Company: Microsoft Research Asia
// Engineer: Jiansong Zhang
// 
// Create Date:    12:18:33 05/14/2010
// Design Name: 
// Module Name:    Clock_module_FRL
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description: generate clocks from 200MHz clock input. 
//					 If Sora_FRL is used, only 133MHz for DDR2 module is needed
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module Clock_module_FRL(
    input   clk200,
	 input   rst,
    output  clk133,
	 output [4:0] locked_debug,
	 output  unlocked_err
    );

wire locked1, locked_pll;
wire clk176, clk200_pll;
assign unlocked_err = (~locked1) & (~locked_pll);

assign locked_debug = {1'b0, 1'b0, 1'b0, locked1, locked_pll};

PLL_200MI_200MO PLL_200MI_200MO_inst(
   .CLKIN1_IN(clk200),
	.RST_IN(rst),
	.CLKOUT0_OUT(clk200_pll),
	.LOCKED_OUT(locked_pll)
);

DCM_200MI_133MO_176MO DCM_200MI_133MO_176MO_inst(
    .CLKIN_IN(clk200_pll),
    .CLKDV_OUT(clk133),
	 .CLKFX_OUT(clk176),
	 .RST_IN(rst),
	 .LOCKED_OUT(locked1),
	 .CLK0_OUT()
);

endmodule
