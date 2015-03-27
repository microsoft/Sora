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
// Module Name:    Sora_RCB 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description: Sora RCB top level wrapper. This code could be used on either single radio Sora or 4x4 MIMO, without any modification
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
`include "Sora_config.v"  // set verilog include directories in ise synthesize property
`include "PCIe_setting.v"

//-------------------------------------------------------
// Sora_RCB top Module
//-------------------------------------------------------

module    Sora_RCB
#(
   parameter   DDR_SIM = 0 
)   
(
    //-------------------------------------------------------
    // 1. PCI Express Fabric Interface
    //-------------------------------------------------------

    // Tx
    output wire   [(`PCI_EXP_LINK_WIDTH - 1):0]       pci_exp_txp,
    output wire   [(`PCI_EXP_LINK_WIDTH - 1):0]       pci_exp_txn,

    // Rx
    input wire    [(`PCI_EXP_LINK_WIDTH - 1):0]       pci_exp_rxp,
    input wire    [(`PCI_EXP_LINK_WIDTH - 1):0]       pci_exp_rxn,


    //-------------------------------------------------------
    // 4. System (SYS) Interface
    //-------------------------------------------------------

    input wire                                        pcie_sys_clk_p,
    input wire                                        pcie_sys_clk_n,
    input wire                                        sys_reset_n,
    
		input				V5_reset_n,

    //-------------------------------------------------------
    // 6. DDR2 Interface
    //-------------------------------------------------------

    input wire                                         SYS_CLK_P,
    input wire                                         SYS_CLK_N,
	 
    input wire                                         CLK200_P,
    input wire                                         CLK200_N,
    inout wire [63:0]                                  DDR2_DQ,    
    output wire [12:0]                                 DDR2_A,         
    output wire [1:0]                                  DDR2_BA,       
    output wire                                        DDR2_RAS_N,
    output wire                                        DDR2_CAS_N,
    output wire                                        DDR2_WE_N,

    output wire                                        DDR2_RESET_N,

    output wire                                        DDR2_CS_N,
    output wire                                        DDR2_ODT, 
    output wire                                        DDR2_CKE,
    output wire [7:0]                                  DDR2_DM,
    inout  wire [7:0]                                  DDR2_DQS,  
    inout  wire [7:0]                                  DDR2_DQS_N, 
    output wire [1:0]                                  DDR2_CK,               
    output wire [1:0]                                  DDR2_CK_N,
	 
	 //-------------------------------------------------------
    // 7. Radio Interface
    //-------------------------------------------------------

	 output        Sora_FRL_CLK_O_p, 
	 output        Sora_FRL_CLK_O_n, 
	 input         Sora_FRL_CLK_I_p, 
	 input         Sora_FRL_CLK_I_n,								
	 output        Sora_FRL_MSG_OUT_p,
	 output        Sora_FRL_MSG_OUT_n,
	 input         Sora_FRL_MSG_IN_p,
	 input         Sora_FRL_MSG_IN_n,
	 input [3:0]   Sora_FRL_DATA_IN_p,
	 input [3:0]   Sora_FRL_DATA_IN_n, 
	 output [3:0]  Sora_FRL_DATA_OUT_p, 
	 output [3:0]  Sora_FRL_DATA_OUT_n,
	 input         Sora_FRL_STATUS_IN_p,
	 input         Sora_FRL_STATUS_IN_n,
	 output        Sora_FRL_STATUS_OUT_p,
	 output        Sora_FRL_STATUS_OUT_n,

	 output        Sora_FRL_2nd_CLK_O_p, 
	 output        Sora_FRL_2nd_CLK_O_n, 
	 input         Sora_FRL_2nd_CLK_I_p, 
	 input         Sora_FRL_2nd_CLK_I_n,								
	 output        Sora_FRL_2nd_MSG_OUT_p,
	 output        Sora_FRL_2nd_MSG_OUT_n,
	 input         Sora_FRL_2nd_MSG_IN_p,
	 input         Sora_FRL_2nd_MSG_IN_n,
	 input [3:0]   Sora_FRL_2nd_DATA_IN_p,
	 input [3:0]   Sora_FRL_2nd_DATA_IN_n, 
	 output [3:0]  Sora_FRL_2nd_DATA_OUT_p, 
	 output [3:0]  Sora_FRL_2nd_DATA_OUT_n,
	 input         Sora_FRL_2nd_STATUS_IN_p,
	 input         Sora_FRL_2nd_STATUS_IN_n,
	 output        Sora_FRL_2nd_STATUS_OUT_p,
	 output        Sora_FRL_2nd_STATUS_OUT_n,

`ifdef MIMO_4X4
	 output        Sora_FRL_3rd_CLK_O_p, 
	 output        Sora_FRL_3rd_CLK_O_n, 
	 input         Sora_FRL_3rd_CLK_I_p, 
	 input         Sora_FRL_3rd_CLK_I_n,								
	 output        Sora_FRL_3rd_MSG_OUT_p,
	 output        Sora_FRL_3rd_MSG_OUT_n,
	 input         Sora_FRL_3rd_MSG_IN_p,
	 input         Sora_FRL_3rd_MSG_IN_n,
	 input [3:0]   Sora_FRL_3rd_DATA_IN_p,
	 input [3:0]   Sora_FRL_3rd_DATA_IN_n, 
	 output [3:0]  Sora_FRL_3rd_DATA_OUT_p, 
	 output [3:0]  Sora_FRL_3rd_DATA_OUT_n,
	 input         Sora_FRL_3rd_STATUS_IN_p,
	 input         Sora_FRL_3rd_STATUS_IN_n,
	 output        Sora_FRL_3rd_STATUS_OUT_p,
	 output        Sora_FRL_3rd_STATUS_OUT_n,

	 output        Sora_FRL_4th_CLK_O_p, 
	 output        Sora_FRL_4th_CLK_O_n, 
	 input         Sora_FRL_4th_CLK_I_p, 
	 input         Sora_FRL_4th_CLK_I_n,								
	 output        Sora_FRL_4th_MSG_OUT_p,
	 output        Sora_FRL_4th_MSG_OUT_n,
	 input         Sora_FRL_4th_MSG_IN_p,
	 input         Sora_FRL_4th_MSG_IN_n,
	 input [3:0]   Sora_FRL_4th_DATA_IN_p,
	 input [3:0]   Sora_FRL_4th_DATA_IN_n, 
	 output [3:0]  Sora_FRL_4th_DATA_OUT_p, 
	 output [3:0]  Sora_FRL_4th_DATA_OUT_n,
	 input         Sora_FRL_4th_STATUS_IN_p,
	 input         Sora_FRL_4th_STATUS_IN_n,
	 output        Sora_FRL_4th_STATUS_OUT_p,
	 output        Sora_FRL_4th_STATUS_OUT_n,
`endif //MIMO_4X4
	 
    //-------------------------------------------------------
    // 7. Debug Interface
    //-------------------------------------------------------  

	// the signals are not connected to FPGA pins, putting here to prevent the signals been removed by synthesize tool
	 output [31:0]	TXMSG_MISS_cnt,
	 output [31:0]	TXMSG_PASS_cnt,

	// RCB LEDs
	output wire				LED_link_up_and_phy_init_initialization_done_n,
	output wire				Sora_FRL_done_n,
	output wire				Radio_TX_n,
	output wire				Radio_RX_blink,
	 
	// frontpanel leds
	output wire		Front_PCIe_DDR_up,
	output wire		Front_Sora_FRL_up,
	output wire		Front_Radio_TX,
	output wire		Front_Radio_RX_blink,
	 
	output wire                            nPLOAD_1,
	output wire                            nPLOAD_2

	);
                      


    
    //-------------------------------------------------------
    // Local Wires
    //-------------------------------------------------------

    wire                                              sys_clk_c;
    wire                                              sys_reset_n_c;
    wire                                              trn_clk_c; 

    wire                                    clk_0_from_mem_ctrl_dcm;
	 
    wire [12:0]   pcie_id;
    reg           trn_reset_c;

    reg [12:0] pcie_id_reg;
    reg [2:0] max_pay_size_reg;
    reg [2:0] max_read_req_reg;

    wire pause_read_requests;

	// reset signals
	wire        hostreset;
	wire			hard_reset_n;
	wire			soft_rst;
	wire			rst_from_frl;
	wire			rst_from_frl_2nd;
	wire			rst_from_frl_3rd;
	wire			rst_from_frl_4th;
