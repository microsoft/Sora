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
// Module Name:    pcie_dma_wrapper 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  DMA user application wrapper.  Connects the RX Engine, TX engine,
//          read request fifo, egress data presenter, register file, and misc. 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//              PCIe endpoint_blk_plus Logicore version 1.9 is used.
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps
`include "Sora_config.v"

module pcie_dma_wrapper(
    input clk,
//    input hard_rst,
//	 input rst,
	input		full_rst,
	input		soft_rst,
	 // hot reset to whole system
	 output hostreset_o,
	 /// Jiansong: interface to RX data fifo
	 input  [63:0] RX_FIFO_data,
	 output        RX_FIFO_RDEN,
	 input         RX_FIFO_pempty,	 
	 input  [31:0] RX_TS_FIFO_data,
	 output        RX_TS_FIFO_RDEN,
	 input         RX_TS_FIFO_empty,
	 input  [63:0] RX_FIFO_2nd_data,
	 output        RX_FIFO_2nd_RDEN,
	 input         RX_FIFO_2nd_pempty,	 
	 input  [31:0] RX_TS_FIFO_2nd_data,
	 output        RX_TS_FIFO_2nd_RDEN,
	 input         RX_TS_FIFO_2nd_empty,
`ifdef MIMO_4X4
	 input  [63:0] RX_FIFO_3rd_data,
	 output        RX_FIFO_3rd_RDEN,
	 input         RX_FIFO_3rd_pempty,	 
	 input  [31:0] RX_TS_FIFO_3rd_data,
	 output        RX_TS_FIFO_3rd_RDEN,
	 input         RX_TS_FIFO_3rd_empty,
	 input  [63:0] RX_FIFO_4th_data,
	 output        RX_FIFO_4th_RDEN,
	 input         RX_FIFO_4th_pempty,	 
	 input  [31:0] RX_TS_FIFO_4th_data,
	 output        RX_TS_FIFO_4th_RDEN,
	 input         RX_TS_FIFO_4th_empty,
`endif //MIMO_4X4	 	 
	 output    		RXEnable_o,
	 /// Jiansong: interface to radio module
//	 input         Radio_TX_done,
//	 output        Radio_TX_start,
	 output        TX_Ongoing,
	 
	 output			TX_DDR_data_req,
	 input			TX_DDR_data_ack,
	 output [27:6]	TX_DDR_start_addr,
	 output [2:0]	TX_DDR_xfer_size,

	 output			TX_2nd_DDR_data_req,
	 input			TX_2nd_DDR_data_ack,
	 output [27:6]	TX_2nd_DDR_start_addr,
	 output [2:0]	TX_2nd_DDR_xfer_size,

`ifdef MIMO_4X4
	 output			TX_3rd_DDR_data_req,
	 input			TX_3rd_DDR_data_ack,
	 output [27:6]	TX_3rd_DDR_start_addr,
	 output [2:0]	TX_3rd_DDR_xfer_size,

	 output			TX_4th_DDR_data_req,
	 input			TX_4th_DDR_data_ack,
	 output [27:6]	TX_4th_DDR_start_addr,
	 output [2:0]	TX_4th_DDR_xfer_size,
`endif //MIMO_4X4
	 
    //interface to dma_ddr2_if
	output [2:0]	ingress_xfer_size,
	output [27:6]	ingress_start_addr,
	output			ingress_data_req,
	input				ingress_data_ack,
//    input [1:0]		ingress_fifo_status,
//    output [1:0]		ingress_fifo_ctrl,
	output			ingress_fifo_wren,
	output [127:0]	ingress_data,
	 
    input 				pause_read_requests,

    //interface to pcie block plus 
    input [2:0] pcie_max_pay_size,
    input [2:0] pcie_max_read_req,
    input [12:0] pcie_id,
    output comp_timeout,
	 
    //Tx local-link to pcie block plus 
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
    
	 // Rx Local-Link to pcie block plus
    input [63:0] trn_rd,
    input [7:0] trn_rrem_n,
    input trn_rsof_n,
    input trn_reof_n,
    input trn_rsrc_rdy_n,
    input trn_rsrc_dsc_n,
    output trn_rdst_rdy_n,
    input trn_rerrfwd_n,
    output trn_rnp_ok_n,
    input [6:0] trn_rbar_hit_n,
    input [11:0] trn_rfc_npd_av,
    input [7:0] trn_rfc_nph_av,
    input [11:0] trn_rfc_pd_av,
    input [7:0] trn_rfc_ph_av,
    input [11:0] trn_rfc_cpld_av,
    input [7:0] trn_rfc_cplh_av,
    output trn_rcpl_streaming_n,
	 
	 /// Jiansong: error inputs
	 input egress_overflow_one,
	 input RX_FIFO_full,
    input [31:0] egress_rd_data_count,
    input [31:0] egress_wr_data_count,
    
	 //Signals from memory controller
    input phy_init_done,
	 // hardware status input
	 input trn_lnk_up_n_c,
	 // radio related inputs/outputs

`ifdef RADIO_CHANNEL_REGISTERS

	 output [31:0] Radio_Cmd_Data,
	 output [6:0]  Radio_Cmd_Addr,
	 output 			Radio_Cmd_RdWr,
	 output 			Radio_Cmd_wren,
	 input [31:0]	Channel_Reg_Read_Value,
	 input [7:0]	Channel_Reg_Read_Addr,
	 input			Channel_ReadDone_in,
