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
// Module Name:    rx_trn_data_fsm 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: Receive TRN Data FSM. This module interfaces to the Block Plus RX 
// TRN. It presents the 64-bit data from completer and and forwards that
// data with a data_valid signal.  This block also decodes packet header info
// and forwards it to the rx_trn_monitor block. 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`timescale 1ns / 1ps

module rx_trn_data_fsm(
	input  wire         clk,
	input  wire         rst,
	// Rx Local-Link
	input wire [63:0] trn_rd,
	input wire [7:0] trn_rrem_n,
	input wire trn_rsof_n,
	input wire trn_reof_n,
	input wire trn_rsrc_rdy_n,
	input wire trn_rsrc_dsc_n,
	output reg trn_rdst_rdy_n,
	input wire trn_rerrfwd_n,
	output wire trn_rnp_ok_n,
	input wire [6:0] trn_rbar_hit_n,
	input wire [11:0] trn_rfc_npd_av,
	input wire [7:0] trn_rfc_nph_av,
	input wire [11:0] trn_rfc_pd_av,
	input wire [7:0] trn_rfc_ph_av,
	input wire [11:0] trn_rfc_cpld_av,
	input wire [7:0] trn_rfc_cplh_av,
	output wire trn_rcpl_streaming_n, 
	//DATA FIFO SIGNALS
	output reg   [63:0] data_out,
	output wire [7:0]    data_out_be,
	output reg          data_valid,
	input  wire         data_fifo_status,
	//END DATA FIFO SIGNALS

	//HEADER FIELD SIGNALS
	//The following are registered from the header fields of the current packet
	//See the PCIe Base Specification for definitions of these headers
	output reg fourdw_n_threedw, //fourdw = 1'b1; 3dw = 1'b0;
	output reg payload,
	output reg [2:0] tc, //traffic class
	output reg td, //digest
	output reg ep,  //poisoned bit
	output reg [1:0] attr, //attribute field
	output reg [9:0] dw_length, //DWORD Length
	//the following fields are dependent on the type of TLP being received
	//regs with MEM prefix are valid for memory TLPS and regs with CMP prefix
	//are valid for completion TLPS
	output reg [15:0] MEM_req_id, //requester ID for memory TLPs
	output reg [7:0] MEM_tag, //tag for non-posted memory read request
	output reg [15:0]   CMP_comp_id, //completer id for completion TLPs
	output reg [2:0]CMP_compl_stat, //status for completion TLPs
	output reg  CMP_bcm, //byte count modified field for completions TLPs
	output reg [11:0] CMP_byte_count, //remaining byte count for completion TLPs
	output reg [63:0] MEM_addr,  //address field for memory TLPs
	output reg [15:0] CMP_req_id, //requester if for completions TLPs
	output reg [7:0] CMP_tag, //tag field for completion TLPs
	output reg [6:0] CMP_lower_addr, //lower address field for completion TLPs
	//decode of the format field
	output wire    MRd, //Mem read
	output wire    MWr, //Mem write
	output wire    CplD, //Completion w/ data
	output wire    Msg, //Message TLP
	output wire    UR, //Unsupported request TLP i.e. IO, CPL,etc..

	output reg [6:0] bar_hit, //valid when a BAR is hit
	output reg header_fields_valid//valid signal to qualify the above header fields
	//END HEADER FIELD SIGNALS

);

//state machine states
localparam IDLE         = 3'b000;
localparam NOT_READY    = 3'b001;
localparam SOF          = 3'b010;
localparam HEAD2        = 3'b011;
localparam BODY         = 3'b100;
localparam EOF          = 3'b101;

