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
// Company:  Microsoft Research Asia
// Engineer: Jiansong Zhang
// 
// Create Date:    21:19:22 06/22/2009 
// Design Name: 
// Module Name:    posted_pkt_gen_sora 
// Project Name:   Sora
// Target Devices: virtex-5 LX50T
// Tool versions:  ISE 10.1.02
// Description: This module is created by Jiansong Zhang. Main purpose of this module is 
//              to generate posted packet header and data. We have two data sources: RX path
//              and TX descriptor write back. 
//              On RX path, when every 28DW data is arrived, we 
//              generates three PCIe packets: (1) 28DW data packet (2) RX descriptor for next 
//              data block, valid bit is 0 (3) RX descriptor for this data block, valid bit is 1.
//              This design facilitates driver for recognizing the latest data in RX buffer.
//              For TX desc write back, we write back the TX descriptor and set own bit to 0.
//              TX desc write back has higher priority. 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps
`include "Sora_config.v"

module posted_pkt_gen_sora(
    input clk, 
    input rst, 
    //interface from/to dma_ctrl_wrapper
    /// Jiansong: TX desc write back
	 input        TX_desc_write_back_req,
	 output       TX_desc_write_back_ack,
	 input [63:0] SourceAddr,
    input [31:0] DestAddr,
    input [23:0] FrameSize,
    input [7:0]  FrameControl,
	 input [63:0] DescAddr,             /// Jiansong: pending
	 /// Jiansong: RX path
	 input        RXEnable,
	 input [63:0] RXBufAddr,
	 input [31:0] RXBufSize,
	 input        RXEnable_2nd,
	 input [63:0] RXBufAddr_2nd,
	 input [31:0] RXBufSize_2nd,
`ifdef MIMO_4X4
	 input        RXEnable_3rd,
	 input [63:0] RXBufAddr_3rd,
	 input [31:0] RXBufSize_3rd,
	 input        RXEnable_4th,
	 input [63:0] RXBufAddr_4th,
	 input [31:0] RXBufSize_4th,
`endif //MIMO_4X4
    //interface to PCIe Endpoint Block Plus
	 input [2:0]  max_pay_size,
    input [15:0] req_id,
    //interface to posted header fifo (a64_128_distram_fifo_p)
    output        posted_fifo_wren, 
    output [63:0] posted_fifo_data,
    input         posted_fifo_full,   /// pending
	 //interface to dma write data fifo in TX engine
	 output [63:0] dma_write_data_fifo_data,
	 output        dma_write_data_fifo_wren,
	 input         dma_write_data_fifo_full,	 
	 //interface to RX data fifo
	 input [63:0] RX_FIFO_data,
	 output       RX_FIFO_RDEN,
	 input        RX_FIFO_pempty,
	 input [31:0] RX_TS_FIFO_data,
	 output       RX_TS_FIFO_RDEN,
	 input        RX_TS_FIFO_empty,
	 input [63:0] RX_FIFO_2nd_data,
	 output       RX_FIFO_2nd_RDEN,
	 input        RX_FIFO_2nd_pempty,
	 input [31:0] RX_TS_FIFO_2nd_data,
	 output       RX_TS_FIFO_2nd_RDEN,
	 input        RX_TS_FIFO_2nd_empty,
`ifdef MIMO_4X4
	 input [63:0] RX_FIFO_3rd_data,
	 output       RX_FIFO_3rd_RDEN,
	 input        RX_FIFO_3rd_pempty,
	 input [31:0] RX_TS_FIFO_3rd_data,
	 output       RX_TS_FIFO_3rd_RDEN,
	 input        RX_TS_FIFO_3rd_empty,
	 input [63:0] RX_FIFO_4th_data,
	 output       RX_FIFO_4th_RDEN,
	 input        RX_FIFO_4th_pempty,
	 input [31:0] RX_TS_FIFO_4th_data,
	 output       RX_TS_FIFO_4th_RDEN,
	 input        RX_TS_FIFO_4th_empty
