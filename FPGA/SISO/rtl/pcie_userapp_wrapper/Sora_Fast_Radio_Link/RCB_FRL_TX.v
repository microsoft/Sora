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
// Create Date:    11:07:49 02/11/2009 
// Design Name: 
// Module Name:    utils 
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

module RCB_FRL_TX (	CLK,
						CLKDIV,
						DATA_IN,
						SEND_EN,
						TRAINING_DONE,
						OSER_OQ,
						RST,
						RDEN);

	input 	CLK, CLKDIV, RST;
	input 	[31:0] DATA_IN;
	input   SEND_EN, TRAINING_DONE;
	
	output 	[3:0] OSER_OQ;
	output 	RDEN;

	reg  		[31:0] input_reg;
	wire 		[31:0] fifo_DO;
	wire 		RDEN;
	wire 		CE;
		
	reg [8:0] count;
//	parameter NUM = 9'h064;
	parameter NUM = 10'h008;    // modified by zjs, 2009-3-9
	
	reg RDEN_REG;
	wire [7:0] PATTERN;
		
	assign 	RDEN = RDEN_REG;
	assign	fifo_DO = DATA_IN;
	assign   CE = 1'b1;
	
	always @ (posedge CLKDIV) begin
		if (RST == 1'b1) begin
			count <= 9'h000;
			//RDEN_REG <= 1'b0;
			//OCE_REG <= 1'b0;
		end
		else begin
			if (count == 9'h000) begin
				if (SEND_EN == 1'b1) begin
					count <= 9'h001;
				end
				//OCE_REG <= 1'b0;
			end
//			else if (count == 9'h001) begin
//				//RDEN_REG <= 1'b1;
//				//OCE_REG <= 1'b1;
//				count <= 9'h002;
//			end
//			else if (count == (NUM+9'h001) ) begin
//				//RDEN_REG <= 1'b0;
//				count <= count +9'h001;
//			end
			//else if (count == (NUM+9'h002) ) begin
			//else if (count == (NUM+9'h004) ) begin // determine how many 00 it sends
			else if (count == (NUM+9'h002) ) begin // determine how many 00 it sends
				if (SEND_EN == 1'b1) begin
					count <= 9'h001;
				end
				else begin
					count <= 9'h000;
				end				
			end
			else begin
				count <= count + 9'h001;
			end
		end			
	end
	
	always @ (negedge CLKDIV) begin
		if (RST == 1'b1) begin
			RDEN_REG <= 1'b0;
		end
		else if (count == 9'h002) begin
			RDEN_REG <= 1'b1;
		end
		else if (count == NUM+9'h002) begin
			RDEN_REG <= 1'b0;
		end
	end
		
	RCB_FRL_TrainingPattern RCB_FRL_TrainingPattern_inst(
		.CLK(CLKDIV),
		.RST(RST),
		.DATA_OUT(PATTERN));			
		
	
	reg [7:0] data_count1;		
	always @ (negedge CLKDIV) begin
		if ( RST == 1'b1 ) begin
			input_reg[31:0] <= 32'h00000000;
			data_count1 <= 0;
		end
		else if (count == 9'h001) begin
			input_reg[31:0] <= 32'hF5F5F5F5;
		end
		else if (count == 9'h002) begin
			input_reg[31:0] <= {NUM[7:0],NUM[7:0],NUM[7:0],NUM[7:0]};
		end
		else if (count == 9'h000) begin
			input_reg[31:0] <= 32'h44444444;
			//data_count <= 0;
		end
		else if (count > NUM+9'h002) begin
			input_reg[31:0] <= 32'h00000000;
		end
		else begin
			input_reg[31:0] <= fifo_DO[31:0];
			//data_count1 <= data_count1 + 1;
			//input_reg[31:24] <= data_count1;
			//input_reg[23:16] <= data_count1;
			//input_reg[15:8] <= data_count1;
			//input_reg[7:0] <= data_count1;
		end
	end
	
/*	
		//////////////////////////////////////////////////////////
	/// 8b/10b transform
	wire [39:0] OUTPUT_REG;
	encode_8b10b_lut_base inst_encode_8b10b_lut_base_1(
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
	encode_8b10b_lut_base inst_encode_8b10b_lut_base_2(
		.DIN(input_reg[15:8]),   
		.KIN(),
		.CLK(CLKDIV),              // : IN  STD_LOGIC                    :='0' ;
		.DOUT(OUTPUT_REG[19:10]),      //        : OUT STD_LOGIC_VECTOR(9 DOWNTO 0) :=
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
	encode_8b10b_lut_base inst_encode_8b10b_lut_base_3(
		.DIN(input_reg[23:16]),   
		.KIN(),
		.CLK(CLKDIV),              // : IN  STD_LOGIC                    :='0' ;
		.DOUT(OUTPUT_REG[29:20]),      //        : OUT STD_LOGIC_VECTOR(9 DOWNTO 0) :=
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
	encode_8b10b_lut_base inst_encode_8b10b_lut_base_4(
		.DIN(input_reg[31:24]),   
		.KIN(),
		.CLK(CLKDIV),              // : IN  STD_LOGIC                    :='0' ;
		.DOUT(OUTPUT_REG[39:30]),      //        : OUT STD_LOGIC_VECTOR(9 DOWNTO 0) :=
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
	//assign probe = input_reg[7:0];
	//temp for inversion
	wire [31:0] input_reg_INV;
	/*assign input_reg_INV = {~input_reg[31], ~input_reg[30],~input_reg[29],~input_reg[28],~input_reg[27],~input_reg[26],~input_reg[25],~input_reg[24],
							~input_reg[23], ~input_reg[22],~input_reg[21],~input_reg[20],~input_reg[19],~input_reg[18],~input_reg[17],~input_reg[16],
							~input_reg[15], ~input_reg[14],~input_reg[13],~input_reg[12],~input_reg[11],~input_reg[10],~input_reg[9],~input_reg[8],
							~input_reg[7], ~input_reg[6],~input_reg[5],~input_reg[4],~input_reg[3],~input_reg[2],~input_reg[1],~input_reg[0]};
	
	always @ (negedge CLKDIV) begin
		if ( RST == 1'b1 ) begin
			input_reg_INV <= 0;
		end
		else if (TRAINING_DONE == 1) begin
			input_reg_INV <= OUTPUT_REG;
		end
		else begin
			input_reg_INV <= {PATTERN,PATTERN,PATTERN,PATTERN};
		end
	end
	*/	
	
	
	assign input_reg_INV = TRAINING_DONE ? input_reg : {PATTERN,PATTERN,PATTERN,PATTERN};
	//assign input_reg_INV = {PATTERN,PATTERN,PATTERN,PATTERN};
	
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst1 (.OQ(OSER_OQ[0]), .CLK(CLK), .CLKDIV(CLKDIV), .DI(input_reg_INV[7:0]), .OCE(1'b1), .SR(RST));
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst2 (.OQ(OSER_OQ[1]), .CLK(CLK), .CLKDIV(CLKDIV), .DI(input_reg_INV[15:8]), .OCE(1'b1), .SR(RST));
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst3 (.OQ(OSER_OQ[2]), .CLK(CLK), .CLKDIV(CLKDIV), .DI(input_reg_INV[23:16]), .OCE(1'b1), .SR(RST));
	RCB_FRL_OSERDES RCB_FRL_OSERDES_inst4 (.OQ(OSER_OQ[3]), .CLK(CLK), .CLKDIV(CLKDIV), .DI(input_reg_INV[31:24]), .OCE(1'b1), .SR(RST));
	
endmodule