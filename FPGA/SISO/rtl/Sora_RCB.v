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
// Description: Sora RCB top level wrapper. 
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

`define RCB

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

`ifdef WARPRadio
	 input         RF_DAC_SPI_SDO,
	 output        RF_DAC_SPI_SDI,
	 output        RF_DAC_SPI_CLK,
	 output        RF_DAC_SPI_CSB,
	 output        RF_DAC_RESET,
	 input         RF_DAC_PLLLOCK,
	 output [15:0] RF_DAC_I_DATA,
	 output [15:0] RF_DAC_Q_DATA,
	 
	 output        RF_RSSI_SLEEP,
	 input         RF_RSSI_OTR,
	 output        RF_RSSI_HIZ,
	 output        RF_RSSI_CLK,
	 output        RF_RSSI_CLAMP,
	 input  [9:0]  RF_RSSI_DATA,
	 
	 output        RF_RADIO_TXEN,
	 output        RF_RADIO_RXEN,
	 output        RF_RADIO_SHDN_N,
	 output        RF_RADIO_RXHP,
	 input         RF_RADIO_LD,
	 output        RF_RADIO_SPI_SDI,
	 output        RF_RADIO_SPI_CSB,
	 output        RF_RADIO_SPI_CLK,
	 output [6:0]  RF_RADIO_GAIN,
	 
	 output        RF_ADC_PWDNA,
	 output        RF_ADC_PWDNB,
	 input         RF_ADC_OTRA,
	 input         RF_ADC_OTRB,
	 output        RF_ADC_DFS,
	 output        RF_ADC_DCS,
	 input  [13:0] RF_ADC_DATA_A,
	 input  [13:0] RF_ADC_DATA_B,
	
	 output [2:0]  RF_LED,
	 output        RF_CLK_N,
	 output        RF_CLK_P,
//	 input  [3:0]  RF_DIPSW,
	 output [1:0]  RF_ANTSW,
	 output        RF_5PA_EN_N,
	 output        RF_24PA_EN_N,
`endif

`ifdef SORA_FRL
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
`endif

`ifdef SORA_FRL_2nd
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
`endif
	 
    //-------------------------------------------------------
    // 7. Debug Interface
    //-------------------------------------------------------  

//// Jiansong, 2009-12-10, send RXEnable signal to LED
//	output wire								RXEnable_n,
`ifdef SORA_FRL
	output wire								LED_link_up_and_phy_init_initialization_done_n,
	output wire								Sora_FRL_done_n,
`else
    output wire                            LED_link_up_n,
    output wire                            phy_init_initialization_done_n,
//	output wire								Radio_LO_Lock_n,
`endif
	 /// Jiansong: LED for clock
//	 output wire                            LED_clock,
	 output wire                            Radio_TX_n,
`ifdef RCB
//	 output wire                            LED_clock_1,
	 output wire                            Radio_RX_blink,
`endif
`ifdef RCB
  	   output wire                            nPLOAD_1,
      output wire                            nPLOAD_2	 );
`else
      output wire                            PLOAD_1,
      output wire                            PLOAD_2	 );
`endif
                      


    
    //-------------------------------------------------------
    // Local Wires
    //-------------------------------------------------------

    wire                                              sys_clk_c;
    wire                                              sys_reset_n_c;
    wire                                              trn_clk_c; 

    wire                                    clk_0_from_mem_ctrl_dcm;


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

    wire                                              RST_i;
    
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
    wire  [1:0] ingress_fifo_ctrl;
    wire  [1:0] ingress_fifo_status;
    wire  [2:0] ingress_xfer_size;
    wire [27:6] ingress_start_addr;
    wire        ingress_data_req;
    wire        ingress_data_ack;

    wire [12:0]   pcie_id;
    reg           trn_reset_c;

    reg [12:0] pcie_id_reg;
    reg [2:0] max_pay_size_reg;
    reg [2:0] max_read_req_reg;

    wire pause_read_requests;

    /// Jiansong: wires added by Jiansong
	 wire        hostreset;
	 reg         sys_reset;
    // RX FIFO	 
	 wire [63:0] RX_FIFO_data_out;
	 wire        RX_FIFO_RDEN;
	 wire        RX_FIFO_pempty;
	 wire        RXEnable;
	 wire        RX_FIFO_full;
	 // the 2nd RX FIFO
`ifdef SORA_FRL_2nd
	 wire [63:0] RX_FIFO_2nd_data_out;
	 wire        RX_FIFO_2nd_RDEN;
	 wire        RX_FIFO_2nd_pempty;
	 wire        RX_FIFO_2nd_full;
`endif
	 // Radio Module
////	 wire [31:0] Radio_RX_data;
////    wire [15:0] Radio_RX_loop_data;
`ifdef WARPRadio
    wire [15:0] Radio_RX_data_I;
    wire [15:0] Radio_RX_data_Q;
`endif
`ifdef SORA_FRL
	 wire [31:0] Sora_FRL_RX_data;
	 wire			 Sora_FRL_RX_clk;
	 wire			 Sora_FRL_RX_wren;
`endif
	 // the 2nd Sora FRL
`ifdef SORA_FRL_2nd
	 wire [31:0] Sora_FRL_2nd_RX_data;
	 wire			 Sora_FRL_2nd_RX_clk;
	 wire			 Sora_FRL_2nd_RX_wren;
`endif

	 wire        RX_FIFO_wren;
	 wire        RX_FIFO_wrclk;
	 wire [15:0] Radio_TX_data;
	 wire [1:0]  Radio_TX_FIFO_ctrl;
	 wire [2:0]  Radio_TX_FIFO_status;
	 wire        Radio_TX_done;
	 wire        Radio_TX_start;
	 wire        TX_FIFO_rdclk;
	 // clock wires
//	 wire        clk200;
//	 wire        clk80;
    wire        clk133;
`ifdef WARPRadio
	 wire			 sample_clock_select;		// 0: 44MHz, 1: 40MHz
	 wire			 clk_sample;
	 wire        clk44;
	 wire        clk40;
	 wire        clk11;