/// registers for 2nd to 4th paths/radios
	 output [31:0] Radio_2nd_Cmd_Data,
	 output [6:0]  Radio_2nd_Cmd_Addr,
	 output 			Radio_2nd_Cmd_RdWr,
	 output 			Radio_2nd_Cmd_wren,
	 input [31:0]	Channel_2nd_Reg_Read_Value,
	 input [7:0]	Channel_2nd_Reg_Read_Addr,
	 input			Channel_2nd_ReadDone_in,
`ifdef MIMO_4X4
	 output [31:0] Radio_3rd_Cmd_Data,
	 output [6:0]  Radio_3rd_Cmd_Addr,
	 output 			Radio_3rd_Cmd_RdWr,
	 output 			Radio_3rd_Cmd_wren,
	 input [31:0]	Channel_3rd_Reg_Read_Value,
	 input [7:0]	Channel_3rd_Reg_Read_Addr,
	 input			Channel_3rd_ReadDone_in,
	 output [31:0] Radio_4th_Cmd_Data,
	 output [6:0]  Radio_4th_Cmd_Addr,
	 output 			Radio_4th_Cmd_RdWr,
	 output 			Radio_4th_Cmd_wren,
	 input [31:0]	Channel_4th_Reg_Read_Value,
	 input [7:0]	Channel_4th_Reg_Read_Addr,
	 input			Channel_4th_ReadDone_in,	
`endif //MIMO_4X4
	 
`endif //RADIO_CHANNEL_REGISTERS

    // Debug interface
	 output	TX_Start_one,
	 
	 input [31:0]	DebugRX1Overflowcount_in,
	 input [31:0]	DebugRX2Overflowcount_in,
	 
	 input [31:0]		DebugDDREgressFIFOCnt,
	 input [31:0]		DebugDDRFIFOFullCnt,
	 input [31:0]		DebugDDRSignals,
	 input [8:0]		DebugDDRSMs,
	 
	 input [15:0] PCIeLinkStatus,
	 input [15:0] PCIeLinkControl
    );
	  
	  reg  [15:0] PCIeLinkStatus_r;
	  reg  [15:0] PCIeLinkControl_r;
	  	  
	wire [4:0]	DDR2MaxBurstSize;
	reg [4:0]	DDR2MaxBurstSize_r;
		  
	wire [31:0]	TX_Addr;
	wire [31:0]	TX_Size;
	wire			TX_Start_2nd_one;	
	wire [31:0]	TX_Addr_2nd;
	wire [31:0]	TX_Size_2nd;
`ifdef MIMO_4X4
	wire			TX_Start_3rd_one;	
	wire [31:0]	TX_Addr_3rd;
	wire [31:0]	TX_Size_3rd;
	wire			TX_Start_4th_one;	
	wire [31:0]	TX_Addr_4th;
	wire [31:0]	TX_Size_4th;
`endif //MIMO_4X4

 
	/// Jiansong: pipeline registers
	 reg egress_overflow_one_r;
	 reg RX_FIFO_full_r;
    reg [31:0] egress_rd_data_count_r;
    reg [31:0] egress_wr_data_count_r;

  wire [63:0] dmaras;
  wire [31:0] dmarad;
  wire [31:0] dmarxs;
  wire rd_dma_start;  
  wire rd_dma_done;

  /// Jiansong: TX desc write back, from dma ctrl wrapper to posted pkt generator
  wire TX_desc_write_back_req;
  wire TX_desc_write_back_ack;
  wire [63:0] SourceAddr_out;
  wire [31:0] DestAddr_out;
  wire [23:0] FrameSize_out;
  wire [7:0]  FrameControl_out;
  wire [63:0] DescAddr_out;
  /// Jiansong: RX paths
  wire        RXEnable;
  wire [63:0] RXBufAddr;
  wire [31:0] RXBufSize;
  wire        RXEnable_2nd;
  wire [63:0] RXBufAddr_2nd;
  wire [31:0] RXBufSize_2nd;
`ifdef MIMO_4X4
  wire        RXEnable_3rd;
  wire [63:0] RXBufAddr_3rd;
  wire [31:0] RXBufSize_3rd;
  wire        RXEnable_4th;
  wire [63:0] RXBufAddr_4th;
  wire [31:0] RXBufSize_4th;
