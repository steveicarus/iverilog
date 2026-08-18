// Check that an alias of a local forward declared class shadows an outer class.

class Base;
  integer value = 13;
endclass

module test;

  typedef class Base;
  typedef Base BaseAlias;

  class Derived extends BaseAlias;
  endclass

  class Base;
    integer value = 42;
  endclass

  Derived object;

  initial begin
    object = new;

    if (object.value !== 42) begin
      $display("FAILED: expected 42, got %0d", object.value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
