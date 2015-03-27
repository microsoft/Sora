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
// Create Date:    17:27:17 08/15/2011 
// Design Name: 
// Module Name:    RCB_FRL_BYTE_Alignment 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//				Byte alignment and Sora-FRL packet decapsulation
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module RCB_FRL_BYTE_Alignment(
		input     clk,                      
		input     enable,                 //Active low reset
		input [7:0] data_in,           
		
		output reg		data_valid,
		output reg [7:0]	data_out
   );

//   modified by zjs, 2009-3-9, none of the parameters are really used   
//   parameter lock_pattern = 8'hf5;   //Specifies the lock pattern
//   parameter pack_size = 100;             // Current packet size 
//   parameter word_size = 8;          // Not currently used in code
   
  // output       lockk;
   
   reg [7:0] 	data_reg1;              //First register used to re-align the data
   reg [7:0] 	data_reg2;              //Second register used to re-align the data
   reg [7:0] 	data_reg3;              //Third register used to re-align the data
   reg [2:0] 	shift;                  //designate the bit shift value of the package
   reg 		    lock;                   //if high, then the correct shift value has been found.
   reg [7:0] 	byte_count;             //word count in packet
   reg [2:0]    front_count;
   reg [7:0] 	data_out_tmp;           //pipeline register
//   reg 	    	data_valid;

   reg [7:0] 	frame_length;      
//   reg [7:0] 	data_out;               // output register
   
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
//		   				if(data_reg3 === 8'hf5 & data_reg2 === 8'h1D/*8'h64*/) // modified by zjs, 2009-3-9 
		       				begin
		                      // data_out <= 8'hff; 
							//data_valid <= 1;
                       		lock <= 1;
                       		shift <= 3'h0;
		       				end
		
		   				else if({data_reg3[6:0],data_reg2[7]} === 8'hf5 & {data_reg2[6:0],data_reg1[7]} === 8'h08/*8'h64*/ ) 
//		   				else if({data_reg3[6:0],data_reg2[7]} === 8'hf5 & {data_reg2[6:0],data_reg1[7]} === 8'h1D/*8'h64*/ ) 
               				begin
			     			lock <= 1;
			     			shift <= 3'h1;   
               				end

		    			else if({data_reg3[5:0],data_reg2[7:6]} === 8'hf5 & {data_reg2[5:0],data_reg1[7:6]} === 8'h08/*8'h64*/) 
//		    			else if({data_reg3[5:0],data_reg2[7:6]} === 8'hf5 & {data_reg2[5:0],data_reg1[7:6]} === 8'h1D/*8'h64*/) 
			     			begin
			       			lock <= 1;
			       			shift <= 3'h2;   
			     			end
			
            			else if({data_reg3[4:0],data_reg2[7:5]} === 8'hf5 & {data_reg2[4:0],data_reg1[7:5]} === 8'h08/*8'h64*/) 
//            			else if({data_reg3[4:0],data_reg2[7:5]} === 8'hf5 & {data_reg2[4:0],data_reg1[7:5]} === 8'h1D/*8'h64*/) 
			    			begin
			       			lock <= 1;
			       			shift <= 3'h3;   
			    		    end
			
						else if({data_reg3[3:0],data_reg2[7:4]} === 8'hf5 & {data_reg2[3:0],data_reg1[7:4]} === 8'h08/*8'h64*/) 
//						else if({data_reg3[3:0],data_reg2[7:4]} === 8'hf5 & {data_reg2[3:0],data_reg1[7:4]} === 8'h1D/*8'h64*/) 
			    			begin
                    		lock <= 1;
                    		shift <= 3'h4;   
			    			end
			
			 			else if({data_reg3[2:0],data_reg2[7:3]} === 8'hf5 & {data_reg2[2:0],data_reg1[7:3]} === 8'h08/*8'h64*/)
//			 			else if({data_reg3[2:0],data_reg2[7:3]} === 8'hf5 & {data_reg2[2:0],data_reg1[7:3]} === 8'h1D/*8'h64*/)
                  			begin
				    		lock <= 1;
				    		shift <= 3'h5;   
                  		    end

			 			else if({data_reg3[1:0],data_reg2[7:2]} === 8'hf5 & {data_reg2[1:0],data_reg1[7:2]} === 8'h08/*8'h64*/) 
//			 			else if({data_reg3[1:0],data_reg2[7:2]} === 8'hf5 & {data_reg2[1:0],data_reg1[7:2]} === 8'h1D/*8'h64*/) 
				   			begin
				    		lock <= 1;
				    		shift <= 3'h6;   
				   		    end
				
			 			else if({data_reg3[0],data_reg2[7:1]} === 8'hf5 & {data_reg2[0],data_reg1[7:1]} === 8'h08/*8'h64*/)    //lock_pattern
//			 			else if({data_reg3[0],data_reg2[7:1]} === 8'hf5 & {data_reg2[0],data_reg1[7:1]} === 8'h1D/*8'h64*/)    //lock_pattern
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
//					if(byte_count < 8) // modified by zjs, 2009-3-9
					if(byte_count < 29) // modified by zjs, 2009-3-9
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
