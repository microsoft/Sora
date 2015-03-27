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
						
module RCB_FRL_TX_MSG (
		input 			clk, 
		input 			clkdiv,  
		input 			rst, 
		input [39:0]	data_in, 
		input				empty,
		output reg		rden,
		output 			OSER_OQ,
		input 			ack_r_one, 
		input 			nack_r_one, 
		output reg		txmsg_miss_one, 	// one cycle signal to indicate a TXMSG wasn't correctly received by another Sora-FRL end
		output reg		txmsg_pass_one, 	// one cycle signal to indicate a TXMSG was correctly received by another Sora-FRL end
		input 			msg_r_p_one, 
		input 			msg_r_n_one,
		input 			training_done
	);

	reg [7:0] frame_data;
	wire NotReady;		
	
	reg	msg_r_p_signal;
	reg	msg_r_n_signal;
	
//	reg [8:0] count;

	parameter MSG_HEADER	= 8'hF5;
	parameter MSG_SIZE 	= 8'h06;
	parameter ACK_SYM		= 8'h5F;
	parameter NACK_SYM	= 8'hAF;
	parameter DUMMY_WAIT = 8'h44;
	
	parameter TIME_OUT	= 9'h150;
	
	parameter RETRY_MAX	= 3'b011;	// retransmit 3 times
	
//	reg RDEN_REG;	
//	assign RDEN = RDEN_REG;
	
	parameter IDLE				= 10'b00_0000_0001;
	parameter HEADER			= 10'b00_0000_0010;
	parameter SIZE				= 10'b00_0000_0100;
	parameter ADDR				= 10'b00_0000_1000;
	parameter DATA_1			= 10'b00_0001_0000;
	parameter DATA_2			= 10'b00_0010_0000;
	parameter DATA_3			= 10'b00_0100_0000;
	parameter DATA_4			= 10'b00_1000_0000;
	parameter CRC				= 10'b01_0000_0000;
	parameter WAIT_FOR_ACK	= 10'b10_0000_0000;	
	reg [9:0] msg_framer_state;
	
	reg [8:0] cnt;
	
	// calculate CRC8
	wire [7:0] CRC8;
//	RCB_FRL_CRC_gen RCB_FRL_CRC_gen_inst (
	CRC8_gen RCB_FRL_CRC_gen_inst ( 
		.D({MSG_SIZE,data_in[39:0]}), 
		.NewCRC(CRC8)
	);
	
	// Do not send out control message in training stage
	assign NotReady = empty | (~training_done);		
	
