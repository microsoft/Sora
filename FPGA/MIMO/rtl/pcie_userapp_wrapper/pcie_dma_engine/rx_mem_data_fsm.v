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

//////////////////////////////////////////////////////////////////////////////////
// Company: Microsoft Research Asia
// Engineer: Jiansong Zhang
// 
// Create Date:    21:39:39 06/01/2009 
// Design Name: 
// Module Name:    rx_mem_data_fsm 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: Receive Memory Data State Machine module. This module takes the 
// data from the width conversion fifo (data_trn_mem_fifo) and send it into the
// dma_ddr2_if block
// that passes the data off to the MIG memory controller to write to the DDR2 
// memory. 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
// modified by Jiansong Zhang
//    add TX descriptor handling here ---------------- done
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps


module rx_mem_data_fsm(
	input  wire         clk,
	input  wire         rst,
	//interface to dma_ddr2_if block
	output reg  [127:0] ingress_data,
//	output reg    [1:0] ingress_fifo_ctrl,   //bit 1 = unused    bit 0 = write_en
	output reg			ingress_fifo_wren,
//	input  wire   [1:0] ingress_fifo_status, //bit 1 = full      bit 0 = almostfull
	output reg    [2:0] ingress_xfer_size,
	output reg   [27:6] ingress_start_addr,
	output reg          ingress_data_req,
	input  wire         ingress_data_ack,
	//interface to xfer_trn_mem_fifo
	input  wire         isDes_fifo,           /// Jiansong: added for TX des
	input  wire  [27:6] mem_dest_addr_fifo,
	input  wire  [10:0] mem_dma_size_fifo,
	input  wire         mem_dma_start,//start signal not used-monitor empty instead
	input  wire         mem_trn_fifo_empty,  
	output reg          mem_trn_fifo_rden,   
	//interface to data_trn_mem_fifo
	input  wire [127:0] data_fifo_data,
	output reg          data_fifo_cntrl,
	input  wire         data_fifo_status,
	///Jiansong: interface to dma control wrapper
	output reg          new_des_one,                 /// Jiansong: is one cycle ok?
	output wire        [31:0]  SourceAddr_L,
	output wire        [31:0]  SourceAddr_H,
	output wire        [31:0]  DestAddr,
	output wire        [23:0]  FrameSize,
	output wire        [7:0]   FrameControl
//	output reg			 [4:0]	state			/// liuchang: for debug
);

reg   [4:0]   state;
reg   [9:0]   cnt;
//reg [1:0] ingress_fifo_ctrl_pipe = 2'b00;
reg		ingress_fifo_wren_pipe;

/// liuchang
reg	[10:0] mem_dma_size_fifo_r;
reg	[27:6] mem_dest_addr_fifo_r;

/// Jiansong
reg [127:0] TX_des_1;
reg [127:0] TX_des_2;

/// Jiansong: parse TX descriptor
assign SourceAddr_L[31:0] = TX_des_1[95:64];
assign SourceAddr_H[31:0] = TX_des_1[127:96];
assign DestAddr[31:0]     = TX_des_2[31:0];
assign FrameSize[23:0]    = TX_des_2[87:64];
assign FrameControl[7:0]  = TX_des_2[95:88];
/// Jiansong: check own bit?


reg   rst_reg;
always@(posedge clk) rst_reg <= rst;

// Jiansong: data fifo empty pipeline
reg data_fifo_status_r;
always@(posedge clk) data_fifo_status_r <= data_fifo_status;

localparam IDLE   = 5'b00000;
localparam WREQ   = 5'b00001;
localparam WDATA2 = 5'b00010;
localparam WDONE  = 5'b00011;
localparam MREQ   = 5'b00100;
localparam MREQ2  = 5'b00101;
localparam MREQ_WAIT  = 5'b00110;			/// liuchang

/// Jiansong:
localparam PARSE_TX_DES  = 5'b01000;
localparam READ_TX_DES_1 = 5'b10000;
localparam READ_TX_DES_2 = 5'b11000;

