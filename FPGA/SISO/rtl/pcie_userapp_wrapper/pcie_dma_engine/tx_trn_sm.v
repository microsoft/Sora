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
// Module Name:    tx_trn_sm 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: Transmit TRN State Machine module.  Interfaces to the Endpoint
// Block Plus and transmits packtets out of the TRN interface.  Drains the 
// packets out of FIFOs.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
// Modified by jiansong zhang, 2009-6-18
//     (1) delete dma write done generation logic -------------- done
//     (2) modification on scheduling
//     (3) register/memory read logic -------------------------- done
//
//     semiduplex scheduling to slove possible interlock problem on 
//     north-bridge/memory-controller
//     (1) add np_tx_cnt logic --------------------------------- done
//     (2) add np_rx_cnt input
//     (3) scheduling: if ( (np_tx_cnt == np_rx_cnt) && (p_hdr_fifo != empty) )
//                        send p packets;
//                     else if (np_hdr_fifo != empty)
//                             send np packets;
//                     else if (cmp_hdr_fifo != empty)
//                             send cmp packets;
//                     else
//                             IDLE;
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps
`include "Sora_config.v"

module tx_trn_sm(
    input clk,
    input rst_in,
	 input hostreset_in,
	 output rst_out,
    //interface to the header fifos
    input [63:0] posted_hdr_fifo,
    output posted_hdr_fifo_rden,
    input posted_hdr_fifo_empty,
    input [63:0] nonposted_hdr_fifo,
    output nonposted_hdr_fifo_rden,
    input nonposted_hdr_fifo_empty,
    input [63:0] comp_hdr_fifo,
    input comp_hdr_fifo_empty,
    output comp_hdr_fifo_rden,
	 /// Jiansong: posted_data_fifo interface
	 output reg   posted_data_fifo_rden,
	 input [63:0] posted_data_fifo_data,
	 // it's used, data fifo should not be empty when it is read
	 input        posted_data_fifo_empty, 

    /// Jiansong: interface for Mrd, connect to dma control wrapper, don't need request/ack handshake
    output reg[11:0] Mrd_data_addr,    /// Jiansong: 12 bits register address
    input [31:0] Mrd_data_in,
	 
    //interface to PCIe Endpoint Block Plus TX TRN
    output reg [63:0] trn_td,
    output reg [7:0] trn_trem_n,
    output reg trn_tsof_n,
    output reg trn_teof_n,
    output trn_tsrc_rdy_n,
    output trn_tsrc_dsc_n, 
    input trn_tdst_rdy_n,//if this signal is deasserted (high) must pause all
                         //activity to the TX TRN interface. This signal is 
                         //used as a clock enable to much of the circuitry
                         //in this module
    input trn_tdst_dsc_n,//destination should not discontinue - Endpoint Block
                         //Plus V1.6.1 does not support this signal
    output trn_terrfwd_n,
    input [2:0] trn_tbuf_av,
	 /// Jiansong: input from rx_monitor
	 input [9:0] np_rx_cnt_qw,	 
	 input       transferstart,
 	 input       Wait_for_TX_desc,
	 input       rd_dma_start,  //indicates the start of a read dma xfer
    input  [12:3] dmarxs,        //size of the complete transfer
	 // debug interface
	 output reg [31:0] Debug21RX2,
	 output reg [31:0] Debug25RX6

    );

    //state machine state definitions for state[19:0]
    localparam IDLE =                   21'b0_0000_0000_0000_0000_0000;
    localparam GET_P_HD =               21'b0_0000_0000_0000_0000_0001;
    localparam GET_NP_HD =              21'b0_0000_0000_0000_0000_0010;
    localparam GET_COMP_HD =            21'b0_0000_0000_0000_0000_0100;
    localparam SETUP_P_DATA =           21'b0_0000_0000_0000_0000_1000;
////    localparam WAIT_FOR_DATA_RDY =      20'b0000_0000_0000_0010_0000;
    localparam P_WAIT_STATE  =          21'b0_0000_0000_0000_0001_0000;
    localparam P_WAIT_STATE1 =          21'b0_0000_0000_0000_0010_0000;
    localparam HD1_P_XFER =             21'b0_0000_0000_0000_0100_0000;
    localparam HD2_P_XFER =             21'b0_0000_0000_0000_1000_0000;
    localparam DATA_P_XFER =            21'b0_0000_0000_0001_0000_0000;
    localparam LAST_P_XFER =            21'b0_0000_0000_0010_0000_0000;
    localparam SETUP_NP =               21'b0_0000_0000_0100_0000_0000;
    localparam SETUP_COMP_DATA =        21'b0_0000_0000_1000_0000_0000;
    localparam SETUP_COMP_DATA_WAIT1 =  21'b0_0000_0001_0000_0000_0000;
    localparam SETUP_COMP_DATA_WAIT2 =  21'b0_0000_0010_0000_0000_0000;
    localparam HD1_NP_XFER =            21'b0_0000_0100_0000_0000_0000;
    localparam HD2_NP_XFER =            21'b0_0000_1000_0000_0000_0000;
    localparam NP_WAIT_STATE =          21'b0_0001_0000_0000_0000_0000; 
    localparam WAIT_FOR_COMP_DATA_RDY = 21'b0_0010_0000_0000_0000_0000;
    localparam HD1_COMP_XFER =          21'b0_0100_0000_0000_0000_0000;
    localparam HD2_COMP_XFER =          21'b0_1000_0000_0000_0000_0000;
	 localparam NP_XFER_WAIT =           21'b1_0000_0000_0000_0000_0000;
	 
	 //states for addsub_state
    localparam AS_IDLE         = 2'b00;
    localparam REGISTER_CALC   = 2'b01;
    localparam WAIT_FOR_REG    = 2'b10;

    reg [1:0] addsub_state;

//header fifo signals
reg read_posted_header_fifo, read_posted_header_fifo_d1, 
    read_posted_header_fifo_d2;
reg read_non_posted_header_fifo, read_non_posted_header_fifo_d1,
    read_non_posted_header_fifo_d2;
reg read_comp_header_fifo, read_comp_header_fifo_d1, 
    read_comp_header_fifo_d2;
wire p_trn_fifo_rdy, np_trn_fifo_rdy;
//holding registers for TLP headers
reg [63:0] p_hd1, p_hd2; //posted headers 1 and 2
reg [63:0] np_hd1, np_hd2; //non-posted headers 1 and 2
reg [63:0] comp_hd1, comp_hd2; //completer headers 1 and 2
//datapath registers
reg [31:0] data_reg;
reg [9:0] length_countdown;
wire [63:0] data_swap;

/// Jiansong: swap register data
wire [31:0] Mrd_data_swap;

//decoded TLP signals - mainly used for code comprehension
wire [1:0] p_fmt;
wire [2:0] p_tc;
wire [9:0] p_length;
wire [1:0] np_fmt;
wire [2:0] np_tc;
wire [9:0] np_length;
////reg p_hd_valid;           /// Jiansong: no longer needed

//main state machine signals
reg [20:0] state;
////reg posted; //asserted when the state machine is in the posted flow
////            //used to qualify the data_stall signal as a CE for many of
////            //the blocks that aren't related to the DDR2 
reg [1:0] data_src_mux;
reg trn_tsrc_rdy_n_reg;
reg [3:0] priority_count = 4'b1111;
wire      posted_priority;

/// Jiansong: pipeline register for a complicated problem when trn_tdst_rdy_n is asserted
reg        trn_tdst_rdy_n_r2;
reg [63:0] data_reg_tdst_problem;
reg        trn_tdst_rdy_n_r;   

reg   rst_reg;

/// Jiansong: non_posted_length_counter
reg add_calc = 0;
reg sub_calc = 0;
reg add_complete;
reg sub_complete;
reg [9:0] np_tx_cnt_qw;
reg [9:0] np_tx_cnt_qw_new;
reg [9:0] np_tx_cnt_qw_add;
reg       update_np_tx_cnt_qw;
reg stay_2x;

/// Jiansong:
wire rd_dma_start_one;
reg rd_dma_start_reg;
rising_edge_detect rd_dma_start_one_inst(
                .clk(clk),
                .rst(rst_reg),
                .in(rd_dma_start_reg),
                .one_shot_out(rd_dma_start_one)
                );
//pipe line register for timing purposes
always@(posedge clk) 
        rd_dma_start_reg <= rd_dma_start;

`ifdef sora_simulation
	always@(posedge clk) rst_reg <= rst_in;