`endif //MIMO_4X4
		// interface to 2nd RX data fifo
//`ifdef SORA_FRL_2nd
//	 input [63:0] RX_FIFO_2nd_data,
//	 output       RX_FIFO_2nd_RDEN,
//	 input        RX_FIFO_2nd_pempty,
//`endif
	 // debug interface
//	 output [31:0] Debug20RX1,
////	 output [4:0]  Debug22RX3,
//	 output [31:0] Debug24RX5,
//	 output [31:0] Debug26RX7,
//	 output [31:0] Debug27RX8,
//	 output [31:0] Debug28RX9,
//	 output [31:0] Debug29RX10
    );

//internal wrapper connections
wire [63:0] dmawad_reg;
wire [9:0]  length;
wire ack,go; //handshake signals

//`ifdef SORA_FRL_2nd		// two radios
//    posted_pkt_scheduler_2radios posted_pkt_scheduler_2radios_inst (
//	              .clk(clk),
//					  .rst(rst),
//					  //interface to PCIe Endpoint Block Plus
//					  .max_pay_size(max_pay_size),
//					  //interface from/to RX data fifo
//					  .RX_FIFO_data           (RX_FIFO_data),
//					  .RX_FIFO_RDEN           (RX_FIFO_RDEN),
//					  .RX_FIFO_pempty         (RX_FIFO_pempty),
//					  //interface from/to the 2nd RX data fifo
//					  .RX_FIFO_2nd_data           (RX_FIFO_2nd_data),
//					  .RX_FIFO_2nd_RDEN           (RX_FIFO_2nd_RDEN),
//					  .RX_FIFO_2nd_pempty         (RX_FIFO_2nd_pempty),
//					  //interface from/to dma ctrl wrapper
//					  /// TX descriptor write back
//					  .TX_desc_write_back_req (TX_desc_write_back_req),
//					  .TX_desc_write_back_ack (TX_desc_write_back_ack),
//					  .SourceAddr             (SourceAddr),
//					  .DestAddr               (DestAddr),
//					  .FrameSize              (FrameSize),
//					  .FrameControl           (FrameControl),
//	              .DescAddr               (DescAddr),
//					  /// RX control signals
//					  .RXEnable               (RXEnable),
//					  .RXBufAddr              (RXBufAddr),
//					  .RXBufSize              (RXBufSize),
//					  /// RX control signals for the 2nd RX buffer
//					  .RXBufAddr_2nd          (RXBufAddr_2nd),
//					  .RXBufSize_2nd          (RXBufSize_2nd),					  
//					  //interface from/to dma write data fifo in TX engine
//					  .dma_write_data_fifo_data (dma_write_data_fifo_data),
//					  .dma_write_data_fifo_wren (dma_write_data_fifo_wren),
//					  .dma_write_data_fifo_full (dma_write_data_fifo_full),
//					  //interface to posted pkt builder
//					  .go(go),
//					  .ack(ack),
//					  .dmawad(dmawad_reg[63:0]),
//					  .length(length[9:0]),
//					  //interface from a64_128_distram_p(posted header fifo)
//					  .posted_fifo_full(posted_fifo_full),
//	              // debug interface
//	              .Debug20RX1(Debug20RX1),
//	              .Debug22RX3(Debug22RX3),
//					  .Debug24RX5(Debug24RX5),
//					  .Debug26RX7(Debug26RX7),
//					  .Debug27RX8(Debug27RX8),
//					  .Debug28RX9(Debug28RX9),
//					  .Debug29RX10(Debug29RX10)
//	              );
//`endif

