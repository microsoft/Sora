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

///////////////////////////////////////////////////////////////////////////////
// ?2007-2008 Xilinx, Inc. All Rights Reserved.
// Confidential and proprietary information of Xilinx, Inc.
//////////////////////////////////////////////////////////////////////////////////
// Company: Microsoft Research Asia
// Engineer: Jiansong Zhang
// 
// Create Date:    21:39:39 06/01/2009 
// Design Name: 
// Module Name:    firmware_ctrl_wrapper 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: DMA Control and Status Register File wrapper.  
//          Connects the internal_dma_ctrl (which allows transfers
//          sizes up to 4KB), to the dma_ctrl_status_reg_file module (which 
//          increases the transfer size by queueing mulitple smaller
//          transfers).
//          If the user does not require transfer sizes larger than 4KB,
//          or wishes to control large transfer sizes through software,
//          the dma_ctrl_status_reg_file module may be removed.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//  Jiansong:
//     TX flow control: implemented in dma control wrapper (register file)
//     (1) control signal: TX_fetch_next_4k, one cycle
//          it's the falling edge of egress_data_fifo almost full
//          egress_data_fifo almostfull is slightly less than half of the FIFO size
//     (2) TX has higher priority than transfer, or mrd > mwr
//      
//  modified by zjs:
//     (1) init TX desciptor request --------------------------- done
//     (2) dma read after TX descriptor ------------------------ done
//     (3) dma write TX descriptor after dma read -------------- done
//     (4) remove previous dma write logic --------------------- done
//     (5) control on RX path ---------------------------------- done
//     (5) register read
//     (6) register write
//         register write width, from 7 bits to 12 bits -------- done
//     (7) TX start (divide into small blocks) ----------------- done
//     (8) TX done --------------------------------------------- done
//     (9) radio registers
//     (10) relax 1MB dma read size limitation to 4GB ---------- done
//
//     (11) transfer recovering:
//            If RCB is not mounted to PC stably, some PCIe packets may be lost on PCIe
//            physical layer. In this case, transfer (dma read) may dead-lock at a waiting 
//            for packet state. Transfer recovering is to recover from this state. In this
//            design, mechanism of transfer recovering is aggressive, that if tranferstart
//            bit in transfercontrol register is deasserted, (1) all dma read related state 
//            machine will return to IDLE state, (2) all calculation in dma_ctrl_wrapper, 
//            rx trn monitor and nonposted_pkt_gen will be cleared to zero, (3) read_req_wrapper
//            will be cleared, all WRs will be bypassed, (4) pkt already in non_posted_header_fifo
//            will still be sent out, completer received in rx_engine will still be write 
//            in to ddr. THIS MECHANISM REQUIRES DRIVER DELAY SEVERAL MICROSECONDS BETWEEN 
//            TRANSFERSTART'S DEASSERTING AND ASSERTING, TO WAIT FOR ALL THE PENDING COMPLETERS
//            PROCESSED.  
//
//	(12) performance counter
//				measure two durations:  (1) from TX_des request sent to tx_engine to new des 
//              received (2) from transfer start to transfer done.
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps
`include "Sora_config.v"

module firmware_ctrl_wrapper(
	 input clk,             
//	 input hard_rst,
	 input rst, 
	 // hot reset to whole system
	 output hostreset,
	 
	 output [4:0]	DDR2MaxBurstSize,
	 
	 /// Jiansong: interface to radio module
//	 input  			Radio_TX_done,
//	 output 			Radio_TX_start,
	 output			TX_Ongoing_o,

	 output 			TX_Start_one_o,
	 output [31:0]	TX_Addr,
	 output [31:0]	TX_Size,
	 output 			TX_Start_2nd_one_o,
	 output [31:0]	TX_Addr_2nd,
	 output [31:0]	TX_Size_2nd,
`ifdef MIMO_4X4
	 output 			TX_Start_3rd_one_o,
	 output [31:0]	TX_Addr_3rd,
	 output [31:0]	TX_Size_3rd,
	 output 			TX_Start_4th_one_o,
	 output [31:0]	TX_Addr_4th,
	 output [31:0]	TX_Size_4th,
