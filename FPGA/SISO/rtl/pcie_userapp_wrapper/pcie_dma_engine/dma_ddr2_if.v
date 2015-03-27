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
// Purpose: This module is the logic that interfaces with the Xilinx MIG DDR2 
//          controller for the SODIMM on the ML555 and the PCIe DMA block.  
//          This module is responsible for crossing clock domains between the
//          two blocks.  There are three fifos in this module.  One for egress
//          data, one for ingress data, and one for passing address and
//          command to the MIG controller.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//  Jiansong:
//     TX flow control: 
//     (1) control signal: TX_fetch_next_4k, one cycle
//          it's the falling edge of egress_data_fifo almost full
//          egress_data_fifo almostfull is slightly less than half of the FIFO size
//     (2) TX has higher priority than transfer, or mrd > mwr
//      
//  modified by Jiansong Zhang:
//     (1)  ddr2 read/write scheduling, pending
//          divide address fifo into two fifos: rd and wr ctrl fifo
//          rd/wr ctrl fifo wren control
//          rd/wr scheduling in 200MHz domain
//     (2)  modify egress data fifo, use bram IP core instead -------------- done
//
//   modification on 2009-8-18, to slove the low throughput problem
//   (1) condense ddr_sm, in previous version, overhead could be 100% if data is 64 bytes
//   (2) modify pause_read_requests logic: if ingress_data_fifo is more than half full,
//       delay next dma read request
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

`include "Sora_config.v"

module dma_ddr2_if(
input  wire         dma_clk,
input  wire         ddr_clk,
input  wire         radio_clk,
input  wire         reset,

//DMA SIGNALS
//egress
////output wire [127:0] egress_data,
`ifdef Sora_16B_TX
output wire  [31:0] egress_data,
`else
output wire  [15:0] egress_data,
`endif
input  wire   [1:0] egress_fifo_ctrl,     //bit 1 = reserved   bit 0 = read_en
output wire   [2:0] egress_fifo_status,   //bit 2 = almost full
                                          //(needs to be empty before more 
                                          //egress requests complete)
                                          //bit 1 = empty   bit 0 = almostempty
input  wire   [2:0] egress_xfer_size,
input  wire  [27:6] egress_start_addr,
input  wire         egress_data_req,
output reg          egress_data_ack,
//output              TX_fetch_next_4k,     // signal for TX flow control, when egress_data_fifo
//                                          // becomes less than half-full, this signal is 
//														// asserted for one cycle
//ingress
input  wire [127:0] ingress_data,
input  wire   [1:0] ingress_fifo_ctrl,   //bit 1 = reserved   bit 0 = write_en
output wire   [1:0] ingress_fifo_status, //bit 1 = full       bit 0 = almostfull
input  wire   [2:0] ingress_xfer_size,
input  wire  [27:6] ingress_start_addr,
input  wire         ingress_data_req,
output reg          ingress_data_ack,
//END OF DMA SIGNALS
        
/// Jiansong: error signals
output wire         egress_overflow_one,
output reg [31:0]   egress_wr_data_count,
		  
//MEMORY CNTRLR SIGNALS
output wire [127:0] m_wrdata,
input  wire [127:0] m_rddata,
output reg   [30:0] m_addr,
output reg   [2:0]  m_cmd, //3'b000 == write command
                           //3'b001 == read command
                           //all others invalid - see MIG 1.72 documentaion for
                           //more information
output reg          m_data_wen,
output reg          m_addr_wen,
input  wire         m_data_valid,
input  wire         m_af_afull,
input  wire         m_wdf_afull,
//END OF MEMORY CNTRLR SIGNALS
output reg          pause_read_requests,
output reg [31:0]   Debug18DDR1,
output reg [31:0]   Debug19DDR2
);

reg ddr_reset, ddr_reset1;

