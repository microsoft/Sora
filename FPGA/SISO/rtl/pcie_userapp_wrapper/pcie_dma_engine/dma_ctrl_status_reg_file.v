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
// Purpose: This module is the DMA Control and Status register file.
//          It connects to the Host system behind a 128B PCI BAR.  
//          It also drives the Internal DMA Control block by breaking
//          large transfer requests into 4KB and smaller requests.  Note, if
//          if the user did not require transfers larger than 4KB,
//          this block could be removed from the design and
//          the Internal DMA Control block could be used (with minor
//          modification to the design in place.
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

module dma_ctrl_status_reg_file(
    input clk,
	 // host reset generation logic which should only controlled by hardware reset 
	 // is implemented in this module, so we should differentiate 
	 // hardware reset and host reset in this module
    input hard_rst, 
	 input rst,
	 
	 // hot reset to whole system, two cycles
	 output reg hostreset,
	 
	 /// Jiansong: performance value inputs
	input [23:0] round_trip_latency_i, /// 24 bits performance value
	input [23:0] transfer_duration_i,  /// 24 bits performance value
	
    /// Jiansong: interface to radio module
	 input      Radio_TX_done,
	 output     Radio_TX_start,
    //Interface to host system
    //inputs are driven from RX Engine
    input [31:0] reg_data_in, 
    input [11:0] reg_wr_addr,
    input reg_wren,
	 
	  ///Jiansong: interface from RX engine, TX desc received
	 input                new_des_one,
	 input        [31:0]  SourceAddr_L,
	 input        [31:0]  SourceAddr_H,
	 input        [31:0]  DestAddr,
	 input        [23:0]  FrameSize,
	 input        [7:0]   FrameControl,
    ///Jiansong: interface to RX engine, indicate the system is in dma read for TX desc
	 ///          when this signal is asserted, received cpld will not be count in 
	 ///          length subtraction
    output reg        Wait_for_TX_desc,	
    /// Jiansong: control signal for transfer recovering
    output wire  transferstart_o,
    /// Jiansong: signal output to performance counter
    output wire  transferstart_one,
	 
	 /// Jiansong: interface to/from posted_packet_generator
	 /// TX desc write back
	 output reg         TX_desc_write_back_req,
	 input              TX_desc_write_back_ack,
	 output reg [63:0]  SourceAddr_r,
    output reg [31:0]  DestAddr_r,
    output reg [23:0]  FrameSize_r,
    output reg [7:0]   FrameControl_r,
	 output reg [63:0]  DescAddr_r,
	 /// Jiansong: RX path
	 output         RXEnable_o,
	 output [63:0]  RXBufAddr_o,
	 output [31:0]  RXBufSize_o,
		 /// the 2nd RX path
`ifdef SORA_FRL_2nd
	 output [63:0]  RXBufAddr_2nd_o,
	 output [31:0]  RXBufSize_2nd_o,
`endif
	 /// Jiansong: interface to/from tx engine
    input [11:0] reg_rd_addr,
    output reg [31:0] reg_data_out,

    /// Jiansong: interface to non_posted packet generator
	 output reg rd_TX_des_start,
    output [63:0] TX_des_addr,

    //outputs to Internal DMA CTRL block
    output reg [31:0] reg_data_in_o,
    output reg [6:0] reg_wr_addr_o,
    output reg reg_wren_o,

    //Input DMA done signals from TX and RX Engines
    input rd_dma_done_i,

    //Output DMA done signals to Internal DMA CTRL block
    //these are copies of the *_dma_done_i inputs
    output reg rd_dma_done_o,

    //Performance counts from performance counter module
//    input [31:0] dma_wr_count,
//    input [31:0] dma_rd_count,

    //the *_last signals are used by the tx_trn_sm block and rx_trn_data_fsm
    //block so they can generate the most accurate *_done signals possible;
    //this is needed for accurate performance measurements
    output read_last,  //active high during the very last DMA read of a series
    
	 /// Jiansong: TX related inputs/outputs
	 output reg TX_data_req,
	 input      TX_data_ack,
	 output reg [27:6] TX_start_addr,
	 output reg [2:0]  TX_xfer_size,
	 
	 /// Jiansong: Error inputs
	 input egress_overflow_one,
	 input RX_FIFO_full,
    input [31:0] egress_rd_data_count,
    input [31:0] egress_wr_data_count,
	 
    //input from memory controller        
    input phy_init_done,
	 //hardware status input
	 input trn_lnk_up_n_c,

    //input from dma_ddr2_if
    input pause_read_requests,
	 
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
	 output 			SampleClockSet,
	 output [31:0] CoarseFreq,
	 output [31:0] FinegradeFreq,
	 output [31:0] FreqCompensation,
	 output 			CenterFreqSet,
	 input 		  	RadioLOLock,
	 output [31:0] FilterBandwidth,
	 output 			FilterBandwidthSet,
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
	 output reg [31:0] 	Radio_Cmd_Data,	// defined as register is a redundant, but better for timing
	 output reg [6:0]  	Radio_Cmd_Addr,
	 output reg				Radio_Cmd_RdWr,
	 output reg				Radio_Cmd_wren,
	 input  [31:0]			Channel_Reg_Read_Value,
	 input					Channel_ReadDone_in,
`endif
	 // debug signals
	 input [15:0] PCIeLinkStatus_in,
    input [15:0] PCIeLinkControl_in,
	 input [31:0] Debug18DDR1_in,
	 input [31:0] Debug19DDR2_in,
	 input [31:0] Debug20RX1_in,
	 input [31:0] Debug21RX2_in,
	 input [4:0]  Debug22RX3_in,
	 input [31:0] Debug23RX4_in,
	 input [31:0] Debug24RX5_in,
	 input [31:0] Debug25RX6_in,
	 input [31:0] Debug26RX7_in,
    input [31:0] Debug27RX8_in,
	 input [31:0] Debug28RX9_in,
    input [31:0] Debug29RX10_in,
	 input [9:0]  Debug30RXEngine_in,
    input [11:0] Debug31RXDataFIFOfullcnt_in,
    input [11:0] Debug32RXXferFIFOfullcnt_in,
    input [23:0] Debug33RXDataFIFOWRcnt_in,
    input [23:0] Debug34RXDataFIFORDcnt_in,
    input [23:0] Debug35RXXferFIFOWRcnt_in,
    input [23:0] Debug36RXXferFIFORDcnt_in,
	 input [31:0] Debug37completion_pending_in,
	 input [4:0]  Debug38tag_value_in,
	 input [7:0]  FIFOErrors_in,
	 input [4:0]  locked_debug
    );

    //state machine state definitions for state_sm0
    localparam IDLE_SM0 = 4'h0;
    localparam READ_1 = 4'h7;
    localparam READ_2 = 4'h8;
    localparam READ_3 = 4'h9;
    localparam READ_4 = 4'ha;
    localparam READ_5 = 4'hb;
    localparam READ_6 = 4'hc;

    //state machine state definitions for state_sm3
    localparam IDLE_SM3 = 4'h0;
    localparam CALC_NEXT_READ = 4'h1;
    localparam READ_CALC_4KB = 4'h2;
    localparam READ_CALC_2KB = 4'h3;
    localparam READ_CALC_1KB = 4'h4;
    localparam READ_CALC_512B = 4'h5;
    localparam READ_CALC_256B = 4'h6;
    localparam READ_CALC_128B = 4'h7;
    localparam WAIT_READ_CALC = 4'h8;

    //state machine state definitions for state_sm4
    localparam IDLE_SM4 = 3'b000;
    localparam START_RD = 3'b001;
    localparam WAIT_FOR_RDDONE = 3'b010;
	 
	 //state machine state definitions for TX_state_sm0
	 localparam IDLE_TSM0 = 3'b000;
	 localparam TX_1      = 3'b001;
	 localparam TX_2      = 3'b010;
	 localparam TX_3      = 3'b011;
	 
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

	////////////////////////////////////////////////////////////
	////         Start of Sora Registers               /////////
	////////////////////////////////////////////////////////////
	/// Jiansong: System Registers
	reg [31:0] HWControl; 
	// reg [31:0] PCIeTXBurstSize;     // not used
	// reg [31:0] PCIeRXBlockSize;     // not used
	reg [31:0] HWStatus;            // read only
	reg [15:0] PCIeLinkStatus;      // read only
	reg [15:0] PCIeLinkControl;     // read only
	reg [31:0] FirmwareVersion;	  // read only
	/// Jiansong: Debug Registers, all read only
	reg [31:0] Debug1;              // b0: TXFIFOOverflow, b1: RXFIFOOverflow, b3: illegal tf
	reg [31:0] Debug2;              // TX FIFO read counter
	reg [31:0] Debug3;              // TX FIFO write counter
	reg [31:0] Debug4;              // TX Done counter
	reg [31:0] Debug5SourceAddr_L;  // value in TX descriptor
	reg [31:0] Debug6SourceAddr_H;  // value in TX descriptor
	reg [31:0] Debug7DestAddr;      // value in TX descriptor
	reg [31:0] Debug8FrameSize;     // value in TX descriptor
	reg [31:0] Debug9;              // Egress FIFO overflow counter
	reg [31:0] Debug10;             // RX FIFO overflow counter
	reg [31:0] Debug11TF1;          // Transfer state debug register 1
	reg [31:0] Debug12TF2;          // Transfer state debug register 2
	reg [31:0] Debug13TF3;          // Transfer state debug register 3
	reg [31:0] Debug14TF4;          // Transfer state debug register 4
	reg [31:0] Debug15TX1;          // TX state debug register 1
	reg [31:0] Debug16TX2;          // TX state debug register 2
	reg [31:0] Debug17TX3;          // TX state debug register 3
	reg [31:0] Debug18DDR1;         // DDR state debug register 1
	reg [31:0] Debug19DDR2;         // DDR state debug register 2
	reg [31:0] Debug20RX1;          // RX state debug register 1
	reg [31:0] Debug21RX2;          // RX state debug register 2
	reg [4:0]  Debug22RX3;          // RX state debug register 3
	reg [31:0] Debug23RX4;          // RX state debug register 4
	reg [31:0] Debug24RX5;          // RX state debug register 5
	reg [31:0] Debug25RX6;          // RX state debug register 6
	reg [31:0] Debug26RX7;          // RX state debug register 7
	reg [31:0] Debug27RX8;          // RX state debug register 8
	reg [31:0] Debug28RX9;          // RX state debug register 9
	reg [31:0] Debug29RX10;         // RX state debug register 10
	reg [9:0]  Debug30RXEngine;     // debug register in RX engine
	reg [11:0] Debug31RXDataFIFOfullcnt;  // debug register
	reg [11:0] Debug32RXXferFIFOfullcnt;  // to detect buffer overflow
	reg [23:0] Debug33RXDataFIFOWRcnt;    // Data fifo WR cnt
	reg [23:0] Debug34RXDataFIFORDcnt;    // Data fifo RD cnt
	reg [23:0] Debug35RXXferFIFOWRcnt;    // Xfer fifo WR cnt
	reg [23:0] Debug36RXXferFIFORDcnt;    // Xfer fifo RD cnt
	reg [31:0] Debug37completion_pending;
	reg [4:0]  Debug38tag_value;
	/// Jiansong: DMA Registers
	reg [1:0]  TransferControl;
	reg [31:0] TransferSrcAddr_L;
	reg [31:0] TransferSrcAddr_H;
	reg        RXControl;
	reg [31:0] RXBufAddr_L;
	reg [31:0] RXBufAddr_H;
	reg [31:0] RXBufSize;        /// RX buffer size in bytes
	/// the 2nd RX path
`ifdef SORA_FRL_2nd
	reg [31:0] RXBufAddr_2nd_L;
	reg [31:0] RXBufAddr_2nd_H;
	reg [31:0] RXBufSize_2nd;    /// RX buffer size in bytes
`endif
	// reg [31:0] TransferMask;
	reg [23:0] round_trip_latency;  /// performance value
	reg [23:0] transfer_duration;   /// performance value
	/// Jiansong: Common Radio Registers, pending
	reg		  RadioStatus;      /// Read only
	reg [31:0] RadioID;          /// Read only
	reg [1:0]  TXControl;
	reg [31:0] TXAddr;
	//reg [31:0] TXMask;
	reg [31:0] TXSize;
	/// Jiansong: WARP specific Radio Registers, pending
`ifdef WARP_RADIO_REGISTERS
	reg [31:0] WARP_RFControl;        /// antenna selection
	reg [31:0] WARP_LEDControl;
	//reg [31:0] DIPSW;          /// RadioID
	reg [31:0] WARP_MaximControl;     /// TXEN and RXEN are not used
	reg [31:0] WARP_MaximGainSetting;
	reg [31:0] WARP_MaximStatus;
	reg [31:0] WARP_ADCControl;
	reg [31:0] WARP_ADCStatus;
	reg [31:0] WARP_DACControl;
	reg [31:0] WARP_DACStatus;
	reg [31:0] WARP_MaximSPIControl;
	reg [31:0] WARP_DACSPIControl;
	reg [31:0] WARP_MaximSPIDataIn;
	reg [31:0] WARP_DACSPIDataIn;
	reg [31:0] WARP_DACSPIDataOut;
	reg [31:0] WARP_RSSIADCControl;
	reg [31:0] WARP_RSSIADCData;
`endif
	// radio registers defined for Sora, generalized for all Sora-compatible radios
`ifdef SORA_RADIO_REGISTERS
	reg [1:0]  Sora_RadioControl;
	reg [31:0] Sora_LEDControl;
	reg [31:0] Sora_AntennaSelection;
	reg [31:0] Sora_SampleClock;
	reg [31:0] Sora_CoarseFreq;
	reg [31:0] Sora_FinegradeFreq;
	reg [31:0] Sora_FreqCompensation;
	reg 		  Sora_RadioLOLock;
	reg [31:0] Sora_FilterBandwidth;
	reg [31:0] Sora_TXVGA1;
	reg [31:0] Sora_TXVGA2;
	reg [31:0] Sora_TXPA1;
	reg [31:0] Sora_TXPA2;
	reg [31:0] Sora_RXLNA;
	reg [31:0] Sora_RXPA;
	reg [31:0] Sora_RXVGA1;
	reg [31:0] Sora_RXVGA2;
`endif
`ifdef RADIO_CHANNEL_REGISTERS
	reg [31:0] Channel_RRegValueIn;
	reg 		  Channel_ReadDone;
	reg 		  Channel_Cmd;
	reg [6:0]  Channel_Addr;
	reg [31:0] Channel_RRegValueOut;
	
	wire		  set_Channel_ReadDone_bit;
	reg 		  Channel_ReadDone_in_reg;			// register for cross time domain signal
`endif
	////////////////////////////////////////////////////////////
	////           End of Sora Registers               /////////
	////////////////////////////////////////////////////////////

`ifdef SORA_RADIO_REGISTERS
	reg	SampleClockSet_r1, SampleClockSet_r2, SampleClockSet_r3, SampleClockSet_r4;
	reg	CenterFreqSet_r1, CenterFreqSet_r2, CenterFreqSet_r3, CenterFreqSet_r4;
	reg	FilterBandwidthSet_r1, FilterBandwidthSet_r2, FilterBandwidthSet_r3, FilterBandwidthSet_r4;
`endif

	/// counter to set duration for host reset signal
	reg [7:0] hostresetcnt;

	/// Jiansong: TX registers
	reg TX_calc_next;           // calculate next tx addr and tx size, signal for TX SM 1
	reg next_TX_ready;          // ready to send out next tx request, signal for TX SM 0
	reg stay_2x_TX1;
	reg update_TX_now;          // update TX_*_now
	reg [31:0] TX_addr_next, TX_size_next;
	reg [31:0] TX_addr_now, TX_size_now;
	wire TX_start, TX_start_one;
	reg [2:0] TX_state_0;
	reg [3:0] TX_state_1;
	wire set_TX_done_bit;

	/// Jiansong: pipeline registers
	reg         new_des_one_r;

	//the dma*_now are used for providing DMA parameters to the 
	//DMA Internal CTRL block 
	reg [31:0] dmarad_now;
	reg [63:0] dmaras_now;
	reg [31:0] dmarxs_now;
	//the dma*_next registers make up the "next" set of DMA parameters in a series
	//of DMA transactions 
	reg [31:0] dmarad_next;
	reg [63:0] dmaras_next;
	reg [31:0] dmarxs_next;

	//State machine state variables
	reg [3:0] state_0; 
	reg [3:0] state_3;
	reg [2:0] state_4;

	reg [31:0] reg_data_in_o_r;
	reg [6:0]  reg_wr_addr_o_r;
	reg        reg_wren_o_r;

	reg update_dma_rnow;
	reg set_rd_done_bit;
	reg start_sm0_rdma_flow;
	reg read_calc_next;
	reg stay_2x_3;

	wire RX_FIFO_full_one;

	wire transferstart; //, transferstart_one;

	wire set_transfer_done_bit;
	assign set_transfer_done_bit = set_rd_done_bit;

	/// Jiansong: generate Wait_for_TX_desc signal
	always@(posedge clk)begin
		if(rst | (~transferstart))
			Wait_for_TX_desc <= 0;
		else if (transferstart_one)
			Wait_for_TX_desc <= 1;
		else if (new_des_one_r)
			Wait_for_TX_desc <= 0;
		else
			Wait_for_TX_desc <= Wait_for_TX_desc;
	end 

	assign transferstart = TransferControl[0];
	assign transferstart_o = TransferControl[0];

	/// Jiansong: output signal for tx desc
	always@(posedge clk)begin
		 if(rst | (~transferstart))
			  rd_TX_des_start <= 1'b0;
		 else if (transferstart_one)
			  rd_TX_des_start <= 1'b1;
		 else if (new_des_one_r)
			  rd_TX_des_start <= 1'b0;
		 else
			  rd_TX_des_start <= rd_TX_des_start;
	end

	assign TX_des_addr = {TransferSrcAddr_H,TransferSrcAddr_L};

	/// Jiansong: control signal to radio module
	assign Radio_TX_start = TXControl[0];

	/// Jiansong: control signal to RX path
	assign RXEnable_o  = RXControl;
	assign RXBufAddr_o = {RXBufAddr_H,RXBufAddr_L};
	assign RXBufSize_o = RXBufSize;
	/// control signal to the 2nd RX path
`ifdef SORA_FRL_2nd
	assign RXBufAddr_2nd_o = {RXBufAddr_2nd_H,RXBufAddr_2nd_L};
	assign RXBufSize_2nd_o = RXBufSize_2nd;
`endif

	//drive the *_done outputs to the Internal Control Block
	//with the *_done inputs from the RX and TX engines
	always@(*) rd_dma_done_o = rd_dma_done_i;

	/// Jiansong: this signal controls rd_TX_des_start
	rising_edge_detect rd_dma_start_one_inst(
						 .clk(clk),
						 .rst(rst),
						 .in(transferstart),
						 .one_shot_out(transferstart_one)
						 );

	/// Jiansong: previous reset logic, too simple
	// always@(posedge clk) hostreset <= HWControl[0];

	//////////////////////////////////////////////////////////////////////////////
	//Start of Register File Code
	//////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	/////////////  Jiansong: Registers Write in  ///////////////////
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////
	//// Jiansong: System and DMA registers ////
	////////////////////////////////////////////
	always@(posedge clk) begin
		 if(rst) begin
	//			HWControl          <= 32'h0000_0000; 
	//			PCIeTXBurstSize    <= 32'h0000_0000;
	//			PCIeRXBlockSize    <= 32'h0000_0000;
			 TransferSrcAddr_L  <= 32'h0000_0000;
			 TransferSrcAddr_H  <= 32'h0000_0000;
			 RXControl          <= 1'b0;
			 RXBufAddr_L        <= 32'h0000_0000;
			 RXBufAddr_H        <= 32'h0000_0000;
			 RXBufSize          <= 32'h0000_0000;
			/// the 2nd RX path
`ifdef SORA_FRL_2nd
			 RXBufAddr_2nd_L    <= 32'h0000_0000;
			 RXBufAddr_2nd_H    <= 32'h0000_0000;
			 RXBufSize_2nd      <= 32'h0000_0000;
`endif
	//		 TransferMask       <= 32'h0000_0000;
			 TXAddr             <= 32'h0000_0000;
			 TXSize             <= 32'h0000_0000;
`ifdef WARP_RADIO_REGISTERS
			 WARP_RFControl          <= 32'h0000_0000;
			 WARP_LEDControl         <= 32'h0000_0000;
			 WARP_MaximControl       <= 32'h0000_0000;
			 WARP_MaximGainSetting   <= 32'h0000_0000;
			 WARP_ADCControl         <= 32'h0000_0000;
			 WARP_DACControl         <= 32'h0000_0000;
			 WARP_MaximSPIDataIn     <= 32'h0000_0000;
			 WARP_DACSPIDataIn       <= 32'h0000_0000;
			 WARP_RSSIADCControl     <= 32'h0000_0000;
`endif
`ifdef SORA_RADIO_REGISTERS
			 Sora_RadioControl        	<= 2'b00;
			 Sora_LEDControl          	<= 32'h0000_0000;
			 Sora_AntennaSelection    	<= 32'h0000_0000;
			 Sora_SampleClock         	<= 32'h0000_0000;
			 Sora_CoarseFreq          	<= 32'h0000_0000;
			 Sora_FinegradeFreq       	<= 32'h0000_0000;
			 Sora_FreqCompensation    	<= 32'h0000_0000;
			 Sora_FilterBandwidth      <= 32'h0000_0000;
			 Sora_TXVGA1          		<= 32'h0000_0000;
			 Sora_TXVGA2          		<= 32'h0000_0000;
			 Sora_TXPA1          		<= 32'h0000_0000;
			 Sora_TXPA2          		<= 32'h0000_0000;
			 Sora_RXLNA          		<= 32'h0000_0000;
			 Sora_RXPA          			<= 32'h0000_0000;
			 Sora_RXVGA1          		<= 32'h0000_0000;
			 Sora_RXVGA2          		<= 32'h0000_0000;
`endif
`ifdef RADIO_CHANNEL_REGISTERS
			 Channel_RRegValueIn			<= 32'h0000_0000;
//			 Channel_ReadDone				<= 1'b0;
			 Channel_Cmd					<= 1'b0;
			 Channel_Addr					<= 7'b000_0000;
//			 Channel_RRegValueOut		<= 32'h0000_0000;
`endif
		  end else begin    
				 if(reg_wren) begin 
							case(reg_wr_addr) 
`ifdef RCB_REGISTER_NEW_LAYOUT
							/// Jiansong: System Registers
	//								12'h048: HWControl          <= reg_data_in; //0x00
	//								12'h04C: PCIeTXBurstSize    <= reg_data_in; //0x04
	//								12'h050: PCIeRXBlockSize    <= reg_data_in; //0x08
							/// Jiansong: DMA Registers
									12'h188: TransferSrcAddr_L  <= reg_data_in; //0x104
									12'h18C: TransferSrcAddr_H  <= reg_data_in; //0x108
									12'h164: RXControl          <= reg_data_in[0]; //0x10C
									12'h170: RXBufAddr_L        <= reg_data_in; //0x110
									12'h174: RXBufAddr_H        <= reg_data_in; //0x114
									12'h194: RXBufSize          <= reg_data_in; //0x118
							/// the 2nd RX path
`ifdef SORA_FRL_2nd
									12'h270: RXBufAddr_2nd_L    <= reg_data_in; //0x210
									12'h274: RXBufAddr_2nd_H    <= reg_data_in; //0x214
									12'h294: RXBufSize_2nd      <= reg_data_in; //0x218
`endif
	//								12'h16C: TransferMask       <= reg_data_in; //0x11C
							/// Jiansong: Common Radio Registers, pending
									12'h178: TXAddr             <= reg_data_in;
									12'h190: TXSize             <= reg_data_in;
`else // RCB_REGISTER_NEW_LAYOUT
							/// Jiansong: System Registers
	//								12'h000: HWControl          <= reg_data_in; //0x00
	//								12'h004: PCIeTXBurstSize    <= reg_data_in; //0x04
	//								12'h008: PCIeRXBlockSize    <= reg_data_in; //0x08
							/// Jiansong: DMA Registers
									12'h104: TransferSrcAddr_L  <= reg_data_in; //0x104
									12'h108: TransferSrcAddr_H  <= reg_data_in; //0x108
									12'h10C: RXControl          <= reg_data_in[0]; //0x10C
									12'h110: RXBufAddr_L        <= reg_data_in; //0x110
									12'h114: RXBufAddr_H        <= reg_data_in; //0x114
									12'h118: RXBufSize          <= reg_data_in; //0x118
							/// the 2nd RX path
`ifdef SORA_FRL_2nd
									12'h210: RXBufAddr_2nd_L    <= reg_data_in; //0x210
									12'h214: RXBufAddr_2nd_H    <= reg_data_in; //0x214
									12'h218: RXBufSize_2nd      <= reg_data_in; //0x218
`endif
	//								12'h11C: TransferMask       <= reg_data_in; //0x11C
							/// Jiansong: Common Radio Registers, pending
									12'h184: TXAddr             <= reg_data_in;
									12'h18C: TXSize             <= reg_data_in;
`endif // RCB_REGISTER_NEW_LAYOUT

`ifdef WARP_RADIO_REGISTERS
							/// Jiansong: WARP Radio Registers, pending
									12'h190: WARP_RFControl          <= reg_data_in;
									12'h194: WARP_LEDControl         <= reg_data_in;
									12'h19C: WARP_MaximControl       <= reg_data_in;
									12'h1A0: WARP_MaximGainSetting   <= reg_data_in;
									12'h1A8: WARP_ADCControl         <= reg_data_in;
									12'h1B0: WARP_DACControl         <= reg_data_in;
									12'h1C0: WARP_MaximSPIDataIn     <= reg_data_in;
									12'h1C4: WARP_DACSPIDataIn       <= reg_data_in;
									12'h1CC: WARP_RSSIADCControl     <= reg_data_in;
`endif
`ifdef SORA_RADIO_REGISTERS
									12'h190: Sora_RadioControl       <= reg_data_in[1:0];
									12'h194: Sora_LEDControl         <= reg_data_in;
									12'h198: Sora_AntennaSelection   <= reg_data_in;
									12'h19C: Sora_SampleClock        <= reg_data_in;
									12'h1A0: Sora_CoarseFreq         <= reg_data_in;
									12'h1A4: Sora_FinegradeFreq      <= reg_data_in;
									12'h1A8: Sora_FreqCompensation   <= reg_data_in;
									12'h1B0: Sora_FilterBandwidth    <= reg_data_in;
									12'h1B4: Sora_TXVGA1          	<= reg_data_in;
									12'h1B8: Sora_TXVGA2          	<= reg_data_in;
									12'h1BC: Sora_TXPA1          		<= reg_data_in;
									12'h1C0: Sora_TXPA2          		<= reg_data_in;
									12'h1C4: Sora_RXLNA          		<= reg_data_in;
									12'h1C8: Sora_RXPA          		<= reg_data_in;
									12'h1CC: Sora_RXVGA1          	<= reg_data_in;
									12'h1D0: Sora_RXVGA2					<= reg_data_in;
`endif
`ifdef RADIO_CHANNEL_REGISTERS
									12'h1C0: Channel_RRegValueIn		<= reg_data_in;
									12'h1C4: begin
//												Channel_ReadDone			<= reg_data_in[8];
												Channel_Cmd					<= reg_data_in[7];
												Channel_Addr[6:0]			<= reg_data_in[6:0];
									end
//									12'h1C8: Channel_RRegValueOut		<= reg_data_in;
`endif
									default: begin 
	//                                    HWControl          <= HWControl; 
	//                                    PCIeTXBurstSize    <= PCIeTXBurstSize;
	//                                    PCIeRXBlockSize    <= PCIeRXBlockSize;
													TransferSrcAddr_L  <= TransferSrcAddr_L;
													TransferSrcAddr_H  <= TransferSrcAddr_H;
													RXControl          <= RXControl;
													RXBufAddr_L        <= RXBufAddr_L;
													RXBufAddr_H        <= RXBufAddr_H;
													RXBufSize          <= RXBufSize;
							/// the 2nd RX path
`ifdef SORA_FRL_2nd
													RXBufAddr_2nd_L    <= RXBufAddr_L_2nd;
													RXBufAddr_2nd_H    <= RXBufAddr_H_2nd;
													RXBufSize_2nd      <= RXBufSize_2nd;
`endif
	//		                              TransferMask       <= TransferMask;
													TXAddr             <= TXAddr;
													TXSize             <= TXSize;
`ifdef WARP_RADIO_REGISTERS
													WARP_RFControl          <= WARP_RFControl;
													WARP_LEDControl         <= WARP_LEDControl;
													WARP_MaximControl       <= WARP_MaximControl;
													WARP_MaximGainSetting   <= WARP_MaximGainSetting;
													WARP_ADCControl         <= WARP_ADCControl;
													WARP_DACControl         <= WARP_DACControl;
													WARP_MaximSPIDataIn     <= WARP_MaximSPIDataIn;
													WARP_DACSPIDataIn       <= WARP_DACSPIDataIn;
													WARP_RSSIADCControl     <= WARP_RSSIADCControl;
`endif
`ifdef SORA_RADIO_REGISTERS
													Sora_RadioControl        	<= Sora_RadioControl;
			 										Sora_LEDControl          	<= Sora_LEDControl;
			 										Sora_AntennaSelection    	<= Sora_AntennaSelection;
			 										Sora_SampleClock         	<= Sora_SampleClock;
			 										Sora_CoarseFreq          	<= Sora_CoarseFreq;
			 										Sora_FinegradeFreq       	<= Sora_FinegradeFreq;
			 										Sora_FreqCompensation    	<= Sora_FreqCompensation;
			 										Sora_FilterBandwidth       <= Sora_FilterBandwidth;
			 										Sora_TXVGA1          		<= Sora_TXVGA1;
			 										Sora_TXVGA2          		<= Sora_TXVGA2;
			 										Sora_TXPA1          			<= Sora_TXPA1;
			 										Sora_TXPA2          			<= Sora_TXPA2;
			 										Sora_RXLNA          			<= Sora_RXLNA;
			 										Sora_RXPA          			<= Sora_RXPA;
			 										Sora_RXVGA1          		<= Sora_RXVGA1;
			 										Sora_RXVGA2          		<= Sora_RXVGA2;
`endif
`ifdef RADIO_CHANNEL_REGISTERS
													Channel_RRegValueIn			<= Channel_RRegValueIn;
//													Channel_ReadDone				<= Channel_ReadDone;
													Channel_Cmd						<= Channel_Cmd;
													Channel_Addr					<= Channel_Addr;
//													Channel_RRegValueOut			<= Channel_RRegValueOut;
`endif												
									end
								endcase
				  end 
			  end
		  end
		  
	/// Jiansong: logic for host reset, it's only controlled by hardware reset
	always@(posedge clk)begin
		if (hostresetcnt <= 8'hC8 && hostresetcnt > 8'h00)
			hostreset <= 1'b1;
		else
			hostreset <= 1'b0;
	end
	always@(posedge clk)begin
		if(hard_rst) begin
			HWControl    <= 32'h0000_0000;
			hostresetcnt <= 8'h00;
		end else begin
				if (reg_wren) begin
					case(reg_wr_addr)
`ifdef RCB_REGISTER_NEW_LAYOUT				
							12'h048: begin
`else // RCB_REGISTER_NEW_LAYOUT					
							12'h000: begin
`endif // RCB_REGISTER_NEW_LAYOUT
								HWControl    <= 32'h0000_0001;
								hostresetcnt <= 8'hC8;    // host reset for 200 cycles
							end
							default: begin
								if (hostresetcnt == 8'h00)begin
									HWControl    <= 32'h0000_0000;
									hostresetcnt <= hostresetcnt;
								end else begin
									HWControl    <= 32'h0000_0001;
									hostresetcnt <= hostresetcnt - 8'h01;
								end
							end
					endcase						
				end else begin
					if (hostresetcnt == 8'h00)begin
						HWControl    <= 32'h0000_0000;
						hostresetcnt <= hostresetcnt;
					end else begin
						HWControl    <= 32'h0000_0001;
						hostresetcnt <= hostresetcnt - 8'h01;
					end
				end
		end
	end
		  
	/// Jiansong: transfer start and done
	//use a separate always block for TransferInit and TransferDone in TransferControl register
	//TransferInit:     host sets this bit to start a dma transfer
	//                            
	//TransferDone:     asserted when the dma transfer is finished
	//                  this bit can be polled by the host or it could
	//                  be used to drive hardware block to generate an interrupt
	//                  this bit must be clear by the host by write a '0' to transferinit
	always@(posedge clk) begin
		 if(rst) begin
			 TransferControl[1:0]   <= 2'b00;
		  end else begin   
				//set_transfer_done_bit from ??? state machine
				if(set_transfer_done_bit) begin  
						TransferControl[1] <= 1'b1; //set the TransferDone bit
				 end else if(reg_wren) begin 
							case(reg_wr_addr)
`ifdef RCB_REGISTER_NEW_LAYOUT							
									12'h168: begin  //addr = 0x168
`else // RCB_REGISTER_NEW_LAYOUT							
									12'h100: begin  //addr = 0x100
`endif // RCB_REGISTER_NEW_LAYOUT					
										  //set the start bit if the host writes a 1
										  //the host will clear this bit after TransferDone bit is set
										  if(reg_data_in[0]) 
											  TransferControl[0] <= 1'b1;
										  else begin
										  //clear the TransferDone bit if the host clears the TransferInit bit
											  TransferControl[0] <= 1'b0;
											  TransferControl[1] <= 1'b0;
										  end                             
									 end
									 default: begin 
											  TransferControl[1:0] <= TransferControl[1:0];
									end
								endcase
				  end 
			  end
		  end
		  
	always@(posedge clk) round_trip_latency <= round_trip_latency_i;
	always@(posedge clk) transfer_duration <= transfer_duration_i;
		  
	rising_edge_detect set_TX_done_one_inst(
						 .clk(clk),
						 .rst(rst),
						 .in(Radio_TX_done),
						 .one_shot_out(set_TX_done_bit)
						 );
	/// Jiansong: TX start and TX done
	always@(posedge clk)begin
		if(rst)
			TXControl[1:0] <= 2'b00;
		else begin
			if(set_TX_done_bit) begin
				TXControl[1] <= 1'b1;
			end else if (reg_wren)begin
					  case(reg_wr_addr)
`ifdef RCB_REGISTER_NEW_LAYOUT					  
							  12'h184: begin
`else // RCB_REGISTER_NEW_LAYOUT					  
							  12'h180: begin
`endif // RCB_REGISTER_NEW_LAYOUT							  
								  if(reg_data_in[0])
									  TXControl[0] <= 1'b1;
								  else begin
									  TXControl[1] <= 1'b0;
									  TXControl[0] <= 1'b0;
								  end
							  end
							  default: begin
								  TXControl[1:0] <= TXControl[1:0];
							  end
						endcase
			end
		end
	end

	/// Jiansong: phy_init_done, pending	  
	//block for the rest of the TransferControl bits which are read only
	always@(posedge clk) begin
		 if(rst) begin
			 HWStatus[31:0]        <= 32'h0000_0000;
		  end else begin   
				//HWStatus[0] always mirrors phy_init_done so that the host application
				//can determine the readiness of the memory controller
				HWStatus[31:2]		<= 30'h0000_0000;	
				HWStatus[0]       <= phy_init_done;
				HWStatus[1]       <= ~trn_lnk_up_n_c;
		  end
	end

	always@(posedge clk) PCIeLinkStatus[15:0]  <= PCIeLinkStatus_in[15:0];
	always@(posedge clk) PCIeLinkControl[15:0] <= PCIeLinkControl_in[15:0];

	always@(posedge clk) FirmwareVersion[31:0] <= 32'h0105_0000;			// RCB firmware version is 1.5

	//////////////////////////////////
	/// Jiansong: debug registers ////
	//////////////////////////////////
	rising_edge_detect RX_FIFO_full_one_inst(
						 .clk(clk),
						 .rst(rst),
						 .in(RX_FIFO_full),
						 .one_shot_out(RX_FIFO_full_one)
						 );
	always@(posedge clk)begin
		if(rst)begin
			Debug1[31:16] <= 16'h0000;
			Debug1[3:0]   <= 4'h0;
			Debug2 <= 32'h0000_0000;
			Debug3 <= 32'h0000_0000;
		end else if (egress_overflow_one) begin
			Debug1[0]    <= 1'b1;
			Debug1[3:1]  <= Debug1[3:1];
			Debug1[31:16] <= Debug1[31:16];
		end else if (RX_FIFO_full_one) begin
			Debug1[1]    <= 1'b1;
			Debug1[0] <= Debug1[0];
			Debug1[3:2]   <= Debug1[3:2];
			Debug1[31:16] <= Debug1[31:16];
		end else if (transferstart_one && (dmarxs_now[31:7] != 0)) begin
			Debug1[2] <= 1'b1;
		end else if (~RXControl) begin   /// clear this RX_FIFO_full signal if RX is disabled
			Debug1[1]    <= 1'b0;
			Debug1[0] <= Debug1[0];
			Debug1[3:2]   <= Debug1[3:2];
			Debug1[31:16] <= Debug1[31:16];
		end else begin
			Debug1[3:0]  <= Debug1[3:0];
			Debug1[31:16] <= {11'h000,locked_debug[4:0]};
			Debug2 <= egress_rd_data_count;
			Debug3 <= egress_wr_data_count;
		end
	end

	always@(posedge clk)begin
		if(rst)begin
			Debug1[15:4]  <= 12'h000;
		end else begin
				Debug1[11:4] <= FIFOErrors_in[7:0];
				Debug1[15:12] <= Debug1[15:12];
		end
	end

	/// TX done times
	always@(posedge clk)begin
		if(rst)
			Debug4 <= 32'hFFFF_FFFF;
		else if(set_TX_done_bit)
			Debug4 <= Debug4 + 32'h0000_0001;
		else
			Debug4 <= Debug4;
	end

	always@(posedge clk)begin
		Debug5SourceAddr_L <= SourceAddr_L;  //
		Debug6SourceAddr_H <= SourceAddr_H;
		Debug7DestAddr     <= DestAddr;
		Debug8FrameSize    <= FrameSize;
	end

	always@(posedge clk)begin
		if(rst)
			Debug9 <= 32'h0000_0000;
		else if (egress_overflow_one)
			Debug9 <= Debug9 + 32'h0000_0001;
		else
			Debug9 <= Debug9;
	end

	always@(posedge clk)begin
		if(rst)
			Debug10 <= 32'h0000_0000;
		else if (RX_FIFO_full_one)
			Debug10 <= Debug10 + 32'h0000_0001;
		else
			Debug10 <= Debug10;
	end

	always@(posedge clk)begin
		Debug11TF1[3:0]   <= state_0[3:0];
		Debug11TF1[7:4]   <= state_3[3:0];
		Debug11TF1[11:8]  <= {1'b0,state_4[2:0]};
		Debug11TF1[15:12] <= {3'b000,start_sm0_rdma_flow};
		Debug11TF1[19:16] <= {3'b000,read_last};
		Debug11TF1[23:20] <= {3'b000,pause_read_requests};
		Debug11TF1[27:24] <= {3'b000,TX_desc_write_back_req};
		Debug11TF1[31:28] <= {3'b000,rd_dma_done_i};
	end

	always@(posedge clk) Debug12TF2[31:0] <= dmarad_now[31:0];
	always@(posedge clk) Debug13TF3[31:0] <= dmaras_now[31:0];
	always@(posedge clk) Debug14TF4[31:0] <= dmarxs_now[31:0];

	always@(posedge clk)begin
		Debug15TX1[3:0]   <= {1'b0,TX_state_0[2:0]};
		Debug15TX1[7:4]   <= TX_state_1[3:0];
		Debug15TX1[11:8]  <= {3'b0,TX_start};
		Debug15TX1[15:12] <= {3'b0,Radio_TX_done};
		Debug15TX1[19:16] <= {3'b0,next_TX_ready};
		Debug15TX1[31:20] <= 12'h000;
	end

	always@(posedge clk)begin
		Debug16TX2[31:0] <= TX_addr_now[31:0];
	end

	always@(posedge clk)begin
		Debug17TX3[31:0] <= TX_size_now[31:0];
	end

	always@(posedge clk) Debug18DDR1[31:0] <= Debug18DDR1_in[31:0];
	always@(posedge clk) Debug19DDR2[31:0] <= Debug19DDR2_in[31:0];
	always@(posedge clk) Debug20RX1[31:0] <= Debug20RX1_in[31:0];
	always@(posedge clk) Debug21RX2[31:0] <= Debug21RX2_in[31:0];
	always@(posedge clk) Debug22RX3[4:0]  <= Debug22RX3_in[4:0];
	always@(posedge clk) Debug23RX4[31:0] <= Debug23RX4_in[31:0];
	always@(posedge clk) Debug24RX5[31:0] <= Debug24RX5_in[31:0];
	always@(posedge clk) Debug25RX6[31:0] <= Debug25RX6_in[31:0];
	always@(posedge clk) Debug26RX7[31:0] <= Debug26RX7_in[31:0];
	always@(posedge clk) Debug27RX8[31:0] <= Debug27RX8_in[31:0];
	always@(posedge clk) Debug28RX9[31:0] <= Debug28RX9_in[31:0];
	always@(posedge clk) Debug29RX10[31:0] <= Debug29RX10_in[31:0];
	always@(posedge clk) Debug30RXEngine[9:0] <= Debug30RXEngine_in[9:0];
	always@(posedge clk) Debug31RXDataFIFOfullcnt[11:0] <= Debug31RXDataFIFOfullcnt_in[11:0];
	always@(posedge clk) Debug32RXXferFIFOfullcnt[11:0] <= Debug32RXXferFIFOfullcnt_in[11:0];
	always@(posedge clk) Debug33RXDataFIFOWRcnt[23:0] <= Debug33RXDataFIFOWRcnt_in[23:0];
	always@(posedge clk) Debug34RXDataFIFORDcnt[23:0] <= Debug34RXDataFIFORDcnt_in[23:0];
	always@(posedge clk) Debug35RXXferFIFOWRcnt[23:0] <= Debug35RXXferFIFOWRcnt_in[23:0];
	always@(posedge clk) Debug36RXXferFIFORDcnt[23:0] <= Debug36RXXferFIFORDcnt_in[23:0];
	always@(posedge clk) Debug37completion_pending[31:0] <= Debug37completion_pending_in[31:0];
	always@(posedge clk) Debug38tag_value[4:0] <= Debug38tag_value_in[4:0];
		  
	///////////////////////////////////////////////
	/// Jiansong: radio registers interpreting ////
	///////////////////////////////////////////////
`ifdef WARP_RADIO_REGISTERS
	always@(posedge clk)begin
		RadioStatus		   <= 1'b1;            // radio always alive
//		RadioStatus[31:1] <= 31'h0000_0000;
		RadioID[3:0]      <= RadioDIPSW[3:0];      // id of WARP radio, decided by the switch on WARP radio
		RadioID[31:4]     <= 28'h0001_000;    // class id of WARP radio, this is the first radio of Sora
	end

	assign RadioAntSelect[1:0]   = WARP_RFControl[1:0];
	assign RadioLEDControl[2:0]  = WARP_LEDControl[2:0];
	assign RadioMaximSHDN        = WARP_MaximControl[0];
	assign RadioMaximReset       = WARP_MaximControl[1];
	assign RadioMaximRXHP        = WARP_MaximControl[2]; 
	assign RadioTXGainSetting[5:0] = WARP_MaximGainSetting[5:0];
	assign RadioRXGainSetting[6:0] = WARP_MaximGainSetting[22:16];

	always@(posedge clk)begin
		WARP_MaximStatus[0]    <= RadioLD;
		WARP_MaximStatus[31:1] <= 31'h0000_0000;
	end

	assign RadioADCControl[3:0] = WARP_ADCControl[3:0];

	always@(posedge clk)begin
		WARP_ADCStatus[1:0]    <= RadioADCStatus[1:0];
		WARP_ADCStatus[31:2]   <= 30'h0000_0000;
	end

	assign RadioDACControl = WARP_DACControl[0];

	always@(posedge clk)begin
		WARP_DACStatus[0]      <= RadioDACStatus;
		WARP_DACStatus[31:1]   <= 31'h0000_0000;
	end

	always@(posedge clk)begin
		if(rst)
			WARP_MaximSPIControl[0] <= 1'b0;
		else if (reg_wren)begin
			  case(reg_wr_addr)
					  12'h1B8: begin
								 if(reg_data_in[0])
									 WARP_MaximSPIControl[0] <= 1'b1;
								 else 
									 WARP_MaximSPIControl[0] <= 1'b0;
							 end
					  default: 
								WARP_MaximSPIControl[0] <= WARP_MaximSPIControl[0];
			  endcase
		end
	end
	// simply forward SPI start and done to radio module
	always@(posedge clk) WARP_MaximSPIControl[1] <= RadioMaximSPIDone;
	assign RadioMaximSPIStart = WARP_MaximSPIControl[0];
	always@(posedge clk) WARP_MaximSPIControl[31:2] <= 30'h0000_0000;

	always@(posedge clk)begin
		if(rst)
			WARP_DACSPIControl[0] <= 1'b0;
		else if (reg_wren)begin
			  case(reg_wr_addr)
					  12'h1BC: begin
								 if(reg_data_in[0])
									 WARP_DACSPIControl[0] <= 1'b1;
								 else 
									 WARP_DACSPIControl[0] <= 1'b0;
							 end
					  default: 
								WARP_DACSPIControl[0] <= WARP_DACSPIControl[0];
			  endcase
		end
	end
	// simply forward SPI start and done to radio module
	always@(posedge clk) WARP_DACSPIControl[1] <= RadioDACSPIDone;
	assign RadioDACSPIStart = WARP_DACSPIControl[0];
	always@(posedge clk) WARP_DACSPIControl[31:2] <= 30'h0000_0000;

	assign RadioMaximSPIAddr[3:0]  = WARP_MaximSPIDataIn[3:0];
	assign RadioMaximSPIData[13:0] = WARP_MaximSPIDataIn[17:4];

	assign RadioDACSPIData[7:0]    = WARP_DACSPIDataIn[7:0];
	assign RadioDACSPIInstuct[7:0] = WARP_DACSPIDataIn[15:8];

	always@(posedge clk)begin
		WARP_DACSPIDataOut[7:0]  <= RadioDACSPIDataOut[7:0];
		WARP_DACSPIDataOut[31:8] <= 24'h0000_00;
	end

	assign RadioRSSIADCControl[2:0] = WARP_RSSIADCControl[2:0];

	always@(posedge clk)begin
		WARP_RSSIADCData[9:0]   <= RadioRSSIData[9:0];
		WARP_RSSIADCData[15:10] <= 6'b0000_00;
		WARP_RSSIADCData[16]    <= RadioRSSIOTR;
		WARP_RSSIADCData[31:17] <= 15'h0000;	
	end
`endif

`ifdef SORA_RADIO_REGISTERS
	assign RadioControl[1:0] = Sora_RadioControl[1:0];
	
	always@(posedge clk)begin
		RadioStatus 	<= RadioStatus_in;
		RadioID[31:0]	<= RadioID_in[31:0];
	end
	
	assign LEDControl[31:0] 		= Sora_LEDControl[31:0];
	assign AntennaSelection[31:0] = Sora_AntennaSelection[31:0];
	assign SampleClock[31:0]		= Sora_SampleClock[31:0];
	always@(posedge clk)begin
		SampleClockSet_r2 <= SampleClockSet_r1;
		SampleClockSet_r3 <= SampleClockSet_r2;
		SampleClockSet_r4 <= SampleClockSet_r3;
		if (reg_wr_addr[11:0] == 12'h19C)
			SampleClockSet_r1 <= reg_wren;
		else
			SampleClockSet_r1 <= 1'b0;
	end
	assign SampleClockSet = SampleClockSet_r1 | SampleClockSet_r2 | SampleClockSet_r3 | SampleClockSet_r4;
	
	assign CoarseFreq[31:0]			= Sora_CoarseFreq[31:0];
	assign FinegradeFreq[31:0]		= Sora_FinegradeFreq[31:0];
	assign FreqCompensation[31:0]	= Sora_FreqCompensation[31:0];
	always@(posedge clk)begin
		CenterFreqSet_r2 <= CenterFreqSet_r1;
		CenterFreqSet_r3 <= CenterFreqSet_r2;
		CenterFreqSet_r4 <= CenterFreqSet_r3;
		if ((reg_wr_addr[11:0] == 12'h1A4) | (reg_wr_addr[11:0] == 12'h1A8) )
			CenterFreqSet_r1 <= reg_wren;
		else
			CenterFreqSet_r1 <= 1'b0;
	end
	assign CenterFreqSet = CenterFreqSet_r1 | CenterFreqSet_r2 | CenterFreqSet_r3 | CenterFreqSet_r4;
	
	always@(posedge clk)begin
		Sora_RadioLOLock <= RadioLOLock;
	end
	
	assign FilterBandwidth[31:0]	= Sora_FilterBandwidth[31:0];
	always@(posedge clk)begin
		FilterBandwidthSet_r2 <= FilterBandwidthSet_r1;
		FilterBandwidthSet_r3 <= FilterBandwidthSet_r2;
		FilterBandwidthSet_r4 <= FilterBandwidthSet_r3;
		if (reg_wr_addr[11:0] == 12'h1B0)
			FilterBandwidthSet_r1 <= reg_wren;
		else
			FilterBandwidthSet_r1 <= 1'b0;
	end
	assign FilterBandwidthSet = FilterBandwidthSet_r1 | FilterBandwidthSet_r2 | FilterBandwidthSet_r3 | FilterBandwidthSet_r4;
	
	assign TXVGA1[31:0]				= Sora_TXVGA1[31:0];
	assign TXVGA2[31:0]				= Sora_TXVGA2[31:0];
	assign TXPA1[31:0]				= Sora_TXPA1[31:0];
	assign TXPA2[31:0]				= Sora_TXPA2[31:0];
	assign RXLNA[31:0]				= Sora_RXLNA[31:0];
	assign RXPA[31:0]					= Sora_RXPA[31:0];
	assign RXVGA1[31:0]				= Sora_RXVGA1[31:0];
	assign RXVGA2[31:0]				= Sora_RXVGA2[31:0];
`endif

`ifdef RADIO_CHANNEL_REGISTERS
	always@(posedge clk)begin				// this hard-coded implementation is only for single radio, driver will periodically check this bit
		RadioStatus		   <= 1'b1;
	end

	// Generate radio cmd
	always@(posedge clk) begin
		if(rst) begin
			Radio_Cmd_Data[31:0] <= 32'h0000_0000;
			Radio_Cmd_Addr[6:0]  <= 7'b000_0000;
			Radio_Cmd_RdWr			<= 1'b0;
			Radio_Cmd_wren			<= 1'b0;
		end else if(reg_wren) begin
				case(reg_wr_addr[11:0])
					12'h1C4: begin
						Radio_Cmd_Data[31:0] <= Channel_RRegValueIn[31:0];
						Radio_Cmd_Addr[6:0]  <= reg_data_in[6:0];
						Radio_Cmd_RdWr			<= reg_data_in[7];
						Radio_Cmd_wren			<= 1'b1;
					end
					// also pass TXControl and RXControl to Radio Module
`ifdef RCB_REGISTER_NEW_LAYOUT
					12'h184: begin			// TXControl
`else // RCB_REGISTER_NEW_LAYOUT
					12'h180: begin			// TXControl
`endif // RCB_REGISTER_NEW_LAYOUT					
						Radio_Cmd_Data[31:0] <= reg_data_in[31:0];
						Radio_Cmd_Addr[6:0]  <= 7'h02;
						Radio_Cmd_RdWr			<= 1'b0;
						Radio_Cmd_wren			<= 1'b1;
					end
`ifdef RCB_REGISTER_NEW_LAYOUT
					12'h164: begin			// RXControl					
`else // RCB_REGISTER_NEW_LAYOUT
					12'h10C: begin			// RXControl
`endif // RCB_REGISTER_NEW_LAYOUT				
						Radio_Cmd_Data[31:0] <= reg_data_in[31:0];
						Radio_Cmd_Addr[6:0]  <= 7'h03;
						Radio_Cmd_RdWr			<= 1'b0;
						Radio_Cmd_wren			<= 1'b1;
					end
					default: begin
						Radio_Cmd_Data[31:0] <= 32'h0000_0000;
						Radio_Cmd_Addr[6:0]  <= 7'b000_0000;
						Radio_Cmd_RdWr			<= 1'b0;
						Radio_Cmd_wren			<= 1'b0;
					end
				endcase
		end else begin
			Radio_Cmd_Data[31:0] <= 32'h0000_0000;
			Radio_Cmd_Addr[6:0]  <= 7'b000_0000;
			Radio_Cmd_RdWr			<= 1'b0;
			Radio_Cmd_wren			<= 1'b0;
		end
	end

	// Channel_ReadDone bit & Channel_RRegValueOut
	always@(posedge clk) begin
		if(rst) begin
			Channel_ReadDone <= 1'b0;
			Channel_RRegValueOut[31:0] <= 32'hA0B0_C0D0;
		end else if (reg_wren) begin
						case(reg_wr_addr) 
							12'h1C4: begin
								Channel_ReadDone <= 1'b0;			// means this bit is write-only, any write from PC to this bit will clear this bit
								Channel_RRegValueOut[31:0] <= 32'hA0B0_C0D0; // frequently set to default value
							end
							default: begin									// write in other registers will be bypassed
								Channel_ReadDone <= Channel_ReadDone;
								Channel_RRegValueOut[31:0] <= Channel_RRegValueOut[31:0];
							end
						endcase
		end else if (set_Channel_ReadDone_bit) begin
						Channel_ReadDone <= 1'b1;
						Channel_RRegValueOut[31:0] <= Channel_Reg_Read_Value[31:0];
		end else begin
				Channel_ReadDone <= Channel_ReadDone;
				Channel_RRegValueOut[31:0] <= Channel_RRegValueOut[31:0];
		end
	end
	
	always@(posedge clk) Channel_ReadDone_in_reg <= Channel_ReadDone_in;    // add one register for signal which crosses time domain
	// Channel_ReadDone_in crosses time domain (from 44MHz to 125MHz), it's better to do edge detection
	rising_edge_detect Channel_ReadDone_one_inst(
						 .clk(clk),
						 .rst(rst),
						 .in(Channel_ReadDone_in_reg),
						 .one_shot_out(set_Channel_ReadDone_bit)
						 );
		
`endif

	////////////////////////////////////////////////////////////////
	/////////////  Jiansong: Registers Read out  ///////////////////
	////////////////////////////////////////////////////////////////
	/// Jiansong: register/memory read logic, our design should be added, pending
	// output register for cpu
	// this is a read of the reg_file
	// the case stmt is a mux which selects which reg location
	// makes it to the output data bus
	always@(posedge clk) 
	  begin                                                             
		  if(rst)                          
				 begin
					reg_data_out <= 0;
				 end
		  else
				begin   // read from reg_file location
					case(reg_rd_addr[11:0])
`ifdef RCB_REGISTER_NEW_LAYOUT
					  /// Jiansong: System Registers
					  12'h048: reg_data_out <= HWControl;
	//              12'h04C: reg_data_out <= PCIeTXBurstSize;
	//              12'h050: reg_data_out <= PCIeRXBlockSize;
					  12'h03C: reg_data_out <= HWStatus;
					  12'h040: reg_data_out <= {16'h0000,PCIeLinkStatus};
					  12'h044: reg_data_out <= {16'h0000,PCIeLinkControl};
					  12'h038: reg_data_out <= FirmwareVersion;
					  /// Jiansong: DMA Registers
					  12'h168: reg_data_out <= {30'h0000_0000,TransferControl};
					  12'h188: reg_data_out <= TransferSrcAddr_L;
					  12'h18C: reg_data_out <= TransferSrcAddr_H;
					  12'h164: reg_data_out <= {31'h0000_0000,RXControl};
					  12'h170: reg_data_out <= RXBufAddr_L;
					  12'h174: reg_data_out <= RXBufAddr_H;
					  12'h194: reg_data_out <= RXBufSize;
					  /// the 2nd RX path