`endif //MIMO_4X4
	 //interface from RX Engine 
	 input [31:0] reg_data_in, 
	//// input [6:0] reg_wr_addr,
	 input [11:0] reg_wr_addr, 
	 input reg_wren, 
	 ///Jiansong: interface from RX engine, TX desc recived
	 input                new_des_one,
	 input        [31:0]  SourceAddr_L,
	 input        [31:0]  SourceAddr_H,
	 input        [31:0]  DestAddr,
	 input        [23:0]  FrameSize,
	 input        [7:0]   FrameControl,
    ///Jiansong: interface to RX engine, indicate the system is in dma read for TX desc
	 ///          when this signal is asserted, received cpld will not be count in 
	 ///          length subtraction
    output     Wait_for_TX_desc,
	 output     transferstart_o,
	 output		transferstart_one,
	 input		set_transfer_done_bit,
	 output [63:0]	TransferSrcAddr,
	 /// Jiansong: interface to/from tx engine
	 input [11:0] reg_rd_addr,
	 output [31:0] reg_data_out,
	 //interface to TX Engine
	 output [63:0] dmaras,
	 output [31:0] dmarad,
	 output [31:0] dmarxs,
	 output read_last,
	 output rd_dma_start,
	 input rd_dma_done, //from RX Engine
  
 	 /// Jiansong: interface to/from posted_packet_generator
	 /// TX desc write back
	 output         TX_desc_write_back_req,
	 input          TX_desc_write_back_ack,
	 output [63:0]  SourceAddr_r,
    output [31:0]  DestAddr_r,
    output [23:0]  FrameSize_r,
    output [7:0]   FrameControl_r,
	 output [63:0]  DescAddr_r,
	 /// Jiansong: RX path
	 output         RXEnable,
	 output [63:0]  RXBufAddr,
	 output [31:0]  RXBufSize,
	 output         RXEnable_2nd,
	 output [63:0]  RXBufAddr_2nd,
	 output [31:0]  RXBufSize_2nd,
`ifdef MIMO_4X4
	 output         RXEnable_3rd,
	 output [63:0]  RXBufAddr_3rd,
	 output [31:0]  RXBufSize_3rd,
	 output         RXEnable_4th,
	 output [63:0]  RXBufAddr_4th,
	 output [31:0]  RXBufSize_4th,
`endif //MIMO_4X4
  
	 /// Jiansong: error inputs
	 input egress_overflow_one,
	 input RX_FIFO_full,
	 input [31:0] egress_rd_data_count,
	 input [31:0] egress_wr_data_count,
 
	 //interface from performance counters
	// input [31:0] dma_wr_count,
	// input [31:0] dma_rd_count,
	 //interface from memory controller
	 input phy_init_done,
	 // hardware status input
	 input trn_lnk_up_n_c,
	 //interface from dma_ddr2_if
	 input pause_read_requests,

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
	 output			Radio_2nd_Cmd_RdWr,
	 output			Radio_2nd_Cmd_wren,
	 input [31:0]	Channel_2nd_Reg_Read_Value,
	 input [7:0]	Channel_2nd_Reg_Read_Addr,
	 input			Channel_2nd_ReadDone_in,	 			  
`ifdef MIMO_4X4
	 output [31:0] Radio_3rd_Cmd_Data,	
	 output [6:0]  Radio_3rd_Cmd_Addr,
	 output			Radio_3rd_Cmd_RdWr,
	 output			Radio_3rd_Cmd_wren,
	 input [31:0]	Channel_3rd_Reg_Read_Value,
	 input [7:0]	Channel_3rd_Reg_Read_Addr,
	 input			Channel_3rd_ReadDone_in,		 			  
	 output [31:0] Radio_4th_Cmd_Data,	
	 output [6:0]  Radio_4th_Cmd_Addr,
	 output			Radio_4th_Cmd_RdWr,
	 output			Radio_4th_Cmd_wren,
	 input [31:0]	Channel_4th_Reg_Read_Value,
	 input [7:0]	Channel_4th_Reg_Read_Addr,
	 input			Channel_4th_ReadDone_in,	
`endif //MIMO_4X4
	 
`endif //RADIO_CHANNEL_REGISTERS
	 // debug signals
	 input [31:0]	DebugRX1Overflowcount_in,
	 input [31:0]	DebugRX2Overflowcount_in,
	 
	 input [31:0]		DebugDDREgressFIFOCnt,
	 input [31:0]		DebugDDRFIFOFullCnt,
	 input [31:0]		DebugDDRSignals,
	 input [8:0]		DebugDDRSMs,
	 
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
//    input [4:0]  locked_debug	 
); 

	
	// internal signal for performance counter
//	wire		transferstart_one; // one cycle signal valid when transferstart bit is set by driver
	wire [23:0] round_trip_latency; /// 24 bits performance value
	wire [23:0] transfer_duration; /// 24 bits performance value
	
	// performance counter
	  performance_counter performance_counter_inst (
	    .clk		(clk),
		 .rst		(rst),
		 .transferstart_one		(transferstart_one),
		 .rd_dma_done_one			(rd_dma_done),
		 .new_des_one				(new_des_one),
		 .round_trip_latency		(round_trip_latency),
		 .transfer_duration		(transfer_duration)
	  );
	
  // Instantiate the DMA Control and Status Register File logic
  // This module interfaces with the 