/// Jiansong: debug signals
`ifdef Sora_16B_TX
wire [10:0] egress_read_data_count;
`else
wire [11:0] egress_read_data_count;
`endif
wire [8:0]  egress_write_data_count;

///Jiansong: signal for egress/TX flow control
reg egress_fifo_ready; 
wire egress_almostfull_falling;    // one cycle to detect falling edge of egress_almostfull
reg less_4k_pass;

reg   [2:0] dma_state;
////reg         addr_fifo_wren;
////reg  [31:0] addr_fifo_wrdata;
////wire        addr_fifo_almost_full;
reg         ingress_addr_fifo_wren;
reg  [31:0] ingress_addr_fifo_wrdata;
wire        ingress_addr_fifo_almost_full;
wire        ingress_addr_fifo_almost_empty;
reg         egress_addr_fifo_wren;
reg  [31:0] egress_addr_fifo_wrdata;
wire        egress_addr_fifo_almost_full;
wire        egress_addr_fifo_almost_empty;


reg   [4:0] ddr_state;
reg   [9:0] xfer_cnt;
reg         ingress_fifo_ren;

////reg         addr_fifo_ren;
////wire [31:0] addr_fifo_rddata;
reg         egress_addr_fifo_ren;
wire [31:0] egress_addr_fifo_rddata;
reg         ingress_addr_fifo_ren;
wire [31:0] ingress_addr_fifo_rddata;

reg         egress_fifo_wren;
reg [127:0] m_rddata_reg; 


//for the fifo status logic
wire        egress_full;
wire        egress_almostfull;
wire        egress_empty;

////wire        egress_full_a;
////wire        egress_almostfull_a;
////wire        egress_full_b;
////wire        egress_almostfull_b;
////wire        egress_almostempty_a;
////wire        egress_almostempty_b;


wire        ingress_almostempty;
wire        ingress_empty;
wire        ingress_datanotready;

//wire        ingress_almostempty_a;
//wire        ingress_empty_a;
//wire        ingress_almostempty_b;
//wire        ingress_empty_b;

////wire        addr_fifo_empty;
////wire        addr_fifo_full;
wire        ingress_addr_fifo_empty;
wire        ingress_addr_fifo_full;
wire        egress_addr_fifo_empty;
wire        egress_addr_fifo_full;

//wire  [1:0] ingress_fifo_status_a;
//wire  [1:0] ingress_fifo_status_b;


//ML505 specific wires/regs
wire [8:0] wrcount, rdcount;
reg [8:0] wrcount_gray_dma;
reg [8:0] wrcount_gray_ddr, wrcount_gray_ddr1;
wire [8:0] wrcount_ddr;
reg [8:0] wrcount_ddr_reg;
reg [8:0] rdcount_reg = 0;
////reg at_least_64B;

//States for DMA state machine
localparam IDLE  = 3'b000;
localparam LOADE = 3'b001;
localparam LOADI = 3'b010;
localparam DONE  = 3'b011;
localparam WAITE = 3'b100;        /// Jiansong: add one cycle to wait for the deassertion of egress request

//States for the DDR state machine
localparam NOOP  = 5'b00000;
////localparam LOAD0 = 5'b00001;
////localparam LOAD1 = 5'b00010;
localparam E_LOAD0 = 5'b00001;
localparam E_LOAD1 = 5'b00010;
localparam IN_LOAD0 = 5'b01101;
localparam IN_LOAD1 = 5'b01110;
localparam R1    = 5'b00011;
localparam R2    = 5'b00100;
localparam W0    = 5'b00101;
localparam W1    = 5'b00110;
localparam W2    = 5'b00111;
localparam W3    = 5'b01000;
localparam W4    = 5'b01001;
localparam W5    = 5'b01010;
localparam W6    = 5'b01011;
localparam WAIT_FOR_ROOM = 5'b01100;

/// Jiansong: debug register
always@(posedge ddr_clk)begin
   Debug18DDR1[7:0]   <= {3'b000,ddr_state[4:0]};
   Debug18DDR1[11:8]  <= {3'b000,less_4k_pass};
   Debug18DDR1[15:12] <= {3'b000,egress_fifo_ready};
   Debug18DDR1[19:16] <= {3'b000,egress_empty};
   Debug18DDR1[23:20] <= {3'b000,egress_addr_fifo_empty};
   Debug18DDR1[27:24] <= {3'b000,ingress_addr_fifo_empty};
   Debug18DDR1[31:28] <= {3'b000,ingress_data_req};
end

always@(posedge ddr_clk)begin
   Debug19DDR2[3:0]   <= {3'b000,egress_addr_fifo_full};
	Debug19DDR2[7:4]   <= {1'b0,egress_fifo_status[2:0]};
`ifdef Sora_16B_TX
	Debug19DDR2[18:8]  <= egress_read_data_count[10:0];
