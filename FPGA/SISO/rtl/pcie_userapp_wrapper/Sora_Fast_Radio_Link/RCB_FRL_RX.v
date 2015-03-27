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
// Create Date:    11:19:11 02/11/2009 
// Design Name: 
// Module Name:    RCB_FRL_RX
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
////////////////////
module RCB_FRL_RX(CLK, CLKDIV, DATA_IN, DATA_OUT, RST, RDCLK, RDEN, ALMOSTEMPTY, fifo_WREN);

	//input CLK_p, CLK_n;
	input CLK, CLKDIV;
	//input [3:0] DATA_IN_p, DATA_IN_n;
	input [31:0] DATA_IN;
	output [31:0] DATA_OUT;
	output ALMOSTEMPTY;
	output fifo_WREN;
//	input [7:0] probe;
	input RST, RDCLK, RDEN;
	wire [31:0] DATA_IN;
	
	wire [7:0] fifo_reg1;
	wire [7:0] fifo_reg2;
	wire [7:0] fifo_reg3;
	wire [7:0] fifo_reg4;
	//wire [7:0] probe;
	
	RCB_FRL_channel inst_channel1 (
			.CLK(CLK), .CLKDIV(CLKDIV), .DATA_IN(DATA_IN[7:0]), .RST(RST), .fifo_WREN1(fifo_WREN1), 
			.fifo_reg1(fifo_reg1[7:0])/*, .probe(probe)*/);
	RCB_FRL_channel inst_channel2 (
			.CLK(CLK), .CLKDIV(CLKDIV), .DATA_IN(DATA_IN[15:8]), .RST(RST), .fifo_WREN1(fifo_WREN2), 
			.fifo_reg1(fifo_reg2[7:0]));
	RCB_FRL_channel inst_channel3 (
			.CLK(CLK), .CLKDIV(CLKDIV), .DATA_IN(DATA_IN[23:16]), .RST(RST), .fifo_WREN1(fifo_WREN3), 
			.fifo_reg1(fifo_reg3[7:0]));
	RCB_FRL_channel inst_channel4 (
			.CLK(CLK), .CLKDIV(CLKDIV), .DATA_IN(DATA_IN[31:24]), .RST(RST), .fifo_WREN1(fifo_WREN4), 
			.fifo_reg1(fifo_reg4[7:0]));
	
	assign fifo_WREN = fifo_WREN1 & fifo_WREN2 & fifo_WREN3 & fifo_WREN4;
	
	//assign probe = fifo_reg1;
	
	//used to reverse the sequence
