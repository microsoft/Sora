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
// Create Date:    03:04:50 02/19/2009 
// Design Name: 
// Module Name:    RCB_FRL_LED_Clock 
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
module RCB_FRL_LED_Clock(Test_Clock_in, LED_Clock_out, RST);
    input Test_Clock_in;
    output LED_Clock_out;
    input RST;

	reg[9:0] count1;
	reg[9:0] count2;
	reg[9:0] count3;
	reg LED_Clock_out_reg;
	
	assign LED_Clock_out = LED_Clock_out_reg;

always @(posedge Test_Clock_in or posedge RST) begin
	if (RST) begin
		count1 <= 1'b0;
		count2 <= 1'b0;
		count3 <= 1'b0;
		LED_Clock_out_reg <= 1'b0;
		end
	else begin
		if (count3 < 448) begin
			if (count2 < 1000) begin
				if (count1 < 1000) 
					count1 <= count1 + 1'b1;
				else
					begin
						count1 <= 1'b0;
						count2 <= count2 + 1'b1;
					end
				end
			else
				begin
					count2 <= 1'b0;
					count3 <= count3 + 1'b1;
				end
			end
		else				
			begin
				count3 <= 1'b0;
				LED_Clock_out_reg <= ~LED_Clock_out_reg;
			end
		end
	end

endmodule

module RCB_FRL_LED_Clock_DIV(Test_Clock_in, LED_Clock_out, RST);
    input Test_Clock_in;
    output LED_Clock_out;
    input RST;

	reg[9:0] count1;
	reg[9:0] count2;
	reg[9:0] count3;
	reg LED_Clock_out_reg;
	
	assign LED_Clock_out = LED_Clock_out_reg;

always @(posedge Test_Clock_in or posedge RST) begin
	if (RST) begin
		count1 <= 1'b0;
		count2 <= 1'b0;
		count3 <= 1'b0;
		LED_Clock_out_reg <= 1'b0;
		end
	else begin
		if (count3 < 56) begin
			if (count2 < 1000) begin
				if (count1 < 1000) 
					count1 <= count1 + 1'b1;
				else
					begin
						count1 <= 1'b0;
						count2 <= count2 + 1'b1;
					end
				end
			else
				begin
					count2 <= 1'b0;
					count3 <= count3 + 1'b1;
				end
			end
		else				
			begin
				count3 <= 1'b0;
				LED_Clock_out_reg <= ~LED_Clock_out_reg;
			end
		end
	end

endmodule