//	reg [3:0] times;		// for each control message, we will retransmit up to 3 times
	reg [3:0]	retry_cnt;
	
	always@(posedge clkdiv) begin
		if (rst) begin
			rden					<= 1'b0;
			retry_cnt			<= 3'b000;
			cnt					<= 9'h000;
			frame_data[7:0]	<= 8'h00;
			msg_framer_state	<= IDLE;
		end else begin
			case(msg_framer_state)
				IDLE: begin
					cnt			<= 9'h000;
					retry_cnt	<= 3'b000;
					
					if(msg_r_p_signal) begin
						rden					<= 1'b0;
						frame_data[7:0]	<= ACK_SYM;
						msg_framer_state	<= IDLE;
					end else if (msg_r_n_signal) begin
						rden					<= 1'b0;
						frame_data[7:0]	<= NACK_SYM;
						msg_framer_state	<= IDLE;
					end else if (!NotReady) begin
						rden					<= 1'b1;
						frame_data[7:0]	<= 8'h00;
						msg_framer_state	<= HEADER;
					end else begin
						rden					<= 1'b0;
						frame_data[7:0]	<= 8'h00;
						msg_framer_state	<= IDLE;
					end
					
				end
				
				HEADER: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= MSG_HEADER;
					msg_framer_state	<= SIZE;
				end

				SIZE: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= MSG_SIZE;
					msg_framer_state	<= ADDR;
				end
				
				ADDR: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= data_in[39:32];
					msg_framer_state	<= DATA_1;
				end
				
				DATA_1: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= data_in[31:24];
					msg_framer_state	<= DATA_2;
				end

				DATA_2: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= data_in[23:16];
					msg_framer_state	<= DATA_3;
				end

				DATA_3: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= data_in[15:8];
					msg_framer_state	<= DATA_4;
				end

				DATA_4: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= data_in[7:0];
					msg_framer_state	<= CRC;
				end
				
				CRC: begin
					rden					<= 1'b0;
					retry_cnt			<= retry_cnt;
					cnt					<= 9'h000;
					frame_data[7:0]	<= CRC8;
					msg_framer_state	<= WAIT_FOR_ACK;
				end
				
				WAIT_FOR_ACK: begin					
					rden					<= 1'b0;
					cnt					<= cnt + 9'h001;
					
					if (cnt > 9'h001) begin		// leave two cycle space between a message and an ACK/NACK, 
														// it will be easier for decoder to handle
						if (msg_r_p_signal)
							frame_data[7:0]	<= ACK_SYM;
						else if (msg_r_n_signal)
							frame_data[7:0]	<= NACK_SYM;
						else
							frame_data[7:0]	<= DUMMY_WAIT;
					end else
							frame_data[7:0]	<= DUMMY_WAIT;
											
					if (ack_r_one) begin
						retry_cnt			<= 3'b000;
						msg_framer_state	<= IDLE;
					end else if ( nack_r_one | (cnt > TIME_OUT) ) begin
						if (retry_cnt >= RETRY_MAX) begin
							retry_cnt			<= 3'b000;
							msg_framer_state	<= IDLE;
						end else begin
							retry_cnt			<= retry_cnt + 3'b001;
							msg_framer_state	<= HEADER;
						end
					end else begin
						retry_cnt			<= retry_cnt;
						msg_framer_state	<= WAIT_FOR_ACK;						
					end
					
				end
				
				default: begin
					rden					<= 1'b0;
					retry_cnt			<= 3'b000;
					cnt					<= 9'h000;
					frame_data[7:0]	<= 8'h00;
					msg_framer_state	<= IDLE;
				end
			endcase
		end
		
	end
	
	// msg_r_p_signal
	always@(posedge clkdiv) begin
		if (rst) 
			msg_r_p_signal		<= 1'b0;
		else if (msg_r_p_one)
			msg_r_p_signal		<= 1'b1;
		else if ( (msg_framer_state == IDLE) | ((msg_framer_state == WAIT_FOR_ACK)&(cnt > 9'h001)) )
			msg_r_p_signal		<= 1'b0;
		else
			msg_r_p_signal		<= msg_r_p_signal;
	end
	
	// msg_r_n_signal
	always@(posedge clkdiv) begin
		if (rst) 
			msg_r_n_signal		<= 1'b0;
		else if (msg_r_n_one)
			msg_r_n_signal		<= 1'b1;
		else if ( (msg_framer_state == IDLE) | ((msg_framer_state == WAIT_FOR_ACK)&(cnt > 9'h001)) )
			msg_r_n_signal		<= 1'b0;
		else
			msg_r_n_signal		<= msg_r_n_signal;
	end
	
	// txmsg_miss_one
	always@(posedge clkdiv) begin
		if(rst)
			txmsg_miss_one	<= 1'b0;
		else if ( (retry_cnt >= RETRY_MAX) & (cnt > TIME_OUT | nack_r_one) )
			txmsg_miss_one	<= 1'b1;
		else
			txmsg_miss_one	<= 1'b0;
	end
	
	// txmsg_pass_one
	always@(posedge clkdiv) begin
		if(rst)
			txmsg_pass_one	<= 1'b0;
		else if ( (msg_framer_state == WAIT_FOR_ACK) & (ack_r_one) )
			txmsg_pass_one	<= 1'b1;
		else
			txmsg_pass_one	<= 1'b0;
	end
	
//	// main msg_framer_state machine
//	always@(posedge clkdiv) begin
//		if (rst) begin
//			count 		<= 9'h000;
//			RDEN_REG		<= 1'b0;
//			TXMSG_MISS 	<= 1'b0;
//			TXMSG_PASS 	<= 1'b0;
//			times 		<= 4'h0;
//		end
//		else begin
//			if (count == 9'h1FF | count == 9'h1FE) begin
//				count <= 9'h000;
//			end else if (count == 9'h000) begin
//				if (msg_r_p)	begin				// Jiansong: ACK or NACK first
//					count <= 9'h1FF;
//				end else if (msg_r_n) begin
//					count <= 9'h1FE;
//				end else	begin
//					if ( NotReady && (times == 4'h0) ) begin
//						count <= 9'h000;
//					end else begin	// (!NotReady | times == 4'h1 | times ==4'h2)
//						count <= 9'h001;
//					end	
//				end
//				RDEN_REG 	<= 1'b0;
//				TXMSG_MISS 	<= 1'b0;
//				TXMSG_PASS 	<= 1'b0;
//				times 		<= times;
//			end else if (count == 9'h001) begin
//				count <= 9'h002;
//				
//				if (times == 4'h0)
//					RDEN_REG <= 1'b1;
//				else
//					RDEN_REG <= 1'b0;
//				
//				times	<= times + 4'h1;
//
//				TXMSG_MISS <= 1'b0;
//				TXMSG_PASS <= 1'b0;
//			end else if (count == 9'h002) begin
//				count 		<= 9'h003;
//				RDEN_REG 	<= 1'b0;
//				TXMSG_MISS 	<= 1'b0;
//				TXMSG_PASS 	<= 1'b0;
//				times 		<= times;
//			end else if (count < 9'h009) begin	// ignore ACK/NACK in message transmission period
//				count 		<= count + 9'h001;
//				RDEN_REG 	<= 1'b0;
//				TXMSG_MISS 	<= 1'b0;
//				TXMSG_PASS 	<= 1'b0;
//				times 		<= times;
//			end else begin
//				if(ACK | NACK | count == 9'h150)
//					count 	<= 9'h000;
//				else
//					count 	<= count + 9'h001;
//				
//				RDEN_REG 	<= 1'b0;
//				
//				if(ACK) begin
//					times 		<= 4'h0;	
//					TXMSG_PASS	<= 1'b1;
//					TXMSG_MISS 	<= 1'b0;
//				end else if (NACK | count == 9'h150) begin
//					if (times == 4'h3) begin
//						times			<= 4'h0;
//						TXMSG_MISS 	<= 1'b1;
//					end else begin
//						times			<= times;
//						TXMSG_MISS 	<= 1'b0;
//					end
//					TXMSG_PASS	<= 1'b0;				
//				end else begin
//					times 		<= times;	
//					TXMSG_PASS	<= 1'b0;
//					TXMSG_MISS 	<= 1'b0;
//				end
//			end
//		end			
//	end
//	
//	// frame encapsulation
//	always@(posedge clkdiv) begin
//		if (rst)
//			frame_data[7:0] <= 8'h00;
//		else if (count == 9'h001)
//			frame_data[7:0] <= 8'hF5;
//		else if (count == 9'h002) 
//			frame_data[7:0] <= MSG_SIZE;
//		else if (count == 9'h000)
//			frame_data[7:0] <= 8'h44;		// send 8'h44 on idle msg_framer_state 
//		else if (count == 9'h003) 
//			frame_data[7:0] <= data_in[39:32];
//		else if (count == 9'h004) 
//			frame_data[7:0] <= data_in[31:24];
//		else if (count == 9'h005) 
//			frame_data[7:0] <= data_in[23:16];
//		else if (count == 9'h006) 
//			frame_data[7:0] <= data_in[15:8];
//		else if (count == 9'h007) 
//			frame_data[7:0] <= data_in[7:0];
//		else if (count == 9'h008) 
//			frame_data[7:0] <= CRC[7:0];
//		else if (count == 9'h1FF) 
//			frame_data[7:0] <= 8'h5f;
//		else if (count == 9'h1FE) 
//			frame_data[7:0] <= 8'haf;
//		else 
//			frame_data[7:0] <= 8'h00;
//	end
	
//	wire [7:0] PATTERN;
//	RCB_FRL_TrainingPattern RCB_FRL_TrainingPattern_inst(
//		.clk(clkdiv),
//		.rst(rst),
//		.trainingpattern(PATTERN)
//	);	
	
	wire [7:0] data_to_oserdes;	
//	assign data_to_oserdes = training_done ? frame_data : PATTERN;
	assign data_to_oserdes = training_done ? frame_data : 8'h5c;		// training pattern for spartan6
	
	RCB_FRL_OSERDES_MSG RCB_FRL_OSERDES_MSG_inst (
		.OQ(OSER_OQ), 
		.clk(clk), 
		.clkdiv(clkdiv), 
		.DI(data_to_oserdes), 
		.OCE(1'b1), 
		.SR(rst)
	);
	
endmodule