`ifdef MIMO_4X4	// 4 radios
    posted_pkt_scheduler_4radios posted_pkt_scheduler_4radios_inst (
	              .clk(clk),
					  .rst(rst),
					  //interface to PCIe Endpoint Block Plus
					  .max_pay_size(max_pay_size),
					  //interface from/to RX data fifo
					  .RX_FIFO_data				(RX_FIFO_data),
					  .RX_FIFO_RDEN				(RX_FIFO_RDEN),
					  .RX_FIFO_pempty				(RX_FIFO_pempty),
					  .RX_TS_FIFO_data			(RX_TS_FIFO_data),
					  .RX_TS_FIFO_RDEN			(RX_TS_FIFO_RDEN),
					  .RX_TS_FIFO_empty			(RX_TS_FIFO_empty),
					  //interface from/to the 2nd RX data fifo
					  .RX_FIFO_2nd_data			(RX_FIFO_2nd_data),
					  .RX_FIFO_2nd_RDEN			(RX_FIFO_2nd_RDEN),
					  .RX_FIFO_2nd_pempty		(RX_FIFO_2nd_pempty),
					  .RX_TS_FIFO_2nd_data		(RX_TS_FIFO_2nd_data),
					  .RX_TS_FIFO_2nd_RDEN		(RX_TS_FIFO_2nd_RDEN),
					  .RX_TS_FIFO_2nd_empty		(RX_TS_FIFO_2nd_empty),
					  //interface from/to the 3rd RX data fifo
					  .RX_FIFO_3rd_data			(RX_FIFO_3rd_data),
					  .RX_FIFO_3rd_RDEN			(RX_FIFO_3rd_RDEN),
					  .RX_FIFO_3rd_pempty		(RX_FIFO_3rd_pempty),
					  .RX_TS_FIFO_3rd_data		(RX_TS_FIFO_3rd_data),
					  .RX_TS_FIFO_3rd_RDEN		(RX_TS_FIFO_3rd_RDEN),
					  .RX_TS_FIFO_3rd_empty		(RX_TS_FIFO_3rd_empty),
					  //interface from/to the 4th RX data fifo
					  .RX_FIFO_4th_data			(RX_FIFO_4th_data),
					  .RX_FIFO_4th_RDEN			(RX_FIFO_4th_RDEN),
					  .RX_FIFO_4th_pempty		(RX_FIFO_4th_pempty),
					  .RX_TS_FIFO_4th_data		(RX_TS_FIFO_4th_data),
					  .RX_TS_FIFO_4th_RDEN		(RX_TS_FIFO_4th_RDEN),
					  .RX_TS_FIFO_4th_empty		(RX_TS_FIFO_4th_empty),
					  //interface from/to dma ctrl wrapper
					  /// TX descriptor write back
					  .TX_desc_write_back_req (TX_desc_write_back_req),
					  .TX_desc_write_back_ack (TX_desc_write_back_ack),
					  .SourceAddr             (SourceAddr),
					  .DestAddr               (DestAddr),
					  .FrameSize              (FrameSize),
					  .FrameControl           (FrameControl),
	              .DescAddr               (DescAddr),
					  /// RX control signals
					  .RXEnable               (RXEnable),
					  .RXBuf_1stAddr          (RXBufAddr),
					  .RXBuf_1stSize          (RXBufSize),
					  /// RX control signals for the 2nd RX buffer
					  .RXEnable_2nd           (RXEnable_2nd),
					  .RXBuf_2ndAddr          (RXBufAddr_2nd),
					  .RXBuf_2ndSize          (RXBufSize_2nd),
					  /// RX control signals for the 3rd RX buffer
					  .RXEnable_3rd           (RXEnable_3rd),
					  .RXBuf_3rdAddr          (RXBufAddr_3rd),
					  .RXBuf_3rdSize          (RXBufSize_3rd),
					  /// RX control signals for the 4th RX buffer
					  .RXEnable_4th           (RXEnable_4th),
					  .RXBuf_4thAddr          (RXBufAddr_4th),
					  .RXBuf_4thSize          (RXBufSize_4th),					  
					  //interface from/to dma write data fifo in TX engine
					  .dma_write_data_fifo_data (dma_write_data_fifo_data),
					  .dma_write_data_fifo_wren (dma_write_data_fifo_wren),
					  .dma_write_data_fifo_full (dma_write_data_fifo_full),
					  //interface to posted pkt builder
					  .go(go),
					  .ack(ack),
					  .dmawad(dmawad_reg[63:0]),
					  .length(length[9:0]),
					  //interface from a64_128_distram_p(posted header fifo)
					  .posted_fifo_full(posted_fifo_full)
	              // debug interface
//	              .Debug20RX1(Debug20RX1),
////	              .Debug22RX3(Debug22RX3),
//					  .Debug24RX5(Debug24RX5),
//					  .Debug26RX7(Debug26RX7),
//					  .Debug27RX8(Debug27RX8),
//					  .Debug28RX9(Debug28RX9),
//					  .Debug29RX10(Debug29RX10)
	              );
`endif //MIMO_4X4

