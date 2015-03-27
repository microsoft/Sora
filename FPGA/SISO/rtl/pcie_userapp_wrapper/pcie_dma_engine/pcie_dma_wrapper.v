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
    input hard_rst,
	 input rst,
	 // hot reset to whole system, two cycles
	 output hostreset,
	 /// Jiansong: interface to RX data fifo
	 input  [63:0] RX_FIFO_data,
	 output        RX_FIFO_RDEN,
	 input         RX_FIFO_pempty,
	 output reg    RXEnable_o,
	 // interface to the 2nd RX data fifo
`ifdef SORA_FRL_2nd
	 input  [63:0] RX_FIFO_2nd_data,
	 output        RX_FIFO_2nd_RDEN,
	 input         RX_FIFO_2nd_pempty,
`endif	 
	 /// Jiansong: interface to radio module
	 input         Radio_TX_done,
	 output        Radio_TX_start,
	 
    //interface to dma_ddr2_if
    output  [2:0] ingress_xfer_size,
    output [27:6] ingress_start_addr,
    output ingress_data_req,
    input ingress_data_ack,
    input [1:0] ingress_fifo_status,
    output [1:0] ingress_fifo_ctrl,
    output [127:0] ingress_data,
    input pause_read_requests,
	 /// Jiansong: TX related inputs/outputs
	 output reg TX_DDR_data_req,
	 input      TX_DDR_data_ack,
	 output reg [27:6] TX_DDR_start_addr,
	 output reg [2:0]  TX_DDR_xfer_size,
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
`ifdef WARP_RADIO_REGISTERS
	 output [1:0] RadioAntSelect,
	 input  [3:0] RadioDIPSW,
	 output [2:0] RadioLEDControl,
	 output       RadioMaximSHDN,
	 output       RadioMaximReset,
	 output       RadioMaximRXHP,  
	 output [5:0] RadioTXGainSetting,
	 output [6:0] RadioRXGainSetting,
	 input        RadioLD,
	 output [3:0] RadioADCControl,
	 input  [1:0] RadioADCStatus,
	 output       RadioDACControl,
	 input        RadioDACStatus,
	 output       RadioMaximSPIStart,
	 input        RadioMaximSPIDone,
	 output       RadioDACSPIStart,
	 input        RadioDACSPIDone,
	 output [3:0] RadioMaximSPIAddr,
	 output [13:0] RadioMaximSPIData,
	 output [7:0] RadioDACSPIData,
	 output [7:0] RadioDACSPIInstuct,
	 input  [7:0] RadioDACSPIDataOut,
	 output [2:0] RadioRSSIADCControl,
	 input  [9:0] RadioRSSIData,
	 input        RadioRSSIOTR,
`endif
`ifdef SORA_RADIO_REGISTERS
	 output [1:0] 	RadioControl,
	 input		  	RadioStatus_in,
	 input  [31:0] RadioID_in,
	 output [31:0] LEDControl,
	 output [31:0] AntennaSelection,
	 output [31:0] SampleClock,
	 output			SampleClockSet,
	 output [31:0] CoarseFreq,
	 output [31:0] FinegradeFreq,
	 output [31:0] FreqCompensation,
	 output			CenterFreqSet,
	 input 		  	RadioLOLock,
	 output [31:0] FilterBandwidth,
	 output			FilterBandwidthSet,
	 output [31:0] TXVGA1,
	 output [31:0] TXVGA2,
	 output [31:0] TXPA1,
	 output [31:0] TXPA2,
	 output [31:0] RXLNA,
	 output [31:0] RXPA,
	 output [31:0] RXVGA1,
	 output [31:0] RXVGA2,
`endif
`ifdef RADIO_CHANNEL_REGISTERS
	 output [31:0] Radio_Cmd_Data,
	 output [6:0]  Radio_Cmd_Addr,
	 output 			Radio_Cmd_RdWr,
	 output 			Radio_Cmd_wren,
	 input  [31:0]	Channel_Reg_Read_Value,
	 input			Channel_ReadDone_in,
`endif
    // Debug interface
	 input [15:0] PCIeLinkStatus,
	 input [15:0] PCIeLinkControl,
    input [31:0] Debug18DDR1,
	 input [31:0] Debug19DDR2,
	 input [31:0] Debug23RX4,
    input [4:0]  locked_debug	 
    );

	  /// Debug wires and pipeline regs
	  wire [31:0] Debug20RX1;
	  wire [31:0] Debug21RX2;
	  wire [4:0]  Debug22RX3;
	  wire [31:0] Debug24RX5;
	  wire [31:0] Debug25RX6;
	  wire [31:0] Debug26RX7;
	  wire [31:0] Debug27RX8;
	  wire [31:0] Debug28RX9;
	  wire [31:0] Debug29RX10;
	  wire [9:0]  Debug30RXEngine;
	  wire [11:0] Debug31RXDataFIFOfullcnt;
	  wire [11:0] Debug32RXXferFIFOfullcnt;
	  wire [23:0] Debug33RXDataFIFOWRcnt;
	  wire [23:0] Debug34RXDataFIFORDcnt;
	  wire [23:0] Debug35RXXferFIFOWRcnt;
	  wire [23:0] Debug36RXXferFIFORDcnt;
	  wire [7:0]  FIFOErrors;
	  
	  reg  [15:0] PCIeLinkStatus_r;
	  reg  [15:0] PCIeLinkControl_r;
	  reg  [31:0] Debug18DDR1_r;
	  reg  [31:0] Debug19DDR2_r;
	  reg  [31:0] Debug20RX1_r;
	  reg  [31:0] Debug21RX2_r;
	  reg  [4:0]  Debug22RX3_r;
	  reg  [31:0] Debug23RX4_r;
	  reg  [31:0] Debug24RX5_r;
	  reg  [31:0] Debug25RX6_r;
	  reg  [31:0] Debug26RX7_r;
	  reg  [31:0] Debug27RX8_r;
	  reg  [31:0] Debug28RX9_r;
	  reg  [31:0] Debug29RX10_r;
	  reg  [9:0]  Debug30RXEngine_r;
	  reg  [11:0] Debug31RXDataFIFOfullcnt_r;
	  reg  [11:0] Debug32RXXferFIFOfullcnt_r;
	  reg  [23:0] Debug33RXDataFIFOWRcnt_r;
	  reg  [23:0] Debug34RXDataFIFORDcnt_r;
	  reg  [23:0] Debug35RXXferFIFOWRcnt_r;
	  reg  [23:0] Debug36RXXferFIFORDcnt_r;
	  reg  [31:0] Debug37completion_pending_r;
	  reg  [4:0]  Debug38tag_value_r;
	  reg  [7:0]  FIFOErrors_r;
	  reg  [4:0]  locked_debug_r;
	  
	  wire [9:0]  np_rx_cnt_qw;
 
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
  /// Jiansong: RX path
  wire [63:0] RXBufAddr;
  wire [31:0] RXBufSize;
  /// the 2nd RX path
