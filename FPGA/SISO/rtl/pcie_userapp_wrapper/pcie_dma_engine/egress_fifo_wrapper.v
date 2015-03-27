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
//  \   \        Filename:  egress_fifo_wrapper.v
//  /   /        Date Last Modified: Apr. 1st, 2008 
// /___/   /\    Date Created: Apr. 1st, 2008
// \   \  /  \ 
//  \___\/\___\ 
//
// Device: Virtex-5 LXT
// Purpose: This module wraps two fifo36 primitives and pipelines the Read
//          data output along with the Empty flag.  The interface still 
//          appears exactly like a regular FIFO36 without a pipeline, except
//          there is one extra clock cycle of latency when the fifo goes to
//          a non-empty state.
//
// Reference:  XAPP859
// Revision History:
//   Rev 1.0 - First created, Kraig Lund, Apr. 1 2008.
///////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module egress_fifo_wrapper(
    input  wire RST,
    input  wire WRCLK,
    input  wire WREN,
    input  wire [127:0] DI,
    output wire FULL,
    output wire ALMOSTFULL,
    input  wire RDCLK,
    input  wire RDEN,
    output reg [127:0] DO,
    output wire EMPTY,
    output wire ALMOSTEMPTY
    );

//state machine states
localparam EMPTY_STATE = 2'b00;
localparam DEASSERT_EMPTY = 2'b01;
localparam RDEN_PASS = 2'b10;
localparam WAIT = 2'b11;

wire        full_a;
wire        almostfull_a;
wire        full_b;
wire        almostfull_b;
wire        almostempty_a;
wire        almostempty_b;
wire        empty_a;
wire        empty_b;

wire        empty_or;
reg         empty_reg;
wire        almostempty_or;
reg         almostempty_reg;

reg         rden_reg;
wire        rden_fifo;
reg [1:0] state;

//if the user wishes to remove the pipeline on the read
//path then define SINGLECYCLE
//This reference design does not define SINGLECYCLE as
//the pipeline is needed for 250 MHz timing
`ifndef SINGLECYCLE
reg rden_d1; 
`endif

wire [127:0] do_fifo;

