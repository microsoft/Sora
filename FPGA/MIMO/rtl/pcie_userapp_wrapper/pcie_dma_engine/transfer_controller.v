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
`include "Sora_config.v"

//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    19:46:08 09/03/2012 
// Design Name: 
// Module Name:    transfer_controller 
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
module transfer_controller(
	 input				clk,
	 input				rst,
	 // triggers
`ifdef TF_RECOVERY
	 input				transferstart,
`endif
	 input				transferstart_one,
	 output				set_transfer_done_bit,
	 input [63:0]		TransferSrcAddr,
	  ///Jiansong: interface from RX engine, TX desc received
	 input                new_des_one,
	 input        [31:0]  SourceAddr_L,
	 input        [31:0]  SourceAddr_H,
	 input        [31:0]  DestAddr,
	 input        [23:0]  FrameSize,
	 input        [7:0]   FrameControl,
    ///Jiansong: interface to RX engine, indicate the system is in dma read for TX desc
	 ///          when this signal is asserted, received cpld will not be count in 
	 ///          length subtraction
    output reg        Wait_for_TX_desc,	
	 /// Jiansong: interface to/from posted_packet_generator
	 /// TX desc write back
	 output reg         TX_desc_write_back_req,
	 input              TX_desc_write_back_ack,
	 output reg [63:0]  SourceAddr_r,
    output reg [31:0]  DestAddr_r,
    output reg [23:0]  FrameSize_r,
    output reg [7:0]   FrameControl_r,
	 output reg [63:0]  DescAddr_r,
    /// Jiansong: interface to non_posted packet generator
	 output reg rd_TX_des_start,
    output [63:0] TX_des_addr,
    //outputs to Internal DMA CTRL block
    output reg [31:0] reg_data_in_o,
    output reg [6:0] reg_wr_addr_o,
    output reg reg_wren_o,

    //Input DMA done signals from TX and RX Engines
    input rd_dma_done_i,

    //Output DMA done signals to Internal DMA CTRL block
    //these are copies of the *_dma_done_i inputs
    output reg rd_dma_done_o,
    //the *_last signals are used by the tx_trn_sm block and rx_trn_data_fsm
    //block so they can generate the most accurate *_done signals possible;
    //this is needed for accurate performance measurements
    output read_last,  //active high during the very last DMA read of a series

    //input from dma_ddr2_if
    input pause_read_requests

    );

    //state machine state definitions for state_sm0
    localparam IDLE_SM0 = 4'h0;
    localparam READ_1 = 4'h7;
    localparam READ_2 = 4'h8;
    localparam READ_3 = 4'h9;
    localparam READ_4 = 4'ha;
    localparam READ_5 = 4'hb;
    localparam READ_6 = 4'hc;

    //state machine state definitions for state_sm3
    localparam IDLE_SM3 = 4'h0;
    localparam CALC_NEXT_READ = 4'h1;
    localparam READ_CALC_4KB = 4'h2;
    localparam READ_CALC_2KB = 4'h3;
    localparam READ_CALC_1KB = 4'h4;
    localparam READ_CALC_512B = 4'h5;
    localparam READ_CALC_256B = 4'h6;
    localparam READ_CALC_128B = 4'h7;
    localparam WAIT_READ_CALC = 4'h8;

    //state machine state definitions for state_sm4
    localparam IDLE_SM4 = 3'b000;
    localparam START_RD = 3'b001;
    localparam WAIT_FOR_RDDONE = 3'b010;
	/// Jiansong: pipeline registers
	reg         new_des_one_r;
	//reg [63:0]  SourceAddr_r;
	//reg [31:0]  DestAddr_r;
	//reg [23:0]  FrameSize_r;
	//reg [7:0]   FrameControl_r;

	//the dma*_now are used for providing DMA parameters to the 
	//DMA Internal CTRL block 
	reg [31:0] dmarad_now;
	reg [63:0] dmaras_now;
	reg [31:0] dmarxs_now;
	//the dma*_next registers make up the "next" set of DMA parameters in a series
	//of DMA transactions 
	reg [31:0] dmarad_next;
	reg [63:0] dmaras_next;
	reg [31:0] dmarxs_next;

	//State machine state variables
	reg [3:0] state_0; 
	reg [3:0] state_3;
	reg [2:0] state_4;

	reg [31:0] reg_data_in_o_r;
	reg [6:0]  reg_wr_addr_o_r;
	reg        reg_wren_o_r;

	reg update_dma_rnow;
	reg set_rd_done_bit;
	reg start_sm0_rdma_flow;
	reg read_calc_next;
	reg stay_2x_3;

	assign set_transfer_done_bit = set_rd_done_bit;

	/// Jiansong: generate Wait_for_TX_desc signal
	always@(posedge clk)begin
`ifdef TF_RECOVERY
		if(rst | (~transferstart))
