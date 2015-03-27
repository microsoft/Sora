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

module register_table(
    input clk,
	 // host reset generation logic which should only controlled by hardware reset 
	 // is implemented in this module, so we should differentiate 
	 // hardware reset and host reset in this module
//    input hard_rst, 
	 input rst,
	 
	 // hot reset to whole system
	 output reg hostreset,
	 
	 output reg [4:0]	DDR2MaxBurstSize,		// b0: 256B; b1: 512B; b2: 1KB; b3: 2KB; b4: 4KB
	 
	 /// Jiansong: performance value inputs
	input [23:0] round_trip_latency_i, /// 24 bits performance value
	input [23:0] transfer_duration_i,  /// 24 bits performance value
	
    /// Jiansong: interface to radio module
//	 input      	Radio_TX_done,
//	 output     	Radio_TX_start,	
	 output			TX_Ongoing_o,

	 output reg		TX_Start_one_o,
	 output [31:0]	TX_Addr,
	 output [31:0]	TX_Size,
	 output reg		TX_Start_2nd_one_o,
	 output [31:0]	TX_Addr_2nd,
	 output [31:0]	TX_Size_2nd,
`ifdef MIMO_4X4
	 output reg		TX_Start_3rd_one_o,
	 output [31:0]	TX_Addr_3rd,
	 output [31:0]	TX_Size_3rd,
	 output reg		TX_Start_4th_one_o,
	 output [31:0]	TX_Addr_4th,
	 output [31:0]	TX_Size_4th,
`endif //MIMO_4X4
    //Interface to host system
    //inputs are driven from RX Engine
    input [31:0] reg_data_in, 
    input [11:0] reg_wr_addr,
    input reg_wren,
	 
    /// Jiansong: control signal for transfer recovering
    output			transferstart_o,
    /// Jiansong: signal output to performance counter
    output			transferstart_one,
	 input			set_transfer_done_bit,
	 output [63:0]	TransferSrcAddr,
	 
	 /// Jiansong: RX path
	 output         RXEnable_o,
	 output [63:0]  RXBufAddr_o,
	 output [31:0]  RXBufSize_o,
	 output         RXEnable_2nd_o,
	 output [63:0]  RXBufAddr_2nd_o,
	 output [31:0]  RXBufSize_2nd_o,
`ifdef MIMO_4X4
	 output         RXEnable_3rd_o,
	 output [63:0]  RXBufAddr_3rd_o,
	 output [31:0]  RXBufSize_3rd_o,
	 output         RXEnable_4th_o,
	 output [63:0]  RXBufAddr_4th_o,
	 output [31:0]  RXBufSize_4th_o,
`endif //MIMO_4X4
	 
	 /// Jiansong: interface to/from tx engine
    input [11:0] reg_rd_addr,
    output reg [31:0] reg_data_out,


    //Performance counts from performance counter module
//    input [31:0] dma_wr_count,
//    input [31:0] dma_rd_count,
    
	 
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
`ifdef RADIO_CHANNEL_REGISTERS

// defined as register is a redundant, but better for timing
	 output reg [31:0] 	Radio_Cmd_Data,	
	 output reg [6:0]  	Radio_Cmd_Addr,
	 output reg				Radio_Cmd_RdWr,
	 output reg				Radio_Cmd_wren,
	 input [31:0]			RadioRegRead_Value_in,
	 input [7:0]			RadioRegRead_Addr_in,
	 input					RadioReg_ReadDone_in,	 
/// registers for 2nd to 4th paths/radios					  
	 output reg [31:0] 	Radio_2nd_Cmd_Data,	
	 output reg [6:0]  	Radio_2nd_Cmd_Addr,
	 output reg				Radio_2nd_Cmd_RdWr,
	 output reg				Radio_2nd_Cmd_wren,
	 input [31:0]			RadioRegRead_2nd_Value_in,
	 input [7:0]			RadioRegRead_2nd_Addr_in,
	 input					RadioReg_2nd_ReadDone_in,	 
`ifdef MIMO_4X4	 
	 output reg [31:0] 	Radio_3rd_Cmd_Data,	
	 output reg [6:0]  	Radio_3rd_Cmd_Addr,
	 output reg				Radio_3rd_Cmd_RdWr,
	 output reg				Radio_3rd_Cmd_wren,
	 input [31:0]			RadioRegRead_3rd_Value_in,
	 input [7:0]			RadioRegRead_3rd_Addr_in,
	 input					RadioReg_3rd_ReadDone_in,	 			  
	 output reg [31:0] 	Radio_4th_Cmd_Data,	
	 output reg [6:0]  	Radio_4th_Cmd_Addr,
	 output reg				Radio_4th_Cmd_RdWr,
	 output reg				Radio_4th_Cmd_wren,
	 input [31:0]			RadioRegRead_4th_Value_in,
	 input [7:0]			RadioRegRead_4th_Addr_in,
	 input					RadioReg_4th_ReadDone_in,	 
`endif //MIMO_4X4
	 
`endif //RADIO_CHANNEL_REGISTERS
	 // debug signals
	 input [31:0]	DebugRX1Overflowcount_in,
	 input [31:0]	DebugRX2Overflowcount_in,
	 	 
	 input [31:0]	DebugDDREgressFIFOCnt_in,
	 input [31:0]	DebugDDRFIFOFullCnt_in,
	 input [31:0]	DebugDDRSignals_in,
	 input [8:0]	DebugDDRSMs_in,
	 
	 input [15:0] PCIeLinkStatus_in,
    input [15:0] PCIeLinkControl_in
//	 input [31:0] Debug18DDR1_in,
//	 input [31:0] Debug19DDR2_in,
//	 input [31:0] Debug20RX1_in,
//	 input [31:0] Debug21RX2_in,
////	 input [4:0]  Debug22RX3_in,
//	 input [31:0] Debug23RX4_in,
//	 input [31:0] Debug24RX5_in,
//	 input [31:0] Debug25RX6_in,
//	 input [31:0] Debug26RX7_in,
//    input [31:0] Debug27RX8_in,
//	 input [31:0] Debug28RX9_in,
//    input [31:0] Debug29RX10_in,
//	 input [9:0]  Debug30RXEngine_in,
//    input [11:0] Debug31RXDataFIFOfullcnt_in,
//    input [11:0] Debug32RXXferFIFOfullcnt_in,
//    input [23:0] Debug33RXDataFIFOWRcnt_in,
//    input [23:0] Debug34RXDataFIFORDcnt_in,
//    input [23:0] Debug35RXXferFIFOWRcnt_in,
//    input [23:0] Debug36RXXferFIFORDcnt_in,
//	 input [31:0] Debug37completion_pending_in,
//	 input [4:0]  Debug38tag_value_in,
//	 input [7:0]  FIFOErrors_in,
//	 input [4:0]  locked_debug
    );

	 
	////////////////////////////////////////////////////////////
	////         Start of Sora Registers               /////////
	////////////////////////////////////////////////////////////
	/// Jiansong: System Registers
	reg		SoftReset; 
	// reg [31:0] PCIeTXBurstSize;     // not used
	// reg [31:0] PCIeRXBlockSize;     // not used
	reg [31:0]	HWStatus;            // read only
	reg [15:0]	PCIeLinkStatus;      // read only
	reg [15:0]	PCIeLinkControl;     // read only
	reg [31:0]	FirmwareVersion;	  // read only
//	reg [4:0]	DDR2MaxBurstSize;		// b0: 256B; b1: 512B; b2: 1KB; b3: 2KB; b4: 4KB
	/// Jiansong: Debug Registers, all read only
////	reg [31:0] Debug1;              // b0: TXFIFOOverflow, b1: RXFIFOOverflow, b3: illegal tf
////	reg [31:0] Debug2;              // TX FIFO read counter
////	reg [31:0] Debug3;              // TX FIFO write counter
////	reg [31:0] Debug4;              // TX Done counter
////	reg [31:0] Debug5SourceAddr_L;  // value in TX descriptor
////	reg [31:0] Debug6SourceAddr_H;  // value in TX descriptor
////	reg [31:0] Debug7DestAddr;      // value in TX descriptor
////	reg [31:0] Debug8FrameSize;     // value in TX descriptor
////	reg [31:0] Debug9;              // Egress FIFO overflow counter
////	reg [31:0] Debug10;             // RX FIFO overflow counter
////	reg [31:0] Debug11TF1;          // Transfer state debug register 1
////	reg [31:0] Debug12TF2;          // Transfer state debug register 2
////	reg [31:0] Debug13TF3;          // Transfer state debug register 3
////	reg [31:0] Debug14TF4;          // Transfer state debug register 4
////	reg [31:0] Debug15TX1;          // TX state debug register 1
////	reg [31:0] Debug16TX2;          // TX state debug register 2
////	reg [31:0] Debug17TX3;          // TX state debug register 3
////	reg [31:0] Debug18DDR1;         // DDR state debug register 1
////	reg [31:0] Debug19DDR2;         // DDR state debug register 2
//	reg [31:0] Debug20RX1;          // RX state debug register 1
////	reg [31:0] Debug21RX2;          // RX state debug register 2
////	reg [4:0]  Debug22RX3;          // RX state debug register 3
////	reg [31:0] Debug23RX4;          // RX state debug register 4
//	reg [31:0] Debug24RX5;          // RX state debug register 5
////	reg [31:0] Debug25RX6;          // RX state debug register 6
//	reg [31:0] Debug26RX7;          // RX state debug register 7
//	reg [31:0] Debug27RX8;          // RX state debug register 8
//	reg [31:0] Debug28RX9;          // RX state debug register 9
//	reg [31:0] Debug29RX10;         // RX state debug register 10
////	reg [9:0]  Debug30RXEngine;     // debug register in RX engine
////	reg [11:0] Debug31RXDataFIFOfullcnt;  // debug register
////	reg [11:0] Debug32RXXferFIFOfullcnt;  // to detect buffer overflow
////	reg [23:0] Debug33RXDataFIFOWRcnt;    // Data fifo WR cnt
////	reg [23:0] Debug34RXDataFIFORDcnt;    // Data fifo RD cnt
////	reg [23:0] Debug35RXXferFIFOWRcnt;    // Xfer fifo WR cnt
////	reg [23:0] Debug36RXXferFIFORDcnt;    // Xfer fifo RD cnt
////	reg [31:0] Debug37completion_pending;
////	reg [4:0]  Debug38tag_value;

	reg [31:0]	DebugRX1Overflowcount;
	reg [31:0]	DebugRX2Overflowcount;
	
	reg [31:0]	DebugDDREgressFIFOCnt;
	reg [31:0]	DebugDDRFIFOFullCnt;
	reg [31:0]	DebugDDRSignals;
	reg [8:0]	DebugDDRSMs;

	/// Jiansong: DMA Registers
	reg [1:0]  TransferControl;
	reg [31:0] TransferSrcAddr_L;
	reg [31:0] TransferSrcAddr_H;

	reg			SyncControl;		/// liuchang

	reg			RXControl;
	reg [31:0]	RXBufAddr_L;
	reg [31:0]	RXBufAddr_H;
	reg [31:0]	RXBufSize;      /// RX buffer size in bytes
//	reg [2:0]	TXControl;		// bit0: TXStart; bit1: TXDone; bit2: TXOngoing
	reg			TXControl_Start;
	reg			TXControl_Done;
	reg			TXControl_Ongoing;		
	reg [3:0]	TXMask;
	reg [31:0]	TXAddr;
	reg [31:0]	TXSize;
	reg [31:0]	TXTime;
	reg [31:0]	RadioDelayCompen;
	wire [31:0]	TXTime_start2done;			/// liuchang
/// registers for 2nd to 4th paths/radios
	reg			RXControl_2nd;
	reg [31:0]	RXBufAddr_2nd_L;
	reg [31:0]	RXBufAddr_2nd_H;
	reg [31:0]	RXBufSize_2nd;    /// RX buffer size in bytes
//	reg [2:0]	TXControl_2nd;		// bit0: TXStart; bit1: TXDone; bit2: TXOngoing
	reg			TXControl_Start_2nd;
	reg			TXControl_Done_2nd;
	reg			TXControl_Ongoing_2nd;		
	reg [3:0]	TXMask_2nd;
	reg [31:0]	TXAddr_2nd;
	reg [31:0]	TXSize_2nd;
	reg [31:0]	TXTime_2nd;
	reg [31:0]	RadioDelayCompen_2nd;
	wire [31:0]	TXTime_start2done_2nd;		/// liuchang
