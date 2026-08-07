// Check const property initialization in a derived class.

module test;

  class Base;
    int inherited;
  endclass

  class Derived extends Base;
    const int value;

    function new;
      value = 42;
    endfunction
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