`else
		if(rst)
`endif
			Wait_for_TX_desc <= 0;
		else if (transferstart_one)
			Wait_for_TX_desc <= 1;
		else if (new_des_one_r)
			Wait_for_TX_desc <= 0;
		else
			Wait_for_TX_desc <= Wait_for_TX_desc;
	end 

	/// Jiansong: output signal for tx desc
	always@(posedge clk)begin
`ifdef TF_RECOVERY
		 if(rst | (~transferstart))
`else
		 if(rst)
`endif
			  rd_TX_des_start <= 1'b0;
		 else if (transferstart_one)
			  rd_TX_des_start <= 1'b1;
		 else if (new_des_one_r)
			  rd_TX_des_start <= 1'b0;
		 else
			  rd_TX_des_start <= rd_TX_des_start;
	end

	assign TX_des_addr = TransferSrcAddr[63:0];

	//drive the *_done outputs to the Internal Control Block
	//with the *_done inputs from the RX and TX engines
	always@(*) rd_dma_done_o = rd_dma_done_i;


	/// Jiansong: dma read related logic should be kept, and dma write related
	///           logic is no longer used
	//////////////////////////////////////////////////////////////////////////////
	//NOTE:All of the code beneath this comment block are statemachines for 
	//driving 4KB, 2KB, 1KB, 512B, 256B, and 128B sub-transfers to the 
	//Internal Control Block
	//////////////////////////////////////////////////////////////////////////////

	/// Jiansong: remove dma write related states and keep dma read related states
	//State machine 0 block; Drives the Internal CTRL Block outputs:
	//reg_data_in_o[31:0], reg_wr_addr_o[6:0], and reg_wren_o.

	//This machine (state 0) waits for a signal from state machines 2 or 4
	//(either start_sm0_wdma_flow or start_sm0_rdma_flow signals)
	//When one of these signals is asserted, a series of register writes to
	//the Internal Control Block occurs which effectively causes a small
	//sub-transfer to execute (4KB, 2KB, 1KB, 512B, 256B, 128B.  
	//The dma*_now signals are passed to the Internal Control Block (via
	//reg_data_in_o, reg_wr_addr_o and reg_wren_o) as the sub-transfer parameters.  
	//This state machine also asserts the
	//write_calc_next and read_calc_next signals which are inputs to state 
	//machines 1 and 3 and which cause those state machines to calculate the 
	//DMA parameters for the next sub-transfer.

	always@(posedge clk) begin 
`ifdef TF_RECOVERY
		if(rst | (~transferstart))begin