`ifdef MIMO_4X4
	reg			RXControl_3rd;
	reg [31:0]	RXBufAddr_3rd_L;
	reg [31:0]	RXBufAddr_3rd_H;
	reg [31:0]	RXBufSize_3rd;    /// RX buffer size in bytes
//	reg [2:0]	TXControl_3rd;		// bit0: TXStart; bit1: TXDone; bit2: TXOngoing
	reg			TXControl_Start_3rd;
	reg			TXControl_Done_3rd;
	reg			TXControl_Ongoing_3rd;
	reg [3:0]	TXMask_3rd;
	reg [31:0]	TXAddr_3rd;
	reg [31:0]	TXSize_3rd;
	reg [31:0]	TXTime_3rd;
	reg [31:0]	RadioDelayCompen_3rd;
	wire [31:0]	TXTime_start2done_3rd;		/// liuchang
	reg			RXControl_4th;
	reg [31:0]	RXBufAddr_4th_L;
	reg [31:0]	RXBufAddr_4th_H;
	reg [31:0]	RXBufSize_4th;    /// RX buffer size in bytes
//	reg [2:0]	TXControl_4th;		// bit0: TXStart; bit1: TXDone; bit2: TXOngoing
	reg			TXControl_Start_4th;
	reg			TXControl_Done_4th;
	reg			TXControl_Ongoing_4th;
	reg [3:0]	TXMask_4th;
	reg [31:0]	TXAddr_4th;
	reg [31:0]	TXSize_4th;
	reg [31:0]	TXTime_4th;
	reg [31:0]	RadioDelayCompen_4th;
	wire [31:0]	TXTime_start2done_4th;		/// liuchang
`endif //MIMO_4X4

	// reg [31:0] TransferMask;
	reg [23:0] round_trip_latency;  /// performance value
	reg [23:0] transfer_duration;   /// performance value
	/// Jiansong: Common Radio Registers, pending
//	reg		  RadioStatus;      /// Read only
//	reg [31:0] RadioID;          /// Read only
										// value '0' is reserved for bypassing mode
`ifdef RADIO_CHANNEL_REGISTERS
	reg [31:0] RadioReg_WriteValue;
	reg 		  RadioReg_ReadDone;
	reg 		  RadioReg_RW;
	reg [6:0]  RadioReg_Addr;
	reg [31:0] RadioReg_ValueReturn;
	
	wire			set_RadioReg_ReadDone_bit;
	reg			set_RadioReg_ReadDone_bit_r;
	wire			set_RadioReg_ReadDone_bit_two;	// two cycle signal to guarantee it will not be missed
	reg 			RadioReg_ReadDone_in_reg;			// register for cross time domain signal

/// registers for 2nd to 4th paths/radios		
	reg [31:0] RadioReg_2nd_WriteValue;
	reg 		  RadioReg_2nd_ReadDone;
	reg 		  RadioReg_2nd_RW;
	reg [6:0]  RadioReg_2nd_Addr;
	reg [31:0] RadioReg_2nd_ValueReturn;
	
	wire			set_RadioReg_2nd_ReadDone_bit;
	reg			set_RadioReg_2nd_ReadDone_bit_r;
	wire			set_RadioReg_2nd_ReadDone_bit_two;
	reg 			RadioReg_2nd_ReadDone_in_reg;			// register for cross time domain signal

`ifdef MIMO_4X4		
	reg [31:0] RadioReg_3rd_WriteValue;
	reg 		  RadioReg_3rd_ReadDone;
	reg 		  RadioReg_3rd_RW;
	reg [6:0]  RadioReg_3rd_Addr;
	reg [31:0] RadioReg_3rd_ValueReturn;
	
	wire			set_RadioReg_3rd_ReadDone_bit;
	reg			set_RadioReg_3rd_ReadDone_bit_r;
	wire			set_RadioReg_3rd_ReadDone_bit_two;
	reg 			RadioReg_3rd_ReadDone_in_reg;			// register for cross time domain signal
		
	reg [31:0] RadioReg_4th_WriteValue;
	reg 		  RadioReg_4th_ReadDone;
	reg 		  RadioReg_4th_RW;
	reg [6:0]  RadioReg_4th_Addr;
	reg [31:0] RadioReg_4th_ValueReturn;
	
	wire			set_RadioReg_4th_ReadDone_bit;
	reg			set_RadioReg_4th_ReadDone_bit_r;
	wire			set_RadioReg_4th_ReadDone_bit_two;
	reg 			RadioReg_4th_ReadDone_in_reg;			// register for cross time domain signal
`endif //MIMO_4X4

`endif //RADIO_CHANNEL_REGISTERS
	////////////////////////////////////////////////////////////
	////           End of Sora Registers               /////////
	////////////////////////////////////////////////////////////

	assign TX_Ongoing_o = TXControl_Ongoing;

	/// counter to set duration for host reset signal
	reg [7:0] hostresetcnt;

	reg [3:0]	TXMask_radio;
	reg [3:0]	TXMask_2nd_radio;
	reg [3:0]	TXMask_3rd_radio;
	reg [3:0]	TXMask_4th_radio;

	wire	TX_Start_one;
	wire	TX_Start_2nd_one;
	wire	TX_Start_3rd_one;
	wire	TX_Start_4th_one;
//	wire	TX_Clear_one;
//	wire	TX_Clear_2nd_one;
//	wire	TX_Clear_3rd_one;
//	wire	TX_Clear_4th_one;
	reg	TX_Start_one_r;
	reg	TX_Start_2nd_one_r;
	reg	TX_Start_3rd_one_r;
	reg	TX_Start_4th_one_r;

//	wire set_TX_done_bit;
	reg	set_TX_done_bit;
	reg	set_TX_done_bit_2nd;
	reg	set_TX_done_bit_3rd;
	reg	set_TX_done_bit_4th;

	wire RX_FIFO_full_one;

	/// Jiansong: this signal controls rd_TX_des_start
	rising_edge_detect rd_dma_start_one_inst(
						 .clk(clk),.rst(rst),.in(transferstart),.one_shot_out(transferstart_one));

	assign transferstart = TransferControl[0];
	assign transferstart_o = TransferControl[0];
	assign TransferSrcAddr = {TransferSrcAddr_H,TransferSrcAddr_L};

	/// Jiansong: control signal to radio module
//	assign Radio_TX_start = TXControl[0];
//	assign TX_start = TXControl[0];
	assign TX_Addr = TXAddr[31:0];
	assign TX_Size = TXSize[31:0];
	assign TX_Addr_2nd = TXAddr_2nd[31:0];
	assign TX_Size_2nd = TXSize_2nd[31:0];
	assign TX_Addr_3rd = TXAddr_3rd[31:0];
	assign TX_Size_3rd = TXSize_3rd[31:0];
	assign TX_Addr_4th = TXAddr_4th[31:0];
	assign TX_Size_4th = TXSize_4th[31:0];

	/// Jiansong: control signal to RX path
	assign RXEnable_o  = RXControl;
	assign RXBufAddr_o = {RXBufAddr_H,RXBufAddr_L};
	assign RXBufSize_o = RXBufSize;
	assign RXEnable_2nd_o  = RXControl_2nd;
	assign RXBufAddr_2nd_o = {RXBufAddr_2nd_H,RXBufAddr_2nd_L};
	assign RXBufSize_2nd_o = RXBufSize_2nd;
`ifdef MIMO_4X4
	assign RXEnable_3rd_o  = RXControl_3rd;
	assign RXBufAddr_3rd_o = {RXBufAddr_3rd_H,RXBufAddr_3rd_L};
	assign RXBufSize_3rd_o = RXBufSize_3rd;
	assign RXEnable_4th_o  = RXControl_4th;
	assign RXBufAddr_4th_o = {RXBufAddr_4th_H,RXBufAddr_4th_L};
	assign RXBufSize_4th_o = RXBufSize_4th;
`endif //MIMO_4X4

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
	//			SoftReset          <= 1'b0; 
	//			PCIeTXBurstSize    <= 32'h0000_0000;
	//			PCIeRXBlockSize    <= 32'h0000_0000;
			DDR2MaxBurstSize		<= 5'b0_0000;
	
			 TransferSrcAddr_L	<= 32'h0000_0000;
			 TransferSrcAddr_H	<= 32'h0000_0000;
	//		 TransferMask			<= 32'h0000_0000;

			 SyncControl			<= 1'b0;			/// liuchang

			 RXControl				<= 1'b0;
			 RXBufAddr_L			<= 32'h0000_0000;
			 RXBufAddr_H			<= 32'h0000_0000;
			 RXBufSize				<= 32'h0000_0000;
			 TXControl_Start		<= 1'b0;
			 TXMask					<= 4'b0000;
			 TXAddr					<= 32'h0000_0000;
			 TXSize					<= 32'h0000_0000;
/// registers for 2nd to 4th paths/radios
			 RXControl_2nd			<= 1'b0;
			 RXBufAddr_2nd_L		<= 32'h0000_0000;
			 RXBufAddr_2nd_H		<= 32'h0000_0000;
			 RXBufSize_2nd			<= 32'h0000_0000;
			 TXControl_Start_2nd	<= 1'b0;
			 TXMask_2nd				<= 4'b0000;
			 TXAddr_2nd				<= 32'h0000_0000;
			 TXSize_2nd				<= 32'h0000_0000;
`ifdef MIMO_4X4
			 RXControl_3rd			<= 1'b0;
			 RXBufAddr_3rd_L		<= 32'h0000_0000;
			 RXBufAddr_3rd_H		<= 32'h0000_0000;
			 RXBufSize_3rd			<= 32'h0000_0000;
			 TXControl_Start_3rd	<= 1'b0;
			 TXMask_3rd				<= 4'b0000;
			 TXAddr_3rd				<= 32'h0000_0000;
			 TXSize_3rd				<= 32'h0000_0000;
			 RXControl_4th			<= 1'b0;
			 RXBufAddr_4th_L		<= 32'h0000_0000;
			 RXBufAddr_4th_H		<= 32'h0000_0000;
			 RXBufSize_4th			<= 32'h0000_0000;
			 TXControl_Start_4th	<= 1'b0;
			 TXMask_4th				<= 4'b0000;
			 TXAddr_4th				<= 32'h0000_0000;
			 TXSize_4th				<= 32'h0000_0000;
`endif //MIMO_4X4
`ifdef RADIO_CHANNEL_REGISTERS
			 RadioReg_WriteValue			<= 32'h0000_0000;
			 RadioReg_RW					<= 1'b0;
			 RadioReg_Addr					<= 7'b000_0000;
/// registers for 2nd to 4th paths/radios
			 RadioReg_2nd_WriteValue	<= 32'h0000_0000;
			 RadioReg_2nd_RW				<= 1'b0;
			 RadioReg_2nd_Addr			<= 7'b000_0000;
`ifdef MIMO_4X4
			 RadioReg_3rd_WriteValue	<= 32'h0000_0000;
			 RadioReg_3rd_RW				<= 1'b0;
			 RadioReg_3rd_Addr			<= 7'b000_0000;
			 RadioReg_4th_WriteValue	<= 32'h0000_0000;
			 RadioReg_4th_RW				<= 1'b0;
			 RadioReg_4th_Addr			<= 7'b000_0000;
`endif //MIMO_4X4
`endif //RADIO_CHANNEL_REGISTERS
		  end else begin    
				 if(reg_wren) begin 
							case(reg_wr_addr) 
							/// Jiansong: System Registers
	//								12'h048: SoftReset          <= reg_data_in[0]; //0x00
	//								12'h04C: PCIeTXBurstSize    <= reg_data_in; //0x04
	//								12'h050: PCIeRXBlockSize    <= reg_data_in; //0x08
									12'h020: DDR2MaxBurstSize		<= reg_data_in[4:0];
							/// Jiansong: DMA Registers
									12'h188: TransferSrcAddr_L		<= reg_data_in; //0x104
									12'h18C: TransferSrcAddr_H		<= reg_data_in; //0x108
	//								12'h16C: TransferMask       	<= reg_data_in; //0x11C

									12'h024: SyncControl				<= reg_data_in[0]; //0x024 liuchang
	
									12'h164: RXControl				<= reg_data_in[0]; //0x10C
									12'h170: RXBufAddr_L				<= reg_data_in; //0x110
									12'h174: RXBufAddr_H				<= reg_data_in; //0x114
									12'h194: RXBufSize				<= reg_data_in; //0x118
							/// Jiansong: Common Radio Registers
									12'h184: begin
												TXControl_Start		<= reg_data_in[0];
												TXMask					<= reg_data_in[19:16];
									end
									12'h178: TXAddr					<= reg_data_in;
									12'h190: TXSize					<= reg_data_in;
									12'h1A0: TXTime					<= reg_data_in;
									12'h1A4: RadioDelayCompen		<= reg_data_in;