//     dma_ctrl_status_reg_file dma_ctrl_status_reg_file_inst ( 
	register_table register_table_inst (

      .clk(clk), 
//      .hard_rst(hard_rst),
		.rst(rst),
      // hot reset to whole system
	   .hostreset(hostreset),
		
		.DDR2MaxBurstSize(DDR2MaxBurstSize[4:0]),
		
		/// Jiansong: performance value input		
		.round_trip_latency_i(round_trip_latency), /// 24 bits performance value
		.transfer_duration_i(transfer_duration), /// 24 bits performance value
	
		/// Jiansong: interface to radio module
//		.Radio_TX_done(Radio_TX_done),
//	   .Radio_TX_start(Radio_TX_start),
		.TX_Ongoing_o(TX_Ongoing_o),

	   .TX_Start_one_o		(TX_Start_one_o),
		.TX_Addr					(TX_Addr[31:0]),
		.TX_Size					(TX_Size[31:0]),
	   .TX_Start_2nd_one_o	(TX_Start_2nd_one_o),
		.TX_Addr_2nd			(TX_Addr_2nd[31:0]),
		.TX_Size_2nd			(TX_Size_2nd[31:0]),
`ifdef MIMO_4X4
	   .TX_Start_3rd_one_o	(TX_Start_3rd_one_o),
		.TX_Addr_3rd			(TX_Addr_3rd[31:0]),
		.TX_Size_3rd			(TX_Size_3rd[31:0]),
	   .TX_Start_4th_one_o	(TX_Start_4th_one_o),
		.TX_Addr_4th				(TX_Addr_4th[31:0]),
		.TX_Size_4th			(TX_Size_4th[31:0]),
`endif //MIMO_4X4

       //interface from RX Engine
      .reg_data_in(reg_data_in[31:0]), 
////      .reg_wr_addr(reg_wr_addr[6:0]),
      .reg_wr_addr(reg_wr_addr[11:0]), 
      .reg_wren(reg_wren),
      
		.transferstart_o			(transferstart_o),
		.transferstart_one		(transferstart_one),
		.set_transfer_done_bit	(set_transfer_done_bit),
		.TransferSrcAddr			(TransferSrcAddr[63:0]),
		
		 /// Jiansong: RX path
		 .RXEnable_o             (RXEnable),
		 .RXBufAddr_o            (RXBufAddr),
		 .RXBufSize_o            (RXBufSize),
		 .RXEnable_2nd_o         (RXEnable_2nd),
		 .RXBufAddr_2nd_o        (RXBufAddr_2nd),
		 .RXBufSize_2nd_o        (RXBufSize_2nd),
`ifdef MIMO_4X4
		 .RXEnable_3rd_o         (RXEnable_3rd),
		 .RXBufAddr_3rd_o        (RXBufAddr_3rd),
		 .RXBufSize_3rd_o        (RXBufSize_3rd),
		 .RXEnable_4th_o         (RXEnable_4th),
		 .RXBufAddr_4th_o        (RXBufAddr_4th),
		 .RXBufSize_4th_o        (RXBufSize_4th),
`endif //MIMO_4X4
		
      //register read interface
      .reg_rd_addr(reg_rd_addr[11:0]),
      .reg_data_out(reg_data_out[31:0]),  
	 
	   /// Jiansong: error inputs
		.egress_overflow_one(egress_overflow_one),
		.RX_FIFO_full(RX_FIFO_full),
      .egress_rd_data_count(egress_rd_data_count),
      .egress_wr_data_count(egress_wr_data_count),
      .pause_read_requests(pause_read_requests),
	 
      //interface to/from Perfomance Counter modules
//      .dma_wr_count(dma_wr_count),
//      .dma_rd_count(dma_rd_count),

      //interface from memory controller
      .phy_init_done(phy_init_done),
		//hardware status input
		.trn_lnk_up_n_c(trn_lnk_up_n_c),
		
		// radio related inputs/outputs
`ifdef RADIO_CHANNEL_REGISTERS

		.Radio_Cmd_Data			(Radio_Cmd_Data[31:0]),
		.Radio_Cmd_Addr			(Radio_Cmd_Addr[6:0]),
		.Radio_Cmd_RdWr			(Radio_Cmd_RdWr),
		.Radio_Cmd_wren			(Radio_Cmd_wren),
		.RadioRegRead_Value_in	(Channel_Reg_Read_Value[31:0]),
		.RadioRegRead_Addr_in	(Channel_Reg_Read_Addr[7:0]),
		.RadioReg_ReadDone_in	(Channel_ReadDone_in),
/// registers for 2nd to 4th paths/radios
		.Radio_2nd_Cmd_Data			(Radio_2nd_Cmd_Data[31:0]),
		.Radio_2nd_Cmd_Addr			(Radio_2nd_Cmd_Addr[6:0]),
		.Radio_2nd_Cmd_RdWr			(Radio_2nd_Cmd_RdWr),
		.Radio_2nd_Cmd_wren			(Radio_2nd_Cmd_wren),
		.RadioRegRead_2nd_Value_in	(Channel_2nd_Reg_Read_Value[31:0]),
		.RadioRegRead_2nd_Addr_in	(Channel_2nd_Reg_Read_Addr[7:0]),
		.RadioReg_2nd_ReadDone_in	(Channel_2nd_ReadDone_in),
`ifdef MIMO_4X4
		.Radio_3rd_Cmd_Data			(Radio_3rd_Cmd_Data[31:0]),
		.Radio_3rd_Cmd_Addr			(Radio_3rd_Cmd_Addr[6:0]),
		.Radio_3rd_Cmd_RdWr			(Radio_3rd_Cmd_RdWr),
		.Radio_3rd_Cmd_wren			(Radio_3rd_Cmd_wren),
		.RadioRegRead_3rd_Value_in	(Channel_3rd_Reg_Read_Value[31:0]),
		.RadioRegRead_3rd_Addr_in	(Channel_3rd_Reg_Read_Addr[7:0]),
		.RadioReg_3rd_ReadDone_in	(Channel_3rd_ReadDone_in),
		.Radio_4th_Cmd_Data			(Radio_4th_Cmd_Data[31:0]),
		.Radio_4th_Cmd_Addr			(Radio_4th_Cmd_Addr[6:0]),
		.Radio_4th_Cmd_RdWr			(Radio_4th_Cmd_RdWr),
		.Radio_4th_Cmd_wren			(Radio_4th_Cmd_wren),
		.RadioRegRead_4th_Value_in	(Channel_4th_Reg_Read_Value[31:0]),
		.RadioRegRead_4th_Addr_in	(Channel_4th_Reg_Read_Addr[7:0]),
		.RadioReg_4th_ReadDone_in	(Channel_4th_ReadDone_in),
`endif //MIMO_4X4

`endif //RADIO_CHANNEL_REGISTERS
		.DebugRX1Overflowcount_in	(DebugRX1Overflowcount_in),
		.DebugRX2Overflowcount_in	(DebugRX2Overflowcount_in),
		.PCIeLinkStatus_in(PCIeLinkStatus_in),
      .PCIeLinkControl_in(PCIeLinkControl_in),
		
		.DebugDDREgressFIFOCnt_in	(DebugDDREgressFIFOCnt[31:0]),
		.DebugDDRFIFOFullCnt_in	(DebugDDRFIFOFullCnt[31:0]),
		.DebugDDRSignals_in	(DebugDDRSignals[31:0]),
		.DebugDDRSMs_in		(DebugDDRSMs[8:0])		
		// debug inputs
//		.Debug18DDR1_in(Debug18DDR1_in),
//		.Debug19DDR2_in(Debug19DDR2_in),
//		.Debug20RX1_in(Debug20RX1_in),
//		.Debug21RX2_in(Debug21RX2_in),
////		.Debug22RX3_in(Debug22RX3_in),
//		.Debug23RX4_in(Debug23RX4_in),
//		.Debug24RX5_in(Debug24RX5_in),
//		.Debug25RX6_in(Debug25RX6_in),
//		.Debug26RX7_in(Debug26RX7_in),
//		.Debug27RX8_in(Debug27RX8_in),
//		.Debug28RX9_in(Debug28RX9_in),
//		.Debug29RX10_in(Debug29RX10_in),
//		.Debug30RXEngine_in(Debug30RXEngine_in),
//      .Debug31RXDataFIFOfullcnt_in(Debug31RXDataFIFOfullcnt_in),
//      .Debug32RXXferFIFOfullcnt_in(Debug32RXXferFIFOfullcnt_in),
//      .Debug33RXDataFIFOWRcnt_in(Debug33RXDataFIFOWRcnt_in),
//      .Debug34RXDataFIFORDcnt_in(Debug34RXDataFIFORDcnt_in),
//      .Debug35RXXferFIFOWRcnt_in(Debug35RXXferFIFOWRcnt_in),
//      .Debug36RXXferFIFORDcnt_in(Debug36RXXferFIFORDcnt_in),
//		.Debug37completion_pending_in(Debug37completion_pending_in),
//		.Debug38tag_value_in(Debug38tag_value_in),
//		.FIFOErrors_in(FIFOErrors_in),
//		.locked_debug(locked_debug)
    );

endmodule
