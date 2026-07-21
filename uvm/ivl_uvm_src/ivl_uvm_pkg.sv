// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_pkg.sv
// Author(s): Anirudh Pradyumnan (apseng03@gmail.com)
//            Srinivasan Venkataramanan 
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
`ifndef IVL_UVM_PKG
  `define IVL_UVM_PKG

  `define IVL_UVM
  `define UVM_CMDLINE_NO_DPI
`timescale 1ns/1ns

package ivl_uvm_pkg;
  `include "ivl_uvm_macros.svh"
  `include "ivl_uvm_types.svh"

  `include "ivl_uvm_patches.svh"
  `include "ivl_uvm_msg.svh"
  `include "base/uvm_misc.svh"
  `include "base/uvm_object.svh"
  `include "ivl_uvm_comps.svh"
  `include "ivl_uvm_mbx.svh"
  `include "ivl_uvm_tlm.svh"

  `UVM_TESTNAME uvm_test_top;

  task run_test ();
    `g2u_printf (( "Using UVM_TESTNAME: %s", `GO2UVM_DISP_ARG (`UVM_TESTNAME) ))
    uvm_test_top = new ("uvm_test_top", null);
    uvm_test_top.ivl_uvm_run_all_phases();

  endtask : run_test


endpackage : ivl_uvm_pkg
import ivl_uvm_pkg::*;

`include "ivl_uvm_clp.svh"

// The below file is now part of user's file-list as last file
// `include "ivl_uvm_top.svh"
`endif //  IVL_UVM_PKG