/// registers for 2nd to 4th paths/radios
									12'h264: RXControl_2nd			<= reg_data_in[0]; 
									12'h270: RXBufAddr_2nd_L		<= reg_data_in;
									12'h274: RXBufAddr_2nd_H		<= reg_data_in;
									12'h294: RXBufSize_2nd			<= reg_data_in;
									12'h284: begin
												TXControl_Start_2nd	<= reg_data_in[0];
												TXMask_2nd				<= reg_data_in[19:16];
									end
									12'h278: TXAddr_2nd				<= reg_data_in;
									12'h290: TXSize_2nd				<= reg_data_in;
									12'h2A0: TXTime_2nd				<= reg_data_in;
									12'h2A4: RadioDelayCompen_2nd	<= reg_data_in;
`ifdef MIMO_4X4
									12'h364: RXControl_3rd			<= reg_data_in[0];
									12'h370: RXBufAddr_3rd_L		<= reg_data_in;
									12'h374: RXBufAddr_3rd_H		<= reg_data_in;
									12'h394: RXBufSize_3rd			<= reg_data_in;
									12'h384: begin
												TXControl_Start_3rd	<= reg_data_in[0];
												TXMask_3rd				<= reg_data_in[19:16];
									end
									12'h378: TXAddr_3rd				<= reg_data_in;
									12'h390: TXSize_3rd				<= reg_data_in;
									12'h3A0: TXTime_3rd				<= reg_data_in;
									12'h3A4: RadioDelayCompen_3rd	<= reg_data_in;
									12'h464: RXControl_4th			<= reg_data_in[0];
									12'h470: RXBufAddr_4th_L		<= reg_data_in;
									12'h474: RXBufAddr_4th_H		<= reg_data_in;
									12'h494: RXBufSize_4th			<= reg_data_in;
									12'h484: begin
												TXControl_Start_4th	<= reg_data_in[0];
												TXMask_4th				<= reg_data_in[19:16];
									end
									12'h478: TXAddr_4th				<= reg_data_in;
									12'h490: TXSize_4th				<= reg_data_in;
									12'h4A0: TXTime_4th				<= reg_data_in;
									12'h4A4: RadioDelayCompen_4th	<= reg_data_in;
`endif //MIMO_4X4

`ifdef RADIO_CHANNEL_REGISTERS
									12'h1C0: RadioReg_WriteValue		<= reg_data_in;
									12'h1C4: begin
												RadioReg_RW					<= reg_data_in[7];
												RadioReg_Addr[6:0]		<= reg_data_in[6:0];
									end
/// registers for 2nd to 4th paths/radios
									12'h2C0: RadioReg_2nd_WriteValue		<= reg_data_in;
									12'h2C4: begin
												RadioReg_2nd_RW				<= reg_data_in[7];
												RadioReg_2nd_Addr[6:0]		<= reg_data_in[6:0];
									end
`ifdef MIMO_4X4
									12'h3C0: RadioReg_3rd_WriteValue		<= reg_data_in;
									12'h3C4: begin
												RadioReg_3rd_RW				<= reg_data_in[7];
												RadioReg_3rd_Addr[6:0]		<= reg_data_in[6:0];
									end
									12'h4C0: RadioReg_4th_WriteValue		<= reg_data_in;
									12'h4C4: begin
												RadioReg_4th_RW				<= reg_data_in[7];
												RadioReg_4th_Addr[6:0]		<= reg_data_in[6:0];
									end
`endif //MIMO_4X4
`endif //RADIO_CHANNEL_REGISTERS
									default: begin 
	//                                    SoftReset          <= SoftReset; 
	//                                    PCIeTXBurstSize    <= PCIeTXBurstSize;
	//                                    PCIeRXBlockSize    <= PCIeRXBlockSize;
													DDR2MaxBurstSize		<= DDR2MaxBurstSize[4:0];
													
													TransferSrcAddr_L		<= TransferSrcAddr_L;
													TransferSrcAddr_H		<= TransferSrcAddr_H;
	//		                              TransferMask			<= TransferMask;

													SyncControl				<= SyncControl;	/// liuchang

													RXControl				<= RXControl;
													RXBufAddr_L				<= RXBufAddr_L;
													RXBufAddr_H				<= RXBufAddr_H;
													RXBufSize				<= RXBufSize;
													TXControl_Start		<= TXControl_Start;
													TXMask					<= TXMask;
													TXAddr					<= TXAddr;
													TXSize					<= TXSize;
/// registers for 2nd to 4th paths/radios
													RXControl_2nd			<= RXControl_2nd;
													RXBufAddr_2nd_L		<= RXBufAddr_2nd_L;
													RXBufAddr_2nd_H		<= RXBufAddr_2nd_H;
													RXBufSize_2nd			<= RXBufSize_2nd;
													TXControl_Start_2nd	<= TXControl_Start_2nd;
													TXMask_2nd				<= TXMask_2nd;
													TXAddr_2nd				<= TXAddr_2nd;
													TXSize_2nd				<= TXSize_2nd;

`ifdef MIMO_4X4	
													RXControl_3rd			<= RXControl_3rd;
													RXBufAddr_3rd_L		<= RXBufAddr_3rd_L;
													RXBufAddr_3rd_H		<= RXBufAddr_3rd_H;
													RXBufSize_3rd			<= RXBufSize_3rd;
													TXControl_Start_3rd	<= TXControl_Start_3rd;
													TXMask_3rd				<= TXMask_3rd;
													TXAddr_3rd				<= TXAddr_3rd;
													TXSize_3rd				<= TXSize_3rd;
	
													RXControl_4th			<= RXControl_4th;
													RXBufAddr_4th_L		<= RXBufAddr_4th_L;
													RXBufAddr_4th_H		<= RXBufAddr_4th_H;
													RXBufSize_4th			<= RXBufSize_4th;
													TXControl_Start_4th	<= TXControl_Start_4th;
													TXMask_4th				<= TXMask_4th;
													TXAddr_4th				<= TXAddr_4th;
													TXSize_4th				<= TXSize_4th;
`endif //MIMO_4X4	
`ifdef RADIO_CHANNEL_REGISTERS
													RadioReg_WriteValue			<= RadioReg_WriteValue;
													RadioReg_RW						<= RadioReg_RW;
													RadioReg_Addr					<= RadioReg_Addr;
/// registers for 2nd to 4th paths/radios
													RadioReg_2nd_WriteValue			<= RadioReg_2nd_WriteValue;
													RadioReg_2nd_RW					<= RadioReg_2nd_RW;
													RadioReg_2nd_Addr					<= RadioReg_2nd_Addr;
`ifdef MIMO_4X4
													RadioReg_3rd_WriteValue			<= RadioReg_3rd_WriteValue;
													RadioReg_3rd_RW					<= RadioReg_3rd_RW;
													RadioReg_3rd_Addr					<= RadioReg_3rd_Addr;
													RadioReg_4th_WriteValue			<= RadioReg_4th_WriteValue;
													RadioReg_4th_RW					<= RadioReg_4th_RW;
													RadioReg_4th_Addr					<= RadioReg_4th_Addr;
`endif //MIMO_4X4
`endif //RADIO_CHANNEL_REGISTERS												
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
	initial begin
		SoftReset    <= 1'b0;
		hostresetcnt <= 8'h00;
	end
	always@(posedge clk)begin