//	 reg         sys_reset;

	wire			SYS_RST_FROM_MEM_CTRL;

    //XST treats the PCIe Block Plus as a black-box and will insert
    //a BUFG on the user clock (trn_clk_c) unless buffer_type is none

    //synthesis attribute buffer_type of trn_clk_c is "none"
    //synthesis attribute max_fanout of trn_clk_c is "100000" 

    wire                                              trn_reset_n_c;
    wire                                              trn_lnk_up_n_c;
    wire                                              cfg_trn_pending_n_c;
    wire [(`PCI_EXP_CFG_DSN_WIDTH - 1):0]             cfg_dsn_n_c;
    wire                                              trn_tsof_n_c;
    wire                                              trn_teof_n_c;
    wire                                              trn_tsrc_rdy_n_c;
    wire                                              trn_tdst_rdy_n_c;
    wire                                              trn_tsrc_dsc_n_c;
    wire                                              trn_terrfwd_n_c;
    wire                                              trn_tdst_dsc_n_c;
    wire    [(`PCI_EXP_TRN_DATA_WIDTH - 1):0]         trn_td_c;
    wire    [(`PCI_EXP_TRN_REM_WIDTH - 1):0]          trn_trem_n_c;

    wire    [(`PCI_EXP_TRN_BUF_AV_WIDTH - 1):0]       trn_tbuf_av_c;

    wire                                              trn_rsof_n_c;
    wire                                              trn_reof_n_c;
    wire                                              trn_rsrc_rdy_n_c;
    wire                                              trn_rsrc_dsc_n_c;
    wire                                              trn_rdst_rdy_n_c;
    wire                                              trn_rerrfwd_n_c;
    wire                                              trn_rnp_ok_n_c;
    wire    [(`PCI_EXP_TRN_DATA_WIDTH - 1):0]         trn_rd_c;
    wire    [(`PCI_EXP_TRN_REM_WIDTH - 1):0]          trn_rrem_n_c;

    wire    [(`PCI_EXP_TRN_BAR_HIT_WIDTH - 1):0]      trn_rbar_hit_n_c;
    wire    [(`PCI_EXP_TRN_FC_HDR_WIDTH - 1):0]       trn_rfc_nph_av_c;
    wire    [(`PCI_EXP_TRN_FC_DATA_WIDTH - 1):0]      trn_rfc_npd_av_c;
    wire    [(`PCI_EXP_TRN_FC_HDR_WIDTH - 1):0]       trn_rfc_ph_av_c;
    wire    [(`PCI_EXP_TRN_FC_DATA_WIDTH - 1):0]      trn_rfc_pd_av_c;
    wire    [(`PCI_EXP_TRN_FC_HDR_WIDTH - 1):0]       trn_rfc_cplh_av_c;
    wire    [(`PCI_EXP_TRN_FC_DATA_WIDTH - 1):0]      trn_rfc_cpld_av_c;
    wire                                              trn_rcpl_streaming_n_c;
 
    wire    [(`PCI_EXP_CFG_DATA_WIDTH - 1):0]         cfg_do_c;
    wire    [(`PCI_EXP_CFG_DATA_WIDTH - 1):0]         cfg_di_c;
    wire    [(`PCI_EXP_CFG_ADDR_WIDTH - 1):0]         cfg_dwaddr_c;
    wire    [(`PCI_EXP_CFG_DATA_WIDTH/8 - 1):0]       cfg_byte_en_n_c;
    wire    [(`PCI_EXP_CFG_CPLHDR_WIDTH - 1):0]       cfg_err_tlp_cpl_header_c;
    wire                                              cfg_wr_en_n_c;
    wire                                              cfg_rd_en_n_c;
    wire                                              cfg_rd_wr_done_n_c;
    wire                                              cfg_err_cor_n_c;
    wire                                              cfg_err_ur_n_c;
    wire                                              cfg_err_ecrc_n_c;
    wire                                              cfg_err_cpl_timeout_n_c;
    wire                                              cfg_err_cpl_timeout;
    wire                                              cfg_err_cpl_abort_n_c;
    wire                                              cfg_err_cpl_unexpect_n_c;
    wire                                              cfg_err_posted_n_c;   
    wire                                              cfg_err_cpl_rdy_n_c; 
    wire                                              cfg_interrupt_n_c;
    wire                                              cfg_interrupt_rdy_n_c;

    wire                                              cfg_interrupt_assert_n_c;
    wire [7 : 0]                                      cfg_interrupt_di_c;
    wire [7 : 0]                                      cfg_interrupt_do_c;
    wire [2 : 0]                                      cfg_interrupt_mmenable_c;
    wire                                              cfg_interrupt_msienable_c;

    wire                                              cfg_turnoff_ok_n_c;
    wire                                              cfg_to_turnoff_n;
    wire                                              cfg_pm_wake_n_c;
    wire    [(`PCI_EXP_LNK_STATE_WIDTH - 1):0]        cfg_pcie_link_state_n_c;
    wire    [(`PCI_EXP_CFG_BUSNUM_WIDTH - 1):0]       cfg_bus_number_c;
    wire    [(`PCI_EXP_CFG_DEVNUM_WIDTH - 1):0]       cfg_device_number_c;
    wire    [(`PCI_EXP_CFG_FUNNUM_WIDTH - 1):0]       cfg_function_number_c;
    wire    [(`PCI_EXP_CFG_CAP_WIDTH - 1):0]          cfg_status_c;
    wire    [(`PCI_EXP_CFG_CAP_WIDTH - 1):0]          cfg_command_c;
    wire    [(`PCI_EXP_CFG_CAP_WIDTH - 1):0]          cfg_dstatus_c;
    wire    [(`PCI_EXP_CFG_CAP_WIDTH - 1):0]          cfg_dcommand_c;
    wire    [(`PCI_EXP_CFG_CAP_WIDTH - 1):0]          cfg_lstatus_c;
    wire    [(`PCI_EXP_CFG_CAP_WIDTH - 1):0]          cfg_lcommand_c;

//    wire                                              RST_i;
    
    wire [127:0] WRITE_MEM_DATA;
    wire [127:0] READ_MEM_DATA;
    wire         MEM_SEL;
    wire         WRITE_DATA_FULL;
    wire         ADDR_ALMOST_FULL;
    //added the delay for functional simulation to account for FIFO
    //delay in functional model
    wire [30:0] #1 MEM_ADDR;
    wire [2:0]     MEM_CMD;
    wire #1        DATA_WREN;
    wire #1        ADDR_WREN;

    wire [127:0] ingress_data;
//    wire  [1:0] ingress_fifo_ctrl;
	wire			ingress_fifo_wren;
//    wire  [1:0] ingress_fifo_status;
    wire  [2:0] ingress_xfer_size;
    wire [27:6] ingress_start_addr;
    wire        ingress_data_req;
    wire        ingress_data_ack;

	 wire        RXEnable;

	reg [31:0]	DebugRX1Overflowcount;
	reg [31:0]	DebugRX2Overflowcount;

    // RX FIFO	 
	 wire [63:0] 	RX_FIFO_data_out;
	 wire        	RX_FIFO_RDEN;
	 wire        	RX_FIFO_pempty;
	 wire        	RX_FIFO_full;	 
	 wire [31:0]	RX_TS_FIFO_dataout;
	 wire				RX_TS_FIFO_RDEN;
	 wire				RX_TS_FIFO_empty;
	 wire				RX_TS_FIFO_full;
/// wires for 2nd to 4th paths/radios
	 wire [63:0] 	RX_FIFO_2nd_data_out;
	 wire        	RX_FIFO_2nd_RDEN;
	 wire        	RX_FIFO_2nd_pempty;
	 wire        	RX_FIFO_2nd_full; 
	 wire [31:0]	RX_TS_FIFO_2nd_dataout;
	 wire				RX_TS_FIFO_2nd_RDEN;
	 wire				RX_TS_FIFO_2nd_empty;
	 wire				RX_TS_FIFO_2nd_full;
`ifdef MIMO_4X4
	 wire [63:0] 	RX_FIFO_3rd_data_out;
	 wire        	RX_FIFO_3rd_RDEN;
	 wire        	RX_FIFO_3rd_pempty;
	 wire        	RX_FIFO_3rd_full; 
	 wire [31:0]	RX_TS_FIFO_3rd_dataout;
	 wire				RX_TS_FIFO_3rd_RDEN;
	 wire				RX_TS_FIFO_3rd_empty;
	 wire [63:0] 	RX_FIFO_4th_data_out;
	 wire        	RX_FIFO_4th_RDEN;
	 wire        	RX_FIFO_4th_pempty;
	 wire        	RX_FIFO_4th_full; 
	 wire [31:0]	RX_TS_FIFO_4th_dataout;
	 wire				RX_TS_FIFO_4th_RDEN;
	 wire				RX_TS_FIFO_4th_empty;
`endif //MIMO_4X4

	 // Sora FRL RX paths
	 wire [31:0] Sora_FRL_RX_data;
	 wire			 Sora_FRL_RX_wren;
/// wires for 2nd to 4th paths/radios
	 wire [31:0] Sora_FRL_2nd_RX_data;
	 wire			 Sora_FRL_2nd_RX_wren;
`ifdef MIMO_4X4
	 wire [31:0] Sora_FRL_3rd_RX_data;
	 wire			 Sora_FRL_3rd_RX_wren;
	 wire [31:0] Sora_FRL_4th_RX_data;
	 wire			 Sora_FRL_4th_RX_wren;
`endif //MIMO_4X4
	 
	 // Sora FRLs, come from RAB
	 wire	CLK_Sora_FRL;
/// wires for 2nd to 4th paths/radios
	 wire	CLK_Sora_FRL_2nd;
`ifdef MIMO_4X4
	 wire	CLK_Sora_FRL_3rd;
	 wire	CLK_Sora_FRL_4th;
`endif //MIMO_4X4
	 
	 // Sora FRL TX paths
	 wire [31:0] Radio_TX_data;
//	 wire [15:0] Radio_TX_data;
	 wire			 Radio_TX_FIFO_rden;
	 wire			 Radio_TX_FIFO_pempty;
/// wires for 2nd to 4th paths/radios
	 wire [31:0] Radio_2nd_TX_data;
//	 wire [15:0] Radio_2nd_TX_data;
	 wire			 Radio_2nd_TX_FIFO_rden;
	 wire			 Radio_2nd_TX_FIFO_pempty;
`ifdef MIMO_4X4
	 wire [31:0] Radio_3rd_TX_data;
//	 wire [15:0] Radio_3rd_TX_data;
	 wire			 Radio_3rd_TX_FIFO_rden;
	 wire			 Radio_3rd_TX_FIFO_pempty;
	 wire [31:0] Radio_4th_TX_data;
//	 wire [15:0] Radio_4th_TX_data;
	 wire			 Radio_4th_TX_FIFO_rden;
	 wire			 Radio_4th_TX_FIFO_pempty;
`endif //MIMO_4X4
	 
	 
//	 wire        Radio_TX_done;
//	 wire        Radio_TX_start;
	 wire        TX_Ongoing;
	 // clock wires
    wire        clk133;

	 wire        sys_clk_ibufg;
	 wire        sys_clk_bufg;
	 
	  /// Jiansong: TX related wires
	 wire				TX_DDR_data_req;
	 wire				TX_DDR_data_ack;
	 wire [27:6]	TX_DDR_start_addr;
	 wire [2:0]		TX_DDR_xfer_size;
	 wire				TX_2nd_DDR_data_req;
	 wire				TX_2nd_DDR_data_ack;
	 wire [27:6]	TX_2nd_DDR_start_addr;
	 wire [2:0]		TX_2nd_DDR_xfer_size;
`ifdef MIMO_4X4
	 wire				TX_3rd_DDR_data_req;
	 wire				TX_3rd_DDR_data_ack;
	 wire [27:6]	TX_3rd_DDR_start_addr;
	 wire [2:0]		TX_3rd_DDR_xfer_size;
	 wire				TX_4th_DDR_data_req;
	 wire				TX_4th_DDR_data_ack;
	 wire [27:6]	TX_4th_DDR_start_addr;
	 wire [2:0]		TX_4th_DDR_xfer_size;
`endif //MIMO_4X4	 
	 
	 /// Jiansong: error signals
	 wire        egress_overflow_one;
    wire [31:0] egress_rd_data_count;
    wire [31:0] egress_wr_data_count;
	 