`else
		if(rst)begin
`endif
			state_0 <= IDLE_SM0;
			reg_data_in_o_r[31:0] <= 32'h00000000;
			reg_wr_addr_o_r[6:0] <= 7'b0000000;
			reg_wren_o_r <= 1'b0;
			read_calc_next <= 1'b0;
		end else begin
			 case(state_0) 
			 IDLE_SM0: begin
				reg_data_in_o_r[31:0] <= 32'h00000000;
				reg_wr_addr_o_r[6:0] <= 7'b0000000;
				reg_wren_o_r <= 1'b0;
				read_calc_next <= 1'b0;
				if(start_sm0_rdma_flow & ~pause_read_requests)begin    /// Jiansong: enter READ_1 state
					state_0 <= READ_1;
				end
			 end
			 
			 //start of the series of writes to the dma read sub-transfer registers
			 //in the Internal Control Block
			 READ_1: begin //write the dmarad register
				 reg_data_in_o_r[31:0] <= dmarad_now[31:0];
				 reg_wr_addr_o_r[6:0] <= 7'b0010100;
				 reg_wren_o_r <= 1'b1;
				 read_calc_next <= 1'b1;
				 state_0 <= READ_2;
			 end
			 READ_2: begin //write the dmaras_l register
				 reg_data_in_o_r[31:0] <= dmaras_now[31:0];
				 reg_wr_addr_o_r[6:0] <= 7'b0001100;
				 reg_wren_o_r <= 1'b1;
				 read_calc_next <= 1'b0;
				 state_0 <= READ_3;
			 end
			 READ_3: begin //write the dmarad_u register
				 reg_data_in_o_r[31:0] <= dmaras_now[63:32];
				 reg_wr_addr_o_r[6:0] <= 7'b0010000;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= READ_4;
			 end
			 READ_4: begin //write the dmarxs register with the next
								//largest possible sub-transfer size
	////          if(dmarxs_now[20:12] != 0)
				 if(dmarxs_now[31:12] != 0)
					  reg_data_in_o_r[31:0] <= 32'h00001000; //4KB
				 else if (dmarxs_now[11]) 
					  reg_data_in_o_r[31:0] <= 32'h00000800; //2KB
				 else if (dmarxs_now[10])
					  reg_data_in_o_r[31:0] <= 32'h00000400; //1KB
				 else if (dmarxs_now[9])
					  reg_data_in_o_r[31:0] <= 32'h00000200; //512B
				 else if (dmarxs_now[8])
					  reg_data_in_o_r[31:0] <= 32'h00000100; //256B
				 else if (dmarxs_now[7])
					  reg_data_in_o_r[31:0] <= 32'h00000080; //128B
				 else 
					  reg_data_in_o_r[31:0] <= 32'h00000080;
				 reg_wr_addr_o_r[6:0] <= 7'b0011100;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= READ_5;
			 end
			 READ_5: begin //write a 1 to the done bit (dmacst[3] in Internal
								//Control Block) in order to clear the start
								//bit
				 reg_data_in_o_r[31:0] <= 32'h00000008;
				 reg_wr_addr_o_r[6:0] <= 7'b0101000;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= READ_6;
			 end
			 READ_6: begin //write a 1 to the start bit (dmacst[2] in Internal
								//Control Block) in order to execute the sub-transfer
				 reg_data_in_o_r[31:0] <= 32'h00000004;
				 reg_wr_addr_o_r[6:0] <= 7'b0101000;
				 reg_wren_o_r <= 1'b1;
				 state_0 <= IDLE_SM0;
			 end
			 default:begin
				 state_0 <= IDLE_SM0;
				 reg_data_in_o_r[31:0] <= 32'h00000000;
				 reg_wr_addr_o_r[6:0] <= 7'b0000000;
				 reg_wren_o_r <= 1'b0;
				 read_calc_next <= 1'b0;
			 end
		  endcase
		 end
	 end

	//pipeline register outputs for 250 MHz timing
	always@(posedge clk)begin
		reg_data_in_o[31:0] <= reg_data_in_o_r[31:0];
		reg_wr_addr_o[6:0] <= reg_wr_addr_o_r[6:0];
		reg_wren_o <= reg_wren_o_r;
	end

	//read_last is used by rx_engine so that it can signal
	//the correct *_done signal for the performance counters
	//Normally, rx_engine will use the *_early signal to
	//fire off a continuation transfer; on the last one
	//however, we would like the done signal to be accurate
	//for the performance counters

	////assign read_last = (dmarxs_now == 0) ? 1'b1: 1'b0;
	/// Jiansong: protection. otherwise, if transfer size has size smaller than 128B, it will never stop
	assign read_last = (dmarxs_now[31:7] == 0) ? 1'b1: 1'b0;

	//Calculate the next address, xfer size for writes   /// Jiansong: for reads
	//and transfer to dma*_now registers
	//Uses some multi-cycle paths:
	//      state_3  -> dma*_next
	//and   dma*_now -> dma*_next 
	//are both 2x multi-cycle paths
	//The signal "stay_2x_3" ensures that the state variable
	//state_3 is static for at least two clock
	//cycles when in the READ_CALC_*B states - the
	//dma*_now -> dma*_next paths must also be static during
	//these states
	always@(posedge clk)begin   
`ifdef TF_RECOVERY
		if(rst | (~transferstart))begin
`else
		if(rst)begin