//		if(hard_rst) begin
//			SoftReset    <= 1'b0;
//			hostresetcnt <= 8'h00;
//		end else begin
				if (reg_wren) begin
					case(reg_wr_addr)
							12'h048: begin
								SoftReset    <= 1'b1;
								hostresetcnt <= 8'hC8;    // host reset for 200 cycles
							end
							default: begin
								if (hostresetcnt == 8'h00)begin
									SoftReset    <= 1'b0;
									hostresetcnt <= hostresetcnt;
								end else begin
									SoftReset    <= 1'b1;
									hostresetcnt <= hostresetcnt - 8'h01;
								end
							end
					endcase						
				end else begin
					if (hostresetcnt == 8'h00)begin
						SoftReset    <= 1'b0;
						hostresetcnt <= hostresetcnt;
					end else begin
						SoftReset    <= 1'b1;
						hostresetcnt <= hostresetcnt - 8'h01;
					end
				end
//		end
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
									12'h168: begin  //addr = 0x168
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
		  
		  

	/// Jiansong: TX_Done and TX_Ongoing	
	rising_edge_detect TX_Start_one_inst(
								.clk(clk),.rst(rst),.in(TXControl_Start),.one_shot_out(TX_Start_one));
//	rising_edge_detect TX_Clear_one_inst(
//								.clk(clk),.rst(rst),.in(~TXControl_Start),.one_shot_out(TX_Clear_one));
	always@(posedge clk)	set_TX_done_bit	<= RadioReg_ReadDone_in & RadioRegRead_Addr_in[7];
	always@(posedge clk) begin
		if (rst) begin
			TXControl_Done		<= 1'b0;
			TXControl_Ongoing	<= 1'b0;
			TX_Start_one_o		<= 1'b0;
			TXMask_radio		<= 4'b0000;
		end else if (~TXControl_Ongoing) begin
			if (TX_Start_one | (TX_Start_2nd_one & TXMask_2nd[0]) | (TX_Start_3rd_one & TXMask_3rd[0]) | (TX_Start_4th_one & TXMask_4th[0]) ) begin
				TXControl_Done		<= 1'b0;
				TXControl_Ongoing	<= 1'b1;
//			end else if (TX_Clear_one) begin
//				TXControl_Done		<= 1'b0;
//				TXControl_Ongoing	<= 1'b0;
			end else begin
				TXControl_Done		<= TXControl_Done;
				TXControl_Ongoing	<= TXControl_Ongoing;
			end
			TX_Start_one_o		<= TX_Start_one | (TX_Start_2nd_one & TXMask_2nd[0]) | (TX_Start_3rd_one & TXMask_3rd[0]) | (TX_Start_4th_one & TXMask_4th[0]);
			if (TX_Start_one)
				TXMask_radio	<= TXMask[3:0];
			else if (TX_Start_2nd_one & TXMask_2nd[0])
				TXMask_radio	<= TXMask_2nd[3:0];
			else if (TX_Start_3rd_one & TXMask_3rd[0])
				TXMask_radio	<= TXMask_3rd[3:0];
			else if (TX_Start_4th_one & TXMask_4th[0])
				TXMask_radio	<= TXMask_4th[3:0];
			else
				TXMask_radio	<= TXMask_radio[3:0];
		end else begin			
			if (set_TX_done_bit) begin
				TXControl_Done		<= 1'b1;
				TXControl_Ongoing	<= 1'b0;
			end else begin
				TXControl_Done		<= TXControl_Done;
				TXControl_Ongoing	<= TXControl_Ongoing;
			end
			TX_Start_one_o			<= 1'b0;			// ignore TX_start command if a TX is ongoing
		end
	end
	
	/// liuchang: TXTime_start2done calculate
	reg [31:0]	counter;
	reg [31:0]	TXTime_start_cnt;
	reg [31:0]	TXTime_done_cnt;
	wire			set_TX_done_bit_one;
	
	rising_edge_detect set_TX_done_bit_one_inst(
								.clk(clk),.rst(rst),.in(set_TX_done_bit),.one_shot_out(set_TX_done_bit_one));
								
	always@(posedge clk) begin
	 if(rst)
		 counter <= 32'h0000_0000;
	 else
		 counter <= counter + 32'h0000_0001;
	end	
	always@ (posedge clk) begin
		if (rst)
			TXTime_start_cnt[31:0] <= 32'h0000_0000;
		else if (TX_Start_one)
			TXTime_start_cnt[31:0] <= counter[31:0];
		else
			TXTime_start_cnt[31:0] <= TXTime_start_cnt[31:0];
	end
	
	always@ (posedge clk) begin
		if (rst)
			TXTime_done_cnt[31:0] <= 32'h0000_0000;
		else if (set_TX_done_bit_one)
			TXTime_done_cnt[31:0] <= counter[31:0];
		else
			TXTime_done_cnt[31:0] <= TXTime_done_cnt[31:0];
	end	
	
	assign TXTime_start2done = TXTime_done_cnt - TXTime_start_cnt;
	
//	always@(posedge clk) begin
//		if(rst)
//			TXControl[1:0] <= 2'b00;
//		else begin
//			if(set_TX_done_bit) begin
//				TXControl[1] <= 1'b1;
//				TXControl[0] <= TXControl[0];
//			end else if (reg_wren)begin
//					  case(reg_wr_addr)
//							  12'h184: begin
//									TXControl[1] <= 1'b0;
//									TXControl[0] <= reg_data_in[0];
//							  end
//							  default: begin
//								  TXControl[1:0] <= TXControl[1:0];
//							  end
//						endcase
//			end else begin
//				TXControl[1:0] <= TXControl[1:0];
//			end
//		end
//	end
/// registers for 2nd path/radio	
	rising_edge_detect TX_Start_2nd_one_inst(
								.clk(clk),.rst(rst),.in(TXControl_Start_2nd),.one_shot_out(TX_Start_2nd_one));
//	rising_edge_detect TX_Clear_2nd_one_inst(
//								.clk(clk),.rst(rst),.in(~TXControl_Start_2nd),.one_shot_out(TX_Clear_2nd_one));
	always@(posedge clk)	set_TX_done_bit_2nd	<= RadioReg_2nd_ReadDone_in & RadioRegRead_2nd_Addr_in[7];
	always@(posedge clk) begin
		if (rst) begin
			TXControl_Done_2nd		<= 1'b0;
			TXControl_Ongoing_2nd	<= 1'b0;
			TX_Start_2nd_one_o		<= 1'b0;
			TXMask_2nd_radio			<= 4'b0000;
		end else if (~TXControl_Ongoing_2nd) begin
			if ((TX_Start_one & TXMask[1]) | TX_Start_2nd_one | (TX_Start_3rd_one & TXMask_3rd[1]) | (TX_Start_4th_one & TXMask_4th[1]) ) begin
				TXControl_Done_2nd		<= 1'b0;
				TXControl_Ongoing_2nd	<= 1'b1;
//			end else if (TX_Clear_2nd_one) begin
//				TXControl_Done_2nd		<= 1'b0;
//				TXControl_Ongoing_2nd	<= 1'b0;
			end else begin
				TXControl_Done_2nd		<= TXControl_Done_2nd;
				TXControl_Ongoing_2nd	<= TXControl_Ongoing_2nd;
			end
			TX_Start_2nd_one_o	<= (TX_Start_one & TXMask[1]) | TX_Start_2nd_one | (TX_Start_3rd_one & TXMask_3rd[1]) | (TX_Start_4th_one & TXMask_4th[1]);
			if (TX_Start_one & TXMask[1])
				TXMask_2nd_radio	<= TXMask[3:0];
			else if (TX_Start_2nd_one)
				TXMask_2nd_radio	<= TXMask_2nd[3:0];
			else if (TX_Start_3rd_one & TXMask_3rd[1])
				TXMask_2nd_radio	<= TXMask_3rd[3:0];
			else if (TX_Start_4th_one & TXMask_4th[1])
				TXMask_2nd_radio	<= TXMask_4th[3:0];
			else
				TXMask_2nd_radio	<= TXMask_2nd_radio[3:0];
		end else begin
			TX_Start_2nd_one_o		<= 1'b0;
			if (set_TX_done_bit_2nd) begin
				TXControl_Done_2nd		<= 1'b1;
				TXControl_Ongoing_2nd	<= 1'b0;
			end else begin
				TXControl_Done_2nd		<= TXControl_Done_2nd;
				TXControl_Ongoing_2nd	<= TXControl_Ongoing_2nd;
			end
		end
	end
	
	/// liuchang: TXTime_start2done_2nd calculate
	reg [31:0]	TXTime_start_2nd_cnt;
	reg [31:0]	TXTime_done_2nd_cnt;
	wire			set_TX_done_bit_2nd_one;
	
	rising_edge_detect set_TX_done_bit_2nd_one_inst(
								.clk(clk),.rst(rst),.in(set_TX_done_bit_2nd),.one_shot_out(set_TX_done_bit_2nd_one));
								
	always@ (posedge clk) begin
		if (rst)
			TXTime_start_2nd_cnt[31:0] <= 32'h0000_0000;
		else if (TX_Start_2nd_one)
			TXTime_start_2nd_cnt[31:0] <= counter[31:0];
		else
			TXTime_start_2nd_cnt[31:0] <= TXTime_start_2nd_cnt[31:0];
	end
	
	always@ (posedge clk) begin
		if (rst)
			TXTime_done_2nd_cnt[31:0] <= 32'h0000_0000;
		else if (set_TX_done_bit_2nd_one)
			TXTime_done_2nd_cnt[31:0] <= counter[31:0];
		else
			TXTime_done_2nd_cnt[31:0] <= TXTime_done_2nd_cnt[31:0];
	end	
	
	assign TXTime_start2done_2nd = TXTime_done_2nd_cnt - TXTime_start_2nd_cnt;
	
//	always@(posedge clk)begin
//		if(rst)
//			TXControl_2nd[1:0] <= 2'b00;
//		else begin
//			if(set_TX_done_bit_2nd) begin
//				TXControl_2nd[1] <= 1'b1;
//				TXControl_2nd[0] <= TXControl_2nd[0];
//			end else if (reg_wren)begin
//					  case(reg_wr_addr)
//							  12'h284: begin
//									TXControl_2nd[1] <= 1'b0;
//									TXControl_2nd[0] <= reg_data_in[0];
//							  end
//							  default: begin
//								  TXControl_2nd[1:0] <= TXControl_2nd[1:0];
//							  end
//						endcase
//			end else begin
//				TXControl_2nd[1:0] <= TXControl_2nd[1:0];
//			end
//		end
//	end

`ifdef MIMO_4X4
/// registers for 3rd path/radio		
	rising_edge_detect TX_Start_3rd_one_inst(
								.clk(clk),.rst(rst),.in(TXControl_Start_3rd),.one_shot_out(TX_Start_3rd_one));
//	rising_edge_detect TX_Clear_3rd_one_inst(
//								.clk(clk),.rst(rst),.in(~TXControl_Start_3rd),.one_shot_out(TX_Clear_3rd_one));
	always@(posedge clk)	set_TX_done_bit_3rd	<= RadioReg_3rd_ReadDone_in & RadioRegRead_3rd_Addr_in[7];
	always@(posedge clk) begin
		if (rst) begin
			TXControl_Done_3rd		<= 1'b0;
			TXControl_Ongoing_3rd	<= 1'b0;
			TX_Start_3rd_one_o		<= 1'b0;
			TXMask_3rd_radio			<= 4'b0000;
		end else if (~TXControl_Ongoing_3rd) begin
			if ((TX_Start_one & TXMask[2]) | (TX_Start_2nd_one & TXMask_2nd[2]) | TX_Start_3rd_one | (TX_Start_4th_one & TXMask_4th[2]) ) begin
				TXControl_Done_3rd		<= 1'b0;
				TXControl_Ongoing_3rd	<= 1'b1;
//			end else if (TX_Clear_3rd_one) begin
//				TXControl_Done_3rd		<= 1'b0;
//				TXControl_Ongoing_3rd	<= 1'b0;
			end else begin
				TXControl_Done_3rd		<= TXControl_Done_3rd;
				TXControl_Ongoing_3rd	<= TXControl_Ongoing_3rd;
			end
			TX_Start_3rd_one_o		<= (TX_Start_one & TXMask[2]) | (TX_Start_2nd_one & TXMask_2nd[2]) | TX_Start_3rd_one | (TX_Start_4th_one & TXMask_4th[2]);
			if (TX_Start_one & TXMask[2])
				TXMask_3rd_radio	<= TXMask[3:0];
			else if (TX_Start_2nd_one & TXMask_2nd[2])
				TXMask_3rd_radio	<= TXMask_2nd[3:0];
			else if (TX_Start_3rd_one)
				TXMask_3rd_radio	<= TXMask_3rd[3:0];
			else if (TX_Start_4th_one & TXMask_4th[2])
				TXMask_3rd_radio	<= TXMask_4th[3:0];
			else
				TXMask_3rd_radio	<= TXMask_3rd_radio[3:0];
		end else begin
			if (set_TX_done_bit_3rd) begin
				TXControl_Done_3rd		<= 1'b1;
				TXControl_Ongoing_3rd	<= 1'b0;
			end else begin
				TXControl_Done_3rd		<= TXControl_Done_3rd;
				TXControl_Ongoing_3rd	<= TXControl_Ongoing_3rd;
			end
			TX_Start_3rd_one_o	<= 1'b0;
			TXMask_3rd_radio		<= TXMask_3rd_radio[3:0];
		end
	end
	/// liuchang: TXTime_start2done_3rd calculate
	reg [31:0]	TXTime_start_3rd_cnt;
	reg [31:0]	TXTime_done_3rd_cnt;
	wire			set_TX_done_bit_3rd_one;
	
	rising_edge_detect set_TX_done_bit_3rd_one_inst(
								.clk(clk),.rst(rst),.in(set_TX_done_bit_3rd),.one_shot_out(set_TX_done_bit_3rd_one));
								
	always@ (posedge clk) begin
		if (rst)
			TXTime_start_3rd_cnt[31:0] <= 32'h0000_0000;
		else if (TX_Start_3rd_one)
			TXTime_start_3rd_cnt[31:0] <= counter[31:0];
		else
			TXTime_start_3rd_cnt[31:0] <= TXTime_start_3rd_cnt[31:0];
	end
	
	always@ (posedge clk) begin
		if (rst)
			TXTime_done_3rd_cnt[31:0] <= 32'h0000_0000;
		else if (set_TX_done_bit_3rd_one)
			TXTime_done_3rd_cnt[31:0] <= counter[31:0];
		else
			TXTime_done_3rd_cnt[31:0] <= TXTime_done_3rd_cnt[31:0];
	end	
	
	assign TXTime_start2done_3rd = TXTime_done_3rd_cnt - TXTime_start_3rd_cnt;
	
//	always@(posedge clk)begin
//		if(rst)
//			TXControl_3rd[1:0] <= 2'b00;
//		else begin
//			if(set_TX_done_bit_3rd) begin
//				TXControl_3rd[1] <= 1'b1;
//				TXControl_3rd[0] <= TXControl_3rd[0];
//			end else if (reg_wren)begin
//					  case(reg_wr_addr)
//							  12'h384: begin
//									TXControl_3rd[1] <= 1'b0;
//									TXControl_3rd[0] <= reg_data_in[0];
//							  end
//							  default: begin
//								  TXControl_3rd[1:0] <= TXControl_3rd[1:0];
//							  end
//						endcase
//			end else begin
//				TXControl_3rd[1:0] <= TXControl_3rd[1:0];
//			end
//		end
//	end
/// registers for 4th path/radio		
	rising_edge_detect TX_Start_4th_one_inst(
								.clk(clk),.rst(rst),.in(TXControl_Start_4th),.one_shot_out(TX_Start_4th_one));
//	rising_edge_detect TX_Clear_4th_one_inst(
//								.clk(clk),.rst(rst),.in(~TXControl_Start_4th),.one_shot_out(TX_Clear_4th_one));
	always@(posedge clk)	set_TX_done_bit_4th	<= RadioReg_4th_ReadDone_in & RadioRegRead_4th_Addr_in[7];
	always@(posedge clk) begin
		if (rst) begin
			TXControl_Done_4th		<= 1'b0;
			TXControl_Ongoing_4th	<= 1'b0;
			TX_Start_4th_one_o		<= 1'b0;
			TXMask_4th_radio			<= 4'b0000;
		end else if (~TXControl_Ongoing_4th) begin
			if ((TX_Start_one & TXMask[3]) | (TX_Start_2nd_one & TXMask_2nd[3]) | (TX_Start_3rd_one & TXMask_3rd[3]) | TX_Start_4th_one) begin
				TXControl_Done_4th		<= 1'b0;
				TXControl_Ongoing_4th	<= 1'b1;
//			end else if (TX_Clear_4th_one) begin
//				TXControl_Done_4th		<= 1'b0;
//				TXControl_Ongoing_4th	<= 1'b0;
			end else begin
				TXControl_Done_4th		<= TXControl_Done_4th;
				TXControl_Ongoing_4th	<= TXControl_Ongoing_4th;
			end
			TX_Start_4th_one_o		<= (TX_Start_one & TXMask[3]) | (TX_Start_2nd_one & TXMask_2nd[3]) | (TX_Start_3rd_one & TXMask_3rd[3]) | TX_Start_4th_one;
			if (TX_Start_one & TXMask[3])
				TXMask_4th_radio	<= TXMask[3:0];
			else if (TX_Start_2nd_one & TXMask_2nd[3])
				TXMask_4th_radio	<= TXMask_2nd[3:0];
			else if (TX_Start_3rd_one & TXMask_3rd[3])
				TXMask_4th_radio	<= TXMask_3rd[3:0];
			else if (TX_Start_4th_one)
				TXMask_4th_radio	<= TXMask_4th[3:0];
			else
				TXMask_4th_radio	<= TXMask_4th_radio[3:0];
		end else begin
			if (set_TX_done_bit_4th) begin
				TXControl_Done_4th		<= 1'b1;
				TXControl_Ongoing_4th	<= 1'b0;
			end else begin
				TXControl_Done_4th		<= TXControl_Done_4th;
				TXControl_Ongoing_4th	<= TXControl_Ongoing_4th;
			end
			TX_Start_4th_one_o	<= 1'b0;
			TXMask_4th_radio		<= TXMask_4th_radio[3:0];
		end
	end
	/// liuchang: TXTime_start2done_4th calculate
	reg [31:0]	TXTime_start_4th_cnt;
	reg [31:0]	TXTime_done_4th_cnt;
	wire			set_TX_done_bit_4th_one;
	
	rising_edge_detect set_TX_done_bit_4th_one_inst(
								.clk(clk),.rst(rst),.in(set_TX_done_bit_4th),.one_shot_out(set_TX_done_bit_4th_one));
								
	always@ (posedge clk) begin
		if (rst)
			TXTime_start_4th_cnt[31:0] <= 32'h0000_0000;
		else if (TX_Start_4th_one)
			TXTime_start_4th_cnt[31:0] <= counter[31:0];
		else
			TXTime_start_4th_cnt[31:0] <= TXTime_start_4th_cnt[31:0];
	end
	
	always@ (posedge clk) begin
		if (rst)
			TXTime_done_4th_cnt[31:0] <= 32'h0000_0000;
		else if (set_TX_done_bit_4th_one)
			TXTime_done_4th_cnt[31:0] <= counter[31:0];
		else
			TXTime_done_4th_cnt[31:0] <= TXTime_done_4th_cnt[31:0];
	end	
	
	assign TXTime_start2done_4th = TXTime_done_4th_cnt - TXTime_start_4th_cnt;	

//	always@(posedge clk)begin
//		if(rst)
//			TXControl_4th[1:0] <= 2'b00;
//		else begin
//			if(set_TX_done_bit_4th) begin
//				TXControl_4th[1] <= 1'b1;
//				TXControl_4th[0] <= TXControl_4th[0];
//			end else if (reg_wren)begin
//					  case(reg_wr_addr)
//							  12'h484: begin
//									TXControl_4th[1] <= 1'b0;
//									TXControl_4th[0] <= reg_data_in[0];
//							  end
//							  default: begin
//								  TXControl_4th[1:0] <= TXControl_4th[1:0];
//							  end
//						endcase
//			end else begin
//				TXControl_4th[1:0] <= TXControl_4th[1:0];
//			end
//		end
//	end
`endif //MIMO_4X4

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

	always@(posedge clk) FirmwareVersion[31:0] <= 32'h0200_0000;			// Firmware version is 2.0.0

//	//////////////////////////////////
//	/// Jiansong: debug registers ////
//	//////////////////////////////////

always@(posedge clk)	DebugRX1Overflowcount[31:0]	<= DebugRX1Overflowcount_in[31:0];
always@(posedge clk)	DebugRX2Overflowcount[31:0]	<= DebugRX2Overflowcount_in[31:0];

//	rising_edge_detect RX_FIFO_full_one_inst(
//						 .clk(clk),
//						 .rst(rst),
//						 .in(RX_FIFO_full),
//						 .one_shot_out(RX_FIFO_full_one)
//						 );
//	always@(posedge clk)begin
//		if(rst)begin
//			Debug1[31:16] <= 16'h0000;
//			Debug1[3:0]   <= 4'h0;
//			Debug2 <= 32'h0000_0000;
//			Debug3 <= 32'h0000_0000;
//		end else if (egress_overflow_one) begin
//			Debug1[0]    <= 1'b1;
//			Debug1[3:1]  <= Debug1[3:1];
//			Debug1[31:16] <= Debug1[31:16];
//		end else if (RX_FIFO_full_one) begin
//			Debug1[1]    <= 1'b1;
//			Debug1[0] <= Debug1[0];
//			Debug1[3:2]   <= Debug1[3:2];
//			Debug1[31:16] <= Debug1[31:16];
////		end else if (transferstart_one && (dmarxs_now[31:7] != 0)) begin
////			Debug1[2] <= 1'b1;
//		end else if (~RXControl) begin   /// clear this RX_FIFO_full signal if RX is disabled
//			Debug1[1]    <= 1'b0;
//			Debug1[0] <= Debug1[0];
//			Debug1[3:2]   <= Debug1[3:2];
//			Debug1[31:16] <= Debug1[31:16];
//		end else begin
//			Debug1[3:0]  <= Debug1[3:0];
//			Debug1[31:16] <= {11'h000,locked_debug[4:0]};
//			Debug2 <= egress_rd_data_count;
//			Debug3 <= egress_wr_data_count;
//		end
//	end
//
//	always@(posedge clk)begin
//		if(rst)begin
//			Debug1[15:4]  <= 12'h000;
//		end else begin
//				Debug1[11:4] <= FIFOErrors_in[7:0];
//				Debug1[15:12] <= Debug1[15:12];
//		end
//	end
//
//	/// TX done times
//	always@(posedge clk)begin
//		if(rst)
//			Debug4 <= 32'hFFFF_FFFF;
//		else if(set_TX_done_bit)
//			Debug4 <= Debug4 + 32'h0000_0001;
//		else
//			Debug4 <= Debug4;
//	end
//
////	always@(posedge clk)begin
////		Debug5SourceAddr_L <= SourceAddr_L;  //
////		Debug6SourceAddr_H <= SourceAddr_H;
////		Debug7DestAddr     <= DestAddr;
////		Debug8FrameSize    <= FrameSize;
////	end
//
//	always@(posedge clk)begin
//		if(rst)
//			Debug9 <= 32'h0000_0000;
//		else if (egress_overflow_one)
//			Debug9 <= Debug9 + 32'h0000_0001;
//		else
//			Debug9 <= Debug9;
//	end
//
//	always@(posedge clk)begin
//		if(rst)
//			Debug10 <= 32'h0000_0000;
//		else if (RX_FIFO_full_one)
//			Debug10 <= Debug10 + 32'h0000_0001;
//		else
//			Debug10 <= Debug10;
//	end
//
////	always@(posedge clk)begin
////		Debug11TF1[3:0]   <= state_0[3:0];
////		Debug11TF1[7:4]   <= state_3[3:0];
////		Debug11TF1[11:8]  <= {1'b0,state_4[2:0]};
////		Debug11TF1[15:12] <= {3'b000,start_sm0_rdma_flow};
////		Debug11TF1[19:16] <= {3'b000,read_last};
////		Debug11TF1[23:20] <= {3'b000,pause_read_requests};
////		Debug11TF1[27:24] <= {3'b000,TX_desc_write_back_req};
////		Debug11TF1[31:28] <= {3'b000,rd_dma_done_i};
////	end
//
////	always@(posedge clk) Debug12TF2[31:0] <= dmarad_now[31:0];
////	always@(posedge clk) Debug13TF3[31:0] <= dmaras_now[31:0];
////	always@(posedge clk) Debug14TF4[31:0] <= dmarxs_now[31:0];
//
////	always@(posedge clk)begin
////		Debug15TX1[3:0]   <= {1'b0,TX_state_0[2:0]};
////		Debug15TX1[7:4]   <= TX_state_1[3:0];
////		Debug15TX1[11:8]  <= {3'b0,TX_start};
////		Debug15TX1[15:12] <= {3'b0,Radio_TX_done};
////		Debug15TX1[19:16] <= {3'b0,next_TX_ready};
////		Debug15TX1[31:20] <= 12'h000;
////	end
////
////	always@(posedge clk)begin
////		Debug16TX2[31:0] <= TX_addr_now[31:0];
////	end
////
////	always@(posedge clk)begin
////		Debug17TX3[31:0] <= TX_size_now[31:0];
////	end
//
////	always@(posedge clk) Debug18DDR1[31:0] <= Debug18DDR1_in[31:0];
////	always@(posedge clk) Debug19DDR2[31:0] <= Debug19DDR2_in[31:0];
//	always@(posedge clk) Debug20RX1[31:0] <= Debug20RX1_in[31:0];
////	always@(posedge clk) Debug21RX2[31:0] <= Debug21RX2_in[31:0];
////	always@(posedge clk) Debug22RX3[4:0]  <= Debug22RX3_in[4:0];
////	always@(posedge clk) Debug23RX4[31:0] <= Debug23RX4_in[31:0];
//	always@(posedge clk) Debug24RX5[31:0] <= Debug24RX5_in[31:0];
////	always@(posedge clk) Debug25RX6[31:0] <= Debug25RX6_in[31:0];
//	always@(posedge clk) Debug26RX7[31:0] <= Debug26RX7_in[31:0];
//	always@(posedge clk) Debug27RX8[31:0] <= Debug27RX8_in[31:0];
//	always@(posedge clk) Debug28RX9[31:0] <= Debug28RX9_in[31:0];
//	always@(posedge clk) Debug29RX10[31:0] <= Debug29RX10_in[31:0];
////	always@(posedge clk) Debug30RXEngine[9:0] <= Debug30RXEngine_in[9:0];
////	always@(posedge clk) Debug31RXDataFIFOfullcnt[11:0] <= Debug31RXDataFIFOfullcnt_in[11:0];
////	always@(posedge clk) Debug32RXXferFIFOfullcnt[11:0] <= Debug32RXXferFIFOfullcnt_in[11:0];
////	always@(posedge clk) Debug33RXDataFIFOWRcnt[23:0] <= Debug33RXDataFIFOWRcnt_in[23:0];
////	always@(posedge clk) Debug34RXDataFIFORDcnt[23:0] <= Debug34RXDataFIFORDcnt_in[23:0];
////	always@(posedge clk) Debug35RXXferFIFOWRcnt[23:0] <= Debug35RXXferFIFOWRcnt_in[23:0];
////	always@(posedge clk) Debug36RXXferFIFORDcnt[23:0] <= Debug36RXXferFIFORDcnt_in[23:0];
////	always@(posedge clk) Debug37completion_pending[31:0] <= Debug37completion_pending_in[31:0];
////	always@(posedge clk) Debug38tag_value[4:0] <= Debug38tag_value_in[4:0];

always@(posedge clk)	DebugDDREgressFIFOCnt	<= DebugDDREgressFIFOCnt_in[31:0];
always@(posedge clk)	DebugDDRFIFOFullCnt	<= DebugDDRFIFOFullCnt_in[31:0];
always@(posedge clk)	DebugDDRSignals		<= DebugDDRSignals_in[31:0];
always@(posedge clk)	DebugDDRSMs				<= DebugDDRSMs_in[8:0];
		  
	///////////////////////////////////////////////
	/// Jiansong: radio registers interpreting ////
	///////////////////////////////////////////////

	/// liuchang: add for Sync debug
	wire			SyncControl_one;
	reg [31:0]	SyncControl_cnt;	
	rising_edge_detect SyncControl_inst(
						 .clk(clk),
						 .rst(rst),
						 .in(SyncControl),
						 .one_shot_out(SyncControl_one)
						 );	

	always@ (posedge clk) begin
		if (rst)
			SyncControl_cnt[31:0] <= 32'h0000_0000;
		else if(SyncControl_one)
			SyncControl_cnt[31:0] <= SyncControl_cnt[31:0] + 32'h0000_0001;
		else
			SyncControl_cnt[31:0] <= SyncControl_cnt[31:0];
	end

`ifdef RADIO_CHANNEL_REGISTERS
//	always@(posedge clk)begin				// this hard-coded implementation is only for single radio, driver will periodically check this bit
//		RadioStatus		   <= 1'b1;
//	end

	// Generate radio cmd
	always@(posedge clk) begin
		if(rst) begin
			Radio_Cmd_Data[31:0] <= 32'h0000_0000;
			Radio_Cmd_Addr[6:0]  <= 7'b000_0000;
			Radio_Cmd_RdWr			<= 1'b0;
			Radio_Cmd_wren			<= 1'b0;
		end else if(reg_wren) begin
				case(reg_wr_addr[11:0])
					12'h024: begin							/// liuchang: SyncControl, add for sync
							Radio_Cmd_Data[31:0] <= reg_data_in[31:0];
							Radio_Cmd_Addr[6:0]  <= 7'h18;
							Radio_Cmd_RdWr			<= 1'b0;
							Radio_Cmd_wren			<= 1'b1;		
					end
					12'h1C4: begin
//						if (~reg_data_in[6]) begin		// protected space can not be accessed
							Radio_Cmd_Data[31:0] <= RadioReg_WriteValue[31:0];
							Radio_Cmd_Addr[6:0]  <= reg_data_in[6:0];
							Radio_Cmd_RdWr			<= reg_data_in[7];
							Radio_Cmd_wren			<= 1'b1;
//						end else begin
//							Radio_Cmd_Data[31:0] <= 32'h0000_0000;
//							Radio_Cmd_Addr[6:0]  <= 7'b000_0000;
//							Radio_Cmd_RdWr			<= 1'b0;
//							Radio_Cmd_wren			<= 1'b0;
//						end
					end
//					// also pass TXControl and RXControl to Radio Module
//					12'h184: begin			// TXControl
//						Radio_Cmd_Data[31:0] <= reg_data_in[31:0];
//						Radio_Cmd_Addr[6:0]  <= 7'h02;
//						Radio_Cmd_RdWr			<= 1'b0;
//						Radio_Cmd_wren			<= 1'b1;
//					end
					12'h164: begin			// RXControl					
						Radio_Cmd_Data[31:0] <= reg_data_in[31:0];
//						Radio_Cmd_Addr[6:0]  <= 7'h03;
						Radio_Cmd_Addr[6:0]  <= 7'h41;
						Radio_Cmd_RdWr			<= 1'b0;
						Radio_Cmd_wren			<= 1'b1;
					end
					12'h1A0: begin			// TXTime				
						Radio_Cmd_Data[31:0] <= reg_data_in[31:0];
						Radio_Cmd_Addr[6:0]  <= 7'h42;
						Radio_Cmd_RdWr			<= 1'b0;
						Radio_Cmd_wren			<= 1'b1;
					end
					12'h1A4: begin			// RadioDelayCompen				
						Radio_Cmd_Data[31:0] <= reg_data_in[31:0];
						Radio_Cmd_Addr[6:0]  <= 7'h43;
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
		end else if (TX_Start_one_o | TX_Start_one_r) begin			// only one signal will be high
			Radio_Cmd_Data[31:0]	<= {12'h000,TXMask_radio[3:0],16'h0001};
//			Radio_Cmd_Addr[6:0]	<= 7'h40;
			Radio_Cmd_Addr[6:0]	<= 7'h30;
			Radio_Cmd_RdWr			<= 1'b0;
			Radio_Cmd_wren			<= 1'b1;
		end else begin
			Radio_Cmd_Data[31:0] <= 32'h0000_0000;
			Radio_Cmd_Addr[6:0]  <= 7'b000_0000;
			Radio_Cmd_RdWr			<= 1'b0;
			Radio_Cmd_wren			<= 1'b0;
		end
	end
	always@(posedge clk)	TX_Start_one_r	<= TX_Start_one_o & reg_wren;

	// RadioReg_ReadDone bit & RadioReg_ValueReturn
	always@(posedge clk) begin
		if(rst) begin
			RadioReg_ReadDone <= 1'b0;
			RadioReg_ValueReturn[31:0] <= 32'hA0B0_C0D0;
		end else if (reg_wren) begin
						case(reg_wr_addr) 
							12'h1C4: begin
								if (reg_data_in[6]) begin
									RadioReg_ReadDone <= 1'b1;			
									RadioReg_ValueReturn[31:0] <= 32'hFFFF_0000; // frequently set to default value
								end else begin
									RadioReg_ReadDone <= 1'b0;			// means this bit is write-only, any write from PC to this bit will clear this bit
									RadioReg_ValueReturn[31:0] <= 32'hA0B0_C0D0; // frequently set to default value
								end
							end
							default: begin									// write in other registers will be bypassed
								RadioReg_ReadDone <= RadioReg_ReadDone;
								RadioReg_ValueReturn[31:0] <= RadioReg_ValueReturn[31:0];
							end
						endcase
		end else if (set_RadioReg_ReadDone_bit_two) begin		// two-cycle signal, it will not be missed
						if (~RadioRegRead_Addr_in[7]) begin			// make sure it is a radio register read return event
							RadioReg_ReadDone <= 1'b1;
							RadioReg_ValueReturn[31:0] <= RadioRegRead_Value_in[31:0];
						end else begin
							RadioReg_ReadDone <= RadioReg_ReadDone;
							RadioReg_ValueReturn[31:0] <= RadioReg_ValueReturn[31:0];
						end
		end else begin
				RadioReg_ReadDone <= RadioReg_ReadDone;
				RadioReg_ValueReturn[31:0] <= RadioReg_ValueReturn[31:0];
		end
	end
	
	always@(posedge clk) RadioReg_ReadDone_in_reg <= RadioReg_ReadDone_in;    // add one register for signal which crosses time domain
	// RadioReg_ReadDone_in crosses time domain (from FRL_clk to 125MHz), it's better to do edge detection
	rising_edge_detect RadioReg_ReadDone_one_inst(
						 .clk(clk),.rst(rst),.in(RadioReg_ReadDone_in_reg),.one_shot_out(set_RadioReg_ReadDone_bit));
	always@(posedge clk)	set_RadioReg_ReadDone_bit_r	<= set_RadioReg_ReadDone_bit;
	assign	set_RadioReg_ReadDone_bit_two = set_RadioReg_ReadDone_bit | set_RadioReg_ReadDone_bit_r;
						 
/// registers for 2nd path/radio					  						 						 
// Generate radio cmd
	always@(posedge clk) begin
		if(rst) begin
			Radio_2nd_Cmd_Data[31:0]	<= 32'h0000_0000;
			Radio_2nd_Cmd_Addr[6:0] 	<= 7'b000_0000;
			Radio_2nd_Cmd_RdWr			<= 1'b0;
			Radio_2nd_Cmd_wren			<= 1'b0;
		end else if(reg_wren) begin
				case(reg_wr_addr[11:0])
					12'h024: begin							/// liuchang: SyncControl, add for sync, not really used in RAB
							Radio_2nd_Cmd_Data[31:0] 	<= reg_data_in[31:0];
							Radio_2nd_Cmd_Addr[6:0] 	<= 7'h18;
							Radio_2nd_Cmd_RdWr			<= 1'b0;
							Radio_2nd_Cmd_wren			<= 1'b1;		
					end
					12'h2C4: begin
//						if (~reg_data_in[6]) begin		// protected space can not be accessed
							Radio_2nd_Cmd_Data[31:0]	<= RadioReg_2nd_WriteValue[31:0];
							Radio_2nd_Cmd_Addr[6:0]		<= reg_data_in[6:0];
							Radio_2nd_Cmd_RdWr			<= reg_data_in[7];
							Radio_2nd_Cmd_wren			<= 1'b1;
//						end else begin
//							Radio_2nd_Cmd_Data[31:0] <= 32'h0000_0000;
//							Radio_2nd_Cmd_Addr[6:0]  <= 7'b000_0000;
//							Radio_2nd_Cmd_RdWr			<= 1'b0;
//							Radio_2nd_Cmd_wren			<= 1'b0;
//						end
					end
//					// also pass TXControl and RXControl to Radio Module
//					12'h284: begin			// TXControl
//						Radio_2nd_Cmd_Data[31:0]	<= reg_data_in[31:0];
//						Radio_2nd_Cmd_Addr[6:0]		<= 7'h02;
//						Radio_2nd_Cmd_RdWr			<= 1'b0;
//						Radio_2nd_Cmd_wren			<= 1'b1;
//					end
					12'h264: begin			// RXControl					
						Radio_2nd_Cmd_Data[31:0]	<= reg_data_in[31:0];
//						Radio_2nd_Cmd_Addr[6:0]		<= 7'h03;
						Radio_2nd_Cmd_Addr[6:0]		<= 7'h41;
						Radio_2nd_Cmd_RdWr			<= 1'b0;
						Radio_2nd_Cmd_wren			<= 1'b1;
					end
					12'h2A0: begin			// TXTime	
						Radio_2nd_Cmd_Data[31:0]	<= reg_data_in[31:0];
						Radio_2nd_Cmd_Addr[6:0]		<= 7'h42;
						Radio_2nd_Cmd_RdWr			<= 1'b0;
						Radio_2nd_Cmd_wren			<= 1'b1;
					end
					12'h2A4: begin			// RadioDelayCompen	
						Radio_2nd_Cmd_Data[31:0]	<= reg_data_in[31:0];
						Radio_2nd_Cmd_Addr[6:0]		<= 7'h43;
						Radio_2nd_Cmd_RdWr			<= 1'b0;
						Radio_2nd_Cmd_wren			<= 1'b1;
					end
					
					default: begin
						Radio_2nd_Cmd_Data[31:0]	<= 32'h0000_0000;
						Radio_2nd_Cmd_Addr[6:0]		<= 7'b000_0000;
						Radio_2nd_Cmd_RdWr			<= 1'b0;
						Radio_2nd_Cmd_wren			<= 1'b0;
					end
				endcase
		end else if (TX_Start_2nd_one_o | TX_Start_2nd_one_r) begin
			Radio_2nd_Cmd_Data[31:0]	<= {12'h000,TXMask_2nd_radio[3:0],16'h0001};
//			Radio_2nd_Cmd_Addr[6:0]		<= 7'h40;
			Radio_2nd_Cmd_Addr[6:0]		<= 7'h30;
			Radio_2nd_Cmd_RdWr			<= 1'b0;
			Radio_2nd_Cmd_wren			<= 1'b1;
		end else begin
			Radio_2nd_Cmd_Data[31:0] <= 32'h0000_0000;
			Radio_2nd_Cmd_Addr[6:0]  <= 7'b000_0000;
			Radio_2nd_Cmd_RdWr			<= 1'b0;
			Radio_2nd_Cmd_wren			<= 1'b0;
		end
	end
	always@(posedge clk)	TX_Start_2nd_one_r	<= TX_Start_2nd_one_o & reg_wren;

	// RadioReg_2nd_ReadDone bit & RadioReg_2nd_ValueReturn
	always@(posedge clk) begin
		if(rst) begin
			RadioReg_2nd_ReadDone <= 1'b0;
			RadioReg_2nd_ValueReturn[31:0] <= 32'hA0B0_C0D0;
		end else if (reg_wren) begin
						case(reg_wr_addr) 
							12'h2C4: begin
								if (reg_data_in[6]) begin
									RadioReg_2nd_ReadDone <= 1'b1;			// means this bit is write-only, any write from PC to this bit will clear this bit
									RadioReg_2nd_ValueReturn[31:0] <= 32'hFFFF_0000; // frequently set to default value
								end else begin
									RadioReg_2nd_ReadDone <= 1'b0;			// means this bit is write-only, any write from PC to this bit will clear this bit
									RadioReg_2nd_ValueReturn[31:0] <= 32'hA0B0_C0D0; // frequently set to default value
								end
							end
							default: begin									// write in other registers will be bypassed
								RadioReg_2nd_ReadDone <= RadioReg_2nd_ReadDone;
								RadioReg_2nd_ValueReturn[31:0] <= RadioReg_2nd_ValueReturn[31:0];
							end
						endcase
		end else if (set_RadioReg_2nd_ReadDone_bit_two) begin
						if (~RadioRegRead_2nd_Addr_in[7]) begin			// make sure it is a radio register read return event
							RadioReg_2nd_ReadDone <= 1'b1;
							RadioReg_2nd_ValueReturn[31:0] <= RadioRegRead_2nd_Value_in[31:0];
						end else begin
							RadioReg_2nd_ReadDone <= RadioReg_2nd_ReadDone;
							RadioReg_2nd_ValueReturn[31:0] <= RadioReg_2nd_ValueReturn[31:0];
						end
		end else begin
				RadioReg_2nd_ReadDone <= RadioReg_2nd_ReadDone;
				RadioReg_2nd_ValueReturn[31:0] <= RadioReg_2nd_ValueReturn[31:0];
		end
	end	
	always@(posedge clk) RadioReg_2nd_ReadDone_in_reg <= RadioReg_2nd_ReadDone_in;    // add one register for signal which crosses time domain
	// RadioReg_2nd_ReadDone_in crosses time domain (from FRL_clk to 125MHz), it's better to do edge detection
	rising_edge_detect RadioReg_2nd_ReadDone_one_inst(
						 .clk(clk),.rst(rst),.in(RadioReg_2nd_ReadDone_in_reg),.one_shot_out(set_RadioReg_2nd_ReadDone_bit));
	always@(posedge clk)	set_RadioReg_2nd_ReadDone_bit_r	<= set_RadioReg_2nd_ReadDone_bit;
	assign	set_RadioReg_2nd_ReadDone_bit_two = set_RadioReg_2nd_ReadDone_bit | set_RadioReg_2nd_ReadDone_bit_r;

`ifdef MIMO_4X4						 
/// registers for 3rd path/radio					  						 						 
// Generate radio cmd
	always@(posedge clk) begin
		if(rst) begin
			Radio_3rd_Cmd_Data[31:0]	<= 32'h0000_0000;
			Radio_3rd_Cmd_Addr[6:0] 	<= 7'b000_0000;
			Radio_3rd_Cmd_RdWr			<= 1'b0;
			Radio_3rd_Cmd_wren			<= 1'b0;
		end else if(reg_wren) begin
				case(reg_wr_addr[11:0])
					12'h024: begin							/// liuchang: SyncControl, add for sync
							Radio_3rd_Cmd_Data[31:0] 	<= reg_data_in[31:0];
							Radio_3rd_Cmd_Addr[6:0]  	<= 7'h18;
							Radio_3rd_Cmd_RdWr			<= 1'b0;
							Radio_3rd_Cmd_wren			<= 1'b1;		
					end	
					12'h3C4: begin
//						if (~reg_data_in[6]) begin		// protected space can not be accessed
							Radio_3rd_Cmd_Data[31:0]	<= RadioReg_3rd_WriteValue[31:0];
							Radio_3rd_Cmd_Addr[6:0]		<= reg_data_in[6:0];
							Radio_3rd_Cmd_RdWr			<= reg_data_in[7];
							Radio_3rd_Cmd_wren			<= 1'b1;
//						end else begin
//							Radio_3rd_Cmd_Data[31:0]	<= 32'h0000_0000;
//							Radio_3rd_Cmd_Addr[6:0] 	<= 7'b000_0000;
//							Radio_3rd_Cmd_RdWr			<= 1'b0;
//							Radio_3rd_Cmd_wren			<= 1'b0;
//						end
					end
//					// also pass TXControl and RXControl to Radio Module
//					12'h384: begin			// TXControl
//						Radio_3rd_Cmd_Data[31:0]	<= reg_data_in[31:0];
//						Radio_3rd_Cmd_Addr[6:0]		<= 7'h02;
//						Radio_3rd_Cmd_RdWr			<= 1'b0;
//						Radio_3rd_Cmd_wren			<= 1'b1;
//					end
					12'h364: begin			// RXControl					
						Radio_3rd_Cmd_Data[31:0]	<= reg_data_in[31:0];
//						Radio_3rd_Cmd_Addr[6:0]		<= 7'h03;
						Radio_3rd_Cmd_Addr[6:0]		<= 7'h41;
						Radio_3rd_Cmd_RdWr			<= 1'b0;
						Radio_3rd_Cmd_wren			<= 1'b1;
					end
					12'h3A0: begin			// TXTime	
						Radio_3rd_Cmd_Data[31:0]	<= reg_data_in[31:0];
						Radio_3rd_Cmd_Addr[6:0]		<= 7'h42;
						Radio_3rd_Cmd_RdWr			<= 1'b0;
						Radio_3rd_Cmd_wren			<= 1'b1;
					end
					12'h3A4: begin			// RadioDelayCompen	
						Radio_3rd_Cmd_Data[31:0]	<= reg_data_in[31:0];
						Radio_3rd_Cmd_Addr[6:0]		<= 7'h43;
						Radio_3rd_Cmd_RdWr			<= 1'b0;
						Radio_3rd_Cmd_wren			<= 1'b1;
					end

					default: begin
						Radio_3rd_Cmd_Data[31:0]	<= 32'h0000_0000;
						Radio_3rd_Cmd_Addr[6:0]		<= 7'b000_0000;
						Radio_3rd_Cmd_RdWr			<= 1'b0;
						Radio_3rd_Cmd_wren			<= 1'b0;
					end
				endcase
		end else if (TX_Start_3rd_one_o | TX_Start_3rd_one_r) begin
			Radio_3rd_Cmd_Data[31:0]	<= {12'h000,TXMask_3rd_radio[3:0],16'h0001};
//			Radio_3rd_Cmd_Addr[6:0]		<= 7'h40;
			Radio_3rd_Cmd_Addr[6:0]		<= 7'h30;
			Radio_3rd_Cmd_RdWr			<= 1'b0;
			Radio_3rd_Cmd_wren			<= 1'b1;
		end else begin
			Radio_3rd_Cmd_Data[31:0] <= 32'h0000_0000;
			Radio_3rd_Cmd_Addr[6:0]  <= 7'b000_0000;
			Radio_3rd_Cmd_RdWr			<= 1'b0;
			Radio_3rd_Cmd_wren			<= 1'b0;
		end
	end
	always@(posedge clk)	TX_Start_3rd_one_r	<= TX_Start_3rd_one_o & reg_wren;

	// RadioReg_3rd_ReadDone bit & RadioReg_3rd_ValueReturn
	always@(posedge clk) begin
		if(rst) begin
			RadioReg_3rd_ReadDone <= 1'b0;
			RadioReg_3rd_ValueReturn[31:0] <= 32'hA0B0_C0D0;
		end else if (reg_wren) begin
						case(reg_wr_addr) 
							12'h3C4: begin
								if (reg_data_in[6]) begin
									RadioReg_3rd_ReadDone <= 1'b1;			// means this bit is write-only, any write from PC to this bit will clear this bit
									RadioReg_3rd_ValueReturn[31:0] <= 32'hFFFF_0000; // frequently set to default value
								end else begin
									RadioReg_3rd_ReadDone <= 1'b0;			// means this bit is write-only, any write from PC to this bit will clear this bit
									RadioReg_3rd_ValueReturn[31:0] <= 32'hA0B0_C0D0; // frequently set to default value
								end
							end
							default: begin									// write in other registers will be bypassed
								RadioReg_3rd_ReadDone <= RadioReg_3rd_ReadDone;
								RadioReg_3rd_ValueReturn[31:0] <= RadioReg_3rd_ValueReturn[31:0];
							end
						endcase
		end else if (set_RadioReg_3rd_ReadDone_bit_two) begin
						if (~RadioRegRead_3rd_Addr_in[7]) begin			// make sure it is a radio register read return event
							RadioReg_3rd_ReadDone <= 1'b1;
							RadioReg_3rd_ValueReturn[31:0] <= RadioRegRead_3rd_Value_in[31:0];
						end else begin
							RadioReg_3rd_ReadDone <= RadioReg_3rd_ReadDone;
							RadioReg_3rd_ValueReturn[31:0] <= RadioReg_3rd_ValueReturn[31:0];
						end
		end else begin
				RadioReg_3rd_ReadDone <= RadioReg_3rd_ReadDone;
				RadioReg_3rd_ValueReturn[31:0] <= RadioReg_3rd_ValueReturn[31:0];
		end
	end
	
	always@(posedge clk) RadioReg_3rd_ReadDone_in_reg <= RadioReg_3rd_ReadDone_in;    // add one register for signal which crosses time domain
	// RadioReg_3rd_ReadDone_in crosses time domain (from FRL_clk to 125MHz), it's better to do edge detection
	rising_edge_detect RadioReg_3rd_ReadDone_one_inst(
						 .clk(clk),.rst(rst),.in(RadioReg_3rd_ReadDone_in_reg),.one_shot_out(set_RadioReg_3rd_ReadDone_bit));
	always@(posedge clk)	set_RadioReg_3rd_ReadDone_bit_r	<= set_RadioReg_3rd_ReadDone_bit;
	assign	set_RadioReg_3rd_ReadDone_bit_two = set_RadioReg_3rd_ReadDone_bit | set_RadioReg_3rd_ReadDone_bit_r;
						 
/// registers for 4th path/radio					  						 						 
// Generate radio cmd
	always@(posedge clk) begin
		if(rst) begin
			Radio_4th_Cmd_Data[31:0]	<= 32'h0000_0000;
			Radio_4th_Cmd_Addr[6:0] 	<= 7'b000_0000;
			Radio_4th_Cmd_RdWr			<= 1'b0;
			Radio_4th_Cmd_wren			<= 1'b0;
		end else if(reg_wren) begin
				case(reg_wr_addr[11:0])
					12'h024: begin							/// liuchang: SyncControl, add for sync, not really used in RAB
							Radio_4th_Cmd_Data[31:0] 	<= reg_data_in[31:0];
							Radio_4th_Cmd_Addr[6:0]  	<= 7'h18;
							Radio_4th_Cmd_RdWr			<= 1'b0;
							Radio_4th_Cmd_wren			<= 1'b1;		
					end
					12'h4C4: begin
//						if (~reg_data_in[6]) begin		// protected space can not be accessed
							Radio_4th_Cmd_Data[31:0]	<= RadioReg_4th_WriteValue[31:0];
							Radio_4th_Cmd_Addr[6:0]		<= reg_data_in[6:0];
							Radio_4th_Cmd_RdWr			<= reg_data_in[7];
							Radio_4th_Cmd_wren			<= 1'b1;
//						end else begin
//							Radio_4th_Cmd_Data[31:0]	<= 32'h0000_0000;
//							Radio_4th_Cmd_Addr[6:0] 	<= 7'b000_0000;
//							Radio_4th_Cmd_RdWr			<= 1'b0;
//							Radio_4th_Cmd_wren			<= 1'b0;
//						end
					end
//					// also pass TXControl and RXControl to Radio Module
//					12'h484: begin			// TXControl
//						Radio_4th_Cmd_Data[31:0]	<= reg_data_in[31:0];
//						Radio_4th_Cmd_Addr[6:0]		<= 7'h02;
//						Radio_4th_Cmd_RdWr			<= 1'b0;
//						Radio_4th_Cmd_wren			<= 1'b1;
//					end
					12'h464: begin			// RXControl					
						Radio_4th_Cmd_Data[31:0]	<= reg_data_in[31:0];
//						Radio_4th_Cmd_Addr[6:0]		<= 7'h03;
						Radio_4th_Cmd_Addr[6:0]		<= 7'h41;
						Radio_4th_Cmd_RdWr			<= 1'b0;
						Radio_4th_Cmd_wren			<= 1'b1;
					end
					12'h4A0: begin			// TXTime	
						Radio_4th_Cmd_Data[31:0]	<= reg_data_in[31:0];
						Radio_4th_Cmd_Addr[6:0]		<= 7'h42;
						Radio_4th_Cmd_RdWr			<= 1'b0;
						Radio_4th_Cmd_wren			<= 1'b1;
					end
					12'h4A4: begin			// RadioDelayCompen	
						Radio_4th_Cmd_Data[31:0]	<= reg_data_in[31:0];
						Radio_4th_Cmd_Addr[6:0]		<= 7'h43;
						Radio_4th_Cmd_RdWr			<= 1'b0;
						Radio_4th_Cmd_wren			<= 1'b1;
					end

					default: begin
						Radio_4th_Cmd_Data[31:0]	<= 32'h0000_0000;
						Radio_4th_Cmd_Addr[6:0]		<= 7'b000_0000;
						Radio_4th_Cmd_RdWr			<= 1'b0;
						Radio_4th_Cmd_wren			<= 1'b0;
					end
				endcase
		end else if (TX_Start_4th_one_o | TX_Start_4th_one_r) begin
			Radio_4th_Cmd_Data[31:0]	<= {12'h000,TXMask_4th_radio[3:0],16'h0001};
//			Radio_4th_Cmd_Addr[6:0]		<= 7'h40;
			Radio_4th_Cmd_Addr[6:0]		<= 7'h30;
			Radio_4th_Cmd_RdWr			<= 1'b0;
			Radio_4th_Cmd_wren			<= 1'b1;
		end else begin
			Radio_4th_Cmd_Data[31:0] <= 32'h0000_0000;
			Radio_4th_Cmd_Addr[6:0]  <= 7'b000_0000;
			Radio_4th_Cmd_RdWr			<= 1'b0;
			Radio_4th_Cmd_wren			<= 1'b0;
		end
	end
	always@(posedge clk)	TX_Start_4th_one_r	<= TX_Start_4th_one_o & reg_wren;

	// RadioReg_4th_ReadDone bit & RadioReg_4th_ValueReturn
	always@(posedge clk) begin
		if(rst) begin
			RadioReg_4th_ReadDone <= 1'b0;
			RadioReg_4th_ValueReturn[31:0] <= 32'hA0B0_C0D0;
		end else if (reg_wren) begin
						case(reg_wr_addr) 
							12'h4C4: begin
								if (reg_data_in[6]) begin
									RadioReg_4th_ReadDone <= 1'b1;			// means this bit is write-only, any write from PC to this bit will clear this bit
									RadioReg_4th_ValueReturn[31:0] <= 32'hFFFF_0000; // frequently set to default value
								end else begin
									RadioReg_4th_ReadDone <= 1'b0;			// means this bit is write-only, any write from PC to this bit will clear this bit
									RadioReg_4th_ValueReturn[31:0] <= 32'hA0B0_C0D0; // frequently set to default value
								end
							end
							default: begin									// write in other registers will be bypassed
								RadioReg_4th_ReadDone <= RadioReg_4th_ReadDone;
								RadioReg_4th_ValueReturn[31:0] <= RadioReg_4th_ValueReturn[31:0];
							end
						endcase
		end else if (set_RadioReg_4th_ReadDone_bit_two) begin
						if (~RadioRegRead_4th_Addr_in[7]) begin			// make sure it is a radio register read return event
							RadioReg_4th_ReadDone <= 1'b1;
							RadioReg_4th_ValueReturn[31:0] <= RadioRegRead_4th_Value_in[31:0];
						end else begin
							RadioReg_4th_ReadDone <= RadioReg_4th_ReadDone;
							RadioReg_4th_ValueReturn[31:0] <= RadioReg_4th_ValueReturn[31:0];
						end
		end else begin
				RadioReg_4th_ReadDone <= RadioReg_4th_ReadDone;
				RadioReg_4th_ValueReturn[31:0] <= RadioReg_4th_ValueReturn[31:0];
		end
	end
	
	always@(posedge clk) RadioReg_4th_ReadDone_in_reg <= RadioReg_4th_ReadDone_in;    // add one register for signal which crosses time domain
	// RadioReg_4th_ReadDone_in crosses time domain (from FRL_clk to 125MHz), it's better to do edge detection
	rising_edge_detect RadioReg_4th_ReadDone_one_inst(
						 .clk(clk),.rst(rst),.in(RadioReg_4th_ReadDone_in_reg),.one_shot_out(set_RadioReg_4th_ReadDone_bit));
	always@(posedge clk)	set_RadioReg_4th_ReadDone_bit_r	<= set_RadioReg_4th_ReadDone_bit;
	assign	set_RadioReg_4th_ReadDone_bit_two = set_RadioReg_4th_ReadDone_bit | set_RadioReg_4th_ReadDone_bit_r;
`endif //MIMO_4X4
		
`endif //RADIO_CHANNEL_REGISTERS

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
					  /// Jiansong: System Registers
	//              12'h04C: reg_data_out <= PCIeTXBurstSize;
	//              12'h050: reg_data_out <= PCIeRXBlockSize;
					12'h020: reg_data_out <= {27'h000_0000,DDR2MaxBurstSize[4:0]};
	
					  12'h038: reg_data_out <= FirmwareVersion;
					  12'h03C: reg_data_out <= HWStatus;
					  12'h040: reg_data_out <= {16'h0000,PCIeLinkStatus};
					  12'h044: reg_data_out <= {16'h0000,PCIeLinkControl};
					  12'h048: reg_data_out <= {31'h0000_0000,SoftReset};
					  12'h15C: reg_data_out <= {8'h00,round_trip_latency};
					  12'h160: reg_data_out <= {8'h00,transfer_duration};
					  /// Jiansong: DMA Registers
					  12'h168: reg_data_out <= {30'h0000_0000,TransferControl};
					  12'h188: reg_data_out <= TransferSrcAddr_L;
					  12'h18C: reg_data_out <= TransferSrcAddr_H;
	//              12'h16C: reg_data_out <= TransferMask;
					  12'h024: reg_data_out <= {31'h0000_0000,SyncControl};		/// liuchang
					  12'h028: reg_data_out <= SyncControl_cnt;						/// liuchang: for debug
	
					  12'h164: reg_data_out <= {31'h0000_0000,RXControl};
					  12'h170: reg_data_out <= RXBufAddr_L;
					  12'h174: reg_data_out <= RXBufAddr_H;
					  12'h194: reg_data_out <= RXBufSize;
					  /// Jiansong: Common Radio Registers