//additional pipelines regs for RX TRN interface
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
//second pipeline
reg [63:0] trn_rd_d2;
reg [7:0] trn_rrem_d2_n;
reg trn_rsof_d2_n;
reg trn_reof_d2_n;
reg trn_rsrc_rdy_d2_n;
reg trn_rsrc_dsc_d2_n;
reg trn_rerrfwd_d2_n;
reg [6:0] trn_rbar_hit_d2_n;
reg [11:0] trn_rfc_npd_av_d2;
reg [7:0] trn_rfc_nph_av_d2;
reg [11:0] trn_rfc_pd_av_d2;
reg [7:0] trn_rfc_ph_av_d2;
reg [11:0] trn_rfc_cpld_av_d2;
reg [7:0] trn_rfc_cplh_av_d2;



reg [4:0] rx_packet_type;
reg   [2:0] trn_state;
wire [63:0] data_out_mux;
wire [7:0] data_out_be_mux;
reg       data_valid_early;

reg   rst_reg;
always@(posedge clk) rst_reg <= rst;


// TIE constant signals here
assign trn_rnp_ok_n = 1'b0; 
assign trn_rcpl_streaming_n = 1'b0; //use completion streaming mode


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
    trn_rd_d2[63:0]          <= trn_rd_d1[63:0]         ;
    trn_rrem_d2_n[7:0]       <= trn_rrem_d1_n[7:0]      ;
    trn_rsof_d2_n            <= trn_rsof_d1_n           ;
    trn_reof_d2_n            <= trn_reof_d1_n           ;
    trn_rsrc_rdy_d2_n        <= trn_rsrc_rdy_d1_n       ;
    trn_rsrc_dsc_d2_n        <= trn_rsrc_dsc_d1_n       ;
    trn_rerrfwd_d2_n         <= trn_rerrfwd_d1_n        ;
    trn_rbar_hit_d2_n[6:0]   <= trn_rbar_hit_d1_n[6:0]  ;
    trn_rfc_npd_av_d2[11:0]  <= trn_rfc_npd_av_d1[11:0] ;
    trn_rfc_nph_av_d2[7:0]   <= trn_rfc_nph_av_d1[7:0]  ;
    trn_rfc_pd_av_d2[11:0]   <= trn_rfc_pd_av_d1[11:0]  ;
    trn_rfc_ph_av_d2[7:0]    <= trn_rfc_ph_av_d1[7:0]   ;
    trn_rfc_cpld_av_d2[11:0] <= trn_rfc_cpld_av_d1[11:0];
    trn_rfc_cplh_av_d2[7:0]  <= trn_rfc_cplh_av_d1[7:0] ;   
end


assign    rx_sof_d1 = ~trn_rsof_d1_n & ~trn_rsrc_rdy_d1_n;

// Assign packet type information about the current RX Packet
// rx_packet_type is decoded in always block directly below these assigns
assign    MRd = rx_packet_type[4];
assign    MWr = rx_packet_type[3];
assign    CplD = rx_packet_type[2];
assign    Msg  = rx_packet_type[1];
assign    UR   = rx_packet_type[0];

