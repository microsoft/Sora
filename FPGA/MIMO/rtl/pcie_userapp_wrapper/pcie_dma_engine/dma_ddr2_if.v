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
// Module Name:    tx_engine 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: This module is the logic that interfaces with the Xilinx MIG DDR2 
//          controller for the SODIMM on the ML555 and the PCIe DMA block.  
//          This module is responsible for crossing clock domains between the
//          two blocks.  There are three fifos in this module.  One for egress
//          data, one for ingress data, and one for passing address and
//          command to the MIG controller.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//  Jiansong:
//     TX flow control: 
//     (1) control signal: TX_fetch_next_4k, one cycle
//          it's the falling edge of egress_data_fifo almost full
//          egress_data_fifo almostfull is slightly less than half of the FIFO size
//     (2) TX has higher priority than transfer, or mrd > mwr
//      
//  modified by Jiansong Zhang:
//     (1)  ddr2 read/write scheduling, pending
//          divide address fifo into two fifos: rd and wr ctrl fifo
//          rd/wr ctrl fifo wren control
//          rd/wr scheduling in 200MHz domain
//     (2)  modify egress data fifo, use bram IP core instead -------------- done
//
//   modification on 2009-8-18, to slove the low throughput problem
//   (1) condense ddr_sm, in previous version, overhead could be 100% if data is 64 bytes
//   (2) modify pause_read_requests logic: if ingress_data_fifo is more than half full,
//       delay next dma read request
//
// Revision 1.00 -  add scheduling for four egress fifos and one ingress fifo
// 	notes:	(1) addr_fifo for each egress stream and the ingress stream
//					(2) scheduling between all the addr_fifos and the ToDDR_addr_fifo
//					(3) egress_addr data is passed from ToDDR_addr_fifo to FromDDR_addr_fifo
//					(4) one egress_data_fifo is dispatched to four ToFRL_data_fifos, according to FromDDR_addr_fifo 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

`include "Sora_config.v"

module dma_ddr2_if(
	input		dma_clk,
	input		ddr_clk,
	input		reset,

`ifdef sora_chipscope
	// chipscope
	inout [35:0]	CONTROL0,
	input				TX_Start_one,
`endif
	
	input				radio_clk,
	output [31:0]	ToFRL_data_fifo_rddata,
	output			ToFRL_data_fifo_pempty,
	input				ToFRL_data_fifo_rden,
	input				radio_2nd_clk,
	output [31:0]	ToFRL_2nd_data_fifo_rddata,
	output			ToFRL_2nd_data_fifo_pempty,
	input				ToFRL_2nd_data_fifo_rden,
`ifdef MIMO_4X4
	input				radio_3rd_clk,
	output [31:0]	ToFRL_3rd_data_fifo_rddata,
	output			ToFRL_3rd_data_fifo_pempty,
	input				ToFRL_3rd_data_fifo_rden,
	input				radio_4th_clk,
	output [31:0]	ToFRL_4th_data_fifo_rddata,
	output			ToFRL_4th_data_fifo_pempty,
	input				ToFRL_4th_data_fifo_rden,
`endif //MIMO_4X4
	//input	wire			full_rst,
	//input	wire			soft_rst,

	//DMA SIGNALS
	//`ifdef Sora_16B_TX
//	output wire  [31:0] egress_data,
	//`else
//	output wire  [15:0] egress_data,
	//`endif
	//input  wire   [1:0] egress_fifo_ctrl,     //bit 1 = reserved   bit 0 = read_en
//	input wire		egress_fifo_rden,
	//output wire   [2:0] egress_fifo_status,   //bit 2 = almost full
	//                                          //(needs to be empty before more 
	//                                          //egress requests complete)
	//                                          //bit 1 = empty   bit 0 = almostempty
//	output wire				egress_fifo_pempty,
	
	
	input [2:0]		egress_xfer_size,
	input [27:6]	egress_start_addr,
	input				egress_data_req,
	output reg		egress_data_ack,
	input [2:0]		egress_2nd_xfer_size,
	input [27:6]	egress_2nd_start_addr,
	input				egress_2nd_data_req,
	output reg		egress_2nd_data_ack,
`ifdef MIMO_4X4
	input [2:0]		egress_3rd_xfer_size,
	input [27:6]	egress_3rd_start_addr,
	input				egress_3rd_data_req,
	output reg		egress_3rd_data_ack,
	input [2:0]		egress_4th_xfer_size,
	input [27:6]	egress_4th_start_addr,
	input				egress_4th_data_req,
	output reg		egress_4th_data_ack,
`endif //MIMO_4X4	
	
	//ingress
	input  wire [127:0] ingress_data,
//	input  wire   [1:0] ingress_fifo_ctrl,   //bit 1 = reserved   bit 0 = write_en
	input				ingress_fifo_wren,
//	output wire   [1:0] ingress_fifo_status, //bit 1 = full       bit 0 = almostfull
	input  wire   [2:0] ingress_xfer_size,
	input  wire  [27:6] ingress_start_addr,
	input  wire         ingress_data_req,
	output reg          ingress_data_ack,
	//END OF DMA SIGNALS
			  
	/// Jiansong: error signals
	output wire         egress_overflow_one,
	output reg [31:0]   egress_wr_data_count,
			  
	//MEMORY CNTRLR SIGNALS
	output wire [127:0] m_wrdata,
	input  wire [127:0] m_rddata,
	output reg   [30:0] m_addr,
	output reg   [2:0]  m_cmd, //3'b000 == write command
										//3'b001 == read command
										//all others invalid - see MIG 1.72 documentaion for
										//more information
	output reg          m_data_wen,
	output reg          m_addr_wen,
	input  wire         m_data_valid,
	input  wire         m_af_afull,
	input  wire         m_wdf_afull,
	//END OF MEMORY CNTRLR SIGNALS
	output reg          pause_read_requests,
	
	output reg [31:0]		DebugDDREgressFIFOCnt,		// [31:16]	write cnt;	[15:0]	read cnt
	output reg [31:0]		DebugDDRFIFOFullCnt,
	output reg [31:0]		DebugDDRSignals,
	output reg [8:0]		DebugDDRSMs
	
//	output reg [31:0]   Debug18DDR1,
//	output reg [31:0]   Debug19DDR2
);

reg ddr_reset, ddr_reset1;

/// Jiansong: debug signals
//`ifdef Sora_16B_TX
//wire [10:0] egress_read_data_count;
//`else
//wire [11:0] egress_read_data_count;
//`endif
//wire [8:0]  egress_write_data_count;

///Jiansong: signal for egress/TX flow control
//reg egress_fifo_ready; 
//wire egress_pfull_falling;    // one cycle to detect falling edge of egress_pfull
//reg less_4k_pass;

////reg         addr_fifo_wren;
////reg  [31:0] addr_fifo_wrdata;
////wire        addr_fifo_almost_full;
reg         ingress_addr_fifo_wren;
reg  [31:0] ingress_addr_fifo_wrdata;
wire			ingress_addr_fifo_pfull;
reg         ingress_addr_fifo_rden;
wire [31:0] ingress_addr_fifo_rddata;
wire        ingress_addr_fifo_empty;
wire        ingress_addr_fifo_full;
//wire        ingress_addr_fifo_almost_full;
//wire        ingress_addr_fifo_almost_empty;

reg   [9:0] xfer_cnt;
reg         ingress_fifo_rden;
wire        ingress_fifo_pempty;
wire        ingress_fifo_empty;
wire        ingress_fifo_almostempty;
wire			ingress_fifo_full;
wire			ingress_fifo_pfull;

//reg         ToDDR_addr_fifo_wren;
//reg  [31:0] ToDDR_addr_fifo_wrdata;
//wire        ToDDR_addr_fifo_empty;
//wire        ToDDR_addr_fifo_full;
//reg         ToDDR_addr_fifo_ren;
//wire [31:0] ToDDR_addr_fifo_rddata;

reg         FromDDR_addr_fifo_wren;
reg  [31:0] FromDDR_addr_fifo_wrdata;
wire        FromDDR_addr_fifo_empty;
wire        FromDDR_addr_fifo_full;
reg         FromDDR_addr_fifo_rden;
wire [31:0] FromDDR_addr_fifo_rddata;

reg         egress_addr_fifo_wren;
reg  [31:0] egress_addr_fifo_wrdata;
wire        egress_addr_fifo_empty;
wire        egress_addr_fifo_full;
reg         egress_addr_fifo_rden;
wire [31:0] egress_addr_fifo_rddata;
//wire        egress_addr_fifo_almost_full;
//wire        egress_addr_fifo_almost_empty;
reg         egress_2nd_addr_fifo_wren;
reg  [31:0] egress_2nd_addr_fifo_wrdata;
wire        egress_2nd_addr_fifo_empty;
wire        egress_2nd_addr_fifo_full;
reg         egress_2nd_addr_fifo_rden;
wire [31:0] egress_2nd_addr_fifo_rddata;
`ifdef MIMO_4X4
reg         egress_3rd_addr_fifo_wren;
reg  [31:0] egress_3rd_addr_fifo_wrdata;
wire        egress_3rd_addr_fifo_empty;
wire        egress_3rd_addr_fifo_full;
reg         egress_3rd_addr_fifo_rden;
wire [31:0] egress_3rd_addr_fifo_rddata;
reg         egress_4th_addr_fifo_wren;
reg  [31:0] egress_4th_addr_fifo_wrdata;
wire        egress_4th_addr_fifo_empty;
wire        egress_4th_addr_fifo_full;
reg         egress_4th_addr_fifo_rden;
wire [31:0] egress_4th_addr_fifo_rddata;
`endif //MIMO_4X4


////reg         addr_fifo_ren;
////wire [31:0] addr_fifo_rddata;

wire				ToFRL_data_fifo_almost_full_this;
wire				ToFRL_data_fifo_pfull_this;

reg				ToFRL_data_fifo_wren;
wire [127:0]	ToFRL_data_fifo_wrdata;
wire				ToFRL_data_fifo_pfull;
wire				ToFRL_data_fifo_full;
wire				ToFRL_data_fifo_almost_full;
wire				ToFRL_data_fifo_empty;

reg				ToFRL_2nd_data_fifo_wren;
wire [127:0]	ToFRL_2nd_data_fifo_wrdata;
wire				ToFRL_2nd_data_fifo_pfull;
wire				ToFRL_2nd_data_fifo_full;
wire				ToFRL_2nd_data_fifo_almost_full;
wire				ToFRL_2nd_data_fifo_empty;
`ifdef MIMO_4X4
reg				ToFRL_3rd_data_fifo_wren;
wire [127:0]	ToFRL_3rd_data_fifo_wrdata;
wire				ToFRL_3rd_data_fifo_pfull;
wire				ToFRL_3rd_data_fifo_full;
wire				ToFRL_3rd_data_fifo_almost_full;
wire				ToFRL_3rd_data_fifo_empty;

reg				ToFRL_4th_data_fifo_wren;
wire [127:0]	ToFRL_4th_data_fifo_wrdata;
wire				ToFRL_4th_data_fifo_pfull;
wire				ToFRL_4th_data_fifo_full;
wire				ToFRL_4th_data_fifo_almost_full;
wire				ToFRL_4th_data_fifo_empty;
`endif //MIMO_4X4
//for the fifo status logic
reg				egress_fifo_rden;
reg				egress_fifo_wren;
wire [127:0]	egress_fifo_rddata;
reg [127:0]		m_rddata_reg;		// wrdata 
wire				egress_fifo_full;
wire				egress_fifo_empty;
wire				egress_fifo_pfull;
wire				egress_fifo_pempty;

////wire        egress_full_a;
////wire        egress_pfull_a;
////wire        egress_full_b;
////wire        egress_pfull_b;
////wire        egress_pempty_a;
////wire        egress_pempty_b;



//wire        ingress_fifo_pempty_a;
//wire        ingress_fifo_empty_a;
//wire        ingress_fifo_pempty_b;
//wire        ingress_fifo_empty_b;

////wire        addr_fifo_empty;
////wire        addr_fifo_full;

