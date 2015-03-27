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
// Purpose: A small shadow-RAM to qualify the data entries in the dual-port
// compram 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
// Modification by zjs, 2009-6-18, pending
//    (1) move posted packet generator and non-posted packet generator out --- done 
//    (2) add dma write data fifo --------------- done
//    (3) modify tx sm
//          scheduling -------------------------- done
//          disable write dma done -------------- done
//          register/memory read ---------------- done
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module pending_comp_ram_32x1(    
          input clk,
          input d_in,
          input [4:0] addr,
          input we,
          output reg [31:0] d_out = 32'h00000000
       );


    wire [31:0] addr_decode;

    //binary-to-onehot address decoder for ram
    assign addr_decode[31:0] = 32'h00000001 << addr[4:0]; 

    //generate a 32-entry ram with 
    //addressable inputs entries
    //and outputs which are always present;
    //essentially this is a 32x1 register file
    genvar i;
    generate
    for(i=0;i<32;i=i+1)begin: bitram
        always@(posedge clk)begin
           if(addr_decode[i] && we)begin
              d_out[i] <= d_in;
           end
        end                           
    end
    endgenerate

endmodule