`endif //MIMO_4X4

  //pipeline registers
  reg [63:0] dmaras_r1;
  reg [31:0] dmarad_r1;
  reg [31:0] dmarxs_r1;
  reg rd_dma_start_r1;  
  reg rd_dma_done_r1;
  /// Jiansong: TX desc write back
  reg TX_desc_write_back_req_r;
  reg TX_desc_write_back_ack_r;
  reg [63:0] SourceAddr_r_out;
  reg [31:0] DestAddr_r_out;
  reg [23:0] FrameSize_r_out;
  reg [7:0]  FrameControl_r_out;
  reg [63:0] DescAddr_r_out;
  
  //signals between RX, TX Engines and the Read Request Wrapper
  wire [4:0] tx_waddr;
  wire [31:0] tx_wdata;
  wire tx_we;
  wire [31:0] completion_pending;
  wire [4:0]  rx_waddr;
  wire [31:0]  rx_wdata;
  wire  rx_we;
  wire [4:0]  rx_raddr;
  wire [31:0] rx_rdata;
  wire pending_comp_done;

  //signals between RX and dma_ctrl_wrapper
  wire [6:0]   bar_hit;
  wire         MRd;               
  wire         MWr;   
  wire [31:0]  MEM_addr;
  wire [15:0]  MEM_req_id;
  wire [7:0]   MEM_tag;
  wire         header_fields_valid;
  wire [31:0]  write_data;
  wire         write_data_wren; 
//  reg  [31:0]  MEM_addr_r;
  reg  [11:0]  MEM_addr_r;
  reg          reg_wren_r;
  reg [31:0]   write_data_r;
  wire         read_last;

  //signals between performance_counters and dma_ctrl_wrapper
//  wire [31:0] dma_wr_count, dma_rd_count;

  reg   rst_reg;
  
  /// Jiansong:
  ///     wires and regs added
  ///
  wire rd_TX_des_start;
  reg  rd_TX_des_start_r1;
  wire [63:0] TX_des_addr;
  
  wire non_posted_fifo_wren;
  wire [63:0] non_posted_fifo_data;

  wire posted_fifo_full;
  wire posted_fifo_wren;
  wire [63:0] posted_fifo_data;
	
  wire np_tag_gnt, np_tag_inc;
  
  wire [11:0] Mrd_data_addr;
  wire [31:0] Mrd_data;
  
  /// TX descriptor received, from RX engine to dma ctrl wrapper
  wire                new_des_one;
  wire        [31:0]  SourceAddr_L_in;
  wire        [31:0]  SourceAddr_H_in;
  wire        [31:0]  DestAddr_in;
  wire        [23:0]  FrameSize_in;
  wire        [7:0]   FrameControl_in;
  reg                new_des_one_r_in;
  reg        [31:0]  SourceAddr_L_r_in;
  reg        [31:0]  SourceAddr_H_r_in;
  reg        [31:0]  DestAddr_r_in;
  reg        [23:0]  FrameSize_r_in;
  reg        [7:0]   FrameControl_r_in;
  /// to RX Engine
  wire               Wait_for_TX_desc;
  reg                Wait_for_TX_desc_r;
  /// from register file to rx_engine, non_posted_pkt_gen and read_req_wrapper
  wire               transferstart;
  reg                transferstart_r;
  wire					transferstart_one;
  reg						transferstart_one_r;
  wire					set_transfer_done_bit;
  reg						set_transfer_done_bit_r;
  wire [63:0]			TransferSrcAddr;
  
  // dma write data fifo interface, between posted_pkt_gen and tx engine
  wire [63:0]  dma_write_data_fifo_data;
  wire         dma_write_data_fifo_wren;
  wire         dma_write_data_fifo_full;
  	 
   //internal wires for hooking up the two blocks in this wrapper module
   wire [31:0] reg_data_in_int;
   wire [6:0]  reg_wr_addr_int;
   wire        reg_wren_int;
   wire        rd_dma_done_int;
	 

  wire [7:0] tag_value;
  
	always@(posedge clk) rst_reg <= full_rst | soft_rst;

	transfer_controller transfer_controller_inst(
		.clk			(clk),
		.rst			(rst_reg),
		// triggers
`ifdef TF_RECOVERY
		.transferstart			(transferstart_r),
