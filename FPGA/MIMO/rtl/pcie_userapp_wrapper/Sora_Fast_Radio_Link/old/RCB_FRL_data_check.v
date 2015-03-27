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
module RCB_FRL_data_check(CLK,RST,RDEN_DATA,RDEN_MSG,DATA,MSG,ERR0,ERR1,ERR2,ERR3,ERR_MSG);

input CLK,RST,RDEN_DATA,RDEN_MSG;
input [31:0] DATA;
input [39:0] MSG;

output reg [3:0] ERR0,ERR1,ERR2,ERR3,ERR_MSG;

reg [31:0] DATA_TEMP;
reg [7:0] MSG_TEMP;

always @ (posedge CLK)
begin
	if(RST)
	begin
		ERR0 <= 0; 
		ERR1 <= 0; 
		ERR2 <= 0; 
		ERR3 <= 0; 
		ERR_MSG <= 0; 
		DATA_TEMP <= 0;
		MSG_TEMP <= 0;
	end
	else if(RDEN_DATA)
	begin
		DATA_TEMP <= DATA;
		MSG_TEMP <= MSG;
		if(DATA_TEMP[7:0] + 1 != DATA[7:0] && DATA[7:0] != 0)
			ERR0 <= ERR0 + 1;
		if(DATA_TEMP[15:8] + 1 != DATA[15:8] && DATA[15:8] != 0)
			ERR1 <= ERR1 + 1;
		if(DATA_TEMP[23:16] + 1 != DATA[23:16] && DATA[23:16] != 0)
			ERR2 <= ERR2 + 1;
		if(DATA_TEMP[31:24] + 1 != DATA[31:24] && DATA[31:24] != 0)
			ERR3 <= ERR3 + 1;
		if(MSG[39:32] + 1 != MSG[31:24] || MSG[31:24] + 1 != MSG[23:16] || MSG[23:16] + 1 != MSG[15:8] || MSG[15:8] + 1 != MSG[7:0])
			ERR_MSG <= ERR_MSG + 1;
	end
end
endmodule
