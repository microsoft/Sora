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

`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: Microsoft Research Asia	
// Engineer: Jiansong Zhang
// 
// Create Date:    12:17:59 11/12/2009 
// Design Name: 
// Module Name:    performance_counter 
// Project Name:   Sora
// Target Devices: LX50T1136-1
// Tool versions: ISE 10.02
// Description: We measure the durations in this module (1) from TX_des request sent to tx_engine to new des 
//              received (2) from transfer start to transfer done. 
//              A counter (125MHz or 250MHz depends on DMA clock) is implemeted. 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module performance_counter(
	input clk,
	input rst,
	input transferstart_one,
	input rd_dma_done_one,
	input new_des_one,
	output reg [23:0] round_trip_latency,
	output reg [23:0] transfer_duration
    );

    reg [39:0] counter;    /// free run 40bits counter, more than two hours per cycle on 125MHz clock
	 reg [23:0] snapshot_transferstart;   /// record the lower 24 bit of counter when transferstart, more than 100ms per cycle on 125MHz clock

    /// counter
    always@(posedge clk) begin
	    if(rst)
		    counter <= 40'h00_0000_0000;
		 else
		    counter <= counter + 40'h00_0000_0001;
	 end
	 
	 /// snapshot_transferstart
	 always@(posedge clk) begin
	    if(rst)
		    snapshot_transferstart <= 24'h00_0000;
		 else if (transferstart_one)
		    snapshot_transferstart <= counter[23:0];
		 else
		    snapshot_transferstart <= snapshot_transferstart;
	 end
	 
	 /// round_trip_latency
	 always@(posedge clk) begin
	    if (rst)
		    round_trip_latency <= 24'h00_0000;
		 else if (new_des_one)
		    round_trip_latency <= counter[23:0] + (~snapshot_transferstart) + 24'h00_0001;
		 else
		    round_trip_latency <= round_trip_latency;
	 end
	 
	 /// transfer_duration
	 always@(posedge clk) begin
	    if (rst)
		    transfer_duration <= 24'h00_0000;
		 else if (rd_dma_done_one)
		    transfer_duration <= counter[23:0] + (~snapshot_transferstart) + 24'h00_0001;
		 else
		    transfer_duration <= transfer_duration;
	 end

endmodule
