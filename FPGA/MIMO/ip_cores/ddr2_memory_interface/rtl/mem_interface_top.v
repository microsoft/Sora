//*****************************************************************************
// Copyright (c) 2006 Xilinx, Inc.
// This design is confidential and proprietary of Xilinx, Inc.
// All Rights Reserved
//*****************************************************************************
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor: Xilinx
// \   \   \/     Version: $Name: i+IP+125372 $
//  \   \         Application: MIG
//  /   /         Filename: mem_interface_top.v
// /___/   /\     Date Last Modified: $Date: 2007/04/18 13:49:32 $
// \   \  /  \    Date Created: Wed Aug 16 2006
//  \___\/\___\
//
//Device: Virtex-5
//Design Name: DDR2
//Purpose:
//   Top-level  module. Simple model for what the user might use
//   Typically, the user will only instantiate MEM_INTERFACE_TOP in their
//   code, and generate all the other infrastructure and backend logic
//   separately. This module serves both as an example, and allows the user
//   to synthesize a self-contained design, which they can use to test their
//   hardware.
//   In addition to the memory controller, the module instantiates:
//     1. Clock generation/distribution, reset logic
//     2. IDELAY control block
//     3. Synthesizable testbench - used to model user's backend logic
//Reference:
//Revision History:
//*****************************************************************************

