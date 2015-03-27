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
// Module Name:    rx_trn_monitor 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: Receive TRN Monitor module. This module interfaces to the DMA
// Control/Status Register File and the Read Request Fifo to determine when a 
// DMA transfer has completed fully.  This module could also monitor the TRN
// Interface to determine any errors.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module rx_trn_monitor(
	input  wire         clk,
	input  wire         rst,

	//DMA_CTRL_WRAPPER FILE SIGNALS
	input  wire         rd_dma_start,  //indicates the start of a read dma xfer
	input  wire  [31:0] dmarad,        //destination addres(ddr2) only 13 bits used
	input  wire  [31:0] dmarxs,        //size of the complete transfer
	output reg          rd_dma_done,   //dma transfer complete
	input  wire         read_last,
	///Jiansong: interface to RX engine, indicate the system is in dma read for TX desc
	///          when this signal is asserted, received cpld will not be count in 
	///          length subtraction
	input  wire         Wait_for_TX_desc,
	input  wire         transferstart,     // control signal for transfer recovering
	//Read Request Wrapper interface signals
	//this is requests of the dma transfer and how the TX ENGINE broke them down
	//into smaller xfers
	output reg [4:0]  rx_waddr,
	output reg [31:0] rx_wdata,
	output reg        rx_we,
	output wire [4:0] rx_raddr,
	input [31:0]      rx_rdata,
	output reg        pending_comp_done,
	input [31:0]      completion_pending,
	//PCIe Endpoint Block Plus interface
	// Rx TRN
	input wire [63:0] trn_rd,
	input wire [7:0] trn_rrem_n,
	input wire trn_rsof_n,
	input wire trn_reof_n,
	input wire trn_rsrc_rdy_n,
	input wire trn_rsrc_dsc_n,
	input wire trn_rerrfwd_n,
	input wire [6:0] trn_rbar_hit_n,
	input wire [11:0] trn_rfc_npd_av,
	input wire [7:0] trn_rfc_nph_av,
	input wire [11:0] trn_rfc_pd_av,
	input wire [7:0] trn_rfc_ph_av,
	input wire [11:0] trn_rfc_cpld_av,
	input wire [7:0] trn_rfc_cplh_av,
	//interface from rx_trn_data_fsm 
	input wire fourdw_n_threedw, //fourdw = 1'b1; 3dw = 1'b0;
	input wire payload,
	input wire [2:0] tc, //traffic class
	input wire td, //digest
	input wire ep,  //poisoned bit
	input wire [1:0] attr,
	input wire [9:0] dw_length,
	input wire [15:0] MEM_req_id,
	input wire [7:0] MEM_tag,
	input wire [15:0]   CMP_comp_id,
	input wire [2:0]CMP_compl_stat,
	input wire  CMP_bcm,
	input wire [11:0] CMP_byte_count,
	input wire [63:0] MEM_addr,  
	input wire [15:0] CMP_req_id,
	input wire [7:0] CMP_tag,
	input wire [6:0] CMP_lower_addr,
	input wire    MRd,
	input wire    MWr,
	input wire    CplD,
	input wire    Msg,
	input wire    UR,
	input wire header_fields_valid,
	input wire data_valid,
	//Outputs to xfer_trn_mem_fifo
	output reg isDes,   /// Jiansong: added for TX des
	output wire [27:6] mem_dest_addr,
	output reg [10:0] mem_dma_size,
	output wire mem_dma_start,
	/// Jiansong: output to tx_sm
	output wire [9:0] np_rx_cnt_qw,
	/// Jiansong: debug register
	output reg [9:0] Debug30RXEngine
);

//states for memctrl_state
localparam IDLE            = 5'b00000;
localparam CALC_NEXT_ADDR  = 5'b00001;
localparam WRITEBACK_ADDR  = 5'b00010;
localparam WRITEBACK_ADDR2 = 5'b00011;

//states for addsub_state
localparam AS_IDLE         = 2'b00;
localparam REGISTER_CALC   = 2'b01;
localparam WAIT_FOR_REG    = 2'b10;

