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
// Company: 
// Engineer: 
// 
// Create Date:    18:15:25 09/05/2012 
// Design Name: 
// Module Name:    posted_pkt_scheduler_4radios 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module posted_pkt_scheduler_4radios(
    input clk,
	 input rst,
	 //interface to PCIe Endpoint Block Plus
	 input [2:0]    max_pay_size,
	 //interface from/to RX data fifo
	 input [63:0]   RX_FIFO_data,
	 output     	 RX_FIFO_RDEN,
	 input          RX_FIFO_pempty,
	 input [31:0]   RX_TS_FIFO_data,
	 output			 RX_TS_FIFO_RDEN,
	 input          RX_TS_FIFO_empty,
	 //interface from/to the 2nd RX data fifo
	 input [63:0]   RX_FIFO_2nd_data,
	 output      	 RX_FIFO_2nd_RDEN,
	 input          RX_FIFO_2nd_pempty,
	 input [31:0]   RX_TS_FIFO_2nd_data,
	 output			 RX_TS_FIFO_2nd_RDEN,
	 input          RX_TS_FIFO_2nd_empty,
	 //interface from/to the 3rd RX data fifo
	 input [63:0]   RX_FIFO_3rd_data,
	 output      	 RX_FIFO_3rd_RDEN,
	 input          RX_FIFO_3rd_pempty,
	 input [31:0]   RX_TS_FIFO_3rd_data,
	 output			 RX_TS_FIFO_3rd_RDEN,
	 input          RX_TS_FIFO_3rd_empty,
	 //interface from/to the 4th RX data fifo
	 input [63:0]   RX_FIFO_4th_data,
	 output      	 RX_FIFO_4th_RDEN,
	 input          RX_FIFO_4th_pempty,
	 input [31:0]   RX_TS_FIFO_4th_data,
	 output			 RX_TS_FIFO_4th_RDEN,
	 input          RX_TS_FIFO_4th_empty,
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
	 input          RXEnable,
	 input [63:0]   RXBuf_1stAddr,
	 input [31:0]   RXBuf_1stSize,
	 /// RX control signals for the 2nd RX buffer
	 input          RXEnable_2nd,
	 input [63:0]   RXBuf_2ndAddr,
	 input [31:0]   RXBuf_2ndSize,
	 /// RX control signals for the 3rd RX buffer
	 input          RXEnable_3rd,
	 input [63:0]   RXBuf_3rdAddr,
	 input [31:0]   RXBuf_3rdSize,
	 /// RX control signals for the 4th RX buffer
	 input          RXEnable_4th,
	 input [63:0]   RXBuf_4thAddr,
	 input [31:0]   RXBuf_4thSize,
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
	 input           posted_fifo_full
	 // debug interface
//	 output reg [31:0] Debug20RX1,
////	 output reg [4:0]  Debug22RX3,
//	 output reg [31:0] Debug24RX5,
//	 output reg [31:0] Debug26RX7,
//	 output reg [31:0] Debug27RX8,
//	 output reg [31:0] Debug28RX9,
//	 output reg [31:0] Debug29RX10
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
	 
	 // Signals for RoundRobin scheduling, two RX paths, pending
//	 reg			 Current_Path;			// 0: Current path is RX FIFO 1; 1: Current path is RX FIFO 2

	// Signals for RoundRobin scheduling, four RX paths
	 reg [1:0]	pathindex_inturn;		// 00: path 1; 01: path 2; 10: path 3; 11: path 4 
	 
	 reg			RX_FIFO_RDEN_cntl;	// read RX FIFO signal, we use one state machine for all the FIFOs, pathindex_inturn
												// is used to select one of the RX FIFOs
	 reg			RX_TS_FIFO_RDEN_cntl;	// read RX TS FIFO signal, we use one state machine for all the FIFOs, pathindex_inturn
													// is used to select one of the RX FIFOs
	 wire [31:0]	RX_TS_FIFO_data_cntl;
	 
	 reg [13:0]  cnt;            // counter for RX data
	 reg [63:0]  fifo_data_pipe; // pipeline data register between RX fifo and 
	                             // dma write data fifo, also swap the upper and lower
										  // DW

	 reg  [12:0] length_byte; // output posted packet length

