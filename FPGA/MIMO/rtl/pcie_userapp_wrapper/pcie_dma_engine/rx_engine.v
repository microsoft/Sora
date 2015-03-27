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
// Module Name:    rx_engine 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: RX Engine Wrapper file.  Connects all of the individual modules
// and FIFOs that interface between the Block Plus LogiCore and the DMA-DDR
// Interface Block (dma_ddr2_if)      
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//  modified by Jiansong Zhang:
//     (1)add 1 bit to differentiate TX descriptor from normal data ---- done
//     (2)TX des parser ------------------------------------------------ done
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps
`include "Sora_config.v"

module rx_engine(
	input  wire         clk,
	input  wire         rst,
	//interface to dma_ddr2_if
	output wire [127:0] ingress_data,
//	output wire   [1:0] ingress_fifo_ctrl,   //bit 1 = unused  bit 0 = write_en
	output				ingress_fifo_wren,
//	input  wire   [1:0] ingress_fifo_status, //bit 1 = full    bit 0 = almostfull
	output wire   [2:0] ingress_xfer_size,
	output wire  [27:6] ingress_start_addr,
	output wire         ingress_data_req,
	input  wire         ingress_data_ack,
	//interface to dma_ctrl_wrapper
	input  wire         rd_dma_start,  //indicates the start of a read dma xfer
	input  wire  [31:0] dmarad,        //destination addres(ddr2) only 13 bits used   /// why put destination here?
	input  wire  [31:0] dmarxs, 
	output wire         rd_dma_done,   //dma transfer complete   /// Jiansong: it's fake? not fake
	///Jiansong: interface to dma control wrapper
	output              new_des_one,
	output wire        [31:0]  SourceAddr_L,
	output wire        [31:0]  SourceAddr_H,
	output wire        [31:0]  DestAddr,
	output wire        [23:0]  FrameSize,
	output wire        [7:0]   FrameControl,
	/// Jiansong: interface from dma control wrapper
	input  wire        Wait_for_TX_desc,
`ifdef TF_RECOVERY
	input  wire        transferstart,     // control signal for transfer recovering
