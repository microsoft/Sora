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
// Purpose: Tx engine wrapper file.  Connects  
//          accompanying fifos to the TX TRN State Machine module.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
// Modification by zjs, 2009-6-18, pending
//    (1) move posted packet generator and non-posted packet generator out --- done 
//    (2) add dma write data fifo --------------- done
//    (3) modify tx sm
//          scheduling -------------------------- done
//          disable write dma done -------------- done
//          register/memory read ---------------- done
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module tx_engine(
    input clk,
//    input rst,
//	 input hostreset,
	input	full_rst,
	input	soft_rst,

    /// Jiansong: new interface added for register read / memory read operation
    //interface to DMA control wrapper, for register data
	 input  [31:0] Mrd_data_in,    // 32 bits register read assumed
	 output [11:0] Mrd_data_addr,

    //interface to PCIE Endpoint Block Plus
    input [15:0] pcie_id,
    output [63:0] trn_td,
    output [7:0] trn_trem_n,
    output trn_tsof_n,
    output trn_teof_n,
    output trn_tsrc_rdy_n,
    output trn_tsrc_dsc_n, 
    input trn_tdst_rdy_n,
    input trn_tdst_dsc_n,
    output trn_terrfwd_n,
    input [2:0] trn_tbuf_av,

	/// Jiansong: interface to non_posted_pkt_gen
   input        non_posted_fifo_wren,
   input [63:0] non_posted_fifo_data,

	/// Jiansong: interface to posted_pkt_gen
   input        posted_fifo_wren,
   input [63:0] posted_fifo_data,
	output       posted_fifo_full,

	 /// Jiansong: interface from posted_pkt_gen to dma write data fifo
	 input [63:0] dma_write_data_fifo_data,
	 input        dma_write_data_fifo_wren,
	 output       dma_write_data_fifo_full,	 

	/// Jiansong: keep completion logic in TX engine
    //interface to RX Engine
    input [6:0] bar_hit,
    input MRd,                
    input MWr,   
    input [31:0] MEM_addr,
    input [15:0] MEM_req_id,
    input [7:0] MEM_tag,
    input header_fields_valid,
	 // Jiansong: input from rx_monitor
	 input       rd_dma_start,  //indicates the start of a read dma xfer
    input  [12:3] dmarxs        //size of the complete transfer
//	 input [9:0] np_rx_cnt_qw,
//	 input       transferstart,
// 	 input       Wait_for_TX_desc
	 // debug interface
//	 output [31:0] Debug21RX2,
//	 output [31:0] Debug25RX6,
//	 output [7:0]  FIFOErrors
);
   
   wire posted_hdr_fifo_rden;
   wire [63:0] posted_hdr_fifo;
   wire posted_hdr_fifo_empty;

   wire non_posted_hdr_fifo_rden;
   wire [63:0] non_posted_hdr_fifo;
   wire non_posted_hdr_fifo_empty;
   wire non_posted_hdr_fifo_full; 

   wire comp_fifo_wren;
   wire [63:0] comp_fifo_data;
   wire comp_hdr_fifo_rden;
   wire [63:0] comp_hdr_fifo;
   wire comp_hdr_fifo_empty;
   wire comp_hdr_fifo_full;

   /// Jiansong: posted data fifo interface
	wire [63:0] posted_data_fifo_data;
	wire        posted_data_fifo_rden;
	wire        posted_data_fifo_empty;
	wire        posted_data_fifo_real_full;
	
	/// Jiansong
	wire rst_tx;
	
//	/// FIFO errors
//	reg  p_hdr_fifo_overflow;
//	reg  p_hdr_fifo_underflow;
//	reg  p_data_fifo_overflow;
//	reg  p_data_fifo_underflow;
//	reg  cmp_hdr_fifo_overflow;
//	reg  cmp_hdr_fifo_underflow;
//	reg  np_hdr_fifo_overflow;
//	reg  np_hdr_fifo_underflow;
	
