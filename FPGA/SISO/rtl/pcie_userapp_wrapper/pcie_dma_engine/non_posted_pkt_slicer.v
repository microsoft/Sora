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
// Purpose: Non-Posted Packet Slicer module.  
//          The rules for calculating the number of packets are the following:
//           1 - Max Read Request Size cannot be violated
//           2 - address and length combos cannot cross a 4kb boundary
//           Assumptions:  Xfers will start on 128B boundaries
//               Xfers will be power of 2 multiples of 128B to 4KB
//               i.e. 128, 256, 512, 1024, 2048, 4096 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
// comment by Jiansong Zhang:
//    looks this module can handle dma read request larger than 4KB and up to 4G
//
// modified by Jiansong Zhang:
//   add state for TX descriptor request
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module non_posted_pkt_slicer(
    input         clk,
    input         rst,
    /// Jiansong: control signal for transfer recovering
    input         transferstart,	 
    //input from dma_ctrl_wrapper
    input         rd_dma_start,
    input [31:0]  dmarad,
    input [31:0]  dmarxs,
    input [63:0]  dmaras,
    input [2:0]   read_req_size,//from pcie block
	 /// Jiansong: added for TX descriptor request
	 input         rd_TX_des_start_one,
	 input [63:0]  TX_des_addr,
	 output reg    isDes,
    //in/outs to_From non_posted_pkt_builder
    input         ack,
    output reg    go,
    output reg [31:0] dmarad_reg,
    output reg [63:0] dmaras_reg,
    output [9:0]  length
);
    
    //state machine state definitions for state[3:0]
    localparam IDLE = 4'h0; 
    localparam IDLE_WAIT = 4'h1;
    localparam START = 4'h2; 
    localparam NORMAL = 4'h3; 
    localparam CROSS = 4'h4; 
    localparam LAST = 4'h5;
    localparam WAIT_FOR_ACK = 4'h6;
    localparam REGISTER_DMA_REG = 4'h7;
	 localparam START_TX_DES_REQ = 4'h8;

    //duplicate registers for timing purposes
    (*EQUIVALENT_REGISTER_REMOVAL="NO"*)    reg [31:0]  dmarad_new,dmarad_reg2;
    (*EQUIVALENT_REGISTER_REMOVAL="NO"*)    reg [31:0]  dmarxs_new,dmarxs_reg,
                                                        dmarxs_reg2;
    (*EQUIVALENT_REGISTER_REMOVAL="NO"*)    reg [63:0]  dmaras_new,dmaras_reg2;
    wire [31:0] dmarad_add, dmarad_term;
    wire [31:0] dmarxs_sub, dmarxs_term;
    wire [63:0] dmaras_add, dmaras_term;
    reg [3:0]   state;
    reg         update_dma_reg;
    reg         stay_2x; //to keep state[3:0] from switching during multi-cycle
    reg [12:0]   length_byte;
    wire [63:0] dmaras_temp;
    wire        four_kb_cross; //marks a 4KB crossing combinations
    (*EQUIVALENT_REGISTER_REMOVAL="NO"*)    reg [12:0] four_kb_xfer, 
                                                       four_kb_xfer2;
    wire        less_than_rrs;//marks a transfer less than max read request size
    wire [12:0] read_req_size_bytes;
    reg         last_flag;
    reg   rst_reg;

    always@(posedge clk) rst_reg <= rst;
 

//calculate the max read request size in bytes, 
//instead of the encoding as specified
//in the PCI Express Base Specification
assign read_req_size_bytes =13'h0001<<(read_req_size+7);

//lookahead to see if the next xfer address crosses a 4KB boundary 
//and flag if so
//if the 12th bit of the destination changes than a 4KB boundary will be
//crossed;
//synthesis tool should remove everything above bit[12]
assign dmaras_temp = dmaras_reg + read_req_size_bytes;
assign four_kb_cross = (dmaras_temp[12] == dmaras_reg2[12]) ? 1'b0 : 1'b1;

//flag if the xfer size is less than read request size    
assign less_than_rrs = (dmarxs_reg <= read_req_size_bytes) ? 1'b1 : 1'b0; 