//	wire [7:0] fifo_reg_reverse1, fifo_reg_reverse2, fifo_reg_reverse3, fifo_reg_reverse4;
//	assign fifo_reg_reservse1 = {fifo_reg1[0], fifo_reg1[1],fifo_reg1[2],fifo_reg1[3],fifo_reg1[4],fifo_reg1[5],fifo_reg1[6],fifo_reg1[7]};
//	assign fifo_reg_reservse2 = {fifo_reg2[0], fifo_reg2[1],fifo_reg2[2],fifo_reg2[3],fifo_reg2[4],fifo_reg2[5],fifo_reg2[6],fifo_reg2[7]};
//	assign fifo_reg_reservse3 = {fifo_reg3[0], fifo_reg3[1],fifo_reg3[2],fifo_reg3[3],fifo_reg3[4],fifo_reg3[5],fifo_reg3[6],fifo_reg3[7]};
//	assign fifo_reg_reservse4 = {fifo_reg4[0], fifo_reg4[1],fifo_reg4[2],fifo_reg4[3],fifo_reg4[4],fifo_reg4[5],fifo_reg4[6],fifo_reg4[7]};
//	
	
	wire [31:0] DATA_OUT;
	wire ALMOSTEMPTY1, ALMOSTEMPTY2, ALMOSTEMPTY3, ALMOSTEMPTY4;
	
	RCB_FRL_fifo_RX inst_fifoRX1(
				.ALMOSTEMPTY(ALMOSTEMPTY1), .ALMOSTFULL(), .DO(DATA_OUT [7:0]), .EMPTY(), .FULL(),
				.DI(fifo_reg1[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(fifo_WREN1), .RST(RST));
	RCB_FRL_fifo_RX inst_fifoRX2(
				.ALMOSTEMPTY(ALMOSTEMPTY2), .ALMOSTFULL(), .DO(DATA_OUT [15:8]), .EMPTY(), .FULL(),
				.DI(fifo_reg2[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(fifo_WREN2), .RST(RST));
	RCB_FRL_fifo_RX inst_fifoRX3(
				.ALMOSTEMPTY(ALMOSTEMPTY3), .ALMOSTFULL(), .DO(DATA_OUT [23:16]), .EMPTY(), .FULL(),
				.DI(fifo_reg3[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(fifo_WREN3), .RST(RST));
	RCB_FRL_fifo_RX inst_fifoRX4(
				.ALMOSTEMPTY(ALMOSTEMPTY4), .ALMOSTFULL(), .DO(DATA_OUT [31:24]), .EMPTY(), .FULL(),
				.DI(fifo_reg4[7:0]), .RDCLK(RDCLK), .RDEN(RDEN), .WRCLK(CLKDIV), .WREN(fifo_WREN4), .RST(RST));
								
	wire ALMOSTEMPTY;
	assign ALMOSTEMPTY = ALMOSTEMPTY1 | ALMOSTEMPTY2 | ALMOSTEMPTY3 | ALMOSTEMPTY4;


endmodule


module RCB_FRL_channel (CLK, CLKDIV, DATA_IN, RST, fifo_WREN1, fifo_reg1);

	input CLK, CLKDIV, RST;
	input [7:0] DATA_IN;
	output fifo_WREN1;
	output [7:0] fifo_reg1;

	reg [15:0] input_reg1;
	wire [7:0] input_reg1_wire, input_reg1_wire_inv;
	
	
	/*assign input_reg1_wire = {~input_reg1_wire_inv[7],
										~input_reg1_wire_inv[6],
										~input_reg1_wire_inv[5],
										~input_reg1_wire_inv[4],
										~input_reg1_wire_inv[3],
										~input_reg1_wire_inv[2],
										~input_reg1_wire_inv[1],
										~input_reg1_wire_inv[0]};*/
	assign input_reg1_wire = DATA_IN;
	
	
	RCB_FRL_CMD_shift RCB_FRL_CMD_shift_inst
	(
		.data_in(input_reg1_wire),
		.clk(CLKDIV), 
		.enable(~RST), 
		.data_out(fifo_reg1), 
		.data_valid(fifo_WREN1)
	);

endmodule






module RCB_FRL_fifo_RX (ALMOSTEMPTY, ALMOSTFULL, DO, EMPTY, FULL, DI, RDCLK, RDEN, WRCLK, WREN, RST);

	output 	ALMOSTEMPTY, ALMOSTFULL, EMPTY, FULL;
	output 	[7:0] DO;
	input		[7:0] DI;
	input		RDCLK, RDEN, WRCLK, WREN, RST;
	wire [7:0] temp1;


	FIFO18  FIFO18_inst (
		.ALMOSTEMPTY(ALMOSTEMPTY), // 1-bit almost empty output flag
		.ALMOSTFULL(ALMOSTFULL), // 1-bit almost full output flag
		.DO({temp1, DO[7:0]}), // 16-bit data output
		.DOP(), // 2-bit parity data output
		.EMPTY(EMPTY), // 1-bit empty output flag
		.FULL(FULL), // 1-bit full output flag
		.RDCOUNT(), // 12-bit read count output
		.RDERR(), // 1-bit read error output
		.WRCOUNT(), // 12-bit write count output
		.WRERR(), // 1-bit write error
		.DI({8'h0,DI[7:0]}), // 16-bit data input
		.DIP(), // 2-bit parity input
		.RDCLK(RDCLK), // 1-bit read clock input
		.RDEN(RDEN), // 1-bit read enable input
		.RST(RST), // 1-bit reset input
		.WRCLK(WRCLK), // 1-bit write clock input
		.WREN(WREN) // 1-bit write enable input
	);
	defparam FIFO18_inst.DATA_WIDTH = 9;
	defparam FIFO18_inst.ALMOST_EMPTY_OFFSET = 6;
	//defparam FIFO18_inst.ALMOST_FULL_OFFSET = 7;
	//defparam FIFO18_36_inst.DO_REG = 1;
//	#(
//		.ALMOST_FULL_OFFSET(9'h004), // Sets almost full threshold
//		.ALMOST_EMPTY_OFFSET(9'h004), // Sets the almost empty threshold
//		.DATA_WIDTH(32), // Sets data width to 4, 9 or 18
//		.DO_REG(1), // Enable output register (0 or 1)
//		// Must be 1 if EN_SYN = "FALSE"
//		.EN_SYN("FALSE"), // Specifies FIFO as Asynchronous ("FALSE")
//		// or Synchronous ("TRUE")
//		.FIRST_WORD_FALL_THROUGH("FALSE") // Sets the FIFO FWFT to "TRUE" or "FALSE"
//		)

endmodule


module RCB_FRL_CMD_shift
(
	data_in,
	clk, 
	enable, 
	data_out, 
	data_valid
	);

//   modified by zjs, 2009-3-9, none of the parameters are really used   
//   parameter lock_pattern = 8'hf5;   //Specifies the lock pattern
//   parameter pack_size = 100;             // Current packet size 
//   parameter word_size = 8;          // Not currently used in code
   


   input     clk;                      
   input     enable;                 //Active low reset
   input [7:0] data_in;           
   
   output       data_valid;
   output [7:0] data_out;
  // output       lockk;
   
   reg [7:0] 	data_reg1;              //First register used to re-align the data
   reg [7:0] 	data_reg2;              //Second register used to re-align the data
   reg [7:0] 	data_reg3;              //Third register used to re-align the data
   reg [2:0] 	shift;                  //designate the bit shift value of the package
   reg 		    lock;                   //if high, then the correct shift value has been found.
   reg [7:0] 	byte_count;             //word count in packet
   reg [2:0]    front_count;
   reg [7:0] 	data_out_tmp;           //pipeline register
   reg 	    	data_valid;

   reg [7:0] 	frame_length;      
   reg [7:0] 	data_out;               // output register
   
//   wire 	end_pack = pack_size;   // signifies end of packet
   


   always @(negedge clk)   //¡ã¡ä2¨¦¦Ì?¦Ì???¨ª¡¤¨º?3???3¨¬
 //  always @(negedge clk) // timing broken to set data_valid flag
     begin
		if (!enable)begin
    	      data_out_tmp <= 8'h00;    //test          data_out_tmp <= 8'h00;
				//data_valid<=1'b1;
				end
		else
		begin
		    //data_valid<=1'b1;
    	    case(shift)               //Re-aligns the data depending on shift value
		    	3'h0 : data_out_tmp <= data_reg1;
		    	3'h1 : data_out_tmp <= ({data_reg1[6:0],data_in[7]});
		    	3'h2 : data_out_tmp <= ({data_reg1[5:0],data_in[7:6]});
		    	3'h3 : data_out_tmp <= ({data_reg1[4:0],data_in[7:5]});
		    	3'h4 : data_out_tmp <= ({data_reg1[3:0],data_in[7:4]});
		    	3'h5 : data_out_tmp <= ({data_reg1[2:0],data_in[7:3]});
		    	3'h6 : data_out_tmp <= ({data_reg1[1:0],data_in[7:2]});
		    	3'h7 : data_out_tmp <= ({data_reg1[0],data_in[7:1]});
		    	default : data_out_tmp <= data_reg1;
	        endcase
		 end
	 end



 // Word counter, counts words in packet  
   always@(negedge clk)           //??¨ºy??3¨¬
     begin
		 if(!enable || !lock)		    	      	 // Active low reset
			begin
    	      byte_count <= 0;                   //Initializes byte count
    	      front_count <= 0;
			  //frame_length <= 0;
			end
			
    	 if(lock)//lock   data_valid
    	  begin
    	      byte_count <= byte_count + 1;      //Increments the byte count to keep track of packet boundry
    	      front_count <= front_count+1;
    	  end
     end

   // Data shift registers
   always @(negedge clk)         //
    begin
		if(!enable) 
	  		begin
        	data_reg1 <= 8'h00;           //Initializes the registers 
        	data_reg2 <= 8'h00;           
        	data_reg3 <= 8'h00;
	  	end
		else 
	 		begin
        	data_reg1 <= data_in;     // Registers incoming data, shifts to compare registers 
        	data_reg2 <= data_reg1;   // reg2 and reg3 are compare registers
        	data_reg3 <= data_reg2;  
 	  		end
    end
 
//  Search and validate
  
  always @(negedge clk)          //
     begin
		if(!enable)    //  if (!enable)  
	  		begin
             lock <= 0;
             shift <= 0;
             data_out <= 8'h00;  ///data_out <= 8'h00;     //////
             data_valid <= 0;
			 frame_length <= 0;
	  		end
	
	 	else   //else 1
	  		begin  //
			    //data_out <= 8'hff; 
       			if(!lock)                //If not locked, search for lock pattern
          			begin //begin search
                         
                       // data_out <= data_reg2; 
							  /// temporarily added 64 into this

		   				if(data_reg3 === 8'hf5 & data_reg2 === 8'h08/*8'h64*/) // modified by zjs, 2009-3-9 
		       				begin
		                      // data_out <= 8'hff; 
							//data_valid <= 1;
                       		lock <= 1;
                       		shift <= 3'h0;
		       				end
		
		   				else if({data_reg3[6:0],data_reg2[7]} === 8'hf5 & {data_reg2[6:0],data_reg1[7]} === 8'h08/*8'h64*/ ) 
               				begin
			     			lock <= 1;
			     			shift <= 3'h1;   
               				end

		    			else if({data_reg3[5:0],data_reg2[7:6]} === 8'hf5 & {data_reg2[5:0],data_reg1[7:6]} === 8'h08/*8'h64*/) 
			     			begin
			       			lock <= 1;
			       			shift <= 3'h2;   
			     			end
			
            			else if({data_reg3[4:0],data_reg2[7:5]} === 8'hf5 & {data_reg2[4:0],data_reg1[7:5]} === 8'h08/*8'h64*/) 
			    			begin
			       			lock <= 1;
			       			shift <= 3'h3;   
			    		    end
			
						else if({data_reg3[3:0],data_reg2[7:4]} === 8'hf5 & {data_reg2[3:0],data_reg1[7:4]} === 8'h08/*8'h64*/) 
			    			begin
                    		lock <= 1;
                    		shift <= 3'h4;   
			    			end
			
			 			else if({data_reg3[2:0],data_reg2[7:3]} === 8'hf5 & {data_reg2[2:0],data_reg1[7:3]} === 8'h08/*8'h64*/)
                  			begin
				    		lock <= 1;
				    		shift <= 3'h5;   
                  		    end

			 			else if({data_reg3[1:0],data_reg2[7:2]} === 8'hf5 & {data_reg2[1:0],data_reg1[7:2]} === 8'h08/*8'h64*/) 
				   			begin
				    		lock <= 1;
				    		shift <= 3'h6;   
				   		    end
				
			 			else if({data_reg3[0],data_reg2[7:1]} === 8'hf5 & {data_reg2[0],data_reg1[7:1]} === 8'h08/*8'h64*/)    //lock_pattern
				  			begin
				     		lock <= 1;
				     		shift <= 3'h7;   
			  			    end
							
	         		end    //if (!lock)                           // end search
	
	     		else if (lock)   
            	  begin                         	 	//Confirms that data is valid					
		         /*   if( byte_count == 8'h00)     		//the frame head
					    begin
		               	data_valid <= 0;	
						data_out <= 8'hff;       //output the frame head
						end	
					else if(byte_count == 8'h01)     	//the frame length
						begin
						data_valid <= 0;
						data_out <= data_out_tmp; 
						frame_length <= data_out_tmp;
						end*/
					if(byte_count < 8) // modified by zjs, 2009-3-9
					    begin
						data_valid <= 1;
						data_out <= data_out_tmp; 
						end
					else
						begin
			 			data_valid <= 0;
			 			lock <= 0;
			 			shift <= 0;
			            frame_length <= 0;
               			end
            	  end  //end if(lock)
			   
	      end     //end else if(enable)
     end    //end always


endmodule