`endif
		.transferstart_one	(transferstart_one_r),
		.set_transfer_done_bit	(set_transfer_done_bit),
		.TransferSrcAddr			(TransferSrcAddr[63:0]),
		/// Jiansong: signals from RX Engine, TX desc recived
      .new_des_one             (new_des_one_r_in),
      .SourceAddr_L            (SourceAddr_L_r_in),
      .SourceAddr_H            (SourceAddr_H_r_in),
      .DestAddr                (DestAddr_r_in),
      .FrameSize               (FrameSize_r_in),
      .FrameControl            (FrameControl_r_in),
       ///Jiansong: interface to RX engine, indicate the system is in dma read for TX desc
	    ///          when this signal is asserted, received cpld will not be count in 
	    ///          length subtraction
      .Wait_for_TX_desc			(Wait_for_TX_desc),

	    /// Jiansong: interface to/from posted_packet_generator
		 /// TX desc write back
	    .TX_desc_write_back_req  (TX_desc_write_back_req),
	    .TX_desc_write_back_ack  (TX_desc_write_back_ack_r),
	    .SourceAddr_r            (SourceAddr_out),
       .DestAddr_r              (DestAddr_out),
       .FrameSize_r             (FrameSize_out),
       .FrameControl_r          (FrameControl_out),
		 .DescAddr_r              (DescAddr_out),

     //interface to Internal DMA Control module
      .reg_data_in_o(reg_data_in_int[31:0]),
      .reg_wr_addr_o(reg_wr_addr_int[6:0]),
      .reg_wren_o(reg_wren_int),
      .rd_dma_done_o(rd_dma_done_int),

		/// Jiansong: interface to non_posted packet generator, start signal for tx desc
      .rd_TX_des_start	(rd_TX_des_start),      /// Jiansong: start signal for TX des
		.TX_des_addr		(TX_des_addr),
		
      //done signals from RX and TX Engines
      .rd_dma_done_i		(rd_dma_done_r1), //done signal actually comes from RX engine

      .read_last(read_last), //connects to multiple logic blocks

      //interface from dma_ddr2_if
      .pause_read_requests(pause_read_requests)

	);
	
	// work together with transfer controller 
	internal_dma_ctrl internal_dma_ctrl_inst (
      .clk(clk), 
      .rst(rst_reg), 

      //interface from DMA Control and Status Register File
      .reg_data_in(reg_data_in_int[31:0]), 
      .reg_wr_addr(reg_wr_addr_int[6:0]),
      .reg_wren(reg_wren_int),
      .rd_dma_done(rd_dma_done_int),

      //interface to TX Engine
      .dmaras(dmaras), 
      .dmarad(dmarad), 
      .dmarxs(dmarxs), 
      .rd_dma_start(rd_dma_start), 
       
      //unused inputs/outputs
      .reg_rd_addr(7'b0000000),
      .reg_data_out(),
      .dma_wr_count(32'h00000000),
      .dma_rd_count(32'h00000000)
	);

	// controller for (legacy) TX
	TX_controller_noloss TX_controller_inst(
		.clk					(clk),
		.rst					(rst_reg),
		
		.DDR2MaxBurstSize	(DDR2MaxBurstSize_r[4:0]),
		// triggers
		.TX_Start_one		(TX_Start_one),
		.TX_Addr				(TX_Addr[31:0]),
		.TX_Size				(TX_Size[31:0]),
		// interface to dma_ddr2_if (DDR memory controller/scheduler)
	   .TX_data_req		(TX_DDR_data_req),
	   .TX_data_ack		(TX_DDR_data_ack),
	   .TX_Start_addr		(TX_DDR_start_addr),
	   .TX_xfer_size		(TX_DDR_xfer_size)
	);
	
	TX_controller_noloss TX_controller_2nd_inst(
		.clk					(clk),
		.rst					(rst_reg),
		
		.DDR2MaxBurstSize	(DDR2MaxBurstSize_r[4:0]),
		// triggers
		.TX_Start_one		(TX_Start_2nd_one),
		.TX_Addr				(TX_Addr_2nd[31:0]),
		.TX_Size				(TX_Size_2nd[31:0]),
		// interface to dma_ddr2_if (DDR memory controller/scheduler)
	   .TX_data_req		(TX_2nd_DDR_data_req),
	   .TX_data_ack		(TX_2nd_DDR_data_ack),
	   .TX_Start_addr		(TX_2nd_DDR_start_addr),
	   .TX_xfer_size		(TX_2nd_DDR_xfer_size)
	);

`ifdef MIMO_4X4
	TX_controller_noloss TX_controller_3rd_inst(
		.clk					(clk),
		.rst					(rst_reg),
		
		.DDR2MaxBurstSize	(DDR2MaxBurstSize_r[4:0]),
		// triggers
		.TX_Start_one		(TX_Start_3rd_one),
		.TX_Addr				(TX_Addr_3rd[31:0]),
		.TX_Size				(TX_Size_3rd[31:0]),
		// interface to dma_ddr2_if (DDR memory controller/scheduler)
	   .TX_data_req		(TX_3rd_DDR_data_req),
	   .TX_data_ack		(TX_3rd_DDR_data_ack),
	   .TX_Start_addr		(TX_3rd_DDR_start_addr),
	   .TX_xfer_size		(TX_3rd_DDR_xfer_size)
	);

	TX_controller_noloss TX_controller_4th_inst(
		.clk					(clk),
		.rst					(rst_reg),
		
		.DDR2MaxBurstSize	(DDR2MaxBurstSize_r[4:0]),
		// triggers
		.TX_Start_one		(TX_Start_4th_one),
		.TX_Addr				(TX_Addr_4th[31:0]),
		.TX_Size				(TX_Size_4th[31:0]),
		// interface to dma_ddr2_if (DDR memory controller/scheduler)
	   .TX_data_req		(TX_4th_DDR_data_req),
	   .TX_data_ack		(TX_4th_DDR_data_ack),
	   .TX_Start_addr		(TX_4th_DDR_start_addr),
	   .TX_xfer_size		(TX_4th_DDR_xfer_size)
	);
`endif //MIMO_4X4
	
	//  dma_ctrl_wrapper dma_ctrl_wrapper_inst (
  firmware_ctrl_wrapper firmware_ctrl_wrapper_inst (
      .clk(clk), 
		.rst(rst_reg), 

      // hot reset to whole system
	   .hostreset(hostreset_o),
		
		.DDR2MaxBurstSize(DDR2MaxBurstSize[4:0]),
	 
	   /// Jiansong: interface to radio module
//		.Radio_TX_done(Radio_TX_done_r),
//	   .Radio_TX_start(Radio_TX_start),
		.TX_Ongoing_o(TX_Ongoing),

	   .TX_Start_one_o		(TX_Start_one),
		.TX_Addr					(TX_Addr[31:0]),
		.TX_Size					(TX_Size[31:0]),
	   .TX_Start_2nd_one_o	(TX_Start_2nd_one),
		.TX_Addr_2nd			(TX_Addr_2nd[31:0]),
		.TX_Size_2nd			(TX_Size_2nd[31:0]),
`ifdef MIMO_4X4
	   .TX_Start_3rd_one_o	(TX_Start_3rd_one),
		.TX_Addr_3rd			(TX_Addr_3rd[31:0]),
		.TX_Size_3rd			(TX_Size_3rd[31:0]),
	   .TX_Start_4th_one_o	(TX_Start_4th_one),
		.TX_Addr_4th			(TX_Addr_4th[31:0]),
		.TX_Size_4th			(TX_Size_4th[31:0]),
