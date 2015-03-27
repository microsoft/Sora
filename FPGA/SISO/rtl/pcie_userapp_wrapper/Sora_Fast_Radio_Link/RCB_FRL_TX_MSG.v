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
// Create Date:    13:46:27 02/23/2009 
// Design Name: 
// Module Name:    MSG_utils 
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
//module inst_TX_MSG ( 	CLK, 
//								CLKDIV,
//								DATA_IN, 
//								OSER_OQ, 
//								RST, 
//								CLKWR,
//								WREN, 
//								CON_P, 									//input feedback from decode module
//								CON_N, 
//								BACK_WRONG, 		//feedback output to internal module
//								BACK_RIGHT,
//								ALMOSTFULL,
//								EMPTY);
						
module RCB_FRL_TX_MSG (
		CLK, 
		CLKDIV, 
		DATA_IN, 
		OSER_OQ, 
		RST, 
		CLKWR, 
		WREN, 
		CON_P, 
		CON_N, 
		BACK_WRONG, 
		BACK_RIGHT, 
		ALMOSTFULL, 
		EMPTY, 
		probe, 
		EN_SIG, 
		RE_SIG, 
		TRAINING_DONE
	);

	input CLK, CLKDIV, RST, CLKWR, WREN, CON_P, CON_N, TRAINING_DONE;
	input [39:0] DATA_IN;
	output probe;
	input RE_SIG, EN_SIG;
	
	output OSER_OQ;
	output BACK_WRONG, BACK_RIGHT, ALMOSTFULL;
	output EMPTY;

	reg [7:0] input_reg;
	wire [39:0] fifo_DO;
	wire ALMOSTEMPTY, ALMOSTFULL, EMPTY, FULL;
	wire EMPTY_fifo;				// Added by Jiansong, 2010-5-27
	wire RDEN;
	
	reg [8:0] count;
	parameter NUM = 8'h06;
	
	reg RDEN_REG;
		
	RCB_FRL_fifo_MSG RCB_FRL_fifo_MSG_inst (
			.ALMOSTEMPTY(ALMOSTEMPTY), 
			.ALMOSTFULL(ALMOSTFULL), 
			.DO(fifo_DO), 
//			.EMPTY(EMPTY),
			.EMPTY(EMPTY_fifo),			// modified by Jiansong, 2010-5-27
			.FULL(FULL), 
			.DI(DATA_IN), 
			.RDCLK(CLKDIV), 
			.RDEN(RDEN_REG), 
			.WRCLK(CLKWR), 
			.WREN(WREN), 
			.RST(RST)
	);
	//assign EMPTY = 0;  //xt
	
	wire [7:0] CRC;
	RCB_FRL_CRC_gen RCB_FRL_CRC_gen_inst ( .D({{NUM},{fifo_DO}}), .NewCRC(CRC));
	
	// Added by Jiansong, 2010-5-27, do not send out control message in training stage
	assign EMPTY = EMPTY_fifo | (~TRAINING_DONE);		
	
	reg [3:0] times;
	reg BACK_WRONG, BACK_RIGHT;
	
	assign probe = RDEN_REG;
	
	always @ (posedge CLKDIV) begin
		if (RST == 1'b1) begin
			count <= 9'h000;
			RDEN_REG <= 1'b0;
			times <= 4'h0;
		end
		else begin
			if (count == 9'h1FF | count == 9'h1FE) 
			begin
				count <= 9'h000;
			end
			else if (count == 9'h000) begin
				if ( EN_SIG == 1'b1)					// Jiansong: ACK first
				begin
					count <= 9'h1FF;
				end
				else if (RE_SIG == 1'b1)
				begin
					count <= 9'h1FE;
				end
				else
				begin
					if (EMPTY == 1'b1 & times == 4'h0) begin
						count <= 9'h000;
					end
					else if (EMPTY == 1'b0) begin
						count <= 9'h001;
						RDEN_REG <= 1'b0;
					end
					else if (times == 4'h1 | times ==4'h2) begin
						count <= 9'h001;
						RDEN_REG <= 1'b0;
					end
					else if (times == 4'h3) begin
						times <= 1'b0;
						RDEN_REG <= 1'b0;
					end						
				end
				BACK_WRONG <= 1'b0;
				BACK_RIGHT <= 1'b0;
			end
			else if (count == 9'h001) begin
				if (times == 4'h0) begin
					RDEN_REG <= 1'b1;
				end
				times <= times + 4'h1;
				count <= 9'h002;
				BACK_WRONG <= 1'b0;
				BACK_RIGHT <= 1'b0;
			end
			else if (count == 9'h002) begin
				RDEN_REG <= 1'b0;
				count <= 9'h003;
			end
			//else if (count == (NUM+8'h03) ) begin
			else if ( CON_P == 1'b1) begin	
//				if (EMPTY == 1'b0) begin
//					count <= 9'h001;
//				end
//				else begin
					count <= 9'h000;
//				end
				times <= 4'h0;
				BACK_RIGHT <= 1'b1;
			end
			else if (times == 4'h3 & count == 9'h150) begin
//				if (EMPTY == 1'b0) begin
//					count <= 9'h001;
//				end
//				else begin
				count <= 9'h000;
//				end
				times <= 4'h0;
				BACK_WRONG <= 1'b1;
			end
			else if ( CON_N == 1'b1 | count == 9'h150) begin
				count <= 9'h000;
			end			
			else begin
				count <= count + 9'h001;
			end
		end			
	end
		
	reg [8:0] data_counter;
	
	always @ (negedge CLKDIV) begin
		if ( RST == 1'b1 ) begin
			input_reg[7:0] <= 8'h00;
			data_counter <= 8'h00;
		end
		else if (count == 9'h001) begin
			input_reg[7:0] <= 8'hF5;
		end
		else if (count == 9'h002) begin
			input_reg[7:0] <= NUM;
		end
		else if (count == 9'h000) begin
			input_reg[7:0] <= 8'h44;		// modified by zjs, 2009-3-8, send 8'h44 on idle state to train LVDS receiver
		end
		else if (count == 9'h003) begin
			input_reg[7:0] <= fifo_DO[39:32];
			//data_counter <= data_counter + 1;
			//input_reg[7:0] <= data_counter;
		end
		else if (count == 9'h004) begin
			input_reg[7:0] <= fifo_DO[31:24];
			//data_counter <= data_counter + 1;
			//input_reg[7:0] <= data_counter;
		end
		else if (count == 9'h005) begin
			input_reg[7:0] <= fifo_DO[23:16];
			//data_counter <= data_counter + 1;
			//input_reg[7:0] <= data_counter;
		end
		else if (count == 9'h006) begin
			input_reg[7:0] <= fifo_DO[15:8];
			//data_counter <= data_counter + 1;
			//input_reg[7:0] <= data_counter;
		end
		else if (count == 9'h007) begin
			input_reg[7:0] <= fifo_DO[7:0];
			//data_counter <= data_counter + 1;
			//input_reg[7:0] <= data_counter;
		end
		else if (count == 9'h008) begin
			//data_counter <= data_counter + 1;
			//input_reg[7:0] <= data_counter;
			input_reg[7:0] <= CRC[7:0];
			
			//input_reg[7:0] = 8'h02;
		end
		else if (count == 9'h1FF) begin
			input_reg[7:0] <= 8'h5f;
		end
		else if (count == 9'h1FE) begin
			input_reg[7:0] <= 8'haf;
		end
		else begin
			input_reg[7:0] <= 8'h00;
		end
	end
/*	
	wire CE;
	assign CE = 1'b1;
	wire [9:0] OUTPUT_REG;
	encode_8b10b_lut_base inst_encode_8b10b_lut_base_CMD(
		.DIN(input_reg[7:0]),   
		.KIN(),
		.CLK(CLKDIV),              // : IN  STD_LOGIC                    :='0' ;
		.DOUT(OUTPUT_REG[9:0]),      //        : OUT STD_LOGIC_VECTOR(9 DOWNTO 0) :=
                      //  str_to_slv(C_FORCE_CODE_VAL, 10) ;
		.CE(CE),       //         : IN  STD_LOGIC                    :='0' ;
		.FORCE_CODE(),   //     : IN  STD_LOGIC                    :='0' ;
		.FORCE_DISP(), //       : IN  STD_LOGIC                    :='0' ;
		.DISP_IN(),     //      : IN  STD_LOGIC                    :='0' ;
		.DISP_OUT(),    //      : OUT STD_LOGIC                    :=
                     //   bint_2_sl(C_FORCE_CODE_DISP);
		.KERR(),        //      : OUT STD_LOGIC                    :='0' ;
		.ND()           //     : OUT STD_LOGIC                    :='0'
	);
*/	
	wire [7:0] PATTERN;
	RCB_FRL_TrainingPattern RCB_FRL_TrainingPattern_inst(
		.CLK(CLKDIV),
		.RST(RST),
		.DATA_OUT(PATTERN));	
	
	wire [7:0] input_reg_inv;	
	assign input_reg_inv = TRAINING_DONE ? input_reg : PATTERN;
	//assign input_reg_inv = PATTERN;
	
	//inst_OSERDES_MSG inst_OSERDES1 (.OQ(OSER_OQ), .CLK(CLK), .CLKDIV(CLKDIV), .DI(input_reg_inv), .OCE(1'b1), .SR(RST));
	RCB_FRL_OSERDES_MSG RCB_FRL_OSERDES_MSG_inst (
			.OQ(OSER_OQ), .CLK(CLK), .CLKDIV(CLKDIV), .DI(input_reg_inv), .OCE(1'b1), .SR(RST));
	
endmodule
