#------------------------------------------------------------------------------
#  (c) Copyright 2013-2015 Xilinx, Inc. All rights reserved.
#
#  This file contains confidential and proprietary information
#  of Xilinx, Inc. and is protected under U.S. and
#  international copyright and other intellectual property
#  laws.
#
#  DISCLAIMER
#  This disclaimer is not a license and does not grant any
#  rights to the materials distributed herewith. Except as
#  otherwise provided in a valid license issued to you by
#  Xilinx, and to the maximum extent permitted by applicable
#  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
#  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
#  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
#  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
#  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
#  (2) Xilinx shall not be liable (whether in contract or tort,
#  including negligence, or under any other theory of
#  liability) for any loss or damage of any kind or nature
#  related to, arising under or in connection with these
#  materials, including for any direct, or any indirect,
#  special, incidental, or consequential loss or damage
#  (including loss of data, profits, goodwill, or any type of
#  loss or damage suffered as a result of any action brought
#  by a third party) even if such damage or loss was
#  reasonably foreseeable or Xilinx had been advised of the
#  possibility of the same.
#
#  CRITICAL APPLICATIONS
#  Xilinx products are not designed or intended to be fail-
#  safe, or for use in any application requiring fail-safe
#  performance, such as life-support or safety devices or
#  systems, Class III medical devices, nuclear facilities,
#  applications related to the deployment of airbags, or any
#  other applications that could lead to death, personal
#  injury, or severe property or environmental damage
#  (individually and collectively, "Critical
#  Applications"). Customer assumes the sole risk and
#  liability of any use of Xilinx products in Critical
#  Applications, subject only to applicable laws and
#  regulations governing limitations on product liability.
#
#  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
#  PART OF THIS FILE AT ALL TIMES.
#------------------------------------------------------------------------------


# UltraScale FPGAs Transceivers Wizard IP core-level XDC file
# ----------------------------------------------------------------------------------------------------------------------

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y0
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y0 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[24].*gen_gthe3_channel_inst[0].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin BC1 [get_ports gthrxn_in[0]]
#set_property package_pin BC2 [get_ports gthrxp_in[0]]
#set_property package_pin BF4 [get_ports gthtxn_out[0]]
#set_property package_pin BF5 [get_ports gthtxp_out[0]]

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y1
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y1 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[24].*gen_gthe3_channel_inst[1].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin BA1 [get_ports gthrxn_in[1]]
#set_property package_pin BA2 [get_ports gthrxp_in[1]]
#set_property package_pin BD4 [get_ports gthtxn_out[1]]
#set_property package_pin BD5 [get_ports gthtxp_out[1]]

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y2
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y2 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[24].*gen_gthe3_channel_inst[2].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin AW3 [get_ports gthrxn_in[2]]
#set_property package_pin AW4 [get_ports gthrxp_in[2]]
#set_property package_pin BB4 [get_ports gthtxn_out[2]]
#set_property package_pin BB5 [get_ports gthtxp_out[2]]

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y3
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y3 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[24].*gen_gthe3_channel_inst[3].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin AV1 [get_ports gthrxn_in[3]]
#set_property package_pin AV2 [get_ports gthrxp_in[3]]
#set_property package_pin AV6 [get_ports gthtxn_out[3]]
#set_property package_pin AV7 [get_ports gthtxp_out[3]]

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y4
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y4 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[25].*gen_gthe3_channel_inst[0].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin AU3 [get_ports gthrxn_in[4]]
#set_property package_pin AU4 [get_ports gthrxp_in[4]]
#set_property package_pin AU8 [get_ports gthtxn_out[4]]
#set_property package_pin AU9 [get_ports gthtxp_out[4]]

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y5
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y5 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[25].*gen_gthe3_channel_inst[1].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin AT1 [get_ports gthrxn_in[5]]
#set_property package_pin AT2 [get_ports gthrxp_in[5]]
#set_property package_pin AT6 [get_ports gthtxn_out[5]]
#set_property package_pin AT7 [get_ports gthtxp_out[5]]

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y6
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y6 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[25].*gen_gthe3_channel_inst[2].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin AR3 [get_ports gthrxn_in[6]]
#set_property package_pin AR4 [get_ports gthrxp_in[6]]
#set_property package_pin AR8 [get_ports gthtxn_out[6]]
#set_property package_pin AR9 [get_ports gthtxp_out[6]]

# Commands for enabled transceiver GTHE3_CHANNEL_X1Y7
# ----------------------------------------------------------------------------------------------------------------------

# Channel primitive location constraint
set_property LOC GTHE3_CHANNEL_X1Y7 [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[25].*gen_gthe3_channel_inst[3].GTHE3_CHANNEL_PRIM_INST}]

# Channel primitive serial data pin location constraints
# (Provided as comments for your reference. The channel primitive location constraint is sufficient.)
#set_property package_pin AP1 [get_ports gthrxn_in[7]]
#set_property package_pin AP2 [get_ports gthrxp_in[7]]
#set_property package_pin AP6 [get_ports gthtxn_out[7]]
#set_property package_pin AP7 [get_ports gthtxp_out[7]]


# False path constraints
# ----------------------------------------------------------------------------------------------------------------------