//wire  [1:0] ingress_fifo_status_a;
//wire  [1:0] ingress_fifo_status_b;


//ML505 specific wires/regs
//wire [8:0] wrcount, rdcount;
//reg [8:0] wrcount_gray_dma;
//reg [8:0] wrcount_gray_ddr, wrcount_gray_ddr1;
//wire [8:0] wrcount_ddr;
//reg [8:0] wrcount_ddr_reg;
//reg [8:0] rdcount_reg = 0;
////reg at_least_64B;

//reg [1:0]	dma_state;
////States for DMA state machine
//localparam IDLE  = 2'b00;
//localparam LOADI = 2'b01;
//localparam DONE  = 2'b10;
////localparam LOADE = 3'b001;
////localparam WAITE = 3'b100;        /// Jiansong: add one cycle to wait for the deassertion of egress request

reg [2:0]	token; // 000: egress; 001: egress2; 010: egress3; 011: egress4; 100: ingress
////States for rr scheduler
reg [3:0]	sche_state;
localparam IDLE	= 4'b0000;
localparam LOADE	= 4'b0001;
localparam SETE	= 4'b0010;
localparam LOADE2	= 4'b0011;
localparam SETE2	= 4'b0100;
localparam LOADE3	= 4'b0101;
localparam SETE3	= 4'b0110;
localparam LOADE4	= 4'b0111;
localparam SETE4	= 4'b1000;
localparam LOADI	= 4'b1001;
localparam SETI	= 4'b1010;

reg   [4:0] ddr_state;
//States for the DDR state machine
localparam NOOP  = 5'b00000;
////localparam LOAD0 = 5'b00001;
////localparam LOAD1 = 5'b00010;
//localparam E_LOAD0 = 5'b00001;
//localparam E_LOAD1 = 5'b00010;
//localparam IN_LOAD0 = 5'b01101;
//localparam IN_LOAD1 = 5'b01110;
localparam R1    = 5'b00011;
localparam R2    = 5'b00100;
//localparam W0    = 5'b00101;
localparam W1    = 5'b00110;
localparam W2    = 5'b00111;
localparam W3    = 5'b01000;
localparam W4    = 5'b01001;
localparam W5    = 5'b01010;
//localparam W6    = 5'b01011;
//localparam WAIT_FOR_ROOM = 5'b01100;

reg [1:0]	egress_state;
localparam E_IDLE 		= 2'b00;
localparam E_WAIT_ADDR	= 2'b01;
localparam E_PASS_START	= 2'b10;
localparam E_PASS 		= 2'b11;

///// Jiansong: debug register
//always@(posedge ddr_clk)begin
//   Debug18DDR1[7:0]   <= {3'b000,ddr_state[4:0]};
////   Debug18DDR1[11:8]  <= {3'b000,less_4k_pass};
////   Debug18DDR1[15:12] <= {3'b000,egress_fifo_ready};
//   Debug18DDR1[19:16] <= {3'b000,egress_fifo_empty};
//   Debug18DDR1[23:20] <= {3'b000,egress_addr_fifo_empty};
//   Debug18DDR1[27:24] <= {3'b000,ingress_addr_fifo_empty};
//   Debug18DDR1[31:28] <= {3'b000,ingress_data_req};
//end
//
//always@(posedge ddr_clk)begin
//   Debug19DDR2[3:0]   <= {3'b000,egress_addr_fifo_full};
////	Debug19DDR2[7:4]   <= {1'b0,egress_fifo_status[2:0]};
////	Debug19DDR2[7:4]   <= {3'b000,egress_fifo_pempty};
////`ifdef Sora_16B_TX
////	Debug19DDR2[18:8]  <= egress_read_data_count[10:0];
////`else
////	Debug19DDR2[19:8]  <= egress_read_data_count[11:0];
////`endif
////	Debug19DDR2[31:20] <= {3'b000,egress_write_data_count[8:0]};
//end

//synchronize the DMA reset signal to the DDR2 clock
always@(posedge ddr_clk)begin
   ddr_reset1 <= reset;
   ddr_reset <= ddr_reset1;
end


//create a single gating signal for the ingress path so that the transmit
//engine will stop executing read requests if the fifos are nearing full
//condition; this signal is fedback to the DMA CNTRL block
always@(posedge dma_clk)begin
////   pause_read_requests <= ingress_fifo_pfull | addr_fifo_almost_full;
//     pause_read_requests <= ingress_fifo_pfull | ingress_addr_fifo_almost_full;
     pause_read_requests <= (~ingress_fifo_pempty) | ingress_addr_fifo_pfull;		// either fifo more than half full
end

