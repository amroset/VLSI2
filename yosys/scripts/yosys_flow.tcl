# Copyright (c) 2025 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
#
# Synthesis script template for VLSI-2 EX04     
# To use this script in Yosys (from the `yosys` directory):                  
# % > source scripts/yosys_flow.tcl
#
# Authors:
# - Bowen Wang      <bowwang@iis.ee.ethz.ch>
# - Enrico Zelioli  <ezelioli@iis.ee.ethz.ch>
# - Pu Deng         <pudeng@iis.ee.ethz.ch>
# Last Modification: 02.02.2026

# set global variables
set sv_flist   "src/croc.flist"
set out_dir    out
set tmp_dir    tmp
set rep_dir    reports

# global variables imported from environment variables (if set)
# define with scheme: <local-var> { <ENVVAR>  <fallback> }
set proj_name  [expr {[info exists ::env(PROJ_NAME)]  ? $::env(PROJ_NAME)  : "croc"}]
set top_design [expr {[info exists ::env(TOP_DESIGN)] ? $::env(TOP_DESIGN) : "croc_chip"}]

file mkdir $out_dir
file mkdir $tmp_dir
file mkdir $rep_dir


#######################################
###### Read Technology Libraries ######
#######################################

# TODO: student task 2
# Read liberty files for standard cells, SRAM macros, and I/O pads
yosys read_liberty -lib ../technology/lib/ez130_8t_tt_1p20v_25c.lib
yosys read_liberty -lib ../technology/lib/RM_IHPSG13_1P_512x64_c2_bm_bist_typ_1p20V_25C.lib
yosys read_liberty -lib ../technology/lib/sg13g2_io_typ_1p2V_3p3V_25C.lib


#########################
###### Load Design ######
#########################

# TODO: student task 3 & 4
# 3.1: Enable Yosys SystemVerilog frontend
yosys plugin -i slang.so

# 3.2: Load Croc chip design
yosys read_slang --top croc_chip -f src/croc.flist --ignore-unknown-modules --keep-hierarchy
yosys stat

# map dont_touch attribute commonly applied to output-nets of async regs to keep
yosys attrmap -rename dont_touch keep
yosys attrmap -tocase keep -imap keep="true" keep=1
# copy the keep attribute to their driving cells (retain on net for debugging)
yosys attrmvcp -copy -attr keep


#########################
###### Elaboration ######
#########################

# TODO: student task 5
# 5.1 Resolve design hierarchy
yosys hierarchy -top croc_chip 
yosys check

# 5.2 Convert processes to netlists
yosys proc

# 5.3 Export report and netlist
yosys tee -q -o "reports/croc.rpt" stat

####################################
###### Coarse-grain Synthesis ######
####################################

# TODO: student task 6
# 6.1 Early-stage design check
yosys check

# 6.2 First opt pass (no FF)
yosys opt -noff

# 6.3 Extract FSM and write report
yosys fsm
yosys tee -q -o "reports/croc_fsm.rpt" stat

# 6.4 Perform wreduce and peepopt opt
yosys wreduce
yosys peepopt 

# 6.5 Clean and optimization
yosys opt_clean
yosys opt -full

# 6.6 Resource sharing and optmization
yosys share
yosys opt

# 6.7 Infer memories and optimize
yosys memory
yosys opt -fast

# 6.8 Optimize flip-flops
yosys opt_dff -sat -nodffe -nosdff


# 6.9 Final optmizations
yosys opt -full
yosys clean -purge

# 6.10 Write design and report
yosys tee -q -o "reports/croc_opt.rpt" stat


###########################################
###### Define target clock frequency ######
###########################################

# TODO: student task 7
# 7.1 Define clock period variable
set period_ps 20000

##################################
###### Fine-grain synthesis ######
##################################

# TODO: student task 9
# 9.1 Generic cell substitution
yosys techmap
yosys opt
yosys clean

# 9.2 Generate report
yosys tee -q -o "reports/croc_opt.rpt" stat

############################
###### Flatten design ######
############################

# Before flattening the hierarchy to allow cross-module optimizations,
# preserve hierarchy of selected modules/instances.
# 't' means type as in select all instances of this type/module
# yosys-slang uniquifies all modules with the naming scheme:
# <module-name>$<instance-name> -> match for t:<module-name>$$
# Examples:
# yosys setattr -set keep_hierarchy 1 "t:croc_soc$*"
# yosys setattr -set keep_hierarchy 1 "t:cdc_*$*"
# yosys setattr -set keep_hierarchy 1 "t:sync$*"

# TODO: student task 13
# 13.1 add keep_hierarchy
yosys setattr -set keep_hierarchy 1 soc_ctrl_reg_top
yosys setattr -set keep_hierarchy 1 tc_clk*
yosys setattr -set keep_hierarchy 1 tc_sram



# TODO: student task 12
# 12.1 Flatten design
yosys flatten
yosys clean -purge

################################
###### Technology Mapping ######
################################

# TODO: student task 10
# 10.1 Register mapping
yosys dfflibmap -liberty ../technology/lib/ez130_8t_tt_1p20v_25c.lib

# 10.2 Generate a report
yosys tee -q -o "reports/croc_opt.rpt" stat

# 10.3 Combinational logic mapping
yosys abc -liberty ../technology/lib/ez130_8t_tt_1p20v_25c.lib -D 20000 -constr scripts/abc-opt.script
yosys clean -purge

# 10.4 Export netlist


#######################################
###### Prepare for OpenROAD flow ######
#######################################

# TODO: student task 14
# 14.1 Split multi-bit nets
yosys splitnets

# 14.2 Replace undefined constants
yosys setundef -zero
yosys clean 

# 14.3 Replace constant bits with driver cells
yosys hilomap

# 14.4 Export
yosys write_verilog "out/croc.v"


exit