//register the packet header fields and decode the packet type
//both memory and completion TLP header fields are registered for each
//received packet, however, only the fields for the incoming type will be 
//valid
always@(posedge clk )
begin
  if(rst_reg)begin
    rx_packet_type[4:0] <= 5'b00000;
    fourdw_n_threedw <= 0;
    payload <= 0;
    tc[2:0] <= 0; //traffic class
    td <= 0; //digest
    ep <= 0;  //poisoned bit
    attr[1:0] <= 0;
    dw_length[9:0] <= 0;
    MEM_req_id[15:0] <= 0;
    MEM_tag[7:0] <= 0;
    CMP_comp_id[15:0] <= 0;
    CMP_compl_stat[2:0] <= 0;
    CMP_bcm <= 0;
    CMP_byte_count[11:0] <= 0;
  end else begin
    if(rx_sof_d1)begin
       //these fields same for all TLPs
       fourdw_n_threedw <= trn_rd_d1[61];
       payload <= trn_rd_d1[62];
       tc[2:0] <= trn_rd_d1[54:52]; //traffic class
       td <= trn_rd_d1[47]; //digest
       ep <= trn_rd_d1[46];  //poisoned bit
       attr[1:0] <= trn_rd_d1[45:44];
       dw_length[9:0] <= trn_rd_d1[41:32];
       //also latch bar_hit
       bar_hit[6:0] <= ~trn_rbar_hit_d1_n[6:0];
       //these following fields dependent on packet type
       //i.e. memory packet fields are only valid for mem packet types
       //and completer packet fields are only valid for completer packet type;
       //memory packet fields
       MEM_req_id[15:0] <= trn_rd_d1[31:16];
       MEM_tag[7:0] <= trn_rd_d1[15:8];
       //first and last byte enables not needed because plus core delivers
           
       //completer packet fields    
       CMP_comp_id[15:0] <= trn_rd_d1[31:16];
       CMP_compl_stat[2:0] <= trn_rd_d1[15:13];
       CMP_bcm <= trn_rd_d1[12];
       CMP_byte_count[11:0] <= trn_rd_d1[11:0];
       
       //add message fields here if needed 
       
      //decode the packet type and register in rx_packet_type         
      casex({trn_rd_d1[62],trn_rd_d1[60:56]})
       6'b000000: begin //mem read
             rx_packet_type[4:0] <= 5'b10000;
       end
       6'b100000: begin //mem write
             rx_packet_type[4:0] <= 5'b01000;
       end
       6'b101010: begin //completer with data
             rx_packet_type[4:0] <= 5'b00100;
       end    
       6'bx10xxx: begin //message
             rx_packet_type[4:0] <= 5'b00010;
       end
       default: begin  //all other packet types are unsupported for this design
             rx_packet_type[4:0] <= 5'b00001;
       end
     endcase
    end 
  end
end