//					  12'h154: reg_data_out <= {31'h0000_0000,RadioStatus};
//					  12'h158: reg_data_out <= RadioID;
					  12'h184: reg_data_out <= {12'h000,TXMask[3:0],13'h0000,TXControl_Ongoing,TXControl_Done,TXControl_Start};
					  12'h178: reg_data_out <= TXAddr;
					  12'h190: reg_data_out <= TXSize;					
					  12'h1A0: reg_data_out <= TXTime;
					  12'h1A4: reg_data_out <= RadioDelayCompen;  					  
					  12'h1A8: reg_data_out <= TXTime_start2done;									/// liuchang
					  12'h1AC: reg_data_out <= TXTime_start_cnt;
					  12'h1B0: reg_data_out <= TXTime_done_cnt;
/// registers for 2nd to 4th paths/radios
					  12'h264: reg_data_out <= {31'h0000_0000,RXControl_2nd};
					  12'h270: reg_data_out <= RXBufAddr_2nd_L;
					  12'h274: reg_data_out <= RXBufAddr_2nd_H;
					  12'h294: reg_data_out <= RXBufSize_2nd;
					  12'h284: reg_data_out <= {12'h000,TXMask_2nd[3:0],13'h0000,TXControl_Ongoing_2nd,TXControl_Done_2nd,TXControl_Start_2nd};
					  12'h278: reg_data_out <= TXAddr_2nd;
					  12'h290: reg_data_out <= TXSize_2nd;
					  12'h2A0: reg_data_out <= TXTime_2nd;
					  12'h2A4: reg_data_out <= RadioDelayCompen_2nd;
					  12'h2A8: reg_data_out <= TXTime_start2done_2nd;								/// liuchang
					  12'h2AC: reg_data_out <= TXTime_start_2nd_cnt;
					  12'h2B0: reg_data_out <= TXTime_done_2nd_cnt;
