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
// Purpose: Internal DMA Control and status register file for PCIE-DDR2 DMA 
//          design.  This register file should only be used for dma transfers
//          up to 4KB in size.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module internal_dma_ctrl
(
	 input clk,             
	 input rst, 
	 //interface from dma_ctrl_status_reg file;
	 //these inputs could also be directly driven from the host system if desired
	 //in which case the dma_ctrl_status_reg_file block should be removed from the
	 //design 
	 input [31:0] reg_data_in, 
	 input [6:0] reg_wr_addr, 
	 input [6:0] reg_rd_addr, 
	 input reg_wren,    
	 output reg [31:0] reg_data_out, //reg_data_out is never used

	 //DMA parameter control outputs to TX and RX engines
	 output [63:0] dmaras,     //Read address source (from host memory)
	 output reg [31:0] dmarad, //Read address destination (to backend memory)
	 output reg [31:0] dmarxs, //Read transfer size in bytes

	 output rd_dma_start,  //read dma start control signal
	 input rd_dma_done,    //read dma done signal from RX engine

	 //Performance counts from performance counter module
	 //Not used in this module because the copies in dma_ctrl_status_reg_file
	 //are used instead
	 input [31:0] dma_wr_count,
	 input [31:0] dma_rd_count
); 


reg [31:0] dmaras_l, dmaras_u;
reg [31:0] dmacst;

//concatanate to form the 64 bit outputs
assign dmaras[63:0] = {dmaras_u,dmaras_l};

////assign wr_dma_start = dmacst[0];
assign rd_dma_start = dmacst[2];


//block for writing into the regfile
//--when reg_wren is asserted, reg_data_in will be written to one of the 
//registers as chosen by reg_wr_addr selection signal
always@(posedge clk or posedge rst) begin
    if(rst) begin
       dmaras_l <= 0;
       dmaras_u <= 0;
       dmarad   <= 0;
       dmarxs   <= 0;
     end else begin    
          if(reg_wren) begin 
                  case(reg_wr_addr) 
                        7'b000_1100: dmaras_l <= reg_data_in; //0x0C
                        7'b001_0000: dmaras_u <= reg_data_in; //0x10
                        7'b001_0100: dmarad   <= reg_data_in; //0x14
                        7'b001_1100: dmarxs   <= reg_data_in; //0x1C
                        default: begin 
                                      dmaras_l <= dmaras_l;
                                      dmaras_u <= dmaras_u;
                                      dmarad <= dmarad;
                                      dmarxs <= dmarxs;
                        end
                     endcase
            end 
        end
     end

//use a separate always block for dmacst[3:2] for clarity  
//dmacst[2] == rd_dma_start;  host sets this bit to start a dma transfer
//                            it is automatically cleared when the 
//                            dma transfer completes
//dmacst[3] == rd_dma_done;   asserted when the dma transfer is finished
//                              this bit can be polled by the host or it could
//                              be used to drive hardware block to generate
//                              an interrupt
//                              this bit must be cleared by the host by 
//                              writing a "1" to it.       
always@(posedge clk) begin
    if(rst) begin
       dmacst[3:2]   <= 2'b00;
     end else begin    
         if(rd_dma_done) begin //rd_dma_done from RX Engine
               dmacst[2] <= 1'b0;
               dmacst[3] <= 1'b1;
          end else if(reg_wren) begin 
                  case(reg_wr_addr) 
                        7'b010_1000: begin //0x28
								     /// Jiansong:
								     //take care of the unused bits in this always
                             //block
                             dmacst[31:4] <= reg_data_in[31:4];
									  dmacst[1:0]  <= reg_data_in[1:0];
                             //set the start bit if the host writes a 1
                             //the host cannot clear this bit
                             if(reg_data_in[2])
                                dmacst[2] <= 1'b1;
                             else
                                dmacst[2] <= dmacst[2];

                             //clear the done bit if the host writes a 1
                             //the host cannot set this bit
                             if(reg_data_in[3])
                                dmacst[3] <= 1'b0;
                             else
                                dmacst[3] <= dmacst[3]; 
                         end
                         default: begin 
                                 dmacst[3:2] <= dmacst[3:2];
                        end
                     endcase
               end 
        end
     end

// output register for cpu
// this is a read of the reg_file
// the case stmt is a mux which selects which reg location
// makes it to the output data bus
// Not used in this design

always@(posedge clk or posedge rst ) 
  begin                                                                
     if(rst)                           
          begin
      reg_data_out <= 0;
        end
     else
         begin                              
            case(reg_rd_addr[6:0])
              7'b000_1100: reg_data_out <= dmaras_l;
              7'b001_0000: reg_data_out <= dmaras_u;
              7'b001_0100: reg_data_out <= dmarad;
              7'b001_1100: reg_data_out <= dmarxs;
              7'b010_1000: reg_data_out <= dmacst;
              7'b011_0000: reg_data_out <= dma_wr_count;
              7'b011_0100: reg_data_out <= dma_rd_count;
            endcase
        end
   end

  endmodule
