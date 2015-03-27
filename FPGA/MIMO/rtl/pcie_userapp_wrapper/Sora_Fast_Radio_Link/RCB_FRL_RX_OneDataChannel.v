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
// Create Date:    15:57:22 08/15/2011 
// Design Name: 
// Module Name:    RCB_FRL_RX_OneDataChannel 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//				packet header is 0xF508, packet size is 29 bytes on each channel, no error check
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module RCB_FRL_RX_OneDataChannel(
		input	CLKDIV, 
		input	RST,
		input [7:0]	DATA_IN,
//		output	data_valid,
//		output [7:0]	data_out
		output reg	data_valid,
		output reg [7:0]	data_out
   );		

//	reg [15:0] input_reg1;
//	wire [7:0] input_reg1_wire, input_reg1_wire_inv;
//	
//	assign input_reg1_wire = DATA_IN;
		
//	RCB_FRL_BYTE_Alignment RCB_FRL_BYTE_Alignment_inst (
////		.data_in(input_reg1_wire),
//		.data_in(DATA_IN),
//		.clk(CLKDIV), 
//		.enable(~RST), 
//		.data_out(data_out), 
//		.data_valid(data_valid)
//	);

	reg [1:0] frame_decap_state;
	localparam IDLE		= 2'b00;
	localparam HEADER		= 2'b01;
	localparam DATA_OUT	= 2'b10;

	reg [7:0] data_reg;
	reg [4:0] counter;
	always@(posedge CLKDIV) data_reg[7:0] <= DATA_IN[7:0];

	always@(posedge CLKDIV) begin
		if (RST) begin
			counter[4:0]		<= 5'h00;
			data_valid			<= 1'b0;
			data_out[7:0]		<= 8'h00;
			frame_decap_state <= IDLE;
		end else begin
			case (frame_decap_state)
				IDLE: begin
					counter[4:0]	<= 5'h00;
					data_valid		<= 1'b0;
					data_out[7:0]	<= 8'h00;
					
					if ( data_reg[7:0] == 8'hF5 )	// frame detected
						frame_decap_state <= HEADER;
					else
						frame_decap_state <= IDLE;					
				end
				
				HEADER: begin
					counter[4:0]	<= 5'h00;
					data_valid		<= 1'b0;
					data_out[7:0]	<= 8'h00;
					
					if ( data_reg[7:0] == 8'h1D )	// frame detected
//					if ( data_reg[7:0] == 8'h08 )	// frame detected

						frame_decap_state <= DATA_OUT;
					else if ( data_reg[7:0] == 8'hF5 )
						frame_decap_state <= HEADER;	
					else
						frame_decap_state <= IDLE;	
				end
				
				DATA_OUT: begin
					counter[4:0]	<= counter[4:0] + 5'h01;
					data_valid		<= 1'b1;
					data_out[7:0]	<= data_reg[7:0];
					
					if (counter >= 5'h1C)
						frame_decap_state <= IDLE;
					else
						frame_decap_state <= DATA_OUT;
				end
				
				default: begin
					counter[4:0]	<= 5'h00;
					data_valid		<= 1'b0;
					data_out[7:0]	<= 8'h00;
					frame_decap_state <= IDLE;					
				end
				
			endcase
		end
	end

endmodule
