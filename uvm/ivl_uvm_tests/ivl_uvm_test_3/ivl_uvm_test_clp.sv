// ========== Copyright Header Begin ==========================
// 
// Project: IVL_UVM
// File: ivl_uvm_run_test.sv
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

module ivl_uvm_run_test;
  import ivl_uvm_pkg::*;
  uvm_object u0;
  uvm_component uc_0;

  initial begin
    run_test ();
  end

  
   initial begin : test
     #100;
     `uvm_info("IVL_UVM", "UVM_MEDIUM: Hello World", UVM_MEDIUM) 
     u0 = new ();
     u0.print();

     uc_0 = new("uc_0", null);
     uc_0.ivl_uvm_run_all_phases ();
     `g2u_display ("Expect a UVM_TIMEOUT error in this test")
     #10ms;
     `g2u_display ("DO not expect to see this print due to UVM_TIMEOUT error in this test")

     report_summarize ();
   end : test

endmodule : ivl_uvm_run_test
