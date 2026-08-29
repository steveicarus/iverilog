// Check that a forward declared base class works inside a package.

package p;

  typedef class Base;

  class Derived extends Base;
  endclass

  class Base;
    integer value = 42;
  endclass

endpackage

module test;

  p::Derived object;

  initial begin
    object = new;

    if (object.value !== 42) begin
      $display("FAILED: expected 42, got %0d", object.value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
