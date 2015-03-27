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

module RCB_FRL_fifo_MSG(ALMOSTEMPTY, ALMOSTFULL, DO, EMPTY, FULL, DI, RDCLK, RDEN, WRCLK, WREN, RST);

	output 	ALMOSTEMPTY, ALMOSTFULL, EMPTY, FULL;
	output 	[39:0] DO;
	input		[39:0] DI;
	input		RDCLK, RDEN, WRCLK, WREN, RST;
	
	wire [63:40] zero;
	
	FIFO36_72 #(
		.ALMOST_FULL_OFFSET(9'h080), // Sets almost full threshold
		.ALMOST_EMPTY_OFFSET(9'h080), // Sets the almost empty threshold
		.DO_REG(1), // Enable output register (0 or 1)
		// Must be 1 if EN_SYN = "FALSE"
		.EN_ECC_READ("FALSE"), // Enable ECC decoder, "TRUE" or "FALSE"
		.EN_ECC_WRITE("FALSE"), // Enable ECC encoder, "TRUE" or "FALSE"
		.EN_SYN("FALSE"), // Specifies FIFO as Asynchronous ("FALSE")
		// or Synchronous ("TRUE")
		.FIRST_WORD_FALL_THROUGH("FALSE") // Sets the FIFO FWFT to "TRUE" or "FALSE"
		) FIFO36_72_inst (
		.ALMOSTEMPTY(ALMOSTEMPTY), // 1-bit almost empty output flag
		.ALMOSTFULL(ALMOSTFULL), // 1-bit almost full output flag
		.DBITERR(), // 1-bit double bit error status output
		.DO({zero[63:40],DO[39:0]}), // 32-bit data output
		.DOP(), // 4-bit parity data output
		.ECCPARITY(), // 8-bit generated error correction parity
		.EMPTY(EMPTY), // 1-bit empty output flag
		.FULL(FULL), // 1-bit full output flag
		.RDCOUNT(), // 9-bit read count output
		.RDERR(), // 1-bit read error output
		.SBITERR(), // 1-bit single bit error status output
		.WRCOUNT(), // 9-bit write count output
		.WRERR(), // 1-bit write error
		.DI({24'h000,DI}), // 32-bit data input
		.DIP(), // 4-bit parity input
		.RDCLK(RDCLK), // 1-bit read clock input
		.RDEN(RDEN), // 1-bit read enable input
		.RST(RST), // 1-bit reset input
		.WRCLK(WRCLK), // 1-bit write clock input
		.WREN(WREN) // 1-bit write enable input
	);
	
endmodule
