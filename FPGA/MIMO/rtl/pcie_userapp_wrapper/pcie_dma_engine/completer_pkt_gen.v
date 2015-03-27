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
// Module Name:    completer_pkt_gen 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: Completer Packet Generator module.  This block creates the header
// info for completer packets- it also passes along the data source and 
// address info for the TRN state machine block to request the data from 
// the egress data presenter.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module completer_pkt_gen(
    input clk,
    input rst,
    //interface from RX Engine
    input [6:0] bar_hit, //denotes which base address was hit
    input comp_req, //gets asserted when the rx engine recevies a MemRd request
    input [31:0] MEM_addr, //needed to fetch data from egress_data_presenter
    input [15:0] MEM_req_id,//needed for completion header
    input [15:0] comp_id, //needed for completion header
    input [7:0] MEM_tag, //neede for completion header
    //interface to completion header fifo
    output reg comp_fifo_wren,
    output reg [63:0] comp_fifo_data
);


//State machine states
localparam IDLE = 4'h0; 
localparam HEAD1 = 4'h1; 
localparam HEAD2 = 4'h2;

//parameters used to define fixed header fields
localparam rsvd = 1'b0;
localparam fmt = 2'b10; //always with data
localparam CplD = 5'b01010; //completer
localparam TC = 3'b000;
localparam TD = 1'b0;
localparam EP = 1'b0;
localparam ATTR = 2'b00;
localparam Length = 10'b0000000001; //length is always one DWORD
localparam ByteCount = 12'b000000000100; //BC is always one DWORD
localparam BCM = 1'b0;

    reg [3:0] state;
    reg [6:0] bar_hit_reg;
    reg [26:0] MEM_addr_reg;
    reg [15:0] MEM_req_id_reg;
    reg [15:0] comp_id_reg;
    reg [7:0] MEM_tag_reg;

reg   rst_reg;
always@(posedge clk) rst_reg <= rst;

//if there is a memory read request then latch the header information
//needed to create the completion TLP header
always@(posedge clk)begin
    if(comp_req)begin
      bar_hit_reg <= bar_hit;
      MEM_addr_reg[26:0] <= MEM_addr[26:0];
      MEM_req_id_reg <= MEM_req_id;
      comp_id_reg <= comp_id;
      MEM_tag_reg <= MEM_tag;
    end
end


// State machine
// Builds headers for completion TLP headers 
// Writes them into a FIFO
always @ (posedge clk) begin
  if (rst_reg) begin
      comp_fifo_data <= 0;
      comp_fifo_wren <= 1'b0;
      state <= IDLE;
  end else begin
      case (state)
        IDLE : begin
           comp_fifo_data <= 0;
           comp_fifo_wren <= 1'b0; 
           if(comp_req)
             state<= HEAD1;
           else
             state<= IDLE;
         end
         HEAD1 : begin //create first 64-bit completion TLP header
//NOTE: bar_hit_reg[6:0],MEM_addr_reg[26:2] are not part of completion TLP 
//header but are used by tx_trn_sm module to fetch data from the 
//egress_data_presenter
             comp_fifo_data <= {bar_hit_reg[6:0],MEM_addr_reg[26:2],            
                                rsvd,fmt,CplD,rsvd,TC,rsvd,rsvd,rsvd,rsvd,
                                TD,EP,ATTR,rsvd,rsvd,Length};  

             comp_fifo_wren <= 1'b1; //write to comp header fifo
             state <= HEAD2;
         end
         HEAD2 : begin //create second 64-bit completion TLP header
             comp_fifo_data <= {comp_id_reg[15:0],3'b000, BCM,ByteCount,
                                MEM_req_id_reg[15:0],MEM_tag_reg[7:0],rsvd,
                                MEM_addr_reg[6:0]};
             comp_fifo_wren <= 1'b1; //write to comp header fifo          
             state <= IDLE;
         end
         default : begin
             comp_fifo_data <= 0;
             comp_fifo_wren <= 1'b0; 
             state <= IDLE;
         end
      endcase
   end
 end


endmodule