//Pre-calculate the four KB boundary address in case it's needed
//Essentially four_kb_xfer is how many bytes in the current transfer
//are we away from the 4KB boundary
//need four_kb_xfer to be a register in order to meet 250 MHz timing 
always@(posedge clk)begin
    four_kb_xfer[12:0] = 13'h1000 - dmaras_reg[11:0]; 
    four_kb_xfer2[12:0] = 13'h1000 - dmaras_reg2[11:0]; 
end

//This state machine determines how to break up the larger requests into
//the smaller packets based on: RRS and 4KB crossing
//
//Uses some multi-cycle paths:
//      state, dma*_reg, four_kb_xfer  -> dma*_new
//are 2x multi-cycle paths
//The signal "stay_2x" ensures that the state variable
//state are static for at least two clock
//cycles when in the NORMAL, CROSS, and LAST states - of course
//dma*_reg and four_kb_xfer signals must also be static for 2 cycles
//while in these states
  always @ (posedge clk) begin
  if (rst_reg | (~transferstart)) begin
      state <= IDLE;
      update_dma_reg <= 0;
      stay_2x <= 1'b0;
      go <= 0;
      last_flag <= 1'b0;
  end else begin
      case (state)
        IDLE : begin
           //wait for the start signal from the dma_ctrl_wrapper before doing
           //anything
           update_dma_reg <= 0;
           stay_2x <= 1'b0;
           go <= 0;
           last_flag <= 1'b0;
           if(rd_dma_start) 
             state <= IDLE_WAIT;
           else if(rd_TX_des_start_one)    /// Jiansong:
			    state <= START_TX_DES_REQ;    ///     add states for tx descriptor request generation        
			  else                            
             state <= IDLE;
           end
         IDLE_WAIT: begin //idle_wait added because of four_kb_xfer 
                           //being registered
             state <= START;
         end
			
			/// Jiansong: states added for tx descriptor request
			START_TX_DES_REQ: begin
			  go <= 1;  //send the current address/length values while
                     //the new ones are being calculated
           if(stay_2x == 1'b0)begin
              state <= START_TX_DES_REQ;
              stay_2x <= 1'b1;
           end else begin
			     last_flag <= 1;
              stay_2x <= 1'b0; 
              state <= WAIT_FOR_ACK;
           end
			end
			
         START : begin        /// Jiansong: start dma read
           update_dma_reg <= 0;
           stay_2x <= 1'b0;
           go <= 0;
           //determine addresses/lengths of the next packet 
           //may be normal i.e. read request size
           //cross a 4kb boundary
           //or the last packet of the xfer (which may also be normal if the
           //last packet happens to be the same as max payload size)
           //and go to the correct state to do the arithmetic
            case ({four_kb_cross, less_than_rrs})
               2'b00:
                  state <= NORMAL;
               2'b01:
                  state <= LAST;
               2'b10:
                  state <= CROSS;
               2'b11://because four_kb_cross uses look-ahead math, need to 
                     //determine if we will really cross a four KB boundary
                     //in the case where we have a transfer which is flagged
                     //as less_than_mps and do the appropriate transfer
                 if(dmarxs_reg > four_kb_xfer)
                  state <= CROSS;
                 else
                  state <= LAST;
            endcase      
         end
   
         NORMAL : begin //add a RRS to the current parameters
           go <= 1;  //send the current address/length values while
                     //the new ones are being calculated
           if(stay_2x == 1'b0)begin
              state <= NORMAL;
              stay_2x <= 1'b1;
           end else begin
              stay_2x <= 1'b0; 
              state <= WAIT_FOR_ACK;
           end
          end
         CROSS : begin //add just enough to the current parameters
                       //to get us to the 4KB boundary    
           go <= 1;           
           if(stay_2x == 1'b0)begin
              state <= CROSS;
              stay_2x <= 1'b1;
           end else begin
              stay_2x <= 1'b0; 
              state <= WAIT_FOR_ACK;
           end
         end
         LAST : begin //add the remaining to the current parameters
           go <= 1;          
           last_flag <= 1'b1;           
           if(stay_2x == 1'b0)begin
              state <= LAST;
              stay_2x <= 1'b1;
           end else begin
              stay_2x <= 1'b0; 
              state <= WAIT_FOR_ACK;
           end
          end

         WAIT_FOR_ACK : begin
           if(ack)begin
             update_dma_reg <= 1'b1;
             go <= 1'b0;
             if(last_flag)begin
               state <= IDLE;
             end else begin
               state <= REGISTER_DMA_REG;
             end
           end else begin 
             update_dma_reg <= 1'b0;
             go <= 1'b1;
             state <= WAIT_FOR_ACK;
           end
         end

         REGISTER_DMA_REG: begin
           //go ahead and update the current address/length values
           //for the next go-around through the state machine
           update_dma_reg <= 1'b0;
           state <= IDLE_WAIT;
         end

         default : begin
           update_dma_reg <= 0;
           go <= 0;
           state <= IDLE;
         end
      endcase   
     end
    end

//determine what we will add to the dest. address
assign dmarad_term = (state == NORMAL) ? read_req_size_bytes : 
                     (state == CROSS)  ? four_kb_xfer : 
                     (state == LAST)   ? dmarxs_reg : 
                                         read_req_size_bytes;
                                                                                //do the addition for the dest. address                           
assign dmarad_add = dmarad_reg2 + dmarad_term;

//determine what we will add to the source address
assign dmaras_term = (state == NORMAL) ? read_req_size_bytes : 
                     (state == CROSS)  ? four_kb_xfer2 : 
                     (state == LAST)   ? dmarxs_reg : 
                                         read_req_size_bytes;
                                                                                //do the addition for the source address                          
assign dmaras_add = dmaras_reg2 + dmaras_term;

//determine how much to subtract from the transfer size  
assign dmarxs_term = (state == NORMAL) ? read_req_size_bytes : 
                     (state == CROSS)  ? four_kb_xfer2 : 
                     (state == LAST)   ? dmarxs_reg : 
                                         read_req_size_bytes;
//do the subtraction to the transfer size                    
assign dmarxs_sub = dmarxs_reg2 - dmarxs_term;

always@(posedge clk)begin
   if(stay_2x)begin
           dmarad_new <= dmarad_add;
           dmaras_new <= dmaras_add;
           dmarxs_new <= dmarxs_sub;
   end
end

//register dmarad,dmarxs, and dmaras when rd_dma_start is high
//rd_dma_start is only asserted for one clock cycle
always@(posedge clk)begin
   if(rst_reg)begin
      dmarad_reg <= 32'h0000_0000;
      dmarxs_reg <= 32'h0000_0000;
      dmaras_reg <= 64'h0000_0000_0000_0000;
      dmarad_reg2 <= 32'h0000_0000;
      dmarxs_reg2 <= 32'h0000_0000;
      dmaras_reg2 <= 64'h0000_0000_0000_0000;      
   end else if(rd_dma_start)begin
      dmarad_reg <= dmarad;
      dmarxs_reg <= dmarxs;
      dmaras_reg <= dmaras;
      dmarad_reg2 <= dmarad;
      dmarxs_reg2 <= dmarxs;
      dmaras_reg2 <= dmaras;      
   end else if(update_dma_reg)begin
      dmarad_reg <= dmarad_new; 
      dmarxs_reg <= dmarxs_new;
      dmaras_reg <= dmaras_new;   
      dmarad_reg2 <= dmarad_new; 
      dmarxs_reg2 <= dmarxs_new;
      dmaras_reg2 <= dmaras_new;      
   end else if(rd_TX_des_start_one)begin    /// Jiansong:
	   dmaras_reg <= TX_des_addr;            ///     added for TX descriptor request
	end
end   

//additional output from state machine
always@(posedge clk)begin
   if(rst_reg)
      length_byte[12:0] <= 0;
   else if(state == NORMAL)
      length_byte[12:0] <= read_req_size_bytes[12:0];
   else if (state == LAST)
      length_byte[12:0] <= dmarxs_reg2[12:0];
   else if (state == CROSS)
      length_byte[12:0] <= four_kb_xfer2[12:0];
   else if (state == START_TX_DES_REQ)      /// Jiansong:
	   length_byte[12:0] <= 13'h0020;        ///     added for TX descriptor request, 32 bytes TX des
	else
      length_byte <= length_byte;
end

assign length[9:0] = length_byte[11:2];  

/// Jiansong: added for TX descriptor request
always@ (posedge clk)begin
   if(rst_reg)
	   isDes <= 0;
	else if ((state == NORMAL) | (state == LAST) | (state == CROSS))
	   isDes <= 0;
	else if (state == START_TX_DES_REQ)
	   isDes <= 1;
	else
	   isDes <= isDes;
end 

endmodule
