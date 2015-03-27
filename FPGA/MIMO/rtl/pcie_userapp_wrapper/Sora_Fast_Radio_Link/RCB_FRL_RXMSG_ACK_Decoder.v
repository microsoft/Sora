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
// Create Date:    19:29:55 08/15/2011 
// Design Name: 
// Module Name:    RCB_FRL_RXMSG_ACK_Decoder 
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
module RCB_FRL_RXMSG_ACK_Decoder(
   input     	clk,                      
   input     	enable,                 //Active low reset
   input [7:0]	data_in,           
	output reg	CON_P, 
	output reg	CON_N
	);

//   parameter lock_pattern = 8'hff	;   //Specifies the lock pattern
//   parameter pack_size = 100;             // Current packet size 
//   parameter word_size = 8;          // Not currently used in code
   		
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
	reg			data_correct;
	reg			data_wrong;
	reg [39:0]  data_all;
   
//   wire 	end_pack = pack_size;   // signifies end of packet
	
   always @(negedge clk)   // timing broken to set data_valid flag
     begin
		if (!enable)
		begin
    	      data_out_tmp <= 8'h00;    //test          
		end
		else
		begin
    	    case(shift)               //Re-aligns the data depending on shift value
		    	3'h0 : data_out_tmp <= data_reg3;
		    	3'h1 : data_out_tmp <= ({data_reg3[6:0],data_reg2[7]});
		    	3'h2 : data_out_tmp <= ({data_reg3[5:0],data_reg2[7:6]});
		    	3'h3 : data_out_tmp <= ({data_reg3[4:0],data_reg2[7:5]});
		    	3'h4 : data_out_tmp <= ({data_reg3[3:0],data_reg2[7:4]});
		    	3'h5 : data_out_tmp <= ({data_reg3[2:0],data_reg2[7:3]});
		    	3'h6 : data_out_tmp <= ({data_reg3[1:0],data_reg2[7:2]});
		    	3'h7 : data_out_tmp <= ({data_reg3[0],data_reg2[7:1]});
		    	default : data_out_tmp <= data_reg3;
	        endcase
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
		if(!enable)  
	  		begin
             lock <= 0;
             shift <= 0;
             data_out <= 8'h00;  
             data_valid <= 0;
				 frame_length <= 0;
	  		end
	
	 	else   
		begin
			if(data_reg3 === 8'h5f ) 
		       				begin
		                      CON_P <= 1'b1;
		       				end
		
		   				else if({data_reg3[6:0],data_reg2[7]} === 8'h5f ) 
               				begin
			     			 CON_P <= 1'b1;  
               				end

		    			else if({data_reg3[5:0],data_reg2[7:6]} === 8'h5f ) 
			     			begin
			       			 CON_P <= 1'b1;
			     			end
			
            			else if({data_reg3[4:0],data_reg2[7:5]} === 8'h5f ) 
			    			begin
			       			 CON_P <= 1'b1;
			    		    end
			
						else if({data_reg3[3:0],data_reg2[7:4]} === 8'h5f ) 
			    			begin
                    		 CON_P <= 1'b1;
			    			end
			
			 			else if({data_reg3[2:0],data_reg2[7:3]} === 8'h5f )
                  			begin
				    	 CON_P <= 1'b1;
                  		    end

			 			else if({data_reg3[1:0],data_reg2[7:2]} === 8'h5f) 
				   			begin
				    		 CON_P <= 1'b1;
				   		    end
				
			 			else if({data_reg3[0],data_reg2[7:1]} === 8'h5f)    //lock_pattern
				  			begin
				     		 CON_P <= 1'b1;
			  			    end
							 else begin
							 CON_P <= 1'b0;
							end
			   				if(data_reg3 === 8'haf ) 
		       				begin
		                      CON_N <= 1'b1;
		       				end
		
		   				else if({data_reg3[6:0],data_reg2[7]} === 8'haf ) 
               				begin
			     			 CON_N <= 1'b1;  
               				end

		    			else if({data_reg3[5:0],data_reg2[7:6]} === 8'haf ) 
			     			begin
			       			 CON_N <= 1'b1;
			     			end
			
            			else if({data_reg3[4:0],data_reg2[7:5]} === 8'haf ) 
			    			begin
			       			 CON_N <= 1'b1;
			    		    end
			
						else if({data_reg3[3:0],data_reg2[7:4]} === 8'haf ) 
			    			begin
                    		 CON_N <= 1'b1;
			    			end
			
			 			else if({data_reg3[2:0],data_reg2[7:3]} === 8'haf )
                  			begin
				    	 CON_N <= 1'b1;
                  		    end

			 			else if({data_reg3[1:0],data_reg2[7:2]} === 8'haf )
				   			begin
				    		 CON_N <= 1'b1;
				   		    end
				
			 			else if({data_reg3[0],data_reg2[7:1]} === 8'haf    )//lock_pattern
				  			begin
				     		 CON_N <= 1'b1;
			  			    end
							 else begin
							 CON_N <= 1'b0;
							end
							
	         		end    //if (!lock)                           // end search
	

	end

endmodule