// Now do the same for the second header of the current packet
always@(posedge clk )begin
  if(rst_reg)begin
       MEM_addr[63:0] <= 0;  
       CMP_req_id[15:0] <= 0;
       CMP_tag[7:0] <= 0;
       CMP_lower_addr[6:0] <= 0;
  end else begin
     if(trn_state == SOF &  ~trn_rsrc_rdy_d1_n)begin //packet is in process of
                                                     //reading out second header
       if(fourdw_n_threedw)
          MEM_addr[63:0] <= trn_rd_d1[63:0];
       else
          MEM_addr[63:0] <= {32'h00000000,trn_rd_d1[63:32]};
          
       CMP_req_id[15:0] <= trn_rd_d1[63:48];
       CMP_tag[7:0] <= trn_rd_d1[47:40];
       CMP_lower_addr[6:0] <= trn_rd_d1[48:32];
     end
  end
end

// generate a valid signal for the headers field
always@(posedge clk)begin
   if(rst_reg)
     header_fields_valid <= 0;
    else 
     header_fields_valid <= ~trn_rsrc_rdy_d2_n & trn_rsof_d1_n;
end


//This state machine keeps track of what state the RX TRN interface 
//is currently in
always @ (posedge clk )
begin
  if(rst_reg)
  begin
    trn_state <= IDLE;
    trn_rdst_rdy_n <= 1'b0;
  end
  else
  begin
    case(trn_state)
    IDLE:  begin
       trn_rdst_rdy_n <= 1'b0;
       if(rx_sof_d1)
          trn_state <= SOF;
       else
          trn_state <= IDLE;
    end 
	 /// Jiansong: notice, completion streaming here
    NOT_READY:  begin  // This state is a placeholder only - it is currently not
                       // entered from any other state
                       // This state could be used for throttling the PCIe
                       // Endpoint Block Plus RX TRN interface, however, this
                       // should not be done when using completion streaming
                       // mode as this reference design does
       trn_rdst_rdy_n <= 1'b1;
       trn_state <= IDLE;
    end
    SOF:  begin
       if(~trn_reof_d1_n & ~trn_rsrc_rdy_d1_n)
          trn_state <= EOF;
       else if(trn_reof_d1_n & ~trn_rsrc_rdy_d1_n)
          trn_state <= HEAD2;
       else
          trn_state <= SOF;
    end
    HEAD2:  begin  
       if(~trn_reof_d1_n & ~trn_rsrc_rdy_d1_n)
          trn_state <= EOF;
       else if(trn_reof_d1_n & ~trn_rsrc_rdy_d1_n)
          trn_state <= BODY;
       else
          trn_state <= HEAD2;
    end
    BODY:  begin
       if(~trn_reof_d1_n & ~trn_rsrc_rdy_d1_n)
          trn_state <= EOF;
       else
          trn_state <= BODY;
    end
    EOF:  begin
       if(~trn_rsof_d1_n & ~trn_rsrc_rdy_d1_n)
          trn_state <= SOF;
       else if(trn_rsof_d1_n & trn_rsrc_rdy_d1_n)
          trn_state <= IDLE;
       else if(~trn_reof_d1_n & ~trn_rsrc_rdy_d1_n)
          trn_state <= EOF;
       else
          trn_state <= IDLE;
    end
   default:  begin
       trn_state <= IDLE;
    end
    endcase
  end
end


//data shifter logic
//need to shift the data depending if we receive a four DWORD or three DWORD
//TLP type - Note that completion packets will always be 3DW TLPs 
    assign data_out_mux[63:0] = (fourdw_n_threedw) 
                                ? trn_rd_d2[63:0] 
                                : {trn_rd_d2[31:0],trn_rd_d1[63:32]};

/// Jiansong: notice, why? 64bit data? likely should be modified
//swap the byte ordering to little endian
//e.g. data_out = B7,B6,B5,B4,B3,B2,B1,B0                               
    always@(posedge clk)
       data_out[63:0] <= {data_out_mux[7:0],data_out_mux[15:8],
                          data_out_mux[23:16],data_out_mux[31:24],
                          data_out_mux[39:32],data_out_mux[47:40],
                          data_out_mux[55:48],data_out_mux[63:56]};

//Data byte enable logic:
//Need to add byte enable logic for incoming memory transactions if desired
//to allow memory transaction granularity smaller than DWORD.
//
//This design always requests data on 128 byte boundaries so for
//completion TLPs the byte enables would always be asserted  
//
//Note that the endpoint block plus uses negative logic, however,
//I decided to use positive logic for the user application. 
      assign  data_out_be = 8'hff;

//data_valid generation logic
//Generally, data_valid should be asserted the same amount of cycles 
//that trn_rsrc_rdy_n is asserted (minus the cycles that sof and
//eof are asserted).  
//There are two exceptions to this:  
//   - 3DW TLPs with odd number of DW without Digest
//     In this case an extra cycle is required 
//               - eof is used to generate this extra cycle
//   - 4DW TLPs with even number of DW with Digest
//     In this case an extra cycle needs to be removed 
//               - the last cycle is removed

// Jiansong: fix Mrd data to fifo bug
    always@(*)begin
       case({fourdw_n_threedw, dw_length[0], td})
          3'b010: data_valid_early =   ~trn_rsrc_rdy_d2_n 
                                     &  trn_rsof_d2_n 
                                     & ~trn_reof_d2_n
												 & payload;
          3'b101: data_valid_early =   ~trn_rsrc_rdy_d2_n 
                                     &  trn_reof_d1_n
												 & payload;
          default: data_valid_early =  ~trn_rsrc_rdy_d2_n 
                                     &  trn_rsof_d2_n 
                                     &  trn_reof_d2_n
												 & payload;
       endcase
    end

//delay by one clock to match data_out (and presumably data_out_be)
    always@(posedge clk)
      if(rst_reg)
       data_valid <= 1'b0;
      else
       data_valid <= data_valid_early;

endmodule

