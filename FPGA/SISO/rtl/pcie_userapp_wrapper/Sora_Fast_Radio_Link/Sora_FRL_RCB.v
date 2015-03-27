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

module Sora_FRL_RCB(	/*CLK_ext_T_p,
								CLK_ext_T_n,*/ //used for external clock
			// signals to FPGA pins
			output 			CLK_O_p, //CLK1_p, 
			output			CLK_O_n, //CLK1_n, 
			input				CLK_I_p, //CLK2_p, 
			input				CLK_I_n, //CLK2_n,
			input	[3:0]		DATA_IN_p,
			input	[3:0]		DATA_IN_n, 
			output [3:0]	DATA_OUT_p, 
			output [3:0]	DATA_OUT_n,								
			output			MSG_OUT_p,
			output			MSG_OUT_n,
			input				MSG_IN_p,
			input				MSG_IN_n,
			input				STATUS_IN_p,
			input				STATUS_IN_n,
			output			STATUS_OUT_p,
			output			STATUS_OUT_n,
			
			// signals to internal modules
			input				RST_internal,
			input				SEND_EN,
			input				SEND_EN_MSG,
			input	[7:0]		DATA_INT_IN_3,
			input	[7:0]		DATA_INT_IN_2,
			input	[7:0]		DATA_INT_IN_1,
			input	[7:0]		DATA_INT_IN_0,
			output			CLKRD_INT_DATA_IN,
			output			RDEN_DATA_INT_IN,
			output [31:0]	DATA_INT_OUT,
			output			CLKWR_INT_DATA_OUT,
			output			WREN_DATA_INT_OUT,
			output			DATA_ALMOSTEMPTY,
			input	[39:0]	MSG_INT_IN,
			input				CLKWR_INT_MSG_IN,
			input				WREN_MSG_INT_IN,
			// modified by Jiansong, 2010-5-27, we do not use FIFO to buf MSG_INT_OUT
			output [31:0]	MSG_INT_OUT_Data,
			output [7:0]	MSG_INT_OUT_Addr,
			output			MSG_Valid,
			output			STATUS_INT_OUT_CORRECT,
			output			STATUS_INT_OUT_WRONG,
			input				TxInit,
			output			TxDone,
			output			Radio_status_error,    // indicates error in received status signal from radio moudle
			output			Sora_FRL_linkup,
			output [1:0] 	LED
		);

	
