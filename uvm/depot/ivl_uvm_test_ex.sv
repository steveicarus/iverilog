`ifndef UVM_TESTNAME 
  `define UVM_TESTNAME ivl_uvm_base_test
`endif // UVM_TESTNAME 

package test_pkg;
  class uvm_component;
  endclass : uvm_component

  virtual class ivl_uvm_base_test extends uvm_component;
    function new (string name="ivl_uvm_base_test",
                  uvm_component parent = null);
    endfunction : new 

    virtual task run_phase ();
      $display ("%m run_phase");
    endtask : run_phase 
  endclass : ivl_uvm_base_test

  class my_test extends ivl_uvm_base_test;
    function new (string name="my_test",
                  uvm_component parent = null);
      super.new (name, parent);
    endfunction : new 

    virtual task run_phase ();
      $display ("%m run_phase");
    endtask : run_phase 
  endclass : my_test 

  `UVM_TESTNAME uvm_test_top;

  task run_test ();

    uvm_test_top = new();
    uvm_test_top.run_phase ();
  endtask : run_test 
endpackage : test_pkg

module m;
  import test_pkg::*;

  initial begin
    run_test ();
  end
endmodule : m