`else
	always@(posedge clk) begin
		if(state == IDLE)
			rst_reg <= rst_in | hostreset_in;
		else
			rst_reg <= 1'b0;
	end
`endif
assign rst_out = rst_reg;

// debug register
always@(posedge clk)begin
   Debug21RX2[19:0]  <= state[19:0];
	Debug21RX2[29:20] <= length_countdown[9:0];
	Debug21RX2[31:30] <= 2'b00;
end

always@(posedge clk)begin
   if (rst_reg)
	   Debug25RX6 <= 32'h0000_0000;
	else if (posted_data_fifo_rden)
	   Debug25RX6 <= Debug25RX6 + 32'h0000_0001;
	else
	   Debug25RX6 <= Debug25RX6;
end

//tie off to pcie trn tx
assign trn_tsrc_dsc_n = 1'b1;    /// Jiansong: transmit source discontinue
assign trn_terrfwd_n = 1'b1;     /// Jiansong: error?

//if there is a data_stall need to pause the TX TRN interface
////assign trn_tsrc_rdy_n = (data_stall & posted) | trn_tsrc_rdy_n_reg;
assign trn_tsrc_rdy_n = trn_tsrc_rdy_n_reg;

/// Jiansong: need modification? big-endian for 64 bit data? Don't modify
// swap byte ordering of data to big-endian per PCIe Base spec
////assign data_swap[63:0] = {data[7:0],data[15:8], 
////                          data[23:16],data[31:24],
////                          data[39:32],data[47:40],
////                          data[55:48],data[63:56]};
assign data_swap[63:0] = {posted_data_fifo_data[7:0],posted_data_fifo_data[15:8], 
                          posted_data_fifo_data[23:16],posted_data_fifo_data[31:24],
                          posted_data_fifo_data[39:32],posted_data_fifo_data[47:40],
                          posted_data_fifo_data[55:48],posted_data_fifo_data[63:56]};