//additional pipelines regs for RXTRN interface to match rx_trn_data_fsm module
reg [63:0] trn_rd_d1;
reg [7:0] trn_rrem_d1_n;
reg trn_rsof_d1_n;
reg trn_reof_d1_n;
reg trn_rsrc_rdy_d1_n;
reg trn_rsrc_dsc_d1_n;
reg trn_rerrfwd_d1_n;
reg [6:0] trn_rbar_hit_d1_n;
reg [11:0] trn_rfc_npd_av_d1;
reg [7:0] trn_rfc_nph_av_d1;
reg [11:0] trn_rfc_pd_av_d1;
reg [7:0] trn_rfc_ph_av_d1;
reg [11:0] trn_rfc_cpld_av_d1;
reg [7:0] trn_rfc_cplh_av_d1;


//registers to store output of comp ram   
reg [27:6] cur_dest_addr; //bottom 5 lsb all zero

//state machine registers
reg [4:0] memctrl_state;
reg [1:0] addsub_state;

//single-shot signal which qualifies the header fields
wire header_fields_valid_one;

//ddr2 memory address which gets written back into the compram
wire [27:6] next_mem_dest_addr;

reg data_valid_reg;

//pipelined registers
reg CplD_r1, CplD_r2;

//signals to calculate and assert rd_dma_done
reg add_calc = 0;
reg sub_calc = 0;
reg add_complete;
reg sub_complete;
reg stay_2x;
reg update_dmarxs_div8_reg;
reg [9:0] dmarxs_div8_reg_new;
reg [9:0] dw_length_d1;
reg [9:0] dmarxs_div8_reg;
reg [9:0] dmarxs_div8_now;
wire rd_dma_done_early, rd_dma_done_early_one;
reg rd_dma_done_i; 
wire rd_dma_done_one; 


reg   rst_reg;
always@(posedge clk) rst_reg <= rst;

/// Jiansong: debug output
always@(posedge clk) begin
   Debug30RXEngine[9:0]   <= dmarxs_div8_now[9:0];
end


/// Jiansong:
wire rd_dma_start_one;
reg rd_dma_start_reg;
rising_edge_detect rd_dma_start_one_inst(
                .clk(clk),
                .rst(rst_reg),
                .in(rd_dma_start_reg),
                .one_shot_out(rd_dma_start_one)
                );
//register for timing purposes
always@(posedge clk) 
        rd_dma_start_reg <= rd_dma_start;




/// Jiansong: notice, pipeline registers
//all the outputs of the endpoint should be pipelined
//to help meet required timing of an 8 lane design
always @ (posedge clk)
begin
    trn_rd_d1[63:0]          <= trn_rd[63:0]         ;
    trn_rrem_d1_n[7:0]       <= trn_rrem_n[7:0]      ;
    trn_rsof_d1_n            <= trn_rsof_n           ;
    trn_reof_d1_n            <= trn_reof_n           ;
    trn_rsrc_rdy_d1_n        <= trn_rsrc_rdy_n       ;
    trn_rsrc_dsc_d1_n        <= trn_rsrc_dsc_n       ;
    trn_rerrfwd_d1_n         <= trn_rerrfwd_n        ;
    trn_rbar_hit_d1_n[6:0]   <= trn_rbar_hit_n[6:0]  ;
    trn_rfc_npd_av_d1[11:0]  <= trn_rfc_npd_av[11:0] ;
    trn_rfc_nph_av_d1[7:0]   <= trn_rfc_nph_av[7:0]  ;
    trn_rfc_pd_av_d1[11:0]   <= trn_rfc_pd_av[11:0]  ;
    trn_rfc_ph_av_d1[7:0]    <= trn_rfc_ph_av[7:0]   ;
    trn_rfc_cpld_av_d1[11:0] <= trn_rfc_cpld_av[11:0];
    trn_rfc_cplh_av_d1[7:0]  <= trn_rfc_cplh_av[7:0] ;