`ifdef MIMO_4X4
					  12'h364: reg_data_out <= {31'h0000_0000,RXControl_3rd};
					  12'h370: reg_data_out <= RXBufAddr_3rd_L;
					  12'h374: reg_data_out <= RXBufAddr_3rd_H;
					  12'h394: reg_data_out <= RXBufSize_3rd;
					  12'h384: reg_data_out <= {12'h000,TXMask_3rd[3:0],13'h0000,TXControl_Ongoing_3rd,TXControl_Done_3rd,TXControl_Start_3rd};
					  12'h378: reg_data_out <= TXAddr_3rd;
					  12'h390: reg_data_out <= TXSize_3rd;
					  12'h3A0: reg_data_out <= TXTime_3rd;
					  12'h3A4: reg_data_out <= RadioDelayCompen_3rd;
					  12'h3A8: reg_data_out <= TXTime_start2done_3rd;								/// liuchang
					  12'h3AC: reg_data_out <= TXTime_start_3rd_cnt;
					  12'h3B0: reg_data_out <= TXTime_done_3rd_cnt;					  
					  12'h464: reg_data_out <= {31'h0000_0000,RXControl_4th};
					  12'h470: reg_data_out <= RXBufAddr_4th_L;
					  12'h474: reg_data_out <= RXBufAddr_4th_H;
					  12'h494: reg_data_out <= RXBufSize_4th;
					  12'h484: reg_data_out <= {12'h000,TXMask_4th[3:0],13'h0000,TXControl_Ongoing_4th,TXControl_Done_4th,TXControl_Start_4th};
					  12'h478: reg_data_out <= TXAddr_4th;
					  12'h490: reg_data_out <= TXSize_4th;
					  12'h4A0: reg_data_out <= TXTime_4th;
					  12'h4A4: reg_data_out <= RadioDelayCompen_4th;
					  12'h4A8: reg_data_out <= TXTime_start2done_4th;								/// liuchang
					  12'h4AC: reg_data_out <= TXTime_start_4th_cnt;
					  12'h4B0: reg_data_out <= TXTime_done_4th_cnt;					  
`endif //MIMO_4X4

