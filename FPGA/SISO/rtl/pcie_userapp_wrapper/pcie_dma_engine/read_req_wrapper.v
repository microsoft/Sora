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
// Module Name:    read_req_wrapper 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: A wrapper which contains a RAM with read request information
// (dualport_32x32_compram) and a small shadow RAM which qualifies the 
// entries in the dualport_32x32_compram
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module read_req_wrapper(
          input clk,
          input rst,
			 input transferstart,     // Jiansong: control signal for transfer recovering
          input [4:0] tx_waddr,
          input [31:0] tx_wdata,
          input tx_we,
          input [4:0] rx_waddr,
          input [31:0] rx_wdata,
          input rx_we,
          input [4:0] rx_raddr,
          output [31:0] rx_rdata,
          input  pending_comp_done,    /// Jiansong: for what purpose?
          output [31:0] completion_pending,
          output comp_timeout
     );


  reg we;
  reg [4:0] wr_addr;
  reg [31:0] wr_data;  
  
  reg [4:0] wr_addr_r;
  wire [4:0] wr_addr_rst;
  reg pending_data;

  //32x32 dual-port distributed ram built with CoreGen
  //this ram holds completion information for the RX engine
  //provided from the TX engine
  //The tag from outgoing non-posted memory read requests is
  //used as the address to the ram.  The data stored in the
  //32-bit data window is as follows
  //  bits 21 - 0 = ddr2 destination address (6 LSBs not stored)
  //       31 - 22 = length in DWORDs of the request
  //
  //Both the RX and TX engines have write access to this ram
  //The TX engine writes the initial information and the RX engine
  //updates the DDR2 address as the completion packets are serviced
  //In order to share the write port a simple protocol is followed
  //    1. TX engine has priority over RX engine
  //    2. TX engine writes are always a single-cycle and cannot occur in
  //       consecutive cycles
  //    3. RX engine writes are always double-cycle       

  always@(posedge clk) begin
     if (rst | (~transferstart))
        wr_addr_r[4:0] <= wr_addr_r[4:0] + 5'b00001;
	  else
	     wr_addr_r[4:0] <= 5'b00000;
  end
  
  assign wr_addr_rst = wr_addr_r;

/// Jiansong: reset logic added. clear the rams
//  always@(*)begin
   always@(posedge clk)begin
      if(rst | (~transferstart))begin
		   wr_addr[4:0]  <= wr_addr_rst[4:0];
			we            <= 1'b1;
			wr_data[31:0] <= 32'h0000_0000;
			pending_data  <= 1'b0;
      end else if(tx_we)begin
         wr_addr[4:0]  <= tx_waddr[4:0];
         we            <= tx_we;
         wr_data[31:0] <= tx_wdata[31:0];
			pending_data  <= 1'b1;
      end else begin
         wr_addr[4:0]  <= rx_waddr[4:0];
         we            <= rx_we;
         wr_data[31:0] <= rx_wdata[31:0];
			pending_data  <= ~pending_comp_done;
      end 
  end
  
  dualport_32x32_compram dualport_32x32_compram_inst(
        .a(wr_addr[4:0]),
        .d(wr_data[31:0]),
        .dpra(rx_raddr[4:0]),
        .clk(clk),
        .we(we),
        .spo(),
        .dpo(rx_rdata[31:0])
     );

  //A small 32x1 shadow ram to qualify the entries in the dualport
  //This ram has an addressable write port, the read data port is not
  //addressable, i.e. all 32 bits are present. The data stored represents
  //whether the dual-port ram contains a valid entry or not.  The tx engine
  //sets the bit when writing to the dualport ram - the rx engine clears it
  //when the final completion packet has been received.
  //Same protocol rules as dual-port ram for writes

  pending_comp_ram_32x1 pending_comp_ram_32x1_inst(
          .clk(clk),
//          .d_in((tx_we)?1'b1:~pending_comp_done),
          .d_in(pending_data),
          .addr(wr_addr[4:0]),
          .we(we),
          .d_out(completion_pending[31:0])
    );

  completion_timeout completion_timeout_inst(
          .clk(clk),
          .rst(rst | (~transferstart)),
          .pending_req(completion_pending[31:0]),
          .comp_timeout(comp_timeout)
    );
    

endmodule
