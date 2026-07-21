// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_test_msg.sv
// Author(s): Anirudh Pradyumnan (apseng03@gmail.com)
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
module ivl_uvm_test_msg;
  import ivl_uvm_pkg::*;
  
  initial begin
    run_test ();
  end

   initial begin : test
     #100;
     `uvm_info("IVL_UVM", "UVM_MEDIUM: Hello World", UVM_MEDIUM) 
     `uvm_info("IVL_UVM", "UVM_HIGH: Hello World", UVM_HIGH) 
     `uvm_info("IVL_UVM", "UVM_FULL: Hello World", UVM_FULL) 
     #100 `uvm_info("IVL_UVM", "NONE: Hello World", UVM_NONE) 

     #100 `uvm_warning("IVL_UVM", "Sample Warning!")
     #100 `uvm_error("IVL_UVM", "Sample Error!")
     #100 `uvm_error("IVL_UVM", "Sample Error!")
     #100 `uvm_error("IVL_UVM", "Sample Error!")
     #100 `uvm_fatal("IVL_UVM", "Sample Fatal!")
   end : test

endmodule : ivl_uvm_test_msg
