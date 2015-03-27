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
// Module Name:    parameters for Sora's PCIe configuration 
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

//////////////////////////////////////
//////// PCIe settings ///////////////
//////////////////////////////////////

`ifdef sora_simulation
	  `define BOARDx08 1
`else		  
	  `define BOARDx04 1
`endif

`ifdef BOARDx01                        

`define PCI_EXP_EP                     endpoint_blk_plus_v1_5 
`define XILINX_PCI_EXP_EP               xilinx_pci_exp_1_lane_ep
`define PCI_EXP_LINK_WIDTH              1

`endif // BOARDx01

`ifdef BOARDx04                        

`define PCI_EXP_EP                      endpoint_blk_plus_v1_5 
`define XILINX_PCI_EXP_EP               xilinx_pci_exp_4_lane_ep
`define PCI_EXP_LINK_WIDTH              4

`endif // BOARDx04

`ifdef BOARDx08

`define PCI_EXP_EP                      endpoint_blk_plus_v1_5
`define XILINX_PCI_EXP_EP               xilinx_pci_exp_8_lane_ep
`define PCI_EXP_LINK_WIDTH              8

`endif // BOARDx08

`define PCI_EXP_EP_INST                 ep

//-------------------------------------------------------
// Config File Module
//-------------------------------------------------------

`define PCI_EXP_CFG                     pci_exp_cfg

`define PCI_EXP_CFG_INST                pci_exp_cfg

//-------------------------------------------------------
// Transaction (TRN) Interface
//-------------------------------------------------------

`define PCI_EXP_TRN_DATA_WIDTH          64
`define PCI_EXP_TRN_REM_WIDTH           8
`define PCI_EXP_TRN_BUF_AV_WIDTH        4

`define PCI_EXP_TRN_BAR_HIT_WIDTH       7
`define PCI_EXP_TRN_FC_HDR_WIDTH        8
`define PCI_EXP_TRN_FC_DATA_WIDTH       12

//-------------------------------------------------------
// Application Stub Module
//-------------------------------------------------------

`define PCI_EXP_APP                     pci_exp_64b_app

//-------------------------------------------------------
// Configuration (CFG) Interface
//-------------------------------------------------------

`define PCI_EXP_CFG_DATA_WIDTH          32
`define PCI_EXP_CFG_ADDR_WIDTH          10
`define PCI_EXP_CFG_BUSNUM_WIDTH        8
`define PCI_EXP_CFG_DEVNUM_WIDTH        5
`define PCI_EXP_CFG_FUNNUM_WIDTH        3
`define PCI_EXP_CFG_CPLHDR_WIDTH        48
`define PCI_EXP_CFG_CAP_WIDTH           16
`define PCI_EXP_CFG_CFG_WIDTH           1024
`define PCI_EXP_CFG_WIDTH               1024
`define PCI_EXP_LNK_STATE_WIDTH         3

`define PCI_EXP_CFG_BUSNUM_WIDTH        8
`define PCI_EXP_CFG_DEVNUM_WIDTH        5
`define PCI_EXP_CFG_FUNNUM_WIDTH        3
`define PCI_EXP_CFG_DSN_WIDTH           64
`define PCI_EXP_EP_OUI                  24'h000A35
`define PCI_EXP_EP_DSN_1                {{8'h1},`PCI_EXP_EP_OUI}
`define PCI_EXP_EP_DSN_2                32'h00000001

//////////////////////////////////////
//////// PCIe settings ///////////////
//////////////////////////////////////