end



/*****************************************************************************
 Placeholder for the following TBD logic blocks:

- Check ECRC Logic - not implemented and not supported by Endpoint Block Plus
                     Rev 1.6.1

- Check unsupported request
   - Mem transaction but no BAR hit is unsupported

- Completion timeout logic error

- Check for unexpected completer 

- Completion Status bits check

******************************************************************************/

//Jiansong: we don't need to modify CMP-write-to-fifo logics, but the address may be wrong 
//           (RCB address 0). 

//use the completion packet tag field to address the comp ram for reading
assign   rx_raddr[4:0] = CMP_tag[4:0]; 

///////////////////////////////////////////////////////////////////////////////
//State machine for calculating the next DDR2 address in a series of
//completions i.e. a single read request split up into multiple completions
//Update the address in the compram for each completion packet received
always@(posedge clk)begin
   if(rst_reg)begin
      memctrl_state <= IDLE;
      cur_dest_addr[27:6] <= 0;
		isDes <= 0;                      /// Jiansong: added for TX des
      rx_waddr <= 0;
      rx_wdata <= 0;
      rx_we <= 1'b0;
      pending_comp_done <= 1'b0;
   end else begin
      case(memctrl_state)
         IDLE:begin
            rx_waddr[4:0] <= 5'b00000;
            rx_wdata[31:0] <= 32'h00000000;
            rx_we <= 1'b0;
            pending_comp_done <= 1'b0;
             //when header_fields_valid_one and CplD, rx_rdata already has
             //the correct information on it so go ahead and latch the
             //current destination address, then jump to CALC_NEXT_ADDR
             //in order to calculate the destination address for the next
             //future completion packet
             if(header_fields_valid_one && CplD)begin    
                memctrl_state <=  CALC_NEXT_ADDR; 
                cur_dest_addr[27:6] <= rx_rdata[21:0];
					 isDes <= rx_rdata[31];                    /// Jiansong: added for TX des
             end else begin
                memctrl_state <= IDLE;
             end
         end
         CALC_NEXT_ADDR:begin //wait state
            memctrl_state <= WRITEBACK_ADDR;
         end
         WRITEBACK_ADDR:begin //write back the ddr2 destination address
            //don't really care about the length field in the compram as
            //I use the packet header information to determine when the 
            //series of completion packets are done - see below
            // i.e.  if(dw_length[9:0] == CMP_byte_count[11:2]) 
            memctrl_state <= WRITEBACK_ADDR2;
            rx_waddr[4:0] <= rx_raddr[4:0];
            rx_wdata[31:0] <= {10'b0000000000, next_mem_dest_addr[27:6]};
            rx_we <= 1'b1;
            //if byte_count and length field are the same then it is the 
            //last packet and assert pending_comp_done
            if(dw_length[9:0] == CMP_byte_count[11:2]) 
                pending_comp_done <= 1'b1;
            else
                pending_comp_done <= 1'b0;
         end
         WRITEBACK_ADDR2:begin //rx write needs to be at least 2 clock cycles
            memctrl_state <= IDLE;
         end
         default:begin
            cur_dest_addr[27:6] <= 0;
            rx_waddr <= 0;
            rx_wdata <= 0;
            rx_we <= 1'b0;
            pending_comp_done <= 1'b0;
         end
      endcase
    end
  end  
//Additional statemachine inputs/outputs

//the falling edge of data_valid creates a one shot signal to start the 
//memory xfer and start calculations for the next xfer address
rising_edge_detect header_fields_valid_one_inst(  
                .clk(clk),
                .rst(rst_reg),
                .in(header_fields_valid), 
                .one_shot_out(header_fields_valid_one)
                );    
                

assign  next_mem_dest_addr[27:6] = (dw_length[9:0] != 10'b0000000000) 
                                 //assumes 64B boundary 
                                 ? cur_dest_addr[27:6] + dw_length[9:4]  
                                 //special case for 4KB                         
                                 : cur_dest_addr[27:6] + 7'b1000000;     
//end of state machine
///////////////////////////////////////////////////////////////////////////////
                                 
///////////////////////////////////////////////////////////////////////////////
//logic for interfacing to xfer_trn_mem_fifo
//  -mem_dest_addr
//  -mem_dma_size
//  -mem_dma_start                                 

assign mem_dest_addr[27:6] = cur_dest_addr[27:6];

always@(posedge clk)begin
   if(rst_reg)begin
      mem_dma_size[10:0] <= 11'b00000000000;
   end else begin
      if(CplD & header_fields_valid_one)begin   
         mem_dma_size[10:0] <= (dw_length[9:0] != 10'b0000000000) 
                               ? {1'b0,dw_length[9:0]}
                               : 11'b10000000000;
      end
   end
end

//Make a falling edge detector of data_valid
//This will signal mem_dma_start which means that for
//every completer packet received, it will get written into
//the DDR2 as soon as the last chunk of data has been written
//to the buffering fifo.  Make sure to qualify with the CplD signal
//as the data_valid signal could also be asserted during a Posted MemWr target
//transaction.
always@(posedge clk)begin
   if(rst_reg)begin
      data_valid_reg <= 1'b0;
   end else begin
      data_valid_reg <= data_valid;
   end
end

always@(posedge clk)begin
   CplD_r1 <= CplD;
   CplD_r2 <= CplD_r1;
end

/// Jiansong: falling edge detector
assign mem_dma_start = data_valid_reg & ~data_valid & CplD_r2;          
//////////////////////////////////////////////////////////////////////////////

/// Jiansong: modification in following logics for transfer recovering
//////////////////////////////////////////////////////////////////////////////
//Logic for detecting when an DMA Read has completed
//and for starting a continuation transfer early.
//Due to the way the dma_ctrl_wrapper works, i.e. breaks up large 
//transfers into 4KB or smaller
//sub-transfers, to keep the bandwidth high the dma_ctrl_wrapper must
//issue these sub-transfers in a staggered fashion.  That is, the next
//sub-transfer should be executed slightly before the current one is finished.
//The rd_dma_done_early signal accomplishes this by detecting when the current
//transfer is ALMOST complete and signalling to the dma_ctrl_wrapper to
//issue the next sub-transfer.  In addition to detecting early completion,
//we need logic to detect when the full (large) dma transfer is completed
//A register, dmarxs_div8_now[9:0], keeps track of how many outstanding
//DWORDS of data are remaining to be received.  As a completion packet is
//received, the dwlength field in the packet is subtracted off of this 
//register.  Conversely, as transfers are executed by the dma_ctrl_wrapper,
//the amount of the transfer is added to the register. When the register
//reaches 0 the DMA is basically complete (although there may be some
//transfers which are smaller than 1KB that need to execute to completely
//finish the full dma transfer - see the comments below about the
//rd_dma_done_early signal).

//divide dmarxs by 8 (left-shift 3 bits) also pipeline for timing
//this is the term that will get added to dmarxs_div8_now register
always@(posedge clk) dmarxs_div8_reg[9:0] <= dmarxs[12:3];

//need to make sure we use dw_length_d1 in calculations
//below since sub_calc is set using trn_reof_d1_n i.e.
//there is potential for dw_length to change very soon
//after trn_reof_d1_n, like if another type of packet
//gets received or a different size completion;
//dw_length_d1 is the term that will get subtracted from dmarxs_div8_now reg
//always@(posedge clk) dw_length_d1[9:0] <= dw_length[9:0];
// Jiansong: make sure dw_length will not change before subcalculation
always@(posedge clk) begin
   if (header_fields_valid_one & CplD)
      dw_length_d1[9:0] <= dw_length[9:0];
end

//add_calc and sub_calc signals are inputs to the addsub statemachine
//and tell the state machine which arithmetic operation to execute

//if dma_ctrl_wrapper executes a dma transfer by asserting rd_dma_start
//then we need to do the addition
/// Jiansong: add reset and transferstart control 
always@(posedge clk)begin  
     if (rst_reg | (~transferstart))
	      add_calc <=  1'b0;
     else if(rd_dma_start_one) //set the bit
         add_calc <=  1'b1;
     else if (add_complete) //reset the bit
         add_calc <=  1'b0;
end

//if a completion packet has been received then we need to subtract
/// Jiansong: add reset and transferstart control 
always@(posedge clk)begin  
////        if(~trn_reof_d1_n & ~trn_rsrc_rdy_d1_n & CplD) //set the bit
        if (rst_reg | (~transferstart))
	         sub_calc <=  1'b0;
        else if(~trn_reof_d1_n & ~trn_rsrc_rdy_d1_n & CplD & ~Wait_for_TX_desc) //set the bit, fliter out TX des 
            sub_calc <=  1'b1;
        else if (sub_complete) //reset the bit
            sub_calc <=  1'b0;
end

//This state machine does the addtion and subtraction of the dmarxs_div8_now
//register
//
//Uses some multi-cycle paths:
//      addsub_state, dmarxs_div8_reg, dmarxs_div8_now  -> dmarxs_div8_reg_new
//are 2x multi-cycle paths
//The signal "stay_2x" ensures that the state variable
//addsub_state are static for at least two clock
//cycles when in the AS_IDLE state - of course
// dmarxs_div8_reg, dmarxs_div8_now signals must also be static for 2 cycles
// while in this state
always@(posedge clk)begin
     if(rst_reg | (~transferstart))begin
        dmarxs_div8_reg_new[9:0] <=  0;
        update_dmarxs_div8_reg <=  1'b0;
        add_complete <=  1'b0;
        sub_complete <=  1'b0;
        stay_2x <= 1'b0;
        addsub_state <=  AS_IDLE;
     end else begin
        case(addsub_state)
           AS_IDLE: begin
              update_dmarxs_div8_reg <=  1'b0;
              if(add_calc)begin
              //if add_calc is asserted then add the current value (*_now) to 
              //the incoming dma xfer size (*_reg)
                    dmarxs_div8_reg_new[9:0] <=  dmarxs_div8_now[9:0] 
                                               + dmarxs_div8_reg[9:0];
                    //make sure to stay in this state for two clock cycles
                    if(~stay_2x)begin
                       addsub_state <= AS_IDLE;
                       add_complete <= 1'b0;
                       update_dmarxs_div8_reg <= 1'b0;
                       stay_2x <= 1'b1;
                    //then update the current value (dmawxs_div8_now)
                    end else begin
                       addsub_state <= REGISTER_CALC;
                       add_complete <= 1'b1;//clear add_calc
                       update_dmarxs_div8_reg <= 1'b1;
                       stay_2x <= 1'b0;
                    end
              end else if (sub_calc)begin
              //if sub_calc is asserted then subtract the dw_length field
              //from the incoming completion packet from the current value
                    dmarxs_div8_reg_new[9:0] <=  dmarxs_div8_now[9:0] 
                                               - {1'b0, dw_length_d1[9:1]};
                    //likewise make sure to stat in this state for two clocks
                    if(~stay_2x)begin
                       addsub_state <= AS_IDLE;
                       sub_complete <= 1'b0;
                       update_dmarxs_div8_reg <= 1'b0;
                       stay_2x <= 1'b1;
                    //then update the current value (dmawxs_div8_now)
                    end else begin
                       addsub_state <= REGISTER_CALC;
                       sub_complete <= 1'b1;//clear sub_calc
                       update_dmarxs_div8_reg <= 1'b1;
                       stay_2x <= 1'b0;
                    end
              end else begin
                    dmarxs_div8_reg_new[9:0] <=  dmarxs_div8_now[9:0];
                    addsub_state <=  AS_IDLE;
                    sub_complete <=  1'b0;
                    add_complete <=  1'b0;
                    stay_2x <= 1'b0;
              end
           end
           REGISTER_CALC:begin 
              sub_complete <=  1'b0;
              add_complete <=  1'b0;
              addsub_state <= WAIT_FOR_REG;
              update_dmarxs_div8_reg <=  1'b1;
              stay_2x <= 1'b0;
            end
           WAIT_FOR_REG:begin 
              update_dmarxs_div8_reg <=  1'b0;
              stay_2x <= 1'b0;
              addsub_state <=  AS_IDLE;
           end
           default:begin
              dmarxs_div8_reg_new[9:0] <=  0;
              update_dmarxs_div8_reg <=  1'b0;
              add_complete <=  1'b0;
              sub_complete <=  1'b0;
              stay_2x <= 1'b0;
              addsub_state <=  AS_IDLE;
           end
        endcase
      end
end

//dmarxs_div8_now keeps the running total of the outstanding dma request
always@(posedge clk)begin
  if(rst_reg | (~transferstart))begin
      dmarxs_div8_now[9:0] <=  0;
  end else if(update_dmarxs_div8_reg)begin
      dmarxs_div8_now[9:0] <=  dmarxs_div8_reg_new[9:0];
  end
end  

/// Jiansong: why pipeline?
//when dmarxs_div8_now is zero then assert rd_dma_done_i
//need to pipeline rd_dma_done_i for 250 MHz timing
//reason: because it is fed-back into dmarxs_div8_now counter
//(via update_dmarxs_div8_reg)and creates too many levels of logic if not
//pipelined
always@(posedge clk)begin
        rd_dma_done_i <= (dmarxs_div8_now[9:0] == 0) ? 1'b1 :  1'b0;
end

//the rd_dma_done_early signal is asserted when the current count
//dmarxs_div8_now has decremented down to 176 DWORDS (or 704 bytes)
//the rd_dma_done_early signal is fedback to the dma_ctrl_wrapper so that
//it can execute another sub-transfer
//assign rd_dma_done_early = (dmarxs_div8_now[9:0] == 10'h0B0) ? 1'b1 :  1'b0;
// Jiansong: the size of compeleter could be 64bytes or 128bytes, "==" could be not satisfied
//assign rd_dma_done_early = (dmarxs_div8_now[9:0] <= 10'h0B0) ? 1'b1 :  1'b0;
assign rd_dma_done_early = (dmarxs_div8_now[9:0] == 10'h000) ? 1'b1 :  1'b0;

//rising edge detectors for rd_dma_done_i and rd_dma_done_early
rising_edge_detect rd_dma_done_one_inst(
                .clk(clk),
                .rst(rst),
                .in(rd_dma_done_i),
                .one_shot_out(rd_dma_done_one)
                );
rising_edge_detect rd_dma_done_early_one_inst(
                .clk(clk),
                .rst(rst),
                .in(rd_dma_done_early),
                .one_shot_out(rd_dma_done_early_one)
                );

/// Jiansong: parameter to tune
//If rd_dma_done_early comparison number is adjusted than this logic should be
//adjusted accordingly. For example, if the comparison is 10'h070 (or 448
//bytes) than the _early signal should only be used when the transfer size, 
//dmarxs, is larger than this.  In this particular case 1KB is the next larger
//transfer size.  Note that these choices could be fine
//tuned for higher-performance on a system-by-system basis   
always@(posedge clk)begin
   //Any transfer 1KB or larger and the *early signal is used - 
   //Also, always use the regular done signal if it is the very last transfer.
////   if(read_last || dmarxs[12:10] == 0)  ///Jiansong: bug in 1KB size
   if(read_last || dmarxs[12:11] == 0)
      rd_dma_done <= rd_dma_done_one;
   else  
      rd_dma_done <= rd_dma_done_early_one;
end

///// Jiansong: output to tx_sm
//always@(posedge clk)begin
//   np_rx_cnt_qw[9:0] <= dmarxs_div8_now[9:0];
//end
assign np_rx_cnt_qw[9:0] = dmarxs_div8_now[9:0];

endmodule


