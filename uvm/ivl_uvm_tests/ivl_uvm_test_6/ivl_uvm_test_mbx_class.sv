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

`define IVL_UVM_MBX_T apb_xn

package test_pkg;
  import ivl_uvm_pkg::*;

  typedef enum {IDLE, APB_WR, APB_RD} apb_xfer_e;

  class apb_xn extends uvm_object;
    rand bit [7:0] addr;
    rand bit [7:0] data;
    rand apb_xfer_e xfer_kind;
  endclass : apb_xn 

  class sanity_test extends uvm_test;
    function new (string name = "sanity_test");
      super.new(name);
      `g2u_display ("%m");
    endfunction : new
  endclass : sanity_test 

endpackage : test_pkg

module ivl_uvm_run_test;
  import ivl_uvm_pkg::*;
  import test_pkg::*;
  
  ivl_uvm_mbx i_mbx_0;
  int lv_put_val;
  int lv_get_val;

  task do_puts();
    repeat (10) begin
      lv_put_val = $urandom();
      `g2u_printf (("Test MBX: Put: 0x%0h", lv_put_val))
      i_mbx_0.put (lv_put_val);
      #10;
    end
  endtask : do_puts

  task do_gets();
    repeat (10) begin
      #5;
      i_mbx_0.get (lv_get_val);
      `g2u_printf (("Test MBX: Get: 0x%0h", lv_get_val))
    end
  endtask : do_gets

   initial begin : test
     #100;
     `uvm_info("IVL_UVM", "UVM_MEDIUM: Hello World", UVM_MEDIUM) 
     i_mbx_0 = new();

     fork
       do_puts();
       do_gets();
     join



     $finish (2);
   end : test

endmodule : ivl_uvm_run_test
