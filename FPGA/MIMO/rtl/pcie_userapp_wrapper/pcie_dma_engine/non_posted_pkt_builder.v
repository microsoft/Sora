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
// Purpose: Non-Posted Packet Builder module.  This module takes the 
// length info from the Non-Posted Packet Slicer,  and requests a tag from 
// the Tag Generator and uses that info to build a non-posted memory read 
// header which it writes into a FIFO
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
// modified by zjs:
//   tag content modified: from {length[9:0], addr[21:0]} to {isDes, 8'h000, addr[21:0]}
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module non_posted_pkt_builder(
    input clk,
    input rst,
    input [15:0] req_id, //from pcie block
    //to/from the non_posted_pkt_slicer
    input go,
    output reg ack,
    input [63:0] dmaras,
    input [31:0] dmarad,
    input [9:0] length,
	 /// Jiansong: added for TX des request
	 input isDes,
    //to/from the tag_generator
    input [7:0] tag_value,
    input tag_gnt,
    output reg tag_inc,
    //to the non_posted_pkt_header_fifo
    output reg [63:0] header_data_out,
    output reg header_data_wren,
    //to the read_request_wrapper
    output reg [4:0] tx_waddr,
    output reg [31:0] tx_wdata,
    output reg tx_we
);

//State machine states
localparam IDLE = 4'h0; 
localparam HEAD1 = 4'h1; 
localparam HEAD2 = 4'h2;    
localparam WAIT_FOR_GO_DEASSERT = 4'h3;

//parameters used to define fixed header fields
localparam rsvd = 1'b0; //reserved and unused header fields to zero
localparam MRd = 5'b00000; //format for memory read header
localparam TC = 3'b000;    //traffic class 0
localparam TD = 1'b0;      //digest bit always 0
localparam EP = 1'b0;      //poisoned bit always 0
//localparam ATTR = 2'b10;//enable relaxed ordering to allow completions to pass
                        //for completion streaming mode
localparam ATTR = 2'b00;     //Jiansong: Maybe it's the cause of CPU memory read lock problem
localparam LastBE = 4'b1111; //LastBE is always asserted since all transfers
                             //are on 128B boundaries and are always at least
                             //128B long
localparam FirstBE = 4'b1111;//FirstBE is always asserted since all transfers
                             //are on 128B boundaries


wire [1:0] fmt;
reg [3:0] state;
reg [63:0] dmaras_reg;
reg [31:0] dmarad_reg;


reg   rst_reg;
always@(posedge clk) rst_reg <= rst;


//if the upper DWord of the destination address is zero
//than make the format of the packet header 3DW; otherwise 4DW
assign fmt[1:0] = (dmaras_reg[63:32] == 0) ? 2'b00 : 2'b01;

//if the non_posted_pkt_slicer asserts "go" then register the dma read params 
always@(posedge clk)begin
  if(rst_reg)begin
    dmaras_reg[63:0] <= 0;
  end else if(go)begin
    dmaras_reg <= dmaras;
  end
end

//dmarad is sent to the read_request_wrapper so that the RX engine knows
//where to put the incoming completion data in the DDR2
always@(posedge clk)begin
  if(rst_reg)begin
    dmarad_reg[31:0] <= 0;
  end else if(go)begin
    dmarad_reg <= dmarad;
  end
end

// State machine
// Builds headers for non-posted memory reads 
// Writes them into a FIFO
always @ (posedge clk) begin
  if (rst_reg) begin
      header_data_out <= 0;
      header_data_wren <= 1'b0;
      ack <= 1'b0;
      tag_inc <=1'b0;   
      tx_waddr[4:0] <= 0;
      tx_wdata[31:0] <= 0;
      tx_we <= 1'b0;
      state <= IDLE;
  end else begin
      case (state)
        IDLE : begin
           header_data_out <= 0;
           header_data_wren <= 1'b0;
           ack <= 1'b0;
           tag_inc <=1'b0;   
           tx_waddr[4:0] <= 0;
           tx_wdata[31:0] <= 0;
           tx_we <= 1'b0;
           if(go)
             state<= HEAD1;
           else
             state<= IDLE;
         end
         HEAD1 : begin
           //wait for the tag_generator to grant a tag via tag_gnt and then
           //write the first 64-bits of a non-posted header into the 
           //non-posted fifo
           header_data_out <= {rsvd,fmt[1:0],MRd,rsvd,TC,rsvd,rsvd,rsvd,rsvd,
                               TD,EP,ATTR,rsvd,rsvd,length[9:0],req_id[15:0],
                               tag_value[7:0],LastBE,FirstBE};
           ack <= 0;
           tag_inc <=1'b0;   
           tx_waddr[4:0] <= 0;
           tx_wdata[31:0] <= 0;
           tx_we <= 1'b0;
           if(tag_gnt == 1'b0)begin
             state <= HEAD1;
             header_data_wren <= 1'b0;
           end else begin
             header_data_wren <= 1'b1;
             state <= HEAD2;
           end
         end
         HEAD2 : begin
             //write the next 32 or 64 bits of a non-posted header to the
             //non-posted header fifo (32 if 3DW - 64 if 4DW header)
             header_data_out <= (fmt[0]==1'b1) 
                                ? {dmaras_reg[63:2],2'b00} 
                                : {dmaras_reg[31:2], 2'b00, dmaras_reg[63:32]};
             header_data_wren <= 1'b1;
             //also write needed information by the RX engine into the 
             //Read Request Wrapper
             tx_waddr[4:0] <= tag_value[4:0];
////             tx_wdata[31:0] <= {length[9:0],dmarad_reg[27:6]};
             tx_wdata[31:0] <= {isDes,9'b0_0000_0000,dmarad_reg[27:6]};
             tx_we <= 1'b1;
             ack <= 1'b1; //acknowledge to the non-posted_packet_slicer that
                          //the packet has been queued up for transmission  
             tag_inc <=1'b1;//only assert tag_inc once - the tag gets 
                            //incremented for every clock cycle that it is
                            //asserted         
             state <= WAIT_FOR_GO_DEASSERT;
         end
         WAIT_FOR_GO_DEASSERT : begin
             //ack causes "go" to deassert but we need to give the 
             //non-posted_pkt_slicer a chance to deassert "go" before returning
             //to IDLE 
             header_data_wren <= 1'b0;
             tx_we <= 1'b0;
             tag_inc <=1'b0; 
             ack <= 1'b0;                
             state <= IDLE;
         end
         default : begin
             header_data_out <= 0;
             header_data_wren <= 1'b0;
             ack <= 1'b0;
             tag_inc <=1'b0;  
             tx_waddr[4:0] <= 0;
             tx_wdata[31:0] <= 0;
             tx_we <= 1'b0;
             state <= IDLE;
         end
      endcase
   end
 end
endmodule