`ifdef RADIO_CHANNEL_REGISTERS
	 wire [31:0]	RChannel_Cmd_Data;
	 wire [6:0]		RChannel_Cmd_Addr;
	 wire				RChannel_Cmd_RdWr;
	 wire				RChannel_Cmd_wren;
	 wire	[31:0]	RChannel_Reg_Read_Value;
	 wire [7:0]		RChannel_Reg_Read_Addr;
	 wire				RChannel_ReadDone;
/// registers for 2nd to 4th paths/radios
	 wire [31:0]	RChannel_2nd_Cmd_Data;
	 wire [6:0]		RChannel_2nd_Cmd_Addr;
	 wire				RChannel_2nd_Cmd_RdWr;
	 wire				RChannel_2nd_Cmd_wren;
	 wire	[31:0]	RChannel_2nd_Reg_Read_Value;
	 wire [7:0]		RChannel_2nd_Reg_Read_Addr;
	 wire				RChannel_2nd_ReadDone;
`ifdef MIMO_4X4
	 wire [31:0]	RChannel_3rd_Cmd_Data;
	 wire [6:0]		RChannel_3rd_Cmd_Addr;
	 wire				RChannel_3rd_Cmd_RdWr;
	 wire				RChannel_3rd_Cmd_wren;
	 wire	[31:0]	RChannel_3rd_Reg_Read_Value;
	 wire [7:0]		RChannel_3rd_Reg_Read_Addr;
	 wire				RChannel_3rd_ReadDone;
	 wire [31:0]	RChannel_4th_Cmd_Data;
	 wire [6:0]		RChannel_4th_Cmd_Addr;
	 wire				RChannel_4th_Cmd_RdWr;
	 wire				RChannel_4th_Cmd_wren;
	 wire	[31:0]	RChannel_4th_Reg_Read_Value;
	 wire [7:0]		RChannel_4th_Reg_Read_Addr;
	 wire				RChannel_4th_ReadDone;
`endif //MIMO_4X4
	 // TX MSG FIFOs
	 wire				TXMSG_FIFO_rden;
	 wire [39:0]	TXMSG_FIFO_dataout;
	 wire				TXMSG_FIFO_empty;
/// registers for 2nd to 4th paths/radios
	 wire				TXMSG_FIFO_2nd_rden;
	 wire [39:0]	TXMSG_FIFO_2nd_dataout;
	 wire				TXMSG_FIFO_2nd_empty;
`ifdef MIMO_4X4
	 wire				TXMSG_FIFO_3rd_rden;
	 wire [39:0]	TXMSG_FIFO_3rd_dataout;
	 wire				TXMSG_FIFO_3rd_empty;
	 wire				TXMSG_FIFO_4th_rden;
	 wire [39:0]	TXMSG_FIFO_4th_dataout;
	 wire				TXMSG_FIFO_4th_empty;
`endif //MIMO_4X4

`endif //RADIO_CHANNEL_REGISTERS		
		
	wire [31:0]		DebugDDREgressFIFOCnt;
	wire [31:0]		DebugDDRFIFOFullCnt; 
	wire [31:0]		DebugDDRSignals;
	wire [8:0]		DebugDDRSMs;
	 
	 
	reg		  	Sora_RX_wren_LED;
	wire			Sora_FRL_linkup;
	wire			Sora_FRL_2nd_linkup;
	wire			Sora_FRL_3rd_linkup;
	wire			Sora_FRL_4th_linkup;
	 
    assign pcie_id[12:0] = {cfg_bus_number_c[7:0],
                            cfg_device_number_c[4:0]};
   
    always@(posedge trn_clk_c)begin                 
       max_pay_size_reg[2:0] <= cfg_dcommand_c[7:5];
       max_read_req_reg[2:0] <= cfg_dcommand_c[14:12];
       pcie_id_reg[12:0] <= pcie_id[12:0];
    end

    /// Jiansong: added
    assign nPLOAD_2 = 1'b0;    
    assign nPLOAD_1 = 1'b0; //Always deassert nPLOAD_1 to the clock syntheziser 
                           //chip so that a parallel load of the dip switch 
                           //is forced
                           //The Dip switch should be preset to create the 
                           //correct freq. for the DDR2 design

  //-------------------------------------------------------
  // System Reset Input Pad Instance
  //-------------------------------------------------------
  // 100 MHz clock
  IBUFDS refclk_ibuf (.O(sys_clk_c), .I(pcie_sys_clk_p), .IB(pcie_sys_clk_n)); 
  IBUF sys_reset_n_ibuf (.O(sys_reset_n_c), .I(sys_reset_n));

  
    // Front-panel LEDs	 
	OBUF Front_Sora_FRL_up_OBUF	(.O(Front_Sora_FRL_up), .I(Sora_FRL_linkup));
	OBUF Front_PCIe_DDR_up_OBUF	(.O(Front_PCIe_DDR_up), .I( phy_init_initialization_done & (~trn_lnk_up_n_c) ) );
	OBUF Front_Radio_TX_OBUF		(.O(Front_Radio_TX), .I(TX_Ongoing));
	OBUF Front_Radio_RX_blink_OBUF(.O(Front_Radio_RX_blink), .I(Sora_RX_wren_LED));


	OBUF Sora_FRL_done_n_OBUF (.O(Sora_FRL_done_n), .I(~Sora_FRL_linkup));
	OBUF LED_link_up_and_phy_init_initialization_done_n_OBUF (.O(LED_link_up_and_phy_init_initialization_done_n),
																				 .I( (~phy_init_initialization_done) | trn_lnk_up_n_c ) );																				 
	OBUF Radio_TX_n_OBUF(.O(Radio_TX_n), .I(~TX_Ongoing));
	OBUF Radio_RX_blink_OBUF(.O(Radio_RX_blink), .I(~Sora_RX_wren_LED));

  assign cfg_err_cpl_timeout_n_c = ~cfg_err_cpl_timeout;

  always@(posedge trn_clk_c)
     trn_reset_c <= ~trn_reset_n_c;
	  

`ifdef sora_chipscope
	// chipscope
	wire [35:0] CONTROL0;
	icon icon_inst(.CONTROL0(CONTROL0));
`endif