`ifdef SORA_FRL_2nd
					  12'h270: reg_data_out <= RXBufAddr_2nd_L;
					  12'h274: reg_data_out <= RXBufAddr_2nd_H;
					  12'h294: reg_data_out <= RXBufSize_2nd;
`endif // SORA_FRL_2nd
	//              12'h16C: reg_data_out <= TransferMask;
					  12'h15C: reg_data_out <= {8'h00,round_trip_latency};
					  12'h160: reg_data_out <= {8'h00,transfer_duration};
					  /// Jiansong: Common Radio Registers
					  12'h154: reg_data_out <= {31'h0000_0000,RadioStatus};
					  12'h158: reg_data_out <= RadioID;
					  12'h184: reg_data_out <= {30'h0000_0000,TXControl};
					  12'h178: reg_data_out <= TXAddr;
					  12'h190: reg_data_out <= TXSize;
`else // RCB_REGISTER_NEW_LAYOUT
					  /// Jiansong: System Registers
					  12'h000: reg_data_out <= HWControl;
	//              12'h004: reg_data_out <= PCIeTXBurstSize;
	//              12'h008: reg_data_out <= PCIeRXBlockSize;
					  12'h00C: reg_data_out <= HWStatus;
					  12'h010: reg_data_out <= {16'h0000,PCIeLinkStatus};
					  12'h014: reg_data_out <= {16'h0000,PCIeLinkControl};
					  12'h018: reg_data_out <= FirmwareVersion;
					  /// Jiansong: DMA Registers
					  12'h100: reg_data_out <= {30'h0000_0000,TransferControl};
					  12'h104: reg_data_out <= TransferSrcAddr_L;
					  12'h108: reg_data_out <= TransferSrcAddr_H;
					  12'h10C: reg_data_out <= {31'h0000_0000,RXControl};
					  12'h110: reg_data_out <= RXBufAddr_L;
					  12'h114: reg_data_out <= RXBufAddr_H;
					  12'h118: reg_data_out <= RXBufSize;
					  /// the 2nd RX path
`ifdef SORA_FRL_2nd
					  12'h210: reg_data_out <= RXBufAddr_2nd_L;
					  12'h214: reg_data_out <= RXBufAddr_2nd_H;
					  12'h218: reg_data_out <= RXBufSize_2nd;