`endif
	 wire        sys_clk_ibufg;
	 wire        sys_clk_bufg;
	  /// Jiansong: TX related wires
	 wire        TX_DDR_data_req;
	 wire        TX_DDR_data_ack;
	 wire [27:6] TX_DDR_start_addr;
	 wire [2:0]  TX_DDR_xfer_size;
	 /// Jiansong: error signals
	 wire        egress_overflow_one;
    wire [31:0] egress_rd_data_count;
    wire [31:0] egress_wr_data_count;
	 
    /// Jiansong: pipeline registers for ddr
	 reg [127:0] WRITE_MEM_DATA_r;
    reg  [30:0] MEM_ADDR_r;
    reg  [2:0]  MEM_CMD_r;
    reg         DATA_WREN_r;
    reg         ADDR_WREN_r;

`ifdef SORA_RADIO_REGISTERS
	 wire [1:0]  Sora_RadioControl;
	 wire [31:0] Sora_LEDControl;
	 wire [31:0] Sora_AntennaSelection;
	 wire [31:0] Sora_SampleClock;
	 wire			 Sora_SampleClockSet;
	 wire [31:0] Sora_CoarseFreq;
	 wire [31:0] Sora_FinegradeFreq;
	 wire [31:0] Sora_FreqCompensation;
	 wire			 Sora_CenterFreqSet;
	 wire 		 Sora_RadioLOLock;
	 wire [31:0] Sora_FilterBandwidth;
	 wire			 Sora_FilterBandwidthSet;
	 wire [31:0] Sora_TXVGA1;
	 wire [31:0] Sora_TXVGA2;
	 wire [31:0] Sora_TXPA1;
	 wire [31:0] Sora_TXPA2;
	 wire [31:0] Sora_RXLNA;
	 wire [31:0] Sora_RXPA;
	 wire [31:0] Sora_RXVGA1;
	 wire [31:0] Sora_RXVGA2;
`endif

`ifdef RADIO_CHANNEL_REGISTERS
	 wire [31:0] RChannel_Cmd_Data;
	 wire [6:0]  RChannel_Cmd_Addr;
	 wire			 RChannel_Cmd_RdWr;
	 wire			 RChannel_Cmd_wren;
	 wire	[31:0] RChannel_Reg_Read_Value;
	 wire			 RChannel_ReadDone;
	 
	 wire			 RChannel_Cmd_FIFO_rden;
	 wire [39:0] RChannel_Cmd_FIFO_data;
	 wire			 RChannel_Cmd_FIFO_empty;
`endif		
	
    /// host register to WARP radio interface
`ifdef WARPRadio
	 wire [7:0] host_SPI_DAC_instruct;
	 wire [7:0] host_SPI_DAC_wdata;
	 wire       host_SPI_DAC_start;
	 wire       host_SPI_DAC_done;
	 wire [7:0] host_SPI_DAC_rdata;
	 wire [3:0] host_SPI_Radio_Addr;
	 wire [13:0] host_SPI_Radio_Data;
	 wire       host_SPI_Radio_start;
	 wire       host_SPI_Radio_done;
	 wire       host_Radio_SHDN;
	 wire       host_Radio_Reset;
	 wire       host_Radio_RXHP;
	 wire       host_Radio_LD; 
	 wire [5:0] host_Radio_TXGAIN;
	 wire [6:0] host_Radio_RXGAIN;
	 wire       host_DAC_RESET;
	 wire       host_DAC_PLLLOCK;
	 wire       host_RSSI_SLEEP;
	 wire       host_RSSI_OTR;
	 wire       host_RSSI_HIZ;
	 wire       host_RSSI_CLAMP;
	 wire [9:0] host_RSSI_Data;
	 wire       host_ADC_PWDNA;
	 wire       host_ADC_PWDNB;
	 wire       host_ADC_OTRA;
	 wire       host_ADC_OTRB;
	 wire       host_ADC_DFS;
	 wire       host_ADC_DCS;
	 wire [2:0] host_Radio_LED;
	 wire [1:0] host_Radio_ANTSelect;
	 wire [3:0] host_Radio_DIPSW;
`endif
	 
`ifdef SORA_FRL
	 wire			RX_RST;
	 wire			IDLE_RESET;
`endif
	 
	 /// Jiansong: debug wires
	 wire [31:0] Debug18DDR1;
	 wire [31:0] Debug19DDR2;
	 wire [31:0] Debug23RX4;
	 wire [4:0]  locked_debug;
	 
	 // Jiansong: wires for output signals
//	 wire LED_clock_1_i;
//    wire LED_clock_i;
//	 wire RF_CLK_N_i;
//	 wire RF_CLK_P_i;

	reg		  	Sora_RX_wren_LED;
`ifdef SORA_FRL
	wire			Sora_FRL_linkup;
`endif
	 
    assign pcie_id[12:0] = {cfg_bus_number_c[7:0],
                            cfg_device_number_c[4:0]};
   
    always@(posedge trn_clk_c)begin                 
       max_pay_size_reg[2:0] <= cfg_dcommand_c[7:5];
       max_read_req_reg[2:0] <= cfg_dcommand_c[14:12];
       pcie_id_reg[12:0] <= pcie_id[12:0];
    end

`ifdef RCB
    /// Jiansong: added
    assign nPLOAD_2 = 1'b0;
    
    assign nPLOAD_1 = 1'b0; //Always assert PLOAD_1 to the clock syntheziser 
                           //chip so that a parallel load of the dip switch 
                           //is forced
                           //The Dip switch should be preset to create the 
                           //correct freq. for the DDR2 design
`else
    /// Jiansong: added
    assign PLOAD_2 = 1'b1;
    
    assign PLOAD_1 = 1'b1; //Always assert PLOAD_1 to the clock syntheziser 
                           //chip so that a parallel load of the dip switch 
                           //is forced
                           //The Dip switch should be preset to create the 
                           //correct freq. for the DDR2 design
`endif

  //-------------------------------------------------------
  // System Reset Input Pad Instance
  //-------------------------------------------------------
  // 100 MHz clock
  IBUFDS refclk_ibuf (.O(sys_clk_c), .I(pcie_sys_clk_p), .IB(pcie_sys_clk_n)); 
  IBUF sys_reset_n_ibuf (.O(sys_reset_n_c), .I(sys_reset_n));

//// Jiansong, 2009-12-10, send RXEnable signal to LED
//	OBUF RXEnable_n_OBUF (.O(RXEnable_n), .I(~RXEnable));
`ifdef SORA_FRL
	OBUF Sora_FRL_done_n_OBUF (.O(Sora_FRL_done_n), .I(~Sora_FRL_linkup));
	OBUF LED_link_up_and_phy_init_initialization_done_n_OBUF (.O(LED_link_up_and_phy_init_initialization_done_n),
																				 .I( (~phy_init_initialization_done) | trn_lnk_up_n_c ) );
`else  
	OBUF LED_link_up_n_OBUF(.O(LED_link_up_n), .I(trn_lnk_up_n_c));
	OBUF DDR2_phy_init_n_OBUF(.O(phy_init_initialization_done_n), 
                            .I(~phy_init_initialization_done));