`timescale 1ns/1ps

module mem_interface_top #
  (
   parameter BANK_WIDTH           = 3,       // # of memory bank addr bits
   parameter CKE_WIDTH            = 1,       // # of memory clock enable outputs
   parameter CLK_WIDTH            = 1,       // # of clock outputs
   parameter COL_WIDTH            = 10,       // # of memory column bits
   parameter CS_NUM               = 1,       // # of separate memory chip selects
   parameter CS_WIDTH             = 1,       // # of total memory chip selects
   parameter CS_BITS              = 0,       // set to log2(CS_NUM) (rounded up)
   parameter DM_WIDTH             = 9,       // # of data mask bits
   parameter DQ_WIDTH             = 72,       // # of data width
   parameter DQ_PER_DQS           = 8,       // # of DQ data bits per strobe
   parameter DQS_WIDTH            = 9,       // # of DQS strobes
   parameter DQ_BITS              = 7,       // set to log2(DQS_WIDTH*DQ_PER_DQS)
   parameter DQS_BITS             = 4,       // set to log2(DQS_WIDTH)
   parameter ODT_WIDTH            = 1,       // # of memory on-die term enables
   parameter ROW_WIDTH            = 14,       // # of memory row and # of addr bits
   parameter ADDITIVE_LAT         = 0,       // additive write latency 
   parameter BURST_LEN            = 4,       // burst length (in double words)
   parameter BURST_TYPE           = 0,       // burst type (=0 seq; =1 interleaved)
   parameter CAS_LAT              = 3,       // CAS latency
   parameter ECC_ENABLE           = 0,       // enable ECC (=1 enable)
   parameter MULTI_BANK_EN        = 1,       // Keeps multiple banks open. (= 1 enable)
   parameter ODT_TYPE             = 0,       // ODT (=0(none),=1(75),=2(150),=3(50))
   parameter REDUCE_DRV           = 0,       // reduced strength mem I/O (=1 yes)
   parameter REG_ENABLE           = 1,       // registered addr/ctrl (=1 yes)
   parameter TREFI_NS             = 7800,       // auto refresh interval (uS)
   parameter TRAS                 = 40000,       // active->precharge delay
   parameter TRCD                 = 15000,       // active->read/write delay
   parameter TRFC                 = 127500,       // refresh->refresh, refresh->active delay
   parameter TRP                  = 15000,       // precharge->command delay
   parameter TRTP                 = 7500,       // read->precharge delay
   parameter TWR                  = 15000,       // used to determine write->precharge
   parameter TWTR                 = 10000,       // write->read delay
   parameter IDEL_HIGH_PERF       = "TRUE",       // # initial # taps for DQ IDELAY
   parameter SIM_ONLY             = 0,       // = 1 to skip SDRAM power up delay
   parameter CLK_PERIOD           = 5000,       // Core/Memory clock period (in ps)
   parameter RST_ACT_LOW          = 1,       // =1 for active low reset, =0 for active high
   parameter DLL_FREQ_MODE        = "HIGH"        // DCM Frequency range
   )
  (
   inout  [DQ_WIDTH-1:0]  ddr2_dq,
   output  [ROW_WIDTH-1:0]  ddr2_a,
   output  [BANK_WIDTH-1:0]  ddr2_ba,
   output  ddr2_ras_n,
   output  ddr2_cas_n,
   output  ddr2_we_n,
   output  [CS_WIDTH-1:0]  ddr2_cs_n,
   output  [ODT_WIDTH-1:0]  ddr2_odt,
   output  [CKE_WIDTH-1:0]  ddr2_cke,
   output  ddr2_reset_n,
   output  [DM_WIDTH-1:0]  ddr2_dm,
////   input  sys_clk_p,
////   input  sys_clk_n,
   input  sys_clk,
   input  clk200_p,
   input  clk200_n,
	/// Jiansong: 200MHz clock output
	output clk200_o,
   input  sys_rst_n,
   output  phy_init_done,
   output rst0_tb,
   output clk0_tb,
   output  app_wdf_afull,
   output  app_af_afull,
   output  rd_data_valid,
   input  app_wdf_wren,
   input  app_af_wren,
   input  [30:0]  app_af_addr,
   input  [2:0]  app_af_cmd,
   output  [(2*DQ_WIDTH)-1:0]  rd_data_fifo_out,
   input  [(2*DQ_WIDTH)-1:0]  app_wdf_data,
   input  [(2*DM_WIDTH)-1:0]  app_wdf_mask_data,
   inout  [DQS_WIDTH-1:0]  ddr2_dqs,
   inout  [DQS_WIDTH-1:0]  ddr2_dqs_n,
   output  [CLK_WIDTH-1:0]  ddr2_ck,
   output  [CLK_WIDTH-1:0]  ddr2_ck_n
   );

  wire  rst0;
  wire  rst90;
  wire  rst200;
  wire  clk0;
  wire  clk90;
  wire  clk200;
  wire  idelay_ctrl_rdy;

  //***************************************************************************
  assign  rst0_tb = rst0;
  assign  clk0_tb = clk0;
  assign  ddr2_reset_n= ~rst0;
  
  /// Jiansong: 
  assign clk200_o = clk200;

mem_interface_top_idelay_ctrl u_idelay_ctrl
  (
   .rst200(rst200),
   .clk200(clk200),
   .idelay_ctrl_rdy(idelay_ctrl_rdy)
   );

mem_interface_top_infrastructure #
  (
   .CLK_PERIOD(CLK_PERIOD),
   .RST_ACT_LOW(RST_ACT_LOW),
   .DLL_FREQ_MODE(DLL_FREQ_MODE)
   )
u_infrastructure
  (
////   .sys_clk_p(sys_clk_p),
////   .sys_clk_n(sys_clk_n),
   .sys_clk(sys_clk),
   .clk200_p(clk200_p),
   .clk200_n(clk200_n),
   .sys_rst_n(sys_rst_n),
   .rst0(rst0),
   .rst90(rst90),
   .rst200(rst200),
   .clk0(clk0),
   .clk90(clk90),
   .clk200(clk200),
   .idelay_ctrl_rdy(idelay_ctrl_rdy)
   );

mem_interface_top_ddr2_top_0 #
  (
   .BANK_WIDTH(BANK_WIDTH),
   .CKE_WIDTH(CKE_WIDTH),
   .CLK_WIDTH(CLK_WIDTH),
   .COL_WIDTH(COL_WIDTH),
   .CS_NUM(CS_NUM),
   .CS_WIDTH(CS_WIDTH),
   .CS_BITS(CS_BITS),
   .DM_WIDTH(DM_WIDTH),
   .DQ_WIDTH(DQ_WIDTH),
   .DQ_PER_DQS(DQ_PER_DQS),
   .DQS_WIDTH(DQS_WIDTH),
   .DQ_BITS(DQ_BITS),
   .DQS_BITS(DQS_BITS),
   .ODT_WIDTH(ODT_WIDTH),
   .ROW_WIDTH(ROW_WIDTH),
   .ADDITIVE_LAT(ADDITIVE_LAT),
   .BURST_LEN(BURST_LEN),
   .BURST_TYPE(BURST_TYPE),
   .CAS_LAT(CAS_LAT),
   .ECC_ENABLE(ECC_ENABLE),
   .MULTI_BANK_EN(MULTI_BANK_EN),
   .ODT_TYPE(ODT_TYPE),
   .REDUCE_DRV(REDUCE_DRV),
   .REG_ENABLE(REG_ENABLE),
   .TREFI_NS(TREFI_NS),
   .TRAS(TRAS),
   .TRCD(TRCD),
   .TRFC(TRFC),
   .TRP(TRP),
   .TRTP(TRTP),
   .TWR(TWR),
   .TWTR(TWTR),
   .IDEL_HIGH_PERF(IDEL_HIGH_PERF),
   .SIM_ONLY(SIM_ONLY),
   .CLK_PERIOD(CLK_PERIOD)
   )
u_ddr2_top_0
  (
   .ddr2_dq(ddr2_dq),
   .ddr2_a(ddr2_a),
   .ddr2_ba(ddr2_ba),
   .ddr2_ras_n(ddr2_ras_n),
   .ddr2_cas_n(ddr2_cas_n),
   .ddr2_we_n(ddr2_we_n),
   .ddr2_cs_n(ddr2_cs_n),
   .ddr2_odt(ddr2_odt),
   .ddr2_cke(ddr2_cke),
   .ddr2_dm(ddr2_dm),
   .phy_init_done(phy_init_done),
   .rst0(rst0),
   .rst90(rst90),
   .clk0(clk0),
   .clk90(clk90),
   .app_wdf_afull(app_wdf_afull),
   .app_af_afull(app_af_afull),
   .rd_data_valid(rd_data_valid),
   .app_wdf_wren(app_wdf_wren),
   .app_af_wren(app_af_wren),
   .app_af_addr(app_af_addr),
   .app_af_cmd(app_af_cmd),
   .rd_data_fifo_out(rd_data_fifo_out),
   .app_wdf_data(app_wdf_data),
   .app_wdf_mask_data(app_wdf_mask_data),
   .ddr2_dqs(ddr2_dqs),
   .ddr2_dqs_n(ddr2_dqs_n),
   .ddr2_ck(ddr2_ck),
   .ddr2_ck_n(ddr2_ck_n)
   );



endmodule
