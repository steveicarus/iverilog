// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_types.svh
// Author(s): Srinivasan Venkataramanan 
//
// Copyright (c) VerifWorks 2016-2020  All Rights Reserved.
// Contact us via: support@verifworks.com
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License version 3 as published by the Free Software Foundation.
// 
// This program is distributed in the hope that it will be 
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this work; if not, write to the Free Software
// 
// ========== Copyright Header End ============================
////////////////////////////////////////////////////////////////////////
`ifndef __IVL_UVM_TYPES__
`define __IVL_UVM_TYPES__

`include "base/uvm_object_globals.svh"


int uvm_info_counter;
int uvm_warn_counter;
int uvm_err_counter;
int uvm_fatal_counter;
int ivl_uvm_glb_verb;
bit report_summarize_done;

time ivl_uvm_glb_timeout;
int unsigned ivl_uvm_max_quit_count = '1;

string log_id = "IVL_GO2UVM";

typedef class `UVM_TESTNAME;

`endif // __IVL_UVM_TYPES__