`endif //MIMO_4X4
	 
      //interface from RX ENGINE
      .reg_data_in				(write_data_r[31:0]), 
      .reg_wr_addr				(MEM_addr_r[11:0]), 
      .reg_wren					(reg_wren_r), 
		.new_des_one				(new_des_one_r_in),		/// liuchang
		.rd_dma_done				(rd_dma_done_r1),
		
		/// signal to RX Engine
		.transferstart_o			(transferstart),
		.transferstart_one		(transferstart_one),
		.set_transfer_done_bit	(set_transfer_done_bit_r),
		.TransferSrcAddr			(TransferSrcAddr[63:0]),

		// register read interface
      .reg_rd_addr(Mrd_data_addr[11:0]),       /// Jiansong: 12bit register address
		.reg_data_out(Mrd_data),
 		
		 /// Jiansong: RX path
		 .RXEnable			(RXEnable),
		 .RXBufAddr			(RXBufAddr),
		 .RXBufSize			(RXBufSize),
		 .RXEnable_2nd		(RXEnable_2nd),
		 .RXBufAddr_2nd	(RXBufAddr_2nd),
		 .RXBufSize_2nd	(RXBufSize_2nd),
`ifdef MIMO_4X4
		 .RXEnable_3rd		(RXEnable_3rd),
		 .RXBufAddr_3rd	(RXBufAddr_3rd),
		 .RXBufSize_3rd	(RXBufSize_3rd),
		 .RXEnable_4th		(RXEnable_4th),
		 .RXBufAddr_4th	(RXBufAddr_4th),
		 .RXBufSize_4th	(RXBufSize_4th),
`endif //MIMO_4X4
		 
      /// Jiansong: error inputs
		.egress_overflow_one(egress_overflow_one_r),
		.RX_FIFO_full(RX_FIFO_full_r),
      .egress_rd_data_count(egress_rd_data_count_r),
      .egress_wr_data_count(egress_wr_data_count_r),

      //interface from performance counter block
//      .dma_wr_count(dma_wr_count),
//      .dma_rd_count(dma_rd_count),

      //interface from memory controller
      .phy_init_done(phy_init_done),
		//hardware status input
		.trn_lnk_up_n_c(trn_lnk_up_n_c),

      //interface from dma_ddr2_if
      .pause_read_requests		(pause_read_requests),
		
		// radio related inputs/outputs
`ifdef RADIO_CHANNEL_REGISTERS

		.Radio_Cmd_Data			(Radio_Cmd_Data[31:0]),
		.Radio_Cmd_Addr			(Radio_Cmd_Addr[6:0]),
		.Radio_Cmd_RdWr			(Radio_Cmd_RdWr),
		.Radio_Cmd_wren			(Radio_Cmd_wren),
		.Channel_Reg_Read_Value	(Channel_Reg_Read_Value[31:0]),
		.Channel_Reg_Read_Addr	(Channel_Reg_Read_Addr[7:0]),
		.Channel_ReadDone_in		(Channel_ReadDone_in),
/// registers for 2nd to 4th paths/radios		
		.Radio_2nd_Cmd_Data			(Radio_2nd_Cmd_Data[31:0]),
		.Radio_2nd_Cmd_Addr			(Radio_2nd_Cmd_Addr[6:0]),
		.Radio_2nd_Cmd_RdWr			(Radio_2nd_Cmd_RdWr),
		.Radio_2nd_Cmd_wren			(Radio_2nd_Cmd_wren),
		.Channel_2nd_Reg_Read_Value	(Channel_2nd_Reg_Read_Value[31:0]),
		.Channel_2nd_Reg_Read_Addr		(Channel_2nd_Reg_Read_Addr[7:0]),
		.Channel_2nd_ReadDone_in		(Channel_2nd_ReadDone_in),
`ifdef MIMO_4X4
		.Radio_3rd_Cmd_Data			(Radio_3rd_Cmd_Data[31:0]),
		.Radio_3rd_Cmd_Addr			(Radio_3rd_Cmd_Addr[6:0]),
		.Radio_3rd_Cmd_RdWr			(Radio_3rd_Cmd_RdWr),
		.Radio_3rd_Cmd_wren			(Radio_3rd_Cmd_wren),
		.Channel_3rd_Reg_Read_Value	(Channel_3rd_Reg_Read_Value[31:0]),
		.Channel_3rd_Reg_Read_Addr		(Channel_3rd_Reg_Read_Addr[7:0]),
		.Channel_3rd_ReadDone_in		(Channel_3rd_ReadDone_in),
		.Radio_4th_Cmd_Data			(Radio_4th_Cmd_Data[31:0]),
		.Radio_4th_Cmd_Addr			(Radio_4th_Cmd_Addr[6:0]),
		.Radio_4th_Cmd_RdWr			(Radio_4th_Cmd_RdWr),
		.Radio_4th_Cmd_wren			(Radio_4th_Cmd_wren),
		.Channel_4th_Reg_Read_Value	(Channel_4th_Reg_Read_Value[31:0]),
		.Channel_4th_Reg_Read_Addr		(Channel_4th_Reg_Read_Addr[7:0]),
		.Channel_4th_ReadDone_in		(Channel_4th_ReadDone_in),
`endif //MIMO_4X4

`endif //RADIO_CHANNEL_REGISTERS		
		.DebugRX1Overflowcount_in(DebugRX1Overflowcount_in),
		.DebugRX2Overflowcount_in(DebugRX2Overflowcount_in),

		.DebugDDREgressFIFOCnt	(DebugDDREgressFIFOCnt[31:0]),
		.DebugDDRFIFOFullCnt	(DebugDDRFIFOFullCnt[31:0]),
		.DebugDDRSignals	(DebugDDRSignals[31:0]),
		.DebugDDRSMs		(DebugDDRSMs[8:0]),
		
		.PCIeLinkStatus_in(PCIeLinkStatus_r),
      .PCIeLinkControl_in(PCIeLinkControl_r)
    );

/// Jiansong: moved here from tx engine
// Instantiate the non_posted packet generator
   non_posted_pkt_gen non_posted_pkt_gen_inst (
      .clk(clk), 
      .rst(rst_reg), 
		
`ifdef TF_RECOVERY
		.transferstart(transferstart_r),