//This state machine block waits for the xfer_trn_mem_fifo to go non-empty.
//It then performs the following steps:
//    1.  read the transfer size and destination 
//        information out of the xfer_trn_mem_fifo
//    2.  encode the transfer size info from DWORDS to the encoding used by
//        dma_ddr2_if block
//    3.  transfer the correct amount of data from the data_trn_mem_fifo to
//        the dma_ddr2_if (ingress data fifo)
always @ (posedge clk)
begin
  if(rst_reg)
  begin
    state              <= IDLE;
    ingress_xfer_size  <= 3'b000;
    ingress_start_addr <= 22'h000000;
    ingress_data_req   <= 1'b0;
    data_fifo_cntrl    <= 1'b0; 
    mem_trn_fifo_rden  <= 1'b0;
    cnt                <= 10'h000;
	 new_des_one        <= 1'b0;
	 TX_des_1           <= 128'h00000000_00000000_00000000_00000000;
	 TX_des_2           <= 128'h00000000_00000000_00000000_00000000;
	 mem_dma_size_fifo_r 			<= 11'h000;			/// liuchang
	 mem_dest_addr_fifo_r[27:6]	<= 22'h00_0000;			/// liuchang
  end
  else
  begin
    case(state)
      IDLE:   begin
		          new_des_one <= 1'b0;
                //wait for non-empty case and assert the read enable if so
                if(~mem_trn_fifo_empty)begin
                   mem_trn_fifo_rden <= 1'b1;
                   ingress_data_req   <= 1'b0;
                   state <= MREQ;
                end else begin
                  state  <= IDLE;
                end
              end
      MREQ:  begin //deassert the read enable
               mem_trn_fifo_rden <= 1'b0;
//               state <= MREQ2;												/// liuchang
					state <= MREQ_WAIT;											/// liuchang
             end
		MREQ_WAIT: begin															/// liuchang
					mem_dma_size_fifo_r 			<= mem_dma_size_fifo;
					mem_dest_addr_fifo_r[27:6]	<= mem_dest_addr_fifo[27:6];
					state <= MREQ2;
				 end
      MREQ2:begin
		         if(isDes_fifo) begin            /// Jiansong: parse TX des
					   // check whether TX descriptor data arrived in
