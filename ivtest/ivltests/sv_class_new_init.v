// Check that the class new initializer can be used for all sorts for variable
// declarations

package P;
  class C;
  endclass
  C c = new;
endpackage

module test;

  class C;
    task check;
      $display("PASSED");
    endtask
  endclass

  class D;
    C c = new;
  endclass

  C c = new;

  initial begin
    static C c = new;
    c.check();
  end

endmodule