`endif
		
      //interface to dma_ctrl_wrapper
		.dmaras(dmaras_r1), 
      .dmarad(dmarad_r1), 
      .dmarxs(dmarxs_r1), 
      .rd_dma_start(rd_dma_start_r1), 
		
		/// Jiansong: interface to dma control wrapper, for tx des
	   .rd_TX_des_start(rd_TX_des_start_r1),
	   .TX_des_addr(TX_des_addr),
		
      //interface to PCIe Endpoint Block Plus
		.read_req_size(pcie_max_read_req), 
      .req_id({pcie_id,3'b000}),
		
      //inteface to non-posted header fifo (a64_64_distram_fifo_np)
      .non_posted_fifo_wren(non_posted_fifo_wren), 
      .non_posted_fifo_data(non_posted_fifo_data),
		
      //interface to tag_generator
      .tag_inc(np_tag_inc), 
      .tag_gnt(np_tag_gnt), 
      .tag_value(tag_value),

      //interface to read_request_wrapper
      .tx_waddr(tx_waddr[4:0]),
      .tx_wdata(tx_wdata[31:0]),
      .tx_we(tx_we)
   );

/// Jiansong: moved here from tx engine, pending
// Instantiate the posted packet generator
   posted_pkt_gen_sora posted_pkt_gen_inst (
      .clk(clk), 
      .rst(rst_reg), 
      //interface to dma_ctrl_wrapper
		/// Jiansong: TX desc write back
	    .TX_desc_write_back_req(TX_desc_write_back_req_r),
	    .TX_desc_write_back_ack(TX_desc_write_back_ack),
	    .SourceAddr            (SourceAddr_r_out),
       .DestAddr              (DestAddr_r_out),
       .FrameSize             (FrameSize_r_out),
       .FrameControl          (FrameControl_r_out),
		 .DescAddr              (DescAddr_r_out),
		 /// Jiansong: RX paths
		 .RXEnable			(RXEnable),
		 .RXBufAddr			(RXBufAddr),
		 .RXBufSize			(RXBufSize),
		 .RXEnable_2nd		(RXEnable_2nd),
		 .RXBufAddr_2nd	(RXBufAddr_2nd),
		 .RXBufSize_2nd	(RXBufSize_2nd),
`ifdef MIMO_4X4
		 .RXEnable_3rd		(RXEnable_3rd),
		 .RXBufAddr_3rd	(RXBufAddr_3rd),
		 .RXBufSize_3rd	(RXBufSize_3rd),
		 .RXEnable_4th		(RXEnable_4th),
		 .RXBufAddr_4th	(RXBufAddr_4th),
		 .RXBufSize_4th	(RXBufSize_4th),
`endif //MIMO_4X4

      //interface to PCIe Endpoint Block Plus
		.max_pay_size(pcie_max_pay_size),
      .req_id({pcie_id,3'b000}),
		
      //interface to posted header fifo (a64_128_distram_fifo_p)
      .posted_fifo_wren(posted_fifo_wren), 
      .posted_fifo_data(posted_fifo_data),
		.posted_fifo_full(posted_fifo_full),   /// pending
		//interface to dma write data fifo in TX engine
	   .dma_write_data_fifo_data(dma_write_data_fifo_data),
	   .dma_write_data_fifo_wren(dma_write_data_fifo_wren),
	   .dma_write_data_fifo_full(dma_write_data_fifo_full),	 
		// interface to RX data fifo
		.RX_FIFO_data		(RX_FIFO_data[63:0]),
		.RX_FIFO_RDEN		(RX_FIFO_RDEN),
		.RX_FIFO_pempty	(RX_FIFO_pempty),
		.RX_TS_FIFO_data	(RX_TS_FIFO_data[31:0]),
		.RX_TS_FIFO_RDEN	(RX_TS_FIFO_RDEN),
		.RX_TS_FIFO_empty	(RX_TS_FIFO_empty),
		.RX_FIFO_2nd_data			(RX_FIFO_2nd_data[63:0]),
		.RX_FIFO_2nd_RDEN			(RX_FIFO_2nd_RDEN),
		.RX_FIFO_2nd_pempty		(RX_FIFO_2nd_pempty),
		.RX_TS_FIFO_2nd_data		(RX_TS_FIFO_2nd_data[31:0]),
		.RX_TS_FIFO_2nd_RDEN		(RX_TS_FIFO_2nd_RDEN),
		.RX_TS_FIFO_2nd_empty	(RX_TS_FIFO_2nd_empty),
`ifdef MIMO_4X4
		.RX_FIFO_3rd_data			(RX_FIFO_3rd_data[63:0]),
		.RX_FIFO_3rd_RDEN			(RX_FIFO_3rd_RDEN),
		.RX_FIFO_3rd_pempty		(RX_FIFO_3rd_pempty),
		.RX_TS_FIFO_3rd_data		(RX_TS_FIFO_3rd_data[31:0]),
		.RX_TS_FIFO_3rd_RDEN		(RX_TS_FIFO_3rd_RDEN),
		.RX_TS_FIFO_3rd_empty	(RX_TS_FIFO_3rd_empty),
		.RX_FIFO_4th_data			(RX_FIFO_4th_data[63:0]),
		.RX_FIFO_4th_RDEN			(RX_FIFO_4th_RDEN),
		.RX_FIFO_4th_pempty		(RX_FIFO_4th_pempty),
		.RX_TS_FIFO_4th_data		(RX_TS_FIFO_4th_data[31:0]),
		.RX_TS_FIFO_4th_RDEN		(RX_TS_FIFO_4th_RDEN),
		.RX_TS_FIFO_4th_empty	(RX_TS_FIFO_4th_empty)
