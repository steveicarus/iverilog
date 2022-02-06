// Check that base class defined in a package is handled correctly

package P;
  class B;
    task check;
      $display("PASSED");
    endtask
  endclass
endpackage

module test;

  class B;
    task check;
      $display("FAILED");
    endtask
  endclass

  class C extends P::B;
  endclass

  C c;

  initial begin
    c = new;
    c.check();
  end

endmodule