//////////////		RX Path 1	//////////////////////
    reg         buf_inuse;      // whether this RX Buf is in use
	 reg [31:0]  RoundNumber;	  // indicates how many rounds we have wroten in RX buffer
	 reg [31:0]  RoundNumber_next;
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]  dmawad_now1_1st, dmawad_now2_1st;
    reg [63:0]  dmawad_next_1st;    // next RX data destination address
//	 reg [63:0]  dmawad_desc;    // current rx descriptor address
	 // Pipeline registers
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]   RXBuf_1stAddr_r1, RXBuf_1stAddr_r2;
	 reg [31:0]   RXBuf_1stSize_r;

//////////////		RX Path 2	//////////////////////
    reg         buf_inuse_2nd;      // whether this RX Buf is in use
	 reg [31:0]  RoundNumber_2nd;	  // indicates how many rounds we have wroten in RX buffer
	 reg [31:0]  RoundNumber_next_2nd;
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]  dmawad_now1_2nd, dmawad_now2_2nd;
    reg [63:0]  dmawad_next_2nd;    // next RX data destination address
//	 reg [63:0]  dmawad_desc_2nd;    // current rx descriptor address
	 // Pipeline registers
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]   RXBuf_2ndAddr_r1, RXBuf_2ndAddr_r2;
	 reg [31:0]   RXBuf_2ndSize_r;

//////////////		RX Path 3	//////////////////////
    reg         buf_inuse_3rd;      // whether this RX Buf is in use
	 reg [31:0]  RoundNumber_3rd;	  // indicates how many rounds we have wroten in RX buffer
	 reg [31:0]  RoundNumber_next_3rd;
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]  dmawad_now1_3rd, dmawad_now2_3rd;
    reg [63:0]  dmawad_next_3rd;    // next RX data destination address
//	 reg [63:0]  dmawad_desc_3rd;    // current rx descriptor address
	 // Pipeline registers
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]   RXBuf_3rdAddr_r1, RXBuf_3rdAddr_r2;
	 reg [31:0]   RXBuf_3rdSize_r;

//////////////		RX Path 4	//////////////////////
    reg         buf_inuse_4th;      // whether this RX Buf is in use
	 reg [31:0]  RoundNumber_4th;	  // indicates how many rounds we have wroten in RX buffer
	 reg [31:0]  RoundNumber_next_4th;
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]  dmawad_now1_4th, dmawad_now2_4th;
    reg [63:0]  dmawad_next_4th;    // next RX data destination address
//	 reg [63:0]  dmawad_desc_4th;    // current rx descriptor address
	 // Pipeline registers
	 (*EQUIVALENT_REGISTER_REMOVAL="NO"*) reg [63:0]   RXBuf_4thAddr_r1, RXBuf_4thAddr_r2;
	 reg [31:0]   RXBuf_4thSize_r;

	 
	 reg         rst_reg;	 
	 always@(posedge clk) rst_reg <= rst;

