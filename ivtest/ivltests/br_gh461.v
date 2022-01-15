package u_lib;

  class uvm_void;
  endclass
  class uvm_object extends uvm_void;
    task passed;
      $display("PASSED");
    endtask
  endclass

endpackage

package t1_pkg;
  import u_lib::*;

  class drvr extends uvm_object;
  endclass
endpackage : t1_pkg

module m;
  import t1_pkg::*;

  drvr obj;
  initial begin
    obj = new;
    obj.passed();
  end
endmodule