////assign data_swap[63:0] = {posted_data_fifo_data[39:32],posted_data_fifo_data[47:40],
////                          posted_data_fifo_data[55:48],posted_data_fifo_data[63:56],
////								  posted_data_fifo_data[7:0],posted_data_fifo_data[15:8], 
////                          posted_data_fifo_data[23:16],posted_data_fifo_data[31:24]};
								  
/// Jiansong: swap register read / memory read data
assign Mrd_data_swap[31:0] = {Mrd_data_in[7:0],Mrd_data_in[15:8],
                              Mrd_data_in[23:16],Mrd_data_in[31:24]};

// output logic from statemachine that controls read of the posted packet fifo   
// read the two headers from the posted fifo and store in registers
// the signal that kicks off the read is a single-cycle signal from 
// the state machine
// read_posted_header_fifo
  always@(posedge clk)begin
     if(rst_reg)begin
         read_posted_header_fifo_d1 <= 1'b0;
         read_posted_header_fifo_d2 <= 1'b0;
////     end else if(~trn_tdst_rdy_n & ~data_stall)begin 
//     end else if(~trn_tdst_rdy_n)begin
     end else begin
	      /// Jiansong: pipeline the fifo rden signal
         read_posted_header_fifo_d1 <= read_posted_header_fifo;
         read_posted_header_fifo_d2 <= read_posted_header_fifo_d1;         
     end
  end

  //stretch read enable to two clocks and qualify with trn_tdst_rdy_n
  assign posted_hdr_fifo_rden =  (read_posted_header_fifo_d1 
                                  | read_posted_header_fifo);
//                                  & ~trn_tdst_rdy_n;											 
////                                  & ~trn_tdst_rdy_n
////                                  & ~data_stall;  
                                  

// use the read enable signals to enable registers p_hd1 and p_hd2
 always@(posedge clk)begin
     if(rst_reg)begin
         p_hd1[63:0] <= 64'h0000000000000000;
////     end else if(~trn_tdst_rdy_n & ~data_stall)begin
//     end else if(~trn_tdst_rdy_n) begin
     end else begin
        if(read_posted_header_fifo_d1)begin
            p_hd1 <= posted_hdr_fifo;
        end else begin
            p_hd1 <= p_hd1;
        end
     end
  end

 always@(posedge clk)begin
     if(rst_reg)begin
         p_hd2[63:0] <= 64'h0000000000000000;
////     end else if(~trn_tdst_rdy_n & ~data_stall)begin
//     end else if(~trn_tdst_rdy_n)begin
     end else begin
        if(read_posted_header_fifo_d2)begin
            p_hd2 <= posted_hdr_fifo;
        end else begin
            p_hd2 <= p_hd2;
        end
     end
  end

//assign signals for reading clarity
assign p_fmt[1:0] = p_hd1[62:61]; //format field
assign p_tc[2:0] = p_hd1[54:52];  //traffic class field
assign p_length[9:0] = p_hd1[41:32]; //DW length field    

assign  p_trn_fifo_rdy = trn_tbuf_av[1];


// output logic from statemachine that controls read of the 
// non_posted packet fifo   
// read the two headers from the non posted fifo and store in registers
// the signal that kicks off the read is a single-cycle signal from the
// state machine
// read_posted_header_fifo
  always@(posedge clk)begin
     if(rst_reg)begin
         read_non_posted_header_fifo_d1 <= 1'b0;
         read_non_posted_header_fifo_d2 <= 1'b0;
//     end else if(~trn_tdst_rdy_n) begin
     end else begin
	      /// Jiansong: pipeline the fifo rden signal
         read_non_posted_header_fifo_d1 <= read_non_posted_header_fifo;
         read_non_posted_header_fifo_d2 <= read_non_posted_header_fifo_d1;      
     end
  end

//stretch read enable to two clocks and qualify with trn_tdst_rdy_n
  assign nonposted_hdr_fifo_rden =  (read_non_posted_header_fifo_d1
                                    | read_non_posted_header_fifo);
//                                    & ~trn_tdst_rdy_n; 