/////////////////////////////////////////////////////////////////////////////
// wire anouncement
	
	wire CRT;
	wire CRT1;
	wire CRT_n;	
	wire CLK, /*CLKDIV,*/ CLK_R, CLKDIV_R1, CLKDIV_R;
	
	wire [31:0] DATA_CHK;
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
	
	wire [39:0] MSG;//, MSG_INT_OUT, MSG_INT_IN;
	wire OSER_OQ_MSG,CON_P,CON_N,BACK_WRONG, BACK_RIGHT;
	
	wire STATUS_IN;
	wire SEND_EN_TX;
	
	wire DATA_SEND_EN, MSG_ALMOSTFULL, MSG_WREN;
	
	wire FIFO_FULL_RX,TRAINING_DONE_RX;
	
	wire STATUS_OUT_MSG_buf;
	wire EN_SIG;
	
	reg TxDone_reg;
	reg TxingFlag;
	
	wire TRAINING_DONE_TX;
	
	wire RX_RST;
	wire RST;
	
	assign Sora_FRL_linkup = TRAINING_DONE_TX & TRAINING_DONE_RX;
	
	// Added by Jiansong, 2010-6-9, power on reset
	reg [31:0] 	poweron_rst_counter;
	reg			poweron_reset;
	always @ (posedge CLKDIV_R) begin
		// added by Jiansong, 2010-5-24, an explicit reset is necessary for simulation
		if (RST_internal) begin
			poweron_rst_counter	<= 32'h0000_0000;
			poweron_reset			<= 1'b1;
		end else if(poweron_rst_counter < 32'h0000_1130) begin		// 100us
			poweron_rst_counter <= poweron_rst_counter + 1;
			poweron_reset <= 1;
		end else
			poweron_reset <= 0;	
	end
	
	assign RST = RST_internal | RX_RST | poweron_reset;
	
	
/////////////////////////////////////////////////////////////////////	
	
	
	assign TxDone = TxDone_reg;

	assign DATA_SEND_EN 	=  ~FIFO_FULL_RX & SEND_EN & TRAINING_DONE_RX; // modified by Jiansong, 2010-5-25, add training done as condition
	assign WREN_DATA_INT_OUT   =  ~DATA_ALMOSTEMPTY;
	assign CLKRD_INT_DATA_IN 	=  CLKDIV_R;
	assign CLKWR_INT_DATA_OUT  =  CLKDIV_R;
	
	
// Determine whether TX will send DATA or MSG.
	wire [3:0] ERR0,ERR1,ERR2,ERR3,ERR_MSG;
	assign LED[0] = LED_CLOCK_0;
	assign LED[1] = LED_CLOCK_1;

	always @ (posedge CLKDIV_R) begin
		if (RST == 1'b1) begin
			TxDone_reg <= 1'b0;
			TxingFlag <= 1'b0;
		end
		else begin
			if (TxInit) begin
				if (TxingFlag) begin
					if (SEND_EN == 1'b0) begin
						TxDone_reg <= 1'b1;
						TxingFlag <= 1'b0;
					end
				end
				else begin
					if (SEND_EN == 1'b1) begin
						TxDone_reg <= 1'b0;
						TxingFlag <= 1'b1;
					end
				end				
			end
			else begin
				TxDone_reg <= 1'b0;
				TxingFlag <= 1'b0;
			end
		end
	end


	RCB_FRL_STATUS_IN RCB_FRL_STATUS_IN_inst(	
				.CLK(CLKDIV_R),
				.MODULE_RST(RST_internal),
				
				.RESET(RX_RST),				//output indicating reset state
				.FIFO_FULL(FIFO_FULL_RX),			//output indicating full state
				.TRAINING_DONE(TRAINING_DONE_RX),		//output indicating done state
				.STATUS(STATUS_IN),
				.status_error(Radio_status_error)
				);
								
	RCB_FRL_STATUS_OUT RCB_FRL_STATUS_OUT_inst(	
				.CLK(CLKDIV_R),
				.MODULE_RST(RST_internal),
				
				.RESET(RST & ~RX_RST),				//output indicating reset state
				.FIFO_FULL(1'b0),			//output indicating full state
				.TRAINING_DONE(TRAINING_DONE_TX),		//output indicating done state
				.STATUS(STATUS_OUT),
				.INT_SAT()
				);

	
	RCB_FRL_TX RCB_FRL_TX_inst 	( 						
					.RST(RST), 
					.CLK(CLK_R),
					.CLKDIV(CLKDIV_R),
					
					.DATA_IN({DATA_INT_IN_3, DATA_INT_IN_2, DATA_INT_IN_1, DATA_INT_IN_0}),
					.OSER_OQ(OSER_OQ),
					.SEND_EN(DATA_SEND_EN),
					.TRAINING_DONE(TRAINING_DONE_RX),
					.RDEN(RDEN_DATA_INT_IN)
					);
	
									
	RCB_FRL_TX_MSG RCB_FRL_TX_MSG_inst ( 
						.RST(RST), 
						.CLK(CLK_R), 
						.CLKDIV(CLKDIV_R),
						
						.DATA_IN(MSG_INT_IN), 
						.OSER_OQ(OSER_OQ_MSG), 
						.CLKWR(CLKWR_INT_MSG_IN),
						.WREN(WREN_MSG_INT_IN), 
						.CON_P(CON_P), 									//input feedback from decode module
						.CON_N(CON_N), 
						.BACK_WRONG(STATUS_INT_OUT_CORRECT), 		//feedback output to internal module
						.BACK_RIGHT(STATUS_INT_OUT_WRONG),
						.ALMOSTFULL(MSG_ALMOSTFULL),
						.EN_SIG(EN_SIG),
						.RE_SIG(RE_SIG),
						.EMPTY(),
						.probe(),
						.TRAINING_DONE(TRAINING_DONE_RX)
						);
										 



	OBUFDS //#(
//			.DIFF_TERM("FALSE")
//	) 
	OBUFDS_inst1 (
			.I(OSER_OQ[0]),
			.O(DATA_OUT_p[0]), 
			.OB(DATA_OUT_n[0])
			);

	OBUFDS //#(
//			.DIFF_TERM("FALSE")
//	) 
	OBUFDS_inst2 (
			.I(OSER_OQ[1]),
			.O(DATA_OUT_p[1]), 
			.OB(DATA_OUT_n[1])
			);
			
	OBUFDS //#(
	//		.DIFF_TERM("FALSE")
//	) 
	OBUFDS_inst3 (
			.I(OSER_OQ[2]),
			.O(DATA_OUT_p[2]), 
			.OB(DATA_OUT_n[2])
			);
			
	OBUFDS //#(
	//		.DIFF_TERM("FALSE")
//	) 
	OBUFDS_inst4 (
			.I(OSER_OQ[3]),
			.O(DATA_OUT_p[3]), 
			.OB(DATA_OUT_n[3])
			);
//	
	OBUFDS //#(
	//		.DIFF_TERM("FALSE")
//	) 
	OBUFDS_inst5 (
			//.I(CLKDIV_R),   // 320MHz clock output
			.I(CLK_R),
			.O(CLK_O_p), 
			.OB(CLK_O_n)
			);
			
	OBUFDS //#(
	//		.DIFF_TERM("FALSE")
//	) 
	OBUFDS_inst6 (
			.I(OSER_OQ_MSG),
			.O(MSG_OUT_p), 
			.OB(MSG_OUT_n)
			);


	OBUFDS //#(
	//		.DIFF_TERM("FALSE")
//	) 
	OBUFDS_inst7 (
			.I(STATUS_OUT),
			.O(STATUS_OUT_p), 
			.OB(STATUS_OUT_n)
			);
	

	IBUFDS #(
			.DIFF_TERM("FALSE")
	) IBUFDS_inst7 (
			.I(STATUS_IN_p), 
			.IB(STATUS_IN_n), 
			.O(STATUS_IN)
			);
			
						
	IBUFDS #(
			.DIFF_TERM("FALSE")
	) IBUFDS_inst8 (
			.I(CLK_I_p), 
			.IB(CLK_I_n), 
			.O(CLK_R)
			);
						
	BUFR #(
	.BUFR_DIVIDE("4"),
	.SIM_DEVICE("VIRTEX5")
	) BUFR_inst1(
	.O(CLKDIV_R1),
	.CE(1'b1),
	.CLR(1'b0),
	.I(CLK_R)
	);
	
	BUFG BUFG_inst1 (.I(CLKDIV_R1), .O(CLKDIV_R));
	

	RCB_FRL_DDR_8TO1_16CHAN_RX RCB_FRL_DDR_8TO1_16CHAN_RX_inst
	(
		.RXCLK(CLK_R),
		.RXCLKDIV(CLKDIV_R),
		.RESET(RST),
		.IDLY_RESET(RST),
		
		.DATA_RX_P({DATA_IN_p,MSG_IN_p}),
		.DATA_RX_N({DATA_IN_n,MSG_IN_n}),
		.INC_PAD(1'b0),
		.DEC_PAD(1'b0),
		.DATA_FROM_ISERDES({DATA_IN[31:0],MSG_IN[7:0]}),
		.BITSLIP_PAD(1'b0),
		.TAP_00(),
		.TAP_01(),
		.TAP_02(),
		.TAP_03(),
		.TAP_04(),
		.TAP_CLK(),
		.TRAINING_DONE(TRAINING_DONE_TX)
	);
	


	RCB_FRL_data_check RCB_FRL_data_check_inst(
						.CLK(CLKDIV_R),
						.RST(RST),
						
						.RDEN_DATA(~DATA_ALMOSTEMPTY),
						.RDEN_MSG(),
						.DATA(DATA_INT_OUT),
						.MSG(MSG_INT_OUT),
						.ERR0(ERR0),
						.ERR1(ERR1),
						.ERR2(ERR2),
						.ERR3(ERR3),
						.ERR_MSG(ERR_MSG));

	RCB_FRL_RX RCB_FRL_RX_inst (
						.CLK(CLK_R), 
						.CLKDIV(CLKDIV_R),  
						.RDCLK(CLKDIV_R), 
						.RST(RST), 
						
						.DATA_IN(DATA_IN), 
						.DATA_OUT(DATA_INT_OUT),
						.RDEN(~DATA_ALMOSTEMPTY), 
						.ALMOSTEMPTY(DATA_ALMOSTEMPTY),
						.fifo_WREN(fifo_WREN)
						);
				
	assign MSG_Valid = EN_SIG;				// added by jiansong, 2010-5-27
	RCB_FRL_RX_MSG RCB_FRL_RX_MSG_inst (
						.CLK(CLK_R),
						.CLKDIV(CLKDIV_R),
						.RST(RST),
						
						.DATA_IN(MSG_IN),
						.DATA_OUT({MSG_INT_OUT_Addr[7:0],MSG_INT_OUT_Data[31:0]}),
						.RE_SIG(RE_SIG),
						.EN_SIG(EN_SIG),
						.CON_N(CON_N),
						.CON_P(CON_P)
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
