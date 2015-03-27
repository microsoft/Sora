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
// © 2007-2008 Xilinx, Inc. All Rights Reserved.
// Confidential and proprietary information of Xilinx, Inc.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____ 
//  /   /\/   / 
// /___/  \  /   Vendor: Xilinx 
// \   \   \/    Version: 1.0
//  \   \        Filename:  performance_counters.v
//  /   /        Date Last Modified: Apr. 1st, 2008 
// /___/   /\    Date Created: Apr. 1st, 2008
// \   \  /  \ 
//  \___\/\___\ 
//
// Device: Virtex-5 LXT
// Purpose: Counters for the Host to compute RX and TX transfer bandwidth.
// Counters begin counting when xx_dma_start signal is detected and stop
// xx_dma_done signal is detected.   
// Reference:  XAPP859
// Revision History:
//   Rev 1.0 - First created, Kraig Lund, Apr. 1 2008.
///////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module performance_counters(
    input clk,
    input rst,
    input wr_dma_start,
    input wr_dma_done,
    input rd_dma_start,
    input rd_dma_done,
    output reg [31:0] dma_wr_count,
    output reg [31:0] dma_rd_count,
    input read_last,
    input write_last
    );

wire wr_dma_start_one;
wire rd_dma_start_one;
wire wr_dma_done_one;
wire rd_dma_done_one;

reg write_active, read_active;

rising_edge_detect wr_dma_start_one_inst(
                .clk(clk),
                .rst(rst),
                .in(wr_dma_start),
                .one_shot_out(wr_dma_start_one)
                );
                
rising_edge_detect rd_dma_start_one_inst(
                .clk(clk),
                .rst(rst),
                .in(rd_dma_start),
                .one_shot_out(rd_dma_start_one)
                );
rising_edge_detect wr_dma_done_one_inst(
                .clk(clk),
                .rst(rst),
                .in(wr_dma_done),
                .one_shot_out(wr_dma_done_one)
                );
rising_edge_detect rd_dma_done_one_inst(
                .clk(clk),
                .rst(rst),
                .in(rd_dma_done),
                .one_shot_out(rd_dma_done_one)
                );

//detect when a read or a write dma is in process and assert
//write_active or read_active - these two signals are used
//as clock enables for the actual performance counters
always@(posedge clk)begin
   if(rst)begin
      write_active <= 1'b0;
   end else if(wr_dma_start_one | wr_dma_done_one)begin
      case({wr_dma_start_one, wr_dma_done_one, write_last})
         3'b000,3'b001,3'b110,3'b111:begin //invalid case included for
                                           // completeness
              write_active <= write_active;
         end
         3'b011:begin
              write_active <= 1'b0;
         end
         3'b100,3'b101,3'b010:begin
              write_active <= 1'b1;
         end
         default:begin write_active <= 1'b0;end
      endcase
   end else begin
       write_active <= write_active;  //implied
   end
end

always@(posedge clk)begin
   if(rst)begin
      read_active <= 1'b0;
   end else if(rd_dma_start_one | rd_dma_done_one)begin
      case({rd_dma_start_one, rd_dma_done_one, read_last})
         3'b000,3'b001,3'b110,3'b111:begin  //invalid cases included for 
                                            //completeness
              read_active <= read_active;
         end
         3'b011:begin
              read_active <= 1'b0;
         end
         3'b100,3'b101,3'b010:begin
              read_active <= 1'b1;
         end
         default:begin read_active <= 1'b0;end
      endcase
   end else begin
       read_active <= read_active;  //implied
   end
end


always@(posedge clk)begin
   if(rst)
      dma_wr_count[31:0] <= 0;
   else if(write_active)
      dma_wr_count <= dma_wr_count + 1;
end      
 
always@(posedge clk)begin
   if(rst)
      dma_rd_count[31:0] <= 0;
   else if(read_active)
      dma_rd_count <= dma_rd_count + 1;
end    

endmodule
