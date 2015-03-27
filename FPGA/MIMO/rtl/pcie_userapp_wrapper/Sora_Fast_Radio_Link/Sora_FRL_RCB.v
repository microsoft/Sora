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
// Company: 		Microsoft Research Asia
// Engineer: 		Jiansong Zhang
// 
// Create Date:    21:54:12 04/26/2010 
// Design Name: 	 Sora_FRL_RCB
// Module Name:    Sora_FRL_RCB 
// Project Name: 
// Target Devices: Virtex5 LX50T
// Tool versions:  ISE10.1.03
// Description: 	top of Sora FRL module, this is an implementation of Sora fast radio link. Sora FRL is bi-directional that there're
//						both transmitter and reciver in this module. 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////

//More explanantion of IOs is available below

module Sora_FRL_RCB(	
			//////// Sora FRL interface to FPGA pins ////////
			// inputs
			input				CLK_I_p, //CLK2_p, 
			input				CLK_I_n, //CLK2_n,
			input	[3:0]		DATA_IN_p,
			input	[3:0]		DATA_IN_n, 
			input				MSG_IN_p,
			input				MSG_IN_n, 
			input				STATUS_IN_p,
			input				STATUS_IN_n,
			// outputs
			output 			CLK_O_p, //CLK1_p, 
			output			CLK_O_n, //CLK1_n,
			output [3:0]	DATA_OUT_p, 
			output [3:0]	DATA_OUT_n,								
			output			MSG_OUT_p,
			output			MSG_OUT_n,
			output			STATUS_OUT_p,
			output			STATUS_OUT_n,
			
			//////// signals to FPGA internal logic ////////
			// reset from FPGA fabric
			input				rst_in_internal,
			output			rst_out_internal,
			output			CLKDIV_R,
			// data input from internal logic
			input				SEND_EN,
			input	[31:0]	DATA_INT_IN,
			output			RDEN_DATA_INT_IN,
			// control message input from internal logic
			input [39:0]	MSG_INT_IN,
			input				EMPTY_MSG_INT_IN,
			output			RDEN_MSG_INT_IN,
			// data output to internal logic
			output [31:0]	DATA_INT_OUT,
			output reg		WREN_DATA_INT_OUT,
			// state message output to internal logic
			// modified by Jiansong, 2010-5-27, we do not use FIFO to buf MSG_INT_OUT
			output [31:0]	MSG_INT_OUT_Data,
			output [7:0]	MSG_INT_OUT_Addr,
			output			MSG_Valid,
			// debug info
			output reg [31:0]	TXMSG_MISS_cnt,
			output reg [31:0]	TXMSG_PASS_cnt,
			output			Radio_status_error,    // indicates error in received status signal from radio moudle
			output			Sora_FRL_linkup,
			output [1:0] 	LED
		);
		
	wire CLK_R; 
	
	wire ALMOSTEMPTY, ALMOSTFULL;

	wire [31:0] DATA;
	wire WREN;			
	wire [3:0] OSER_OQ;
	wire [31:0] DATA_IN;
	wire [7:0] MSG_IN;
	wire Lock_Status_TX;
	wire Lock_Status_RX;
	
	wire LED_CLOCK_0;
	wire LED_CLOCK_1;
	wire LED_CLOCK_2;
	wire LED_CLOCK_3;
	
	wire CLK180;
	
	wire [39:0] MSG;
	wire OSER_OQ_MSG,ack_r_one,nack_r_one,BACK_WRONG, BACK_RIGHT;
	
	wire STATUS_IN;
	wire STATUS_OUT;
	wire SEND_EN_TX;
	
	wire DATA_SEND_EN, MSG_WREN;
	
	// Status signal from adapter
	wire rst_in_status;	
	wire FIFO_FULL_RX;	
	wire TRAINING_DONE_RCB2Adapter;
	wire IDLE_RX;
	wire Status_RX_error;
		
	wire STATUS_OUT_MSG_buf;
	wire msg_r_p_one;
	wire DATA_ALMOSTEMPTY;
		
	// Status signal to adapter
	wire TRAINING_DONE_Adapter2RCB;