`endif // SORA_FRL_2nd
	//              12'h11C: reg_data_out <= TransferMask;
					  12'h124: reg_data_out <= {8'h00,round_trip_latency};
					  12'h128: reg_data_out <= {8'h00,transfer_duration};
					  /// Jiansong: Common Radio Registers
					  12'h170: reg_data_out <= {31'h0000_0000,RadioStatus};
					  12'h174: reg_data_out <= RadioID;
					  12'h180: reg_data_out <= {30'h0000_0000,TXControl};
					  12'h184: reg_data_out <= TXAddr;
					  12'h18C: reg_data_out <= TXSize;
`endif // RCB_REGISTER_NEW_LAYOUT 

					  /// Jiansong: Debug registers
					  12'h080: reg_data_out <= Debug1;
					  12'h084: reg_data_out <= Debug2;
					  12'h088: reg_data_out <= Debug3;
					  12'h08C: reg_data_out <= Debug4;
					  12'h090: reg_data_out <= Debug5SourceAddr_L;
					  12'h094: reg_data_out <= Debug6SourceAddr_H;
					  12'h098: reg_data_out <= Debug7DestAddr;
					  12'h09C: reg_data_out <= Debug8FrameSize;
					  12'h0A0: reg_data_out <= Debug9;
					  12'h0A4: reg_data_out <= Debug10;
					  12'h0A8: reg_data_out <= Debug11TF1;
					  12'h0AC: reg_data_out <= Debug12TF2;
					  12'h0B0: reg_data_out <= Debug13TF3;
					  12'h0B4: reg_data_out <= Debug14TF4;
					  12'h0B8: reg_data_out <= Debug15TX1;
					  12'h0BC: reg_data_out <= Debug16TX2;
					  12'h0C0: reg_data_out <= Debug17TX3;
					  12'h0C4: reg_data_out <= Debug18DDR1;
					  12'h0C8: reg_data_out <= Debug19DDR2;
					  12'h0CC: reg_data_out <= Debug20RX1;
					  12'h0D0: reg_data_out <= Debug21RX2;
					  12'h0D4: reg_data_out <= {27'h0000_000,Debug22RX3};
					  12'h0D8: reg_data_out <= Debug23RX4;
					  12'h0DC: reg_data_out <= Debug24RX5;
					  12'h0E0: reg_data_out <= Debug25RX6;
					  12'h0E4: reg_data_out <= Debug26RX7;
					  12'h0E8: reg_data_out <= Debug27RX8;
					  12'h0EC: reg_data_out <= Debug28RX9;
					  12'h0F0: reg_data_out <= Debug29RX10;
					  12'h0F4: reg_data_out <= {22'h00_0000,Debug30RXEngine};
					  12'h0F8: reg_data_out <= {20'h00000,Debug31RXDataFIFOfullcnt};
					  12'h0FC: reg_data_out <= {20'h00000,Debug32RXXferFIFOfullcnt};
					  12'hF00: reg_data_out <= {8'h00,Debug33RXDataFIFOWRcnt};
					  12'hF04: reg_data_out <= {8'h00,Debug34RXDataFIFORDcnt};
					  12'hF08: reg_data_out <= {8'h00,Debug35RXXferFIFOWRcnt};
					  12'hF0C: reg_data_out <= {8'h00,Debug36RXXferFIFORDcnt};
					  12'hF10: reg_data_out <= Debug37completion_pending;
					  12'hF14: reg_data_out <= {27'h0000_000,Debug38tag_value};
					 