//	assign FIFOErrors[0] = p_hdr_fifo_overflow;
//	assign FIFOErrors[1] = p_hdr_fifo_underflow;
//	assign FIFOErrors[2] = p_data_fifo_overflow;
//	assign FIFOErrors[3] = p_data_fifo_underflow;
//	assign FIFOErrors[4] = cmp_hdr_fifo_overflow;
//	assign FIFOErrors[5] = cmp_hdr_fifo_underflow;
//	assign FIFOErrors[6] = np_hdr_fifo_overflow;
//	assign FIFOErrors[7] = np_hdr_fifo_underflow;
//	
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    p_hdr_fifo_overflow <= 1'b0;
//		else if (posted_fifo_full & posted_fifo_wren)
//		    p_hdr_fifo_overflow <= 1'b1;
//		else
//		    p_hdr_fifo_overflow <= p_hdr_fifo_overflow;
//	end
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    p_hdr_fifo_underflow <= 1'b0;
//		else if (posted_hdr_fifo_empty & posted_hdr_fifo_rden)
//		    p_hdr_fifo_underflow <= 1'b1;
//		else
//		    p_hdr_fifo_underflow <= p_hdr_fifo_underflow;
//	end
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    p_data_fifo_overflow <= 1'b0;
//		else if (posted_data_fifo_real_full & dma_write_data_fifo_wren)
//		    p_data_fifo_overflow <= 1'b1;
//		else
//		    p_data_fifo_overflow <= p_data_fifo_overflow;
//	end
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    p_data_fifo_underflow <= 1'b0;
//		else if (posted_data_fifo_empty & posted_data_fifo_rden)
//		    p_data_fifo_underflow <= 1'b1;
//		else
//		    p_data_fifo_underflow <= p_data_fifo_underflow;
//	end
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    cmp_hdr_fifo_overflow <= 1'b0;
//		else if (comp_hdr_fifo_full & comp_fifo_wren)
//		    cmp_hdr_fifo_overflow <= 1'b1;
//		else
//		    cmp_hdr_fifo_overflow <= cmp_hdr_fifo_overflow;
//	end
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    cmp_hdr_fifo_underflow <= 1'b0;
//		else if (comp_hdr_fifo_empty & comp_hdr_fifo_rden)
//		    cmp_hdr_fifo_underflow <= 1'b1;
//		else
//		    cmp_hdr_fifo_underflow <= cmp_hdr_fifo_underflow;
//	end
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    np_hdr_fifo_overflow <= 1'b0;
//		else if (non_posted_hdr_fifo_full & non_posted_fifo_wren)
//		    np_hdr_fifo_overflow <= 1'b1;
//		else
//		    np_hdr_fifo_overflow <= np_hdr_fifo_overflow;
//	end
//	always@(posedge clk) begin
//	   if (rst_tx)
//		    np_hdr_fifo_underflow <= 1'b0;
//		else if (non_posted_hdr_fifo_empty & non_posted_hdr_fifo_rden)
//		    np_hdr_fifo_underflow <= 1'b1;
//		else
//		    np_hdr_fifo_underflow <= np_hdr_fifo_underflow;
//	end

   /// Jiansong: timing solution, what does it mean?
//   //register and dup wr_dma_start_one for 250 MHz timing;
//   (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg wr_dma_start_one_reg1, 
//                                            wr_dma_start_one_reg2;
////	reg wr_dma_start_one_reg1;

/// Jiansong: why the depth is 128? Performance consideration?
/// Jiansong: full signal is never used
// Fifo is a 64 x 128 coregen fifo made out of distributed ram   
  a64_128_distram_fifo a64_128_distram_fifo_p(  
   .clk(clk),
   .rst(rst_tx),
   //interface to posted_pkt_gen
   .din(posted_fifo_data),
   .wr_en(posted_fifo_wren),
////   .full(posted_hdr_fifo_full),
   .full(posted_fifo_full),
   //interface to tx_trn_sm
   .dout(posted_hdr_fifo),
   .rd_en(posted_hdr_fifo_rden),
   .empty(posted_hdr_fifo_empty)
);   
 
  /// Jiansong: full signal is never used
  //Fifo is a 64 x 64 coregen fifo made out of distributed ram   
  a64_64_distram_fifo a64_64_distram_fifo_np(  
   .clk(clk),
   .rst(rst_tx),
   //inteface to non_posted_pkt_gen
   .din(non_posted_fifo_data),
   .wr_en(non_posted_fifo_wren),
   .full(non_posted_hdr_fifo_full),
   //interface to tx_trn_sm
   .dout(non_posted_hdr_fifo),
   .rd_en(non_posted_hdr_fifo_rden),
   .empty(non_posted_hdr_fifo_empty)
   );      
   