//EGRESS or READ from DDR2 MEMORY FIFO
FIFO36_72 #( 
.ALMOST_EMPTY_OFFSET (9'h005),
.ALMOST_FULL_OFFSET  (9'h114),
.DO_REG              (1),
.EN_ECC_WRITE        ("FALSE"),
.EN_ECC_READ         ("FALSE"),
.EN_SYN              ("FALSE"),
.FIRST_WORD_FALL_THROUGH ("FALSE"))
egress_fifo_a(
.ALMOSTEMPTY (almostempty_a), 
.ALMOSTFULL  (almostfull_a), 
.DBITERR     (), 
.DO          (do_fifo[63:0]), 
.DOP         (), 
.ECCPARITY   (), 
.EMPTY       (empty_a),
.FULL        (full_a), 
.RDCOUNT     (), 
.RDERR       (), 
.SBITERR     (), 
.WRCOUNT     (), 
.WRERR       (),          
.DI          (DI[63:0]), 
.DIP         (), 
.RDCLK       (RDCLK), 
.RDEN        (rden_fifo),
.RST         (RST), 
.WRCLK       (WRCLK), 
.WREN        (WREN)
);

FIFO36_72 #( 
.ALMOST_EMPTY_OFFSET (9'h005),
.ALMOST_FULL_OFFSET  (9'h114),
.DO_REG              (1),
.EN_ECC_WRITE        ("FALSE"),
.EN_ECC_READ         ("FALSE"),
.EN_SYN              ("FALSE"),
.FIRST_WORD_FALL_THROUGH ("FALSE"))
egress_fifo_b(
.ALMOSTEMPTY (almostempty_b), 
.ALMOSTFULL  (almostfull_b), 
.DBITERR     (), 
.DO          (do_fifo[127:64]), 
.DOP         (), 
.ECCPARITY   (), 
.EMPTY       (empty_b),
.FULL        (full_b), 
.RDCOUNT     (), 
.RDERR       (), 
.SBITERR     (), 
.WRCOUNT     (), 
.WRERR       (),          
.DI          (DI[127:64]), 
.DIP         (), 
.RDCLK       (RDCLK), 
.RDEN        (rden_fifo),
.RST         (RST), 
.WRCLK       (WRCLK), 
.WREN        (WREN)
);


//Careful with the fifo status signals when using two fifos in parallel

//Empty flags (and Almost Empty flags) which are synchronous to rdclk
//could deassert on different rdclk cycles due to minute differences in the
//wrclk arrival time (wrclk clock skew).  This is because deassertion 
//is caused by writing data into an empty fifo i.e. a wrclk domain event
//and this event must cross clock domains. 
//Assertion is caused by reading the last piece of data out of the fifo. 
//Since rden is a rdclk domain signal/event it is guaranteed that both fifos 
//will assert empty on the same rdclk cycle (as long as rden and rdclk are
//are the same signals for both fifos)

//Similarily the Full flags (and almost full flags) which are synchronous to
//wrclk could deassert on different wrclk cycles due to minute differences
//in the rdclk arrival time (rdclk clock skew).

//In both cases the flags should be wire or'ed (since they are positive logic)
//so that the flag doesn't deassert unless both flags are deasserted
assign empty_or = empty_a | empty_b;
assign almostempty_or = almostempty_a | almostempty_b;
assign ALMOSTFULL = almostfull_a | almostfull_b; 
assign FULL = full_a | full_b; 

assign EMPTY = empty_reg; //empty_reg is output from
                          //the state machine

`ifdef SINGLECYCLE
assign rden_fifo = (RDEN | rden_reg) & ~empty_or; 
`else
//read the fifo if the state machine requests it (rden_reg)
//or if the outside RDEN is asserted (delayed by one - rden_d1)        
assign rden_fifo = (rden_d1 | rden_reg) & ~empty_or; 
`endif

//pipeline the read path for 250 Mhz timing
//empty flag gets registered in the state machine
always@(posedge RDCLK)begin
   almostempty_reg <= almostempty_or;
end


//pipeline the read data path to match the pipelined empty signal
always@(posedge RDCLK)begin
  if(RDEN)
        DO[127:0] <= do_fifo[127:0];
end

`ifndef SINGLECYCLE
//once the FIFO36 actually go empty, do not pass the RDEN signal to the
//primitives - otherwise pass it to keep the pipeline filled
always@(posedge RDCLK)begin
  if(state == RDEN_PASS & (empty_or & RDEN))
      rden_d1 <= 1'b0;
  else if(state == RDEN_PASS & ~(empty_or & RDEN))
      rden_d1 <= RDEN;
end
`endif

//State machine block
//This state machine monitors the empty flags of the FIFO36 primitives and
//controls the assertion/deassertion of pipelined empty flag.  It also controls
//whether the RDEN gets passed to the actual FIFO36 or not
always@(posedge RDCLK)begin
    if(RST)begin
        state <= EMPTY_STATE;
        empty_reg <= 1;
        rden_reg <= 0;
    end else begin
        case(state)
           EMPTY_STATE:begin
                   empty_reg <= 1'b1;
                   if(~empty_or)begin
                       rden_reg <= 1'b1;
                       state <= DEASSERT_EMPTY;
                   end else begin
                       rden_reg <= 1'b0;
                       state <= EMPTY_STATE;
                   end
            end
            DEASSERT_EMPTY:begin //deassert the empty flag one clock later
                   empty_reg <= 1'b0;
                   rden_reg <= 1'b0;
                   state <= RDEN_PASS;
            end
            RDEN_PASS:begin //now allow the RDEN signal to pass to the FIFO36
                    rden_reg <= 1'b0;
                    if(empty_or & RDEN)begin
                       empty_reg <= 1'b1;
                       `ifdef SINGLECYCLE
                         state <= EMPTY_STATE;
                       `else
                         state <= WAIT;
                       `endif
                    end else begin
                       empty_reg <= 1'b0;
                       state <= RDEN_PASS;
                    end
             end
             `ifndef SINGLECYCLE
             WAIT:begin
                  empty_reg <= 1'b1;
                  rden_reg <= 1'b0;
                  state <= EMPTY_STATE;
             end
             `endif
             default:begin
                  state <= EMPTY_STATE;
                  empty_reg <= 1;
                  rden_reg <= 0;
             end
           endcase
     end
end     


endmodule
