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

module RCB_FRL_CRC_gen ( D, NewCRC);

	input [47:0] D;
	output [7:0] NewCRC;
	
	
	assign NewCRC[0] = D[46] ^ D[42] ^ D[41] ^ D[37] ^ D[36] ^ D[35] ^ D[34] ^ 
                D[33] ^ D[31] ^ D[30] ^ D[29] ^ D[27] ^ D[26] ^ D[24] ^ 
                D[20] ^ D[18] ^ D[17] ^ D[16] ^ D[15] ^ D[14] ^ D[13] ^ 
                D[8] ^ D[7] ^ D[6] ^ D[3] ^ D[1] ^ D[0];
    assign NewCRC[1] = D[47] ^ D[43] ^ D[42] ^ D[38] ^ D[37] ^ D[36] ^ D[35] ^ 
                D[34] ^ D[32] ^ D[31] ^ D[30] ^ D[28] ^ D[27] ^ D[25] ^ 
                D[21] ^ D[19] ^ D[18] ^ D[17] ^ D[16] ^ D[15] ^ D[14] ^ 
                D[9] ^ D[8] ^ D[7] ^ D[4] ^ D[2] ^ D[1];
    assign NewCRC[2] = D[46] ^ D[44] ^ D[43] ^ D[42] ^ D[41] ^ D[39] ^ D[38] ^ 
                D[34] ^ D[32] ^ D[30] ^ D[28] ^ D[27] ^ D[24] ^ D[22] ^ 
                D[19] ^ D[14] ^ D[13] ^ D[10] ^ D[9] ^ D[7] ^ D[6] ^ 
                D[5] ^ D[2] ^ D[1] ^ D[0];
    assign NewCRC[3] = D[47] ^ D[45] ^ D[44] ^ D[43] ^ D[42] ^ D[40] ^ D[39] ^ 
                D[35] ^ D[33] ^ D[31] ^ D[29] ^ D[28] ^ D[25] ^ D[23] ^ 
                D[20] ^ D[15] ^ D[14] ^ D[11] ^ D[10] ^ D[8] ^ D[7] ^ 
                D[6] ^ D[3] ^ D[2] ^ D[1];
    assign NewCRC[4] = D[45] ^ D[44] ^ D[43] ^ D[42] ^ D[40] ^ D[37] ^ D[35] ^ 
                D[33] ^ D[32] ^ D[31] ^ D[27] ^ D[21] ^ D[20] ^ D[18] ^ 
                D[17] ^ D[14] ^ D[13] ^ D[12] ^ D[11] ^ D[9] ^ D[6] ^ 
                D[4] ^ D[2] ^ D[1] ^ D[0];
    assign NewCRC[5] = D[46] ^ D[45] ^ D[44] ^ D[43] ^ D[41] ^ D[38] ^ D[36] ^ 
                D[34] ^ D[33] ^ D[32] ^ D[28] ^ D[22] ^ D[21] ^ D[19] ^ 
                D[18] ^ D[15] ^ D[14] ^ D[13] ^ D[12] ^ D[10] ^ D[7] ^ 
                D[5] ^ D[3] ^ D[2] ^ D[1];
    assign NewCRC[6] = D[47] ^ D[45] ^ D[44] ^ D[41] ^ D[39] ^ D[36] ^ D[31] ^ 
                D[30] ^ D[27] ^ D[26] ^ D[24] ^ D[23] ^ D[22] ^ D[19] ^ 
                D[18] ^ D[17] ^ D[11] ^ D[7] ^ D[4] ^ D[2] ^ D[1] ^ 
                D[0];
    assign NewCRC[7] = D[45] ^ D[41] ^ D[40] ^ D[36] ^ D[35] ^ D[34] ^ D[33] ^ 
                D[32] ^ D[30] ^ D[29] ^ D[28] ^ D[26] ^ D[25] ^ D[23] ^ 
                D[19] ^ D[17] ^ D[16] ^ D[15] ^ D[14] ^ D[13] ^ D[12] ^ 
                D[7] ^ D[6] ^ D[5] ^ D[2] ^ D[0];
					 
endmodule
