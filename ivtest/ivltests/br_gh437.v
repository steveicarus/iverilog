package ivl_uvm_pkg;
  virtual class uvm_test;
    task ok;
      $display("PASSED");
    endtask
  endclass : uvm_test
endpackage : ivl_uvm_pkg

package test_pkg;
  import ivl_uvm_pkg::*;
  class sanity_test extends uvm_test;
  endclass : sanity_test
endpackage : test_pkg

module m;

  import test_pkg::*;

  sanity_test obj;

  initial begin
    obj = new;
    obj.ok;
  end

endmodule : m