`endif
	//interface to read_request_wrapper
	output [4:0] rx_waddr,
	output [31:0] rx_wdata,
	output rx_we,
	output [4:0] rx_raddr,
	input [31:0] rx_rdata,
	output  pending_comp_done,
	input [31:0] completion_pending,
	//interface from PCIe Endpoint Block Plus - RX TRN
	input wire [63:0] trn_rd,
	input wire [7:0] trn_rrem_n,
	input wire trn_rsof_n,
	input wire trn_reof_n,
	input wire trn_rsrc_rdy_n,
	input wire trn_rsrc_dsc_n,
	output wire trn_rdst_rdy_n,
	input wire trn_rerrfwd_n,
	output wire trn_rnp_ok_n,
	input wire [6:0] trn_rbar_hit_n,
	input wire [11:0] trn_rfc_npd_av,
	input wire [7:0] trn_rfc_nph_av,
	input wire [11:0] trn_rfc_pd_av,
	input wire [7:0] trn_rfc_ph_av,
	input wire [11:0] trn_rfc_cpld_av,
	input wire [7:0] trn_rfc_cplh_av,
	output wire trn_rcpl_streaming_n,
	//interface to TX Engine (completer_pkt_gen)
	output wire [6:0] bar_hit_o,
	output wire MRd_o,                
	output wire MWr_o,   
	output wire [31:0] MEM_addr_o,
	output wire [15:0] MEM_req_id_o,
	output wire [7:0] MEM_tag_o,
	output wire header_fields_valid_o,
	output wire [31:0] write_data,
	output wire write_data_wren,
	input wire  read_last
	/// Jiansong: output to tx_sm
//	output wire [9:0] np_rx_cnt_qw
//	/// Jiansong: debug register
//	output [9:0] Debug30RXEngine,
//	output reg [11:0] Debug31RXDataFIFOfullcnt,
//	output reg [11:0] Debug32RXXferFIFOfullcnt,
//	output reg [23:0] Debug33RXDataFIFOWRcnt,
//	output reg [23:0] Debug34RXDataFIFORDcnt,
//	output reg [23:0] Debug35RXXferFIFOWRcnt,
//	output reg [23:0] Debug36RXXferFIFORDcnt
);


wire  [27:6] mem_dest_addr;
wire  [10:0] mem_dma_size;
wire         mem_dma_start;

wire  [63:0] write_data_fifo_data;
wire         write_data_fifo_cntrl;
wire         write_data_fifo_status;
wire [127:0] read_data_fifo_data;
wire         read_data_fifo_cntrl;
wire         read_data_fifo_status;

wire fourdw_n_threedw; //fourdw = 1'b1; 3dw = 1'b0;
wire payload;
wire [2:0] tc; 
wire td; 
wire ep;  
wire [1:0] attr;
wire [9:0] dw_length;
wire [15:0] MEM_req_id;
wire [7:0] MEM_tag;
wire [15:0] CMP_comp_id;
wire [2:0] CMP_compl_stat;
wire CMP_bcm;
wire [11:0] CMP_byte_count;
wire [63:0] MEM_addr;  
wire [15:0] CMP_req_id;
wire [7:0] CMP_tag;
wire [6:0] CMP_lower_addr;
wire MRd;
wire MWr;
wire CplD;
reg  CplD_r;   // Jiansong: pipeline register to solve Mrd write to fifo bug
wire Msg;
wire UR;
wire [6:0] bar_hit;
wire header_fields_valid;

reg [63:0] trn_rd_reg;
reg [7:0] trn_rrem_reg_n;
reg trn_rsof_reg_n;
reg trn_reof_reg_n;
reg trn_rsrc_rdy_reg_n;
reg trn_rsrc_dsc_reg_n;
reg trn_rerrfwd_reg_n;
reg [6:0] trn_rbar_hit_reg_n;
reg [11:0] trn_rfc_npd_av_reg;
reg [7:0] trn_rfc_nph_av_reg;
reg [11:0] trn_rfc_pd_av_reg;
reg [7:0] trn_rfc_ph_av_reg;
reg [11:0] trn_rfc_cpld_av_reg;
reg [7:0] trn_rfc_cplh_av_reg;

wire read_xfer_fifo_status;
wire write_xfer_fifo_status;
wire xfer_trn_mem_fifo_rden;
wire  [27:6] mem_dest_addr_fifo;
wire  [10:0] mem_dma_size_fifo;

/// Jiansong: added for TX des
wire isDes;
wire isDes_fifo;

//assign some outputs 
assign bar_hit_o[6:0] = bar_hit[6:0];
assign MRd_o = MRd;                
assign MWr_o = MWr;   
assign MEM_addr_o[31:0] = MEM_addr[31:0];
assign MEM_req_id_o[15:0] = MEM_req_id[15:0];
assign MEM_tag_o[7:0] = MEM_tag[7:0];
assign header_fields_valid_o = header_fields_valid;
//send the write data info and wren enable signal out to write Posted MemWr
//data to the dma_ctrl_wrapper (or any other target block for that matter)
assign write_data[31:0] = write_data_fifo_data[31:0]; 
assign write_data_wren = write_data_fifo_cntrl;

//all the outputs of the endpoint need to be pipelined
//to meet 250 MHz timing of an 8 lane design
always @ (posedge clk)
begin
    trn_rd_reg[63:0]          <= trn_rd[63:0]         ;
    trn_rrem_reg_n[7:0]       <= trn_rrem_n[7:0]      ;
    trn_rsof_reg_n            <= trn_rsof_n           ;
    trn_reof_reg_n            <= trn_reof_n           ;
    trn_rsrc_rdy_reg_n        <= trn_rsrc_rdy_n       ;
    trn_rsrc_dsc_reg_n        <= trn_rsrc_dsc_n       ;
    trn_rerrfwd_reg_n         <= trn_rerrfwd_n        ;
    trn_rbar_hit_reg_n[6:0]   <= trn_rbar_hit_n[6:0]  ;
    trn_rfc_npd_av_reg[11:0]  <= trn_rfc_npd_av[11:0] ;
    trn_rfc_nph_av_reg[7:0]   <= trn_rfc_nph_av[7:0]  ;
    trn_rfc_pd_av_reg[11:0]   <= trn_rfc_pd_av[11:0]  ;
    trn_rfc_ph_av_reg[7:0]    <= trn_rfc_ph_av[7:0]   ;
    trn_rfc_cpld_av_reg[11:0] <= trn_rfc_cpld_av[11:0];
    trn_rfc_cplh_av_reg[7:0]  <= trn_rfc_cplh_av[7:0] ;
end



//Instantiate the Receive TRN Monitor block
//This module interfaces to the DMA
//Control/Status Register File and the Read Request Fifo to determine when a 
//DMA transfer has completed fully.  
rx_trn_monitor rx_trn_monitor_inst(
   .clk                (clk),
   .rst                (rst),
   // interface to dma_ctrl_wrapper
   .rd_dma_start       (rd_dma_start), 
   .dmarad             (dmarad[31:0]),        
   .dmarxs             (dmarxs[31:0]),  
   .rd_dma_done        (rd_dma_done), 
   .read_last          (read_last),
	///Jiansong: signal from dma control wrapper
	.Wait_for_TX_desc   (Wait_for_TX_desc),
`ifdef TF_RECOVERY
	.transferstart      (transferstart),