//						if (~data_fifo_status)begin
						if (~data_fifo_status_r)begin
					      state <= PARSE_TX_DES;
						   data_fifo_cntrl <= 1'b1;        /// read enable the 1st cycle
						end else begin
						   state <= MREQ2;
							data_fifo_cntrl <= 1'b0;
						end
					end else begin
                     state <= WREQ;
                     //encode the transfer size information for the dma_ddr2_if
                     //also load a counter with the number of 128-bit (16 byte)
                     //transfers it will require to fullfill the total data
							
							/// liuchang
							if(mem_dma_size_fifo_r[10]) begin
								ingress_xfer_size <= 3'b110; 				// 4k byte
								cnt               <= 10'h100;
								mem_dma_size_fifo_r  		<= mem_dma_size_fifo_r - 11'b10000000000;
								mem_dest_addr_fifo_r[27:6] <= mem_dest_addr_fifo_r[27:6] + 7'b1000000;
							end else if(mem_dma_size_fifo_r[9]) begin	// 2k
								ingress_xfer_size  <= 3'b101;
								cnt                <= 10'h080;
								mem_dma_size_fifo_r <= mem_dma_size_fifo_r - 11'b01000000000;
								mem_dest_addr_fifo_r[27:6] <= mem_dest_addr_fifo_r[27:6] + 7'b0100000;
							end else if(mem_dma_size_fifo_r[8]) begin	// 1k
								ingress_xfer_size  <= 3'b100;
								cnt                <= 10'h040;	
								mem_dma_size_fifo_r <= mem_dma_size_fifo_r - 11'b00100000000;
								mem_dest_addr_fifo_r[27:6] <= mem_dest_addr_fifo_r[27:6] + 7'b0010000;
							end else if(mem_dma_size_fifo_r[7]) begin	// 512
								ingress_xfer_size  <= 3'b011;
								cnt                <= 10'h020;	
								mem_dma_size_fifo_r <= mem_dma_size_fifo_r - 11'b00010000000;
								mem_dest_addr_fifo_r[27:6] <= mem_dest_addr_fifo_r[27:6] + 7'b0001000;
							end else if(mem_dma_size_fifo_r[6]) begin	// 256
								ingress_xfer_size  <= 3'b010;
								cnt                <= 10'h010;	
								mem_dma_size_fifo_r <= mem_dma_size_fifo_r - 11'b00001000000;
								mem_dest_addr_fifo_r[27:6] <= mem_dest_addr_fifo_r[27:6] + 7'b0000100;
							end else if(mem_dma_size_fifo_r[5]) begin	// 128
								ingress_xfer_size  <= 3'b001;
								cnt                <= 10'h008;									
								mem_dma_size_fifo_r <= mem_dma_size_fifo_r - 11'b00000100000;
								mem_dest_addr_fifo_r[27:6] <= mem_dest_addr_fifo_r[27:6] + 7'b0000010;
							end else if(mem_dma_size_fifo_r[4]) begin	// 64 byte
								ingress_xfer_size  <= 3'b000;
								cnt                <= 10'h004;
								mem_dma_size_fifo_r <= mem_dma_size_fifo_r - 11'b00000010000;
								mem_dest_addr_fifo_r[27:6] <= mem_dest_addr_fifo_r[27:6] + 7'b0000001;
							end
							
							/// liuchang
							/*
                     case(mem_dma_size_fifo)
                       11'b00000010000:  begin //64 byte
                                           ingress_xfer_size  <= 3'b000;
                                           //64 bytes / 16 byte/xfer = 4 xfers
                                           cnt                <= 10'h004;
                                         end  
                       11'b00000100000:  begin //128
                                           ingress_xfer_size  <= 3'b001;
                                           cnt                <= 10'h008;
                                         end  
                       11'b00001000000:  begin //256
                                           ingress_xfer_size  <= 3'b010;
                                           cnt                <= 10'h010;
                                         end  
                       11'b00010000000:  begin //512
                                           ingress_xfer_size  <= 3'b011;
                                           cnt                <= 10'h020;
                                         end  
                       11'b00100000000:  begin //1k
                                           ingress_xfer_size  <= 3'b100;
                                           cnt                <= 10'h040;
                                         end  
                       11'b01000000000:  begin //2k
                                           ingress_xfer_size  <= 3'b101;
                                           cnt                <= 10'h080;
                                         end  
                       11'b10000000000:  begin //4k
                                           ingress_xfer_size  <= 3'b110;
                                           cnt                <= 10'h100;
                                        end 
                     endcase   
							*/
                     ingress_start_addr[27:6] <= mem_dest_addr_fifo_r[27:6];
                     ingress_data_req   <= 1'b1;//request access to ingress fifo
                 end
				  end
				  
	   /// Jiansong: parse TX des
		PARSE_TX_DES:  begin
		                  state <= READ_TX_DES_1;
							   data_fifo_cntrl <= 1'b1;    /// read enable the 2nd cycle
		               end
		READ_TX_DES_1: begin
		                  state <= READ_TX_DES_2;
								data_fifo_cntrl <= 1'b0;
								/// read data, 1st cycle
								TX_des_2[127:0] <= data_fifo_data[127:0];
								new_des_one <= 1'b0;
		               end
		READ_TX_DES_2: begin
		                  state <= IDLE;                    /// Jiansong: possible timing problem here
								/// read data, 2nd cycle
								TX_des_2[127:0] <= data_fifo_data[127:0];
								TX_des_1[127:0] <= TX_des_2[127:0];
								new_des_one <= 1'b1;
		               end
				  
      WREQ:   begin          /// Jiansong: data should be already in data_trn_mem_fifo
                if(ingress_data_ack) begin//wait for a grant from the dma_ddr2_if
                  state              <= WDATA2;
                  ingress_data_req   <= 1'b0;
                  data_fifo_cntrl    <= 1'b1; 
                end else begin
                  state <= WREQ;
                end
              end
      WDATA2: begin
                //keep data_fifo_cntrl asserted until cnt is 1 - then deassert
                //and finish
                if(cnt == 10'h001)begin
                  state <= WDONE;
                  data_fifo_cntrl    <= 1'b0;
                end else begin
                  cnt                <= cnt - 1'b1;                 
                  data_fifo_cntrl    <= 1'b1;
                  state <= WDATA2;
                end
              end

      WDONE:  begin
//						state              <= IDLE;						/// liuchang
                if(mem_dma_size_fifo_r[10:4] == 7'h00)			/// liuchang
						state              <= IDLE;
					 else
						state					 <= MREQ2;
              end  
      default:begin
                state              <= IDLE;
                ingress_xfer_size  <= 3'b000;
                ingress_start_addr <= 22'h000000;
                ingress_data_req   <= 1'b0;
                data_fifo_cntrl    <= 1'b0; 
                mem_trn_fifo_rden  <= 1'b0;
                cnt                <= 10'h000;
					 mem_dma_size_fifo_r				<= 11'h000;						/// liuchang
					 mem_dest_addr_fifo_r[27:6]	<= 22'h00_0000;				/// liuchang
              end
    endcase
  end
end

/// Jiansong: timing ok? all driven by the same clk
//data_fifo_cntrl is used as both a read enable to the data_trn_mem_fifo and
//a write enable to the ingress fifo (in dma_ddr2_if).  The write enable
//needs to be pipelined by two clocks so that it is synched with the 
//ingress_data (normally it would only need one pipeline register but since
//ingress_data is pipelined, it requires two.
always@(posedge clk) begin    // Jiansong: add logic to prevent TX des write into DDR
//      ingress_fifo_ctrl_pipe[1:0] <=  {1'b0,(data_fifo_cntrl&(~isDes_fifo))};//1st pipeline
//      ingress_fifo_ctrl[1:0] <= ingress_fifo_ctrl_pipe[1:0]; //2nd pipeline
		ingress_fifo_wren_pipe	<= data_fifo_cntrl&(~isDes_fifo); //1st pipeline
		ingress_fifo_wren			<= ingress_fifo_wren_pipe; //2nd pipeline
		ingress_data[127:0]		<= data_fifo_data[127:0]; //pipelined data
end

endmodule
