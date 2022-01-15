package test_pkg;

  class uvm_phase;
    function void print(string str);
      $display(str);
    endfunction
  endclass : uvm_phase

  class uvm_component;
    virtual function void build_phase(uvm_phase phase);
      phase.print("building");
    endfunction : build_phase

    virtual task run_phase(uvm_phase phase);
      phase.print("running");
    endtask : run_phase

    virtual task run_all();
      uvm_phase p0;

      p0 = new();

      this.build_phase(p0);
      this.run_phase(p0);
    endtask : run_all

  endclass : uvm_component

endpackage : test_pkg

module m;
  import test_pkg::*;
  uvm_component u0;

  initial begin : test
    u0 = new();
    u0.run_all();
  end : test

endmodule : m