// use the read enable signals to enable registers np_hd1 and np_hd2
 always@(posedge clk)begin
     if(rst_reg)begin
         np_hd1[63:0] <= 64'h0000000000000000;
//     end else if(~trn_tdst_rdy_n)begin
     end else begin
        if(read_non_posted_header_fifo_d1)begin
            np_hd1 <= nonposted_hdr_fifo;
        end else begin
            np_hd1 <= np_hd1;
        end
     end
  end

 always@(posedge clk)begin
     if(rst_reg)begin
         np_hd2[63:0] <= 64'h0000000000000000;
//     end else if(~trn_tdst_rdy_n)begin
     end else begin
        if(read_non_posted_header_fifo_d2)begin
            np_hd2 <= nonposted_hdr_fifo;
        end else begin
            np_hd2 <= np_hd2;
        end
     end
  end


//assign signals for reading clarity
assign np_fmt[1:0] = np_hd1[62:61]; //format field
assign np_tc[2:0] = np_hd1[54:52];  //traffic class field
assign np_length[9:0] = np_hd1[41:32]; //DW length field

assign  np_trn_fifo_rdy = trn_tbuf_av[0];

// output logic from statemachine that controls read of the comp packet fifo   
// read the two headers from the comp fifo and store in registers
// the signal that kicks off the read is a single-cycle signal from 
// the state machine
// read_comp_header_fifo
  always@(posedge clk)begin
     if(rst_reg)begin
         read_comp_header_fifo_d1 <= 1'b0;
         read_comp_header_fifo_d2 <= 1'b0;
////     end else if(~trn_tdst_rdy_n & (~data_stall | ~posted))begin
//     end else if(~trn_tdst_rdy_n)begin   /// pending
     end else begin
         read_comp_header_fifo_d1 <= read_comp_header_fifo;
         read_comp_header_fifo_d2 <= read_comp_header_fifo_d1;         
     end
  end

//stretch read enable to two clocks and qualify with trn_tdst_rdy_n
  assign comp_hdr_fifo_rden =  (read_comp_header_fifo_d1 
                                  | read_comp_header_fifo); 
//                                  & ~trn_tdst_rdy_n;  
                                  

// use the read enable signals to enable registers comp_hd1 and comp_hd2
 always@(posedge clk)begin
     if(rst_reg)begin
         comp_hd1[63:0] <= 64'h0000000000000000;
////     end else if(~trn_tdst_rdy_n & (~data_stall | ~posted))begin
//     end else if(~trn_tdst_rdy_n)begin   /// pending
     end else begin
        if(read_comp_header_fifo_d1)begin
            comp_hd1 <= comp_hdr_fifo;
        end else begin
            comp_hd1 <= comp_hd1;
        end
     end
  end

 always@(posedge clk)begin
     if(rst_reg)begin
         comp_hd2[63:0] <= 64'h0000000000000000;
////     end else if(~trn_tdst_rdy_n & (~data_stall | ~posted))begin
//     end else if(~trn_tdst_rdy_n)begin      /// pending
     end else begin
        if(read_comp_header_fifo_d2)begin
            comp_hd2 <= comp_hdr_fifo;
        end else begin
            comp_hd2 <= comp_hd2;
        end
     end
  end
  
assign  comp_trn_fifo_rdy = trn_tbuf_av[2];

//encode data_src
//reg_file = BAR_HIT[6:0] = 0000001 -> 01 
//ROM_BAR  = BAR_HIT[6:0] = 1000000 -> 10
//DDR2 -> 00 no need to decode as DDR2 not supported as a PCIe target
always@(*)begin             //// Jiansong: no clock driven, or whatever clock driven
   case(comp_hd1[63:57])
     7'b0000001:  data_src_mux[1:0] <= 2'b01;
     7'b1000000:  data_src_mux[1:0] <= 2'b10;
////     default: data_src_mux[1:0] <= 2'b00;
     default: data_src_mux[1:0] <= 2'b01;
   endcase
end

/// Jiansong: pending
//countdown to control amount of data tranfer in state machine  
//count in quadwords since trn_td is 64-bits     
 always@(posedge clk)begin       
     if(rst_reg)
       length_countdown <= 10'b00_0000_0000;
////     else if (~trn_tdst_rdy_n & ~data_stall)begin
     else if (~trn_tdst_rdy_n)begin
        if(state == HD1_P_XFER)
         length_countdown <= p_length>>1; //count in quadwords
        else if(length_countdown != 0)
         length_countdown <= length_countdown - 1;     
     end else
	      length_countdown <= length_countdown;
 end


//data_xfer is a state machine output that tells the egress_data_presenter
// to transfer data; every clock cycle it is asserted one 64-bit data
// is valid on the next cycle - unless data_stall is asserted
////assign data_xfer = data_xfer_reg;  
// data steering logic
 always@(posedge clk)begin
     if(rst_reg)begin