`endif
   //interface to read_request_wrapper 
   .rx_waddr (rx_waddr[4:0]),
   .rx_wdata  (rx_wdata[31:0]),
   .rx_we (rx_we),
   .rx_raddr  (rx_raddr[4:0]),
   .rx_rdata(rx_rdata[31:0]),
   .pending_comp_done(pending_comp_done),
   .completion_pending(completion_pending[31:0]),
   //PCIe Endpoint Block Plus interface
   // RX TRN
   .trn_rd             (trn_rd_reg),         // I [63/31:0]
   .trn_rrem_n         (trn_rrem_reg_n),     // I [7:0]
   .trn_rsof_n         (trn_rsof_reg_n),     // I
   .trn_reof_n         (trn_reof_reg_n),     // I
   .trn_rsrc_rdy_n     (trn_rsrc_rdy_reg_n), // I
   .trn_rsrc_dsc_n     (trn_rsrc_dsc_reg_n), // I
   .trn_rerrfwd_n      (trn_rerrfwd_reg_n),  // I
   .trn_rbar_hit_n     (trn_rbar_hit_reg_n), // I [6:0]
   .trn_rfc_npd_av     (trn_rfc_npd_av_reg), // I [11:0]
   .trn_rfc_nph_av     (trn_rfc_nph_av_reg), // I [7:0]
   .trn_rfc_pd_av      (trn_rfc_pd_av_reg),  // I [11:0]
   .trn_rfc_ph_av      (trn_rfc_ph_av_reg),  // I [7:0]
   .trn_rfc_cpld_av    (trn_rfc_cpld_av_reg),// I [11:0]
   .trn_rfc_cplh_av    (trn_rfc_cplh_av_reg),// I [7:0]
   //interface from rx_trn_data_fsm 
   .fourdw_n_threedw   (fourdw_n_threedw), 
   .payload            (payload),
   .tc                 (tc[2:0]), 
   .td                 (td), 
   .ep                 (ep),  
   .attr               (attr[1:0]),
   .dw_length          (dw_length[9:0]),
   .MEM_req_id         (MEM_req_id[15:0]),
   .MEM_tag            (MEM_tag[7:0]),
   .CMP_comp_id        (CMP_comp_id[15:0]),
   .CMP_compl_stat     (CMP_compl_stat[2:0]),
   .CMP_bcm            (CMP_bcm),
   .CMP_byte_count     (CMP_byte_count[11:0]),
   .MEM_addr           (MEM_addr[63:0]),  
   .CMP_req_id         (CMP_req_id[15:0]),
   .CMP_tag            (CMP_tag[7:0]),
   .CMP_lower_addr     (CMP_lower_addr[6:0]),
   .MRd                (MRd),
   .MWr                (MWr),
   .CplD               (CplD),
   .Msg                (Msg),
   .UR                 (UR),
   .header_fields_valid(header_fields_valid),
   .data_valid         (write_data_fifo_cntrl),
   //Outputs to xfer_trn_mem_fifo
	.isDes              (isDes),        ///Jiansong: added for TX des
   .mem_dest_addr      (mem_dest_addr),
   .mem_dma_size       (mem_dma_size),
   .mem_dma_start      (mem_dma_start)
	// Jiansong: output to tx_sm
//	.np_rx_cnt_qw       (np_rx_cnt_qw)
	// Debug output
//	.Debug30RXEngine    (Debug30RXEngine)
   );

//Instantiate the Recieve TRN State Machine
//This module interfaces to the Block Plus RX 
//TRN. It presents the 64-bit data from completer and and forwards that
//data with a data_valid signal.  This block also decodes packet header info
//and forwards it to the rx_trn_monitor block. 
rx_trn_data_fsm rx_trn_data_fsm_inst(
   .clk                   (clk),
   .rst                   (rst),
   // Rx Local-Link from PCIe Endpoint Block Plus
   .trn_rd              (trn_rd_reg),            // I [63/31:0]
   .trn_rrem_n          (trn_rrem_reg_n),        // I [7:0]
   .trn_rsof_n          (trn_rsof_reg_n),        // I
   .trn_reof_n          (trn_reof_reg_n),        // I
   .trn_rsrc_rdy_n      (trn_rsrc_rdy_reg_n),    // I
   .trn_rsrc_dsc_n      (trn_rsrc_dsc_reg_n),    // I
   .trn_rdst_rdy_n      (trn_rdst_rdy_n),        // O
   .trn_rerrfwd_n       (trn_rerrfwd_reg_n),     // I
   .trn_rnp_ok_n        (trn_rnp_ok_n),          // O
   .trn_rbar_hit_n      (trn_rbar_hit_reg_n),    // I [6:0]
   .trn_rfc_npd_av      (trn_rfc_npd_av_reg),    // I [11:0]
   .trn_rfc_nph_av      (trn_rfc_nph_av_reg),    // I [7:0]
   .trn_rfc_pd_av       (trn_rfc_pd_av_reg),     // I [11:0]
   .trn_rfc_ph_av       (trn_rfc_ph_av_reg),     // I [7:0]
   .trn_rfc_cpld_av     (trn_rfc_cpld_av_reg),   // I [11:0]
   .trn_rfc_cplh_av     (trn_rfc_cplh_av_reg),   // I [7:0]
   .trn_rcpl_streaming_n(trn_rcpl_streaming_n),  // O 
   //Data write signals for writing completion data to the data_trn_mem_fifo
   //or for writing data to targets memories.  Could easily demux using bar_hit
   //signals to steer the data to different locations
   .data_out            (write_data_fifo_data[63:0]),
   .data_out_be         (),
   .data_valid          (write_data_fifo_cntrl),
   .data_fifo_status    (write_data_fifo_status),
   //Header field signals
   //interfaced to rx_trn_monitor
   .fourdw_n_threedw   (fourdw_n_threedw), 
   .payload            (payload),
   .tc                 (tc[2:0]), 
   .td                 (td), 
   .ep                 (ep),  
   .attr               (attr[1:0]),
   .dw_length          (dw_length[9:0]),
   .MEM_req_id         (MEM_req_id[15:0]),
   .MEM_tag            (MEM_tag[7:0]),
   .CMP_comp_id        (CMP_comp_id[15:0]),
   .CMP_compl_stat     (CMP_compl_stat[2:0]),
   .CMP_bcm            (CMP_bcm),
   .CMP_byte_count     (CMP_byte_count[11:0]),
   .MEM_addr           (MEM_addr[63:0]),  
   .CMP_req_id         (CMP_req_id[15:0]),
   .CMP_tag            (CMP_tag[7:0]),
   .CMP_lower_addr     (CMP_lower_addr[6:0]),
   .MRd                (MRd),
   .MWr                (MWr),
   .CplD               (CplD),
   .Msg                (Msg),
   .UR                 (UR),
   .bar_hit            (bar_hit[6:0]),
   .header_fields_valid(header_fields_valid)
);

always@(posedge clk) CplD_r <= CplD;

//Instantiate the Data TRN Mem FIFO
//This is an 8KB FIFO constructed of BRAM
//Provides additional buffering in case the dma_ddr2_if is busy with
//egress.  Also, converts the datapath from 64-bit to 128 bit
data_trn_mem_fifo data_trn_mem_fifo_inst(
   .din    (write_data_fifo_data[63:0]),
   .rd_clk (clk),
   .rd_en  (read_data_fifo_cntrl),
   .rst    (rst),
   .wr_clk (clk),
//   .wr_en  (write_data_fifo_cntrl & CplD),
   .wr_en  (write_data_fifo_cntrl & CplD_r),     // Jiansong: slove Mrd write to data fifo bug 
   //by swapping the DWORD order on dout we get        /// Jiansong: swapping is necessary?
   // read_data_fifo_data[127:0] = 
   // B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0
   .dout   ({read_data_fifo_data[63:0],read_data_fifo_data[127:64]}),
   .empty  (read_data_fifo_status),
   .full   (write_data_fifo_status)    /// Jiansong: no control here
);


//Instantiate the Xfer TRN Mem FIFO
//This is an 34X128 FIFO constructed of Distributed RAM
xfer_trn_mem_fifo xfer_trn_mem_fifo_inst(
   .din    ({isDes,mem_dest_addr[27:6],mem_dma_size[10:0]}),
   .clk    (clk),
   .rd_en  (xfer_trn_mem_fifo_rden),
   .rst    (rst),
   .wr_en  (mem_dma_start),
   .dout   ({isDes_fifo,mem_dest_addr_fifo[27:6],mem_dma_size_fifo[10:0]}),
   .empty  (read_xfer_fifo_status),
   .full   (write_xfer_fifo_status)          /// Jiansong: no control here
);

//// Jiansong: Debug register out
//always@(posedge clk)begin
//   if (rst)
//	   Debug31RXDataFIFOfullcnt <= 32'h0000_0000;
//	else if (write_data_fifo_status)
//	   Debug31RXDataFIFOfullcnt <= Debug31RXDataFIFOfullcnt + 1'b1;
//	else
//	   Debug31RXDataFIFOfullcnt <= Debug31RXDataFIFOfullcnt;
//end
//always@(posedge clk)begin
//   if (rst)
//	   Debug32RXXferFIFOfullcnt <= 32'h0000_0000;
//	else if (write_xfer_fifo_status)
//	   Debug32RXXferFIFOfullcnt <= Debug32RXXferFIFOfullcnt + 1'b1;
//	else
//	   Debug32RXXferFIFOfullcnt <= Debug32RXXferFIFOfullcnt;
//end
//always@(posedge clk)begin
//   if (rst)
//	   Debug33RXDataFIFOWRcnt <= 32'h0000_0000;
//	else if (write_data_fifo_cntrl & CplD)
//	   Debug33RXDataFIFOWRcnt <= Debug33RXDataFIFOWRcnt + 1'b1;
//	else
//	   Debug33RXDataFIFOWRcnt <= Debug33RXDataFIFOWRcnt;
//end
//always@(posedge clk)begin
//   if (rst)
//	   Debug34RXDataFIFORDcnt <= 32'h0000_0000;
//	else if (read_data_fifo_cntrl)
//	   Debug34RXDataFIFORDcnt <= Debug34RXDataFIFORDcnt + 1'b1;
//	else
//	   Debug34RXDataFIFORDcnt <= Debug34RXDataFIFORDcnt;
//end
//always@(posedge clk)begin
//   if (rst)
//	   Debug35RXXferFIFOWRcnt <= 32'h0000_0000;
//	else if (mem_dma_start)
//	   Debug35RXXferFIFOWRcnt <= Debug35RXXferFIFOWRcnt + 1'b1;
//	else
//	   Debug35RXXferFIFOWRcnt <= Debug35RXXferFIFOWRcnt;
//end
//always@(posedge clk)begin
//   if (rst)
//	   Debug36RXXferFIFORDcnt <= 32'h0000_0000;
//	else if (xfer_trn_mem_fifo_rden)
//	   Debug36RXXferFIFORDcnt <= Debug36RXXferFIFORDcnt + 1'b1;
//	else
//	   Debug36RXXferFIFORDcnt <= Debug36RXXferFIFORDcnt;
//end

//Instantiate the Receive Memory Data State Machine
rx_mem_data_fsm rx_mem_data_fsm_inst(
   .clk                 (clk),
   .rst                 (rst),
   //interface to dma_ddr2_if block
   .ingress_data        (ingress_data),
//   .ingress_fifo_ctrl   (ingress_fifo_ctrl),
	.ingress_fifo_wren	(ingress_fifo_wren),
//   .ingress_fifo_status (ingress_fifo_status), 
   .ingress_xfer_size   (ingress_xfer_size),
   .ingress_start_addr  (ingress_start_addr),
   .ingress_data_req    (ingress_data_req),
   .ingress_data_ack    (ingress_data_ack),
   //interface to xfer_trn_mem_fifo 
	.isDes_fifo(isDes_fifo),                      /// Jiansong: added for TX des
   .mem_dest_addr_fifo  (mem_dest_addr_fifo),
   .mem_dma_size_fifo   (mem_dma_size_fifo),
   .mem_dma_start       (1'b0),
   .mem_trn_fifo_empty  (read_xfer_fifo_status),
   .mem_trn_fifo_rden   (xfer_trn_mem_fifo_rden),
   //interface to data_trn_mem_fifo
   .data_fifo_data      (read_data_fifo_data[127:0]),
   .data_fifo_cntrl     (read_data_fifo_cntrl),   
   .data_fifo_status    (read_data_fifo_status),
   ///Jiansong: interface to dma control wrapper
   .new_des_one(new_des_one),
   .SourceAddr_L(SourceAddr_L),
   .SourceAddr_H(SourceAddr_H),
   .DestAddr(DestAddr),
   .FrameSize(FrameSize),
   .FrameControl(FrameControl)	
   );

endmodule

