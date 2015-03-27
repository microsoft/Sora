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
// Module Name:    Sora_RCB_top 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: Non-Posted Packet Generator wrapper file.  
// Connects the NonPosted Packet Slicer and Non-Posted Packet Builder modules 
// together
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
// modified by Jiansong Zhang:
//   add logic for TX descriptor request ----------- done
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module non_posted_pkt_gen(
    input         clk,
    input         rst,
	 /// Jiansong: control signal for transfer recovering
	 input         transferstart,
    //inputs from dma_ctrl_wrapper
    input [63:0]  dmaras,
    input [31:0]  dmarad,
    input [31:0]  dmarxs,
    input         rd_dma_start_one,
	 /// Jiansong: added for TX descriptor request
	 input         rd_TX_des_start_one,
	 input [63:0]  TX_des_addr,
    //inputs from pcie block plus
    input [2:0]   read_req_size,
    input [15:0]  req_id,
    //outputs to non posted header fifo
    output        non_posted_fifo_wren,
    output [63:0] non_posted_fifo_data,
    //in and outs to tag_generator
    output        tag_inc,
    input         tag_gnt,
    input [7:0]   tag_value,
    //outputs to read_request_wrapper
    output [4:0] tx_waddr,
    output [31:0] tx_wdata,
    output tx_we
    );

//internal wrapper connections
wire [31:0] dmarad_reg;
wire [63:0] dmaras_reg;
wire [9:0]  length;
wire ack,go; //handshake signals

wire isDes;
                
non_posted_pkt_slicer non_posted_pkt_slicer_inst(
                .clk(clk), 
                .rst(rst), 
					 /// Jiansong: control signal for transfer recovering
					 .transferstart(transferstart),
                //interface to dma_ctrl_wrapper
					 .rd_TX_des_start_one(rd_TX_des_start_one),/// Jiansong:
                .TX_des_addr(TX_des_addr),                ///     added for TX des request
					 .isDes(isDes),                            ///
					 .rd_dma_start(rd_dma_start_one), 
                .dmarad(dmarad), 
                .dmarxs(dmarxs), 
                .dmaras(dmaras), 
                .read_req_size(read_req_size), //from pcie block
                //interface to non_posted_pkt_builder
                .ack(ack), 
                .go(go), 
                .dmarad_reg(dmarad_reg[31:0]), 
                .dmaras_reg(dmaras_reg[63:0]), 
                .length(length[9:0])
                );
 
non_posted_pkt_builder non_posted_pkt_builder_inst(    
                .clk(clk),
                .rst(rst),
                .req_id(req_id[15:0]), //from pcie block
                //interface to/from non_posted_pkt_slicer
                .go(go),
                .ack(ack),
                .dmaras(dmaras_reg[63:0]),
                .dmarad(dmarad_reg[31:0]),
                .length(length[9:0]),
					 .isDes(isDes),                            /// Jiansong:
                //interface to/from tag_generator
                .tag_value(tag_value[7:0]),
                .tag_gnt(tag_gnt),
                .tag_inc(tag_inc),
                 //interface to/from a64_64_distram_np(non-posted header fifo)
                .header_data_out(non_posted_fifo_data[63:0]),
                .header_data_wren(non_posted_fifo_wren),
                //interface to read_request_wrapper
                .tx_waddr(tx_waddr[4:0]),
                .tx_wdata(tx_wdata[31:0]),
                .tx_we(tx_we)
                );


endmodule