`ifdef SORA_FRL_2nd
  wire [63:0] RXBufAddr_2nd;
  wire [31:0] RXBufSize_2nd;
`endif
  wire        RXEnable;

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
  /// Jiansong: RX path
  reg [63:0] RXBufAddr_r;
  reg [31:0] RXBufSize_r;
  /// the 2nd RX path
`ifdef SORA_FRL_2nd
  reg [63:0] RXBufAddr_2nd_r;
  reg [31:0] RXBufSize_2nd_r;
`endif
  reg        RXEnable_r;
  
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
  wire rd_dma_start_one;
  wire rd_TX_des_start;
  reg  rd_TX_des_start_r1;
  wire rd_TX_des_start_one;
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
  wire               transferstart_o;
  reg                transferstart_r;
  
  // dma write data fifo interface, between posted_pkt_gen and tx engine
  wire [63:0]  dma_write_data_fifo_data;
  wire         dma_write_data_fifo_wren;
  wire         dma_write_data_fifo_full;
  
  /// Jiansong: TX related inputs/outputs, pipelined
  wire TX_data_req;
  reg  TX_data_ack;
  wire [27:6] TX_start_addr;
  wire [2:0]  TX_xfer_size;
	 
  /// Jiansong: interface to radio module
  reg  Radio_TX_done_r;

  wire [7:0] tag_value;
  
   
	always@(posedge clk) rst_reg <= rst | hostreset;


  // Instantiate the Transmit Engine
   tx_engine tx_engine_inst   (
      .clk(clk), 
      .rst(rst),
		.hostreset(hostreset),

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
		.dmarxs(dmarxs_r1[12:3]),          
		.np_rx_cnt_qw(np_rx_cnt_qw),
		.transferstart (transferstart_r),
 	   .Wait_for_TX_desc(Wait_for_TX_desc_r),
      // debug interface
      .Debug21RX2(Debug21RX2),
      .Debug25RX6(Debug25RX6),
      .FIFOErrors(FIFOErrors)		
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
		
		.transferstart (transferstart_r),
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
     .ingress_fifo_ctrl(ingress_fifo_ctrl), //bit 1 = unused, bit 0 = write_en
     .ingress_fifo_status(ingress_fifo_status),//bit 1 = full,bit 0 = almostfull
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
	  .transferstart(transferstart_r),

     //TX ENGINE I/F SIGNALs
     //The read request ram contains the data required by the RX engine
     //to verify completions due to Non-Posted Memory Read Requests
     //generated by the TX engine
     .rx_waddr (rx_waddr[4:0]),
     .rx_wdata  (rx_wdata[31:0]),
     .rx_we (rx_we),
     .rx_raddr  (rx_raddr[4:0]),
     .rx_rdata(rx_rdata[31:0]),
     .pending_comp_done(pending_comp_done),
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
     .read_last(read_last),
	  // output to tx_sm
	  .np_rx_cnt_qw(np_rx_cnt_qw),
	  // Debug output
	  .Debug30RXEngine(Debug30RXEngine),
	  .Debug31RXDataFIFOfullcnt(Debug31RXDataFIFOfullcnt),
	  .Debug32RXXferFIFOfullcnt(Debug32RXXferFIFOfullcnt),
     .Debug33RXDataFIFOWRcnt(Debug33RXDataFIFOWRcnt),
     .Debug34RXDataFIFORDcnt(Debug34RXDataFIFORDcnt),
     .Debug35RXXferFIFOWRcnt(Debug35RXXferFIFOWRcnt),
     .Debug36RXXferFIFORDcnt(Debug36RXXferFIFORDcnt)
  );


   always@(posedge clk)begin
          reg_wren_r <= bar_hit[0] & MWr & write_data_wren;
          write_data_r[31:0] <= write_data[31:0];
			 MEM_addr_r[11:0] <= MEM_addr[11:0];
			 /// Jiansong: pipeline registers for TX des			 
          new_des_one_r_in <= new_des_one;
          SourceAddr_L_r_in <= SourceAddr_L_in;
          SourceAddr_H_r_in <= SourceAddr_H_in;
          DestAddr_r_in <= DestAddr_in;
          FrameSize_r_in <= FrameSize_in;
          FrameControl_r_in <= FrameControl_in;
			 ///
			 Wait_for_TX_desc_r <= Wait_for_TX_desc;
			 transferstart_r    <= transferstart_o;
			 /// TX related pipeline
			 TX_DDR_data_req   <= TX_data_req;
	       TX_data_ack       <= TX_DDR_data_ack;
	       TX_DDR_start_addr <= TX_start_addr;
	       TX_DDR_xfer_size  <= TX_xfer_size;
			 /// Jiansong: interface to radio module
		    Radio_TX_done_r   <= Radio_TX_done;
			 /// debug signals			 
		    egress_overflow_one_r <= egress_overflow_one;
		    RX_FIFO_full_r <= RX_FIFO_full;
          egress_rd_data_count_r <= egress_rd_data_count;
          egress_wr_data_count_r <= egress_wr_data_count;
			 PCIeLinkStatus_r       <= PCIeLinkStatus;
          PCIeLinkControl_r      <= PCIeLinkControl;
			 Debug18DDR1_r          <= Debug18DDR1;
			 Debug19DDR2_r          <= Debug19DDR2;
			 Debug20RX1_r           <= Debug20RX1;
			 Debug21RX2_r           <= Debug21RX2;
			 Debug22RX3_r           <= Debug22RX3;
			 Debug23RX4_r           <= Debug23RX4;
			 Debug24RX5_r           <= Debug24RX5;
			 Debug25RX6_r           <= Debug25RX6;
			 Debug26RX7_r           <= Debug26RX7;
			 Debug27RX8_r           <= Debug27RX8;
			 Debug28RX9_r           <= Debug28RX9;
			 Debug29RX10_r          <= Debug29RX10;
			 Debug30RXEngine_r      <= Debug30RXEngine;
          Debug31RXDataFIFOfullcnt_r <= Debug31RXDataFIFOfullcnt;
          Debug32RXXferFIFOfullcnt_r <= Debug32RXXferFIFOfullcnt;
          Debug33RXDataFIFOWRcnt_r   <= Debug33RXDataFIFOWRcnt;
          Debug34RXDataFIFORDcnt_r   <= Debug34RXDataFIFORDcnt;
          Debug35RXXferFIFOWRcnt_r   <= Debug35RXXferFIFOWRcnt;
          Debug36RXXferFIFORDcnt_r   <= Debug36RXXferFIFORDcnt;
			 Debug37completion_pending_r <= completion_pending;
          Debug38tag_value_r[4:0]     <= tag_value[4:0];
			 FIFOErrors_r[7:0]           <= FIFOErrors[7:0];
			 locked_debug_r         <= locked_debug;
   end

  // Instantiate the Register File and accompanying logic
  dma_ctrl_wrapper dma_ctrl_wrapper_inst (
      .clk(clk), 
		.hard_rst(hard_rst),
		.rst(rst_reg),

      // hot reset to whole system, two cycles
	   .hostreset(hostreset),
	 
	   /// Jiansong: interface to radio module
		.Radio_TX_done(Radio_TX_done_r),
	   .Radio_TX_start(Radio_TX_start),
	 
      //interface from RX ENGINE
      .reg_data_in(write_data_r[31:0]), 
      .reg_wr_addr(MEM_addr_r[11:0]), 
      .reg_wren(reg_wren_r), 
		
		/// Jiansong: signals from RX Engine, for tx desc
      .new_des_one             (new_des_one_r_in),
      .SourceAddr_L            (SourceAddr_L_r_in),
      .SourceAddr_H            (SourceAddr_H_r_in),
      .DestAddr                (DestAddr_r_in),
      .FrameSize               (FrameSize_r_in),
      .FrameControl            (FrameControl_r_in),
		/// signal to RX Engine
		.Wait_for_TX_desc        (Wait_for_TX_desc),
		.transferstart_o(transferstart_o),

      .reg_rd_addr(Mrd_data_addr[11:0]),       /// Jiansong: 12bit register address
		.reg_data_out(Mrd_data),
 
      //interface to TX Engine
      .dmaras(dmaras), 
      .dmarad(dmarad), 
      .dmarxs(dmarxs), 
      .read_last(read_last), //connects to multiple logic blocks
      .rd_dma_start(rd_dma_start), 
      .rd_dma_done(rd_dma_done_r1), //done signal actually comes from RX engine
		
		/// Jiansong: interface to non_posted packet generator, start signal for tx desc
      .rd_TX_des_start(rd_TX_des_start),      /// Jiansong: start signal for TX des
		.TX_des_addr(TX_des_addr),
		
	    /// Jiansong: interface to/from posted_packet_generator
		 /// TX desc write back
	    .TX_desc_write_back_req  (TX_desc_write_back_req),
	    .TX_desc_write_back_ack  (TX_desc_write_back_ack_r),
	    .SourceAddr_r            (SourceAddr_out),
       .DestAddr_r              (DestAddr_out),
       .FrameSize_r             (FrameSize_out),
       .FrameControl_r          (FrameControl_out),
		 .DescAddr_r              (DescAddr_out),
		 /// Jiansong: RX path
		 .RXEnable(RXEnable),
		 .RXBufAddr(RXBufAddr),
		 .RXBufSize(RXBufSize),
		 /// the 2nd RX path
`ifdef SORA_FRL_2nd
		 .RXBufAddr_2nd(RXBufAddr_2nd),
		 .RXBufSize_2nd(RXBufSize_2nd),
`endif
		 
		 /// Jiansong: TX related inputs/outputs
	    .TX_data_req   (TX_data_req),
	    .TX_data_ack   (TX_data_ack),
	    .TX_start_addr (TX_start_addr),
	    .TX_xfer_size  (TX_xfer_size),

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
      .pause_read_requests(pause_read_requests),
		
		// radio related inputs/outputs
`ifdef WARP_RADIO_REGISTERS
	   .RadioAntSelect(RadioAntSelect),
		.RadioDIPSW(RadioDIPSW),
		.RadioLEDControl(RadioLEDControl),
      .RadioMaximSHDN(RadioMaximSHDN),
      .RadioMaximReset(RadioMaximReset),
      .RadioMaximRXHP(RadioMaximRXHP),  
      .RadioTXGainSetting(RadioTXGainSetting),
      .RadioRXGainSetting(RadioRXGainSetting),
	   .RadioLD(RadioLD),
	   .RadioADCControl(RadioADCControl),
	   .RadioADCStatus(RadioADCStatus),
	   .RadioDACControl(RadioDACControl),
	   .RadioDACStatus(RadioDACStatus),
	   .RadioMaximSPIStart(RadioMaximSPIStart),
	   .RadioMaximSPIDone(RadioMaximSPIDone),
	   .RadioDACSPIStart(RadioDACSPIStart),
	   .RadioDACSPIDone(RadioDACSPIDone),
	   .RadioMaximSPIAddr(RadioMaximSPIAddr),
	   .RadioMaximSPIData(RadioMaximSPIData),
      .RadioDACSPIData(RadioDACSPIData),
      .RadioDACSPIInstuct(RadioDACSPIInstuct),
	   .RadioDACSPIDataOut(RadioDACSPIDataOut),
	   .RadioRSSIADCControl(RadioRSSIADCControl),
	   .RadioRSSIData(RadioRSSIData),
	   .RadioRSSIOTR(RadioRSSIOTR),
`endif
`ifdef SORA_RADIO_REGISTERS
		.RadioControl(RadioControl),
		.RadioStatus_in(RadioStatus_in),
		.RadioID_in(RadioID_in),
		.LEDControl(LEDControl),
		.AntennaSelection(AntennaSelection),
		.SampleClock(SampleClock),
		.SampleClockSet(SampleClockSet),
		.CoarseFreq(CoarseFreq),
		.FinegradeFreq(FinegradeFreq),
		.FreqCompensation(FreqCompensation),
		.CenterFreqSet(CenterFreqSet),
		.RadioLOLock(RadioLOLock),
		.FilterBandwidth(FilterBandwidth),
		.FilterBandwidthSet(FilterBandwidthSet),
		.TXVGA1(TXVGA1),
		.TXVGA2(TXVGA2),
		.TXPA1(TXPA1),
		.TXPA2(TXPA2),
		.RXLNA(RXLNA),
		.RXPA(RXPA),
		.RXVGA1(RXVGA1),
		.RXVGA2(RXVGA2),
`endif
`ifdef RADIO_CHANNEL_REGISTERS
		.Radio_Cmd_Data(Radio_Cmd_Data[31:0]),
		.Radio_Cmd_Addr(Radio_Cmd_Addr[6:0]),
		.Radio_Cmd_RdWr(Radio_Cmd_RdWr),
		.Radio_Cmd_wren(Radio_Cmd_wren),
		.Channel_Reg_Read_Value(Channel_Reg_Read_Value[31:0]),
		.Channel_ReadDone_in(Channel_ReadDone_in),
`endif		
		.PCIeLinkStatus_in(PCIeLinkStatus_r),
      .PCIeLinkControl_in(PCIeLinkControl_r),
		.Debug18DDR1_in(Debug18DDR1_r),
		.Debug19DDR2_in(Debug19DDR2_r),
		.Debug20RX1_in(Debug20RX1_r),
		.Debug21RX2_in(Debug21RX2_r),
		.Debug22RX3_in(Debug22RX3_r),
		.Debug23RX4_in(Debug23RX4_r),
		.Debug24RX5_in(Debug24RX5_r),
		.Debug25RX6_in(Debug25RX6_r),
		.Debug26RX7_in(Debug26RX7_r),
		.Debug27RX8_in(Debug27RX8_r),
		.Debug28RX9_in(Debug28RX9_r),
		.Debug29RX10_in(Debug29RX10_r),
		.Debug30RXEngine_in(Debug30RXEngine_r),
      .Debug31RXDataFIFOfullcnt_in(Debug31RXDataFIFOfullcnt_r),
      .Debug32RXXferFIFOfullcnt_in(Debug32RXXferFIFOfullcnt_r),
      .Debug33RXDataFIFOWRcnt_in(Debug33RXDataFIFOWRcnt_r),
      .Debug34RXDataFIFORDcnt_in(Debug34RXDataFIFORDcnt_r),
      .Debug35RXXferFIFOWRcnt_in(Debug35RXXferFIFOWRcnt_r),
      .Debug36RXXferFIFORDcnt_in(Debug36RXXferFIFORDcnt_r),
		.Debug37completion_pending_in(Debug37completion_pending_r),
		.Debug38tag_value_in(Debug38tag_value_r),
		.FIFOErrors_in(FIFOErrors_r),
		.locked_debug(locked_debug_r)
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
		 /// Jiansong: RX path
		 .RXEnable(RXEnable_r),
		 .RXBufAddr(RXBufAddr_r),
		 .RXBufSize(RXBufSize_r),
		 /// the 2nd RX path
`ifdef SORA_FRL_2nd
		 .RXBufAddr_2nd(RXBufAddr_2nd_r),
		 .RXBufSize_2nd(RXBufSize_2nd_r),
`endif		
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
		.RX_FIFO_data(RX_FIFO_data),
		.RX_FIFO_RDEN(RX_FIFO_RDEN),
		.RX_FIFO_pempty(RX_FIFO_pempty),
		// interface to 2nd RX data fifo
`ifdef SORA_FRL_2nd
		.RX_FIFO_2nd_data(RX_FIFO_2nd_data),
		.RX_FIFO_2nd_RDEN(RX_FIFO_2nd_RDEN),
		.RX_FIFO_2nd_pempty(RX_FIFO_2nd_pempty),
`endif
		// Debug interface
		.Debug20RX1(Debug20RX1),
      .Debug22RX3(Debug22RX3),
 	   .Debug24RX5(Debug24RX5),
		.Debug26RX7(Debug26RX7),
 	   .Debug27RX8(Debug27RX8),
		.Debug28RX9(Debug28RX9),
		.Debug29RX10(Debug29RX10)
   );