`else
	Debug19DDR2[19:8]  <= egress_read_data_count[11:0];
`endif
	Debug19DDR2[31:20] <= {3'b000,egress_write_data_count[8:0]};
end

//synchronize the DMA reset signal to the DDR2 clock
always@(posedge ddr_clk)begin
   ddr_reset1 <= reset;
   ddr_reset <= ddr_reset1;
end


//create a single gating signal for the ingress path so that the transmit
//engine will stop executing read requests if the fifos are nearing full
//condition; this signal is fedback to the DMA CNTRL block
always@(posedge dma_clk)begin
////   pause_read_requests <= ingress_fifo_status[0] | addr_fifo_almost_full;
//     pause_read_requests <= ingress_fifo_status[0] | ingress_addr_fifo_almost_full;
     pause_read_requests <= (~ingress_almostempty) | ingress_addr_fifo_almost_full;
end

/// Jiansong: modifications
///          (1) separate addr_cntrl_fifo to ingress addr fifo and egress addr fifo
///          (2) scheduling at addr fifo rd side, or 200MHz clock domain
///          (3) egress always has higher priority
///////////////////////////////////////////////////////////////////////////////
// DMA state machine  
//
// Takes request from the dma and loads into the ADDRESS/CNTRL FIFO(64 deep)
// for the ddr state machine
// 
// The TX and RX engines provide a request signal (*_data_req), and encoded
// transfer size (*_xfer_size) and a starting address (*_start_addr).
// The bottom 6 bits of the start address are not needed since the
// address should always be aligned on a 64B boundary (which is the smallest
// incremental address that a completion packet may be returned on) 
///////////////////////////////////////////////////////////////////////////////
always @ (posedge dma_clk) begin
  if(reset)begin
    dma_state        <= IDLE;
    ingress_addr_fifo_wren   <= 1'b0;
    ingress_addr_fifo_wrdata[31:0] <= 32'h00000000;
	 egress_addr_fifo_wren   <= 1'b0;
    egress_addr_fifo_wrdata[31:0] <= 32'h00000000;
    egress_data_ack  <= 1'b0;
    ingress_data_ack <= 1'b0;
  end else begin
    case(dma_state)
      IDLE:   begin
                 egress_data_ack  <= 1'b0;
                 ingress_data_ack <= 1'b0;
                 egress_addr_fifo_wren   <= 1'b0;
					  ingress_addr_fifo_wren   <= 1'b0;
////                 if(~addr_fifo_full)begin //don't do anything if addr_fifo is full
/// Jiansong: ddr read/write scheduling here
////                   if(ingress_data_req) begin
                     if(ingress_data_req & 
							   ~ingress_addr_fifo_full & 
								~ingress_fifo_status[0]) begin   /// Jiansong: if ingress_fifo_status
								                                 ///           is almostfull, block
																		   ///           ingress data request
                       dma_state <= LOADI;
                       //assert the ingress ack and load the ADDR/CNTRL fifo with
                       //the correct ddr2 write parameters
                       ingress_data_ack <= 1'b1;
                       ingress_addr_fifo_wrdata[31:0] <= ({6'b000000,
                                                  ingress_xfer_size[2:0],
                                                  ingress_start_addr[27:6],
                                                   1'b0});//bit 0 == 1'b0 denotes
                                                          //write to ddr2
                       ingress_addr_fifo_wren   <= 1'b1;    
////                   end else if(egress_data_req)begin
                     end else if(egress_data_req & ~egress_addr_fifo_full)begin
                       //Don't grant ack for egress if the egress fifo is almost
                       //full, otherwise the MIG controller might overfill the
                       //egress fifo.
////                       if(egress_almostfull) begin      /// Jiansong: how much is almost?
////                         dma_state <= IDLE;
////                       end else begin
                         //Otherwise, assert the egress ack and load the 
                         //ADDR/CNTRL fifo with the correct ddr2 read parameters
                         dma_state <= LOADE;
                         egress_data_ack <= 1'b1;
                         egress_addr_fifo_wrdata[31:0] <= ({6'b000000,
                                                    egress_xfer_size[2:0],
                                                    egress_start_addr[27:6],
                                                    1'b1});//bit 0 == 1'b1 denotes
                                                           //read from ddr2
                         egress_addr_fifo_wren   <= 1'b1;
////                       end
////                     end    
                 end else begin
                   dma_state <= IDLE;
                 end
               end
      //LOADE and LOADI are for deasserting the ack and wren signals
      //before returning to IDLE                
      LOADE:   begin
                 egress_addr_fifo_wren   <= 1'b0;
                 egress_data_ack <= 1'b0;
                 dma_state        <= WAITE;
               end
		/// Jiansong: add one cycle to wait for the deassertion of egress request
		WAITE:   begin
                 egress_addr_fifo_wren   <= 1'b0;
                 egress_data_ack <= 1'b0;
                 dma_state        <= DONE;
               end
      LOADI:   begin
                 ingress_addr_fifo_wren   <= 1'b0;
                 ingress_data_ack <= 1'b0;
                 dma_state        <= DONE;
               end
       DONE:   begin
                 dma_state        <= IDLE;
                 egress_data_ack  <= 1'b0;
                 ingress_data_ack <= 1'b0;
                 ingress_addr_fifo_wren   <= 1'b0;
					  egress_addr_fifo_wren   <= 1'b0;
               end
    default:   begin
                 dma_state        <= IDLE;
                 egress_data_ack  <= 1'b0;
                 ingress_data_ack <= 1'b0;
                 ingress_addr_fifo_wren   <= 1'b0;
					  egress_addr_fifo_wren   <= 1'b0;
               end
    endcase
  end
end

/// Jiansong: generate egress_fifo_ready signal
rising_edge_detect egress_fifo_almostfull_falling_inst(
                .clk(ddr_clk),
                .rst(rst),
                .in(~egress_almostfull),
                .one_shot_out(egress_almostfull_falling)
                );
always@(posedge ddr_clk)begin
   if(rst)
	   egress_fifo_ready <= 1'b0;
	// During a 4k-read-requests writing into ddr module, if a DRAM refresh cycle occurs that read 
	// requests write in is interrupted, the egress_almostfull signal may be deasserted and asserted
	// one more time unexpectedly. (ddr_stat != R1) means to bypass the deassert event if current 4k 
	// reading is still ongoing, which will prevent asserting egress_fifo_ready by mistake.
	else if(egress_almostfull_falling && (ddr_state != R1))    
	   egress_fifo_ready <= 1'b1;
	else if(egress_addr_fifo_ren)
	   egress_fifo_ready <= 1'b0;
end
always@(posedge ddr_clk)begin
   if(rst)
	   less_4k_pass <= 1'b0;
   else if(egress_addr_fifo_rddata[25:23] < 3'b110)
	   less_4k_pass <= 1'b1;
	else
	   less_4k_pass <= 1'b0;
end
/// Jiansong: modifications
///           (1) ingress/egress scheduling
///           (2) egress always has higher priority
///           (3) egress flow control to prevent egress_data_fifo overflow
///////////////////////////////////////////////////////////////////////
// DDR state machine  
//
// Monitors the ADDR/CNTRL FIFO and and translates incoming requests for 
// MIG DDR2 controller
//
///////////////////////////////////////////////////////////////////////
always@(posedge ddr_clk)
begin
  if(ddr_reset)begin
    ddr_state      <= NOOP;
////    addr_fifo_ren  <= 1'b0;
    egress_addr_fifo_ren  <= 1'b0;
	 ingress_addr_fifo_ren  <= 1'b0;
    xfer_cnt       <= 10'b0000000000;
    m_addr         <= 31'h00000000;
    m_cmd          <= 3'b000;
    m_addr_wen     <= 1'b0;
    m_data_wen     <= 1'b0;
    ingress_fifo_ren <= 1'b0;
  end else begin
    case(ddr_state)
      NOOP:    begin
                 m_data_wen    <= 1'b0;
                 m_addr_wen    <= 1'b0;
////                 //if the ADDR/CTRL fifo is not empty then read one entry
////                 //from the fifo, otherwise stay in this state and do nothing
////                 if(addr_fifo_empty) begin
////                   ddr_state     <= NOOP;
////                 end else begin
////                   ddr_state     <= LOAD0;
////                   addr_fifo_ren <= 1'b1;
////                 end
                 
					  // egress flow control: egress_fifo_ready signal and less_4k_pass signal
					  //       read out data only in 3 situation:
					  //       (1) first request (egress_empty)
					  //       (2) data in fifo becomes less than half full(egress_fifo_ready)
					  //       (3) small/the_last requests will not make egress_data_fifo overflow
					  //           (less_4k_pass)
					  // egress always has higher priority
                 if(~egress_addr_fifo_empty & 
					     (egress_empty | egress_fifo_ready | less_4k_pass)) begin
                   ddr_state     <= E_LOAD0;
                   egress_addr_fifo_ren <= 1'b1;
                 end else if(~ingress_addr_fifo_empty) begin
                   ddr_state     <= IN_LOAD0;
						 ingress_addr_fifo_ren <= 1'b1;
                 end else begin
                   ddr_state     <= NOOP;
                 end
               end
     E_LOAD0:  begin //state for deasserting the addr_fifo_ren signal
                 ddr_state     <= E_LOAD1;
                 egress_addr_fifo_ren <= 1'b0;
               end
     E_LOAD1:  begin /// Jiansong: m_addr distance 1 means 8bytes(64bits) 
                 //map a byte address into ddr2 column address:
                 //since the ddr2 memory in this ref. design is 
                 //64 bits (i.e. 8 bytes) wide, each column address
                 //addresses 8 bytes - therefore the byte address
                 //needs to be divided-by-8 for the ddr2 memory cntrl
                 //NOTE: addr_fifo_rddata[0] denotes read vs. write
                 //and is not part of the address
                 m_addr[30:25] <= 6'b000000; //upper bits not used
                 m_addr[24:3] <= egress_addr_fifo_rddata[22:1];//byte-to-column mapping
                 m_addr[2:0]   <= 3'b000; //always 0 for 64B boundary
                 egress_addr_fifo_ren <= 1'b0;
                 //decode the transfer size information into bytes
                 //and setup a counter (xfer_cnt) to keep track
                 //of how much data is transferred (ingress or egress)
                 case(egress_addr_fifo_rddata[25:23])
                    3'b000:  xfer_cnt <= 10'b0000000100;   //64B            
                    3'b001:  xfer_cnt <= 10'b0000001000;   //128B
                    3'b010:  xfer_cnt <= 10'b0000010000;   //256B
                    3'b011:  xfer_cnt <= 10'b0000100000;   //512B
                    3'b100:  xfer_cnt <= 10'b0001000000;   //1KB
                    3'b101:  xfer_cnt <= 10'b0010000000;   //2KB
                    3'b110:  xfer_cnt <= 10'b0100000000;   //4KB                
                    default: xfer_cnt <= 10'b0000001000; //default to 128B      
                 endcase
					  if(~m_af_afull)begin
					    ddr_state     <= R1;
                   m_addr_wen    <= 1'b1;//assert the write enable first
                   m_cmd[2:0] <= 3'b001;
					  end else begin
					    ddr_state <= E_LOAD1;
					  end
					end
    IN_LOAD0:  begin //state for deasserting the addr_fifo_ren signal
                 ddr_state     <= IN_LOAD1;
                 ingress_addr_fifo_ren <= 1'b0;
               end
    IN_LOAD1:  begin /// Jiansong: m_addr distance 1 means 8bytes(64bits) 
                 //map a byte address into ddr2 column address:
                 //since the ddr2 memory in this ref. design is 
                 //64 bits (i.e. 8 bytes) wide, each column address
                 //addresses 8 bytes - therefore the byte address
                 //needs to be divided-by-8 for the ddr2 memory cntrl
                 //NOTE: addr_fifo_rddata[0] denotes read vs. write
                 //and is not part of the address
                 m_addr[30:25] <= 6'b000000; //upper bits not used
                 m_addr[24:3] <= ingress_addr_fifo_rddata[22:1];//byte-to-column mapping
                 m_addr[2:0]   <= 3'b000; //always 0 for 64B boundary
                 ingress_addr_fifo_ren <= 1'b0;
                 //decode the transfer size information into bytes
                 //and setup a counter (xfer_cnt) to keep track
                 //of how much data is transferred (ingress or egress)
                 case(ingress_addr_fifo_rddata[25:23])
                    3'b000:  xfer_cnt <= 10'b0000000100;   //64B            
                    3'b001:  xfer_cnt <= 10'b0000001000;   //128B
                    3'b010:  xfer_cnt <= 10'b0000010000;   //256B
                    3'b011:  xfer_cnt <= 10'b0000100000;   //512B
                    3'b100:  xfer_cnt <= 10'b0001000000;   //1KB
                    3'b101:  xfer_cnt <= 10'b0010000000;   //2KB
                    3'b110:  xfer_cnt <= 10'b0100000000;   //4KB                
                    default: xfer_cnt <= 10'b0000001000; //default to 128B      
                 endcase
					  if(~m_af_afull && ~m_wdf_afull)begin           
                   ddr_state     <= W1;
                   m_cmd[2:0] <= 3'b000;
					  end else begin
					    ddr_state <= IN_LOAD1;
					  end
					end
////       LOAD0:  begin //state for deasserting the addr_fifo_ren signal
////                 ddr_state     <= LOAD1;
////                 addr_fifo_ren <= 1'b0;
////               end 
////       //LOAD1 state for latching the read data from the ADDR/CTRL fifo
////       //and decoding the information          
////       LOAD1:  begin /// Jiansong: m_addr distance 1 means 8bytes(64bits) 
////                 //map a byte address into ddr2 column address:
////                 //since the ddr2 memory in this ref. design is 
////                 //64 bits (i.e. 8 bytes) wide, each column address
////                 //addresses 8 bytes - therefore the byte address
////                 //needs to be divided-by-8 for the ddr2 memory cntrl
////                 //NOTE: addr_fifo_rddata[0] denotes read vs. write
////                 //and is not part of the address
////                 m_addr[30:25] <= 6'b000000; //upper bits not used
////                 m_addr[24:3] <= addr_fifo_rddata[22:1];//byte-to-column mapping
////                 m_addr[2:0]   <= 3'b000; //always 0 for 128B boundary
////                 addr_fifo_ren <= 1'b0;
////                 //decode the transfer size information into bytes
////                 //and setup a counter (xfer_cnt) to keep track
////                 //of how much data is transferred (ingress or egress)
////                 case(addr_fifo_rddata[25:23])
////                    3'b000:  xfer_cnt <= 10'b0000000100;   //64B            
////                    3'b001:  xfer_cnt <= 10'b0000001000;   //128B
////                    3'b010:  xfer_cnt <= 10'b0000010000;   //256B
////                    3'b011:  xfer_cnt <= 10'b0000100000;   //512B
////                    3'b100:  xfer_cnt <= 10'b0001000000;   //1KB
////                    3'b101:  xfer_cnt <= 10'b0010000000;   //2KB
////                    3'b110:  xfer_cnt <= 10'b0100000000;   //4KB                
////                    default: xfer_cnt <= 10'b0000001000; //default to 128B      
////                 endcase
////                 //if bit 0 is a 1 then egress or read from ddr
////                 //and jump to egress flow (state R)
////					  /// Jiansong: if egress fifo is full, block the process
////					  ///           it's temporary solution to prevent egress fifo overflow
////////                 if(addr_fifo_rddata[0] && ~m_af_afull)begin 
////                 if(addr_fifo_rddata[0] && ~m_af_afull && ~egress_almostfull)begin
////  					    ddr_state     <= R1;
////                   m_addr_wen    <= 1'b1;//assert the write enable first
////                   m_cmd[2:0] <= 3'b001; 
////                //otherwise it is ingress or write to ddr
////                //and jump to ingress flow (state W1) 
////////                 end else if(~m_af_afull && ~m_wdf_afull)begin
////                end else if(~addr_fifo_rddata[0] && ~m_af_afull && ~m_wdf_afull)begin            
////                   ddr_state     <= W1;
////                   m_cmd[2:0] <= 3'b000;
////                 end else begin
////                   ddr_state <= LOAD1;
////                 end    
////               end
        //Start of read from ddr2 (egress) flow      
         R1:    begin
                 //assert the write enable and increment the address
                 //to the memory cntrl;
                 //the ddr2 memory in this reference design is configured as
                 //burst-of-4 the address needs to be incremented by four
                 //for each read request
                 m_addr[30:0]     <= m_addr[30:0] + 3'b100;
                 xfer_cnt   <= xfer_cnt - 3'b10;
                 //when it gets to the penultimate transfer, go to R2
                 //to deassert the write enable
                 if(xfer_cnt == 10'h002)begin
                    ddr_state  <= NOOP;
                    m_addr_wen    <= 1'b0;
                 end else begin 
                    ddr_state  <= R1;
                    m_addr_wen    <= 1'b1;
                 end
               end
        R2:    begin
                 ddr_state   <= NOOP;
                 m_addr_wen  <= 1'b0;
               end
        //Start of write to ddr2 (ingress) flow 
        W1:    begin //assert the read enable from the ingress fifo
                     //to get the ingress data ready for writing 
                     //into the memory cntrl
                 m_addr_wen       <= 1'b0;
//                 if(at_least_64B && ~m_wdf_afull)begin
                 if(~ingress_datanotready && ~m_wdf_afull)begin
                   ddr_state        <= W2;
                   ingress_fifo_ren <= 1'b1;
                 end else begin
                   ddr_state      <= W1;
                   ingress_fifo_ren <= 1'b0;
                 end
               end       
        W2:    begin //now assert the address and data write enables
                     //to the memory cntrl
                 ddr_state     <= W3;
                 m_addr_wen    <= 1'b1;
                 m_data_wen    <= 1'b1;   
               end
        W3:    begin //deassert the address write enable but keep the
                     //data write enable asserted - this is because
                     //the data is written in 16 bytes (128 bit) at a time
                     //and each address is for 32 bytes, so we need two
                     //data cycles for each address cycles;
                     //also increment the address to the memory controller
                     //to the next column
                 ddr_state     <= W4;
                 m_addr_wen    <= 1'b0;
                 m_data_wen    <= 1'b1; 
                 m_addr[30:0]  <= m_addr[30:0] + 3'b100;  
               end
        W4:    begin //write the second column address to the memory cntrl
                 ddr_state     <= W5;
                 m_addr_wen    <= 1'b1;
                 m_data_wen    <= 1'b1; 
               end
        W5:    begin //decide whether to repeat the cycle or not
                 m_addr_wen    <= 1'b0;
                   //when it gets to the penultimate transfer, deassert the
                   //read enable to the ingress fifo read enable and then
                   //jump to W6 so that the data and address write enables
                   //to the memory cntrl are deasserted one clock later
                 if(xfer_cnt == 10'h004) begin 
                   ddr_state        <= NOOP;            /// bug? no. state W6 is not used
                   ingress_fifo_ren <= 1'b0;
                 end else if(~m_af_afull && ~m_wdf_afull)begin
                   //otherwise decrement the transfer count, increase the
                   //address and repeat the cycle
                   ddr_state     <= W2;
                   ingress_fifo_ren <= 1'b1;
                   xfer_cnt      <= xfer_cnt - 3'b100;
                   m_addr[30:0]  <= m_addr[30:0] + 3'b100;  
                 end else begin
                   ddr_state <= WAIT_FOR_ROOM;
                   ingress_fifo_ren <= 1'b0;
                 end
               end
        W6:    begin                    /// this state is not used
                 ddr_state     <= NOOP;
                 m_data_wen    <= 1'b0;
                 m_addr_wen    <= 1'b0;
               end
        WAIT_FOR_ROOM: begin
                 m_addr_wen    <= 1'b0;
                 if(~m_af_afull && ~m_wdf_afull)begin
                    m_data_wen <= 1'b1;
                    ddr_state <= W5;
                 end else begin     
                    m_data_wen <= 1'b0;
                    ddr_state <= WAIT_FOR_ROOM;
                 end
               end
     default:  begin
                 ddr_state      <= NOOP;
////                 addr_fifo_ren  <= 1'b0;
                 egress_addr_fifo_ren  <= 1'b0;
					  ingress_addr_fifo_ren  <= 1'b0;
                 xfer_cnt       <= 10'b0000000000;
                 m_addr[30:0]   <= 31'h00000000;
                 m_cmd[2:0]     <= 3'b000;
                 m_addr_wen     <= 1'b0;
                 m_data_wen     <= 1'b0;
                 ingress_fifo_ren <= 1'b0;
               end
    endcase
  end
end

/// Jiansong: added for egress/TX requests
egress_addr_cntrl_fifo egress_addr_cntrl_fifo(
.din          (egress_addr_fifo_wrdata[31:0]), //32
.rd_clk       (ddr_clk),
.rd_en        (egress_addr_fifo_ren),
.rst          (reset),
.wr_clk       (dma_clk),
.wr_en        (egress_addr_fifo_wren),
.almost_empty (egress_addr_fifo_almost_empty),
.almost_full  (egress_addr_fifo_almost_full),
.dout         (egress_addr_fifo_rddata[31:0]), //32
.empty        (egress_addr_fifo_empty),
.full         (egress_addr_fifo_full)
);

/// Jiansong: added for ingress/transfer requests
ingress_addr_cntrl_fifo ingress_addr_cntrl_fifo(
.din          (ingress_addr_fifo_wrdata[31:0]), //32
.rd_clk       (ddr_clk),
.rd_en        (ingress_addr_fifo_ren),
.rst          (reset),
.wr_clk       (dma_clk),
.wr_en        (ingress_addr_fifo_wren),
.almost_empty (ingress_addr_fifo_almost_empty),
.almost_full  (ingress_addr_fifo_almost_full),
.dout         (ingress_addr_fifo_rddata[31:0]), //32
.empty        (ingress_addr_fifo_empty),
.full         (ingress_addr_fifo_full)
);

//////ADDRESS/CNTRL FIFO to cross clock domains  32X64
////addr_cntrl_fifo addr_cntrl_fifo_inst(
////.din          (addr_fifo_wrdata[31:0]), //32
////.rd_clk       (ddr_clk),
////.rd_en        (addr_fifo_ren),
////.rst          (reset),
////.wr_clk       (dma_clk),
////.wr_en        (addr_fifo_wren),
////.almost_empty (addr_fifo_almost_empty),
////.almost_full  (addr_fifo_almost_full),
////.dout         (addr_fifo_rddata[31:0]), //32
////.empty        (addr_fifo_empty),
////.full         (addr_fifo_full)
////);
////// END ADDRESS/CNTRL FIFO

/// Jiansong: generate egress_overflow_one signal
rising_edge_detect egress_overflow_one_inst(
                .clk(ddr_clk),
                .rst(rst),
                .in(egress_full),
                .one_shot_out(egress_overflow_one)
                );
/// Jiansong: TX data fifo

`ifdef Sora_16B_TX
Egress_data_FIFO_16b Egress_data_FIFO_16b_inst (
	.rst              (reset),
////   .din              (m_rddata_reg[127:0]),
//   .din              ({m_rddata_reg[15:0],m_rddata_reg[31:16],m_rddata_reg[47:32],
//	                    m_rddata_reg[63:48],m_rddata_reg[79:64],m_rddata_reg[95:80],
//							  m_rddata_reg[111:96],m_rddata_reg[127:112]}),
   // Jiansong: temporary for 32b data
	.din					({m_rddata_reg[31:0],m_rddata_reg[63:32],
	                    m_rddata_reg[95:64],m_rddata_reg[127:96]}),
	.wr_clk           (ddr_clk),
	.wr_en            (egress_fifo_wren),
	.dout             (egress_data),
	.rd_clk           (radio_clk),
	.rd_en            (egress_fifo_ctrl[0]),
	.empty            (egress_empty),
	.full             (egress_full),
	.prog_empty       (egress_almostempty),
	.prog_full        (egress_almostfull),
	.rd_data_count    (egress_read_data_count),                 // debug purpose
	.wr_data_count    (egress_write_data_count)
);
`else
Egress_data_FIFO Egress_data_FIFO_inst (
	.rst              (reset),
////   .din              (m_rddata_reg[127:0]),
   .din              ({m_rddata_reg[15:0],m_rddata_reg[31:16],m_rddata_reg[47:32],
	                    m_rddata_reg[63:48],m_rddata_reg[79:64],m_rddata_reg[95:80],
							  m_rddata_reg[111:96],m_rddata_reg[127:112]}),
   // Jiansong: temporary for 32b data
////	.din					({m_rddata_reg[31:0],m_rddata_reg[63:32],
////	                    m_rddata_reg[95:64],m_rddata_reg[127:96]}),
	.wr_clk           (ddr_clk),
	.wr_en            (egress_fifo_wren),
	.dout             (egress_data),
	.rd_clk           (radio_clk),
	.rd_en            (egress_fifo_ctrl[0]),
	.empty            (egress_empty),
	.full             (egress_full),
	.prog_empty       (egress_almostempty),
	.prog_full        (egress_almostfull),
	.rd_data_count    (egress_read_data_count),                 // debug purpose
	.wr_data_count    (egress_write_data_count)
);
`endif

/// Jiansong: data count
always@(posedge ddr_clk)begin
   if(rst)
	   egress_wr_data_count <= 32'h0000_0000;
	else if (egress_fifo_wren)
	   egress_wr_data_count <= egress_wr_data_count + 32'h0000_0001;
	else
	   egress_wr_data_count <= egress_wr_data_count;
end

// Jiansong: it is replaced by an IP core fifo. The new fifo is from 200MHz to 
//           44MHz (or 40MHz). The old one is from 200MHz to 250MHz. So the timing
//           requirement is relaxed.
/////// Jiansong: why not use IP core?
//////the egress fifos are built using two fifo36 primitives in 
//////parallel - they have been placed in a wrapper because the
//////read data path and empty signal has been pipelined for timing purposes 
//////(mainly because of tight timing on the empty signal) and
//////require some complex logic to support - the signals
//////at this wrapper interface appear to the user exactly like
//////a regular fifo i.e. there is no extra clock of latency on the
//////read datapath due to the pipeline.
////egress_fifo_wrapper egress_fifo_wrapper_inst(
////        .RST(reset),
////        .WRCLK(ddr_clk),
////        .WREN(egress_fifo_wren),
////        .DI(m_rddata_reg[127:0]),
////        .FULL(egress_full),
////        .ALMOSTFULL(egress_almostfull),
////        .RDCLK(dma_clk),
////        .RDEN(egress_fifo_ctrl[0]),
////        .DO(egress_data[127:0]),
////        .EMPTY(empty),
////        .ALMOSTEMPTY(egress_almostempty)
////);


assign egress_fifo_status[2] = egress_almostfull;
assign egress_fifo_status[1] = egress_empty;
assign egress_fifo_status[0] = egress_almostempty;

ingress_data_fifo ingress_data_fifo_inst(
	.din(ingress_data[127:0]),
	.rd_clk(ddr_clk),
	.rd_en(ingress_fifo_ren),
	.rst(reset),
	.wr_clk(dma_clk),
	.wr_en(ingress_fifo_ctrl[0]),
	.dout(m_wrdata[127:0]),
	.empty(ingress_empty),
	.almost_empty(ingress_datanotready),
	.full(ingress_fifo_status[1]),
	.prog_empty(ingress_almostempty),
	.prog_full(ingress_fifo_status[0]),
	.rd_data_count(rdcount[8:0]),
	.wr_data_count(wrcount[8:0])
	);

/// Jiansong: the parameter ALMOST_EMPTY_OFFSET not realy works, don't know why
////
//////INGRESS or WRITE to DDR2 MEMORY FIFO
////FIFO36_72 #( 
//////.ALMOST_EMPTY_OFFSET (9'h005),
////.ALMOST_EMPTY_OFFSET (9'h100),   // Jiansong: if (~almost empty), assert pause_read_requests
////.ALMOST_FULL_OFFSET  (9'h1C4),
////.DO_REG              (1),
////.EN_ECC_WRITE        ("FALSE"),
////.EN_ECC_READ         ("FALSE"),
////.EN_SYN              ("FALSE"),
////.FIRST_WORD_FALL_THROUGH ("FALSE"))
////ingress_fifo_a(
////.ALMOSTEMPTY (ingress_almostempty_a), 
////.ALMOSTFULL  (ingress_fifo_status_a[0]),  
////.DBITERR     (), 
////.DO          (m_wrdata[63:0]), 
////.DOP         (), 
////.ECCPARITY   (), 
////.EMPTY       (ingress_empty_a), 
////.FULL        (ingress_fifo_status_a[1]), 
////.RDCOUNT     (), 
////.RDERR       (), 
////.SBITERR     (), 
////.WRCOUNT     (), 
////.WRERR       (),          
////.DI          (ingress_data[63:0]), 
////.DIP         (), 
////.RDCLK       (ddr_clk), 
////.RDEN        (ingress_fifo_ren), 
////.RST         (reset), 
////.WRCLK       (dma_clk), 
////.WREN        (ingress_fifo_ctrl[0])
////);
////
////FIFO36_72 #( 
//////.ALMOST_EMPTY_OFFSET (9'h005),
////.ALMOST_EMPTY_OFFSET (9'h100),   // Jiansong: if (~almost empty), assert pause_read_requests
////.ALMOST_FULL_OFFSET  (9'h1C4),
////.DO_REG              (1),
////.EN_ECC_WRITE        ("FALSE"),
////.EN_ECC_READ         ("FALSE"),
////.EN_SYN              ("FALSE"),
////.FIRST_WORD_FALL_THROUGH ("FALSE"))
////ingress_fifo_b(
////.ALMOSTEMPTY (ingress_almostempty_b), 
////.ALMOSTFULL  (ingress_fifo_status_b[0]), 
////.DBITERR     (), 
////.DO          (m_wrdata[127:64]), 
////.DOP         (), 
////.ECCPARITY   (), 
////.EMPTY       (ingress_empty_b), 
////.FULL        (ingress_fifo_status_b[1]), 
////.RDCOUNT     (rdcount[8:0]), 
////.RDERR       (), 
////.SBITERR     (), 
////.WRCOUNT     (wrcount[8:0]), 
////.WRERR       (),          
////.DI          (ingress_data[127:64]), 
////.DIP         (), 
////.RDCLK       (ddr_clk), 
////.RDEN        (ingress_fifo_ren), 
////.RST         (reset), 
////.WRCLK       (dma_clk), 
////.WREN        (ingress_fifo_ctrl[0])
////);


///////////////////////////////////////////////////////////////////////////
//////
////// Block-based transfer detector needed when DDR2 clock is faster
////// than PCIe clock i.e. ML505.  Otherwise could use empty flag.
//////
////// Monitors the rdcount and wrcount of the ingress_fifo to determine
////// when there is at least 64B block of data in the fifo
//////
////// signal at_least_64B is fed to the ddr state machine
//////
///////////////////////////////////////////////////////////////////////////
//////Binary to Gray encode wrcount                 /// interesting
////always@(posedge dma_clk)begin
////        wrcount_gray_dma[8:0] = {wrcount[8], wrcount[8:1] ^ wrcount[7:0]};
////end
////
//////transfer to ddr_clock domain
////always@(posedge ddr_clk)begin
////     wrcount_gray_ddr  <= wrcount_gray_ddr1;
////     wrcount_gray_ddr1 <= wrcount_gray_dma;
////end
////
//////Gray to Binary decode wrcount_gray_ddr and register the output
////assign wrcount_ddr[8:0] = {wrcount_gray_ddr[8], 
////                               wrcount_ddr[8:1] ^ wrcount_gray_ddr[7:0]};
////always@(posedge ddr_clk)
////      wrcount_ddr_reg[8:0] <= wrcount_ddr[8:0];
////
//////need to pipeline rdcount since DO_REG is set to 1
////always@(posedge ddr_clk)begin
////        if(ingress_fifo_ren) rdcount_reg <= rdcount;
////end
////
//////do a compare - if read count is 4 (or more) less than write count than that
//////means there is at least 64B of data in the ingress fifo and I can
//////safely do a 64B block read of the fifo
////always@(posedge ddr_clk)begin
////      if(ddr_reset)begin
////          at_least_64B <= 1'b0;
////      end else begin
////          at_least_64B <= ((wrcount_ddr_reg[8:0] - rdcount_reg[8:0]) >= 9'h004) 
////                           ? 1'b1 : 1'b0;
////      end
////end



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
//assign ingress_almostempty = ingress_almostempty_a | ingress_almostempty_b;
//assign ingress_almostempty = ingress_almostempty_a & ingress_almostempty_b;
//assign ingress_empty = ingress_empty_a | ingress_empty_b;
//assign ingress_fifo_status[1:0] = ingress_fifo_status_a[1:0] 
//                                  | ingress_fifo_status_b[1:0];



//pipeline the egress write enable and data from the memory cntrl
//and write into the egress fifos         
always@(posedge ddr_clk)begin 
        if(ddr_reset)begin
           egress_fifo_wren <= 1'b0;
        end else begin
           egress_fifo_wren <= m_data_valid;
        end
end

always@(posedge ddr_clk)begin
     m_rddata_reg[127:0] <= m_rddata[127:0]; 
end


endmodule