//        data_reg <= 64'h0000000000000000;
        data_reg[31:0] <= 32'h0000_0000;
////     end else if(~trn_tdst_rdy_n & (~data_stall | ~posted)) begin
     end else if(~trn_tdst_rdy_n) begin       
        data_reg[31:0] <= data_swap[31:0];
     end
 end  

/// Jiansong: this register is added the rden delay problem when trn_tdst_rdy_n is deasserted
always@(posedge clk)begin
     if(rst_reg)begin
	     data_reg_tdst_problem <= 64'h0000000000000000;
	  end else if(~trn_tdst_rdy_n_r)begin
	     data_reg_tdst_problem <= data_swap;
	  end
end

//mux the trn_td[63:0] - dependent on what state the main state machine is in
  always@(posedge clk)begin
     if(rst_reg)
        trn_td <= 0;
////     else if(~trn_tdst_rdy_n & (~data_stall | ~posted))begin
     else if(~trn_tdst_rdy_n)begin
     casex({state,p_fmt[0]})
        {HD1_P_XFER,1'bx}: begin
            trn_td <= p_hd1;
        end
        {HD2_P_XFER,1'b0}: begin
		      if(trn_tdst_rdy_n_r)    /// Jiansong:
				   trn_td <= {p_hd2[63:32],data_reg_tdst_problem[63:32]};
				else
               trn_td <= {p_hd2[63:32],data_swap[63:32]};
        end
        {HD2_P_XFER,1'b1}: begin
            trn_td <= p_hd2[63:0];
        end
        {DATA_P_XFER,1'b0},{LAST_P_XFER,1'b0}: begin
		      if(trn_tdst_rdy_n_r)    /// Jiansong:
				    trn_td[63:0] <= {data_reg[31:0],data_reg_tdst_problem[63:32]};
				else if(trn_tdst_rdy_n_r2)
				    trn_td[63:0] <= {data_reg_tdst_problem[31:0],data_swap[63:32]};
				else
                trn_td[63:0] <= {data_reg[31:0],data_swap[63:32]};
        end
        {DATA_P_XFER,1'b1},{LAST_P_XFER,1'b1}: begin
		      if(trn_tdst_rdy_n_r)    /// Jiansong:
                trn_td[63:0] <= data_reg_tdst_problem[63:0];
            else					 
                trn_td[63:0] <= data_swap[63:0];
        end
        {HD1_NP_XFER,1'bx}: begin
            trn_td <= np_hd1;
        end
        {HD2_NP_XFER,1'bx}: begin
            trn_td <= np_hd2;
        end
        {HD1_COMP_XFER,1'bx}: begin
            trn_td <= {comp_hd1[31:0],comp_hd2[63:32]};
        end
        {HD2_COMP_XFER,1'bx}: begin
////            trn_td <= {comp_hd2[31:0],data_reg[63:32]};
/// Jiansong: rom_bar, keep the old design, but don't know what's it for
            if (data_src_mux[1:0] == 2'b10) begin
					trn_td <= {comp_hd2[31:0],32'h00000000};
				end else if (data_src_mux[1:0] == 2'b01) begin
					trn_td <= {comp_hd2[31:0],Mrd_data_swap};
				end else begin
					trn_td <= {comp_hd2[31:0],Mrd_data_swap};
				end
        end
        default: begin
            trn_td <= 0;
        end
     endcase
    end
  end

/// Jiansong: priority is modified
///           in sora, round-robin will be used for posted, non-posted
///           and completion scheduling
//////Priority signal for posted and non-posted requests
//////When operating in full duplex mode the state machine
//////will do 8 posted requests followed by 8 non-posted requests
//////Note: this ordering assumes that the posted and non-posted
//////requests do not depend on each other.
//////Once inside the V-5 PCIe Block, strict ordering will be
//////followed. Also, note that completions are given
//////lowest priority.  In order to avoid completion time-outs,
//////read requests from the host should not occur during a DMA 
//////transaction.
//////If this is not possible, than the completion queue may need
//////higher priority.
              
/// Jiansong: pipeline registers				  
always@(posedge clk)begin 
   trn_tdst_rdy_n_r2 <= trn_tdst_rdy_n_r;
   trn_tdst_rdy_n_r  <= trn_tdst_rdy_n;
end

// bulk of TX TRN state machine  
  always@(posedge clk)begin
//     if(rst_in)begin
		if(rst_reg)begin
         trn_tsrc_rdy_n_reg <= 1'b1;
         trn_tsof_n <= 1'b1;
         trn_teof_n <= 1'b1;
         trn_trem_n[7:0] <= 8'b11111111; 
         posted_data_fifo_rden <= 1'b0;
         read_posted_header_fifo <= 1'b0;
         read_non_posted_header_fifo <= 1'b0;  
         read_comp_header_fifo <= 1'b0;
         state <= IDLE;
////         //use trn_tdst_rdy_n and data_stall as clock enable - for data_stall
////         //only CE if in the posted flow
////     end else if(~trn_tdst_rdy_n & (~data_stall | ~posted))begin
     // use trn_tdst_rdy_n as clock enable
     end else if(trn_tdst_rdy_n)begin
//     if(trn_tdst_rdy_n)begin
	     /// Jiansong: deassert the rden, write enable signals if PCIe core is not ready
		  posted_data_fifo_rden <= 1'b0;
        read_posted_header_fifo <= 1'b0;
        read_non_posted_header_fifo <= 1'b0;  
        read_comp_header_fifo <= 1'b0; 
////	  end else if(~trn_tdst_rdy_n)begin
     end else begin 
       case(state) 
       IDLE: begin
				if (hostreset_in) begin
				   trn_tsrc_rdy_n_reg <= 1'b1;
					trn_tsof_n <= 1'b1;
					trn_teof_n <= 1'b1;
					trn_trem_n[7:0] <= 8'b11111111;
					posted_data_fifo_rden <= 1'b0;
					read_posted_header_fifo <= 1'b0;
					read_non_posted_header_fifo <= 1'b0;
					read_comp_header_fifo <= 1'b0;
				   state <= IDLE;
				end else begin
					trn_tsrc_rdy_n_reg <= 1'b1;
					trn_tsof_n <= 1'b1;
					trn_teof_n <= 1'b1;
					trn_trem_n[7:0] <= 8'b11111111;
					posted_data_fifo_rden <= 1'b0;
					read_posted_header_fifo <= 1'b0;
					read_non_posted_header_fifo <= 1'b0;
					read_comp_header_fifo <= 1'b0;

               if ( (np_rx_cnt_qw == np_tx_cnt_qw) && (~posted_hdr_fifo_empty) && ~Wait_for_TX_desc)
					   state <= GET_P_HD;
					else if (~nonposted_hdr_fifo_empty)
					   state <= GET_NP_HD;
					else if (~comp_hdr_fifo_empty)
					   state <= GET_COMP_HD;
					else
					   state <= IDLE;
					
            end
         end
         
        GET_P_HD: begin
            read_posted_header_fifo <= 1'b1; //get the headers ready
            trn_tsrc_rdy_n_reg <= 1'b1;
            trn_trem_n[7:0] <= 8'b11111111;
            trn_teof_n <= 1'b1;
            state <= SETUP_P_DATA;     
         end
        GET_NP_HD: begin
            read_non_posted_header_fifo <= 1'b1; //get the headers ready
            trn_tsrc_rdy_n_reg <= 1'b1;
            trn_trem_n[7:0] <= 8'b11111111;
            trn_teof_n <= 1'b1;
            state <= SETUP_NP;  
         end                 
         GET_COMP_HD: begin
            read_comp_header_fifo <= 1'b1; //get the headers ready
            trn_tsrc_rdy_n_reg <= 1'b1;
            trn_trem_n[7:0] <= 8'b11111111;
            trn_teof_n <= 1'b1;
            state <= SETUP_COMP_DATA_WAIT1;     
         end

         /// Jiansong: pending, make it simpler
         //start of completer transaction flow
         SETUP_COMP_DATA_WAIT1: begin //wait state for comp_hd1
            read_comp_header_fifo <= 1'b0;
            state <= SETUP_COMP_DATA_WAIT2;
         end
         SETUP_COMP_DATA_WAIT2: begin //wait state for comp_hd2
            state <= SETUP_COMP_DATA;
         end
         SETUP_COMP_DATA: begin
            Mrd_data_addr[11:0] <= {comp_hd1[41:32],2'b00}; 
            if(comp_trn_fifo_rdy)//make sure the completion fifo in the PCIe
                                 //block is ready
               state <= WAIT_FOR_COMP_DATA_RDY;
            else
               state <= SETUP_COMP_DATA;
         end
			/// Jiansong: wait one more cycle for reg data ready, maybe not necessary
			WAIT_FOR_COMP_DATA_RDY: begin
			    state <= HD1_COMP_XFER;
			end
			
          HD1_COMP_XFER: begin    //transfer first header
             trn_tsof_n <= 1'b0;      
             trn_tsrc_rdy_n_reg <= 1'b0;      
             trn_trem_n[7:0] <= 8'b00000000;    
             state <= HD2_COMP_XFER;            
          end
          HD2_COMP_XFER: begin     //transfer second header + 1 DW of data
			     trn_tsrc_rdy_n_reg <= 1'b0;
              trn_tsof_n <= 1'b1;
              trn_teof_n <= 1'b0;
              state <= IDLE;      
          end

         //start of posted transaction flow         
         SETUP_P_DATA: begin
            read_posted_header_fifo <= 1'b0;
				posted_data_fifo_rden <= 1'b0;
            state <= P_WAIT_STATE;
         end
			P_WAIT_STATE : begin       /// Jiansong: wait one more cycle for hdr ready
			   read_posted_header_fifo <= 1'b0;
				posted_data_fifo_rden <= 1'b0;
				state <= P_WAIT_STATE1;
			end
         P_WAIT_STATE1 : begin
         //wait for the egress data_presenter to have data ready then start
         //transmitting the first posted header
            trn_teof_n <= 1'b1; 
            if(p_trn_fifo_rdy & ~posted_data_fifo_empty) begin //make sure posted fifo in PCIe block is ready
               /// Jiansong: read data fifo?
					if(p_fmt[0] == 0)begin //3DW
					    posted_data_fifo_rden <= 1'b1;
               end else begin //4DW
					    posted_data_fifo_rden <= 1'b0;
               end
					state <= HD1_P_XFER;
            end else begin
				   posted_data_fifo_rden <= 1'b0;
               state <= P_WAIT_STATE1;
				end
          end
          HD1_P_XFER: begin    //transfer first header
             trn_tsof_n <= 1'b0; //assert SOF     
             trn_teof_n <= 1'b1;      
             trn_tsrc_rdy_n_reg <= 1'b0;      
             trn_trem_n[7:0] <= 8'b00000000;
             posted_data_fifo_rden <= 1'b1;				 
             state <= HD2_P_XFER;            
          end
          HD2_P_XFER: begin     //transfer second header (+1 DW of data for 3DW)
			     trn_tsrc_rdy_n_reg <= 1'b0;
              trn_tsof_n <= 1'b1; //deassert SOF
				  /// Jiansong: RX desc is so short (2 cycles) that we need specially consider it
				  if (p_fmt[0] == 0 && p_length <= 10'h004)             
				     posted_data_fifo_rden <= 1'b0;
				  else
				     posted_data_fifo_rden <= 1'b1;
              state <= DATA_P_XFER;
            
          end
          // DATA_P_XFER state for packets with more than 1 DW 
          // for this design the next step up will always be 128B or 32DW
          DATA_P_XFER: begin
              trn_tsrc_rdy_n_reg <= 1'b0;			 
              //use a counter to figure out when we are almost done and 
              //jump to LAST_P_XFER when we reach the penultimate data cycle
              if(length_countdown != 1)begin
                  state <= DATA_P_XFER;
              end else begin 
                  state <= LAST_P_XFER;   
              end  
              //figure out when to deassert data_xfer_reg based on whether
              //this a 3DW or 4DW header posted TLP   
              if(p_fmt[0] == 0)begin  //3DW case         
                 if(length_countdown <=2)
					      posted_data_fifo_rden <= 1'b0;
                 else
					      posted_data_fifo_rden <= 1'b1;
              end else begin         //4DW case
                 if(length_countdown <=1)
					      posted_data_fifo_rden <= 1'b0;
                 else
					      posted_data_fifo_rden <= 1'b1;
              end
          end
              
          LAST_P_XFER: begin
			     trn_tsrc_rdy_n_reg <= 1'b0;
              trn_teof_n <= 1'b0;//assert EOF
              posted_data_fifo_rden <= 1'b0;
              read_posted_header_fifo <= 1'b0;
              //assert the correct remainder bits dependent on 3DW or 4DW TLP
              //headers
              if(p_fmt[0] == 0) //0 for 3dw, 1 for 4dw header
                 trn_trem_n[7:0] <= 8'b00001111; 
              else
                 trn_trem_n[7:0] <= 8'b00000000; 
         
              state <= IDLE;			
           end      

           //start of the non-posted transaction flow
           SETUP_NP: begin
              read_non_posted_header_fifo <= 1'b0;
              state <= NP_WAIT_STATE;
           end
          //NP_WAIT_STATE state needed to let np_hd1 and np_hd2 to catch up
          NP_WAIT_STATE:begin 
              if(np_trn_fifo_rdy)
                 state <= HD1_NP_XFER;
             else
                 state <= NP_WAIT_STATE;
           end
          HD1_NP_XFER: begin    //transfer first header
             trn_tsof_n <= 1'b0; //assert SOF     
             trn_tsrc_rdy_n_reg <= 1'b0;      
             trn_trem_n[7:0] <= 8'b00000000;            
             state <= HD2_NP_XFER;            
          end
          HD2_NP_XFER: begin     //transfer second header + 1 DW of data
			     trn_tsrc_rdy_n_reg <= 1'b0;
              trn_tsof_n <= 1'b1; //deassert EOF
              trn_teof_n <= 1'b0; //assert EOF
              if(np_fmt[0] == 0) //0 for 3dw, 1 for 4dw header
                 trn_trem_n[7:0] <= 8'b00001111; 
              else
                 trn_trem_n[7:0] <= 8'b00000000;
					  
              state <= NP_XFER_WAIT;
          end
          NP_XFER_WAIT : begin   /// Jiansong: add one cycle into NP xfer to support
			    state <= IDLE;      ///           semiduplex scheduling
			 end
           
        endcase
      end
  end
  
	/// Jiansong: logic to maintain np_tx_cnt_qw
	always@(posedge clk)begin  
		  if (rst_reg | (~transferstart))
				add_calc <=  1'b0;
		  else if(rd_dma_start_one) //set the bit
				add_calc <=  1'b1;
		  else if (add_complete) //reset the bit
				add_calc <=  1'b0;
	end
	always@(posedge clk)begin
			  if (rst_reg | (~transferstart) | Wait_for_TX_desc)
					sub_calc <=  1'b0;
			  else if(read_non_posted_header_fifo_d1) //set the bit, fliter out TX des 
					sub_calc <=  1'b1;
			  else if (sub_complete) //reset the bit
					sub_calc <=  1'b0;
	end
	always@(posedge clk) np_tx_cnt_qw_add[9:0] <= dmarxs[12:3];
	always@(posedge clk)begin
	  if(rst_reg | (~transferstart) | Wait_for_TX_desc)begin
			np_tx_cnt_qw[9:0] <=  0;
	  end else if(update_np_tx_cnt_qw)begin
			np_tx_cnt_qw[9:0] <=  np_tx_cnt_qw_new[9:0];
	  end
	end
	always@(posedge clk)begin
		  if(rst_reg | (~transferstart) | Wait_for_TX_desc)begin
			  np_tx_cnt_qw_new[9:0] <=  0;
			  update_np_tx_cnt_qw <=  1'b0;
			  add_complete <=  1'b0;
			  sub_complete <=  1'b0;
			  stay_2x <= 1'b0;
			  addsub_state <=  AS_IDLE;
		  end else begin
			  case(addsub_state)
				  AS_IDLE: begin
					  update_np_tx_cnt_qw <=  1'b0;
					  if(add_calc)begin
					  //if add_calc is asserted then add the current value (*_now) to 
					  //the incoming dma xfer size (*_reg)
							  np_tx_cnt_qw_new[9:0] <=  np_tx_cnt_qw[9:0] 
																  + np_tx_cnt_qw_add[9:0];
							  //make sure to stay in this state for two clock cycles
							  if(~stay_2x)begin
								  addsub_state <= AS_IDLE;
								  add_complete <= 1'b0;
								  update_np_tx_cnt_qw <= 1'b0;
								  stay_2x <= 1'b1;
							  //then update the current value (dmawxs_div8_now)
							  end else begin
								  addsub_state <= REGISTER_CALC;
								  add_complete <= 1'b1;//clear add_calc
								  update_np_tx_cnt_qw <= 1'b1;
								  stay_2x <= 1'b0;
							  end
					  end else if (sub_calc)begin
					  //if sub_calc is asserted then subtract the dw_length field
					  //from the incoming completion packet from the current value
							  np_tx_cnt_qw_new[9:0] <=  np_tx_cnt_qw[9:0] 
																  - {1'b0, np_length[9:1]};
							  //likewise make sure to stat in this state for two clocks
							  if(~stay_2x)begin
								  addsub_state <= AS_IDLE;
								  sub_complete <= 1'b0;
								  update_np_tx_cnt_qw <= 1'b0;
								  stay_2x <= 1'b1;
							  //then update the current value (dmawxs_div8_now)
							  end else begin
								  addsub_state <= REGISTER_CALC;
								  sub_complete <= 1'b1;//clear sub_calc
								  update_np_tx_cnt_qw <= 1'b1;
								  stay_2x <= 1'b0;
							  end
					  end else begin
							  np_tx_cnt_qw_new[9:0] <=  np_tx_cnt_qw[9:0];
							  addsub_state <=  AS_IDLE;
							  sub_complete <=  1'b0;
							  add_complete <=  1'b0;
							  stay_2x <= 1'b0;
					  end
				  end
				  REGISTER_CALC:begin 
					  sub_complete <=  1'b0;
					  add_complete <=  1'b0;
					  addsub_state <= WAIT_FOR_REG;
					  update_np_tx_cnt_qw <=  1'b1;
					  stay_2x <= 1'b0;
					end
				  WAIT_FOR_REG:begin 
					  update_np_tx_cnt_qw <=  1'b0;
					  stay_2x <= 1'b0;
					  addsub_state <=  AS_IDLE;
				  end
				  default:begin
					  np_tx_cnt_qw_new[9:0] <=  0;
					  update_np_tx_cnt_qw <=  1'b0;
					  add_complete <=  1'b0;
					  sub_complete <=  1'b0;
					  stay_2x <= 1'b0;
					  addsub_state <=  AS_IDLE;
				  end
			  endcase
			end
	end

endmodule