`ifdef single_radio
    posted_pkt_scheduler posted_pkt_scheduler_inst (
	              .clk(clk),
					  .rst(rst),
					  //interface to PCIe Endpoint Block Plus
					  .max_pay_size(max_pay_size),
					  //interface from/to RX data fifo
					  .RX_FIFO_data           (RX_FIFO_data[63:0]),
					  .RX_FIFO_RDEN           (RX_FIFO_RDEN),
					  .RX_FIFO_pempty         (RX_FIFO_pempty),
					  .RX_TS_FIFO_data        (RX_TS_FIFO_data[31:0]),
					  .RX_TS_FIFO_RDEN        (RX_TS_FIFO_RDEN),
					  .RX_TS_FIFO_empty       (RX_TS_FIFO_empty),
					  //interface from/to dma ctrl wrapper
					  /// TX descriptor write back
					  .TX_desc_write_back_req (TX_desc_write_back_req),
					  .TX_desc_write_back_ack (TX_desc_write_back_ack),
					  .SourceAddr             (SourceAddr),
					  .DestAddr               (DestAddr),
					  .FrameSize              (FrameSize),
					  .FrameControl           (FrameControl),
	              .DescAddr               (DescAddr),
					  /// RX control signals
					  .RXEnable               (RXEnable),
					  .RXBufAddr              (RXBufAddr),
					  .RXBufSize              (RXBufSize),
					  //interface from/to dma write data fifo in TX engine
					  .dma_write_data_fifo_data (dma_write_data_fifo_data),
					  .dma_write_data_fifo_wren (dma_write_data_fifo_wren),
					  .dma_write_data_fifo_full (dma_write_data_fifo_full),
					  //interface to posted pkt builder
					  .go(go),
					  .ack(ack),
					  .dmawad(dmawad_reg[63:0]),
					  .length(length[9:0]),
					  //interface from a64_128_distram_p(posted header fifo)
					  .posted_fifo_full(posted_fifo_full),
	              // debug interface
	              .Debug20RX1(Debug20RX1),
	              .Debug22RX3(Debug22RX3),
					  .Debug24RX5(Debug24RX5),
					  .Debug26RX7(Debug26RX7),
					  .Debug27RX8(Debug27RX8),
					  .Debug28RX9(Debug28RX9),
					  .Debug29RX10(Debug29RX10)
	              );
`endif //single_radio

    /// build posted packet header and put in fifo
    posted_pkt_builder posted_pkt_builder_inst(    
                .clk(clk),
                .rst(rst),
                .req_id(req_id[15:0]),//from pcie block
                //interface to posted_pkt_scheduler
					 .posted_fifo_full(posted_fifo_full),
                .go(go),
                .ack(ack),
                .dmawad(dmawad_reg[63:0]),
                .length(length[9:0]),
                //interface to/from a64_128_distram_p(posted header fifo)
                .header_data_out(posted_fifo_data[63:0]),
                .header_data_wren(posted_fifo_wren)
                );

endmodule