//	OBUF Radio_LO_Lock_n_OBUF (.O(Radio_LO_Lock_n), .I(~host_Radio_LD));
`endif
																				 



  // output buffer to drive board circuits
//  OBUF LED_clock_1_OBUF(.O(LED_clock_1), .I(~LED_clock_1_i));
	OBUF Radio_TX_n_OBUF(.O(Radio_TX_n), .I(~Radio_TX_start));
//`ifdef SORA_FRL
  OBUF Radio_RX_blink_OBUF(.O(Radio_RX_blink), .I(~Sora_RX_wren_LED));
//`else // SORA_FRL
//  OBUF LED_clock_OBUF(.O(LED_clock), .I(~LED_clock_i));
//`endif // SORA_FRL
//  OBUF RF_CLK_N_OBUF(.O(RF_CLK_N), .I(RF_CLK_N_i));
//  OBUF RF_CLK_P_OBUF(.O(RF_CLK_P), .I(RF_CLK_P_i));

  assign cfg_err_cpl_timeout_n_c = ~cfg_err_cpl_timeout;

  always@(posedge trn_clk_c)
     trn_reset_c <= ~trn_reset_n_c;
	  
  always@(posedge sys_clk_bufg)
     sys_reset <= ~sys_reset_n_c;
	  
	/// Jiansong
  //-------------------------------------------------------
  // Clock module
  //-------------------------------------------------------

`ifdef WARPRadio
	Clock_module clock_module_inst(
		// clock module only accepts hardware reset
		.rst(trn_reset_c),
	//	.clk200(clk200),
		.clk200(sys_clk_bufg),
	//	.clk80(clk80),   /// Jiansong: 80MHz clock for radio module
		.clk133(clk133),
		.clk44(clk44),
		.clk40(clk40),
		.clk11(clk11),
		.locked_debug(locked_debug),
		.unlocked_err()    /// whether the clocks are locked
	);
	
	// sample clock selection
	BUFGMUX_CTRL BUFGMUX_CTRL_inst (
		.O		(clk_sample), // Clock MUX output
		.I0	(clk44), // Clock0 input
		.I1	(clk40), // Clock1 input
		.S		(sample_clock_select) // Clock select input
	);
// End of BUFGMUX_CTRL_inst instantiation
`endif

`ifdef SORA_FRL
	Clock_module_FRL clock_module_FRL_inst(
		// clock module only accepts hardware reset
		.rst(trn_reset_c),
	//	.clk200(clk200),
		.clk200(sys_clk_bufg),
		.clk133(clk133),
		.locked_debug(locked_debug),
		.unlocked_err()    /// whether the clocks are locked
	);
`endif

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
//`ifdef SORA_FRL
	reg [27:0] LED_Counter;
`ifdef SORA_FRL
	always@(posedge Sora_FRL_RX_clk) begin
`else
	always@(posedge RX_FIFO_wrclk) begin
`endif
		if (trn_reset_c | hostreset) begin
			LED_Counter <= 28'h000_0000;
			Sora_RX_wren_LED <= 1'b0;
		end else begin
`ifdef SORA_FRL
			if (Sora_FRL_RX_wren)	LED_Counter <= LED_Counter + 28'h000_0001;
