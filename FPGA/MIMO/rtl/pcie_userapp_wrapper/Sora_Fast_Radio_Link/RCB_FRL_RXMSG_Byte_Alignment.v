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
// Create Date:    19:21:39 08/15/2011 
// Design Name: 
// Module Name:    RCB_FRL_RXMSG_Byte_Alignment 
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
module RCB_FRL_RXMSG_Byte_Alignment(   
		input     	clk,                      
		input     	enable,                 //Active low reset
		input [7:0]	data_in,           
		
		output reg		data_valid,
		output reg [7:0]	data_out,
		output reg		data_correct,
		output reg		data_wrong,
		output reg [39:0]	data_all
	);

//	parameter lock_pattern = 8'hff	;   //Specifies the lock pattern
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

   reg [7:0] 	frame_length;      
   
//   wire 	end_pack = pack_size;   // signifies end of packet
	
	wire [7:0] CRC_ans;
	RCB_FRL_CRC_gen RCB_FRL_CRC_gen_inst ( .D({{8'h06},data_all}), .NewCRC(CRC_ans));
   
   always @(negedge clk) // timing broken to set data_valid flag
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

 // Word counter, counts words in packet  
   always@(negedge clk)        
     begin
		 if(!enable || !lock)		    	      	 // Active low reset
			begin
    	      byte_count <= 0;                   //Initializes byte count
    	      front_count <= 0;
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
       			if(!lock)                //If not locked, search for lock pattern
          			begin //begin search
                         
							  data_correct <= 1'b0;
							  data_wrong <= 1'b0;

		   				if(data_reg3 === 8'hf5 ) 
		       				begin
                       		lock <= 1;
                       		shift <= 3'h0;
		       				end
		
		   				else if({data_reg3[6:0],data_reg2[7]} === 8'hf5 ) 
               				begin
			     			lock <= 1;
			     			shift <= 3'h1;   
               				end

		    			else if({data_reg3[5:0],data_reg2[7:6]} === 8'hf5 ) 
			     			begin
			       			lock <= 1;
			       			shift <= 3'h2;   
			     			end
			
            			else if({data_reg3[4:0],data_reg2[7:5]} === 8'hf5 ) 
			    			begin
			       			lock <= 1;
			       			shift <= 3'h3;   
			    		    end
			
						else if({data_reg3[3:0],data_reg2[7:4]} === 8'hf5 ) 
			    			begin
                    		lock <= 1;
                    		shift <= 3'h4;   
			    			end
			
			 			else if({data_reg3[2:0],data_reg2[7:3]} === 8'hf5 )
                  			begin
				    		lock <= 1;
				    		shift <= 3'h5;   
                  		    end

			 			else if({data_reg3[1:0],data_reg2[7:2]} === 8'hf5) 
				   			begin
				    		lock <= 1;
				    		shift <= 3'h6;   
				   		    end
				
			 			else if({data_reg3[0],data_reg2[7:1]} === 8'hf5)    //lock_pattern
				  			begin
				     		lock <= 1;
				     		shift <= 3'h7;   
			  			    end
							
	         		end    //if (!lock)                           // end search
	
	     		else if (lock)   
            	  begin                         	 	//Confirms that data is valid					
		            if( byte_count == 8'h00)     		//the frame head
					    begin
		               	data_valid <= 0;	
						data_out <= 8'hff;       //output the frame head
						end	
					else if(byte_count == 8'h01)     	//the frame length
						begin
						data_valid <= 0;
						data_out <= data_out_tmp; 
						frame_length <= data_out_tmp;
						end
					else if(byte_count < /*8'd7*/frame_length + 8'h1)
					    begin
						data_valid <= 1;
						data_out <= data_out_tmp;
						if (byte_count == 8'h02)
						begin
							data_all[39:32] <= data_out_tmp;
						end
						else if (byte_count == 8'h03)
						begin
							data_all[31:24] <= data_out_tmp;
						end
						else if (byte_count == 8'h04)
						begin
							data_all[23:16] <= data_out_tmp;
						end
						else if (byte_count == 8'h05)
						begin
							data_all[15:8] <= data_out_tmp;
						end
						else if (byte_count == 8'h06)
						begin
							data_all[7:0] <= data_out_tmp;
						end
					end
						
					else if (byte_count >= frame_length + 8'h1)
						begin
							data_valid <= 0;
							lock <= 0;
							shift <= 0;
			            frame_length <= 0;
							if ( CRC_ans == data_out_tmp)
							begin
								data_correct <=1'b1;
							end
							else
							begin
								data_wrong <=1'b1;
							end
               			end
            	  end  //end if(lock)
			   
	      end     //end else if(enable)
     end    //end always

endmodule