/////////////////// reset logic /////////////////

	wire RST;			
	// FRL local logic reset
	reg [31:0] 	rst_counter;
	reg			reset_frl_local;
	initial 
		rst_counter = 32'h0000_0000;
	always @ (posedge CLKDIV_R) begin
		// a local reset signal is triggered 
		if (rst_in_internal | rst_in_status) begin
			rst_counter			<= 32'h0000_0000;
			reset_frl_local	<= 1'b1;
		end else if(rst_counter < 32'h0000_1130) begin		// 100us
			rst_counter 		<= rst_counter + 32'h0000_0001;
			reset_frl_local	<= 1'b1;
		end else
			reset_frl_local	<= 1'b0;	
	end
	
	// propagate reset signal from status line to RCB internal logic
	assign rst_out_internal	= rst_in_status;
	
	assign RST = rst_in_internal | rst_in_status | reset_frl_local;	
	
//////////////////////////////////////////////////

	assign Sora_FRL_linkup = TRAINING_DONE_Adapter2RCB & TRAINING_DONE_RCB2Adapter;		

	assign DATA_SEND_EN 	=  ~FIFO_FULL_RX & SEND_EN & TRAINING_DONE_RCB2Adapter; // modified by Jiansong, 2010-5-25, add training done as condition
	always@(posedge CLKDIV_R)	WREN_DATA_INT_OUT	<= ~DATA_ALMOSTEMPTY;
	
	
// Determine whether TX will send DATA or MSG.
	assign LED[0] = LED_CLOCK_0;
	assign LED[1] = LED_CLOCK_1;
	
											
	Sora_FRL_STATUS_decoder Sora_FRL_STATUS_decoder_RCB_inst(
		.clk						(CLKDIV_R),
//		.rst						(rst_in_internal),		// no need for a reset signal since there's no internal state
		
		// input from STATUS line
		.status_in				(STATUS_IN),
		
		// four output states
		.RST_state				(rst_in_status),
		.TRAININGDONE_state	(TRAINING_DONE_RCB2Adapter),
		.IDLE_state				(IDLE_RX),				// this signal is used for monitoring
		.FIFOFULL_state		(FIFO_FULL_RX),
		// error state
		.error_state			(Status_RX_error)
	);
	
	Sora_FRL_STATUS_encoder Sora_FRL_STATUS_encoder_RCB_inst(
		.clk						(CLKDIV_R),
//		.rst						(rst_in_internal),	// reset the Status encoder's state machine
		
		// input from STATUS line
		.status_out				(STATUS_OUT),
		
		// four output states
//		.RST_signal				(rst_in_internal | reset_frl_local),	// this signal is generated from internal fabric
		.RST_signal				(rst_in_internal),	// this signal is generated from internal fabric
		.TRAININGDONE_signal	(TRAINING_DONE_Adapter2RCB),
		.IDLE_signal			(rst_in_status),	// this signal informs Status encoder to change its state to IDLE, 
													// 	it happens when an RST state is received from Status line
		.FIFOFULL_signal		(1'b0)		// FIFO should not be full on RX path
	);

	
	RCB_FRL_TX Sora_FRL_RCB_TX_data_inst 	( 						
		.RST				(RST), 
		.CLK				(CLK_R),
		.CLKDIV			(CLKDIV_R),
		
		.DATA_IN			(DATA_INT_IN),
		.OSER_OQ			(OSER_OQ),
		.SEND_EN			(DATA_SEND_EN),
		.TRAINING_DONE	(TRAINING_DONE_RCB2Adapter),
		.RDEN				(RDEN_DATA_INT_IN)
	);
		
	// debug info
	wire	TXMSG_MISS;
	wire	TXMSG_PASS;
	always@(posedge CLKDIV_R)begin
		if(RST)begin
			TXMSG_MISS_cnt	<= 32'h0000_0000;
			TXMSG_PASS_cnt	<= 32'h0000_0000;
		end else if(TXMSG_MISS) begin
			TXMSG_MISS_cnt	<= TXMSG_MISS_cnt + 32'h0000_0001;
			TXMSG_PASS_cnt	<= TXMSG_PASS_cnt;
		end else if(TXMSG_PASS) begin
			TXMSG_MISS_cnt	<= TXMSG_MISS_cnt;
			TXMSG_PASS_cnt	<= TXMSG_PASS_cnt + 32'h0000_0001;
		end else begin
			TXMSG_MISS_cnt	<= TXMSG_MISS_cnt;
			TXMSG_PASS_cnt	<= TXMSG_PASS_cnt;			
		end
	end
									
	RCB_FRL_TX_MSG Sora_FRL_RCB_TX_MSG_inst ( 
		.rst				(RST), 
		.clk				(CLK_R), 
		.clkdiv			(CLKDIV_R),
		
		.data_in			(MSG_INT_IN[39:0]),
		.empty			(EMPTY_MSG_INT_IN),
		.rden				(RDEN_MSG_INT_IN),
		.OSER_OQ			(OSER_OQ_MSG), 
		.ack_r_one		(ack_r_one), 		//input feedback from decode module
		.nack_r_one		(nack_r_one), 
		.txmsg_miss_one(TXMSG_MISS), 	// one cycle signal to indicate a TXMSG wasn't correctly received by another Sora-FRL end
		.txmsg_pass_one(TXMSG_PASS),		// one cycle signal to indicate a TXMSG was correctly received by another Sora-FRL end
		.msg_r_p_one	(msg_r_p_one),
		.msg_r_n_one	(msg_r_n_one),
		.training_done	(TRAINING_DONE_RCB2Adapter)
	);


	// serial clock output to the 'clock' LVDS channel
//	wire TX_CLOCK_PREBUF;
//	ODDR #( .DDR_CLK_EDGE("OPPOSITE_EDGE"), .INIT(1'b0), .SRTYPE("ASYNC") ) 
//		ODDR_TX_CLOCK ( .Q(TX_CLOCK_PREBUF), .C(CLK_R), .CE(1'b1), .D1(1'b1), .D2(1'b0), .R(1'b0), .S(1'b0) );	
	OBUFDS OBUFDS_clock ( .I(CLK_R), .O(CLK_O_p), .OB(CLK_O_n) );

	// differential outputs to LVDS channels
	OBUFDS OBUFDS_data0 ( .I(OSER_OQ[0]), .O(DATA_OUT_p[0]), .OB(DATA_OUT_n[0]) );
	OBUFDS OBUFDS_data1 ( .I(OSER_OQ[1]), .O(DATA_OUT_p[1]), .OB(DATA_OUT_n[1]) );			
	OBUFDS OBUFDS_data2 ( .I(OSER_OQ[2]), .O(DATA_OUT_p[2]), .OB(DATA_OUT_n[2]) );			
	OBUFDS OBUFDS_data3 ( .I(OSER_OQ[3]), .O(DATA_OUT_p[3]), .OB(DATA_OUT_n[3]) );				
	OBUFDS OBUFDS_msg ( .I(OSER_OQ_MSG), .O(MSG_OUT_p), .OB(MSG_OUT_n) );
	OBUFDS OBUFDS_status ( .I(STATUS_OUT), .O(STATUS_OUT_p), .OB(STATUS_OUT_n) );
	
	// differential input from the 'status' LVDS channel 		
	IBUFDS #( .DIFF_TERM("FALSE") ) IBUFDS_inst7 ( .I(STATUS_IN_p), .IB(STATUS_IN_n), .O(STATUS_IN)	);

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// clock ///////////////////////////////////////////
// this is a candidate solution, but not used finally because DCM is not stable in this solution:
// the clock source of Sora FRL comes from RAB, that is, 
// (1) RAB generates LVDS clock from its local oscillator,	
// (2) a serial-rate clock is transmitted together with data channels
// (3) RCB receives the serial-rate clock, using a DCM to generate both serial-rate clock and parallel-rate clock
//     the two clocks should be phase synchronized in this way
// (4) both serial-clock and parallel clock are propagated to Iserdes and Oserdes using BUFG
// note: it is not the optimal clock solution since global clock net introduce larger skew than reginal clock net,
//       but with current PCB layout, reginal clock solution is not able to be applied since some LVDS channels 
//       on a same LVDS port are routed to different IO banks (different clcok regions), which should be routed to 
//       the same IO bank if we use regional clock net (BUFIO can only reach to a single clock region, BUFR can reach
//       to its own and two neighbor clock regions)

// differential input from the 'clock' LVDS channel
//	wire CLOCK_RX_BUF;
//	wire CLOCK_RX_ISERDES_OUT;
//	IBUFDS #( .DIFF_TERM("FALSE") ) IBUFDS_inst8 ( .I(CLK_I_p), .IB(CLK_I_n), .O(CLOCK_RX_BUF) );

//	//IDELAY IN CLOCK PATH
//	IODELAY #( .IDELAY_TYPE("FIXED"), .IDELAY_VALUE(0), .ODELAY_VALUE(0), .REFCLK_FREQUENCY(200.00), .HIGH_PERFORMANCE_MODE("TRUE") ) 
//		IODELAY_CLOCK_RX ( .DATAOUT(CLOCK_RX_ISERDES_OUT), .IDATAIN(CLOCK_RX_BUF), .ODATAIN(1'b0), .DATAIN(1'b0),	.T(),
//								 .CE(1'b0), .INC(1'b0), .C(1'b0), .RST(RST) );

//	BUFG BUFG_input ( .I(CLOCK_RX_ISERDES_OUT), .O(clock_input_global) );
//	BUFG BUFG_input ( .I(CLOCK_RX_BUF), .O(clock_input_global) );
//	
//	DCM_FRL DCM_FRL_inst (
//		.CLKIN_IN	(clock_input_global), 
//		.RST_IN		(rst_in_internal), 
//		.CLKDV_OUT	(CLKDIV_R), 
//		.CLK0_OUT	(CLK_R), 
//		.LOCKED_OUT	()
//    );


// clock solution, it seems serial-clock and parallel clock are not synchronized in phase, the highest frequency will be limited therefore
	wire CLK_R1, CLK_R_pre;
//	IBUFDS #( .DIFF_TERM("FALSE") ) IBUFDS_inst8 ( .I(CLK_I_p), .IB(CLK_I_n), .O(CLK_R) );
	IBUFDS #( .DIFF_TERM("FALSE") ) IBUFDS_inst8 ( .I(CLK_I_p), .IB(CLK_I_n), .O(CLK_R_pre) );
	
//	BUFR #( .BUFR_DIVIDE("1"), .SIM_DEVICE("VIRTEX5") )
//		BUFIO_CLK_R ( .O(CLK_R1), .CE(1'b1), .CLR(1'b0), .I(CLK_R_pre) );
//	BUFG BUFG_CLK_R (.I(CLK_R1), .O(CLK_R));
	BUFG BUFG_CLK_R (.I(CLK_R_pre), .O(CLK_R));
	
	wire CLKDIV_R1;
	BUFR #( .BUFR_DIVIDE("4"), .SIM_DEVICE("VIRTEX5") ) 
//		BUFR_inst1( .O(CLKDIV_R1), .CE(1'b1), .CLR(1'b0), .I(CLK_R) );
		BUFR_inst1( .O(CLKDIV_R1), .CE(1'b1), .CLR(1'b0), .I(CLK_R_pre) );	
	BUFG BUFG_CLKDIV_R (.I(CLKDIV_R1), .O(CLKDIV_R));

//////////////////////////////// clock ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
		
	RCB_FRL_DDR_8TO1_16CHAN_RX Sora_FRL_RCB_RX_ISerDes_and_Training_inst (
		.RXCLK					(CLK_R),
		.RXCLKDIV				(CLKDIV_R),
		.RESET					(RST),
		.IDLY_RESET				(RST),
		
		.DATA_RX_P				({DATA_IN_p,MSG_IN_p}),
		.DATA_RX_N				({DATA_IN_n,MSG_IN_n}),
		.INC_PAD					(1'b0),
		.DEC_PAD					(1'b0),
		.DATA_FROM_ISERDES	({DATA_IN[31:0],MSG_IN[7:0]}),
		.BITSLIP_PAD			(1'b0),
		.TAP_00					(),
		.TAP_01					(),
		.TAP_02					(),
		.TAP_03					(),
		.TAP_04					(),
		.TAP_CLK					(),
		.TRAINING_DONE			(TRAINING_DONE_Adapter2RCB)
	);
	

	RCB_FRL_RX Sora_FRL_RCB_RX_data_inst (
		.CLKDIV			(CLKDIV_R),  
//		.RDCLK			(CLKDIV_R), 
		.RST				(RST), 
		
		.DATA_IN			(DATA_IN), 
		.DATA_OUT		(DATA_INT_OUT),
		.RDEN				(~DATA_ALMOSTEMPTY), 
		.ALMOSTEMPTY	(DATA_ALMOSTEMPTY)
//		.fifo_WREN		(fifo_WREN)
	);
				
	assign MSG_Valid = msg_r_p_one;				// added by jiansong, 2010-5-27
	RCB_FRL_RX_MSG Sora_FRL_RCB_RX_MSG_inst (
		.CLKDIV		(CLKDIV_R),
		.RST			(RST),
		
		.DATA_IN			(MSG_IN[7:0]),
		.DATA_OUT		({MSG_INT_OUT_Addr[7:0],MSG_INT_OUT_Data[31:0]}),
		.msg_r_n_one	(msg_r_n_one),
		.msg_r_p_one	(msg_r_p_one),
		.nack_r_one		(nack_r_one),
		.ack_r_one		(ack_r_one)
	);//these are for the radio board
					
		RCB_FRL_LED_Clock RCB_FRL_LED_Clock_inst (
			.Test_Clock_in(CLK_R),
			.LED_Clock_out(LED_CLOCK_0),
			.RST(RST)
		);
						
		RCB_FRL_LED_Clock_DIV RCB_FRL_LED_Clock_DIV_inst (
			.Test_Clock_in(CLKDIV_R),
			.LED_Clock_out(LED_CLOCK_1),
			.RST(RST)
		);
	
endmodule
