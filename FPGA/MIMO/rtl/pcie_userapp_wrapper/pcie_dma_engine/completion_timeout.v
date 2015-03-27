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
// Purpose: Contains 25 ms timers for each outstanding read request.  If any 
// timer reaches 0  than completion timeout is signalled.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module completion_timeout(
          input clk, 
          input rst, 
          input [31:0] pending_req,
          output reg comp_timeout
  );

`ifdef ML505
reg [15:0] count;
`else
reg [17:0] count;  //free running counter creates 1.048576 ms timer
`endif

wire [31:0] pending_req_rise;
wire [31:0] pending_req_fall;
reg [31:0] shift_in = 0;
reg [4:0] reset_count[31:0];
wire [31:0] srl_reset;
wire [31:0] comp_timeout_vector;
reg [31:0]  comp_timeout_vector_d1;
wire        comp_timeout_or;
wire        comp_timeout_one;
reg         comp_timeout_d1;

    always@(posedge clk)begin
            if(rst)
              count <= 0;
            else 
              count <= count + 1;
     end

    //timer is asserted every 1.045876 ms
    assign timer = (count == 0) ? 1'b1 : 1'b0;

    //create 32 shift register instances and associated logic
    genvar i;
    generate
    for(i=0;i<32;i=i+1)begin: replicate
        edge_detect edge_detect_inst(
          .clk(clk),
          .rst(rst),
          .in(pending_req[i]),
          .rise_out(pending_req_rise[i]),
          .fall_out(pending_req_fall[i])
         );

        //pending req logic
        //create a signal that sets when pending req is high and resets
        //timer pulses.  Pending req gets priority over timer if both
        //signals occur simultaneously
        always@(posedge clk)begin
           if(pending_req_rise[i]) //set latch
              shift_in[i] <= 1'b1;
           else if(timer || pending_req_fall[i])    //reset latch
              shift_in[i] <= 1'b0;
        end

         always@(posedge clk)begin
             if(rst)
                reset_count[i][4:0] <= 5'b00000;
             else if (pending_req_fall[i] == 1'b1)
                reset_count[i][4:0] <= 5'b11001;
             else if (reset_count[i][4:0] == 5'b00000)
                reset_count[i][4:0] <= 5'b00000;
             else 
                reset_count[i][4:0] <= reset_count[i][4:0] - 1;
         end
         assign srl_reset[i] = | reset_count[i][4:0];    



         SRLC32E #(
           .INIT(32'h00000000)
         ) SRLC32E_inst (
            .Q(comp_timeout_vector[i]),     // SRL data output
            .Q31(), // SRL cascade output pin
            .A(5'b11000),  // 5-bit shift depth select input
            .CE((srl_reset[i]) ? srl_reset[i] : timer),   // Clock enable input
            .CLK(clk), // Clock input
            .D((srl_reset[i]) ? ~srl_reset[i] : shift_in[i]) // SRL data input
          );
                 
    end
    endgenerate

    always@(posedge clk)begin
            comp_timeout_vector_d1[31:0] <= comp_timeout_vector[31:0];
    end

    assign  comp_timeout_or = |comp_timeout_vector_d1[31:0];


rising_edge_detect comp_timeout_one_inst(
                .clk(clk),
                .rst(rst),
                .in(comp_timeout_or),
                .one_shot_out(comp_timeout_one)
                );
 
    always@(posedge clk)begin
        comp_timeout_d1 <= comp_timeout_one;
        comp_timeout <= comp_timeout_d1;
    end

endmodule
