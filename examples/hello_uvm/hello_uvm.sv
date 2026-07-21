// Minimal smoke test for the seeded uvm/ (IVL_UVM) library.
// Not Accellera UVM — exercises messaging + run_test only.

module hello_uvm;
  import ivl_uvm_pkg::*;

  initial begin
    run_test();
  end

  initial begin
    #10;
    `uvm_info("HELLO", "hello_uvm smoke: UVM_MEDIUM", UVM_MEDIUM)
    `uvm_info("HELLO", "hello_uvm smoke: done", UVM_NONE)
    #10;
    $finish;
  end
endmodule