/// Jiansong: moved here from tx engine
rising_edge_detect rd_dma_start_one_inst(
                .clk(clk),
                .rst(rst_reg),
                .in(rd_dma_start_r1),
                .one_shot_out(rd_dma_start_one)
                );
					 
/// Jiansong: added for TX des request trigger
rising_edge_detect rd_TX_des_start_one_inst(
                .clk(clk),
                .rst(rst_reg),
                .in(rd_TX_des_start_r1),
                .one_shot_out(rd_TX_des_start_one)
                );

/// Jiansong: moved here from tx engine
// Instantiate the non_posted packet generator
   non_posted_pkt_gen non_posted_pkt_gen_inst (
      .clk(clk), 
      .rst(rst_reg), 
		
		.transferstart(transferstart_r),
		
      //interface to dma_ctrl_wrapper
		.dmaras(dmaras_r1), 
      .dmarad(dmarad_r1), 
      .dmarxs(dmarxs_r1), 
      .rd_dma_start_one(rd_dma_start_one), 
		
		/// Jiansong: interface to dma control wrapper, for tx des
	   .rd_TX_des_start_one(rd_TX_des_start_one),
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

/// Jiansong: moved here from tx engine
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

  // Pipeline registers
  always@(posedge clk)begin
        dmaras_r1            <= dmaras;
        dmarad_r1            <= dmarad;
        dmarxs_r1            <= dmarxs;
        rd_dma_start_r1      <= rd_dma_start;
        rd_dma_done_r1       <= rd_dma_done;
		  /// Jiansong: TX desc write back
		  rd_TX_des_start_r1       <= rd_TX_des_start;
        TX_desc_write_back_req_r <= TX_desc_write_back_req;
        TX_desc_write_back_ack_r <= TX_desc_write_back_ack;		  
        SourceAddr_r_out             <= SourceAddr_out;
        DestAddr_r_out               <= DestAddr_out;
        FrameSize_r_out              <= FrameSize_out;
        FrameControl_r_out           <= FrameControl_out;
		  DescAddr_r_out               <= DescAddr_out;
		  /// Jiansong: RX path
		  RXBufAddr_r              <= RXBufAddr;
		  RXBufSize_r              <= RXBufSize;
		  /// the 2nd RX path
`ifdef SORA_FRL_2nd
		  RXBufAddr_2nd_r          <= RXBufAddr_2nd;
		  RXBufSize_2nd_r          <= RXBufSize_2nd;
`endif
		  RXEnable_r               <= RXEnable;
		  RXEnable_o               <= RXEnable;
   end

endmodule