`endif //MIMO_4X4

   );
	
//////////////////////////////////////////////////////////////
////////// DMA engines below /////////////////////////////////

  // Instantiate the Transmit Engine
   tx_engine tx_engine_inst   (
      .clk(clk), 
		.full_rst(full_rst),
		.soft_rst(soft_rst),

      //interface to/from PCIE block plus
      .pcie_id({pcie_id,3'b000}), 
      .trn_td(trn_td[63:0]),//o [63:0]
      .trn_trem_n(trn_trem_n[7:0]), //o [7:0]
      .trn_tsof_n(trn_tsof_n), //o
      .trn_teof_n(trn_teof_n),//o
      .trn_tsrc_rdy_n(trn_tsrc_rdy_n), //o
      .trn_tsrc_dsc_n(trn_tsrc_dsc_n), //o
      .trn_tdst_rdy_n(trn_tdst_rdy_n), //i
      .trn_tdst_dsc_n(trn_tdst_dsc_n),//i
      .trn_terrfwd_n(trn_terrfwd_n),//o
      .trn_tbuf_av(trn_tbuf_av[2:0]),//i[2:0]
		
      .Mrd_data_addr(Mrd_data_addr),
      .Mrd_data_in(Mrd_data), 
		
		/// Jiansong: interface from non_posted_pkt_gen
		.non_posted_fifo_wren(non_posted_fifo_wren), 
      .non_posted_fifo_data(non_posted_fifo_data),
		
		/// Jiansong: interface from posted_pket_gen
		.posted_fifo_wren(posted_fifo_wren),
      .posted_fifo_data(posted_fifo_data),
		.posted_fifo_full(posted_fifo_full), ///pending
		
		/// Jiansong: interface from posted_pkt_gen
	   .dma_write_data_fifo_data(dma_write_data_fifo_data),
	   .dma_write_data_fifo_wren(dma_write_data_fifo_wren),
	   .dma_write_data_fifo_full(dma_write_data_fifo_full),
				
       //interface to RX Engine
      .bar_hit(bar_hit),//I [6:0]
      .MRd(MRd),//I                
      .MWr(MWr),//I   
      .MEM_addr(MEM_addr),//I [31:0]
      .MEM_req_id(MEM_req_id),//I [15:0]
      .MEM_tag(MEM_tag),//I [7:0]
      .header_fields_valid(header_fields_valid),//I
		// input from rx_monitor
		.rd_dma_start(rd_dma_start_r1),    
		.dmarxs(dmarxs_r1[12:3])          
   );


   //Instantiate Read Request wrapper
   //This block contains a RAM to hold information for
   //incoming completions due to non-posted
   //memory read requests
   //This is essentially the communication
   //mailbox between the RX and TX engines
   read_req_wrapper read_req_wrapper_inst(
      .clk   (clk),
      .rst   (rst_reg),

`ifdef TF_RECOVERY		
		.transferstart (transferstart_r),
`endif
		/// Jiansong: interface to/from non_posted_pkt_gen
      .tx_waddr   (tx_waddr[4:0]),
      .tx_wdata (tx_wdata[31:0]),
      .tx_we   (tx_we),
		
		/// Jiansong: interface to/from RX module
      .rx_waddr (rx_waddr[4:0]),
      .rx_wdata  (rx_wdata[31:0]),
      .rx_we (rx_we),
      .rx_raddr  (rx_raddr[4:0]),
      .rx_rdata(rx_rdata[31:0]),
      .pending_comp_done(pending_comp_done),
		
      .completion_pending(completion_pending[31:0]),
      .comp_timeout(comp_timeout)
     );


  // Instantiate the Recieve Engine
  rx_engine rx_engine_inst(
     .clk                     (clk),
     .rst                     (rst_reg),
     //DDR2 Memory controller I/F SIGNALS
     //ingress
     .ingress_data            (ingress_data),
	  .ingress_fifo_wren		(ingress_fifo_wren),
     .ingress_xfer_size       (ingress_xfer_size),
     .ingress_start_addr      (ingress_start_addr),
     .ingress_data_req        (ingress_data_req),
     .ingress_data_ack        (ingress_data_ack),

     //REG FILE SIGNALS
     .rd_dma_start            (rd_dma_start_r1), // fine tune the pipeline registers to make np_rx_cnt_qw and 	                                                 
     .dmarxs                  (dmarxs_r1),       // np_tx_cnt_qw change at the same clock cycle
                                                 
     .rd_dma_done             (rd_dma_done),     //dma transfer complete
     .dmarad                  (dmarad_r1),       
  
	  ///Jiansong: interface to dma control wrapper
     .new_des_one             (new_des_one),
     .SourceAddr_L            (SourceAddr_L_in),
     .SourceAddr_H            (SourceAddr_H_in),
     .DestAddr                (DestAddr_in),
     .FrameSize               (FrameSize_in),
     .FrameControl            (FrameControl_in),
	  ///
	  .Wait_for_TX_desc(Wait_for_TX_desc_r),
`ifdef TF_RECOVERY
	  .transferstart(transferstart_r),