wire	TX_Start_one;
	  

	/// Jiansong
  //-------------------------------------------------------
  // Clock module
  //-------------------------------------------------------

	Clock_module_FRL clock_module_FRL_inst(
		// clock module only accepts hardware reset
		.rst(trn_reset_c),
		.clk200(sys_clk_bufg),
		.clk133(clk133),
		.unlocked_err()    /// whether the clocks are locked
	);

  IBUFGDS_LVPECL_25 u_ibufg_sys_clk
    (
     .I  (SYS_CLK_P),
     .IB (SYS_CLK_N),
     .O  (sys_clk_ibufg)
     );
	  
    BUFG u_bufg_sys_clk 
    (
     .O (sys_clk_bufg),
     .I (sys_clk_ibufg)
     );

	/// Jiansong
  //-------------------------------------------------------
  // Frequency dividing module for clock output to LED
  //-------------------------------------------------------
	reg [27:0] LED_Counter;
	always@(posedge CLK_Sora_FRL) begin
		if (trn_reset_c | soft_rst) begin
			LED_Counter <= 28'h000_0000;
			Sora_RX_wren_LED <= 1'b0;
		end else begin
			if (Sora_FRL_RX_wren)	LED_Counter <= LED_Counter + 28'h000_0001;
			if (LED_Counter == 28'h14FB180) begin	// 22,000,000
				LED_Counter <= 28'h000_0000;
				Sora_RX_wren_LED <= ~Sora_RX_wren_LED;
			end
		end			
	end
	
	
	////////////////  reset logic  /////////////////////////////////////////////
	//// In RCB logic, there will be two kinds of reset signals. The first kind
	//// is hard_reset_n, means it will reset the whole Sora firmware including PCIe 
	//// core and DDR2 memory controller. These resets are basically hard resets, 
	//// including reset button on RCB, reset signal from PCIe link, or reloading 
	//// RCB firmware. The second kind is SoftReset, it only reset all the firmware
	//// logic except PCIe core and DDR2 memory controller. These resets are generated 
	//// by PC or by reset button on RAB.
	
	// add an explicit reset signal during power on (reloading firmware)
	reg [31:0] 	rst_counter;
	reg			rst_poweron;
	initial 
		rst_counter = 32'h0000_0000;
	always @ (posedge trn_clk_c or negedge V5_reset_n) begin
		// a local reset signal is triggered 
		if (~V5_reset_n) begin
			rst_counter	<= 32'h0000_0000;
			rst_poweron	<= 1'b1;
		end else if(rst_counter < 32'h0000_30D4) begin		// 100us on 125MHz
			rst_counter <= rst_counter + 32'h0000_0001;
			rst_poweron	<= 1'b1;
		end else
			rst_poweron	<= 1'b0;	
	end

	assign hard_reset_n = sys_reset_n_c;
	assign soft_rst = hostreset | rst_from_frl | rst_from_frl_2nd | rst_from_frl_3rd | rst_from_frl_4th | rst_poweron | (~V5_reset_n);

  //-------------------------------------------------------
  // Endpoint Implementation Application
  //-------------------------------------------------------
	// register table inside
   pcie_dma_wrapper pcie_dma_wrapper_inst (
      .clk(trn_clk_c), 
		.full_rst(trn_reset_c),	
		.soft_rst(soft_rst),

      // hot reset to whole system
	   .hostreset_o(hostreset),

      /// Jiansong: interface to RX data fifo
		.RX_FIFO_data			(RX_FIFO_data_out[63:0]),
	   .RX_FIFO_RDEN			(RX_FIFO_RDEN),
	   .RX_FIFO_pempty		(RX_FIFO_pempty),
		.RX_TS_FIFO_data		(RX_TS_FIFO_dataout[31:0]),
		.RX_TS_FIFO_RDEN		(RX_TS_FIFO_RDEN),
		.RX_TS_FIFO_empty		(RX_TS_FIFO_empty),
		.RX_FIFO_2nd_data			(RX_FIFO_2nd_data_out[63:0]),
	   .RX_FIFO_2nd_RDEN			(RX_FIFO_2nd_RDEN),
	   .RX_FIFO_2nd_pempty		(RX_FIFO_2nd_pempty),
		.RX_TS_FIFO_2nd_data		(RX_TS_FIFO_2nd_dataout[31:0]),
		.RX_TS_FIFO_2nd_RDEN		(RX_TS_FIFO_2nd_RDEN),
		.RX_TS_FIFO_2nd_empty	(RX_TS_FIFO_2nd_empty),
`ifdef MIMO_4X4
		.RX_FIFO_3rd_data			(RX_FIFO_3rd_data_out[63:0]),
	   .RX_FIFO_3rd_RDEN			(RX_FIFO_3rd_RDEN),
	   .RX_FIFO_3rd_pempty		(RX_FIFO_3rd_pempty),
		.RX_TS_FIFO_3rd_data		(RX_TS_FIFO_3rd_dataout[31:0]),
		.RX_TS_FIFO_3rd_RDEN		(RX_TS_FIFO_3rd_RDEN),
		.RX_TS_FIFO_3rd_empty	(RX_TS_FIFO_3rd_empty),
		.RX_FIFO_4th_data			(RX_FIFO_4th_data_out[63:0]),
	   .RX_FIFO_4th_RDEN			(RX_FIFO_4th_RDEN),
	   .RX_FIFO_4th_pempty		(RX_FIFO_4th_pempty),
		.RX_TS_FIFO_4th_data		(RX_TS_FIFO_4th_dataout[31:0]),
		.RX_TS_FIFO_4th_RDEN		(RX_TS_FIFO_4th_RDEN),
		.RX_TS_FIFO_4th_empty	(RX_TS_FIFO_4th_empty),
`endif //MIMO_4X4
		
		.RXEnable_o			(RXEnable),
		
      /// Jiansong: interface to radio module
//		.Radio_TX_done		(Radio_TX_done),
//	   .Radio_TX_start	(Radio_TX_start),
	   .TX_Ongoing			(TX_Ongoing),
		
		// interface to dma_ddr2_if (DDR memory controller/scheduler)
	   .TX_DDR_data_req			(TX_DDR_data_req),
	   .TX_DDR_data_ack			(TX_DDR_data_ack),
	   .TX_DDR_start_addr		(TX_DDR_start_addr),
	   .TX_DDR_xfer_size			(TX_DDR_xfer_size),

	   .TX_2nd_DDR_data_req		(TX_2nd_DDR_data_req),
	   .TX_2nd_DDR_data_ack		(TX_2nd_DDR_data_ack),
	   .TX_2nd_DDR_start_addr	(TX_2nd_DDR_start_addr),
	   .TX_2nd_DDR_xfer_size	(TX_2nd_DDR_xfer_size),
		
`ifdef MIMO_4X4
	   .TX_3rd_DDR_data_req		(TX_3rd_DDR_data_req),
	   .TX_3rd_DDR_data_ack		(TX_3rd_DDR_data_ack),
	   .TX_3rd_DDR_start_addr	(TX_3rd_DDR_start_addr),
	   .TX_3rd_DDR_xfer_size	(TX_3rd_DDR_xfer_size),
		
	   .TX_4th_DDR_data_req		(TX_4th_DDR_data_req),
	   .TX_4th_DDR_data_ack		(TX_4th_DDR_data_ack),
	   .TX_4th_DDR_start_addr	(TX_4th_DDR_start_addr),
	   .TX_4th_DDR_xfer_size	(TX_4th_DDR_xfer_size),
`endif //MIMO_4X4

      //interface to dma_ddr2_if 
      .ingress_xfer_size	(ingress_xfer_size), 
      .ingress_start_addr	(ingress_start_addr), 
      .ingress_data_req		(ingress_data_req), 
      .ingress_data_ack		(ingress_data_ack), 
		.ingress_fifo_wren	(ingress_fifo_wren),
      .ingress_data			(ingress_data),
      .pause_read_requests	(pause_read_requests),

      //Misc signals to PCIE Block Plus 
      .pcie_max_pay_size	(max_pay_size_reg[2:0]), 
      .pcie_max_read_req	(max_read_req_reg[2:0]), 
      .pcie_id					(pcie_id_reg[12:0]),
      .comp_timeout			(cfg_err_cpl_timeout),

      // Tx Local-Link PCIE Block Plus
      .trn_td( trn_td_c ),                     // O [63/31:0]
      .trn_trem_n( trn_trem_n_c ),             // O [7:0]
      .trn_tsof_n( trn_tsof_n_c ),             // O
      .trn_teof_n( trn_teof_n_c ),             // O
      .trn_tsrc_rdy_n( trn_tsrc_rdy_n_c ),     // O
      .trn_tsrc_dsc_n( trn_tsrc_dsc_n_c ),     // O
      .trn_tdst_rdy_n( trn_tdst_rdy_n_c ),     // I
      .trn_tdst_dsc_n( trn_tdst_dsc_n_c ),     // I
      .trn_terrfwd_n( trn_terrfwd_n_c ),       // O
      .trn_tbuf_av( trn_tbuf_av_c[2:0] ),           // I [4/3:0]

      // Rx Local-Link PCIE Block Plus
      .trn_rd( trn_rd_c ),                     // I [63/31:0]
      .trn_rrem_n( trn_rrem_n_c ),             // I [7:0]
      .trn_rsof_n( trn_rsof_n_c ),             // I
      .trn_reof_n( trn_reof_n_c ),             // I
      .trn_rsrc_rdy_n( trn_rsrc_rdy_n_c ),     // I
      .trn_rsrc_dsc_n( trn_rsrc_dsc_n_c ),     // I
      .trn_rdst_rdy_n( trn_rdst_rdy_n_c ),     // O
      .trn_rerrfwd_n( trn_rerrfwd_n_c ),       // I
      .trn_rnp_ok_n( trn_rnp_ok_n_c ),         // O
      .trn_rbar_hit_n( trn_rbar_hit_n_c ),     // I [6:0]
      .trn_rfc_npd_av( trn_rfc_npd_av_c ),     // I [11:0]
      .trn_rfc_nph_av( trn_rfc_nph_av_c ),     // I [7:0]
      .trn_rfc_pd_av( trn_rfc_pd_av_c ),       // I [11:0]
      .trn_rfc_ph_av( trn_rfc_ph_av_c ),       // I [7:0]
      .trn_rfc_cpld_av( trn_rfc_cpld_av_c ),   // I [11:0]
      .trn_rfc_cplh_av( trn_rfc_cplh_av_c ),   // I [7:0]
      .trn_rcpl_streaming_n( trn_rcpl_streaming_n_c ), //O

      /// Jiansong: error signals
		.egress_overflow_one(egress_overflow_one),
		.RX_FIFO_full(RX_FIFO_full),
      .egress_rd_data_count(egress_rd_data_count),
      .egress_wr_data_count(egress_wr_data_count),

      //Interface to memory controller
      .phy_init_done(phy_init_initialization_done),

      // Jiansong: HW status register input
      .trn_lnk_up_n_c(trn_lnk_up_n_c),

      // radio related inputs/outputs
`ifdef RADIO_CHANNEL_REGISTERS

		.Radio_Cmd_Data	(RChannel_Cmd_Data[31:0]),
		.Radio_Cmd_Addr	(RChannel_Cmd_Addr[6:0]),
		.Radio_Cmd_RdWr	(RChannel_Cmd_RdWr),
		.Radio_Cmd_wren	(RChannel_Cmd_wren),
		.Channel_Reg_Read_Value	(RChannel_Reg_Read_Value[31:0]),
		.Channel_Reg_Read_Addr	(RChannel_Reg_Read_Addr[7:0]),
		.Channel_ReadDone_in		(RChannel_ReadDone),
/// registers for 2nd to 4th paths/radios
		.Radio_2nd_Cmd_Data	(RChannel_2nd_Cmd_Data[31:0]),
		.Radio_2nd_Cmd_Addr	(RChannel_2nd_Cmd_Addr[6:0]),
		.Radio_2nd_Cmd_RdWr	(RChannel_2nd_Cmd_RdWr),
		.Radio_2nd_Cmd_wren	(RChannel_2nd_Cmd_wren),
		.Channel_2nd_Reg_Read_Value	(RChannel_2nd_Reg_Read_Value[31:0]),
		.Channel_2nd_Reg_Read_Addr		(RChannel_2nd_Reg_Read_Addr[7:0]),
		.Channel_2nd_ReadDone_in		(RChannel_2nd_ReadDone),
`ifdef MIMO_4X4
		.Radio_3rd_Cmd_Data	(RChannel_3rd_Cmd_Data[31:0]),
		.Radio_3rd_Cmd_Addr	(RChannel_3rd_Cmd_Addr[6:0]),
		.Radio_3rd_Cmd_RdWr	(RChannel_3rd_Cmd_RdWr),
		.Radio_3rd_Cmd_wren	(RChannel_3rd_Cmd_wren),
		.Channel_3rd_Reg_Read_Value	(RChannel_3rd_Reg_Read_Value[31:0]),
		.Channel_3rd_Reg_Read_Addr		(RChannel_3rd_Reg_Read_Addr[7:0]),
		.Channel_3rd_ReadDone_in		(RChannel_3rd_ReadDone),
		.Radio_4th_Cmd_Data	(RChannel_4th_Cmd_Data[31:0]),
		.Radio_4th_Cmd_Addr	(RChannel_4th_Cmd_Addr[6:0]),
		.Radio_4th_Cmd_RdWr	(RChannel_4th_Cmd_RdWr),
		.Radio_4th_Cmd_wren	(RChannel_4th_Cmd_wren),
		.Channel_4th_Reg_Read_Value	(RChannel_4th_Reg_Read_Value[31:0]),
		.Channel_4th_Reg_Read_Addr		(RChannel_4th_Reg_Read_Addr[7:0]),
		.Channel_4th_ReadDone_in		(RChannel_4th_ReadDone),
`endif //MIMO_4X4

`endif //RADIO_CHANNEL_REGISTERS		
		//Debug interface
		.TX_Start_one		(TX_Start_one),
		
		.DebugRX1Overflowcount_in(DebugRX1Overflowcount),
		.DebugRX2Overflowcount_in(DebugRX2Overflowcount),
				
		.DebugDDREgressFIFOCnt	(DebugDDREgressFIFOCnt[31:0]),
		.DebugDDRFIFOFullCnt	(DebugDDRFIFOFullCnt[31:0]),		
		.DebugDDRSignals		(DebugDDRSignals[31:0]),
		.DebugDDRSMs			(DebugDDRSMs[8:0]),
		
		.PCIeLinkStatus(cfg_lstatus_c),
		.PCIeLinkControl(cfg_lcommand_c)
	);


    dma_ddr2_if dma_ddr2_if_inst(
       .dma_clk             (trn_clk_c),
       .ddr_clk             (clk_0_from_mem_ctrl_dcm),
       .reset               (SYS_RST_FROM_MEM_CTRL | trn_reset_c | hostreset),		

`ifdef sora_chipscope
		// chipscope
		 .CONTROL0		(CONTROL0),
		 .TX_Start_one	(TX_Start_one),
`endif

       //DMA SIGNALS
       //egress
			.radio_clk           			(CLK_Sora_FRL),
			.ToFRL_data_fifo_rddata			(Radio_TX_data[31:0]),
			.ToFRL_data_fifo_pempty			(Radio_TX_FIFO_pempty),
			.ToFRL_data_fifo_rden			(Radio_TX_FIFO_rden),
			.radio_2nd_clk						(CLK_Sora_FRL_2nd),
			.ToFRL_2nd_data_fifo_rddata	(Radio_2nd_TX_data[31:0]),
			.ToFRL_2nd_data_fifo_pempty	(Radio_2nd_TX_FIFO_pempty),
			.ToFRL_2nd_data_fifo_rden		(Radio_2nd_TX_FIFO_rden),
`ifdef MIMO_4X4
			.radio_3rd_clk						(CLK_Sora_FRL_3rd),
			.ToFRL_3rd_data_fifo_rddata	(Radio_3rd_TX_data[31:0]),
			.ToFRL_3rd_data_fifo_pempty	(Radio_3rd_TX_FIFO_pempty),
			.ToFRL_3rd_data_fifo_rden		(Radio_3rd_TX_FIFO_rden),
			.radio_4th_clk						(CLK_Sora_FRL_4th),
			.ToFRL_4th_data_fifo_rddata	(Radio_4th_TX_data[31:0]),
			.ToFRL_4th_data_fifo_pempty	(Radio_4th_TX_FIFO_pempty),
			.ToFRL_4th_data_fifo_rden		(Radio_4th_TX_FIFO_rden),
`endif //MIMO_4X4
		 		 
       .egress_xfer_size			(TX_DDR_xfer_size),
       .egress_start_addr			(TX_DDR_start_addr),
       .egress_data_req     		(TX_DDR_data_req),
       .egress_data_ack     		(TX_DDR_data_ack),		 
       .egress_2nd_xfer_size		(TX_2nd_DDR_xfer_size),
       .egress_2nd_start_addr		(TX_2nd_DDR_start_addr),
       .egress_2nd_data_req		(TX_2nd_DDR_data_req),
       .egress_2nd_data_ack		(TX_2nd_DDR_data_ack),		 
`ifdef MIMO_4X4
       .egress_3rd_xfer_size		(TX_3rd_DDR_xfer_size),
       .egress_3rd_start_addr		(TX_3rd_DDR_start_addr),
       .egress_3rd_data_req		(TX_3rd_DDR_data_req),
       .egress_3rd_data_ack		(TX_3rd_DDR_data_ack),		 
       .egress_4th_xfer_size		(TX_4th_DDR_xfer_size),
       .egress_4th_start_addr		(TX_4th_DDR_start_addr),
       .egress_4th_data_req		(TX_4th_DDR_data_req),
       .egress_4th_data_ack		(TX_4th_DDR_data_ack),		 
`endif //MIMO_4X4
       
       //ingress
       .ingress_data        (ingress_data),
			.ingress_fifo_wren	(ingress_fifo_wren),
       .ingress_xfer_size   (ingress_xfer_size),
       .ingress_start_addr  (ingress_start_addr),
       .ingress_data_req    (ingress_data_req),
       .ingress_data_ack    (ingress_data_ack),
       //END OF DMA SIGNALS
		 
		 /// Jiansong: error or debug signals
		 .egress_overflow_one (egress_overflow_one),
       .egress_wr_data_count(egress_wr_data_count),

       //MEMORY CNTRLR SIGNALS
       .m_wrdata            (WRITE_MEM_DATA),
       .m_rddata            (READ_MEM_DATA),
       .m_addr              (MEM_ADDR[30:0]),
       .m_cmd               (MEM_CMD[2:0]),
       .m_data_wen          (DATA_WREN),
       .m_addr_wen          (ADDR_WREN),
       .m_data_valid        (READ_MEM_VALID),
       .m_wdf_afull         (WRITE_DATA_FULL),             
       .m_af_afull          (ADDR_ALMOST_FULL),               
       //END OF MEMORY CNTRLR SIGNALS
       .pause_read_requests (pause_read_requests),
		 
		.DebugDDREgressFIFOCnt	(DebugDDREgressFIFOCnt[31:0]),
		.DebugDDRFIFOFullCnt	(DebugDDRFIFOFullCnt[31:0]),
		.DebugDDRSignals		(DebugDDRSignals[31:0]),
		.DebugDDRSMs			(DebugDDRSMs[8:0])
    );




	//-------------------------------------------------------
	// TX Control Message FIFO 1st
	//-------------------------------------------------------
	TX_MSG_fifo TX_MSG_fifo_inst(
		.rst			(trn_reset_c | soft_rst),
		.wr_clk		(trn_clk_c),
		.wr_en		(RChannel_Cmd_wren),
		.din			({RChannel_Cmd_RdWr,RChannel_Cmd_Addr[6:0],RChannel_Cmd_Data[31:0]}),
		.rd_clk		(CLK_Sora_FRL),
		.rd_en		(TXMSG_FIFO_rden),
		.dout			(TXMSG_FIFO_dataout[39:0]),
		.empty		(TXMSG_FIFO_empty),
		.full			(),
		.prog_full	()		
	);
	
	//-------------------------------------------------------
	// TX Control Message FIFO 2nd
	//-------------------------------------------------------
	TX_MSG_fifo TX_MSG_fifo_2nd_inst(
		.rst			(trn_reset_c | soft_rst),
		.wr_clk		(trn_clk_c),
		.wr_en		(RChannel_2nd_Cmd_wren),
		.din			({RChannel_2nd_Cmd_RdWr,RChannel_2nd_Cmd_Addr[6:0],RChannel_2nd_Cmd_Data[31:0]}),
		.rd_clk		(CLK_Sora_FRL_2nd),
		.rd_en		(TXMSG_FIFO_2nd_rden),
		.dout			(TXMSG_FIFO_2nd_dataout[39:0]),
		.empty		(TXMSG_FIFO_2nd_empty),
		.full			(),
		.prog_full	()		
	);

`ifdef MIMO_4X4	
	//-------------------------------------------------------
	// TX Control Message FIFO 3rd
	//-------------------------------------------------------
	TX_MSG_fifo TX_MSG_fifo_3rd_inst(
		.rst			(trn_reset_c | soft_rst),
		.wr_clk		(trn_clk_c),
		.wr_en		(RChannel_3rd_Cmd_wren),
		.din			({RChannel_3rd_Cmd_RdWr,RChannel_3rd_Cmd_Addr[6:0],RChannel_3rd_Cmd_Data[31:0]}),
		.rd_clk		(CLK_Sora_FRL_3rd),
		.rd_en		(TXMSG_FIFO_3rd_rden),
		.dout			(TXMSG_FIFO_3rd_dataout[39:0]),
		.empty		(TXMSG_FIFO_3rd_empty),
		.full			(),
		.prog_full	()		
	);
	
	//-------------------------------------------------------
	// TX Control Message FIFO 4th
	//-------------------------------------------------------
	TX_MSG_fifo TX_MSG_fifo_4th_inst(
		.rst			(trn_reset_c | soft_rst),
		.wr_clk		(trn_clk_c),
		.wr_en		(RChannel_4th_Cmd_wren),
		.din			({RChannel_4th_Cmd_RdWr,RChannel_4th_Cmd_Addr[6:0],RChannel_4th_Cmd_Data[31:0]}),
		.rd_clk		(CLK_Sora_FRL_4th),
		.rd_en		(TXMSG_FIFO_4th_rden),
		.dout			(TXMSG_FIFO_4th_dataout[39:0]),
		.empty		(TXMSG_FIFO_4th_empty),
		.full			(),
		.prog_full	()		
	);
`endif //MIMO_4X4

  //-------------------------------------------------------
  // RX FIFO
  //-------------------------------------------------------

////////////////////////////////////////////////////////////
/////////////////////// RX FIFOs 1st ////////////////////////

	reg [4:0]	data_ts_cnt;	// one TS for every 28 data-samples
	wire	last_of_29cycles;
	always@(posedge CLK_Sora_FRL) begin
		if (trn_reset_c | soft_rst)
			data_ts_cnt[4:0]	<= 5'b0_0000;
		else if (Sora_FRL_RX_wren)
			if (last_of_29cycles)
				data_ts_cnt[4:0]	<= 5'b0_0000;
			else
				data_ts_cnt[4:0]	<= data_ts_cnt[4:0] + 5'b0_0001;
		else
			data_ts_cnt[4:0]	<= data_ts_cnt[4:0];
	end
	
	assign last_of_29cycles = (data_ts_cnt[4:0] == 5'b1_1100) ? 1'b1 : 1'b0;

	/// Jiansong: Data FIFO on RX path, input from Sora FRL and 
	///           output to posted packet generator
	//This is an 4KB FIFO constructed of BRAM
	//Converts the datapath from 32-bit to 64-bit
	RX_data_fifo RX_data_fifo_SORA_FRL_inst(
		.din    (Sora_FRL_RX_data[31:0]),
		.rd_clk (trn_clk_c),
		.rd_en  (RX_FIFO_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk (CLK_Sora_FRL),
		.wr_en  (Sora_FRL_RX_wren & ~last_of_29cycles), 
		.dout   (RX_FIFO_data_out),
		.empty  (),                      /// Jiansong: programing full is used
		.full   (RX_FIFO_full),
		.prog_empty (RX_FIFO_pempty)	
	);

	RX_TS_fifo RX_TS_fifo_inst(
		.din		(Sora_FRL_RX_data[31:0]),
		.rd_clk	(trn_clk_c),
		.rd_en	(RX_TS_FIFO_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk	(CLK_Sora_FRL),
		.wr_en	(Sora_FRL_RX_wren & last_of_29cycles),
		.dout		(RX_TS_FIFO_dataout[31:0]),
		.empty	(RX_TS_FIFO_empty),
		.full		(RX_TS_FIFO_full)
	);
	
	always@(posedge CLK_Sora_FRL) begin
		if (trn_reset_c | soft_rst)
			DebugRX1Overflowcount[31:0] <= 32'h0000_0000;
		else begin
			if (RX_FIFO_full)			DebugRX1Overflowcount[15:0]	<= DebugRX1Overflowcount[15:0] + 16'h0001;
			if (RX_TS_FIFO_full)		DebugRX1Overflowcount[31:16]	<= DebugRX1Overflowcount[31:16] + 16'h0001;
		end
	end
	
////////////////////////////////////////////////////////////
/////////////////////// RX FIFOs 2nd ////////////////////////

	reg [4:0]	data_ts_cnt_2nd;	// one TS for every 28 data-samples
	wire	last_of_29cycles_2nd;
	always@(posedge CLK_Sora_FRL_2nd) begin
		if (trn_reset_c | soft_rst)
			data_ts_cnt_2nd[4:0]	<= 5'b0_0000;
		else if (Sora_FRL_2nd_RX_wren)
			if (last_of_29cycles_2nd)
				data_ts_cnt_2nd[4:0]	<= 5'b0_0000;
			else
				data_ts_cnt_2nd[4:0]	<= data_ts_cnt_2nd[4:0] + 5'b0_0001;
		else
			data_ts_cnt_2nd[4:0]	<= data_ts_cnt_2nd[4:0];
	end
	
	assign last_of_29cycles_2nd = (data_ts_cnt_2nd[4:0] == 5'b1_1100) ? 1'b1 : 1'b0;

	/// Jiansong: Data FIFO on RX path, input from Sora FRL and 
	///           output to posted packet generator
	//This is an 4KB FIFO constructed of BRAM
	//Converts the datapath from 32-bit to 64-bit
	RX_data_fifo RX_data_fifo_SORA_FRL_2nd_inst(
		.din    (Sora_FRL_2nd_RX_data[31:0]),
		.rd_clk (trn_clk_c),
		.rd_en  (RX_FIFO_2nd_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk (CLK_Sora_FRL_2nd),
		.wr_en  (Sora_FRL_2nd_RX_wren & ~last_of_29cycles_2nd), 
		.dout   (RX_FIFO_2nd_data_out),
		.empty  (),                      /// Jiansong: programing full is used
		.full   (RX_FIFO_2nd_full),
		.prog_empty (RX_FIFO_2nd_pempty)	
	);

	RX_TS_fifo RX_TS_fifo_2nd_inst(
		.din		(Sora_FRL_2nd_RX_data[31:0]),
		.rd_clk	(trn_clk_c),
		.rd_en	(RX_TS_FIFO_2nd_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk	(CLK_Sora_FRL_2nd),
		.wr_en	(Sora_FRL_2nd_RX_wren & last_of_29cycles_2nd),
		.dout		(RX_TS_FIFO_2nd_dataout[31:0]),
		.empty	(RX_TS_FIFO_2nd_empty),
		.full		(RX_TS_FIFO_2nd_full)
	);
	
	always@(posedge CLK_Sora_FRL_2nd) begin
		if (trn_reset_c | soft_rst)
			DebugRX2Overflowcount[31:0] <= 32'h0000_0000;
		else begin
			if (RX_FIFO_2nd_full)			DebugRX2Overflowcount[15:0]	<= DebugRX2Overflowcount[15:0] + 16'h0001;
			if (RX_TS_FIFO_2nd_full)		DebugRX2Overflowcount[31:16]	<= DebugRX2Overflowcount[31:16] + 16'h0001;
		end
	end


`ifdef MIMO_4X4
////////////////////////////////////////////////////////////
/////////////////////// RX FIFOs 3rd ////////////////////////

	reg [4:0]	data_ts_cnt_3rd;	// one TS for every 28 data-samples
	wire	last_of_29cycles_3rd;
	always@(posedge CLK_Sora_FRL_3rd) begin
		if (trn_reset_c | soft_rst)
			data_ts_cnt_3rd[4:0]	<= 5'b0_0000;
		else if (Sora_FRL_3rd_RX_wren)
			if (last_of_29cycles_3rd)
				data_ts_cnt_3rd[4:0]	<= 5'b0_0000;
			else
				data_ts_cnt_3rd[4:0]	<= data_ts_cnt_3rd[4:0] + 5'b0_0001;
		else
			data_ts_cnt_3rd[4:0]	<= data_ts_cnt_3rd[4:0];
	end
	
	assign last_of_29cycles_3rd = (data_ts_cnt_3rd[4:0] == 5'b1_1100) ? 1'b1 : 1'b0;

	/// Jiansong: Data FIFO on RX path, input from Sora FRL and 
	///           output to posted packet generator
	//This is an 4KB FIFO constructed of BRAM
	//Converts the datapath from 32-bit to 64-bit
	RX_data_fifo RX_data_fifo_SORA_FRL_3rd_inst(
		.din    (Sora_FRL_3rd_RX_data[31:0]),
		.rd_clk (trn_clk_c),
		.rd_en  (RX_FIFO_3rd_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk (CLK_Sora_FRL_3rd),
		.wr_en  (Sora_FRL_3rd_RX_wren & ~last_of_29cycles_3rd), 
		.dout   (RX_FIFO_3rd_data_out),
		.empty  (),                      /// Jiansong: programing full is used
		.full   (RX_FIFO_3rd_full),
		.prog_empty (RX_FIFO_3rd_pempty)	
	);

	RX_TS_fifo RX_TS_fifo_3rd_inst(
		.din		(Sora_FRL_3rd_RX_data[31:0]),
		.rd_clk	(trn_clk_c),
		.rd_en	(RX_TS_FIFO_3rd_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk	(CLK_Sora_FRL_3rd),
		.wr_en	(Sora_FRL_3rd_RX_wren & last_of_29cycles_3rd),
		.dout		(RX_TS_FIFO_3rd_dataout[31:0]),
		.empty	(RX_TS_FIFO_3rd_empty),
		.full		()
	);
	

////////////////////////////////////////////////////////////
/////////////////////// RX FIFOs 4th ////////////////////////

	reg [4:0]	data_ts_cnt_4th;	// one TS for every 28 data-samples
	wire	last_of_29cycles_4th;
	always@(posedge CLK_Sora_FRL_4th) begin
		if (trn_reset_c | soft_rst)
			data_ts_cnt_4th[4:0]	<= 5'b0_0000;
		else if (Sora_FRL_4th_RX_wren)
			if (last_of_29cycles_4th)
				data_ts_cnt_4th[4:0]	<= 5'b0_0000;
			else
				data_ts_cnt_4th[4:0]	<= data_ts_cnt_4th[4:0] + 5'b0_0001;
		else
			data_ts_cnt_4th[4:0]	<= data_ts_cnt_4th[4:0];
	end
	
	assign last_of_29cycles_4th = (data_ts_cnt_4th[4:0] == 5'b1_1100) ? 1'b1 : 1'b0;

	/// Jiansong: Data FIFO on RX path, input from Sora FRL and 
	///           output to posted packet generator
	//This is an 4KB FIFO constructed of BRAM
	//Converts the datapath from 32-bit to 64-bit
	RX_data_fifo RX_data_fifo_SORA_FRL_4th_inst(
		.din    (Sora_FRL_4th_RX_data[31:0]),
		.rd_clk (trn_clk_c),
		.rd_en  (RX_FIFO_4th_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk (CLK_Sora_FRL_4th),
		.wr_en  (Sora_FRL_4th_RX_wren & ~last_of_29cycles_4th), 
		.dout   (RX_FIFO_4th_data_out),
		.empty  (),                      /// Jiansong: programing full is used
		.full   (RX_FIFO_4th_full),
		.prog_empty (RX_FIFO_4th_pempty)	
	);

	RX_TS_fifo RX_TS_fifo_4th_inst(
		.din		(Sora_FRL_4th_RX_data[31:0]),
		.rd_clk	(trn_clk_c),
		.rd_en	(RX_TS_FIFO_4th_RDEN),
		.rst		(trn_reset_c | soft_rst),
		.wr_clk	(CLK_Sora_FRL_4th),
		.wr_en	(Sora_FRL_4th_RX_wren & last_of_29cycles_4th),
		.dout		(RX_TS_FIFO_4th_dataout[31:0]),
		.empty	(RX_TS_FIFO_4th_empty),
		.full		()
	);
`endif //MIMO_4X4




  //-------------------------------------------------------
  // Radio module
  //-------------------------------------------------------

//`ifndef sora_simulation				// we use parallel mode in simulation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++			Sora Fast Radio Link			+++++++++++++++++++++


	Sora_FRL_RCB Sora_FRL_RCB_inst(
			// signals to FPGA pins 
			// inputs
			.CLK_I_p					(Sora_FRL_CLK_I_p), 
			.CLK_I_n					(Sora_FRL_CLK_I_n),
			.DATA_IN_p				(Sora_FRL_DATA_IN_p),
			.DATA_IN_n				(Sora_FRL_DATA_IN_n),
			.MSG_IN_p				(Sora_FRL_MSG_IN_p),
			.MSG_IN_n				(Sora_FRL_MSG_IN_n), 
			.STATUS_IN_p			(Sora_FRL_STATUS_IN_p),
			.STATUS_IN_n			(Sora_FRL_STATUS_IN_n),
			// outputs
         .CLK_O_p					(Sora_FRL_CLK_O_p), 
			.CLK_O_n					(Sora_FRL_CLK_O_n),								
			.MSG_OUT_p				(Sora_FRL_MSG_OUT_p),
			.MSG_OUT_n				(Sora_FRL_MSG_OUT_n),
			.DATA_OUT_p				(Sora_FRL_DATA_OUT_p), 
			.DATA_OUT_n				(Sora_FRL_DATA_OUT_n),
			.STATUS_OUT_p			(Sora_FRL_STATUS_OUT_p),
			.STATUS_OUT_n			(Sora_FRL_STATUS_OUT_n),
			
			// signals to internal modules
			.rst_in_internal		(trn_reset_c | hostreset | rst_poweron | (~V5_reset_n) | rst_from_frl_2nd | rst_from_frl_3rd | rst_from_frl_4th),
			.rst_out_internal		(rst_from_frl),
			.CLKDIV_R				(CLK_Sora_FRL),
			// data input from internal logic
			.SEND_EN					(~Radio_TX_FIFO_pempty),
			.DATA_INT_IN			(Radio_TX_data[31:0]),	
			.RDEN_DATA_INT_IN		(Radio_TX_FIFO_rden),
			// control message input from internal logic
			.MSG_INT_IN				(TXMSG_FIFO_dataout[39:0]),
			.EMPTY_MSG_INT_IN		(TXMSG_FIFO_empty),
			.RDEN_MSG_INT_IN		(TXMSG_FIFO_rden),
			// data output to internal logic
			.DATA_INT_OUT			(Sora_FRL_RX_data[31:0]),
			.WREN_DATA_INT_OUT	(Sora_FRL_RX_wren),
			// state message output to internal logic
			// modified by Jiansong, 2010-5-27, we do not use FIFO to buf MSG_INT_OUT
			.MSG_INT_OUT_Data		(RChannel_Reg_Read_Value[31:0]),
			.MSG_INT_OUT_Addr		(RChannel_Reg_Read_Addr[7:0]),
			.MSG_Valid				(RChannel_ReadDone),
			// debug info
			.TXMSG_MISS_cnt		(TXMSG_MISS_cnt),
			.TXMSG_PASS_cnt		(TXMSG_PASS_cnt),
			.Radio_status_error	(),
			.Sora_FRL_linkup		(Sora_FRL_linkup),
			.LED						()
			);
			
	Sora_FRL_RCB Sora_FRL_2nd_RCB_inst(
			// signals to FPGA pins 
			// inputs
			.CLK_I_p					(Sora_FRL_2nd_CLK_I_p), 
			.CLK_I_n					(Sora_FRL_2nd_CLK_I_n),
			.DATA_IN_p				(Sora_FRL_2nd_DATA_IN_p),
			.DATA_IN_n				(Sora_FRL_2nd_DATA_IN_n),
			.MSG_IN_p				(Sora_FRL_2nd_MSG_IN_p),
			.MSG_IN_n				(Sora_FRL_2nd_MSG_IN_n), 
			.STATUS_IN_p			(Sora_FRL_2nd_STATUS_IN_p),
			.STATUS_IN_n			(Sora_FRL_2nd_STATUS_IN_n),
			// outputs
         .CLK_O_p					(Sora_FRL_2nd_CLK_O_p), 
			.CLK_O_n					(Sora_FRL_2nd_CLK_O_n),								
			.MSG_OUT_p				(Sora_FRL_2nd_MSG_OUT_p),
			.MSG_OUT_n				(Sora_FRL_2nd_MSG_OUT_n),
			.DATA_OUT_p				(Sora_FRL_2nd_DATA_OUT_p), 
			.DATA_OUT_n				(Sora_FRL_2nd_DATA_OUT_n),
			.STATUS_OUT_p			(Sora_FRL_2nd_STATUS_OUT_p),
			.STATUS_OUT_n			(Sora_FRL_2nd_STATUS_OUT_n),
			
			// signals to internal modules
			.rst_in_internal		(trn_reset_c | hostreset | rst_poweron | (~V5_reset_n) | rst_from_frl | rst_from_frl_3rd | rst_from_frl_4th),
			.rst_out_internal		(rst_from_frl_2nd),
			.CLKDIV_R				(CLK_Sora_FRL_2nd),
			// data input from internal logic
			.SEND_EN					(~Radio_2nd_TX_FIFO_pempty),
			.DATA_INT_IN			(Radio_2nd_TX_data[31:0]),	
			.RDEN_DATA_INT_IN		(Radio_2nd_TX_FIFO_rden),
			// control message input from internal logic
			.MSG_INT_IN				(TXMSG_FIFO_2nd_dataout[39:0]),
			.EMPTY_MSG_INT_IN		(TXMSG_FIFO_2nd_empty),
			.RDEN_MSG_INT_IN		(TXMSG_FIFO_2nd_rden),
			// data output to internal logic
			.DATA_INT_OUT			(Sora_FRL_2nd_RX_data[31:0]),
			.WREN_DATA_INT_OUT	(Sora_FRL_2nd_RX_wren),
			// state message output to internal logic
			// modified by Jiansong, 2010-5-27, we do not use FIFO to buf MSG_INT_OUT
			.MSG_INT_OUT_Data		(RChannel_2nd_Reg_Read_Value[31:0]),
			.MSG_INT_OUT_Addr		(RChannel_2nd_Reg_Read_Addr[7:0]),
			.MSG_Valid				(RChannel_2nd_ReadDone),
			// debug info
			.TXMSG_MISS_cnt		(TXMSG_2nd_MISS_cnt),
			.TXMSG_PASS_cnt		(TXMSG_2nd_PASS_cnt),
			.Radio_status_error	(),
			.Sora_FRL_linkup		(Sora_FRL_2nd_linkup),
			.LED						()
			);
		
`ifdef MIMO_4X4		
	Sora_FRL_RCB Sora_FRL_3rd_RCB_inst(
			// signals to FPGA pins 
//			 inputs
			.CLK_I_p					(Sora_FRL_3rd_CLK_I_p), 
			.CLK_I_n					(Sora_FRL_3rd_CLK_I_n),
			.DATA_IN_p				(Sora_FRL_3rd_DATA_IN_p),
			.DATA_IN_n				(Sora_FRL_3rd_DATA_IN_n),
			.MSG_IN_p				(Sora_FRL_3rd_MSG_IN_p),
			.MSG_IN_n				(Sora_FRL_3rd_MSG_IN_n), 
			.STATUS_IN_p			(Sora_FRL_3rd_STATUS_IN_p),
			.STATUS_IN_n			(Sora_FRL_3rd_STATUS_IN_n),
			// outputs
         .CLK_O_p					(Sora_FRL_3rd_CLK_O_p), 
			.CLK_O_n					(Sora_FRL_3rd_CLK_O_n),								
			.MSG_OUT_p				(Sora_FRL_3rd_MSG_OUT_p),
			.MSG_OUT_n				(Sora_FRL_3rd_MSG_OUT_n),
			.DATA_OUT_p				(Sora_FRL_3rd_DATA_OUT_p), 
			.DATA_OUT_n				(Sora_FRL_3rd_DATA_OUT_n),
			.STATUS_OUT_p			(Sora_FRL_3rd_STATUS_OUT_p),
			.STATUS_OUT_n			(Sora_FRL_3rd_STATUS_OUT_n),
			
			
			
			// signals to internal modules
			.rst_in_internal		(trn_reset_c | hostreset | rst_poweron | (~V5_reset_n) | rst_from_frl | rst_from_frl_2nd | rst_from_frl_4th),
			.rst_out_internal		(rst_from_frl_3rd),
			.CLKDIV_R				(CLK_Sora_FRL_3rd),
			// data input from internal logic
			.SEND_EN					(~Radio_3rd_TX_FIFO_pempty),
			.DATA_INT_IN			(Radio_3rd_TX_data[31:0]),
			.RDEN_DATA_INT_IN		(Radio_3rd_TX_FIFO_rden),
			// control message input from internal logic
			.MSG_INT_IN				(TXMSG_FIFO_3rd_dataout[39:0]),
			.EMPTY_MSG_INT_IN		(TXMSG_FIFO_3rd_empty),
			.RDEN_MSG_INT_IN		(TXMSG_FIFO_3rd_rden),
			// data output to internal logic
			.DATA_INT_OUT			(Sora_FRL_3rd_RX_data[31:0]),
			.WREN_DATA_INT_OUT	(Sora_FRL_3rd_RX_wren),
			// state message output to internal logic
			// modified by Jiansong, 2010-5-27, we do not use FIFO to buf MSG_INT_OUT
			.MSG_INT_OUT_Data		(RChannel_3rd_Reg_Read_Value[31:0]),
			.MSG_INT_OUT_Addr		(RChannel_3rd_Reg_Read_Addr[7:0]),
			.MSG_Valid				(RChannel_3rd_ReadDone),
			// debug info
			.TXMSG_MISS_cnt		(TXMSG_3rd_MISS_cnt),
			.TXMSG_PASS_cnt		(TXMSG_3rd_PASS_cnt),
			.Radio_status_error	(),
			.Sora_FRL_linkup		(Sora_FRL_3rd_linkup),
			.LED						()
			);
			
	Sora_FRL_RCB Sora_FRL_4th_RCB_inst(
			// signals to FPGA pins 
			// inputs
			.CLK_I_p					(Sora_FRL_4th_CLK_I_p), 
			.CLK_I_n					(Sora_FRL_4th_CLK_I_n),
			.DATA_IN_p				(Sora_FRL_4th_DATA_IN_p),
			.DATA_IN_n				(Sora_FRL_4th_DATA_IN_n),
			.MSG_IN_p				(Sora_FRL_4th_MSG_IN_p),
			.MSG_IN_n				(Sora_FRL_4th_MSG_IN_n), 
			.STATUS_IN_p			(Sora_FRL_4th_STATUS_IN_p),
			.STATUS_IN_n			(Sora_FRL_4th_STATUS_IN_n),
			// outputs
         .CLK_O_p					(Sora_FRL_4th_CLK_O_p), 
			.CLK_O_n					(Sora_FRL_4th_CLK_O_n),								
			.MSG_OUT_p				(Sora_FRL_4th_MSG_OUT_p),
			.MSG_OUT_n				(Sora_FRL_4th_MSG_OUT_n),
			.DATA_OUT_p				(Sora_FRL_4th_DATA_OUT_p), 
			.DATA_OUT_n				(Sora_FRL_4th_DATA_OUT_n),
			.STATUS_OUT_p			(Sora_FRL_4th_STATUS_OUT_p),
			.STATUS_OUT_n			(Sora_FRL_4th_STATUS_OUT_n),
						
			// signals to internal modules
			.rst_in_internal		(trn_reset_c | hostreset | rst_poweron | (~V5_reset_n) | rst_from_frl | rst_from_frl_2nd | rst_from_frl_3rd),
			.rst_out_internal		(rst_from_frl_4th),
			.CLKDIV_R				(CLK_Sora_FRL_4th),
			// data input from internal logic
			.SEND_EN					(~Radio_4th_TX_FIFO_pempty),
			.DATA_INT_IN			(Radio_4th_TX_data[31:0]),
			.RDEN_DATA_INT_IN		(Radio_4th_TX_FIFO_rden),
			// control message input from internal logic
			.MSG_INT_IN				(TXMSG_FIFO_4th_dataout[39:0]),
			.EMPTY_MSG_INT_IN		(TXMSG_FIFO_4th_empty),
			.RDEN_MSG_INT_IN		(TXMSG_FIFO_4th_rden),
			// data output to internal logic
			.DATA_INT_OUT			(Sora_FRL_4th_RX_data[31:0]),
			.WREN_DATA_INT_OUT	(Sora_FRL_4th_RX_wren),
			// state message output to internal logic
			// modified by Jiansong, 2010-5-27, we do not use FIFO to buf MSG_INT_OUT
			.MSG_INT_OUT_Data		(RChannel_4th_Reg_Read_Value[31:0]),
			.MSG_INT_OUT_Addr		(RChannel_4th_Reg_Read_Addr[7:0]),
			.MSG_Valid				(RChannel_4th_ReadDone),
			// debug info
			.TXMSG_MISS_cnt		(TXMSG_4th_MISS_cnt),
			.TXMSG_PASS_cnt		(TXMSG_4th_PASS_cnt),
			.Radio_status_error	(),
			.Sora_FRL_linkup		(Sora_FRL_4th_linkup),
			.LED						()
			);
`endif //MIMO_4X4
//+++++++++++++++			Sora Fast Radio Link			+++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


  //-------------------------------------------------------
  // Block Plus Core for PCI Express Instance
  //-------------------------------------------------------

`ifdef sora_simulation
  endpoint_blk_plus_v1_8    ep  (       /// Jiansong: temporarily for simulation use
`else
//  endpoint_blk_plus_v1_9    ep  (  /// Jiansong: for synthesize use
  endpoint_blk_plus_v1_14    ep  (  /// Jiansong: for synthesize use
`endif
      //
      // PCI Express Fabric Interface
      //
      .pci_exp_txp( pci_exp_txp ),             // O [7/3/0:0]
      .pci_exp_txn( pci_exp_txn ),             // O [7/3/0:0]
      .pci_exp_rxp( pci_exp_rxp ),             // O [7/3/0:0]
      .pci_exp_rxn( pci_exp_rxn ),             // O [7/3/0:0]

      //
      // Transaction ( TRN ) Interface
      //
      .trn_clk( trn_clk_c ),                   // O
      .trn_reset_n( trn_reset_n_c ),           // O
      .trn_lnk_up_n( trn_lnk_up_n_c ),         // O

      // Tx Local-Link
      .trn_td( trn_td_c ),                     // I [63/31:0]
      .trn_trem_n( trn_trem_n_c ),             // I [7:0]
      .trn_tsof_n( trn_tsof_n_c ),             // I
      .trn_teof_n( trn_teof_n_c ),             // I
      .trn_tsrc_rdy_n( trn_tsrc_rdy_n_c ),     // I
      .trn_tsrc_dsc_n( trn_tsrc_dsc_n_c ),     // I
      .trn_tdst_rdy_n( trn_tdst_rdy_n_c ),     // O
      .trn_tdst_dsc_n( trn_tdst_dsc_n_c ),     // O
      .trn_terrfwd_n( trn_terrfwd_n_c ),       // I
      .trn_tbuf_av( trn_tbuf_av_c ),           // O [4/3:0]

      // Rx Local-Link
      .trn_rd( trn_rd_c ),                     // O [63/31:0]
      .trn_rrem_n( trn_rrem_n_c ),             // O [7:0]
      .trn_rsof_n( trn_rsof_n_c ),             // O
      .trn_reof_n( trn_reof_n_c ),             // O
      .trn_rsrc_rdy_n( trn_rsrc_rdy_n_c ),     // O
      .trn_rsrc_dsc_n( trn_rsrc_dsc_n_c ),     // O
      .trn_rdst_rdy_n( trn_rdst_rdy_n_c ),     // I
      .trn_rerrfwd_n( trn_rerrfwd_n_c ),       // O
      .trn_rnp_ok_n( trn_rnp_ok_n_c ),         // I
      .trn_rbar_hit_n( trn_rbar_hit_n_c ),     // O [6:0]
      .trn_rfc_nph_av( trn_rfc_nph_av_c ),     // O [11:0]
      .trn_rfc_npd_av( trn_rfc_npd_av_c ),     // O [7:0]
      .trn_rfc_ph_av( trn_rfc_ph_av_c ),       // O [11:0]
      .trn_rfc_pd_av( trn_rfc_pd_av_c ),       // O [7:0]
      .trn_rcpl_streaming_n( trn_rcpl_streaming_n_c ),       // I

      //
      // Host ( CFG ) Interface
      //
      .cfg_do( cfg_do_c ),                                    // O [31:0]
      .cfg_rd_wr_done_n( cfg_rd_wr_done_n_c ),                // O
      .cfg_di( 32'h00000000),                                 // I [31:0]
      .cfg_byte_en_n( 4'b1111),                               // I [3:0]
      .cfg_dwaddr( 10'b0000000000),                           // I [9:0]
      .cfg_wr_en_n( 1'b1),                                    // I
      .cfg_rd_en_n( 1'b1),                                    // I

      .cfg_err_cor_n( 1'b1),                                  // I
      .cfg_err_ur_n( 1'b1),                                   // I
      .cfg_err_ecrc_n( 1'b1),                                 // I
// Jiansong, 2010-3-16, disable error report
//      .cfg_err_cpl_timeout_n(cfg_err_cpl_timeout_n_c),        // I
		.cfg_err_cpl_timeout_n(1'b1),        							// I
      .cfg_err_cpl_abort_n( 1'b1),                            // I
      .cfg_err_cpl_unexpect_n( 1'b1),                         // I
      .cfg_err_posted_n( 1'b1),                               // I
      .cfg_err_tlp_cpl_header( 48'h000000000000),             // I [47:0]
      .cfg_err_cpl_rdy_n(cfg_err_cpl_rdy_n_c),                // O
      .cfg_err_locked_n(1'b1),                                // I
      .cfg_interrupt_n( 1'b1),                                // I
      .cfg_interrupt_rdy_n( cfg_interrupt_rdy_n_c ),          // O

      .cfg_interrupt_assert_n(1'b1),                          // I
      .cfg_interrupt_di(8'b00000000),                         // I [7:0]
      .cfg_interrupt_do(cfg_interrupt_do_c),                  // O [7:0]
      .cfg_interrupt_mmenable(cfg_interrupt_mmenable_c),      // O [2:0]
      .cfg_interrupt_msienable(cfg_interrupt_msienable_c),    // O  

      .cfg_pm_wake_n( 1'b1),                                  // I
      .cfg_pcie_link_state_n( cfg_pcie_link_state_n_c ),      // O [2:0]
      .cfg_to_turnoff_n( cfg_to_turnoff_n_c ),                // O
      .cfg_trn_pending_n( 1'b1),                              // I
      .cfg_dsn( 64'h0000000000000000),                        // I [63:0]

      .cfg_bus_number( cfg_bus_number_c ),                    // O [7:0]
      .cfg_device_number( cfg_device_number_c ),              // O [4:0]
      .cfg_function_number( cfg_function_number_c ),          // O [2:0]
      .cfg_status( cfg_status_c ),                            // O [15:0]
      .cfg_command( cfg_command_c ),                          // O [15:0]
      .cfg_dstatus( cfg_dstatus_c ),                          // O [15:0]
      .cfg_dcommand( cfg_dcommand_c ),                        // O [15:0]
      .cfg_lstatus( cfg_lstatus_c ),                          // O [15:0]
      .cfg_lcommand( cfg_lcommand_c ),                        // O [15:0]

      // System ( SYS ) Interface
      .sys_clk( sys_clk_c ),                                   // I
      .sys_reset_n( hard_reset_n),              // I 

      .refclkout(),

       // The following is used for simulation only.  Setting
       // the following core input to 1 will result in a fast
       // train simulation to happen.  This bit should not be set
       // during synthesis or the core may not operate properly.

       `ifdef SIMULATION
       .fast_train_simulation_only(1'b1)
       `else
       .fast_train_simulation_only(1'b0)
       `endif    
      );


    //Instantiate DDR2 memory controller
    //This reference design uses a 64bit DDR design
    mem_interface_top #(
       .BANK_WIDTH(2),       
       .CLK_WIDTH(2),
       .DM_WIDTH(8),      
       .DQ_WIDTH(64),    
       .DQS_WIDTH(8),      
       .ROW_WIDTH(13),      
       .CAS_LAT(4),      
       .REG_ENABLE(0),
       .SIM_ONLY(DDR_SIM)     
    )
    ddr2_cntrl_inst(
       .ddr2_dq          (DDR2_DQ),       //inout  [63:0]  
       .ddr2_a           (DDR2_A),        //output  [13:0]  
       .ddr2_ba          (DDR2_BA),       //output  [1:0]  
       .ddr2_ras_n       (DDR2_RAS_N),    //output  
       .ddr2_cas_n       (DDR2_CAS_N),    //output  
       .ddr2_we_n        (DDR2_WE_N),     //output 
       .ddr2_reset_n     (DDR2_RESET_N),  //output 

       .ddr2_cs_n        (DDR2_CS_N),     //output  [0:0]  
       .ddr2_odt         (DDR2_ODT),      //output  [0:0]  
       .ddr2_cke         (DDR2_CKE),      //output  [0:0]  
       .ddr2_dm          (DDR2_DM),       //output  [8:0]   

       .sys_clk          (clk133),  


       .clk200_p         (CLK200_P),      //input  
       .clk200_n         (CLK200_N),      //input
       /// Jiansong: 200MHz output
       .clk200_o         (),		 
       .sys_rst_n        (hard_reset_n),    // DCM need multi-cycle reset 
       .rst0_tb          (SYS_RST_FROM_MEM_CTRL),        //output 
       .clk0_tb          (clk_0_from_mem_ctrl_dcm),      //output
       .phy_init_done    (phy_init_initialization_done), //output  

       .app_wdf_afull    (WRITE_DATA_FULL),              //output  
       .app_af_afull     (ADDR_ALMOST_FULL),             //output  
       .rd_data_valid    (READ_MEM_VALID),               //output  
       .app_wdf_wren     (DATA_WREN),                    //input
       .app_af_wren      (ADDR_WREN),                    //input  
       .app_af_addr      (MEM_ADDR[30:0]),               //input  [30:0]    
       .app_af_cmd       (MEM_CMD[2:0]),  
       .rd_data_fifo_out (READ_MEM_DATA),                 
       .app_wdf_data     (WRITE_MEM_DATA),                 
       .app_wdf_mask_data(16'b0),                        //input  [15:0]  
       .ddr2_dqs         (DDR2_DQS),                     //inout  [7:0]  
       .ddr2_dqs_n       (DDR2_DQS_N),                   //inout  [7:0]  
       .ddr2_ck          (DDR2_CK),                      //output  [1:0]  
       .ddr2_ck_n        (DDR2_CK_N)                     //output  [1:0]  
    );
    
endmodule 
