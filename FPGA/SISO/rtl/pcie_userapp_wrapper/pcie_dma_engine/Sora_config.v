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
// Create Date:    21:39:39 01/05/2010 
// Design Name: 
// Module Name:    parameters for Sora's configuration 
// Project Name: Sora
// Target Devices: Virtex5 LX50T
// Tool versions: ISE10.1.03
// Description:  
// Purpose: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////


// `define sora_simulation          // PCIe endpoint v1.8 is used in simulation, v1.9 is used in implementation

 `define RCB_REGISTER_NEW_LAYOUT

/////////// One and Only one should be enabled in the following three options 
//`define WARP_RADIO_REGISTERS	// only if WARPRadio is defined
//`define SORA_RADIO_REGISTERS	// temp usage, radio registers mapped into RCB registers 
 `define RADIO_CHANNEL_REGISTERS

/////////// WARP radio with GPIOs /////////////
// `define WARPRadio   // enabled if parallel model for WARP is used

/////////// Sora Fast Radio Link //////////////
 `define SORA_FRL
// `define SORA_FRL_2nd

//////////// one of the following two is enabled only if WARPRadio is defined and one of SORA_RADIO_REGISTERS and RADIO_CHANNEL_REGISTERS is defined
//////////// `define SORA_RADIO_REGISTERS_TO_WARP
// `define RADIO_CHANNEL_REGISTERS_TO_WARP

//////////// sample clock from FPGA can be differential or signal wire, WARP radio only
// `define DIFF_SAMPLE_CLOCK
// `define SINGLE_SAMPLE_CLOCK

//////////// `define SAMPLE40MHZ