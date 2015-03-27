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
// Company:     Microsoft Research Asia
// Engineer:    Jiansong
// 
// Create Date:    20:44:03 06/25/2009 
// Design Name: 
// Module Name:    posted_pkt_scheduler 
// Project Name:   Sora
// Target Devices: Virtex5 LX50T
// Tool versions:  ISE 10.1.02
// Description: This module schedule posted packet header and data generation. There's
//              two kinds of posted packets in sora: RX data packet and TX descriptor 
//              write back packet. TX descriptor write back has higher priority than
//              RX data packet.
//     Input of this module: RX data fifo, TX desc write back request
//     Output of this module: posted packet header fifo, dma write (posted data) fifo
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module posted_pkt_scheduler(
    input clk,
	 input rst,
	 //interface to PCIe Endpoint Block Plus
	 input [2:0]    max_pay_size,
	 //interface from/to RX data fifo
	 input [63:0]   RX_FIFO_data,
	 output reg     RX_FIFO_RDEN,
	 input          RX_FIFO_pempty,
	 //interface from/to dma ctrl wrapper
	 /// TX descriptor write back
	 input          TX_desc_write_back_req,
	 output reg     TX_desc_write_back_ack,
	 input [63:0]   SourceAddr,
	 input [31:0]   DestAddr,
	 input [23:0]   FrameSize,
	 input [7:0]    FrameControl,
	 input [63:0]   DescAddr,
	 /// RX control signals
	 input          RXEnable,              // not used
	 input [63:0]   RXBufAddr,
	 input [31:0]   RXBufSize,
	 //interface from/to dma write data fifo in TX engine
	 output reg [63:0]   dma_write_data_fifo_data,
	 output reg          dma_write_data_fifo_wren,
	 input               dma_write_data_fifo_full,
	 //interface to posted pkt builder
	 output reg        go,
	 input             ack,
	 output reg [63:0] dmawad,
	 output     [9:0]  length,
	 //interface from a64_128_distram_p(posted header fifo)
	 input           posted_fifo_full,
	 // debug interface
	 output reg [31:0] Debug20RX1,
	 output reg [4:0]  Debug22RX3,
	 output reg [31:0] Debug24RX5,
	 output reg [31:0] Debug26RX7,
	 output reg [31:0] Debug27RX8,
	 output reg [31:0] Debug28RX9,
	 output reg [31:0] Debug29RX10
    );

    //state machine state definitions for state
	 localparam IDLE                = 5'b00000;
	 localparam TX_DESC_WRITE_BACK  = 5'b00001;
	 localparam TX_DESC_WRITE_BACK2 = 5'b00010;
	 localparam TX_DESC_WRITE_BACK3 = 5'b00011;
	 localparam TX_DESC_WRITE_BACK4 = 5'b00100;
	 localparam WAIT_FOR_ACK        = 5'b00101;
	 localparam RX_PACKET           = 5'b00110;
	 localparam RX_PACKET2          = 5'b00111;
    localparam RX_PACKET3          = 5'b01000;	 
	 localparam RX_PACKET4          = 5'b01001;
	 localparam RX_PACKET5          = 5'b01010;
	 localparam RX_PACKET6          = 5'b01011;
	 localparam RX_PACKET_WAIT_FOR_ACK = 5'b01100;
	 localparam RX_DESC_WAIT        = 5'b10011;
	 localparam RX_DESC             = 5'b01101;
	 localparam RX_DESC2            = 5'b01110;
	 localparam RX_CLEAR            = 5'b10000;
	 localparam RX_CLEAR2           = 5'b11111;
	 localparam RX_CLEAR_WAIT_FOR_ACK = 5'b10101;
	 localparam RX_CLEAR_WAIT         = 5'b11100;
	 
	 reg [4:0] state;
	 
	 localparam RX_FRAME_SIZE_BYTES = 13'h0070;    // data frame size is 112 bytes
	 localparam TX_DESC_SIZE_BYTES  = 13'h0020;    // TX descriptor size is 32 bytes
	 localparam RX_DESC_SIZE_BYTES  = 13'h0010;    // RX descriptor size is 16 bytes
	 
	 reg [13:0]  cnt;            // counter for RX data
	 reg [63:0]  fifo_data_pipe; // pipeline data register between RX fifo and 
	                             // dma write data fifo, also swap the upper and lower
										  // DW

    reg         firstData;      // whether it's first data after reset
	 reg [31:0]  RoundNumber;	  // indicates how many rounds we have wroten in RX buffer
	 reg [31:0]  RoundNumber_next;
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]  dmawad_now1, dmawad_now2;
    reg [63:0]  dmawad_next;    // next RX data destination address
	 reg [63:0]  dmawad_desc;    // current rx descriptor address