`endif

     //TX ENGINE I/F SIGNALs
     //The read request ram contains the data required by the RX engine
     //to verify completions due to Non-Posted Memory Read Requests
     //generated by the TX engine
     .rx_waddr	(rx_waddr[4:0]),
     .rx_wdata	(rx_wdata[31:0]),
     .rx_we		(rx_we),
     .rx_raddr	(rx_raddr[4:0]),
     .rx_rdata	(rx_rdata[31:0]),
     .pending_comp_done	(pending_comp_done),
     .completion_pending(completion_pending[31:0]),

     //RX TRN interface(Endpoint Plus Core)
     // Rx Local-Link
     .trn_rd( trn_rd),                      // I [63/31:0]
     .trn_rrem_n( trn_rrem_n ),             // I [7:0]
     .trn_rsof_n( trn_rsof_n ),             // I
     .trn_reof_n( trn_reof_n ),             // I
     .trn_rsrc_rdy_n( trn_rsrc_rdy_n ),     // I
     .trn_rsrc_dsc_n( trn_rsrc_dsc_n ),     // I
     .trn_rdst_rdy_n( trn_rdst_rdy_n ),     // O
     .trn_rerrfwd_n( trn_rerrfwd_n ),       // I
     .trn_rnp_ok_n( trn_rnp_ok_n ),         // O
     .trn_rbar_hit_n( trn_rbar_hit_n ),     // I [6:0]
     .trn_rfc_npd_av( trn_rfc_npd_av ),     // I [11:0]
     .trn_rfc_nph_av( trn_rfc_nph_av ),     // I [7:0]
     .trn_rfc_pd_av( trn_rfc_pd_av ),       // I [11:0]
     .trn_rfc_ph_av( trn_rfc_ph_av ),       // I [7:0]
     .trn_rfc_cpld_av( trn_rfc_cpld_av ),   // I [11:0]
     .trn_rfc_cplh_av( trn_rfc_cplh_av ),   // I [7:0]
     .trn_rcpl_streaming_n( trn_rcpl_streaming_n ), //O
     //Signals for TX engine and dma_ctrl_wrapper
     .bar_hit_o(bar_hit),//O [6:0]
     .MRd_o(MRd),//O                
     .MWr_o(MWr),//O   
     .MEM_addr_o(MEM_addr),//O [31:0]
     .MEM_req_id_o(MEM_req_id),//O [15:0]
     .MEM_tag_o(MEM_tag),//O [7:0]
     .header_fields_valid_o(header_fields_valid),//O
     .write_data(write_data[31:0]),
     .write_data_wren(write_data_wren),
     .read_last(read_last)
  );

  //Instantiate tag generator
   tag_generator tag_generator_inst (
      .clk(clk), 
      .rst(rst_reg), 
      //inteface to non_posted_pkt_gen
      .np_tag_gnt(np_tag_gnt), 
      .np_tag_inc(np_tag_inc), 
      .tag_value(tag_value),
      //interface to read_request_wrapper
      .completion_pending(completion_pending[31:0])
   );


/// Jiansong: temporarily disable this module   
//  performance_counters performance_counters_inst(
//    .clk(clk), 
//    .rst(rst_reg),
//    .wr_dma_start(wr_dma_start_r1),
//    .wr_dma_done(wr_dma_done),
//    .rd_dma_start(rd_dma_start_r1),
//    .rd_dma_done(rd_dma_done),
//    .dma_wr_count(dma_wr_count),
//    .dma_rd_count(dma_rd_count),
//    .read_last(read_last),
//    .write_last(write_last)
//    );



//assign RXEnable_o = RXEnable_r;
assign RXEnable_o = RXEnable;
	 	
	// pipeline registers
   always@(posedge clk)begin
		 // register write related
		 reg_wren_r				<= bar_hit[0] & MWr & write_data_wren;
		 write_data_r[31:0]	<= write_data[31:0];
		 MEM_addr_r[11:0] 	<= MEM_addr[11:0];
		 
		 // transfer related (trigger)
		 dmaras_r1            <= dmaras;
		 dmarad_r1            <= dmarad;
		 dmarxs_r1            <= dmarxs;
		 rd_dma_start_r1      <= rd_dma_start;
		 rd_dma_done_r1       <= rd_dma_done;
		 // transfer related (TX descriptor)
		 new_des_one_r_in		<= new_des_one;			
		 SourceAddr_L_r_in	<= SourceAddr_L_in;
		 SourceAddr_H_r_in	<= SourceAddr_H_in;
		 DestAddr_r_in			<= DestAddr_in;
		 FrameSize_r_in		<= FrameSize_in;
		 FrameControl_r_in 	<= FrameControl_in;
		 // transfer related (control signal)
		 Wait_for_TX_desc_r 			<= Wait_for_TX_desc;
		 transferstart_r    			<= transferstart;
		 transferstart_one_r			<= transferstart_one;
		 set_transfer_done_bit_r	<= set_transfer_done_bit;
		 // transfer related (TX desc write back)
		 rd_TX_des_start_r1      	<= rd_TX_des_start;
		 TX_desc_write_back_req_r	<= TX_desc_write_back_req;
		 TX_desc_write_back_ack_r	<= TX_desc_write_back_ack;		  
		 SourceAddr_r_out          <= SourceAddr_out;
		 DestAddr_r_out            <= DestAddr_out;
		 FrameSize_r_out           <= FrameSize_out;
		 FrameControl_r_out        <= FrameControl_out;
		 DescAddr_r_out            <= DescAddr_out;
		 

		DDR2MaxBurstSize_r	<= DDR2MaxBurstSize[4:0];
		 

		 /// debug signals			 
		 egress_overflow_one_r 	<= egress_overflow_one;
		 RX_FIFO_full_r 			<= RX_FIFO_full;
		 egress_rd_data_count_r <= egress_rd_data_count;
		 egress_wr_data_count_r <= egress_wr_data_count;
		 PCIeLinkStatus_r       <= PCIeLinkStatus;
		 PCIeLinkControl_r      <= PCIeLinkControl;
   end

endmodule