//    // Debug register
//	 always@(posedge clk)begin
//	    Debug20RX1[3:0]   <= state[3:0];
//		 Debug20RX1[7:4]   <= {3'b000,buf_inuse};		 
//		 Debug20RX1[23:8]  <= {3'b000,length_byte[12:0]};
//		 Debug20RX1[27:24] <= {1'b0,max_pay_size};
//		 Debug20RX1[31:28] <= {3'b000,posted_fifo_full};		 
//	 end
//	 
////	 always@(posedge clk)begin
////	    Debug22RX3[3:0]   <= {3'b000,dma_write_data_fifo_full};
////		 Debug22RX3[4]     <= RX_FIFO_pempty;
////	 end
//
//    always@(posedge clk)begin
//	    if (rst)
//		    Debug24RX5[31:0] <= 32'h0000_0000;
//		 else begin
//			if (RX_FIFO_RDEN)
//				 Debug24RX5 <= Debug24RX5 + 8'h0000_0001;
//			else
//				 Debug24RX5 <= Debug24RX5;
//		 end
//	 end
//
//    always@(posedge clk)begin
//	    if(rst)
//		    Debug26RX7 <= 32'h0000_0000;
//		 else if (dma_write_data_fifo_wren)
//		    Debug26RX7 <= Debug26RX7 + 32'h0000_0001;
//		 else
//		    Debug26RX7 <= Debug26RX7;
//	 end
//	 
//	 always@(posedge clk)begin
//	    if (rst)
//		    Debug27RX8 <= 32'h0000_0000;
//		 else if (state == RX_PACKET)
//		    Debug27RX8 <= Debug27RX8 + 32'h0000_0001;
//		 else
//		    Debug27RX8 <= Debug27RX8;
//	 end
//	 
//	 always@(posedge clk)begin
//	    Debug28RX9  <= dmawad[31:0];
//		 Debug29RX10 <= dmawad[63:32];
//	 end

	 // pipeline registers for RX path 1
    always@(posedge clk)begin
	    RXBuf_1stAddr_r1 <= RXBuf_1stAddr;
		 RXBuf_1stAddr_r2 <= RXBuf_1stAddr;
		 RXBuf_1stSize_r <= RXBuf_1stSize;
	 end

	 // pipeline registers for RX path 2
    always@(posedge clk)begin
	    RXBuf_2ndAddr_r1 <= RXBuf_2ndAddr;
		 RXBuf_2ndAddr_r2 <= RXBuf_2ndAddr;
		 RXBuf_2ndSize_r <= RXBuf_2ndSize;
	 end
	 
	 // pipeline registers for RX path 3
    always@(posedge clk)begin
	    RXBuf_3rdAddr_r1 <= RXBuf_3rdAddr;
		 RXBuf_3rdAddr_r2 <= RXBuf_3rdAddr;
		 RXBuf_3rdSize_r <= RXBuf_3rdSize;
	 end
	 
	 // pipeline registers for RX path 4
    always@(posedge clk)begin
	    RXBuf_4thAddr_r1 <= RXBuf_4thAddr;
		 RXBuf_4thAddr_r2 <= RXBuf_4thAddr;
		 RXBuf_4thSize_r <= RXBuf_4thSize;
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
//	always@(posedge clk) fifo_data_pipe[63:0] <= (~Current_Path) ? 
//																 {RX_FIFO_data[15:0],RX_FIFO_data[31:16],
//																 RX_FIFO_data[47:32],RX_FIFO_data[63:48]} : 
//																 {RX_FIFO_2nd_data[15:0],RX_FIFO_2nd_data[31:16],
//																 RX_FIFO_2nd_data[47:32],RX_FIFO_2nd_data[63:48]};
	always@(posedge clk) fifo_data_pipe[63:0] <= (pathindex_inturn[1] == 1'b0) ?
																	(	(pathindex_inturn[0] == 1'b0) ?
																			{RX_FIFO_data[15:0],RX_FIFO_data[31:16],
																			 RX_FIFO_data[47:32],RX_FIFO_data[63:48]} : 
																			{RX_FIFO_2nd_data[15:0],RX_FIFO_2nd_data[31:16],
																			 RX_FIFO_2nd_data[47:32],RX_FIFO_2nd_data[63:48]}
																	)
																:	(	(pathindex_inturn[0] == 1'b0) ?	 
																			{RX_FIFO_3rd_data[15:0],RX_FIFO_3rd_data[31:16],
																			 RX_FIFO_3rd_data[47:32],RX_FIFO_3rd_data[63:48]} : 
																			{RX_FIFO_4th_data[15:0],RX_FIFO_4th_data[31:16],
																			 RX_FIFO_4th_data[47:32],RX_FIFO_4th_data[63:48]}
																	);

//	assign RX_FIFO_RDEN = Current_Path & RX_FIFO_RDEN_cntl;
//	assign RX_FIFO_2nd_RDEN = (~Current_Path) & RX_FIFO_RDEN_cntl;
	
	assign RX_FIFO_RDEN 		= (pathindex_inturn == 2'b00) & RX_FIFO_RDEN_cntl;
	assign RX_FIFO_2nd_RDEN	= (pathindex_inturn == 2'b01) & RX_FIFO_RDEN_cntl;
	assign RX_FIFO_3rd_RDEN	= (pathindex_inturn == 2'b10) & RX_FIFO_RDEN_cntl;
	assign RX_FIFO_4th_RDEN	= (pathindex_inturn == 2'b11) & RX_FIFO_RDEN_cntl;
	
	assign RX_TS_FIFO_RDEN 		= (pathindex_inturn == 2'b00) & RX_TS_FIFO_RDEN_cntl;
	assign RX_TS_FIFO_2nd_RDEN	= (pathindex_inturn == 2'b01) & RX_TS_FIFO_RDEN_cntl;
	assign RX_TS_FIFO_3rd_RDEN	= (pathindex_inturn == 2'b10) & RX_TS_FIFO_RDEN_cntl;
	assign RX_TS_FIFO_4th_RDEN	= (pathindex_inturn == 2'b11) & RX_TS_FIFO_RDEN_cntl;
	
	assign RX_TS_FIFO_data_cntl[31:0] = (pathindex_inturn[1] == 1'b0) ?
												(	(pathindex_inturn[0] == 1'b0) ?	RX_TS_FIFO_data[31:0] : RX_TS_FIFO_2nd_data[31:0]	)
											:	(	(pathindex_inturn[0] == 1'b0) ?	RX_TS_FIFO_3rd_data[31:0] : RX_TS_FIFO_4th_data[31:0]	);
	
	//main state machine
	always@ (posedge clk) begin
		if (rst_reg) begin
			 state <= IDLE;
			 RX_FIFO_RDEN_cntl <= 1'b0;
			 RX_TS_FIFO_RDEN_cntl	<= 1'b0;
			 dma_write_data_fifo_wren <= 1'b0;
			 dma_write_data_fifo_data <= 64'h0000_0000_0000_0000;
			 go <= 1'b0;
			 buf_inuse <= 1'b0;
			 buf_inuse_2nd <= 1'b0;
			 buf_inuse_3rd <= 1'b0;
			 buf_inuse_4th <= 1'b0;
			 pathindex_inturn	<= 2'b00;
			 cnt <= 13'h0000;
		end else begin
			 case (state)
				IDLE : begin
						cnt <= 13'h0000;
						go <= 1'b0;
						RX_FIFO_RDEN_cntl <= 1'b0;
						dma_write_data_fifo_wren <= 1'b0;
						dma_write_data_fifo_data <= 64'h0000_0000_0000_0000;
						// check if need to generate a posted packet
						if(~posted_fifo_full & ~dma_write_data_fifo_full) begin
							if(TX_desc_write_back_req)
							  state <= TX_DESC_WRITE_BACK;
							else begin
								casex({	pathindex_inturn[1:0], 
											((~RX_FIFO_pempty)&(~RX_TS_FIFO_empty)), 
											((~RX_FIFO_2nd_pempty)&(~RX_TS_FIFO_2nd_empty)), 
											((~RX_FIFO_3rd_pempty)&(~RX_TS_FIFO_3rd_empty)), 
											((~RX_FIFO_4th_pempty)&(~RX_TS_FIFO_4th_empty))	})
								
									6'b00_1xxx, 6'b01_1000, 6'b10_1x00, 6'b11_1xx0: begin		// path1's turn
										pathindex_inturn	<= 2'b00;
										buf_inuse			<= 1'b1;
										cnt 					<= frame_size_bytes;
										RX_TS_FIFO_RDEN_cntl	<= 1'b1;
										state 				<= RX_CLEAR;
									end
									
									6'b00_01xx, 6'b01_x1xx, 6'b10_0100, 6'b11_01x0: begin		// path2's turn
										pathindex_inturn	<= 2'b01;
										buf_inuse_2nd		<= 1'b1;
										cnt 					<= frame_size_bytes;
										RX_TS_FIFO_RDEN_cntl	<= 1'b1;
										state 				<= RX_CLEAR;
									end
									
									6'b00_001x, 6'b01_x01x, 6'b10_xx1x, 6'b11_0010: begin		// path3's turn
										pathindex_inturn	<= 2'b10;
										buf_inuse_3rd		<= 1'b1;
										cnt 					<= frame_size_bytes;
										RX_TS_FIFO_RDEN_cntl	<= 1'b1;
										state 				<= RX_CLEAR;
									end
									
									6'b00_0001, 6'b01_x001, 6'b10_xx01, 6'b11_xxx1: begin		// path4's turn
										pathindex_inturn	<= 2'b11;
										buf_inuse_4th		<= 1'b1;
										cnt 					<= frame_size_bytes;
										RX_TS_FIFO_RDEN_cntl	<= 1'b1;
										state 				<= RX_CLEAR;
									end
									
									default: begin			// IDLE state
										RX_TS_FIFO_RDEN_cntl	<= 1'b1;
										state <= IDLE;
									end
									
								endcase
								
//								case({Path_Priority, ~RX_FIFO_pempty, ~RX_FIFO_2nd_pempty})
//									3'b010, 3'b011, 3'b110: begin
//										Current_Path <= 1'b0;
//										buf_inuse <= 1'b0;
//										cnt <= frame_size_bytes;
//										state <= RX_CLEAR;
//									end
//									3'b101, 3'b111, 3'b001: begin
//										Current_Path <= 1'b1;
//										buf_inuse_2nd <= 1'b0;
//										cnt <= frame_size_bytes;
//										state <= RX_CLEAR;
//									end
//									3'b000, 3'b100: begin
//										state <= IDLE;
//									end
//								endcase
							end
						end else
							 state <= IDLE;
					end
				// start TX desc write back flow
				TX_DESC_WRITE_BACK : begin     // write TX descriptor into dma write data fifo
						go <= 1'b0;
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
						// TimeStamp(4B), TXStatus(2B), CRC(2B)
						dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
						state <= TX_DESC_WRITE_BACK2;					
					end
				TX_DESC_WRITE_BACK2 : begin
						go <= 1'b0;
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
						// SourceAddr(8B)
						dma_write_data_fifo_data[63:0] <= SourceAddr;
						state <= TX_DESC_WRITE_BACK3;
					end
				TX_DESC_WRITE_BACK3 : begin
						go <= 1'b0;
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
						// NextDesc(4B), DestAddr(4B)
						dma_write_data_fifo_data[63:0] <= {32'h0000_0000,DestAddr};
						state <= TX_DESC_WRITE_BACK4;
					end
				TX_DESC_WRITE_BACK4 : begin
						go <= 1'b1;
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
						// Reserved(4B), FrameControl, FrameSize(3B)
						dma_write_data_fifo_data[63:0] <= {32'h0000_0000,FrameControl,FrameSize};
						state <= WAIT_FOR_ACK;
					end
				WAIT_FOR_ACK : begin
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
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
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						go <= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
						// clear RX desc
						dma_write_data_fifo_data[63:0] <= {64'h0000_0000_0000_0000};
						state <= RX_CLEAR2;
					end
				RX_CLEAR2 : begin
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						go <= 1'b1;
						dma_write_data_fifo_wren <= 1'b1;
						// write round number in RX desc. In case it's the head of RX buffer, 
						// RoundNumber could be old
//						dma_write_data_fifo_data[63:0] <= {32'h0000_0000,RoundNumber};
						dma_write_data_fifo_data[63:0] <= {RX_TS_FIFO_data_cntl[31:0]+32'h0000_001C,RoundNumber[31:0]};
						state <= RX_CLEAR_WAIT_FOR_ACK;	
					end
				RX_CLEAR_WAIT_FOR_ACK : begin
	//			      RX_FIFO_RDEN <= 1'b0;		// this is a modification here, previously (in SISO), we read out the first data in IDLE state
						dma_write_data_fifo_wren <= 1'b0;
						dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						if(ack) begin
							go <= 1'b0;
							if (~posted_fifo_full & ~dma_write_data_fifo_full) begin
								RX_FIFO_RDEN_cntl <= 1'b1;
								state <= RX_PACKET;
							end else begin
								RX_FIFO_RDEN_cntl <= 1'b0;
								state <= RX_CLEAR_WAIT;
							end
						end else begin
							go <= 1'b1;
							RX_FIFO_RDEN_cntl <= 1'b0;
							state <= RX_CLEAR_WAIT_FOR_ACK;
						end
					end
				RX_CLEAR_WAIT : begin
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						if (~posted_fifo_full & ~dma_write_data_fifo_full) begin
							RX_FIFO_RDEN_cntl <= 1'b1;
							state <= RX_PACKET;
						end else begin
							RX_FIFO_RDEN_cntl <= 1'b0;
							state <= RX_CLEAR_WAIT;
						end
				end
				
				// start of RX packet generation flow
				RX_PACKET : begin
						go <= 1'b0;
						cnt <= cnt - 13'h0008;
						RX_FIFO_RDEN_cntl <= 1'b1;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b0;
						dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
						state <= RX_PACKET2;
					end
				RX_PACKET2 : begin
						go <= 1'b0;
						cnt <= cnt - 13'h0008;
						RX_FIFO_RDEN_cntl <= 1'b1;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b0;
						dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
	////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
						state <= RX_PACKET3;
					end
				RX_PACKET3 : begin
						go <= 1'b0;
						RX_FIFO_RDEN_cntl <= 1'b1;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
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
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
	////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
						dma_write_data_fifo_data[63:0] <= fifo_data_pipe[63:0];
						state <= RX_PACKET5;
					end
				RX_PACKET5 : begin
						go <= 1'b0;
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
	////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
						dma_write_data_fifo_data[63:0] <= fifo_data_pipe[63:0];
						state <= RX_PACKET6;
					end
				RX_PACKET6 : begin
						go <= 1'b1;
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
	////					fifo_data_pipe[63:0] <= {RX_FIFO_data[31:0],RX_FIFO_data[63:32]};
						dma_write_data_fifo_data[63:0] <= fifo_data_pipe[63:0];
						state <= RX_PACKET_WAIT_FOR_ACK;
					end
				RX_PACKET_WAIT_FOR_ACK : begin
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
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
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						if (~posted_fifo_full & ~dma_write_data_fifo_full)
							state <= RX_DESC;
						else
							state <= RX_DESC_WAIT;
				end
					
				// start of RX desc flow
				RX_DESC : begin
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						go <= 1'b0;
						dma_write_data_fifo_wren <= 1'b1;
						// data will be swapped later
//						dma_write_data_fifo_data[63:0] <= {56'h00_0000_0000_0000,8'h01};
						dma_write_data_fifo_data[63:0] <= 64'hFFFF_FFFF_FFFF_FFFF;
						state <= RX_DESC2;
					end
				RX_DESC2 : begin
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						go <= 1'b1;
						dma_write_data_fifo_wren <= 1'b1;
	//					dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
						// Write Round number in RX desc. It's used as debug info
//						dma_write_data_fifo_data[63:0] <= {32'h0000_0000,RoundNumber};
						dma_write_data_fifo_data[63:0] <= {RX_TS_FIFO_data_cntl[31:0],RoundNumber[31:0]};
						state <= WAIT_FOR_ACK;					
					end
				default: begin
						go <= 1'b0;
						RX_FIFO_RDEN_cntl <= 1'b0;
						RX_TS_FIFO_RDEN_cntl	<= 1'b0;
						dma_write_data_fifo_wren <= 1'b0;
						dma_write_data_fifo_data[63:0] <= 64'h0000_0000_0000_0000;
						state <= IDLE;
					end
			endcase
		end
	end

	// update dmawad_next_1st and RoundNumber_next for RX path 1
	always@(posedge clk) begin
		if(~buf_inuse | rst_reg) begin
			dmawad_next_1st <= RXBuf_1stAddr_r1;
			RoundNumber_next <= 32'h0000_0000;
		end else if ((state == RX_PACKET3) && (pathindex_inturn[1:0] == 2'b00)) begin
			// if end of RX buf is arrived, dmawad_next_1st will return to start address at next cycle
	//	   if((dmawad_now2_1st + 64'h0000_0000_0000_0080) == (RXBuf_1stAddr_r2+RXBuf_1stSize_r))
			if((dmawad_now2_1st + 64'h0000_0000_0000_0080) >= (RXBuf_1stAddr_r2+RXBuf_1stSize_r)) begin
				dmawad_next_1st <= RXBuf_1stAddr_r1;
				RoundNumber_next <= RoundNumber + 32'h0000_0001;
			end else begin
				dmawad_next_1st <= dmawad_now1_1st + 64'h0000_0000_0000_0080;
				RoundNumber_next <= RoundNumber_next;
			end
		end else begin
				dmawad_next_1st <= dmawad_next_1st;
				RoundNumber_next <= RoundNumber_next;
			 end
	end

	// update dmawad_next_2nd and RoundNumber_next_2nd for RX path 2
	always@(posedge clk) begin
		if(~buf_inuse_2nd | rst_reg) begin
			dmawad_next_2nd <= RXBuf_2ndAddr_r1;
			RoundNumber_next_2nd <= 32'h0000_0000;
		end else if ((state == RX_PACKET3) && (pathindex_inturn[1:0] == 2'b01)) begin
			// if end of RX buf is arrived, dmawad_next will return to start address at next cycle
	//	   if((dmawad_now2_1st + 64'h0000_0000_0000_0080) == (RXBuf_1stAddr_r2+RXBuf_1stSize_r))
			if((dmawad_now2_2nd + 64'h0000_0000_0000_0080) >= (RXBuf_2ndAddr_r2+RXBuf_2ndSize_r)) begin
				dmawad_next_2nd <= RXBuf_2ndAddr_r1;
				RoundNumber_next_2nd <= RoundNumber_2nd + 32'h0000_0001;
			end else begin
				dmawad_next_2nd <= dmawad_now1_2nd + 64'h0000_0000_0000_0080;
				RoundNumber_next_2nd <= RoundNumber_next_2nd;
			end
		end else begin
				dmawad_next_2nd <= dmawad_next_2nd;
				RoundNumber_next_2nd <= RoundNumber_next_2nd;
			 end
	end

	// update dmawad_next_3rd and RoundNumber_next_3rd for RX path 3
	always@(posedge clk) begin
		if(~buf_inuse_3rd | rst_reg) begin
			dmawad_next_3rd <= RXBuf_3rdAddr_r1;
			RoundNumber_next_3rd <= 32'h0000_0000;
		end else if ((state == RX_PACKET3) && (pathindex_inturn[1:0] == 2'b10)) begin
			// if end of RX buf is arrived, dmawad_next will return to start address at next cycle
	//	   if((dmawad_now2 + 64'h0000_0000_0000_0080) == (RXBufAddr_r2+RXBufSize_r))
			if((dmawad_now2_3rd + 64'h0000_0000_0000_0080) >= (RXBuf_3rdAddr_r2+RXBuf_3rdSize_r)) begin
				dmawad_next_3rd <= RXBuf_3rdAddr_r1;
				RoundNumber_next_3rd <= RoundNumber_3rd + 32'h0000_0001;
			end else begin
				dmawad_next_3rd <= dmawad_now1_3rd + 64'h0000_0000_0000_0080;
				RoundNumber_next_3rd <= RoundNumber_next_3rd;
			end
		end else begin
				dmawad_next_3rd <= dmawad_next_3rd;
				RoundNumber_next_3rd <= RoundNumber_next_3rd;
			 end
	end

	// update dmawad_next_2nd and RoundNumber_next_2nd for RX path 4
	always@(posedge clk) begin
		if(~buf_inuse_4th | rst_reg) begin
			dmawad_next_4th <= RXBuf_4thAddr_r1;
			RoundNumber_next_4th <= 32'h0000_0000;
		end else if ((state == RX_PACKET3) && (pathindex_inturn[1:0] == 2'b11)) begin
			// if end of RX buf is arrived, dmawad_next will return to start address at next cycle
	//	   if((dmawad_now2 + 64'h0000_0000_0000_0080) == (RXBufAddr_r2+RXBufSize_r))
			if((dmawad_now2_4th + 64'h0000_0000_0000_0080) >= (RXBuf_4thAddr_r2+RXBuf_4thSize_r)) begin
				dmawad_next_4th <= RXBuf_4thAddr_r1;
				RoundNumber_next_4th <= RoundNumber_4th + 32'h0000_0001;
			end else begin
				dmawad_next_4th <= dmawad_now1_4th + 64'h0000_0000_0000_0080;
				RoundNumber_next_4th <= RoundNumber_next_4th;
			end
		end else begin
				dmawad_next_4th <= dmawad_next_4th;
				RoundNumber_next_4th <= RoundNumber_next_4th;
			 end
	end

	// update dmawad_now for RX path 1
	always@(posedge clk)begin
		if(state == IDLE)begin
			dmawad_now1_1st <= dmawad_next_1st;
			dmawad_now2_1st <= dmawad_next_1st;
			RoundNumber <= RoundNumber_next;
		end else begin
			dmawad_now1_1st <= dmawad_now1_1st;
			dmawad_now2_1st <= dmawad_now2_1st;
			RoundNumber <= RoundNumber;
		end		
	end

	// update dmawad_now_2nd for RX path 2
	always@(posedge clk)begin
		if(state == IDLE)begin
			dmawad_now1_2nd <= dmawad_next_2nd;
			dmawad_now2_2nd <= dmawad_next_2nd;
			RoundNumber_2nd <= RoundNumber_next_2nd;
		end else begin
			dmawad_now1_2nd <= dmawad_now1_2nd;
			dmawad_now2_2nd <= dmawad_now2_2nd;
			RoundNumber_2nd <= RoundNumber_2nd;
		end		
	end

	// update dmawad_now_3rd for RX path 3
	always@(posedge clk)begin
		if(state == IDLE)begin
			dmawad_now1_3rd <= dmawad_next_3rd;
			dmawad_now2_3rd <= dmawad_next_3rd;
			RoundNumber_3rd <= RoundNumber_next_3rd;
		end else begin
			dmawad_now1_3rd <= dmawad_now1_3rd;
			dmawad_now2_3rd <= dmawad_now2_3rd;
			RoundNumber_3rd <= RoundNumber_3rd;
		end		
	end

	// update dmawad_now_4th for RX path 4
	always@(posedge clk)begin
		if(state == IDLE)begin
			dmawad_now1_4th <= dmawad_next_4th;
			dmawad_now2_4th <= dmawad_next_4th;
			RoundNumber_4th <= RoundNumber_next_4th;
		end else begin
			dmawad_now1_4th <= dmawad_now1_4th;
			dmawad_now2_4th <= dmawad_now2_4th;
			RoundNumber_4th <= RoundNumber_4th;
		end		
	end

	// dmawad output 
	always@(posedge clk) begin
		if(rst_reg) begin
			dmawad      <= 64'h0000_0000_0000_0000;
		end else if (state == TX_DESC_WRITE_BACK) begin
				dmawad      <= DescAddr;
		end else if (state == RX_CLEAR) begin
				if (pathindex_inturn[1] == 1'b0) begin
					if (pathindex_inturn[0] == 1'b0) begin	// pathindex_inturn == 2'b00
						if((dmawad_now2_1st + 64'h0000_0000_0000_0080) >= (RXBuf_1stAddr_r2+RXBuf_1stSize_r)) begin
							dmawad <= RXBuf_1stAddr_r1;
						end else begin
							dmawad <= dmawad_now1_1st + 64'h0000_0000_0000_0080;
						end
					end else begin									// pathindex_inturn == 2'b01
						if((dmawad_now2_2nd + 64'h0000_0000_0000_0080) >= (RXBuf_2ndAddr_r2+RXBuf_2ndSize_r)) begin
							dmawad <= RXBuf_2ndAddr_r1;
						end else begin
							dmawad <= dmawad_now1_2nd + 64'h0000_0000_0000_0080;
						end
					end
				end else begin										
					if (pathindex_inturn[0] == 1'b0) begin	// pathindex_inturn == 2'b10
						if((dmawad_now2_3rd + 64'h0000_0000_0000_0080) >= (RXBuf_3rdAddr_r2+RXBuf_3rdSize_r)) begin
							dmawad <= RXBuf_3rdAddr_r1;
						end else begin
							dmawad <= dmawad_now1_3rd + 64'h0000_0000_0000_0080;
						end
					end else begin									// pathindex_inturn == 2'b11
						if((dmawad_now2_4th + 64'h0000_0000_0000_0080) >= (RXBuf_4thAddr_r2+RXBuf_4thSize_r)) begin
							dmawad <= RXBuf_4thAddr_r1;
						end else begin
							dmawad <= dmawad_now1_4th + 64'h0000_0000_0000_0080;
						end
					end
				end
		end else if (state == RX_PACKET) begin    // calculation
				dmawad      <= (pathindex_inturn[1] == 1'b0) ? 
										(	(pathindex_inturn[0] == 1'b0) ?
											dmawad_now1_1st + 64'h0000_0000_0000_0010 : dmawad_now1_2nd + 64'h0000_0000_0000_0010
										)
									:	(	(pathindex_inturn[0] == 1'b0) ?
											dmawad_now1_3rd + 64'h0000_0000_0000_0010 : dmawad_now1_4th + 64'h0000_0000_0000_0010
										);
		end else if (state == RX_DESC) begin
				dmawad      <= (pathindex_inturn[1] == 1'b0) ? 
										(	(pathindex_inturn[0] == 1'b0) ?
											dmawad_now1_1st : dmawad_now1_2nd
										)
									:	(	(pathindex_inturn[0] == 1'b0) ?
											dmawad_now1_3rd : dmawad_now1_4th
										);
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