// Instantiate the completer packet generator
   completer_pkt_gen completer_pkt_gen_inst (
      .clk(clk), 
      .rst(rst_tx), 
      //interface to RX Engine (except comp_id from PCIe block) 
      .bar_hit(bar_hit[6:0]),
      .comp_req(MRd & header_fields_valid),                 
      .MEM_addr(MEM_addr[31:0]),
      .MEM_req_id(MEM_req_id[15:0]),
      .comp_id(pcie_id[15:0]), //req_id becomes completer id
      .MEM_tag(MEM_tag[7:0]),
      //inteface to completion header fifo (a64_64_distram_fifo_comp)
      .comp_fifo_wren(comp_fifo_wren), 
      .comp_fifo_data(comp_fifo_data[63:0])
   );
   
  //Fifo is a 64 x 64 coregen fifo made out of distributed ram   
  a64_64_distram_fifo a64_64_distram_fifo_comp(  
   .clk(clk),
   .rst(rst_tx),
   //interface to completer_pkt_gen
   .din(comp_fifo_data[63:0]),
   .wr_en(comp_fifo_wren),
   .full(comp_hdr_fifo_full),
   //interface to tx_trn_sm
   .dout(comp_hdr_fifo),
   .rd_en(comp_hdr_fifo_rden),
   .empty(comp_hdr_fifo_empty)
 ); 

/// Jiansong: Data TRN DMA Write FIFO, pending
//Instantiate the Data TRN DMA Write FIFO
//This is an 4KB FIFO constructed of BRAM
//Provides buffering for RX data and RX descriptor
data_trn_dma_write_fifo data_trn_dma_write_fifo_inst(
   .din    (dma_write_data_fifo_data),
   .rd_en  (posted_data_fifo_rden),
   .rst    (rst_tx),
   .clk    (clk),
   .wr_en  (dma_write_data_fifo_wren),
   .dout   (posted_data_fifo_data),
   .empty  (posted_data_fifo_empty),
   .full   (posted_data_fifo_real_full),
   .prog_full (dma_write_data_fifo_full)	
);
	 
  //Instantiate the TRN interface state machine
   tx_trn_sm tx_trn_sm_inst   (
      .clk(clk), 
//      .rst_in(rst),
//		.hostreset_in(hostreset),
		.full_rst(full_rst),
		.soft_rst(soft_rst),
      .rst_out(rst_tx),		
      //interface to the header fifos
      .posted_hdr_fifo(posted_hdr_fifo), 
      .posted_hdr_fifo_rden(posted_hdr_fifo_rden), 
      .posted_hdr_fifo_empty(posted_hdr_fifo_empty), 
      .nonposted_hdr_fifo(non_posted_hdr_fifo), 
      .nonposted_hdr_fifo_rden(non_posted_hdr_fifo_rden), 
      .nonposted_hdr_fifo_empty(non_posted_hdr_fifo_empty), 
      .comp_hdr_fifo(comp_hdr_fifo), 
      .comp_hdr_fifo_empty(comp_hdr_fifo_empty),
      .comp_hdr_fifo_rden(comp_hdr_fifo_rden),
		/// Jiansong: posted data fifo interface
		.posted_data_fifo_data(posted_data_fifo_data),
		.posted_data_fifo_rden(posted_data_fifo_rden),
		.posted_data_fifo_empty(posted_data_fifo_empty),

      .Mrd_data_addr(Mrd_data_addr),		
      .Mrd_data_in(Mrd_data_in),
		
      //interface to PCIe Endpoint Block Plus TX TRN
      .trn_td(trn_td[63:0]),           //O [63:0]
      .trn_trem_n(trn_trem_n[7:0]),    //O [7:0]
      .trn_tsof_n(trn_tsof_n),         //O
      .trn_teof_n(trn_teof_n),         //O
      .trn_tsrc_rdy_n(trn_tsrc_rdy_n), //O
      .trn_tsrc_dsc_n(trn_tsrc_dsc_n), //O
      .trn_tdst_rdy_n(trn_tdst_rdy_n), //I
      .trn_tdst_dsc_n(trn_tdst_dsc_n), //I
      .trn_terrfwd_n(trn_terrfwd_n),   //O
      .trn_tbuf_av(trn_tbuf_av[2:0]),  //I [3:0]
		/// Jiansong: input from rx_monitor
		.rd_dma_start(rd_dma_start),
		.dmarxs(dmarxs)
//		.np_rx_cnt_qw(np_rx_cnt_qw),
//		.transferstart (transferstart),
// 	   .Wait_for_TX_desc(Wait_for_TX_desc)
	   // debug interface
//	   .Debug21RX2(Debug21RX2),
//		.Debug25RX6(Debug25RX6)		
   );

endmodule
