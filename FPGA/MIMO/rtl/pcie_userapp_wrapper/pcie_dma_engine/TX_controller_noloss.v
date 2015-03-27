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
// Create Date:    17:44:53 09/03/2012 
// Design Name: 
// Module Name:    TX_controller 
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
module TX_controller_noloss(
	 input			clk,
	 input			rst,
	 
	 input [4:0]	DDR2MaxBurstSize,
	 // triggers
	 input			TX_Start_one,
	 input [31:0]	TX_Addr,
	 input [31:0]	TX_Size,
	 /// connection to dma_ddr2_if
	 output reg TX_data_req,
	 input      TX_data_ack,
	 output reg [27:6] TX_Start_addr,
	 output reg [2:0]  TX_xfer_size
);

	 //state machine state definitions for TX_state_sm0
	 localparam IDLE_TSM0 = 3'b000;
	 localparam TX_1      = 3'b001;
	 localparam TX_2      = 3'b010;
	 localparam TX_3      = 3'b011;
	 localparam TX_4      = 3'b100;
	 
	 //state machine state definitions for TX_state_sm1
	 localparam IDLE_TSM1    = 4'h0;
	 localparam CALC_NEXT_TX = 4'h1;
	 localparam TX_CALC_4KB  = 4'h2;
	 localparam TX_CALC_2KB  = 4'h3;
	 localparam TX_CALC_1KB  = 4'h4;
	 localparam TX_CALC_512B = 4'h5;
	 localparam TX_CALC_256B = 4'h6;
	 localparam TX_CALC_128B = 4'h7;
	 localparam TX_CALC_64B  = 4'h8;
	 localparam WAIT_TX_CALC = 4'h9;

	/// Jiansong: TX registers
	reg TX_calc_next;           // calculate next tx addr and tx size, signal for TX SM 1
	reg next_TX_ready;          // ready to send out next tx request, signal for TX SM 0
	reg [31:0] TX_addr_now;
	reg [31:0] TX_size_now;
	reg [2:0] TX_state_0;
	reg [3:0] TX_state_1;


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	////  Jiansong:                                                 //////
	////  Logic for generating read DDR2 requests, according to     //////
	////  TX request from host. A TX request is to transmit a block //////
	////  of data to radio module. The size of a block could be as  //////
	////  large as several mega-bytes. In the following logic, a    //////
	////  block will be divided to a series of small blocks with    //////
	////  size up to 4KB.                                           //////
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	reg	TX_Start_signal;
	reg	TX_Start_ack;
	
	always@(posedge clk) begin
		if (rst)
			TX_Start_signal	<= 1'b0;
		else if (TX_Start_one)
			TX_Start_signal	<= 1'b1;
		else if (TX_Start_ack)
			TX_Start_signal	<= 1'b0;
		else
			TX_Start_signal	<= TX_Start_signal;			
	end

	/// TX state machine 0 
	/// generate output TX_data_req, and TX_calc_next for TX_state_1
	always@(posedge clk)begin
		if(rst)begin
			TX_calc_next	<= 1'b0;
			TX_data_req		<= 1'b0;
			TX_Start_ack	<= 1'b0;
			TX_state_0		<= IDLE_TSM0;
		end else begin
			case(TX_state_0)
				IDLE_TSM0: begin
					TX_calc_next	<= 1'b0;
					TX_data_req		<= 1'b0;
					
					// next_TX_ready will remain high after the last TX request, but it doesn't matter
					if (next_TX_ready) begin
						TX_Start_ack	<= 1'b0;
						TX_state_0		<= TX_1;
					end else if (TX_Start_signal) begin
						TX_Start_ack	<= 1'b1;
						TX_state_0		<= TX_1;
					end else begin
						TX_Start_ack	<= 1'b0;
						TX_state_0		<= IDLE_TSM0;
					end
					
				end
				TX_1: begin // this state is used to set TX_xfer_size and TX_Start_addr output 
					TX_calc_next	<= 1'b1;
					TX_data_req		<= 1'b0; 
					TX_state_0		<= TX_2;
				end
				TX_2: begin // calc TX_size_next and TX_addr_next, send out TX request to DMA_DDR_IF
					TX_calc_next	<= 1'b0;
					TX_data_req		<= 1'b1;
					TX_state_0		<= TX_3;
				end
				TX_3: begin
					TX_calc_next	<= 1'b0;
					if(TX_data_ack)begin
						TX_data_req	<= 1'b0;
						TX_state_0	<= TX_4;				
					end else begin
						TX_data_req	<= 1'b1;
						TX_state_0	<= TX_3;
					end
				end
				TX_4: begin // wait for next_TX_ready
					TX_calc_next	<= 1'b0;
					TX_data_req		<= 1'b0;
					TX_state_0		<= IDLE_TSM0;					
				end
				default:begin
					TX_calc_next	<= 1'b0;
					TX_data_req		<= 1'b0;
					TX_state_0		<= IDLE_TSM0;
				end
			endcase
		end
	end


	// set output: TX_xfer_size and TX_Start_addr[27:6]
	always@(posedge clk) TX_Start_addr[6] <= 1'b0;
	always@(posedge clk)begin
		if(rst)begin
			TX_xfer_size        <= 3'b000;
			TX_Start_addr[27:7] <= 21'h00_0000; 
			
		end else if (TX_state_0 == TX_1)begin
			TX_Start_addr[27:7] <= TX_addr_now[27:7];
			
			if(TX_size_now[27:10] != 0)                   // 1KB
				TX_xfer_size     <= 3'b100;
			else if(TX_size_now[9] != 0)                    // 512B
				TX_xfer_size     <= 3'b011;
			else if(TX_size_now[8] != 0)                    // 256B
				TX_xfer_size     <= 3'b010;
			else if(TX_size_now[7] != 0)                    // 128B
				TX_xfer_size     <= 3'b001;
			else                                            // default to 128B
				TX_xfer_size     <= 3'b001;
						
		end else begin
			TX_xfer_size        <= TX_xfer_size;
			TX_Start_addr[27:7] <= TX_Start_addr[27:7];
		end
	end

	/// TX state machine 1 
	/// calculate next size and address
	always@(posedge clk)begin
		if(rst)begin
			TX_addr_now[31:0]	<= 32'h0000_0000;
			TX_size_now[31:0]	<= 32'h0000_0000;
			next_TX_ready		<= 1'b0;
			TX_state_1			<= IDLE_TSM1;
		end else begin
				case(TX_state_1)
					IDLE_TSM1:begin
						next_TX_ready		<= next_TX_ready;
						if(TX_Start_signal & (~next_TX_ready))begin
							TX_addr_now[31:0] <= TX_Addr[31:0];
							TX_size_now[31:0] <= TX_Size[31:0];
						end else begin
							TX_addr_now[31:0] <= TX_addr_now[31:0];
							TX_size_now[31:0] <= TX_size_now[31:0];
						end
						if(TX_calc_next)
							TX_state_1 <= CALC_NEXT_TX;
						else
							TX_state_1 <= IDLE_TSM1;
					end
					CALC_NEXT_TX:begin
						TX_addr_now[31:0] <= TX_addr_now[31:0];
						TX_size_now[31:0] <= TX_size_now[31:0];
						next_TX_ready		<= 1'b0;
						 
						if (TX_size_now[27:10] != 0)
							TX_state_1 <= TX_CALC_1KB;
						else if (TX_size_now[9] != 0)
							TX_state_1 <= TX_CALC_512B;
						else if (TX_size_now[8] != 0)
							TX_state_1 <= TX_CALC_256B;
						else if (TX_size_now[7] != 0)
							TX_state_1 <= TX_CALC_128B;
						else
							TX_state_1 <= TX_CALC_128B;
						 
					end
					TX_CALC_1KB:begin
						TX_addr_now[31:0]		<= TX_addr_now[31:0] + 32'h0000_0400;
						TX_size_now[31:10]	<= TX_size_now[31:10] - 1'b1;
						next_TX_ready		<= 1'b0;
						TX_state_1				<= WAIT_TX_CALC;
					end
					TX_CALC_512B:begin
						TX_addr_now[31:0]	<= TX_addr_now[31:0] + 32'h0000_0200;
						TX_size_now[31:9]	<= TX_size_now[31:9] - 1'b1;
						next_TX_ready		<= 1'b0;
						TX_state_1			<= WAIT_TX_CALC;
					end
					TX_CALC_256B:begin
						TX_addr_now[31:0]	<= TX_addr_now[31:0] + 32'h0000_0100;
						TX_size_now[31:8]	<= TX_size_now[31:8] - 1'b1;
						next_TX_ready		<= 1'b0;
						TX_state_1			<= WAIT_TX_CALC;
					end
					TX_CALC_128B:begin
						TX_addr_now[31:0]	<= TX_addr_now[31:0] + 32'h0000_0080;
						TX_size_now[31:7]	<= TX_size_now[31:7] - 1'b1;
						next_TX_ready		<= 1'b0;
						TX_state_1			<= WAIT_TX_CALC;
					end
					
					WAIT_TX_CALC:begin
						TX_addr_now[31:0] <= TX_addr_now[31:0];
						TX_size_now[31:0] <= TX_size_now[31:0];
						if (TX_size_now[31:7] != 0)
							next_TX_ready	<= 1'b1;
						else
							next_TX_ready	<= 1'b0;
						TX_state_1			<= IDLE_TSM1;
					end
					
					default:begin
						TX_addr_now[31:0]	<= 32'h0000_0000;
						TX_size_now[31:0]	<= 32'h0000_0000;
						TX_state_1			<= IDLE_TSM1;
					end
				endcase
		end
	end

//	/// update TX_addr_now and TX_size_now
//	always@(posedge clk)begin
//		if(rst)begin
//			TX_addr_now[31:0] <= 32'h0000_0000;
//			TX_size_now[31:0] <= 32'h0000_0000;
//		end else if (TX_Start_one)begin
//			TX_addr_now[31:0] <= TX_Addr[31:0];
//			TX_size_now[31:0] <= TX_Size[31:0];
//		end else if (update_TX_now)begin
//			TX_addr_now[31:0] <= TX_addr_next[31:0];
//			TX_size_now[31:0] <= TX_size_next[31:0];
//		end else begin
//			TX_addr_now[31:0] <= TX_addr_now[31:0];
//			TX_size_now[31:0] <= TX_size_now[31:0];
//		end
//	end
//
//	/// generate next_TX_ready
//	always@(posedge clk)begin
//		if(rst)
//			next_TX_ready <= 1'b0;
//		else if (update_TX_now)
//			next_TX_ready <= 1'b1;
//		else if (TX_calc_next)
//			next_TX_ready <= 1'b0;
//		else
//			next_TX_ready <= next_TX_ready;
//	end

endmodule
