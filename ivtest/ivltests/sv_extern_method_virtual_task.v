// Extern virtual tasks in a base and a derived class, defined in the
// package after the classes, with end labels.

package test_pkg;
  string last;

  virtual class base_test;
    function new (); endfunction : new
    extern virtual task run_phase ();
  endclass : base_test

  class child_test extends base_test;
    extern virtual task run_phase ();
  endclass : child_test

  task base_test::run_phase ();
    last = "base_test::run_phase";
    $display("base_test::run_phase");
  endtask : run_phase

  task child_test::run_phase ();
    last = "child_test::run_phase";
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
    if (last == "child_test::run_phase")
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule : m