`else 
			if (RX_FIFO_wren)	LED_Counter <= LED_Counter + 28'h000_0001;
`endif
			if (LED_Counter == 28'h14FB180) begin	// 22,000,000
				LED_Counter <= 28'h000_0000;
				Sora_RX_wren_LED <= ~Sora_RX_wren_LED;
			end
		end			
	end

  //-------------------------------------------------------
  // Endpoint Implementation Application
  //-------------------------------------------------------

   pcie_dma_wrapper pcie_dma_wrapper_inst (
      .clk(trn_clk_c), 
		.hard_rst(sys_reset),
		.rst(trn_reset_c),
//      .rst(trn_reset_c | hostreset),

      // hot reset to whole system, two cycles
	   .hostreset(hostreset),

      /// Jiansong: interface to RX data fifo
		.RX_FIFO_data(RX_FIFO_data_out),
	   .RX_FIFO_RDEN(RX_FIFO_RDEN),
	   .RX_FIFO_pempty(RX_FIFO_pempty),
		.RXEnable_o(RXEnable),
		
		// interface to 2nd RX data fifo
`ifdef SORA_FRL_2nd
		.RX_FIFO_2nd_data(RX_FIFO_2nd_data_out),
	   .RX_FIFO_2nd_RDEN(RX_FIFO_2nd_RDEN),
	   .RX_FIFO_2nd_pempty(RX_FIFO_2nd_pempty),
//		.RXEnable_o(RXEnable),
`endif

      /// Jiansong: interface to radio module
		.Radio_TX_done(Radio_TX_done),
	   .Radio_TX_start(Radio_TX_start),

      //interface to dma_ddr2_if 
      .ingress_xfer_size(ingress_xfer_size), 
      .ingress_start_addr(ingress_start_addr), 
      .ingress_data_req(ingress_data_req), 
      .ingress_data_ack(ingress_data_ack), 
      .ingress_fifo_status(ingress_fifo_status), 
      .ingress_fifo_ctrl(ingress_fifo_ctrl), 
      .ingress_data(ingress_data),
      .pause_read_requests(pause_read_requests),

      /// Jiansong: TX related inputs/outputs
	   .TX_DDR_data_req(TX_DDR_data_req),
	   .TX_DDR_data_ack(TX_DDR_data_ack),
	   .TX_DDR_start_addr(TX_DDR_start_addr),
	   .TX_DDR_xfer_size(TX_DDR_xfer_size),

      //Misc signals to PCIE Block Plus 
      .pcie_max_pay_size(max_pay_size_reg[2:0]), 
      .pcie_max_read_req(max_read_req_reg[2:0]), 
      .pcie_id(pcie_id_reg[12:0]),
      .comp_timeout(cfg_err_cpl_timeout),

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
`ifdef SORA_FRL_2nd
		.RX_FIFO_2nd_full(RX_FIFO_2nd_full),
`endif
      .egress_rd_data_count(egress_rd_data_count),
      .egress_wr_data_count(egress_wr_data_count),

      //Interface to memory controller
      .phy_init_done(phy_init_initialization_done),

      // Jiansong: HW status register input
      .trn_lnk_up_n_c(trn_lnk_up_n_c),

      // radio related inputs/outputs
`ifdef WARP_RADIO_REGISTERS
	   .RadioAntSelect     (host_Radio_ANTSelect),
		.RadioDIPSW         (host_Radio_DIPSW),
		.RadioLEDControl    (host_Radio_LED),
      .RadioMaximSHDN     (host_Radio_SHDN),
      .RadioMaximReset    (host_Radio_Reset),
      .RadioMaximRXHP     (host_Radio_RXHP),
      .RadioTXGainSetting (host_Radio_TXGAIN),
      .RadioRXGainSetting (host_Radio_RXGAIN),
	   .RadioLD            (host_Radio_LD),
	   .RadioADCControl    ({host_ADC_DFS,host_ADC_DCS,host_ADC_PWDNB,host_ADC_PWDNA}),
		.RadioADCStatus     ({host_ADC_OTRB,host_ADC_OTRA}),
		.RadioDACControl    (host_DAC_RESET),
		.RadioDACStatus     (host_DAC_PLLLOCK),
	   .RadioMaximSPIStart (host_SPI_Radio_start),
	   .RadioMaximSPIDone  (host_SPI_Radio_done),
	   .RadioDACSPIStart   (host_SPI_DAC_start),
	   .RadioDACSPIDone    (host_SPI_DAC_done),
	   .RadioMaximSPIAddr  (host_SPI_Radio_Addr),
	   .RadioMaximSPIData  (host_SPI_Radio_Data),
      .RadioDACSPIData    (host_SPI_DAC_wdata),
      .RadioDACSPIInstuct (host_SPI_DAC_instruct),
	   .RadioDACSPIDataOut (host_SPI_DAC_rdata),
	   .RadioRSSIADCControl({host_RSSI_HIZ,host_RSSI_SLEEP,host_RSSI_CLAMP}),
	   .RadioRSSIData      (host_RSSI_Data),
	   .RadioRSSIOTR       (host_RSSI_OTR),
`endif
`ifdef SORA_RADIO_REGISTERS
		.RadioControl		(Sora_RadioControl),
		.RadioStatus_in	(Sora_RadioStatus_in),
		.RadioID_in			(Sora_RadioID_in),
		.LEDControl			(Sora_LEDControl),
		.AntennaSelection	(Sora_AntennaSelection),
		.SampleClock		(Sora_SampleClock),
		.SampleClockSet	(Sora_SampleClockSet),
		.CoarseFreq			(Sora_CoarseFreq),
		.FinegradeFreq		(Sora_FinegradeFreq),
		.FreqCompensation	(Sora_FreqCompensation),
		.CenterFreqSet		(Sora_CenterFreqSet),
		.RadioLOLock		(Sora_RadioLOLock),
		.FilterBandwidth	(Sora_FilterBandwidth),
		.FilterBandwidthSet(Sora_FilterBandwidthSet),
		.TXVGA1				(Sora_TXVGA1),
		.TXVGA2				(Sora_TXVGA2),
		.TXPA1				(Sora_TXPA1),
		.TXPA2				(Sora_TXPA2),
		.RXLNA				(Sora_RXLNA),
		.RXPA					(Sora_RXPA),
		.RXVGA1				(Sora_RXVGA1),
		.RXVGA2				(Sora_RXVGA2),
`endif
`ifdef RADIO_CHANNEL_REGISTERS
		.Radio_Cmd_Data	(RChannel_Cmd_Data[31:0]),
		.Radio_Cmd_Addr	(RChannel_Cmd_Addr[6:0]),
		.Radio_Cmd_RdWr	(RChannel_Cmd_RdWr),
		.Radio_Cmd_wren	(RChannel_Cmd_wren),
		.Channel_Reg_Read_Value	(RChannel_Reg_Read_Value[31:0]),
		.Channel_ReadDone_in		(RChannel_ReadDone),
`endif		
		//Debug interface
		.PCIeLinkStatus(cfg_lstatus_c),
		.PCIeLinkControl(cfg_lcommand_c),
		.Debug18DDR1(Debug18DDR1),
		.Debug19DDR2(Debug19DDR2),
		.Debug23RX4(Debug23RX4),
		.locked_debug(locked_debug)
	);

  //-------------------------------------------------------
  // RX FIFO
  //-------------------------------------------------------

`ifdef WARPRadio
	/// Jiansong: Data FIFO on RX path, input from radio module and 
	///           output to posted packet generator
	//This is an 4KB FIFO constructed of BRAM
	//Converts the datapath from 32-bit to 64-bit
	RX_data_fifo RX_data_fifo_WARP_inst(
	////   .din    (Radio_RX_data),
	////   .din    (Radio_RX_loop_data),
		.din    ({Radio_RX_data_Q,Radio_RX_data_I}),
		.rd_clk (trn_clk_c),
		.rd_en  (RX_FIFO_RDEN),
		.rst    (trn_reset_c | hostreset),           /// Jiansong: both hardware and host reset
	////   .wr_clk (clk80),
		.wr_clk (RX_FIFO_wrclk),
	////   .wr_en  (~RX_FIFO_full & RXEnable),
	   .wr_en  (RX_FIFO_wren & RXEnable),
//		.wr_en  (RX_FIFO_wren), // Jiansong: disable RX control for loopback
		.dout   (RX_FIFO_data_out),
		.empty  (),                      /// Jiansong: programing full is used
		.full   (RX_FIFO_full),
		.prog_empty (RX_FIFO_pempty)	
	);
`endif

`ifdef SORA_FRL
	/// Jiansong: Data FIFO on RX path, input from Sora FRL and 
	///           output to posted packet generator
	//This is an 4KB FIFO constructed of BRAM
	//Converts the datapath from 32-bit to 64-bit
	RX_data_fifo RX_data_fifo_SORA_FRL_inst(
		.din    (Sora_FRL_RX_data[31:0]),
		.rd_clk (trn_clk_c),
		.rd_en  (RX_FIFO_RDEN),
		.rst    (trn_reset_c | hostreset),           /// Jiansong: both hardware and host reset
		.wr_clk (Sora_FRL_RX_clk),
		.wr_en  (Sora_FRL_RX_wren & RXEnable), // Jiansong: disable RX control for loopback
		.dout   (RX_FIFO_data_out),
		.empty  (),                      /// Jiansong: programing full is used
		.full   (RX_FIFO_full),
		.prog_empty (RX_FIFO_pempty)	
	);
`endif

`ifdef SORA_FRL_2nd
	/// Jiansong: Data FIFO on RX path, input from the 2nd Sora FRL and 
	///           output to posted packet generator
	//This is an 4KB FIFO constructed of BRAM
	//Converts the datapath from 32-bit to 64-bit
	RX_data_fifo RX_data_fifo_SORA_FRL_2nd_inst(
		.din    (Sora_FRL_2nd_RX_data[31:0]),
		.rd_clk (trn_clk_c),
		.rd_en  (RX_FIFO_2nd_RDEN),
		.rst    (trn_reset_c | hostreset),           /// Jiansong: both hardware and host reset
		.wr_clk (Sora_FRL_2nd_RX_clk),
		.wr_en  (Sora_FRL_2nd_RX_wren & RXEnable), // Jiansong: disable RX control for loopback
		.dout   (RX_FIFO_2nd_data_out),
		.empty  (),                      /// Jiansong: programing full is used
		.full   (RX_FIFO_2nd_full),
		.prog_empty (RX_FIFO_2nd_pempty)	
	);
`endif


  //-------------------------------------------------------
  // Radio module
  //-------------------------------------------------------

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++		WARP Radio in GPIO Mode	+++++++++++++++++++++

// Jiansong: this module interprets Sora Radio registers into control signals for WARP radio. This module is used when WARP radio is 
//           directly connected to Sora RCB via GPIOs, and the radio control interface between driver and Sora RCB is "Sora Radio 
//           registers". This design is for backward compatibility. Because at the very beginning, the radio control interface between
//           driver and Sora RCB was "WARP specific radio registers"
//`ifdef SORA_RADIO_REGISTERS
`ifdef SORA_RADIO_REGISTERS_TO_WARP
//`ifdef WARPRadio
	Sora_Radio_Interpretation_for_WARP Sora_Radio_Interpretation_for_WARP_inst(
		 .rst(trn_reset_c | hostreset),    // both hardware and host reset
		 .clk(clk_sample),
		
		// interface to PCIe module (host registers)
		.Sora_RadioControl		(Sora_RadioControl),
		.Sora_RadioStatus_in		(Sora_RadioStatus_in),
		.Sora_RadioID_in			(Sora_RadioID_in),
		.Sora_LEDControl			(Sora_LEDControl),
		.Sora_AntennaSelection	(Sora_AntennaSelection),
		.Sora_SampleClock			(Sora_SampleClock),
		.Sora_SampleClockSet		(Sora_SampleClockSet),
		.Sora_CoarseFreq			(Sora_CoarseFreq),
		.Sora_FinegradeFreq		(Sora_FinegradeFreq),
		.Sora_FreqCompensation	(Sora_FreqCompensation),
		.Sora_CenterFreqSet		(Sora_CenterFreqSet),
		.Sora_RadioLOLock			(Sora_RadioLOLock),
		.Sora_FilterBandwidth	(Sora_FilterBandwidth),
		.Sora_FilterBandwidthSet(Sora_FilterBandwidthSet),
		.Sora_TXVGA1				(Sora_TXVGA1),
		.Sora_TXVGA2				(Sora_TXVGA2),
		.Sora_TXPA1					(Sora_TXPA1),
		.Sora_TXPA2					(Sora_TXPA2),
		.Sora_RXLNA					(Sora_RXLNA),
		.Sora_RXPA					(Sora_RXPA),
		.Sora_RXVGA1				(Sora_RXVGA1),
		.Sora_RXVGA2				(Sora_RXVGA2),
		
		// interface to WARP radio module
		 .host_SPI_DAC_instruct (host_SPI_DAC_instruct),
		 .host_SPI_DAC_wdata    (host_SPI_DAC_wdata),
		 .host_SPI_DAC_start    (host_SPI_DAC_start),
		 .host_SPI_DAC_done     (host_SPI_DAC_done),
		 .host_SPI_DAC_rdata    (host_SPI_DAC_rdata),
		 
		 .host_SPI_Radio_Addr   (host_SPI_Radio_Addr),
		 .host_SPI_Radio_Data   (host_SPI_Radio_Data),
		 .host_SPI_Radio_start  (host_SPI_Radio_start),
		 .host_SPI_Radio_done   (host_SPI_Radio_done),
		 
		 .host_Radio_SHDN       (host_Radio_SHDN),
		 .host_Radio_Reset      (host_Radio_Reset),
		 .host_Radio_RXHP       (host_Radio_RXHP),
		 .host_Radio_LD         (host_Radio_LD),
		 .host_Radio_TXGAIN     (host_Radio_TXGAIN),
		 .host_Radio_RXGAIN     (host_Radio_RXGAIN),
		 
		 .host_DAC_RESET        (host_DAC_RESET),
		 .host_DAC_PLLLOCK      (host_DAC_PLLLOCK),
		 
		 .host_RSSI_SLEEP       (host_RSSI_SLEEP),
		 .host_RSSI_OTR         (host_RSSI_OTR),
		 .host_RSSI_HIZ         (host_RSSI_HIZ),
		 .host_RSSI_CLAMP       (host_RSSI_CLAMP),
		 .host_RSSI_Data        (host_RSSI_Data),
		 
		 .host_ADC_PWDNA        (host_ADC_PWDNA),
		 .host_ADC_PWDNB        (host_ADC_PWDNB),
		 .host_ADC_OTRA         (host_ADC_OTRA),
		 .host_ADC_OTRB         (host_ADC_OTRB),
		 .host_ADC_DFS          (host_ADC_DFS),
		 .host_ADC_DCS          (host_ADC_DCS),
		 
		 .host_Radio_LED        (host_Radio_LED),
		 .host_Radio_ANTSelect  (host_Radio_ANTSelect),
		 .host_Radio_DIPSW      (host_Radio_DIPSW)
	);
//`endif
`endif

// Jiansong: The basic function is the same as Sora_Radio_Interpretation_for_WARP. The difference is the interface. Radio Register 
//				 Table is implemented in this module and no longer mapped into RCB Register Table. The interface becomes basic function for
//				 writing radio registers and reading radio registers
//`ifdef RADIO_CHANNEL_REGISTERS
`ifdef RADIO_CHANNEL_REGISTERS_TO_WARP
	// This FIFO acts as buffer for radio control commands from DMA module to radio module
	Radio_Register_set_FIFO Radio_Register_set_FIFO_inst(
		.din		({RChannel_Cmd_RdWr, RChannel_Cmd_Addr, RChannel_Cmd_Data}),
		.rd_clk	(clk_sample),
		.rd_en	(RChannel_Cmd_FIFO_rden),
		.rst		(trn_reset_c | hostreset),
		.wr_clk	(trn_clk_c),
		.wr_en	(RChannel_Cmd_wren),
		.dout		(RChannel_Cmd_FIFO_data),
		.empty	(RChannel_Cmd_FIFO_empty),
		.full		(),
		.sbiterr	(),
		.dbiterr	()		
	);

	Channel_Radio_Interpretation_for_WARP Channel_Radio_Interpretation_for_WARP_inst(
		 .rst(trn_reset_c | hostreset),    // both hardware and host reset
		 .clk(clk_sample),
		 .sample_clock_select			(sample_clock_select),
		
		// interface to PCIe module (host registers)
		 .RChannel_Cmd_FIFO_rden	(RChannel_Cmd_FIFO_rden),
		 .RChannel_Cmd_FIFO_data	(RChannel_Cmd_FIFO_data[39:0]),
		 .RChannel_Cmd_FIFO_empty	(RChannel_Cmd_FIFO_empty),
		 .RChannel_Reg_Read_Value	(RChannel_Reg_Read_Value),
		 .RChannel_Reg_Read_Done	(RChannel_ReadDone),
		
		// interface to WARP radio module
		 .host_SPI_DAC_instruct (host_SPI_DAC_instruct),
		 .host_SPI_DAC_wdata    (host_SPI_DAC_wdata),
		 .host_SPI_DAC_start    (host_SPI_DAC_start),
		 .host_SPI_DAC_done     (host_SPI_DAC_done),
		 .host_SPI_DAC_rdata    (host_SPI_DAC_rdata),
		 
		 .host_SPI_Radio_Addr   (host_SPI_Radio_Addr),
		 .host_SPI_Radio_Data   (host_SPI_Radio_Data),
		 .host_SPI_Radio_start  (host_SPI_Radio_start),
		 .host_SPI_Radio_done   (host_SPI_Radio_done),
		 
		 .host_Radio_SHDN       (host_Radio_SHDN),
		 .host_Radio_Reset      (host_Radio_Reset),
		 .host_Radio_RXHP       (host_Radio_RXHP),
		 .host_Radio_LD         (host_Radio_LD),
		 .host_Radio_TXGAIN     (host_Radio_TXGAIN),
		 .host_Radio_RXGAIN     (host_Radio_RXGAIN),
		 
		 .host_DAC_RESET        (host_DAC_RESET),
		 .host_DAC_PLLLOCK      (host_DAC_PLLLOCK),
		 
		 .host_RSSI_SLEEP       (host_RSSI_SLEEP),
		 .host_RSSI_OTR         (host_RSSI_OTR),
		 .host_RSSI_HIZ         (host_RSSI_HIZ),
		 .host_RSSI_CLAMP       (host_RSSI_CLAMP),
		 .host_RSSI_Data        (host_RSSI_Data),
		 
		 .host_ADC_PWDNA        (host_ADC_PWDNA),
		 .host_ADC_PWDNB        (host_ADC_PWDNB),
		 .host_ADC_OTRA         (host_ADC_OTRA),
		 .host_ADC_OTRB         (host_ADC_OTRB),
		 .host_ADC_DFS          (host_ADC_DFS),
		 .host_ADC_DCS          (host_ADC_DCS),
		 
		 .host_Radio_LED        (host_Radio_LED),
		 .host_Radio_ANTSelect  (host_Radio_ANTSelect),
		 .host_Radio_DIPSW      (host_Radio_DIPSW)
	);
`endif		

// Jiansong: WARP radio module. If WARP radio is directly connected to Sora RCB via GPIOs, this module should be used. Otherwise 
//           if (whatever) radio module is connected to Sora RCB via LVDS, this module should not be used.
`ifdef WARPRadio
	WARP_Radio_module WARP_radio_module_inst(
		 .rst(trn_reset_c | hostreset),    // both hardware and host reset
		 .data_clk(clk_sample),

		 .spi_clk(clk11),	 
		 // Control from internal logic
		 .RXEnable(RXEnable),
		 .TXStart(Radio_TX_start),	 
		 // Status to internal logic
		 .RadioTXDone(Radio_TX_done),
		 .egress_rd_data_count(egress_rd_data_count),
		 // interfaces to internal FIFOs
		 .TX_FIFO_rdclk(TX_FIFO_rdclk),
		 .TX_FIFO_DataI(Radio_TX_data[7:0]),
		 .TX_FIFO_DataQ(Radio_TX_data[15:8]),
		 .TX_FIFO_Control(Radio_TX_FIFO_ctrl),
		 .TX_FIFO_Status(Radio_TX_FIFO_status),
		 .RX_FIFO_wrclk(RX_FIFO_wrclk),
		 .RX_FIFO_Control(RX_FIFO_wren),
		 .RX_FIFO_Status(RX_FIFO_full),
		 .RX_FIFO_DataI(Radio_RX_data_I), 
		 .RX_FIFO_DataQ(Radio_RX_data_Q),
		 // interfaces to internal logic/host registers
		 .host_SPI_DAC_instruct (host_SPI_DAC_instruct),
		 .host_SPI_DAC_wdata    (host_SPI_DAC_wdata),
		 .host_SPI_DAC_start    (host_SPI_DAC_start),
		 .host_SPI_DAC_done     (host_SPI_DAC_done),
		 .host_SPI_DAC_rdata    (host_SPI_DAC_rdata),
		 
		 .host_SPI_Radio_Addr   (host_SPI_Radio_Addr),
		 .host_SPI_Radio_Data   (host_SPI_Radio_Data),
		 .host_SPI_Radio_start  (host_SPI_Radio_start),
		 .host_SPI_Radio_done   (host_SPI_Radio_done),
		 
		 .host_Radio_SHDN       (host_Radio_SHDN),
		 .host_Radio_Reset      (host_Radio_Reset),
		 .host_Radio_RXHP       (host_Radio_RXHP),
		 .host_Radio_LD         (host_Radio_LD),
		 .host_Radio_TXGAIN     (host_Radio_TXGAIN),
		 .host_Radio_RXGAIN     (host_Radio_RXGAIN),
		 
		 .host_DAC_RESET        (host_DAC_RESET),
		 .host_DAC_PLLLOCK      (host_DAC_PLLLOCK),
		 
		 .host_RSSI_SLEEP       (host_RSSI_SLEEP),
		 .host_RSSI_OTR         (host_RSSI_OTR),
		 .host_RSSI_HIZ         (host_RSSI_HIZ),
		 .host_RSSI_CLAMP       (host_RSSI_CLAMP),
		 .host_RSSI_Data        (host_RSSI_Data),
		 
		 .host_ADC_PWDNA        (host_ADC_PWDNA),
		 .host_ADC_PWDNB        (host_ADC_PWDNB),
		 .host_ADC_OTRA         (host_ADC_OTRA),
		 .host_ADC_OTRB         (host_ADC_OTRB),
		 .host_ADC_DFS          (host_ADC_DFS),
		 .host_ADC_DCS          (host_ADC_DCS),
		 
		 .host_Radio_LED        (host_Radio_LED),
		 .host_Radio_ANTSelect  (host_Radio_ANTSelect),
		 .host_Radio_DIPSW      (host_Radio_DIPSW),
		 
		 // interfaces to pads/WARP Radio board
		 .RF_DAC_SPI_SDO        (RF_DAC_SPI_SDO),
		 .RF_DAC_SPI_SDI        (RF_DAC_SPI_SDI),
		 .RF_DAC_SPI_CLK        (RF_DAC_SPI_CLK),
		 .RF_DAC_SPI_CSB        (RF_DAC_SPI_CSB),
		 .RF_DAC_RESET          (RF_DAC_RESET),
		 .RF_DAC_PLLLOCK        (RF_DAC_PLLLOCK),
		 .RF_DAC_I_DATA         (RF_DAC_I_DATA),
		 .RF_DAC_Q_DATA         (RF_DAC_Q_DATA),
		 
		 .RF_RSSI_SLEEP         (RF_RSSI_SLEEP),
		 .RF_RSSI_OTR           (RF_RSSI_OTR),
		 .RF_RSSI_HIZ           (RF_RSSI_HIZ),
		 .RF_RSSI_CLK           (RF_RSSI_CLK),
		 .RF_RSSI_CLAMP         (RF_RSSI_CLAMP),
		 .RF_RSSI_DATA          (RF_RSSI_DATA),
		 
		 .RF_RADIO_TXEN         (RF_RADIO_TXEN),
		 .RF_RADIO_RXEN         (RF_RADIO_RXEN),
		 .RF_RADIO_SHDN_N       (RF_RADIO_SHDN_N),
		 .RF_RADIO_RXHP         (RF_RADIO_RXHP),
		 .RF_RADIO_LD           (RF_RADIO_LD),
		 .RF_RADIO_SPI_SDI      (RF_RADIO_SPI_SDI),
		 .RF_RADIO_SPI_CSB      (RF_RADIO_SPI_CSB),
		 .RF_RADIO_SPI_CLK      (RF_RADIO_SPI_CLK),
		 .RF_RADIO_GAIN         (RF_RADIO_GAIN),
		 
		 .RF_ADC_PWDNA          (RF_ADC_PWDNA),
		 .RF_ADC_PWDNB          (RF_ADC_PWDNB),
		 .RF_ADC_OTRA           (RF_ADC_OTRA),
		 .RF_ADC_OTRB           (RF_ADC_OTRB),
		 .RF_ADC_DFS            (RF_ADC_DFS),
		 .RF_ADC_DCS            (RF_ADC_DCS),
		 .RF_ADC_DATA_A         (RF_ADC_DATA_A),
		 .RF_ADC_DATA_B         (RF_ADC_DATA_B),
		
		 .RF_LED                (RF_LED),
		 .RF_CLK_N              (RF_CLK_N),
		 .RF_CLK_P              (RF_CLK_P),
		 .RF_ANTSW              (RF_ANTSW),
		 .RF_5PA_EN_N           (RF_5PA_EN_N),
		 .RF_24PA_EN_N          (RF_24PA_EN_N),
		 
		 // debug output
		 .Debug23RX4(Debug23RX4)
	);
`endif

//+++++++++++++++		WARP Radio in Parallel Mode	+++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//`ifndef sora_simulation				// we use parallel mode in simulation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++			Sora Fast Radio Link			+++++++++++++++++++++
`ifdef SORA_FRL
	Sora_FRL_RCB Sora_FRL_RCB_inst(
			// signals to FPGA pins
         .CLK_O_p					(Sora_FRL_CLK_O_p), 
			.CLK_O_n					(Sora_FRL_CLK_O_n), 
			.CLK_I_p					(Sora_FRL_CLK_I_p), 
			.CLK_I_n					(Sora_FRL_CLK_I_n),								
			.MSG_OUT_p				(Sora_FRL_MSG_OUT_p),
			.MSG_OUT_n				(Sora_FRL_MSG_OUT_n),
			.MSG_IN_p				(Sora_FRL_MSG_IN_p),
			.MSG_IN_n				(Sora_FRL_MSG_IN_n),
			.DATA_IN_p				(Sora_FRL_DATA_IN_p),
			.DATA_IN_n				(Sora_FRL_DATA_IN_n), 
			.DATA_OUT_p				(Sora_FRL_DATA_OUT_p), 
			.DATA_OUT_n				(Sora_FRL_DATA_OUT_n),
			.STATUS_IN_p			(Sora_FRL_STATUS_IN_p),
			.STATUS_IN_n			(Sora_FRL_STATUS_IN_n),
			.STATUS_OUT_p			(Sora_FRL_STATUS_OUT_p),
			.STATUS_OUT_n			(Sora_FRL_STATUS_OUT_n),
			
			// signals to internal modules
			.RST_internal			(trn_reset_c | hostreset),
			.SEND_EN					(~Radio_TX_FIFO_status[0]),
			.DATA_INT_IN_3			(Radio_TX_data[7:0]),
			.DATA_INT_IN_2			(8'h00),
			.DATA_INT_IN_1			(Radio_TX_data[15:8]),
			.DATA_INT_IN_0			(8'h00),
			.CLKRD_INT_DATA_IN	(TX_FIFO_rdclk),
			.RDEN_DATA_INT_IN		(Radio_TX_FIFO_ctrl[0]),
			.DATA_INT_OUT			(Sora_FRL_RX_data[31:0]),
			.CLKWR_INT_DATA_OUT	(Sora_FRL_RX_clk),
			.WREN_DATA_INT_OUT	(Sora_FRL_RX_wren),
			.DATA_ALMOSTEMPTY		(),
			.SEND_EN_MSG			(1'b1),
			.MSG_INT_IN				({RChannel_Cmd_RdWr,RChannel_Cmd_Addr[6:0],RChannel_Cmd_Data[31:0]}),
			.CLKWR_INT_MSG_IN		(trn_clk_c),
			.WREN_MSG_INT_IN		(RChannel_Cmd_wren),
			// modified by Jiansong, 2010-5-27, we do not use FIFO to buf MSG_INT_OUT
			.MSG_INT_OUT_Data		(RChannel_Reg_Read_Value[31:0]),
			.MSG_INT_OUT_Addr		(),
			.MSG_Valid				(RChannel_ReadDone),
			.STATUS_INT_OUT_CORRECT(),
			.STATUS_INT_OUT_WRONG(),
			.TxInit					(Radio_TX_start),
			.TxDone					(Radio_TX_done),
			.Radio_status_error	(),
			.Sora_FRL_linkup		(Sora_FRL_linkup),
			.LED						()
			);
`endif

`ifdef SORA_FRL_2nd
	Sora_FRL_RCB Sora_FRL_RCB_2nd_inst(
			// signals to FPGA pins
         .CLK_O_p					(Sora_FRL_2nd_CLK_O_p), 
			.CLK_O_n					(Sora_FRL_2nd_CLK_O_n), 
			.CLK_I_p					(Sora_FRL_2nd_CLK_I_p), 
			.CLK_I_n					(Sora_FRL_2nd_CLK_I_n),								
			.MSG_OUT_p				(Sora_FRL_2nd_MSG_OUT_p),
			.MSG_OUT_n				(Sora_FRL_2nd_MSG_OUT_n),
			.MSG_IN_p				(Sora_FRL_2nd_MSG_IN_p),
			.MSG_IN_n				(Sora_FRL_2nd_MSG_IN_n),
			.DATA_IN_p				(Sora_FRL_2nd_DATA_IN_p),
			.DATA_IN_n				(Sora_FRL_2nd_DATA_IN_n), 
			.DATA_OUT_p				(Sora_FRL_2nd_DATA_OUT_p), 
			.DATA_OUT_n				(Sora_FRL_2nd_DATA_OUT_n),
			.STATUS_IN_p			(Sora_FRL_2nd_STATUS_IN_p),
			.STATUS_IN_n			(Sora_FRL_2nd_STATUS_IN_n),
			.STATUS_OUT_p			(Sora_FRL_2nd_STATUS_OUT_p),
			.STATUS_OUT_n			(Sora_FRL_2nd_STATUS_OUT_n),
			
			// signals to internal modules
			.RST						(trn_reset_c | RX_RST | IDLE_RESET),  //xt
			.SEND_EN					(~Radio_TX_FIFO_status[0]),
			.DATA_INT_IN_3			(Radio_TX_data[7:0]),
			.DATA_INT_IN_2			(8'h00),
			.DATA_INT_IN_1			(Radio_TX_data[15:8]),
			.DATA_INT_IN_0			(8'h00),
			.CLKRD_INT_DATA_IN	(TX_FIFO_rdclk),
			.RDEN_DATA_INT_IN		(Radio_TX_FIFO_ctrl[0]),
			.DATA_INT_OUT			(Sora_FRL_2nd_RX_data[31:0]),
			.CLKWR_INT_DATA_OUT	(Sora_FRL_2nd_RX_clk),
			.WREN_DATA_INT_OUT	(Sora_FRL_2nd_RX_wren),
			.DATA_ALMOSTEMPTY		(),
			.SEND_EN_MSG			(1'b1),
			.MSG_INT_IN				({RChannel_Cmd_RdWr,RChannel_Cmd_Addr[6:0],RChannel_Cmd_Data[31:0]}),
			.CLKWR_INT_MSG_IN		(trn_clk_c),
			.WREN_MSG_INT_IN		(RChannel_Cmd_wren),
			.MSG_INT_OUT			(),
			.CLKRD_INT_MSG_OUT	(1'b0),
			.RDEN_MSG_INT_OUT		(1'b0),
			.STATUS_INT_OUT_CORRECT(),
			.STATUS_INT_OUT_WRONG(),
			.TxInit					(Radio_TX_start),
			.TxDone					(Radio_TX_done),
			.RX_RST 					(RX_RST),   //xt
			.IDLE_RESET				(IDLE_RESET),			//xt
			.LED						()
			);
`endif
//`endif
//+++++++++++++++			Sora Fast Radio Link			+++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  //-------------------------------------------------------
  // Block Plus Core for PCI Express Instance
  //-------------------------------------------------------

`ifdef sora_simulation
  endpoint_blk_plus_v1_8    ep  (       /// Jiansong: used for simulation
`else
//  endpoint_blk_plus_v1_9    ep  (  /// Jiansong: used for synthesize
  endpoint_blk_plus_v1_14    ep  (  /// Jiansong: used for synthesize
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
      .sys_reset_n( sys_reset_n_c),              // I 

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

    dma_ddr2_if dma_ddr2_if_inst(
	    .radio_clk           (TX_FIFO_rdclk),
       .dma_clk             (trn_clk_c),
       .ddr_clk             (clk_0_from_mem_ctrl_dcm),
       .reset               (SYS_RST_FROM_MEM_CTRL | trn_reset_c | hostreset),

       //DMA SIGNALS
       //egress
       .egress_data         (Radio_TX_data),
       .egress_fifo_ctrl    (Radio_TX_FIFO_ctrl),
       .egress_fifo_status  (Radio_TX_FIFO_status),
       .egress_xfer_size    (TX_DDR_xfer_size),
       .egress_start_addr   (TX_DDR_start_addr),
       .egress_data_req     (TX_DDR_data_req),
       .egress_data_ack     (TX_DDR_data_ack),		 
       
       //ingress
       .ingress_data        (ingress_data),
       .ingress_fifo_ctrl   (ingress_fifo_ctrl),   //bit 1 = reserved  
                                                   //bit 0 = write_en
       .ingress_fifo_status (ingress_fifo_status), //bit 1 = full      
                                                   //bit 0 = almostfull
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
       .Debug18DDR1(Debug18DDR1),
		 .Debug19DDR2(Debug19DDR2)
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
       .sys_rst_n        (sys_reset_n_c),    // DCM need multi-cycle reset 
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