`endif
			state_3 <= IDLE_SM3;
			stay_2x_3 <= 1'b0;
			update_dma_rnow <= 1'b0;
			dmarxs_next[31:0] <= 13'b0000000000000;
			dmaras_next[63:0] <= 64'h0000000000000000; 
			dmarad_next[31:0] <= 32'h00000000;
		end else begin
			case(state_3)
			IDLE_SM3:begin
				update_dma_rnow <= 1'b0;
				stay_2x_3 <= 1'b0;
				if(new_des_one_r)begin
					dmarad_next <= DestAddr_r;
					dmarxs_next <= FrameSize_r;
					dmaras_next <= SourceAddr_r;
				end
				//if state machine 0 asserts read_calc_next and there
				//is still sub-transfers to be completed then go 
				//ahead and precalculate the dma*_next parameters for the
				//next sub-transfer
	////         if(read_calc_next && (dmarxs_now[20:7] != 0))
				if(read_calc_next && (dmarxs_now[31:7] != 0))    /// Jiansong: 1M size limitation is relaxed
					state_3 <= CALC_NEXT_READ;
				else
					state_3 <= IDLE_SM3;
			end
			//This state is to figure out which will be the next
			//sub-transfer size based on sampling the dmarxs_now
			//signals - priority encoded for the largest possible transfer
			//to occur first.
			CALC_NEXT_READ:begin
				stay_2x_3 <= 1'b0; 
				update_dma_rnow <= 1'b0;         
	////         if(dmarxs_now[20:12] != 0)
				if(dmarxs_now[31:12] != 0)       /// Jianosng: 1M size limitation is relaxed to 4G
					state_3 <= READ_CALC_4KB;
				else if (dmarxs_now[11])
					state_3 <= READ_CALC_2KB;
				else if (dmarxs_now[10])
					state_3 <= READ_CALC_1KB;
				else if (dmarxs_now[9])
					state_3 <= READ_CALC_512B;
				else if (dmarxs_now[8])
					state_3 <= READ_CALC_256B;
				else if (dmarxs_now[7])
					state_3 <= READ_CALC_128B;
				else
					state_3 <= READ_CALC_128B;
			 end
			 //The READ_CALC_*B states are for updating the dma*_next registers
			 //with the correct terms
			 READ_CALC_4KB:begin
				//subtract 4KB from dmarxs and add 4KB to dmarad_next and dmaras_next
	////         dmarxs_next[20:12] <= dmarxs_now[20:12] - 1'b1;
				dmarxs_next[31:12] <= dmarxs_now[31:12] - 1'b1;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000001000; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00001000;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_4KB;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_2KB:begin
				//subtract 2KB from dmarxs and add 2KB to dmarad_next and dmaras_next
				dmarxs_next[11] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000800; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000800;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_2KB;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_1KB:begin
				//subtract 1KB from dmarxs and add 1KB to dmarad_next and dmaras_next
				dmarxs_next[10] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000400; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000400;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_1KB;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_512B:begin
				//subtract 512B from dmarxs and add 512B to dmarad_next and dmaras_next
				dmarxs_next[9] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000200; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000200;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_512B;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_256B:begin
				//subtract 256B from dmarxs and add 256B to dmarad_next and dmaras_next
				dmarxs_next[8] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000100; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000100;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_256B;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 READ_CALC_128B:begin
				//subtract 128B from dmarxs and add 128B to dmarad_next and dmaras_next
				dmarxs_next[7] <= 1'b0;
				dmaras_next[63:0] <= dmaras_now[63:0] + 64'h0000000000000080; 
				dmarad_next[31:0] <= dmarad_now[31:0] + 31'h00000080;
				//stay in this state for at least two clock cycles
				if(stay_2x_3 == 1'b0)begin
					state_3 <= READ_CALC_128B;
					stay_2x_3 <= 1'b1;
					update_dma_rnow <= 1'b0;
				end else begin
					state_3 <= WAIT_READ_CALC;
					stay_2x_3 <= 1'b0;
					update_dma_rnow <= 1'b1;
				end
			 end
			 WAIT_READ_CALC:begin
				stay_2x_3 <= 1'b0;
				update_dma_rnow <= 1'b0;      
				state_3 <= IDLE_SM3;
			 end
			 default:begin
				state_3 <= IDLE_SM3;
				stay_2x_3 <= 1'b0;
				update_dma_rnow <= 1'b0;
				dmarxs_next[31:0] <= 13'b0000000000000;
				dmaras_next[63:0] <= 64'h0000000000000000; 
				dmarad_next[31:0] <= 32'h00000000;
			 end
		  endcase
	  end  
	end 


	/// Jiansong:generate request for tx desc write back
	always@(posedge clk)begin
`ifdef TF_RECOVERY
		if(rst | (~transferstart)) begin
`else
		if(rst) begin
`endif
			TX_desc_write_back_req <= 1'b0;
		end else if (set_rd_done_bit) begin
						TX_desc_write_back_req <= 1'b1;
		end else if (TX_desc_write_back_ack) begin
						TX_desc_write_back_req <= 1'b0;
		end
	end

	/// Jiansong: pipeline registers for timing
	always@(posedge clk)begin
`ifdef TF_RECOVERY
		if(rst | (~transferstart))begin
`else
		if(rst)begin
`endif
			new_des_one_r <= 1'b0;
		end else begin
			new_des_one_r <= new_des_one;
		end
	end
	always@(posedge clk)begin
`ifdef TF_RECOVERY
		if(rst | ((~transferstart)))begin