//					  /// Jiansong: Debug registers
						12'h080: reg_data_out	<= DebugRX1Overflowcount[31:0];
						12'h084: reg_data_out	<= DebugRX2Overflowcount[31:0];
						12'h088: reg_data_out	<= DebugDDREgressFIFOCnt[31:0];
						12'h08C: reg_data_out	<= DebugDDRFIFOFullCnt[31:0];
						12'h090: reg_data_out	<= {23'h0000_00,DebugDDRSMs[8:0]};
						12'h094: reg_data_out	<= DebugDDRSignals[31:0];
//					  12'h080: reg_data_out <= Debug1;
//					  12'h084: reg_data_out <= Debug2;
//					  12'h088: reg_data_out <= Debug3;
//					  12'h08C: reg_data_out <= Debug4;
////					  12'h090: reg_data_out <= Debug5SourceAddr_L;
////					  12'h094: reg_data_out <= Debug6SourceAddr_H;
////					  12'h098: reg_data_out <= Debug7DestAddr;
////					  12'h09C: reg_data_out <= Debug8FrameSize;
//					  12'h0A0: reg_data_out <= Debug9;
//					  12'h0A4: reg_data_out <= Debug10;
////					  12'h0A8: reg_data_out <= Debug11TF1;
////					  12'h0AC: reg_data_out <= Debug12TF2;
////					  12'h0B0: reg_data_out <= Debug13TF3;
////					  12'h0B4: reg_data_out <= Debug14TF4;
////					  12'h0B8: reg_data_out <= Debug15TX1;
////					  12'h0BC: reg_data_out <= Debug16TX2;
////					  12'h0C0: reg_data_out <= Debug17TX3;
//					  12'h0C4: reg_data_out <= Debug18DDR1;
//					  12'h0C8: reg_data_out <= Debug19DDR2;
//					  12'h0CC: reg_data_out <= Debug20RX1;
////					  12'h0D0: reg_data_out <= Debug21RX2;
////					  12'h0D4: reg_data_out <= {27'h0000_000,Debug22RX3};
////					  12'h0D8: reg_data_out <= Debug23RX4;
//					  12'h0DC: reg_data_out <= Debug24RX5;
////					  12'h0E0: reg_data_out <= Debug25RX6;
//					  12'h0E4: reg_data_out <= Debug26RX7;
//					  12'h0E8: reg_data_out <= Debug27RX8;
//					  12'h0EC: reg_data_out <= Debug28RX9;
//					  12'h0F0: reg_data_out <= Debug29RX10;
//					  12'h0F4: reg_data_out <= {22'h00_0000,Debug30RXEngine};
//					  12'h0F8: reg_data_out <= {20'h00000,Debug31RXDataFIFOfullcnt};
//					  12'h0FC: reg_data_out <= {20'h00000,Debug32RXXferFIFOfullcnt};
//					  12'hF00: reg_data_out <= {8'h00,Debug33RXDataFIFOWRcnt};
//					  12'hF04: reg_data_out <= {8'h00,Debug34RXDataFIFORDcnt};
//					  12'hF08: reg_data_out <= {8'h00,Debug35RXXferFIFOWRcnt};
//					  12'hF0C: reg_data_out <= {8'h00,Debug36RXXferFIFORDcnt};
//					  12'hF10: reg_data_out <= Debug37completion_pending;
//					  12'hF14: reg_data_out <= {27'h0000_000,Debug38tag_value};
					 
