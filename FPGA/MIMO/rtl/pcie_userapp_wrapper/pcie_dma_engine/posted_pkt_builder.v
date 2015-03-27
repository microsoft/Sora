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
// Purpose: Posted Packet Builder module.  This module takes the 
// length info from the Posted Packet Slicer,  and requests a tag from 
// the Tag Generator and uses that info to build a posted memory write header
// which it writes into a FIFO
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module posted_pkt_builder(
    input clk,
    input rst,
    input [15:0] req_id, //from pcie block
    //to/from posted_pkt_slicer
	 input posted_fifo_full,
    input go,
    output reg ack,
    input [63:0] dmawad,
    input [9:0] length,
    //to posted_pkt_header_fifo
    output reg [63:0] header_data_out,
    output reg header_data_wren
);
 

//State machine states
localparam IDLE = 4'h0; 
localparam HEAD1 = 4'h1; 
localparam HEAD2 = 4'h2;
localparam WAIT_FOR_GO_DEASSERT = 4'h3;

//parameters used to define fixed header fields
localparam rsvd = 1'b0; //reserved and unused header fields to zero
localparam MWr = 5'b00000; //format for memory write header
localparam TC = 3'b000;    //traffic class 0
localparam TD = 1'b0;      //digest bit always 0
localparam EP = 1'b0;      //poisoned bit always 0
localparam ATTR = 2'b00; //no snoop or relaxed-ordering
localparam LastBE = 4'b1111; //LastBE is always asserted since all transfers
                             //are on 128B boundaries and are always at least
                             //128B long
localparam FirstBE = 4'b1111;//FirstBE is always asserted since all transfers
                             //are on 128B boundaries


wire [1:0] fmt;
reg [3:0] state;
reg [63:0] dmawad_reg;

reg   rst_reg;
always@(posedge clk) rst_reg <= rst;


//if the upper DWord of the destination address is zero
//than make the format of the packet header 3DW; otherwise 4DW
assign fmt[1:0] = (dmawad_reg[63:32] == 0) ? 2'b10 : 2'b11;

//if the posted_pkt_slicer asserts "go" then register the dma write params
always@(posedge clk)begin
  if(rst_reg)begin
    dmawad_reg[63:0] <= 0;
  end else if(go)begin
    dmawad_reg <= dmawad;
  end
end


// State machine
// Builds headers for posted memory writes 
// Writes them into a FIFO
always @ (posedge clk) begin
  if (rst_reg) begin
      header_data_out <= 0;
      header_data_wren <= 1'b0;
      ack <= 1'b0;
      state <= IDLE;
  end else begin
      case (state)
        IDLE : begin
           header_data_out <= 0;
           header_data_wren <= 1'b0;
           ack <= 1'b0;
           if(go & ~posted_fifo_full)   // Jiansong: prevent p_hdr_fifo overflow
             state<= HEAD1;
           else
             state<= IDLE;
         end
         HEAD1 : begin
           //write the first 64-bits of a posted header into the 
           //posted fifo
           header_data_out <= {rsvd,fmt[1:0],MWr,rsvd,TC,rsvd,rsvd,rsvd,rsvd,
                               TD,EP,ATTR,rsvd,rsvd,length[9:0],req_id[15:0],
                               8'b00000000 ,LastBE,FirstBE};
           ack <= 0;
           header_data_wren <= 1'b1;
           state <= HEAD2;
         end
         HEAD2 : begin
             //write the next 32 or 64 bits of a posted header to the
             //posted header fifo (32 if 3DW - 64 if 4DW header)
             header_data_out <= (fmt[0]==1'b1) 
                                ? {dmawad_reg[63:2],2'b00} 
                                : {dmawad_reg[31:2], 2'b00, dmawad_reg[63:32]};
             header_data_wren <= 1'b1;
             ack <= 1'b1; //acknowledge to the posted_packet_slicer that
                          //the packet has been queued up for transmission      
             state <= WAIT_FOR_GO_DEASSERT;
         end
         WAIT_FOR_GO_DEASSERT: begin 
             //ack causes "go" to deassert but we need to give the 
             //posted_pkt_slicer a chance to deassert "go" before returning
             //to IDLE 
             header_data_out <= 0;
             header_data_wren <= 1'b0;
             ack <= 1'b0;
             state <= IDLE;
         end
         default : begin
             header_data_out <= 0;
             header_data_wren <= 1'b0;
             ack <= 1'b0; 
             state <= IDLE;
         end
      endcase
   end
 end
endmodule