`ifdef WARP_RADIO_REGISTERS
					  /// Jiansong: WARP Radio Registers
					  12'h190: reg_data_out <= WARP_RFControl;
					  12'h194: reg_data_out <= WARP_LEDControl;
					  12'h19C: reg_data_out <= WARP_MaximControl;
					  12'h1A0: reg_data_out <= WARP_MaximGainSetting;
					  12'h1A4: reg_data_out <= WARP_MaximStatus;
					  12'h1A8: reg_data_out <= WARP_ADCControl;
					  12'h1AC: reg_data_out <= WARP_ADCStatus;
					  12'h1B0: reg_data_out <= WARP_DACControl;
					  12'h1B4: reg_data_out <= WARP_DACStatus;
					  12'h1B8: reg_data_out <= WARP_MaximSPIControl;
					  12'h1BC: reg_data_out <= WARP_DACSPIControl;
					  12'h1C0: reg_data_out <= WARP_MaximSPIDataIn;
					  12'h1C4: reg_data_out <= WARP_DACSPIDataIn;
					  12'h1C8: reg_data_out <= WARP_DACSPIDataOut;
					  12'h1CC: reg_data_out <= WARP_RSSIADCControl;
					  12'h1D0: reg_data_out <= WARP_RSSIADCData;
`endif				  
`ifdef SORA_RADIO_REGISTERS
					  12'h190: reg_data_out <= {30'h0000_0000,Sora_RadioControl};
					  12'h194: reg_data_out <= Sora_LEDControl;
					  12'h198: reg_data_out <= Sora_AntennaSelection;
					  12'h19C: reg_data_out <= Sora_SampleClock;
					  12'h1A0: reg_data_out <= Sora_CoarseFreq;
					  12'h1A4: reg_data_out <= Sora_FinegradeFreq;
					  12'h1A8: reg_data_out <= Sora_FreqCompensation;
					  12'h1AC: reg_data_out <= Sora_RadioLOLock;
					  12'h1B0: reg_data_out <= Sora_FilterBandwidth;
					  12'h1B4: reg_data_out <= Sora_TXVGA1;
					  12'h1B8: reg_data_out <= Sora_TXVGA2;
					  12'h1BC: reg_data_out <= Sora_TXPA1;
					  12'h1C0: reg_data_out <= Sora_TXPA2;
					  12'h1C4: reg_data_out <= Sora_RXLNA;
					  12'h1C8: reg_data_out <= Sora_RXPA;
					  12'h1CC: reg_data_out <= Sora_RXVGA1;
					  12'h1D0: reg_data_out <= Sora_RXVGA2;
`endif
`ifdef RADIO_CHANNEL_REGISTERS	
					  12'h1C0: reg_data_out <= Channel_RRegValueIn;
					  12'h1C4: reg_data_out <= {23'h00_0000, Channel_ReadDone, Channel_Cmd, Channel_Addr[6:0]};
					  12'h1C8: reg_data_out <= Channel_RRegValueOut;
`endif
					  default: reg_data_out <= 32'hF5F5F5F5;
					endcase
			  end
		end
	//////////////////////////////////////////////////////////////////////////////
	//END of Register File Code
	//////////////////////////////////////////////////////////////////////////////


	/// Jiansong: dma read related logic should be kept, and dma write related
	///           logic is no longer used
	//////////////////////////////////////////////////////////////////////////////
	//NOTE:All of the code beneath this comment block are statemachines for 
	//driving 4KB, 2KB, 1KB, 512B, 256B, and 128B sub-transfers to the 
	//Internal Control Block
	//////////////////////////////////////////////////////////////////////////////

	/// Jiansong: remove dma write related states and keep dma read related states
	//State machine 0 block; Drives the Internal CTRL Block outputs:
	//reg_data_in_o[31:0], reg_wr_addr_o[6:0], and reg_wren_o.

	//This machine (state 0) waits for a signal from state machines 2 or 4
	//(either start_sm0_wdma_flow or start_sm0_rdma_flow signals)
	//When one of these signals is asserted, a series of register writes to
	//the Internal Control Block occurs which effectively causes a small
	//sub-transfer to execute (4KB, 2KB, 1KB, 512B, 256B, 128B.  
	//The dma*_now signals are passed to the Internal Control Block (via
	//reg_data_in_o, reg_wr_addr_o and reg_wren_o) as the sub-transfer parameters.  
	//This state machine also asserts the
	//write_calc_next and read_calc_next signals which are inputs to state 
	//machines 1 and 3 and which cause those state machines to calculate the 
	//DMA parameters for the next sub-transfer.

	always@(posedge clk) begin 
		if(rst | (~transferstart))begin
			state_0 <= IDLE_SM0;
			reg_data_in_o_r[31:0] <= 32'h00000000;
			reg_wr_addr_o_r[6:0] <= 7'b0000000;
			reg_wren_o_r <= 1'b0;
			read_calc_next <= 1'b0;
		end else begin
			 case(state_0) 
			 IDLE_SM0: begin
				reg_data_in_o_r[31:0] <= 32'h00000000;
				reg_wr_addr_o_r[6:0] <= 7'b0000000;
				reg_wren_o_r <= 1'b0;
				read_calc_next <= 1'b0;
				if(start_sm0_rdma_flow & ~pause_read_requests)begin    /// Jiansong: enter READ_1 state
					state_0 <= READ_1;
				end
			 end
			 
			 //start of the series of writes to the dma read sub-transfer registers
			 //in the Internal Control Block
			 READ_1: begin //write the dmarad register
				 reg_data_in_o_r[31:0] <= dmarad_now[31:0];
				 reg_wr_addr_o_r[6:0] <= 7'b0010100;
				 reg_wren_o_r <= 1'b1;
				 read_calc_next <= 1'b1;
				 state_0 <= READ_2;
			 end
			 READ_2: begin //write the dmaras_l register
				 reg_data_in_o_r[31:0] <= dmaras_now[31:0];
				 reg_wr_addr_o_r[6:0] <= 7'b0001100;
				 reg_wren_o_r <= 1'b1;
				 read_calc_next <= 1'b0;
				 state_0 <= READ_3;
			 end
			 READ_3: begin //write the dmarad_u register
				 reg_data_in_o_r[31:0] <= dmaras_now[63:32];
				 reg_wr_addr_o_r[6:0] <= 7'b0010000;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= READ_4;
			 end
			 READ_4: begin //write the dmarxs register with the next
								//largest possible sub-transfer size
	////          if(dmarxs_now[20:12] != 0)
				 if(dmarxs_now[31:12] != 0)
					  reg_data_in_o_r[31:0] <= 32'h00001000; //4KB
				 else if (dmarxs_now[11]) 
					  reg_data_in_o_r[31:0] <= 32'h00000800; //2KB
				 else if (dmarxs_now[10])
					  reg_data_in_o_r[31:0] <= 32'h00000400; //1KB
				 else if (dmarxs_now[9])
					  reg_data_in_o_r[31:0] <= 32'h00000200; //512B
				 else if (dmarxs_now[8])
					  reg_data_in_o_r[31:0] <= 32'h00000100; //256B
				 else if (dmarxs_now[7])
					  reg_data_in_o_r[31:0] <= 32'h00000080; //128B
				 else 
					  reg_data_in_o_r[31:0] <= 32'h00000080;
				 reg_wr_addr_o_r[6:0] <= 7'b0011100;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= READ_5;
			 end
			 READ_5: begin //write a 1 to the done bit (dmacst[3] in Internal
								//Control Block) in order to clear the start
								//bit
				 reg_data_in_o_r[31:0] <= 32'h00000008;
				 reg_wr_addr_o_r[6:0] <= 7'b0101000;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= READ_6;
			 end
			 READ_6: begin //write a 1 to the start bit (dmacst[2] in Internal
								//Control Block) in order to execute the sub-transfer
				 reg_data_in_o_r[31:0] <= 32'h00000004;
				 reg_wr_addr_o_r[6:0] <= 7'b0101000;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= IDLE_SM0;
			 end
			 default:begin
				 state_0 <= IDLE_SM0;
				 reg_data_in_o_r[31:0] <= 32'h00000000;
				 reg_wr_addr_o_r[6:0] <= 7'b0000000;
				 reg_wren_o_r <= 1'b0;
				 read_calc_next <= 1'b0;
			 end
		  endcase
		 end
	 end

	//pipeline register outputs for 250 MHz timing
	always@(posedge clk)begin
		reg_data_in_o[31:0] <= reg_data_in_o_r[31:0];
		reg_wr_addr_o[6:0] <= reg_wr_addr_o_r[6:0];
		reg_wren_o <= reg_wren_o_r;
	end

	//read_last is used by rx_engine so that it can signal
	//the correct *_done signal for the performance counters
	//Normally, rx_engine will use the *_early signal to
	//fire off a continuation transfer; on the last one
	//however, we would like the done signal to be accurate
	//for the performance counters

	////assign read_last = (dmarxs_now == 0) ? 1'b1: 1'b0;
	/// Jiansong: protection. otherwise, if transfer size has size smaller than 128B, it will never stop
	assign read_last = (dmarxs_now[31:7] == 0) ? 1'b1: 1'b0;

	//Calculate the next address, xfer size for writes   /// Jiansong: for reads
	//and transfer to dma*_now registers
	//Uses some multi-cycle paths:
	//      state_3  -> dma*_next
	//and   dma*_now -> dma*_next 
	//are both 2x multi-cycle paths
	//The signal "stay_2x_3" ensures that the state variable
	//state_3 is static for at least two clock
	//cycles when in the READ_CALC_*B states - the
	//dma*_now -> dma*_next paths must also be static during
	//these states
	always@(posedge clk)begin   
		if(rst | (~transferstart))begin
			state_3 <= IDLE_SM3;
			stay_2x_3 <= 1'b0;
			update_dma_rnow <= 1'b0;
			dmarxs_next[31:0] <= 13'b0000000000000;
			dmaras_next[63:0] <= 64'h0000000000000000; 
			dmarad_next[31:0] <= 32'h00000000;
		end else begin
			case(state_3)
			IDLE_SM3:begin
				update_dma_rnow <= 1'b0;
				stay_2x_3 <= 1'b0;
				if(new_des_one_r)begin
					dmarad_next <= DestAddr_r;
					dmarxs_next <= FrameSize_r;
					dmaras_next <= SourceAddr_r;
				end
				//if state machine 0 asserts read_calc_next and there
				//is still sub-transfers to be completed then go 
				//ahead and precalculate the dma*_next parameters for the
				//next sub-transfer
	////         if(read_calc_next && (dmarxs_now[20:7] != 0))
				if(read_calc_next && (dmarxs_now[31:7] != 0))    /// Jiansong: 1M size limitation is relaxed
					state_3 <= CALC_NEXT_READ;
				else
					state_3 <= IDLE_SM3;
			end
			//This state is to figure out which will be the next
			//sub-transfer size based on sampling the dmarxs_now
			//signals - priority encoded for the largest possible transfer
			//to occur first.
			CALC_NEXT_READ:begin
				stay_2x_3 <= 1'b0; 
				update_dma_rnow <= 1'b0;         
	////         if(dmarxs_now[20:12] != 0)
				if(dmarxs_now[31:12] != 0)       /// Jianosng: 1M size limitation is relaxed to 4G
					state_3 <= READ_CALC_4KB;
				else if (dmarxs_now[11])
					state_3 <= READ_CALC_2KB;
				else if (dmarxs_now[10])
					state_3 <= READ_CALC_1KB;
				else if (dmarxs_now[9])
					state_3 <= READ_CALC_512B;
				else if (dmarxs_now[8])
					state_3 <= READ_CALC_256B;
				else if (dmarxs_now[7])
					state_3 <= READ_CALC_128B;
				else
					state_3 <= READ_CALC_128B;
			 end
			 //The READ_CALC_*B states are for updating the dma*_next registers
			 //with the correct terms
			 READ_CALC_4KB:begin
				//subtract 4KB from dmarxs and add 4KB to dmarad_next and dmaras_next
	////         dmarxs_next[20:12] <= dmarxs_now[20:12] - 1'b1;
				dmarxs_next[31:12] <= dmarxs_now[31:12] - 1'b1;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000001000; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00001000;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_4KB;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_2KB:begin
				//subtract 2KB from dmarxs and add 2KB to dmarad_next and dmaras_next
				dmarxs_next[11] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000800; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000800;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_2KB;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_1KB:begin
				//subtract 1KB from dmarxs and add 1KB to dmarad_next and dmaras_next
				dmarxs_next[10] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000400; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000400;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_1KB;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_512B:begin
				//subtract 512B from dmarxs and add 512B to dmarad_next and dmaras_next
				dmarxs_next[9] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000200; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000200;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_512B;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_256B:begin
				//subtract 256B from dmarxs and add 256B to dmarad_next and dmaras_next
				dmarxs_next[8] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000100; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000100;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_256B;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_128B:begin
				//subtract 128B from dmarxs and add 128B to dmarad_next and dmaras_next
				dmarxs_next[7] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000080; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000080;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_128B;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 WAIT_READ_CALC:begin
				stay_2x_3 <= 1'b0;
				update_dma_rnow <= 1'b0;      
				state_3 <= IDLE_SM3;
			 end
			 default:begin
				state_3 <= IDLE_SM3;
				stay_2x_3 <= 1'b0;
				update_dma_rnow <= 1'b0;
				dmarxs_next[31:0] <= 13'b0000000000000;
				dmaras_next[63:0] <= 64'h0000000000000000; 
				dmarad_next[31:0] <= 32'h00000000;
			 end
		  endcase
	  end  
	end 


	/// Jiansong:generate request for tx desc write back
	always@(posedge clk)begin
		if(rst | (~transferstart)) begin
			TX_desc_write_back_req <= 1'b0;
		end else if (set_rd_done_bit) begin
						TX_desc_write_back_req <= 1'b1;
		end else if (TX_desc_write_back_ack) begin
						TX_desc_write_back_req <= 1'b0;
		end
	end

	/// Jiansong: pipeline registers for timing
	always@(posedge clk)begin
		if(rst | (~transferstart))begin
			new_des_one_r <= 1'b0;
		end else begin
			new_des_one_r <= new_des_one;
		end
	end
	always@(posedge clk)begin
		if(rst | ((~transferstart)))begin
			SourceAddr_r   <= 64'h0000_0000_0000_0000;
			DestAddr_r     <= 32'h0000_0000;
			FrameSize_r    <= 24'h00_00_00;
			FrameControl_r <= 8'h00;
			DescAddr_r     <= 64'h0000_0000_0000_0000;
		end else if(new_des_one)begin
							SourceAddr_r   <= {SourceAddr_H,SourceAddr_L};
							DestAddr_r     <= DestAddr;
							FrameSize_r    <= FrameSize;
							FrameControl_r <= FrameControl;
							DescAddr_r     <= {TransferSrcAddr_H,TransferSrcAddr_L};
		end else if (set_rd_done_bit) begin
						SourceAddr_r   <= SourceAddr_r;
						DestAddr_r     <= DestAddr_r;
						FrameSize_r    <= FrameSize_r;
						FrameControl_r <= {FrameControl_r[7:1],1'b0}; // clear own bit
						DescAddr_r     <= DescAddr_r;
		end else begin
						SourceAddr_r   <= SourceAddr_r;
						DestAddr_r     <= DestAddr_r;
						FrameSize_r    <= FrameSize_r;
						FrameControl_r <= FrameControl_r;
						DescAddr_r     <= DescAddr_r;
		end
	end

	//register dmarad,dmarxs, and dmaras when rd_dma_start_one is high
	//rd_dma_start_one is only asserted for one clock cycle
	always@(posedge clk)begin
		if(rst | (~transferstart))begin
			dmaras_now <= 64'h0000_0000_0000_0000;
			dmarxs_now <= 32'h0000_0000;
			dmarad_now <= 32'h0000_0000;
		end else if(new_des_one_r)begin    /// Jiansong: dma read (or data transfer) is started 
													///           after a new descriptor is received
			dmarad_now <= DestAddr_r;
			dmarxs_now <= FrameSize_r;
			dmaras_now <= SourceAddr_r; 		
		end else if(update_dma_rnow)begin
			dmarad_now <=  dmarad_next;
			dmarxs_now <=  dmarxs_next;
			dmaras_now <=  dmaras_next;   
		end
	end   

	//state machine for start_sm0_rdma_flow signal
	//This state machine controls state machine 0 by
	//driving the start_sm0_rdma_flow signal.  It monitors
	//the rd_dma_done signal from the rx_engine 
	//and starts a new dma subtransfer whenever rd_dma_done is signalled
	always@(posedge clk)begin
		if(rst | (~transferstart))begin
			  start_sm0_rdma_flow <= 1'b0;
			  state_4 <= IDLE_SM4;
			  set_rd_done_bit <= 1'b0;
		end else begin  
		  case(state_4)
			  IDLE_SM4:begin
			  start_sm0_rdma_flow <= 1'b0;
			  set_rd_done_bit <= 1'b0;   
			  //wait for the start signal from the host
			  if(new_des_one_r)      /// Jiansong: dma read (or data transfer) is started 
										  ///           after a new descriptor is received
					  state_4 <= START_RD;
			  end
			  START_RD:begin
				  //start the state_0 state machine by
				  //asserting start_sm0_rdma_flow signal        
				  start_sm0_rdma_flow <= 1'b1;
				  //when state_0 finally starts go to the 
				  //WAIT_FOR_RDDONE state
				  if(state_0 == READ_1)
					  state_4 <= WAIT_FOR_RDDONE;
				  else
					  state_4 <= START_RD;
			  end
			  WAIT_FOR_RDDONE:begin
				  start_sm0_rdma_flow <= 1'b0;
				  //If the rx_engine signals rd_dma_done_i and we
				  //have completed the last subtransfer (i.e. dmarxs_now == 0) then 
				  //set the dmacst[3] bit via set_rd_done_bit and go to the IDLE state;
				  //Otherwise start the next subtransfer by going to the
				  //START_RD state
	////           if(rd_dma_done_i && (dmarxs_now == 0))begin    ///Jiansong: will not stop if has piece smaller than 128B
				  if(rd_dma_done_i && (dmarxs_now[31:7] == 0))begin
					  state_4 <= IDLE_SM4;
					  set_rd_done_bit <= 1'b1;
				  end else if(rd_dma_done_i)begin
					  state_4 <= START_RD;
					  set_rd_done_bit <= 1'b0;
				  end else begin
					  state_4 <= WAIT_FOR_RDDONE;
					  set_rd_done_bit <= 1'b0;
				  end
			 end
			 default:begin
				 start_sm0_rdma_flow <= 1'b0;
				 state_4 <= IDLE_SM4;
				 set_rd_done_bit <= 1'b0;
			 end 
			endcase
		 end
	end

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

	assign TX_start = TXControl[0];
	rising_edge_detect TX_start_one_inst(
						 .clk(clk),
						 .rst(rst),
						 .in(TX_start),
						 .one_shot_out(TX_start_one)
						 );

	/// TX state machine 0 
	/// generate output TX_data_req, and TX_calc_next for TX_state_1
	always@(posedge clk)begin
		if(rst)begin
			TX_calc_next <= 1'b0;
			TX_data_req  <= 1'b0;
			TX_state_0   <= IDLE_TSM0;
		end else begin
			case(TX_state_0)
				IDLE_TSM0: begin
					TX_calc_next <= 1'b0;
					TX_data_req  <= 1'b0;   
					// next_TX_ready will remain high after the last TX request, but it doesn't matter
					if(TX_start_one | 
	//				   (next_TX_ready&&(TX_size_next[27:6] != 0))) begin
						(next_TX_ready&&(TX_size_next[27:7] != 0))) begin
						TX_state_0 <= TX_1;
					end else
							TX_state_0 <= IDLE_TSM0;
				end
				TX_1: begin // this state is used to set TX_xfer_size and TX_start_addr output 
					TX_calc_next <= 1'b0;
					TX_data_req  <= 1'b0; 
					TX_state_0   <= TX_2;
				end
				TX_2: begin // calc TX_size_next and TX_addr_next, send out TX request to DMA_DDR_IF
					TX_calc_next <= 1'b1;
					TX_data_req  <= 1'b1;
					TX_state_0   <= TX_3;
				end
				TX_3: begin
					TX_calc_next <= 1'b0;
					if(TX_data_ack)begin
						TX_data_req <= 1'b0;
						TX_state_0   <= IDLE_TSM0;				
					end else begin
						TX_data_req <= 1'b1;
						TX_state_0   <= TX_3;
					end
				end
				default:begin
					TX_calc_next <= 1'b0;
					TX_data_req  <= 1'b0;
					TX_state_0   <= IDLE_TSM0;
				end
			endcase
		end
	end

	always@(posedge clk) TX_start_addr[6] <= 1'b0;
	/// set output: TX_xfer_size and TX_start_addr[27:6]
	always@(posedge clk)begin
		if(rst)begin
			TX_xfer_size        <= 3'b000;
	//		TX_start_addr[27:6] <= 22'h00_0000;
			TX_start_addr[27:7] <= 21'h00_0000; 
		end else if (TX_state_0 == TX_1)begin
	//	   TX_start_addr[27:6] <= TX_addr_now[27:6];
			TX_start_addr[27:7] <= TX_addr_now[27:7];
			if(TX_size_now[27:12] != 0)                     // 4KB
				TX_xfer_size     <= 3'b110;
			else if(TX_size_now[11] != 0)                   // 2KB
				TX_xfer_size     <= 3'b101;
			else if(TX_size_now[10] != 0)                   // 1KB
				TX_xfer_size     <= 3'b100;
			else if(TX_size_now[9] != 0)                    // 512B
				TX_xfer_size     <= 3'b011;
			else if(TX_size_now[8] != 0)                    // 256B
				TX_xfer_size     <= 3'b010;
			else if(TX_size_now[7] != 0)                    // 128B
				TX_xfer_size     <= 3'b001;
	//		else if(TX_size_now[6] != 0)                    // 64B
	//		   TX_xfer_size     <= 3'b000;
			else                                            // default to 128B
	//		   TX_xfer_size     <= 3'b000;
				TX_xfer_size     <= 3'b001;
		end else begin
			TX_xfer_size        <= TX_xfer_size;
	//		TX_start_addr[27:6] <= TX_start_addr[27:6];
			TX_start_addr[27:7] <= TX_start_addr[27:7];
		end
	end

	/// TX state machine 1 
	/// calculate next size and address
	always@(posedge clk)begin
		if(rst)begin
			stay_2x_TX1        <= 1'b0;
			update_TX_now      <= 1'b0;
			TX_addr_next[31:0] <= 32'h0000_0000;
			TX_size_next[31:0] <= 32'h0000_0000;
			TX_state_1         <= IDLE_TSM1;
		end else begin
				 case(TX_state_1)
					 IDLE_TSM1:begin
						 stay_2x_TX1   <= 1'b0;
						 update_TX_now <= 1'b0;
						 if(TX_start_one)begin
							 TX_addr_next[31:0] <= TXAddr[31:0];
							 TX_size_next[31:0] <= TXSize[31:0];
						 end else begin
								  TX_addr_next[31:0] <= TX_addr_next[31:0];
								  TX_size_next[31:0] <= TX_size_next[31:0];
						 end
	//					 if(TX_calc_next && (TX_size_now[27:6] != 0))  // here is the end of a flow
						 if(TX_calc_next && (TX_size_now[27:7] != 0))
							 TX_state_1 <= CALC_NEXT_TX;
						 else
							 TX_state_1 <= IDLE_TSM1;
					 end
					 CALC_NEXT_TX:begin
						 stay_2x_TX1   <= 1'b0;
						 update_TX_now <= 1'b0;
						 TX_addr_next[31:0] <= TX_addr_next[31:0];
						 TX_size_next[31:0] <= TX_size_next[31:0];
						 if(TX_size_now[27:12] != 0)
							 TX_state_1 <= TX_CALC_4KB;
						 else if (TX_size_now[11] != 0)
							 TX_state_1 <= TX_CALC_2KB;
						 else if (TX_size_now[10] != 0)
							 TX_state_1 <= TX_CALC_1KB;
						 else if (TX_size_now[9] != 0)
							 TX_state_1 <= TX_CALC_512B;
						 else if (TX_size_now[8] != 0)
							 TX_state_1 <= TX_CALC_256B;
						 else if (TX_size_now[7] != 0)
							 TX_state_1 <= TX_CALC_128B;
	//					 else if (TX_size_now[6] != 0)
	//					    TX_state_1 <= TX_CALC_64B;
						 else
	//					    TX_state_1 <= TX_CALC_64B;
							 TX_state_1 <= TX_CALC_128B;
					 end
					 TX_CALC_4KB:begin
						 TX_addr_next[31:0]  <= TX_addr_now[31:0] + 32'h0000_1000;
						 TX_size_next[31:12] <= TX_size_now[31:12] - 1'b1;
						 if(stay_2x_TX1 == 1'b0)begin
							 stay_2x_TX1   <= 1'b1;
							 update_TX_now <= 1'b0;
							 TX_state_1    <= TX_CALC_4KB;
						 end else begin
							 stay_2x_TX1   <= 1'b0;
							 update_TX_now <= 1'b1;
							 TX_state_1    <= WAIT_TX_CALC;
						 end
					 end
					 TX_CALC_2KB:begin
						 TX_addr_next[31:0]  <= TX_addr_now[31:0] + 32'h0000_0800;
						 TX_size_next[11]    <= 1'b0;    
						 if(stay_2x_TX1 == 1'b0)begin
							 stay_2x_TX1   <= 1'b1;
							 update_TX_now <= 1'b0;
							 TX_state_1    <= TX_CALC_2KB;
						 end else begin
							 stay_2x_TX1   <= 1'b0;
							 update_TX_now <= 1'b1;
							 TX_state_1    <= WAIT_TX_CALC;
						 end
					 end
					 TX_CALC_1KB:begin
						 TX_addr_next[31:0]  <= TX_addr_now[31:0] + 32'h0000_0400;
						 TX_size_next[10]    <= 1'b0;
						 if(stay_2x_TX1 == 1'b0)begin
							 stay_2x_TX1   <= 1'b1;
							 update_TX_now <= 1'b0;
							 TX_state_1    <= TX_CALC_1KB;
						 end else begin
							 stay_2x_TX1   <= 1'b0;
							 update_TX_now <= 1'b1;
							 TX_state_1    <= WAIT_TX_CALC;
						 end
					 end
					 TX_CALC_512B:begin
						 TX_addr_next[31:0]  <= TX_addr_now[31:0] + 32'h0000_0200;
						 TX_size_next[9]     <= 1'b0;
						 if(stay_2x_TX1 == 1'b0)begin
							 stay_2x_TX1   <= 1'b1;
							 update_TX_now <= 1'b0;
							 TX_state_1    <= TX_CALC_512B;
						 end else begin
							 stay_2x_TX1   <= 1'b0;
							 update_TX_now <= 1'b1;
							 TX_state_1    <= WAIT_TX_CALC;
						 end
					 end
					 TX_CALC_256B:begin
						 TX_addr_next[31:0]  <= TX_addr_now[31:0] + 32'h0000_0100;
						 TX_size_next[8]     <= 1'b0;
						 if(stay_2x_TX1 == 1'b0)begin
							 stay_2x_TX1   <= 1'b1;
							 update_TX_now <= 1'b0;
							 TX_state_1    <= TX_CALC_256B;
						 end else begin
							 stay_2x_TX1   <= 1'b0;
							 update_TX_now <= 1'b1;
							 TX_state_1    <= WAIT_TX_CALC;
						 end
					 end
					 TX_CALC_128B:begin
						 TX_addr_next[31:0]  <= TX_addr_now[31:0] + 32'h0000_0080;
						 TX_size_next[7]     <= 1'b0;
						 if(stay_2x_TX1 == 1'b0)begin
							 stay_2x_TX1   <= 1'b1;
							 update_TX_now <= 1'b0;
							 TX_state_1    <= TX_CALC_128B;
						 end else begin
							 stay_2x_TX1   <= 1'b0;
							 update_TX_now <= 1'b1;
							 TX_state_1    <= WAIT_TX_CALC;
						 end
					 end
	//				 TX_CALC_64B:begin
	//				    TX_addr_next[31:0]  <= TX_addr_now[31:0] + 32'h0000_0040;
	//					 TX_size_next[6]     <= 1'b0;
	//					 if(stay_2x_TX1 == 1'b0)begin
	//					    stay_2x_TX1   <= 1'b1;
	//						 update_TX_now <= 1'b0;
	//					    TX_state_1    <= TX_CALC_64B;
	//					 end else begin
	//					    stay_2x_TX1   <= 1'b0;
	//						 update_TX_now <= 1'b1;
	//					    TX_state_1    <= WAIT_TX_CALC;
	//					 end
	//				 end
					 WAIT_TX_CALC:begin
						 stay_2x_TX1   <= 1'b0;
						 update_TX_now <= 1'b0;
						 TX_state_1    <= IDLE_TSM1;
					 end
					 default:begin
						 stay_2x_TX1        <= 1'b0;
						 update_TX_now      <= 1'b0;
						 TX_addr_next[31:0] <= 32'h0000_0000;
						 TX_size_next[31:0] <= 32'h0000_0000;
						 TX_state_1         <= IDLE_TSM1;
					 end
				 endcase
		end
	end

	/// update TX_addr_now and TX_size_now
	always@(posedge clk)begin
		if(rst)begin
			TX_addr_now[31:0] <= 32'h0000_0000;
			TX_size_now[31:0] <= 32'h0000_0000;
		end else if (TX_start_one)begin
			TX_addr_now[31:0] <= TXAddr[31:0];
			TX_size_now[31:0] <= TXSize[31:0];
		end else if (update_TX_now)begin
			TX_addr_now[31:0] <= TX_addr_next[31:0];
			TX_size_now[31:0] <= TX_size_next[31:0];
		end else begin
			TX_addr_now[31:0] <= TX_addr_now[31:0];
			TX_size_now[31:0] <= TX_size_now[31:0];
		end
	end

	/// generate next_TX_ready
	always@(posedge clk)begin
		if(rst)
			next_TX_ready <= 1'b0;
		else if (update_TX_now)
			next_TX_ready <= 1'b1;
		else if (TX_calc_next)
			next_TX_ready <= 1'b0;
		else
			next_TX_ready <= next_TX_ready;
	end

endmodule