`ifdef RADIO_CHANNEL_REGISTERS	
					  12'h1C0: reg_data_out <= RadioReg_WriteValue;
					  12'h1C4: reg_data_out <= {23'h00_0000, RadioReg_ReadDone, RadioReg_RW, RadioReg_Addr[6:0]};
					  12'h1C8: reg_data_out <= RadioReg_ValueReturn;
					  
/// registers for 2nd to 4th paths/radios					  
					  12'h2C0: reg_data_out <= RadioReg_2nd_WriteValue;
					  12'h2C4: reg_data_out <= {23'h00_0000, RadioReg_2nd_ReadDone, RadioReg_2nd_RW, RadioReg_2nd_Addr[6:0]};
					  12'h2C8: reg_data_out <= RadioReg_2nd_ValueReturn;
`ifdef MIMO_4X4
					  12'h3C0: reg_data_out <= RadioReg_3rd_WriteValue;
					  12'h3C4: reg_data_out <= {23'h00_0000, RadioReg_3rd_ReadDone, RadioReg_3rd_RW, RadioReg_3rd_Addr[6:0]};
					  12'h3C8: reg_data_out <= RadioReg_3rd_ValueReturn;
					  12'h4C0: reg_data_out <= RadioReg_4th_WriteValue;
					  12'h4C4: reg_data_out <= {23'h00_0000, RadioReg_4th_ReadDone, RadioReg_4th_RW, RadioReg_4th_Addr[6:0]};
					  12'h4C8: reg_data_out <= RadioReg_4th_ValueReturn;
`endif //MIMO_4X4

`endif //RADIO_CHANNEL_REGISTERS
					  default: reg_data_out <= 32'hF5F5F5F5;
					endcase
			  end
		end
	//////////////////////////////////////////////////////////////////////////////
	//END of Register File Code
	//////////////////////////////////////////////////////////////////////////////


endmodule