//	 reg [63:0]  dmawad_next_r;  // pipeline register
	 
	 reg  [12:0] length_byte; // output posted packet length
	 
	 // Pipeline registers
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]   RXBufAddr_r1, RXBufAddr_r2;
	 reg [31:0]   RXBufSize_r;
	 
	 reg         rst_reg;	 
	 always@(posedge clk) rst_reg <= rst;

    // Debug register
	 always@(posedge clk)begin
	    Debug20RX1[3:0]   <= state[3:0];
		 Debug20RX1[7:4]   <= {3'b000,firstData};		 
		 Debug20RX1[23:8]  <= {3'b000,length_byte[12:0]};
		 Debug20RX1[27:24] <= {1'b0,max_pay_size};
		 Debug20RX1[31:28] <= {3'b000,posted_fifo_full};		 
	 end
	 
	 always@(posedge clk)begin
	    Debug22RX3[3:0]   <= {3'b000,dma_write_data_fifo_full};
		 Debug22RX3[4]     <= RX_FIFO_pempty;
	 end

    always@(posedge clk)begin
	    if (rst)
		    Debug24RX5 <= 32'h0000_0000;
		 else if (RX_FIFO_RDEN)
		    Debug24RX5 <= Debug24RX5 + 32'h0000_0001;
		 else
		    Debug24RX5 <= Debug24RX5;
	 end

    always@(posedge clk)begin
	    if(rst)
		    Debug26RX7 <= 32'h0000_0000;
		 else if (dma_write_data_fifo_wren)
		    Debug26RX7 <= Debug26RX7 + 32'h0000_0001;
		 else
		    Debug26RX7 <= Debug26RX7;
	 end
	 
	 always@(posedge clk)begin
	    if (rst)
		    Debug27RX8 <= 32'h0000_0000;
		 else if (state == RX_PACKET)
		    Debug27RX8 <= Debug27RX8 + 32'h0000_0001;
		 else
		    Debug27RX8 <= Debug27RX8;
	 end
	 
	 always@(posedge clk)begin
	    Debug28RX9  <= dmawad[31:0];
		 Debug29RX10 <= dmawad[63:32];
	 end

    always@(posedge clk)begin
	    RXBufAddr_r1 <= RXBufAddr;
		 RXBufAddr_r2 <= RXBufAddr;
		 RXBufSize_r <= RXBufSize;
	 end

	 wire [12:0] frame_size_bytes;    // RX data frame size, min(RX_FRAME_SIZE_BYTES, max_pay_size_bytes)
	 wire [12:0] max_pay_size_bytes;  // max payload size from pcie core
//calculate the max payload size in bytes for ease of calculations
//instead of the encoding as specified
//in the PCI Express Base Specification
assign max_pay_size_bytes =13'h0001<<(max_pay_size+7);
assign frame_size_bytes = (RX_FRAME_SIZE_BYTES <= max_pay_size_bytes) ? RX_FRAME_SIZE_BYTES : 
                                                                        max_pay_size_bytes; 
//generate TX_desc_write_back_ack signal
always@(posedge clk) begin
   if (rst_reg)
	    TX_desc_write_back_ack <= 1'b0;
   else if (state == TX_DESC_WRITE_BACK)
	    TX_desc_write_back_ack <= 1'b1;
	else
	    TX_desc_write_back_ack <= 1'b0;	    
end

/// Jiansong: pipeline data register between RX fifo and 
///           dma write data fifo, also swap the upper and lower DW
//always@(posedge clk) fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
always@(posedge clk) fifo_data_pipe[63:0] <= {RX_FIFO_data[15:0],RX_FIFO_data[31:16],
                                              RX_FIFO_data[47:32],RX_FIFO_data[63:48]};


//main state machine
always@ (posedge clk) begin
   if (rst_reg) begin
	    state <= IDLE;
		 RX_FIFO_RDEN <= 1'b0;
		 dma_write_data_fifo_wren <= 1'b0;
		 dma_write_data_fifo_data <= 64'h0000_0000_0000_0000;
		 go <= 1'b0;
		 firstData <= 1'b1;
		 cnt <= 13'h0000;
	end else begin
	    case (state)
		   IDLE : begin
			      cnt <= 13'h0000;
			      go <= 1'b0;
					RX_FIFO_RDEN <= 1'b0;
		         dma_write_data_fifo_wren <= 1'b0;
		         dma_write_data_fifo_data <= 64'h0000_0000_0000_0000;
					// check if need to generate a posted packet
					if(~posted_fifo_full & ~dma_write_data_fifo_full) begin
			         if(TX_desc_write_back_req)
				        state <= TX_DESC_WRITE_BACK;
			         else if (~RX_FIFO_pempty) begin
					        firstData <= 1'b0;
							  RX_FIFO_RDEN <= 1'b1;
					        cnt <= frame_size_bytes;
//				           state <= RX_PACKET;
                       // clear RX desc of next block at the front of a RX packet flow
                       state <= RX_CLEAR;
					     end
				      else
				        state <= IDLE;
					end else
					    state <= IDLE;
				end
			// start TX desc write back flow
			TX_DESC_WRITE_BACK : begin     // write TX descriptor into dma write data fifo
			      go <= 1'b0;
					RX_FIFO_RDEN <= 1'b0;
			      dma_write_data_fifo_wren <= 1'b1;
					// TimeStamp(4B), TXStatus(2B), CRC(2B)
					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
               state <= TX_DESC_WRITE_BACK2;					
			   end
			TX_DESC_WRITE_BACK2 : begin
			      go <= 1'b0;
					RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
					// SourceAddr(8B)
					dma_write_data_fifo_data[63:0] <= SourceAddr;
					state <= TX_DESC_WRITE_BACK3;
			   end
			TX_DESC_WRITE_BACK3 : begin
			      go <= 1'b0;
					RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
					// NextDesc(4B), DestAddr(4B)
					dma_write_data_fifo_data[63:0] <= {32'h0000_0000,DestAddr};
					state <= TX_DESC_WRITE_BACK4;
			   end
			TX_DESC_WRITE_BACK4 : begin
			      go <= 1'b1;
					RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
					// Reserved(4B), FrameControl, FrameSize(3B)
					dma_write_data_fifo_data[63:0] <= {32'h0000_0000,FrameControl,FrameSize};
					state <= WAIT_FOR_ACK;
			   end
			WAIT_FOR_ACK : begin
			      RX_FIFO_RDEN <= 1'b0;
			      dma_write_data_fifo_wren <= 1'b0;
					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
					if(ack) begin
					  go <= 1'b0;
					  state <= IDLE;
					end else begin
					      go <= 1'b1;
					      state <= WAIT_FOR_ACK;
					end
			   end
				
			// start of a clear next desc -> generate RX packet -> write des flow, 
			// clear RX descriptor of next block
			RX_CLEAR : begin
			      RX_FIFO_RDEN <= 1'b0;
			      go <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
					// clear RX desc
					dma_write_data_fifo_data[63:0] <= {64'h0000_0000_0000_0000};
					state <= RX_CLEAR2;
			   end
			RX_CLEAR2 : begin
			      RX_FIFO_RDEN <= 1'b0;
			      go <= 1'b1;
					dma_write_data_fifo_wren <= 1'b1;
					// write round number in RX desc. In case it's the head of RX buffer, 
					// RoundNumber could be old
					dma_write_data_fifo_data[63:0] <= {32'h0000_0000,RoundNumber};
					state <= RX_CLEAR_WAIT_FOR_ACK;	
			   end
			RX_CLEAR_WAIT_FOR_ACK : begin
			      RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b0;
					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
					if(ack) begin
					   go <= 1'b0;
						if (~posted_fifo_full & ~dma_write_data_fifo_full)
							state <= RX_PACKET;
						else
						   state <= RX_CLEAR_WAIT;
					end else begin
					   go <= 1'b1;
						state <= RX_CLEAR_WAIT_FOR_ACK;
					end
			   end
			RX_CLEAR_WAIT : begin
			      if (~posted_fifo_full & ~dma_write_data_fifo_full)
						state <= RX_PACKET;
					else
						state <= RX_CLEAR_WAIT;
			end
			
			// start of RX packet generation flow
			RX_PACKET : begin
			      go <= 1'b0;
					cnt <= cnt - 13'h0008;
					RX_FIFO_RDEN <= 1'b1;
					dma_write_data_fifo_wren <= 1'b0;
					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
			      state <= RX_PACKET2;
			   end
			RX_PACKET2 : begin
			      go <= 1'b0;
					cnt <= cnt - 13'h0008;
					RX_FIFO_RDEN <= 1'b1;
					dma_write_data_fifo_wren <= 1'b0;
					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
					state <= RX_PACKET3;
			   end
			RX_PACKET3 : begin
			      go <= 1'b0;
			      RX_FIFO_RDEN <= 1'b1;
					dma_write_data_fifo_wren <= 1'b1;
////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
					dma_write_data_fifo_data[63:0] <= fifo_data_pipe[63:0];
			      if (cnt == 13'h0010) begin
					   state <= RX_PACKET4;
					end else begin
					      cnt   <= cnt - 13'h0008;
							state <= RX_PACKET3;
					end
			   end
			RX_PACKET4 : begin
			      go <= 1'b0;
					RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
					dma_write_data_fifo_data[63:0] <= fifo_data_pipe[63:0];
					state <= RX_PACKET5;
			   end
			RX_PACKET5 : begin
			      go <= 1'b0;
					RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
					dma_write_data_fifo_data[63:0] <= fifo_data_pipe[63:0];
					state <= RX_PACKET6;
			   end
			RX_PACKET6 : begin
			      go <= 1'b1;
					RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
					dma_write_data_fifo_data[63:0] <= fifo_data_pipe[63:0];
					state <= RX_PACKET_WAIT_FOR_ACK;
			   end
			RX_PACKET_WAIT_FOR_ACK : begin
			      RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b0;
					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
					if(ack) begin
					   go <= 1'b0;
						if (~posted_fifo_full & ~dma_write_data_fifo_full)
							state <= RX_DESC;
						else
						   state <= RX_DESC_WAIT;
					end else begin
					   go <= 1'b1;
						state <= RX_PACKET_WAIT_FOR_ACK;
					end
			   end
			RX_DESC_WAIT : begin
			      if (~posted_fifo_full & ~dma_write_data_fifo_full)
						state <= RX_DESC;
					else
					   state <= RX_DESC_WAIT;
			end
				
			// start of RX desc flow
			RX_DESC : begin
			      RX_FIFO_RDEN <= 1'b0;
			      go <= 1'b0;
					dma_write_data_fifo_wren <= 1'b1;
					// data will be swapped later
//					dma_write_data_fifo_data[63:0] <= {56'h00_0000_0000_0000,8'h01};
					dma_write_data_fifo_data[63:0] <= 64'hFFFF_FFFF_FFFF_FFFF;	// modified by Jiansong, 2010-6-22, support up to 64 virtual streams
					state <= RX_DESC2;
			   end
			RX_DESC2 : begin
			      RX_FIFO_RDEN <= 1'b0;
			      go <= 1'b1;
					dma_write_data_fifo_wren <= 1'b1;
//					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
               // Write Round number in RX desc. It's used as debug info
               dma_write_data_fifo_data[63:0] <= {32'h0000_0000,RoundNumber};
					state <= WAIT_FOR_ACK;					
			   end
			default: begin
			      go <= 1'b0;
					RX_FIFO_RDEN <= 1'b0;
					dma_write_data_fifo_wren <= 1'b0;
					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
					state <= IDLE;
			   end
		endcase
	end
end

// update dmawad_next
always@(posedge clk) begin
   if(firstData | rst_reg) begin
	   dmawad_next <= RXBufAddr_r1;
		RoundNumber_next <= 32'h0000_0000;
	end else if (state == RX_PACKET3) begin
	   // if end of RX buf is arrived, dmawad_next will return to start address at next cycle
//	   if((dmawad_now2 + 64'h0000_0000_0000_0080) == (RXBufAddr_r2+RXBufSize_r))
		if((dmawad_now2 + 64'h0000_0000_0000_0080) >= (RXBufAddr_r2+RXBufSize_r)) begin
		   dmawad_next <= RXBufAddr_r1;
			RoundNumber_next <= RoundNumber + 32'h0000_0001;
		end else begin
		   dmawad_next <= dmawad_now1 + 64'h0000_0000_0000_0080;
			RoundNumber_next <= RoundNumber_next;
		end
	end else begin
	      dmawad_next <= dmawad_next;
			RoundNumber_next <= RoundNumber_next;
		 end
end

// update dmawad_now
always@(posedge clk)begin
   if(state == IDLE)begin
	   dmawad_now1 <= dmawad_next;
		dmawad_now2 <= dmawad_next;
		RoundNumber <= RoundNumber_next;
	end else begin
	   dmawad_now1 <= dmawad_now1;
		dmawad_now2 <= dmawad_now2;
		RoundNumber <= RoundNumber;
	end		
end

// dmawad output 
always@(posedge clk) begin
   if(rst_reg) begin
	   dmawad      <= 64'h0000_0000_0000_0000;
	end else if (state == TX_DESC_WRITE_BACK) begin
	      dmawad      <= DescAddr;
	end else if (state == RX_CLEAR) begin
	      if((dmawad_now2 + 64'h0000_0000_0000_0080) >= (RXBufAddr_r2+RXBufSize_r)) begin
		      dmawad <= RXBufAddr_r1;
		   end else begin
			   dmawad <= dmawad_now1 + 64'h0000_0000_0000_0080;
		   end
	end else if (state == RX_PACKET) begin    // calculation
	      dmawad      <= dmawad_now1 + 64'h0000_0000_0000_0010;
	end else if (state == RX_DESC) begin
	      dmawad      <= dmawad_now1;
	end else begin
	      dmawad      <= dmawad;
	end
end

// length output value from state machine
always@(posedge clk) begin
   if(rst_reg)
	   length_byte[12:0] <= 13'h0000;
	else if (state == TX_DESC_WRITE_BACK)
	   length_byte[12:0] <= TX_DESC_SIZE_BYTES[12:0];
	else if (state == RX_CLEAR)
	   length_byte[12:0] <= RX_DESC_SIZE_BYTES[12:0];
	else if (state == RX_PACKET)
	   length_byte[12:0] <= frame_size_bytes[12:0];
	else if (state == RX_DESC)
	   length_byte[12:0] <= RX_DESC_SIZE_BYTES[12:0];
	else
	   length_byte <= length_byte;
end

assign length[9:0] = length_byte[11:2];

endmodule