`else
		if(rst)begin
`endif
			SourceAddr_r   <= 64'h0000_0000_0000_0000;
			DestAddr_r     <= 32'h0000_0000;
			FrameSize_r    <= 24'h00_00_00;
			FrameControl_r <= 8'h00;
			DescAddr_r     <= 64'h0000_0000_0000_0000;
		end else if(new_des_one)begin
							SourceAddr_r   <= {SourceAddr_H,SourceAddr_L};
							DestAddr_r     <= DestAddr;
							FrameSize_r    <= FrameSize;
							FrameControl_r <= FrameControl;
							DescAddr_r     <= TransferSrcAddr[63:0];
		end else if (set_rd_done_bit) begin
						SourceAddr_r   <= SourceAddr_r;
						DestAddr_r     <= DestAddr_r;
						FrameSize_r    <= FrameSize_r;
						FrameControl_r <= {FrameControl_r[7:1],1'b0}; // clear own bit
						DescAddr_r     <= DescAddr_r;
		end else begin
						SourceAddr_r   <= SourceAddr_r;
						DestAddr_r     <= DestAddr_r;
						FrameSize_r    <= FrameSize_r;
						FrameControl_r <= FrameControl_r;
						DescAddr_r     <= DescAddr_r;
		end
	end

	//register dmarad,dmarxs, and dmaras when rd_dma_start_one is high
	//rd_dma_start_one is only asserted for one clock cycle
	always@(posedge clk)begin
`ifdef TF_RECOVERY
		if(rst | (~transferstart))begin
`else
		if(rst)begin
`endif
			dmaras_now <= 64'h0000_0000_0000_0000;
			dmarxs_now <= 32'h0000_0000;
			dmarad_now <= 32'h0000_0000;
		end else if(new_des_one_r)begin    /// Jiansong: dma read (or data transfer) is started 
													///           after a new descriptor is received
			dmarad_now <= DestAddr_r;
			dmarxs_now <= FrameSize_r;
			dmaras_now <= SourceAddr_r; 		
		end else if(update_dma_rnow)begin
			dmarad_now <=  dmarad_next;
			dmarxs_now <=  dmarxs_next;
			dmaras_now <=  dmaras_next;   
		end
	end   

	//state machine for start_sm0_rdma_flow signal
	//This state machine controls state machine 0 by
	//driving the start_sm0_rdma_flow signal.  It monitors
	//the rd_dma_done signal from the rx_engine 
	//and starts a new dma subtransfer whenever rd_dma_done is signalled
	always@(posedge clk)begin
`ifdef TF_RECOVERY
		if(rst | (~transferstart))begin
`else
		if(rst)begin
`endif
			  start_sm0_rdma_flow <= 1'b0;
			  state_4 <= IDLE_SM4;
			  set_rd_done_bit <= 1'b0;
		end else begin  
		  case(state_4)
			  IDLE_SM4:begin
			  start_sm0_rdma_flow <= 1'b0;
			  set_rd_done_bit <= 1'b0;   
			  //wait for the start signal from the host
			  if(new_des_one_r)      /// Jiansong: dma read (or data transfer) is started 
										  ///           after a new descriptor is received
					  state_4 <= START_RD;
			  end
			  START_RD:begin
				  //start the state_0 state machine by
				  //asserting start_sm0_rdma_flow signal        
				  start_sm0_rdma_flow <= 1'b1;
				  //when state_0 finally starts go to the 
				  //WAIT_FOR_RDDONE state
				  if(state_0 == READ_1)
					  state_4 <= WAIT_FOR_RDDONE;
				  else
					  state_4 <= START_RD;
			  end
			  WAIT_FOR_RDDONE:begin
				  start_sm0_rdma_flow <= 1'b0;
				  //If the rx_engine signals rd_dma_done_i and we
				  //have completed the last subtransfer (i.e. dmarxs_now == 0) then 
				  //set the dmacst[3] bit via set_rd_done_bit and go to the IDLE state;
				  //Otherwise start the next subtransfer by going to the
				  //START_RD state
	////           if(rd_dma_done_i && (dmarxs_now == 0))begin    ///Jiansong: will not stop if has piece smaller than 128B
				  if(rd_dma_done_i && (dmarxs_now[31:7] == 0))begin
					  state_4 <= IDLE_SM4;
					  set_rd_done_bit <= 1'b1;
				  end else if(rd_dma_done_i)begin
					  state_4 <= START_RD;
					  set_rd_done_bit <= 1'b0;
				  end else begin
					  state_4 <= WAIT_FOR_RDDONE;
					  set_rd_done_bit <= 1'b0;
				  end
			 end
			 default:begin
				 start_sm0_rdma_flow <= 1'b0;
				 state_4 <= IDLE_SM4;
				 set_rd_done_bit <= 1'b0;
			 end 
			endcase
		 end
	end

endmodule
