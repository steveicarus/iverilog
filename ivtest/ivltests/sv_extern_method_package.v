// Check extern methods of classes declared in a package, where the base
// class method is virtual and the derived class overrides it.

package test_pkg;
  virtual class base_test;
    function new ();
    endfunction : new
    extern virtual task run_phase ();
  endclass : base_test

  class child_test extends base_test;
    extern virtual task run_phase ();
  endclass : child_test

  task base_test::run_phase ();
    $display ("base_test::run_phase");
  endtask : run_phase

  task child_test::run_phase ();
    $display ("child_test::run_phase");
  endtask : run_phase
endpackage : test_pkg

module m;
  import test_pkg::*;
  base_test bh;
  child_test ch;

  initial begin
    ch = new ();
    bh = ch;
    bh.run_phase ();
    $display ("PASSED");
  end
endmodule : m