/// Jiansong: modifications
///          (1) separate addr_cntrl_fifo to ingress addr fifo and egress addr fifo
///          (2) scheduling at addr fifo rd side, or 200MHz clock domain
///          (3) egress always has higher priority
///////////////////////////////////////////////////////////////////////////////
// DMA state machine  
//
// Takes request from the dma and loads into the ADDRESS/CNTRL FIFO(64 deep)
// for the ddr state machine
// 
// The TX and RX engines provide a request signal (*_data_req), and encoded
// transfer size (*_xfer_size) and a starting address (*_start_addr).
// The bottom 6 bits of the start address are not needed since the
// address should always be aligned on a 64B boundary (which is the smallest
// incremental address that a completion packet may be returned on) 
///////////////////////////////////////////////////////////////////////////////
always@(posedge dma_clk) begin
	if (reset) begin
		egress_data_ack			<= 1'b0;
		egress_addr_fifo_wren	<= 1'b0;
		egress_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_data_ack) begin
		egress_data_ack			<= 1'b0;
		egress_addr_fifo_wren	<= 1'b0;
		egress_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_data_req & ~egress_addr_fifo_full) begin
		egress_data_ack			<= 1'b1;
		egress_addr_fifo_wren	<= 1'b1;
		egress_addr_fifo_wrdata	<= ({4'b0000,2'b00,egress_xfer_size[2:0],egress_start_addr[27:6],1'b1});//bit 0 == 1'b1 denotes read from ddr2
	end else begin
		egress_data_ack			<= 1'b0;
		egress_addr_fifo_wren	<= 1'b0;
		egress_addr_fifo_wrdata	<= 32'h00000000;
	end
end
always@(posedge dma_clk) begin
	if (reset) begin
		egress_2nd_data_ack				<= 1'b0;
		egress_2nd_addr_fifo_wren		<= 1'b0;
		egress_2nd_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_2nd_data_ack) begin
		egress_2nd_data_ack				<= 1'b0;
		egress_2nd_addr_fifo_wren		<= 1'b0;
		egress_2nd_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_2nd_data_req & ~egress_2nd_addr_fifo_full) begin
		egress_2nd_data_ack				<= 1'b1;
		egress_2nd_addr_fifo_wren		<= 1'b1;
		egress_2nd_addr_fifo_wrdata	<= ({4'b0000,2'b01,egress_2nd_xfer_size[2:0],egress_2nd_start_addr[27:6],1'b1});//bit 0 == 1'b1 denotes read from ddr2
	end else begin
		egress_2nd_data_ack				<= 1'b0;
		egress_2nd_addr_fifo_wren		<= 1'b0;
		egress_2nd_addr_fifo_wrdata	<= 32'h00000000;
	end
end
`ifdef MIMO_4X4
always@(posedge dma_clk) begin
	if (reset) begin
		egress_3rd_data_ack				<= 1'b0;
		egress_3rd_addr_fifo_wren		<= 1'b0;
		egress_3rd_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_3rd_data_ack) begin
		egress_3rd_data_ack				<= 1'b0;
		egress_3rd_addr_fifo_wren		<= 1'b0;
		egress_3rd_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_3rd_data_req & ~egress_3rd_addr_fifo_full) begin
		egress_3rd_data_ack				<= 1'b1;
		egress_3rd_addr_fifo_wren		<= 1'b1;
		egress_3rd_addr_fifo_wrdata	<= ({4'b0000,2'b10,egress_3rd_xfer_size[2:0],egress_3rd_start_addr[27:6],1'b1});//bit 0 == 1'b1 denotes read from ddr2
	end else begin
		egress_3rd_data_ack				<= 1'b0;
		egress_3rd_addr_fifo_wren		<= 1'b0;
		egress_3rd_addr_fifo_wrdata	<= 32'h00000000;
	end
end
always@(posedge dma_clk) begin
	if (reset) begin
		egress_4th_data_ack				<= 1'b0;
		egress_4th_addr_fifo_wren		<= 1'b0;
		egress_4th_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_4th_data_ack) begin
		egress_4th_data_ack				<= 1'b0;
		egress_4th_addr_fifo_wren		<= 1'b0;
		egress_4th_addr_fifo_wrdata	<= 32'h00000000;
	end else if (egress_4th_data_req & ~egress_4th_addr_fifo_full) begin
		egress_4th_data_ack				<= 1'b1;
		egress_4th_addr_fifo_wren		<= 1'b1;
		egress_4th_addr_fifo_wrdata	<= ({4'b0000,2'b11,egress_4th_xfer_size[2:0],egress_4th_start_addr[27:6],1'b1});//bit 0 == 1'b1 denotes read from ddr2
	end else begin
		egress_4th_data_ack				<= 1'b0;
		egress_4th_addr_fifo_wren		<= 1'b0;
		egress_4th_addr_fifo_wrdata	<= 32'h00000000;
	end
end
`endif //MIMO_4X4

always@(posedge dma_clk) begin
	if (reset) begin
		ingress_data_ack				<= 1'b0;
		ingress_addr_fifo_wren		<= 1'b0;
		ingress_addr_fifo_wrdata	<= 32'h00000000;
	end else if (ingress_data_ack) begin
		ingress_data_ack			<= 1'b0;
		ingress_addr_fifo_wren	<= 1'b0;
		ingress_addr_fifo_wrdata	<= 32'h00000000;
	end else if (ingress_data_req & ~ingress_addr_fifo_full & ~ingress_fifo_pfull) begin
		ingress_data_ack				<= 1'b1;
		ingress_addr_fifo_wren		<= 1'b1;
		ingress_addr_fifo_wrdata	<= ({6'b000000,ingress_xfer_size[2:0],ingress_start_addr[27:6],1'b0});//bit 0 == 1'b0 denotes write to ddr2
	end else begin
		ingress_data_ack				<= 1'b0;
		ingress_addr_fifo_wren		<= 1'b0;
		ingress_addr_fifo_wrdata	<= 32'h00000000;
	end
end

////ingress_addr_fifo
//always @ (posedge dma_clk) begin
//	if(reset)begin
//		ingress_addr_fifo_wren				<= 1'b0;
//		ingress_addr_fifo_wrdata[31:0]	<= 32'h00000000;
//		ingress_data_ack						<= 1'b0;
//		dma_state								<= IDLE;
//	end else begin
//		case(dma_state)
//			IDLE: begin
//					ingress_data_ack			<= 1'b0;
//					ingress_addr_fifo_wren	<= 1'b0;
//					if(ingress_data_req & 
//						~ingress_addr_fifo_full & 
//						~ingress_fifo_pfull) begin   /// Jiansong: if ingress_fifo_status is almostfull, block ingress data request
//                       //assert the ingress ack and load the ADDR/CNTRL fifo with the correct ddr2 write parameters
//							ingress_data_ack					<= 1'b1;
//							ingress_addr_fifo_wrdata[31:0]<= ({6'b000000,ingress_xfer_size[2:0],ingress_start_addr[27:6],1'b0});//bit 0 == 1'b0 denotes write to ddr2
//							ingress_addr_fifo_wren			<= 1'b1;
//							dma_state							<= LOADI;
//					end else begin
//						dma_state <= IDLE;
//					end
//			end
//			LOADI: begin
//					ingress_addr_fifo_wren	<= 1'b0;
//					ingress_data_ack			<= 1'b0;
//					dma_state					<= DONE;
//			end
//			DONE: begin
//					ingress_data_ack			<= 1'b0;
//					ingress_addr_fifo_wren	<= 1'b0;
//					dma_state					<= IDLE;
//			end
//			default: begin
//					ingress_data_ack			<= 1'b0;
//					ingress_addr_fifo_wren	<= 1'b0;
//					dma_state					<= IDLE;
//			end
//		endcase
//	end
//end

//always @ (posedge dma_clk) begin
//  if(reset)begin
//    dma_state        <= IDLE;
//    ingress_addr_fifo_wren   <= 1'b0;
//    ingress_addr_fifo_wrdata[31:0] <= 32'h00000000;
//	 egress_addr_fifo_wren   <= 1'b0;
//    egress_addr_fifo_wrdata[31:0] <= 32'h00000000;
//    egress_data_ack  <= 1'b0;
//    ingress_data_ack <= 1'b0;
//  end else begin
//    case(dma_state)
//      IDLE:   begin
//                 egress_data_ack  <= 1'b0;
//                 ingress_data_ack <= 1'b0;
//                 egress_addr_fifo_wren   <= 1'b0;
//					  ingress_addr_fifo_wren   <= 1'b0;
//////                 if(~addr_fifo_full)begin //don't do anything if addr_fifo is full
///// Jiansong: ddr read/write scheduling here
//////                   if(ingress_data_req) begin
//                     if(ingress_data_req & 
//							   ~ingress_addr_fifo_full & 
//								~ingress_fifo_pfull) begin   /// Jiansong: if ingress_fifo_status
//								                                 ///           is almostfull, block
//																		   ///           ingress data request
//                       dma_state <= LOADI;
//                       //assert the ingress ack and load the ADDR/CNTRL fifo with
//                       //the correct ddr2 write parameters
//                       ingress_data_ack <= 1'b1;
//                       ingress_addr_fifo_wrdata[31:0] <= ({6'b000000,
//                                                  ingress_xfer_size[2:0],
//                                                  ingress_start_addr[27:6],
//                                                   1'b0});//bit 0 == 1'b0 denotes
//                                                          //write to ddr2
//                       ingress_addr_fifo_wren   <= 1'b1;    
//////                   end else if(egress_data_req)begin
//                     end else if(egress_data_req & ~egress_addr_fifo_full)begin
//                       //Don't grant ack for egress if the egress fifo is almost
//                       //full, otherwise the MIG controller might overfill the
//                       //egress fifo.
//////                       if(egress_pfull) begin      /// Jiansong: how much is almost?
//////                         dma_state <= IDLE;
//////                       end else begin
//                         //Otherwise, assert the egress ack and load the 
//                         //ADDR/CNTRL fifo with the correct ddr2 read parameters
//                         dma_state <= LOADE;
//                         egress_data_ack <= 1'b1;
//                         egress_addr_fifo_wrdata[31:0] <= ({6'b000000,
//                                                    egress_xfer_size[2:0],
//                                                    egress_start_addr[27:6],
//                                                    1'b1});//bit 0 == 1'b1 denotes
//                                                           //read from ddr2
//                         egress_addr_fifo_wren   <= 1'b1;
//////                       end
//////                     end    
//                 end else begin
//                   dma_state <= IDLE;
//                 end
//               end
//      //LOADE and LOADI are for deasserting the ack and wren signals
//      //before returning to IDLE                
//      LOADE:   begin
//                 egress_addr_fifo_wren   <= 1'b0;
//                 egress_data_ack <= 1'b0;
//                 dma_state        <= WAITE;
//               end
//		/// Jiansong: add one cycle to wait for the deassertion of egress request
//		WAITE:   begin
//                 egress_addr_fifo_wren   <= 1'b0;
//                 egress_data_ack <= 1'b0;
//                 dma_state        <= DONE;
//               end
//      LOADI:   begin
//                 ingress_addr_fifo_wren   <= 1'b0;
//                 ingress_data_ack <= 1'b0;
//                 dma_state        <= DONE;
//               end
//      DONE:   begin
//                 dma_state        <= IDLE;
//                 egress_data_ack  <= 1'b0;
//                 ingress_data_ack <= 1'b0;
//                 ingress_addr_fifo_wren   <= 1'b0;
//					  egress_addr_fifo_wren   <= 1'b0;
//               end
//    default:   begin
//                 dma_state        <= IDLE;
//                 egress_data_ack  <= 1'b0;
//                 ingress_data_ack <= 1'b0;
//                 ingress_addr_fifo_wren   <= 1'b0;
//					  egress_addr_fifo_wren   <= 1'b0;
//               end
//    endcase
//  end
//end

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//// Jiansong, 2012-12-15, add pipeline to save overhead
////ToDDR_addr_valid
wire	R1_one;
wire	W1_one;
rising_edge_detect R1_one_inst(.clk(ddr_clk),.rst(ddr_reset),.in(ddr_state==R1),.one_shot_out(R1_one));
rising_edge_detect W1_one_inst(.clk(ddr_clk),.rst(ddr_reset),.in(ddr_state==W1),.one_shot_out(W1_one));

reg	ToDDR_addr_valid;
reg [25:0]	ToDDR_addr_data;
always@(posedge ddr_clk) begin
	if (ddr_reset)
		ToDDR_addr_valid	<= 1'b0;
	else if (sche_state==SETE | sche_state==SETE2 | sche_state==SETE3 | sche_state==SETE4 | sche_state==SETI)
		ToDDR_addr_valid	<= 1'b1;
	else if (R1_one | W1_one)
		ToDDR_addr_valid	<= 1'b0;
	else
		ToDDR_addr_valid	<= ToDDR_addr_valid;
end
//////////////////////////////////////////////////////
//////////////////  Scheduler  ///////////////////////
// input: four egress_addr_fifos and one ingress_addr_fifo
// output: ToDDR_addr_fifo
// round-robin scheduling
always@(posedge ddr_clk) begin
	if (ddr_reset) begin
		sche_state							<= IDLE;
		token									<= 3'b000;
		ingress_addr_fifo_rden			<= 1'b0;
		egress_addr_fifo_rden				<= 1'b0;
		egress_2nd_addr_fifo_rden		<= 1'b0;
		egress_3rd_addr_fifo_rden		<= 1'b0;
		egress_4th_addr_fifo_rden		<= 1'b0;
		ToDDR_addr_data					<= 26'h000_0000;
		FromDDR_addr_fifo_wren			<= 1'b0;
		FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//		ToDDR_addr_fifo_wren				<= 1'b0;
//		ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;		
	end else begin
		case (sche_state)
			IDLE: begin
					token								<= token[2:0];
					ToDDR_addr_data				<= ToDDR_addr_data[25:0];
					FromDDR_addr_fifo_wren		<= 1'b0;
					FromDDR_addr_fifo_wrdata	<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b0;
//					ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;
//					if (~ToDDR_addr_fifo_full) begin
					if (~ToDDR_addr_valid) begin
						casex({token[2:0],egress_addr_fifo_empty | ToFRL_data_fifo_pfull,				/// liuchang: add ToFRL_data_fifo_pfull
									egress_2nd_addr_fifo_empty | ToFRL_2nd_data_fifo_pfull,
									egress_3rd_addr_fifo_empty | ToFRL_3rd_data_fifo_pfull,
									egress_4th_addr_fifo_empty | ToFRL_4th_data_fifo_pfull,
									ingress_addr_fifo_empty,
									egress_fifo_pfull|FromDDR_addr_fifo_full})
//							9'b000_0xxxx_0, 9'b001_01111_0, 9'b010_0x111_0, 9'b011_0xx11_0, 9'b1xx_0xxx1_0: begin
							9'b000_0xxxx_0, 9'b001_0111x_0, 9'b010_0x11x_0, 9'b011_0xx1x_0, 9'b1xx_0xxxx_0: begin
								sche_state						<= LOADE;
								egress_addr_fifo_rden		<= 1'b1;
								egress_2nd_addr_fifo_rden	<= 1'b0;
								egress_3rd_addr_fifo_rden	<= 1'b0;
								egress_4th_addr_fifo_rden	<= 1'b0;
								ingress_addr_fifo_rden		<= 1'b0;
							end
//							9'b000_10xxx_0, 9'b001_x0xxx_0, 9'b010_10111_0, 9'b011_10x11_0, 9'b1xx_10xx1_0: begin
							9'b000_10xxx_0, 9'b001_x0xxx_0, 9'b010_1011x_0, 9'b011_10x1x_0, 9'b1xx_10xxx_0: begin
								sche_state						<= LOADE2;
								egress_addr_fifo_rden		<= 1'b0;
								egress_2nd_addr_fifo_rden	<= 1'b1;
								egress_3rd_addr_fifo_rden	<= 1'b0;
								egress_4th_addr_fifo_rden	<= 1'b0;
								ingress_addr_fifo_rden		<= 1'b0;
							end
//							9'b000_110xx_0, 9'b001_x10xx_0, 9'b010_xx0xx_0, 9'b011_11011_0, 9'b1xx_110x1_0: begin
							9'b000_110xx_0, 9'b001_x10xx_0, 9'b010_xx0xx_0, 9'b011_1101x_0, 9'b1xx_110xx_0: begin
								sche_state						<= LOADE3;
								egress_addr_fifo_rden		<= 1'b0;
								egress_2nd_addr_fifo_rden	<= 1'b0;
								egress_3rd_addr_fifo_rden	<= 1'b1;
								egress_4th_addr_fifo_rden	<= 1'b0;
								ingress_addr_fifo_rden		<= 1'b0;
							end
//							9'b000_1110x_0, 9'b001_x110x_0, 9'b010_xx10x_0, 9'b011_xxx0x_0, 9'b1xx_11101_0: begin
							9'b000_1110x_0, 9'b001_x110x_0, 9'b010_xx10x_0, 9'b011_xxx0x_0, 9'b1xx_1110x_0: begin
								sche_state						<= LOADE4;
								egress_addr_fifo_rden		<= 1'b0;
								egress_2nd_addr_fifo_rden	<= 1'b0;
								egress_3rd_addr_fifo_rden	<= 1'b0;
								egress_4th_addr_fifo_rden	<= 1'b1;
								ingress_addr_fifo_rden		<= 1'b0;
							end
//							9'b000_11110_x, 9'b001_x1110_x, 9'b010_xx110_x, 9'b011_xxx10_x, 9'b1xx_xxxx0_x, 9'bxxx_xxxx0_1: begin
							9'b000_11110_x, 9'b001_11110_x, 9'b010_11110_x, 9'b011_11110_x, 9'b1xx_11110_x, 9'bxxx_xxxx0_1: begin
								sche_state						<= LOADI;
								egress_addr_fifo_rden		<= 1'b0;
								egress_2nd_addr_fifo_rden	<= 1'b0;
								egress_3rd_addr_fifo_rden	<= 1'b0;
								egress_4th_addr_fifo_rden	<= 1'b0;
								ingress_addr_fifo_rden		<= 1'b1;
							end
							default: begin
								sche_state						<= IDLE;
								egress_addr_fifo_rden		<= 1'b0;
								egress_2nd_addr_fifo_rden	<= 1'b0;
								egress_3rd_addr_fifo_rden	<= 1'b0;
								egress_4th_addr_fifo_rden	<= 1'b0;
								ingress_addr_fifo_rden		<= 1'b0;
							end
						endcase
					end else begin
						sche_state						<= IDLE;
						egress_addr_fifo_rden		<= 1'b0;
						egress_2nd_addr_fifo_rden	<= 1'b0;
						egress_3rd_addr_fifo_rden	<= 1'b0;
						egress_4th_addr_fifo_rden	<= 1'b0;
						ingress_addr_fifo_rden		<= 1'b0;
					end
			end
			LOADE: begin
					sche_state							<= SETE;
					token									<= 3'b001;
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= ToDDR_addr_data[25:0];
					FromDDR_addr_fifo_wren			<= 1'b0;
					FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b0;
//					ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;
			end
			SETE: begin
					sche_state							<= IDLE;
					token									<= token[2:0];
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= egress_addr_fifo_rddata[25:0];
					FromDDR_addr_fifo_wren			<= 1'b1;
					FromDDR_addr_fifo_wrdata		<= egress_addr_fifo_rddata[31:0];
//					ToDDR_addr_fifo_wren				<= 1'b1;
//					ToDDR_addr_fifo_wrdata[31:0]	<= egress_addr_fifo_rddata[31:0];
			end
			LOADE2: begin
					sche_state							<= SETE2;
					token									<= 3'b010;
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= ToDDR_addr_data[25:0];
					FromDDR_addr_fifo_wren			<= 1'b0;
					FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b0;
//					ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;
			end
			SETE2: begin
					sche_state							<= IDLE;
					token									<= token[2:0];
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= egress_2nd_addr_fifo_rddata[25:0];
					FromDDR_addr_fifo_wren			<= 1'b1;
					FromDDR_addr_fifo_wrdata		<= egress_2nd_addr_fifo_rddata[31:0];
//					ToDDR_addr_fifo_wren				<= 1'b1;
//					ToDDR_addr_fifo_wrdata[31:0]	<= egress_2nd_addr_fifo_rddata[31:0];
			end
			LOADE3: begin
					sche_state							<= SETE3;
					token									<= 3'b011;
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= ToDDR_addr_data[25:0];
					FromDDR_addr_fifo_wren			<= 1'b0;
					FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b0;
//					ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;
			end
			SETE3: begin
					sche_state							<= IDLE;
					token									<= token[2:0];
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= egress_3rd_addr_fifo_rddata[25:0];
					FromDDR_addr_fifo_wren			<= 1'b1;
					FromDDR_addr_fifo_wrdata		<= egress_3rd_addr_fifo_rddata[31:0];
//					ToDDR_addr_fifo_wren				<= 1'b1;
//					ToDDR_addr_fifo_wrdata[31:0]	<= egress_3rd_addr_fifo_rddata[31:0];
			end
			LOADE4: begin
					sche_state							<= SETE4;
//					token									<= 3'b100;
					token									<= 3'b000;
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= ToDDR_addr_data[25:0];
					FromDDR_addr_fifo_wren			<= 1'b0;
					FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b0;
//					ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;
			end
			SETE4: begin
					sche_state							<= IDLE;
					token									<= token[2:0];
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= egress_4th_addr_fifo_rddata[25:0];
					FromDDR_addr_fifo_wren			<= 1'b1;
					FromDDR_addr_fifo_wrdata		<= egress_4th_addr_fifo_rddata[31:0];
//					ToDDR_addr_fifo_wren				<= 1'b1;
//					ToDDR_addr_fifo_wrdata[31:0]	<= egress_4th_addr_fifo_rddata[31:0];
			end
			LOADI: begin
					sche_state							<= SETI;
//					token									<= 3'b000;
					token									<= token[2:0];
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= ToDDR_addr_data[25:0];
					FromDDR_addr_fifo_wren			<= 1'b0;
					FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b0;
//					ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;
			end
			SETI: begin
					sche_state							<= IDLE;
					token									<= token[2:0];
					egress_addr_fifo_rden			<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ingress_addr_fifo_rden			<= 1'b0;
					ToDDR_addr_data					<= ingress_addr_fifo_rddata[25:0];
					FromDDR_addr_fifo_wren			<= 1'b0;					// ingress_addr should not be passed to FromDDR_addr_fifo
					FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b1;
//					ToDDR_addr_fifo_wrdata[31:0]	<= ingress_addr_fifo_rddata[31:0];
			end
			default: begin
					sche_state							<= IDLE;
					token									<= token[2:0];
					ingress_addr_fifo_rden			<= 1'b0;
					egress_addr_fifo_rden				<= 1'b0;
					egress_2nd_addr_fifo_rden		<= 1'b0;
					egress_3rd_addr_fifo_rden		<= 1'b0;
					egress_4th_addr_fifo_rden		<= 1'b0;
					ToDDR_addr_data					<= ToDDR_addr_data[25:0];
					FromDDR_addr_fifo_wren			<= 1'b0;
					FromDDR_addr_fifo_wrdata		<= 32'h0000_0000;
//					ToDDR_addr_fifo_wren				<= 1'b0;
//					ToDDR_addr_fifo_wrdata[31:0]	<= 32'h0000_0000;		
			end
		endcase
	end
end


///// Jiansong: generate egress_fifo_ready signal
//rising_edge_detect egress_fifo_almostfull_falling_inst(
//                .clk(ddr_clk),
//                .rst(reset),
//                .in(~egress_pfull),
//                .one_shot_out(egress_pfull_falling)
//                );
//always@(posedge ddr_clk)begin
//   if(reset)
//	   egress_fifo_ready <= 1'b0;
//	// During a 4k-read-requests writing into ddr module, if a DRAM refresh cycle occurs that read 
//	// requests write in is interrupted, the egress_pfull signal may be deasserted and asserted
//	// one more time unexpectedly. (ddr_stat != R1) means to bypass the deassert event if current 4k 
//	// reading is still ongoing, which will prevent asserting egress_fifo_ready by mistake.
//	else if(egress_pfull_falling && (ddr_state != R1))    
//	   egress_fifo_ready <= 1'b1;
//	else if(egress_addr_fifo_rden)
//	   egress_fifo_ready <= 1'b0;
//end
//always@(posedge ddr_clk)begin
//   if(reset)
//	   less_4k_pass <= 1'b0;
//   else if(egress_addr_fifo_rddata[25:23] < 3'b110)
//	   less_4k_pass <= 1'b1;
//	else
//	   less_4k_pass <= 1'b0;
//end
/// Jiansong: modifications
///           (1) ingress/egress scheduling
///           (2) egress always has higher priority
///           (3) egress flow control to prevent egress_data_fifo overflow
///////////////////////////////////////////////////////////////////////
// DDR state machine  
//
// Monitors the ADDR/CNTRL FIFO and and translates incoming requests for 
// MIG DDR2 controller
//
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//// Jiansong, 2012-12-15, add pipeline to save overhead
////ToDDR_addr_valid
//reg	ToDDR_addr_valid;
//always@(posedge ddr_clk) begin
//	if (rst)
//		ToDDR_addr_valid	<= 1'b0;
//	else if (ToDDR_addr_valid) begin
//		if (ddr_state == R1 | ddr_state == W1)
//			ToDDR_addr_valid	<= 1'b0;
//		else
//			ToDDR_addr_valid	<= ToDDR_addr_valid;
//	end else begin
//		if (ToDDR_addr_fifo_ren)
//			ToDDR_addr_valid	<= 1'b1;
//		else
//			ToDDR_addr_valid	<= ToDDR_addr_valid;
//	end
//end
//// ToDDR_addr_fifo_ren
//always@(posedge ddr_clk) begin
//	if (rst)
//		ToDDR_addr_fifo_ren  <= 1'b0;
//	else if (~ToDDR_addr_valid & ~ToDDR_addr_fifo_ren & ~ToDDR_addr_fifo_empty)		// ren should be one cycle each time
//		ToDDR_addr_fifo_ren  <= 1'b1;
//	else
//		ToDDR_addr_fifo_ren  <= 1'b0;
//end

always@(posedge ddr_clk) begin
	if(ddr_reset)begin
		ddr_state			<= NOOP;
////    addr_fifo_ren  <= 1'b0;
//    egress_addr_fifo_rden  <= 1'b0;
//	 ingress_addr_fifo_rden  <= 1'b0;
		xfer_cnt				<= 10'b0000000000;
		m_addr				<= 31'h00000000;
		m_cmd					<= 3'b000;
		m_addr_wen			<= 1'b0;
		m_data_wen			<= 1'b0;
		ingress_fifo_rden	<= 1'b0;
	end else begin
		case(ddr_state)
//      NOOP:    begin
//                 m_data_wen    <= 1'b0;
//                 m_addr_wen    <= 1'b0;
//////                 //if the ADDR/CTRL fifo is not empty then read one entry
//////                 //from the fifo, otherwise stay in this state and do nothing
//////                 if(addr_fifo_empty) begin
//////                   ddr_state     <= NOOP;
//////                 end else begin
//////                   ddr_state     <= LOAD0;
//////                   addr_fifo_ren <= 1'b1;
//////                 end
//                 
//					  // egress flow control: egress_fifo_ready signal and less_4k_pass signal
//					  //       read out data only in 3 situation:
//					  //       (1) first request (egress_empty)
//					  //       (2) data in fifo becomes less than half full(egress_fifo_ready)
//					  //       (3) small/the_last requests will not make egress_data_fifo overflow
//					  //           (less_4k_pass)
//					  // egress always has higher priority
//                 if(~egress_addr_fifo_empty & 
//					     (egress_empty | egress_fifo_ready | less_4k_pass)) begin
//                   ddr_state     <= E_LOAD0;
//                   egress_addr_fifo_rden <= 1'b1;
//                 end else if(~ingress_addr_fifo_empty & ~ingress_fifo_almostempty) begin
//                   ddr_state     <= IN_LOAD0;
//						 ingress_addr_fifo_rden <= 1'b1;
//                 end else begin
//                   ddr_state     <= NOOP;
//                 end
//               end
//     E_LOAD0:  begin //state for deasserting the addr_fifo_ren signal
//                 ddr_state     <= E_LOAD1;
//                 egress_addr_fifo_rden <= 1'b0;
//               end
//     E_LOAD1:  begin /// Jiansong: m_addr distance 1 means 8bytes(64bits) 
//                 //map a byte address into ddr2 column address:
//                 //since the ddr2 memory in this ref. design is 
//                 //64 bits (i.e. 8 bytes) wide, each column address
//                 //addresses 8 bytes - therefore the byte address
//                 //needs to be divided-by-8 for the ddr2 memory cntrl
//                 //NOTE: addr_fifo_rddata[0] denotes read vs. write
//                 //and is not part of the address
//                 m_addr[30:25] <= 6'b000000; //upper bits not used
//                 m_addr[24:3] <= egress_addr_fifo_rddata[22:1];//byte-to-column mapping
//                 m_addr[2:0]   <= 3'b000; //always 0 for 64B boundary
//                 egress_addr_fifo_rden <= 1'b0;
//                 //decode the transfer size information into bytes
//                 //and setup a counter (xfer_cnt) to keep track
//                 //of how much data is transferred (ingress or egress)
//                 case(egress_addr_fifo_rddata[25:23])
//                    3'b000:  xfer_cnt <= 10'b0000000100;   //64B            
//                    3'b001:  xfer_cnt <= 10'b0000001000;   //128B
//                    3'b010:  xfer_cnt <= 10'b0000010000;   //256B
//                    3'b011:  xfer_cnt <= 10'b0000100000;   //512B
//                    3'b100:  xfer_cnt <= 10'b0001000000;   //1KB
//                    3'b101:  xfer_cnt <= 10'b0010000000;   //2KB
//                    3'b110:  xfer_cnt <= 10'b0100000000;   //4KB                
//                    default: xfer_cnt <= 10'b0000001000; //default to 128B      
//                 endcase
//					  if(~m_af_afull)begin
//					    ddr_state     <= R1;
//                   m_addr_wen    <= 1'b1;//assert the write enable first
//                   m_cmd[2:0] <= 3'b001;
//					  end else begin
//					    ddr_state <= E_LOAD1;
//					  end
//					end
//    IN_LOAD0:  begin //state for deasserting the addr_fifo_ren signal
//                 ddr_state     <= IN_LOAD1;
//                 ingress_addr_fifo_rden <= 1'b0;
//               end
//    IN_LOAD1:  begin /// Jiansong: m_addr distance 1 means 8bytes(64bits) 
//                 //map a byte address into ddr2 column address:
//                 //since the ddr2 memory in this ref. design is 
//                 //64 bits (i.e. 8 bytes) wide, each column address
//                 //addresses 8 bytes - therefore the byte address
//                 //needs to be divided-by-8 for the ddr2 memory cntrl
//                 //NOTE: addr_fifo_rddata[0] denotes read vs. write
//                 //and is not part of the address
//                 m_addr[30:25] <= 6'b000000; //upper bits not used
//                 m_addr[24:3] <= ingress_addr_fifo_rddata[22:1];//byte-to-column mapping
//                 m_addr[2:0]   <= 3'b000; //always 0 for 64B boundary
//                 ingress_addr_fifo_rden <= 1'b0;
//                 //decode the transfer size information into bytes
//                 //and setup a counter (xfer_cnt) to keep track
//                 //of how much data is transferred (ingress or egress)
//                 case(ingress_addr_fifo_rddata[25:23])
//                    3'b000:  xfer_cnt <= 10'b0000000100;   //64B            
//                    3'b001:  xfer_cnt <= 10'b0000001000;   //128B
//                    3'b010:  xfer_cnt <= 10'b0000010000;   //256B
//                    3'b011:  xfer_cnt <= 10'b0000100000;   //512B
//                    3'b100:  xfer_cnt <= 10'b0001000000;   //1KB
//                    3'b101:  xfer_cnt <= 10'b0010000000;   //2KB
//                    3'b110:  xfer_cnt <= 10'b0100000000;   //4KB                
//                    default: xfer_cnt <= 10'b0000001000; //default to 128B      
//                 endcase
//					  if(~m_af_afull && ~m_wdf_afull)begin           
//                   ddr_state     <= W1;
//                   m_cmd[2:0] <= 3'b000;
//					  end else begin
//					    ddr_state <= IN_LOAD1;
//					  end
//					end
//////       LOAD0:  begin //state for deasserting the addr_fifo_ren signal
//////                 ddr_state     <= LOAD1;
//////                 addr_fifo_ren <= 1'b0;
//////               end 
//////       //LOAD1 state for latching the read data from the ADDR/CTRL fifo
//////       //and decoding the information          
//////       LOAD1:  begin /// Jiansong: m_addr distance 1 means 8bytes(64bits) 
//////                 //map a byte address into ddr2 column address:
//////                 //since the ddr2 memory in this ref. design is 
//////                 //64 bits (i.e. 8 bytes) wide, each column address
//////                 //addresses 8 bytes - therefore the byte address
//////                 //needs to be divided-by-8 for the ddr2 memory cntrl
//////                 //NOTE: addr_fifo_rddata[0] denotes read vs. write
//////                 //and is not part of the address
//////                 m_addr[30:25] <= 6'b000000; //upper bits not used
//////                 m_addr[24:3] <= addr_fifo_rddata[22:1];//byte-to-column mapping
//////                 m_addr[2:0]   <= 3'b000; //always 0 for 128B boundary
//////                 addr_fifo_ren <= 1'b0;
//////                 //decode the transfer size information into bytes
//////                 //and setup a counter (xfer_cnt) to keep track
//////                 //of how much data is transferred (ingress or egress)
//////                 case(addr_fifo_rddata[25:23])
//////                    3'b000:  xfer_cnt <= 10'b0000000100;   //64B            
//////                    3'b001:  xfer_cnt <= 10'b0000001000;   //128B
//////                    3'b010:  xfer_cnt <= 10'b0000010000;   //256B
//////                    3'b011:  xfer_cnt <= 10'b0000100000;   //512B
//////                    3'b100:  xfer_cnt <= 10'b0001000000;   //1KB
//////                    3'b101:  xfer_cnt <= 10'b0010000000;   //2KB
//////                    3'b110:  xfer_cnt <= 10'b0100000000;   //4KB                
//////                    default: xfer_cnt <= 10'b0000001000; //default to 128B      
//////                 endcase
//////                 //if bit 0 is a 1 then egress or read from ddr
//////                 //and jump to egress flow (state R)
//////					  /// Jiansong: if egress fifo is full, block the process
//////					  ///           it's temporary solution to prevent egress fifo overflow
//////////                 if(addr_fifo_rddata[0] && ~m_af_afull)begin 
//////                 if(addr_fifo_rddata[0] && ~m_af_afull && ~egress_pfull)begin
//////  					    ddr_state     <= R1;
//////                   m_addr_wen    <= 1'b1;//assert the write enable first
//////                   m_cmd[2:0] <= 3'b001; 
//////                //otherwise it is ingress or write to ddr
//////                //and jump to ingress flow (state W1) 
//////////                 end else if(~m_af_afull && ~m_wdf_afull)begin
//////                end else if(~addr_fifo_rddata[0] && ~m_af_afull && ~m_wdf_afull)begin            
//////                   ddr_state     <= W1;
//////                   m_cmd[2:0] <= 3'b000;
//////                 end else begin
//////                   ddr_state <= LOAD1;
//////                 end    
//////               end
			NOOP: begin
                 m_data_wen    <= 1'b0;	//NOOP
                 m_addr_wen    <= 1'b0;	//NOOP
						if (ToDDR_addr_valid) begin
						  //map a byte address into ddr2 column address:
						  //since the ddr2 memory in this ref. design is 
						  //64 bits (i.e. 8 bytes) wide, each column address
						  //addresses 8 bytes - therefore the byte address
						  //needs to be divided-by-8 for the ddr2 memory cntrl
						  //NOTE: addr_fifo_rddata[0] denotes read vs. write
						  //and is not part of the address
						  m_addr[30:25]	<= 6'b000000; //upper bits not used
						  m_addr[24:3]		<= ToDDR_addr_data[22:1];//byte-to-column mapping
						  m_addr[2:0]		<= 3'b000; //always 0 for 128B(64B?) boundary
	//                 addr_fifo_ren <= 1'b0;
						  //decode the transfer size information into bytes
						  //and setup a counter (xfer_cnt) to keep track
						  //of how much data is transferred (ingress or egress)
						  case(ToDDR_addr_data[25:23])
							  3'b000:  xfer_cnt <= 10'b00_0000_0100;   //64B            
							  3'b001:  xfer_cnt <= 10'b00_0000_1000;   //128B
							  3'b010:  xfer_cnt <= 10'b00_0001_0000;   //256B
							  3'b011:  xfer_cnt <= 10'b00_0010_0000;   //512B
							  3'b100:  xfer_cnt <= 10'b00_0100_0000;   //1KB
							  3'b101:  xfer_cnt <= 10'b00_1000_0000;   //2KB
							  3'b110:  xfer_cnt <= 10'b01_0000_0000;   //4KB                
							  default: xfer_cnt <= 10'b00_0000_1000; //default to 128B      
						  endcase
						  //if bit 0 is a 1 then egress or read from ddr
						  //and jump to egress flow (state R)
						  if((ToDDR_addr_data[0]) && (~m_af_afull))begin 
							 ddr_state		<= R1;
							 m_addr_wen		<= 1'b1;//assert the write enable first
							 m_cmd[2:0]		<= 3'b001; 
						 //otherwise it is ingress or write to ddr
						 //and jump to ingress flow (state W1) 
						  end else if((~ToDDR_addr_data[0]) && (~m_af_afull) && (~m_wdf_afull))begin            
							 ddr_state		<= W1;
							 m_cmd[2:0]		<= 3'b000;
						  end else begin
//							 ddr_state <= LOAD1;
							 ddr_state	<= NOOP;
						  end
						end else begin
//							ddr_state <= LOAD1;
							ddr_state	<= NOOP;
						end
			end
        //Start of read from ddr2 (egress) flow      
			R1: begin
						if(xfer_cnt == 10'h002)begin
							ddr_state	<= NOOP;
							m_addr_wen	<= 1'b0;
							m_addr		<= 31'h0000_0000;
							xfer_cnt		<= 10'h000;
						end else if (~m_af_afull) begin
							ddr_state	<= R1;
							m_addr_wen	<= 1'b1;
							m_addr		<= m_addr[30:0] + 31'h0000_0004;
							xfer_cnt		<= xfer_cnt[9:0] - 10'h002;
						end else begin
							ddr_state	<= R1;
							m_addr_wen	<= 1'b0;
							m_addr		<= m_addr[30:0];
							xfer_cnt		<= xfer_cnt[9:0];
						end
//                 //assert the write enable and increment the address
//                 //to the memory cntrl;
//                 //the ddr2 memory in this reference design is configured as
//                 //burst-of-4 the address needs to be incremented by four
//                 //for each read request
//                 m_addr[30:0]     <= m_addr[30:0] + 3'b100;
//                 xfer_cnt   <= xfer_cnt - 3'b10;
//                 //when it gets to the penultimate transfer, go to R2
//                 //to deassert the write enable
//                 if(xfer_cnt == 10'h002)begin
//                    ddr_state  <= NOOP;
//                    m_addr_wen    <= 1'b0;
//                 end else begin 
//                    ddr_state  <= R1;
//                    m_addr_wen    <= 1'b1;
//                 end
			end
//			R2: begin
//                 ddr_state   <= NOOP;
//                 m_addr_wen  <= 1'b0;
//			end
        //Start of write to ddr2 (ingress) flow 
			W1: begin //assert the read enable from the ingress fifo
                     //to get the ingress data ready for writing 
                     //into the memory cntrl
					m_addr_wen			<= 1'b0;
					m_data_wen			<= 1'b0;
//                 if(at_least_64B && ~m_wdf_afull)begin
					if((~ingress_fifo_almostempty) && (~m_wdf_afull))begin
						ddr_state				<= W2;
						ingress_fifo_rden		<= 1'b1;
					end else begin
						ddr_state				<= W1;
						ingress_fifo_rden		<= 1'b0;
					end
			end       
			W2: begin //now assert the address and data write enables
                     //to the memory cntrl
					ddr_state			<= W3;
					m_addr_wen			<= 1'b1;
					m_data_wen			<= 1'b1;
					ingress_fifo_rden	<= 1'b1;
			end
			W3: begin //deassert the address write enable but keep the
                     //data write enable asserted - this is because
                     //the data is written in 16 bytes (128 bit) at a time
                     //and each address is for 32 bytes, so we need two
                     //data cycles for each address cycles;
                     //also increment the address to the memory controller
                     //to the next column
					if((~ingress_fifo_almostempty) && (~m_wdf_afull))begin
						ddr_state			<= W4;
						m_addr_wen			<= 1'b0;
						m_data_wen			<= 1'b1; 
						ingress_fifo_rden	<= 1'b1;
						m_addr[30:0]		<= m_addr[30:0] + 3'b100;
					end else begin
						ddr_state			<= W3;
						m_addr_wen			<= 1'b0;
						m_data_wen			<= 1'b0; 
						ingress_fifo_rden	<= 1'b0;
						m_addr[30:0]		<= m_addr[30:0];
					end
			end
			W4: begin //write the second column address to the memory cntrl
					ddr_state			<= W5;
					m_addr_wen			<= 1'b1;
					m_data_wen			<= 1'b1;
					ingress_fifo_rden	<= 1'b1; 
			end
			W5: begin //decide whether to repeat the cycle or not
					m_addr_wen    <= 1'b0;
                   //when it gets to the penultimate transfer, deassert the
                   //read enable to the ingress fifo read enable and then
                   //jump to W6 so that the data and address write enables
                   //to the memory cntrl are deasserted one clock later
					if(xfer_cnt == 10'h004) begin 
						ddr_state        	<= NOOP;            /// bug? no. state W6 is not used
						m_data_wen			<= 1'b1;
						ingress_fifo_rden	<= 1'b0;
					end else if((~ingress_fifo_almostempty) && (~m_af_afull) && (~m_wdf_afull))begin
                   //otherwise decrement the transfer count, increase the
                   //address and repeat the cycle
						ddr_state			<= W2;
						m_data_wen			<= 1'b1;
						ingress_fifo_rden	<= 1'b1;
						xfer_cnt				<= xfer_cnt - 3'b100;
						m_addr[30:0]		<= m_addr[30:0] + 3'b100;  
					end else begin
						ddr_state			<= W5;
						m_data_wen			<= 1'b0;
						ingress_fifo_rden	<= 1'b0;
//						ddr_state <= WAIT_FOR_ROOM;
//						ingress_fifo_rden <= 1'b0;
					end
			end
//			W6:    begin                    /// this state is not used
//                 ddr_state     <= NOOP;
//                 m_data_wen    <= 1'b0;
//                 m_addr_wen    <= 1'b0;
//			end
//			WAIT_FOR_ROOM: begin
//                 m_addr_wen    <= 1'b0;
//                 if(~m_af_afull && ~m_wdf_afull)begin
//                    m_data_wen <= 1'b1;
//                    ddr_state <= W5;
//                 end else begin     
//                    m_data_wen <= 1'b0;
//                    ddr_state <= WAIT_FOR_ROOM;
//                 end
//			end
			default:  begin
				ddr_state			<= NOOP;
////                 addr_fifo_ren  <= 1'b0;
//                 egress_addr_fifo_rden  <= 1'b0;
//					  ingress_addr_fifo_rden  <= 1'b0;
				xfer_cnt				<= 10'b00_0000_0000;
				m_addr[30:0]		<= 31'h0000_0000;
				m_cmd[2:0]			<= 3'b000;
				m_addr_wen			<= 1'b0;
				m_data_wen			<= 1'b0;
				ingress_fifo_rden	<= 1'b0;
			end
		endcase
	end
end

///////////////////////////////////////////////////////////
///////  egress_data_fifo to ToFRL_data_fifos  ////////////
///////////////////////////////////////////////////////////
wire	E_PASS_one;
rising_edge_detect E_PASS_one_inst(.clk(ddr_clk),.rst(ddr_reset),.in(egress_state==E_PASS),.one_shot_out(E_PASS_one));

//reg			FromDDR_addr_valid;
reg [8:0]	egress_cnt;

always@(posedge ddr_clk) begin
	if (ddr_reset) begin
		egress_fifo_rden	<= 1'b0;
		egress_cnt			<= 9'b000000_000;
		egress_state		<= E_IDLE;
	end else begin
		case(egress_state)
			E_IDLE: begin
				if (~FromDDR_addr_fifo_empty) begin
					FromDDR_addr_fifo_rden	<= 1'b1;
					egress_fifo_rden			<= 1'b0;
					egress_cnt					<= 9'b000000_000;
					egress_state				<= E_WAIT_ADDR;
				end else begin
					FromDDR_addr_fifo_rden	<= 1'b0;
					egress_fifo_rden			<= 1'b0;
					egress_cnt					<= 9'b000000_000;
					egress_state				<= E_IDLE;
				end
			end
			
			E_WAIT_ADDR: begin
				FromDDR_addr_fifo_rden	<= 1'b0;
				egress_fifo_rden			<= 1'b0;
				egress_cnt					<= 9'b000000_000;
				egress_state				<= E_PASS_START;
			end
			
			E_PASS_START: begin
//				if ( ~ToFRL_data_fifo_pfull_this &	~egress_fifo_pempty ) begin
				if ( ~ToFRL_data_fifo_almost_full_this &	~egress_fifo_pempty ) begin			/// liuchang
					egress_fifo_rden	<= 1'b1;
					case(FromDDR_addr_fifo_rddata[25:23])
						3'b001:  egress_cnt <= 9'b000001_000 - 9'b000000_001;   //128B
						3'b010:  egress_cnt <= 9'b000010_000 - 9'b000000_001;   //256B
						3'b011:  egress_cnt <= 9'b000100_000 - 9'b000000_001;   //512B
						3'b100:  egress_cnt <= 9'b001000_000 - 9'b000000_001;   //1KB
						3'b101:  egress_cnt <= 9'b010000_000 - 9'b000000_001;   //2KB
						3'b110:  egress_cnt <= 9'b100000_000 - 9'b000000_001;   //4KB                
						default: egress_cnt <= 9'b000001_000 - 9'b000000_001; //default to 128B      
					endcase
					egress_state		<= E_PASS;
				end else begin
					egress_fifo_rden	<= 1'b0;
					egress_cnt			<= 9'b000000_000;
					egress_state		<= E_PASS_START;
				end
			end
		
			E_PASS: begin
				if (egress_cnt == 9'b000000_000) begin
					egress_fifo_rden	<= 1'b0;	
					egress_cnt			<= egress_cnt;
					egress_state		<= E_IDLE;
				end else begin
//					if ( ~ToFRL_data_fifo_pfull_this &	~egress_fifo_pempty ) begin		// flow control, while the block size increases to 1kB, flow control becomes necessary
					if ( ~ToFRL_data_fifo_almost_full_this &	~egress_fifo_pempty ) begin		/// liuchang
						egress_fifo_rden	<= 1'b1;
						egress_cnt			<= egress_cnt - 9'b000000_001;
					end else begin
						egress_fifo_rden	<= 1'b0;
						egress_cnt			<= egress_cnt;
					end
					egress_state		<= E_PASS;
				end
			end
			
			default: begin
				egress_fifo_rden	<= 1'b0;
				egress_cnt			<= 9'b000000_000;
				egress_state		<= E_IDLE;
			end
		endcase
	end
end


//always@(posedge ddr_clk) begin
//	if (ddr_reset)
//		FromDDR_addr_valid	<= 1'b0;
//	else if (~FromDDR_addr_valid) begin
//		if (FromDDR_addr_fifo_rden)
//			FromDDR_addr_valid	<= 1'b1;
//		else
//			FromDDR_addr_valid	<= FromDDR_addr_valid;
//	end else begin
//		if (E_PASS_one)
//			FromDDR_addr_valid	<= 1'b0;
//		else
//			FromDDR_addr_valid	<= FromDDR_addr_valid;
//	end
//end
//always@(posedge ddr_clk) begin
//	if (ddr_reset)
//		FromDDR_addr_fifo_rden	<= 1'b0;
//	else if (~FromDDR_addr_valid & ~FromDDR_addr_fifo_rden & ~FromDDR_addr_fifo_empty)
//		FromDDR_addr_fifo_rden	<= 1'b1;
//	else
//		FromDDR_addr_fifo_rden	<= 1'b0;
//end
//
//always@(posedge ddr_clk) begin
//	if (ddr_reset) begin
//		egress_fifo_rden	<= 1'b0;
//		egress_cnt			<= 9'b000000_000;
//		egress_pathindex	<= 2'b00;
//		egress_state		<= E_IDLE;
//	end else begin
//		case(egress_state)
//			E_IDLE: begin
//				if (FromDDR_addr_valid &
//						(	(FromDDR_addr_fifo_rddata[27:26]==2'b00 & ~ToFRL_data_fifo_pfull) |
//							(FromDDR_addr_fifo_rddata[27:26]==2'b01 & ~ToFRL_2nd_data_fifo_pfull) |
//							(FromDDR_addr_fifo_rddata[27:26]==2'b10 & ~ToFRL_3rd_data_fifo_pfull) |
//							(FromDDR_addr_fifo_rddata[27:26]==2'b11 & ~ToFRL_4th_data_fifo_pfull)	) &
//						~egress_fifo_pempty
//						) begin
//					egress_fifo_rden	<= 1'b1;
//					egress_pathindex	<= FromDDR_addr_fifo_rddata[27:26];
//					case(FromDDR_addr_fifo_rddata[25:23])
//						3'b001:  egress_cnt <= 9'b000001_000 - 9'b000000_001;   //128B
//						3'b010:  egress_cnt <= 9'b000010_000 - 9'b000000_001;   //256B
//						3'b011:  egress_cnt <= 9'b000100_000 - 9'b000000_001;   //512B
//						3'b100:  egress_cnt <= 9'b001000_000 - 9'b000000_001;   //1KB
//						3'b101:  egress_cnt <= 9'b010000_000 - 9'b000000_001;   //2KB
//						3'b110:  egress_cnt <= 9'b100000_000 - 9'b000000_001;   //4KB                
//						default: egress_cnt <= 9'b000001_000 - 9'b000000_001; //default to 128B      
//					endcase
//					egress_state		<= E_PASS;
//				end else begin
//					egress_fifo_rden	<= 1'b0;
//					egress_pathindex	<= egress_pathindex;
//					egress_cnt			<= 9'b000000_000;
//					egress_state		<= E_IDLE;
//				end
//			end
//			E_PASS: begin
//				if (egress_cnt == 9'b000000_000) begin
//					egress_fifo_rden	<= 1'b0;	
//					egress_cnt			<= egress_cnt;
//					egress_state		<= E_IDLE;
//				end else begin
//					if (egress_fifo_pempty | ToFRL_data_fifo_almost_full_this) begin		// flow control, while the block size increases to 1kB, flow control becomes necessary
//						egress_fifo_rden	<= 1'b0;
//						egress_cnt			<= egress_cnt;
//					end else begin
//						egress_fifo_rden	<= 1'b1;
//						egress_cnt			<= egress_cnt - 9'b000000_001;
//					end
//					egress_state		<= E_PASS;
//				end
//				egress_pathindex	<= egress_pathindex;
//			end
//			default: begin
//				egress_fifo_rden	<= 1'b0;
//				egress_pathindex	<= egress_pathindex;
//				egress_cnt			<= 9'b000000_000;
//				egress_state		<= E_IDLE;
//			end
//		endcase
//	end
//end

assign ToFRL_data_fifo_pfull_this	= FromDDR_addr_fifo_rddata[27] ?
														(FromDDR_addr_fifo_rddata[26] ?
															ToFRL_4th_data_fifo_pfull : ToFRL_3rd_data_fifo_pfull) :
														(FromDDR_addr_fifo_rddata[26] ?
															ToFRL_2nd_data_fifo_pfull : ToFRL_data_fifo_pfull);

assign ToFRL_data_fifo_almost_full_this	= FromDDR_addr_fifo_rddata[27] ? 
																(FromDDR_addr_fifo_rddata[26] ? 
																	ToFRL_4th_data_fifo_almost_full : ToFRL_3rd_data_fifo_almost_full) : 
																(FromDDR_addr_fifo_rddata[26] ?
																	ToFRL_2nd_data_fifo_almost_full : ToFRL_data_fifo_almost_full);

always@(posedge ddr_clk)	ToFRL_data_fifo_wren	<= egress_fifo_rden & (FromDDR_addr_fifo_rddata[27:26]==2'b00);
assign	ToFRL_data_fifo_wrdata = egress_fifo_rddata[127:0];
always@(posedge ddr_clk)	ToFRL_2nd_data_fifo_wren	<= egress_fifo_rden & (FromDDR_addr_fifo_rddata[27:26]==2'b01);
assign	ToFRL_2nd_data_fifo_wrdata = egress_fifo_rddata[127:0];
`ifdef MIMO_4X4
always@(posedge ddr_clk)	ToFRL_3rd_data_fifo_wren	<= egress_fifo_rden & (FromDDR_addr_fifo_rddata[27:26]==2'b10);
assign	ToFRL_3rd_data_fifo_wrdata = egress_fifo_rddata[127:0];
always@(posedge ddr_clk)	ToFRL_4th_data_fifo_wren	<= egress_fifo_rden & (FromDDR_addr_fifo_rddata[27:26]==2'b11);
assign	ToFRL_4th_data_fifo_wrdata = egress_fifo_rddata[127:0];
`endif //MIMO_4X4

FromDDR_addr_cntrl_fifo FromDDR_addr_cntrl_fifo_inst(
	.clk			(ddr_clk),
	.rst			(ddr_reset),
	.din			(FromDDR_addr_fifo_wrdata[31:0]), //32
	.wr_en		(FromDDR_addr_fifo_wren),
	.rd_en		(FromDDR_addr_fifo_rden),
	.dout			(FromDDR_addr_fifo_rddata[31:0]), //32
	.empty		(FromDDR_addr_fifo_empty),
	.full			(FromDDR_addr_fifo_full)
);

//egress_addr_cntrl_fifo ToDDR_addr_cntrl_fifo(
//	.din          (ToDDR_addr_fifo_wrdata[31:0]), //32
//	.rd_clk       (ddr_clk),
//	.rd_en        (ToDDR_addr_fifo_ren),
//	.rst          (reset),
//	.wr_clk       (dma_clk),
//	.wr_en        (ToDDR_addr_fifo_wren),
//	.dout         (ToDDR_addr_fifo_rddata[31:0]), //32
//	.empty        (ToDDR_addr_fifo_empty),
//	.full         (ToDDR_addr_fifo_full)
//);

/// FIFOs for TX requests
egress_addr_cntrl_fifo egress_addr_cntrl_fifo(
	.din          (egress_addr_fifo_wrdata[31:0]), //16
	.rd_clk       (ddr_clk),
	.rd_en        (egress_addr_fifo_rden),
	.rst          (reset),
	.wr_clk       (dma_clk),
	.wr_en        (egress_addr_fifo_wren),
//	.almost_empty (egress_addr_fifo_almost_empty),
//	.almost_full  (egress_addr_fifo_almost_full),
	.dout         (egress_addr_fifo_rddata[31:0]), //16
	.empty        (egress_addr_fifo_empty),
	.full         (egress_addr_fifo_full)
);
egress_addr_cntrl_fifo egress_2nd_addr_cntrl_fifo(
	.din          (egress_2nd_addr_fifo_wrdata[31:0]), //16
	.rd_clk       (ddr_clk),
	.rd_en        (egress_2nd_addr_fifo_rden),
	.rst          (reset),
	.wr_clk       (dma_clk),
	.wr_en        (egress_2nd_addr_fifo_wren),
	.dout         (egress_2nd_addr_fifo_rddata[31:0]), //16
	.empty        (egress_2nd_addr_fifo_empty),
	.full         (egress_2nd_addr_fifo_full)
);
`ifdef MIMO_4X4
egress_addr_cntrl_fifo egress_3rd_addr_cntrl_fifo(
	.din          (egress_3rd_addr_fifo_wrdata[31:0]), //16
	.rd_clk       (ddr_clk),
	.rd_en        (egress_3rd_addr_fifo_rden),
	.rst          (reset),
	.wr_clk       (dma_clk),
	.wr_en        (egress_3rd_addr_fifo_wren),
	.dout         (egress_3rd_addr_fifo_rddata[31:0]), //16
	.empty        (egress_3rd_addr_fifo_empty),
	.full         (egress_3rd_addr_fifo_full)
);
egress_addr_cntrl_fifo egress_4th_addr_cntrl_fifo(
	.din          (egress_4th_addr_fifo_wrdata[31:0]), //16
	.rd_clk       (ddr_clk),
	.rd_en        (egress_4th_addr_fifo_rden),
	.rst          (reset),
	.wr_clk       (dma_clk),
	.wr_en        (egress_4th_addr_fifo_wren),
	.dout         (egress_4th_addr_fifo_rddata[31:0]), //16
	.empty        (egress_4th_addr_fifo_empty),
	.full         (egress_4th_addr_fifo_full)
);
`endif //MIMO_4X4

/// Jiansong: added for ingress/transfer requests
ingress_addr_cntrl_fifo ingress_addr_cntrl_fifo(
	.din          (ingress_addr_fifo_wrdata[31:0]), //128
	.rd_clk       (ddr_clk),
	.rd_en        (ingress_addr_fifo_rden),
	.rst          (reset),
	.wr_clk       (dma_clk),
	.wr_en        (ingress_addr_fifo_wren),
	.dout         (ingress_addr_fifo_rddata[31:0]), //128
	.prog_full		(ingress_addr_fifo_pfull),
	.empty        (ingress_addr_fifo_empty),
	.full         (ingress_addr_fifo_full)
//	.almost_empty (ingress_addr_fifo_almost_empty),
//	.almost_full  (ingress_addr_fifo_almost_full),
);

//////ADDRESS/CNTRL FIFO to cross clock domains  32X64
////addr_cntrl_fifo addr_cntrl_fifo_inst(
////.din          (addr_fifo_wrdata[31:0]), //32
////.rd_clk       (ddr_clk),
////.rd_en        (addr_fifo_ren),
////.rst          (reset),
////.wr_clk       (dma_clk),
////.wr_en        (addr_fifo_wren),
////.almost_empty (addr_fifo_almost_empty),
////.almost_full  (addr_fifo_almost_full),
////.dout         (addr_fifo_rddata[31:0]), //32
////.empty        (addr_fifo_empty),
////.full         (addr_fifo_full)
////);
////// END ADDRESS/CNTRL FIFO

/// Jiansong: generate egress_overflow_one signal
rising_edge_detect egress_overflow_one_inst(
                .clk(ddr_clk),
                .rst(reset),
                .in(egress_fifo_full),
                .one_shot_out(egress_overflow_one)
                );

ToFRL_data_fifo ToFRL_data_fifo_inst(
	.rst			(ddr_reset),
	.wr_clk		(ddr_clk),
	.rd_clk		(radio_clk),
	.din			(ToFRL_data_fifo_wrdata[127:0]), // Bus [127 : 0] 
	.wr_en		(ToFRL_data_fifo_wren),
	.rd_en		(ToFRL_data_fifo_rden),
	.dout			(ToFRL_data_fifo_rddata[31:0]), // Bus [31 : 0] 
	.full			(ToFRL_data_fifo_full),
	.almost_full(ToFRL_data_fifo_almost_full),
	.empty		(ToFRL_data_fifo_empty),
	.prog_full	(ToFRL_data_fifo_pfull),	// 768 of 128			/// liuchang: 2047 of 128
	.prog_empty	(ToFRL_data_fifo_pempty)	// 7 of 32
);

ToFRL_data_fifo ToFRL_2nd_data_fifo_inst(
	.rst			(ddr_reset),
	.wr_clk		(ddr_clk),
	.rd_clk		(radio_2nd_clk),
	.din			(ToFRL_2nd_data_fifo_wrdata[127:0]), // Bus [127 : 0] 
	.wr_en		(ToFRL_2nd_data_fifo_wren),
	.rd_en		(ToFRL_2nd_data_fifo_rden),
	.dout			(ToFRL_2nd_data_fifo_rddata[31:0]), // Bus [31 : 0] 
	.full			(ToFRL_2nd_data_fifo_full),
	.almost_full(ToFRL_2nd_data_fifo_almost_full),
	.empty		(ToFRL_2nd_data_fifo_empty),
	.prog_full	(ToFRL_2nd_data_fifo_pfull),	// 768 of 128
	.prog_empty	(ToFRL_2nd_data_fifo_pempty)	// 7 of 32
);
`ifdef MIMO_4X4
ToFRL_data_fifo ToFRL_3rd_data_fifo_inst(
	.rst			(ddr_reset),
	.wr_clk		(ddr_clk),
	.rd_clk		(radio_3rd_clk),
	.din			(ToFRL_3rd_data_fifo_wrdata[127:0]), // Bus [127 : 0] 
	.wr_en		(ToFRL_3rd_data_fifo_wren),
	.rd_en		(ToFRL_3rd_data_fifo_rden),
	.dout			(ToFRL_3rd_data_fifo_rddata[31:0]), // Bus [31 : 0] 
	.full			(ToFRL_3rd_data_fifo_full),
	.almost_full(ToFRL_3rd_data_fifo_almost_full),
	.empty		(ToFRL_3rd_data_fifo_empty),
	.prog_full	(ToFRL_3rd_data_fifo_pfull),	// 768 of 128
	.prog_empty	(ToFRL_3rd_data_fifo_pempty)	// 7 of 32
);

ToFRL_data_fifo ToFRL_4th_data_fifo_inst(
	.rst			(ddr_reset),
	.wr_clk		(ddr_clk),
	.rd_clk		(radio_4th_clk),
	.din			(ToFRL_4th_data_fifo_wrdata[127:0]), // Bus [127 : 0] 
	.wr_en		(ToFRL_4th_data_fifo_wren),
	.rd_en		(ToFRL_4th_data_fifo_rden),
	.dout			(ToFRL_4th_data_fifo_rddata[31:0]), // Bus [31 : 0] 
	.full			(ToFRL_4th_data_fifo_full),
	.almost_full(ToFRL_4th_data_fifo_almost_full),
	.empty		(ToFRL_4th_data_fifo_empty),
	.prog_full	(ToFRL_4th_data_fifo_pfull),	// 768 of 128
	.prog_empty	(ToFRL_4th_data_fifo_pempty)	// 7 of 32
);
`endif //MIMO_4X4

//pipeline the egress write enable and data from the memory cntrl
//and write into the egress fifos         
always@(posedge ddr_clk)begin 
	  if(ddr_reset)begin
		  egress_fifo_wren <= 1'b0;
	  end else begin
		  egress_fifo_wren <= m_data_valid;
	  end
end
always@(posedge ddr_clk)	m_rddata_reg[127:0] <= m_rddata[127:0]; 
// size = 8KB
Egress_data_FIFO Egress_data_FIFO_inst (
	.rst              (ddr_reset),
	.clk					(ddr_clk),
////   .din              (m_rddata_reg[127:0]),
//   .din              ({m_rddata_reg[15:0],m_rddata_reg[31:16],m_rddata_reg[47:32],
//	                    m_rddata_reg[63:48],m_rddata_reg[79:64],m_rddata_reg[95:80],
//							  m_rddata_reg[111:96],m_rddata_reg[127:112]}),
   // 32b data width or 16-bit IQ
	.din					({m_rddata_reg[31:0],m_rddata_reg[63:32],
	                    m_rddata_reg[95:64],m_rddata_reg[127:96]}),
//	.wr_clk           (ddr_clk),
//	.rd_en            (egress_fifo_ctrl[0]),
	.rd_en            (egress_fifo_rden),
	.wr_en            (egress_fifo_wren),
	.dout             (egress_fifo_rddata[127:0]),
//	.rd_clk           (radio_clk),
	.empty            (egress_fifo_empty),
//	.empty				(),
	.full             (egress_fifo_full),
	.prog_empty       (egress_fifo_pempty),	// 7 of 128				/// liuchang: 2 of 128
	.prog_full        (egress_fifo_pfull)		//240/512 of 128		/// liuchang: 1919 of 128
);

/// Jiansong: data count
always@(posedge ddr_clk)begin
   if(reset)
	   egress_wr_data_count <= 32'h0000_0000;
	else if (egress_fifo_wren)
	   egress_wr_data_count <= egress_wr_data_count + 32'h0000_0001;
	else
	   egress_wr_data_count <= egress_wr_data_count;
end

// Jiansong: it is replaced by an IP core fifo. The new fifo is from 200MHz to 
//           44MHz (or 40MHz). The old one is from 200MHz to 250MHz. So the timing
//           requirement is relaxed.
/////// Jiansong: why not use IP core?
//////the egress fifos are built using two fifo36 primitives in 
//////parallel - they have been placed in a wrapper because the
//////read data path and empty signal has been pipelined for timing purposes 
//////(mainly because of tight timing on the empty signal) and
//////require some complex logic to support - the signals
//////at this wrapper interface appear to the user exactly like
//////a regular fifo i.e. there is no extra clock of latency on the
//////read datapath due to the pipeline.
////egress_fifo_wrapper egress_fifo_wrapper_inst(
////        .RST(reset),
////        .WRCLK(ddr_clk),
////        .WREN(egress_fifo_wren),
////        .DI(m_rddata_reg[127:0]),
////        .FULL(egress_full),
////        .ALMOSTFULL(egress_pfull),
////        .RDCLK(dma_clk),
////        .RDEN(egress_fifo_ctrl[0]),
////        .DO(egress_data[127:0]),
////        .EMPTY(empty),
////        .ALMOSTEMPTY(egress_pempty)
////);


//assign egress_fifo_status[2] = egress_pfull;
//assign egress_fifo_status[1] = egress_empty;
//assign egress_fifo_status[0] = egress_pempty;
//assign egress_fifo_pempty = egress_pempty;

ingress_data_fifo ingress_data_fifo_inst(
	.din				(ingress_data[127:0]),
	.rd_clk			(ddr_clk),
	.rd_en			(ingress_fifo_rden),
	.rst				(reset),
	.wr_clk			(dma_clk),
//	.wr_en			(ingress_fifo_ctrl[0]),
	.wr_en			(ingress_fifo_wren),
	.dout				(m_wrdata[127:0]),
	.empty			(ingress_fifo_empty),
//	.empty			(),
	.almost_empty	(ingress_fifo_almostempty),
	.full				(ingress_fifo_full),
//	.full				(),
	.prog_empty		(ingress_fifo_pempty),	// set to half of the fifo, used for transfer flow control
	.prog_full		(ingress_fifo_pfull)		// flow control from rx engine
//	.rd_data_count	(rdcount[8:0]),
//	.wr_data_count	(wrcount[8:0])
);

/// Jiansong: the parameter ALMOST_EMPTY_OFFSET not realy works, don't know why
////
//////INGRESS or WRITE to DDR2 MEMORY FIFO
////FIFO36_72 #( 
//////.ALMOST_EMPTY_OFFSET (9'h005),
////.ALMOST_EMPTY_OFFSET (9'h100),   // Jiansong: if (~almost empty), assert pause_read_requests
////.ALMOST_FULL_OFFSET  (9'h1C4),
////.DO_REG              (1),
////.EN_ECC_WRITE        ("FALSE"),
////.EN_ECC_READ         ("FALSE"),
////.EN_SYN              ("FALSE"),
////.FIRST_WORD_FALL_THROUGH ("FALSE"))
////ingress_fifo_a(
////.ALMOSTEMPTY (ingress_fifo_pempty_a), 
////.ALMOSTFULL  (ingress_fifo_status_a[0]),  
////.DBITERR     (), 
////.DO          (m_wrdata[63:0]), 
////.DOP         (), 
////.ECCPARITY   (), 
////.EMPTY       (ingress_fifo_empty_a), 
////.FULL        (ingress_fifo_status_a[1]), 
////.RDCOUNT     (), 
////.RDERR       (), 
////.SBITERR     (), 
////.WRCOUNT     (), 
////.WRERR       (),          
////.DI          (ingress_data[63:0]), 
////.DIP         (), 
////.RDCLK       (ddr_clk), 
////.RDEN        (ingress_fifo_rden), 
////.RST         (reset), 
////.WRCLK       (dma_clk), 
////.WREN        (ingress_fifo_ctrl[0])
////);
////
////FIFO36_72 #( 
//////.ALMOST_EMPTY_OFFSET (9'h005),
////.ALMOST_EMPTY_OFFSET (9'h100),   // Jiansong: if (~almost empty), assert pause_read_requests
////.ALMOST_FULL_OFFSET  (9'h1C4),
////.DO_REG              (1),
////.EN_ECC_WRITE        ("FALSE"),
////.EN_ECC_READ         ("FALSE"),
////.EN_SYN              ("FALSE"),
////.FIRST_WORD_FALL_THROUGH ("FALSE"))
////ingress_fifo_b(
////.ALMOSTEMPTY (ingress_fifo_pempty_b), 
////.ALMOSTFULL  (ingress_fifo_status_b[0]), 
////.DBITERR     (), 
////.DO          (m_wrdata[127:64]), 
////.DOP         (), 
////.ECCPARITY   (), 
////.EMPTY       (ingress_fifo_empty_b), 
////.FULL        (ingress_fifo_status_b[1]), 
////.RDCOUNT     (rdcount[8:0]), 
////.RDERR       (), 
////.SBITERR     (), 
////.WRCOUNT     (wrcount[8:0]), 
////.WRERR       (),          
////.DI          (ingress_data[127:64]), 
////.DIP         (), 
////.RDCLK       (ddr_clk), 
////.RDEN        (ingress_fifo_rden), 
////.RST         (reset), 
////.WRCLK       (dma_clk), 
////.WREN        (ingress_fifo_ctrl[0])
////);


///////////////////////////////////////////////////////////////////////////
//////
////// Block-based transfer detector needed when DDR2 clock is faster
////// than PCIe clock i.e. ML505.  Otherwise could use empty flag.
//////
////// Monitors the rdcount and wrcount of the ingress_fifo to determine
////// when there is at least 64B block of data in the fifo
//////
////// signal at_least_64B is fed to the ddr state machine
//////
///////////////////////////////////////////////////////////////////////////
//////Binary to Gray encode wrcount                 /// interesting
////always@(posedge dma_clk)begin
////        wrcount_gray_dma[8:0] = {wrcount[8], wrcount[8:1] ^ wrcount[7:0]};
////end
////
//////transfer to ddr_clock domain
////always@(posedge ddr_clk)begin
////     wrcount_gray_ddr  <= wrcount_gray_ddr1;
////     wrcount_gray_ddr1 <= wrcount_gray_dma;
////end
////
//////Gray to Binary decode wrcount_gray_ddr and register the output
////assign wrcount_ddr[8:0] = {wrcount_gray_ddr[8], 
////                               wrcount_ddr[8:1] ^ wrcount_gray_ddr[7:0]};
////always@(posedge ddr_clk)
////      wrcount_ddr_reg[8:0] <= wrcount_ddr[8:0];
////
//////need to pipeline rdcount since DO_REG is set to 1
////always@(posedge ddr_clk)begin
////        if(ingress_fifo_rden) rdcount_reg <= rdcount;
////end
////
//////do a compare - if read count is 4 (or more) less than write count than that
//////means there is at least 64B of data in the ingress fifo and I can
//////safely do a 64B block read of the fifo
////always@(posedge ddr_clk)begin
////      if(ddr_reset)begin
////          at_least_64B <= 1'b0;
////      end else begin
////          at_least_64B <= ((wrcount_ddr_reg[8:0] - rdcount_reg[8:0]) >= 9'h004) 
////                           ? 1'b1 : 1'b0;
////      end
////end



//Careful with the fifo status signals when using two fifos in parallel

//Empty flags (and Almost Empty flags) which are synchronous to rdclk
//could deassert on different rdclk cycles due to minute differences in the
//wrclk arrival time (wrclk clock skew).  This is because deassertion 
//is caused by writing data into an empty fifo i.e. a wrclk domain event
//and this event must cross clock domains. 
//Assertion is caused by reading the last piece of data out of the fifo. 
//Since rden is a rdclk domain signal/event it is guaranteed that both fifos 
//will assert empty on the same rdclk cycle (as long as rden and rdclk are
//are the same signals for both fifos)

//Similarily the Full flags (and almost full flags) which are synchronous to
//wrclk could deassert on different wrclk cycles due to minute differences
//in the rdclk arrival time (rdclk clock skew).

//In both cases the flags should be wire or'ed (since they are positive logic)
//so that the flag doesn't deassert unless both flags are deasserted
//assign ingress_fifo_pempty = ingress_fifo_pempty_a | ingress_fifo_pempty_b;
//assign ingress_fifo_pempty = ingress_fifo_pempty_a & ingress_fifo_pempty_b;
//assign ingress_fifo_empty = ingress_fifo_empty_a | ingress_fifo_empty_b;
//assign ingress_fifo_status[1:0] = ingress_fifo_status_a[1:0] 
//                                  | ingress_fifo_status_b[1:0];



always@(posedge ddr_clk) begin
	DebugDDRSignals[0]	<= ~egress_addr_fifo_full;
	DebugDDRSignals[1]	<= ~egress_2nd_addr_fifo_full;
	DebugDDRSignals[2]	<= ~egress_3rd_addr_fifo_full;
	DebugDDRSignals[3]	<= ~egress_4th_addr_fifo_full;
	DebugDDRSignals[4]	<= egress_addr_fifo_empty;
	DebugDDRSignals[5]	<= egress_2nd_addr_fifo_empty;
	DebugDDRSignals[6]	<= egress_3rd_addr_fifo_empty;
	DebugDDRSignals[7]	<= egress_4th_addr_fifo_empty;
	DebugDDRSignals[8]	<= ~ingress_fifo_pfull;
	DebugDDRSignals[9]	<= ~ingress_addr_fifo_full;
	DebugDDRSignals[10]	<= ~ingress_addr_fifo_pfull;
	DebugDDRSignals[11]	<= ToDDR_addr_valid;
	DebugDDRSignals[12]	<= ~FromDDR_addr_fifo_full;
	DebugDDRSignals[13]	<= FromDDR_addr_fifo_empty;
//	DebugDDRSignals[14]	<= ~FromDDR_addr_valid;
//	DebugDDRSignals[15]
	DebugDDRSignals[16]	<= ~m_af_afull;
	DebugDDRSignals[17]	<= ~m_wdf_afull;
//	DebugDDRSignals[19:18]
	DebugDDRSignals[20]	<= egress_fifo_pempty;
	DebugDDRSignals[21]	<= ~egress_fifo_pfull;
	DebugDDRSignals[22]	<= egress_fifo_empty;
	DebugDDRSignals[23]	<= ~egress_fifo_full;
	DebugDDRSignals[24]	<= ~ToFRL_data_fifo_pfull;
	DebugDDRSignals[25]	<= ~ToFRL_2nd_data_fifo_pfull;
	DebugDDRSignals[26]	<= ~ToFRL_3rd_data_fifo_pfull;
	DebugDDRSignals[27]	<= ~ToFRL_4th_data_fifo_pfull;
	DebugDDRSignals[28]	<= ToFRL_data_fifo_pempty;
	DebugDDRSignals[29]	<= ToFRL_2nd_data_fifo_pempty;
	DebugDDRSignals[30]	<= ToFRL_3rd_data_fifo_pempty;
	DebugDDRSignals[31]	<= ToFRL_4th_data_fifo_pempty;
end

always@(posedge ddr_clk) begin
	DebugDDRSMs[2:0]	<= token[2:0];	
	DebugDDRSMs[8:4]	<= ddr_state[4:0];
end


always@(posedge ddr_clk) begin
	if (ddr_reset)
		DebugDDRFIFOFullCnt[31:16]	<= 16'h0000;
	else if (ingress_fifo_full)
		DebugDDRFIFOFullCnt[31:16]	<= DebugDDRFIFOFullCnt[31:16] + 16'h0001;
	else
		DebugDDRFIFOFullCnt[31:16]	<= DebugDDRFIFOFullCnt[31:16];
end
always@(posedge ddr_clk) begin
	if (ddr_reset)
		DebugDDRFIFOFullCnt[15:0]	<= 16'h0000;
	else if (egress_fifo_full)
		DebugDDRFIFOFullCnt[15:0]	<= DebugDDRFIFOFullCnt[15:0] + 16'h0001;
	else
		DebugDDRFIFOFullCnt[15:0]	<= DebugDDRFIFOFullCnt[15:0];
end

always@(posedge ddr_clk) begin
	if (ddr_reset)
		DebugDDREgressFIFOCnt[31:16]	<= 16'h0000;
	else if (egress_fifo_wren)
		DebugDDREgressFIFOCnt[31:16]	<= DebugDDREgressFIFOCnt[31:16] + 16'h0001;
	else
		DebugDDREgressFIFOCnt[31:16]	<= DebugDDREgressFIFOCnt[31:16];
end
always@(posedge ddr_clk) begin
	if (ddr_reset)
		DebugDDREgressFIFOCnt[15:0]	<= 16'h0000;
	else if (egress_fifo_rden)
		DebugDDREgressFIFOCnt[15:0]	<= DebugDDREgressFIFOCnt[15:0] + 16'h0001;
	else
		DebugDDREgressFIFOCnt[15:0]	<= DebugDDREgressFIFOCnt[15:0];
end

`ifdef sora_chipscope

	reg	TX_Start_one_ddr;
	always@(posedge ddr_clk) TX_Start_one_ddr	<= TX_Start_one;

	ila ila_inst (
		 .CONTROL(CONTROL0), // INOUT BUS [35:0]
		 .CLK(ddr_clk), // IN
		 .DATA({	egress_fifo_empty,
					egress_fifo_rden,
					egress_fifo_full,
					egress_fifo_wren,
					egress_fifo_pempty,
					ToFRL_data_fifo_pfull_this,
					ToFRL_data_fifo_pfull,
					ToFRL_2nd_data_fifo_pfull,
					ToFRL_3rd_data_fifo_pfull,
					ToFRL_4th_data_fifo_pfull,
					FromDDR_addr_fifo_empty,
					FromDDR_addr_fifo_rden,
					FromDDR_addr_fifo_full,
					FromDDR_addr_fifo_wren,
					FromDDR_addr_fifo_rddata[27:26],
					FromDDR_addr_fifo_rddata[25:23],
					egress_cnt[8:0],
					ToFRL_data_fifo_wren,
					ToFRL_2nd_data_fifo_wren
					}), // IN BUS [17:0]
		 .TRIG0(TX_Start_one_ddr) // IN BUS [0:0]
	);
`endif

endmodule
